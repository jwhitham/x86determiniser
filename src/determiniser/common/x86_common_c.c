//#define DEBUG
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include <Zydis/Zydis.h>

#include "offsets.h"
#include "x86_flags.h"
#include "common.h"
#include "x86_common.h"

#define TRIGGER_LEVEL   0x1000000

// branch trace encoding data (see ../doc/branch_trace_format.pdf)
#define WORD_DATA_MASK  0x0fffffffU
#define CEM             0x00000000U
#define BNT             0x10000000U
#define SA              0x20000000U
#define BT              0x30000000U
#define SM              0x40000000U

#define BRANCH_TRACE_REFRESH_INTERVAL 10000


static uint8_t          entry_flag = 1;
uint8_t                 x86_quiet_mode = 0;

static uint8_t          fake_endpoint[8];
static uint32_t         min_address = 0;
static uint32_t         max_address = 0;

static uint32_t         branch_trace_temp = 0;
static uint32_t         branch_trace_refresh = BRANCH_TRACE_REFRESH_INTERVAL;
static FILE *           branch_trace = NULL;
static FILE *           out_trace = NULL;
static FILE *           inst_trace = NULL;

static uint64_t         inst_count = 0;
static ZydisDecoder     decoder;
static ZydisFormatter   formatter;

typedef struct superblock_info {
    uint32_t size;
    uint32_t count;
} superblock_info;

static superblock_info * superblocks;

extern uint32_t x86_other_context[];
extern uint8_t x86_switch_from_user[];
extern uint32_t x86_size_of_indirect_call_instruction;

void x86_switch_to_user (uint32_t endpoint);
void x86_make_text_writable (uint32_t min_address, uint32_t max_address);
void x86_begin_single_step (void);
int x86_is_branch_taken (uint32_t flags, uint8_t opcode);

static void dump_regs (uint32_t * gregs)
{
    printf (" EAX = %08x  ", gregs[REG_EAX]);
    printf (" EBX = %08x  ", gregs[REG_EBX]);
    printf (" ECX = %08x  ", gregs[REG_ECX]);
    printf (" EDX = %08x  ", gregs[REG_EDX]);
    printf (" EFL = %08x\n", gregs[REG_EFL]);
    printf (" EIP = %08x  ", gregs[REG_EIP]);
    printf (" ESI = %08x  ", gregs[REG_ESI]);
    printf (" EDI = %08x  ", gregs[REG_EDI]);
    printf (" EBP = %08x  ", gregs[REG_EBP]);
    printf (" ESP = %08x\n", gregs[REG_ESP]);
}

static void superblock_decoder (superblock_info * si, uint32_t pc);

// encode some element into the branch trace, with correct handling
// for values greater than 1 << 28
static void branch_trace_encode (uint32_t trace_opcode, uint32_t addr)
{
    if (branch_trace) {
        uint32_t top_nibble = addr >> 28;

        if ((top_nibble != branch_trace_temp)
        || (branch_trace_refresh == 0)) {
            /* set the value of the top nibble used in the branch trace using
             * the SM trace opcode */
            branch_trace_temp = top_nibble;
            branch_trace_refresh = BRANCH_TRACE_REFRESH_INTERVAL;
            fprintf (branch_trace, "%08x %08x\n", SM | top_nibble, (uint32_t) inst_count);
        }
        /* write an element to the branch trace */
        fprintf (branch_trace, "%08x %08x\n", trace_opcode | (addr & WORD_DATA_MASK), (uint32_t) inst_count);
        branch_trace_refresh --;
    }
}

// write taken branch to branch trace
static inline void branch_taken (uint32_t src, uint32_t dest)
{
    if (branch_trace
    && (src >= min_address) && (src <= max_address)
    && (dest >= min_address) && (dest <= max_address)) {
         branch_trace_encode (SA, src);
         branch_trace_encode (BT, dest);
    }
}

// write not taken branch to branch trace
static inline void branch_not_taken (uint32_t src)
{
    if (branch_trace
    && (src >= min_address) && (src <= max_address)) {
         branch_trace_encode (BNT, src);
    }
}

// single step print_trigger_counted
void x86_trap_handler (uint32_t * gregs, uint32_t trapno)
{
    uint32_t pc;

    if (trapno != 1) {
        fprintf (stderr, "trapno %u not known\n", trapno);
        dump_regs (gregs);
        exit (1);
        return;
    }
        
    pc = gregs[REG_EIP];
#ifdef DEBUG
    printf ("stepping EIP %08x ESP %08x entry_flag %u\n", pc, gregs[REG_ESP], entry_flag);
#endif

    if (!entry_flag) {
        // We have now stepped one instruction in the program, time to leave!
        // Fake an indirect call (to match the other way to exit from user code)
        gregs[REG_ESP] -= 4;
        ((uint32_t *) gregs[REG_ESP])[0] = gregs[REG_EIP] + x86_size_of_indirect_call_instruction; 
        gregs[REG_EIP] = (uint32_t) x86_switch_from_user;
#ifdef DEBUG
        printf ("switch from user: new EIP %08x ESP %08x\n", gregs[REG_EIP], gregs[REG_ESP]);
#endif
        gregs[REG_EFL] &= ~SINGLE_STEP_FLAG;
    } else if ((pc < min_address) || (pc > max_address)) {
        // Still outside program (probably completing the switch from super -> user)
        // Keep stepping
        gregs[REG_EFL] |= SINGLE_STEP_FLAG;
    } else {
        // Stepped one instruction
        entry_flag = 0;
        gregs[REG_EFL] |= SINGLE_STEP_FLAG;
    }
}

static int interpret_control_flow (void)
{
    uint32_t pc = x86_other_context[REG_EIP];
    uint32_t src = pc;
    uint8_t * pc_bytes = (uint8_t *) pc;
    uint32_t flags = x86_other_context[REG_EFL];
    uint32_t rm = 0;
    uint32_t * offset = NULL;
    uint32_t * stack = NULL;
    uint32_t v = 0;

    switch (pc_bytes[0]) {
        case 0x70: case 0x71: case 0x72: case 0x73: // various conditional branches
        case 0x74: case 0x75: case 0x76: case 0x77:
        case 0x78: case 0x79: case 0x7a: case 0x7b:
        case 0x7c: case 0x7d: case 0x7e: case 0x7f:
            pc += 2;
            if (x86_is_branch_taken (flags, pc_bytes[0])) {
                pc += (int8_t) pc_bytes[1];
                branch_taken (src, pc);
            } else {
                branch_not_taken (src);
            }
            break;
        case 0xeb: // JMP (short)
            pc += 2;
            pc += (int8_t) pc_bytes[1];
            branch_taken (src, pc);
            break;
        case 0xe4: // IN imm8, AL (reset counter)
            pc += 2;
            inst_count = 0;
            break;
        case 0xe5: // IN imm8, EAX (load and reset set counter)
            pc += 2;
            x86_other_context[REG_EAX] = (uint32_t) inst_count;
            inst_count = 0;
            break;
        case 0xe6: // OUT imm8, AL (special instruction; generate a marker)
        case 0xe7: // OUT imm8, EAX
            pc += 2;
            v = x86_other_context[REG_EAX];
            if (pc_bytes[0] == 0xe6) {
                v = v & 0xff;
            }
            if (pc_bytes[1] == 0x30) {
                // write to port 0x30 (cem_io_port)
                branch_trace_encode (CEM, v & WORD_DATA_MASK);
                if (!x86_quiet_mode) {
                    printf ("marker %u\n", (uint32_t) v);
                }
            }
            if (out_trace) {
                // fields:
                //  port number
                //  inst count
                //  value (AL or EAX)
                fprintf (out_trace, "%02x %08x %08x\n",
                     (uint32_t) pc_bytes[1],
                     (uint32_t) inst_count, v);
            }
            break;
        case 0x0f: // Two-byte instructions
            switch (pc_bytes[1]) {
                case 0x80: case 0x81: case 0x82: case 0x83: // various conditional branches
                case 0x84: case 0x85: case 0x86: case 0x87:
                case 0x88: case 0x89: case 0x8a: case 0x8b:
                case 0x8c: case 0x8d: case 0x8e: case 0x8f:
                    pc += 6;
                    if (x86_is_branch_taken (flags, pc_bytes[1])) {
                        offset = (uint32_t *) &pc_bytes[2];
                        pc += offset[0];
                        branch_taken (src, pc);
                    } else {
                        branch_not_taken (src);
                    }
                    break;
                case 0x31: // RDTSC
                    x86_other_context[REG_EAX] = inst_count;
                    x86_other_context[REG_EDX] = (uint32_t) (inst_count >> 32);
                    pc += 2;
                    break;
                default:
                    return 0;
            }
            break;

        case 0xe9: // JMP (long)
            pc += 5;
            offset = (uint32_t *) &pc_bytes[1];
            pc += offset[0];
            branch_taken (src, pc);
            break;

        case 0xe8: // CALL
            offset = (uint32_t *) &pc_bytes[1];
            pc += 5;
            stack = (uint32_t *) x86_other_context[REG_ESP];
            stack--;
            stack[0] = pc;
            x86_other_context[REG_ESP] = (intptr_t) stack;
            pc += offset[0];
            branch_taken (src, pc);
            break;

        case 0xf3: // REPZ
            if (pc_bytes[1] != 0xc3) {
                return 0; // not REPZ RET
            }
            // REPZ RET - fall through:
        case 0xc3: // RET
            stack = (uint32_t *) x86_other_context[REG_ESP];
            pc = stack[0];
            stack++;
            x86_other_context[REG_ESP] = (intptr_t) stack;
            branch_taken (src, pc);
            break;

        case 0xff:
            stack = (uint32_t *) x86_other_context[REG_ESP];
            stack--;
#ifdef DEBUG
            printf ("Code ff %02x at %08x, mode %u, R/M %u, opcode %u\n",
                pc_bytes[1], pc, pc_bytes[1] >> 6, pc_bytes[1] & 7, (pc_bytes[1] >> 3) & 7);
#endif

            // Decode R/M field: says which register is used for effective address
            switch (pc_bytes[1] & 7) {
                case 0: rm = x86_other_context[REG_EAX]; break;
                case 1: rm = x86_other_context[REG_ECX]; break;
                case 2: rm = x86_other_context[REG_EDX]; break;
                case 3: rm = x86_other_context[REG_EBX]; break;
                case 6: rm = x86_other_context[REG_ESI]; break;
                case 7: rm = x86_other_context[REG_EDI]; break;
                case 5: // EBP or disp32
                    if ((pc_bytes[1] >> 6) == 0) {
                        offset = (uint32_t *) &pc_bytes[2]; // 32-bit displacement
                        rm = offset[0];
                        pc += 4;
                    } else {
                        rm = x86_other_context[REG_EBP];
                    }
                    break;
                default: // SIB: don't decode that!
                    return 0;
            }

            // Decode the Mode field: says how register should be used
            switch (pc_bytes[1] >> 6) {
                case 0: // dereference with zero offset
                    offset = (uint32_t *) rm;
                    offset = (uint32_t *) offset[0];
                    rm = (uint32_t) offset;
                    pc += 2;
                    break;
                case 1: // dereference with short offset
                    rm += (int8_t) pc_bytes[2];
                    offset = (uint32_t *) rm;
                    offset = (uint32_t *) offset[0];
                    rm = (uint32_t) offset;
                    pc += 3;
                    break;
                case 2: // dereference with long offset
                    offset = (uint32_t *) &pc_bytes[2];
                    rm += offset[0];
                    offset = (uint32_t *) rm;
                    offset = (uint32_t *) offset[0];
                    rm = (uint32_t) offset;
                    pc += 6;
                    break;
                default: // direct
                    pc += 2;
                    break;
            }

            // Decode Opcode field
            switch ((pc_bytes[1] >> 3) & 7) {
                case 0: // INC
                    return 0;
                case 1: // DEC
                    return 0;
                case 2: // CALL
                    x86_other_context[REG_ESP] = (intptr_t) stack;
                    stack[0] = pc;
                    pc = rm;
                    branch_taken (src, pc);
                    break;
                case 3: // CALLF
                    return 0;
                case 4: // JMP
                    pc = rm;
                    branch_taken (src, pc);
                    break;
                case 5: // JMPF
                    return 0;
                case 6: // PUSH
                    return 0;
                default: // illegal?
                    return 0;
            }
            break;

        default:
            return 0;
    }
    x86_other_context[REG_EIP] = pc;
    return 1;
}

// interpreter loop
void x86_interpreter (void)
{
    uint32_t pc, pc_end;

    if (!x86_quiet_mode) {
        printf ("interpreter startup...\n");
    }

    // Startup: run until reaching the program
    pc = x86_other_context[REG_EIP];
#ifdef DEBUG
    printf ("stepping from EIP %08x\n", pc);
#endif
    entry_flag = 1;
    x86_other_context[REG_EFL] |= SINGLE_STEP_FLAG;
    x86_switch_to_user ((uint32_t) fake_endpoint);
    x86_other_context[REG_EFL] &= ~SINGLE_STEP_FLAG;
    pc = x86_other_context[REG_EIP];
    if ((pc < min_address) || (pc > max_address)) {
        printf ("Startup did not reach program (at %08x)\n", pc);
        exit (1);
    }

    if (!x86_quiet_mode) {
        printf ("interpreter ok, entry EIP %08x ESP %08x, program running:\n",
                    pc, x86_other_context[REG_ESP]);
    }

    // here is the main loop
    do {
        pc = x86_other_context[REG_EIP];

        if ((pc >= min_address) && (pc <= max_address)) {
            // We're in the program. Attempt to free run to end of superblock.
            // Find the end first (counting instructions as we go)
            superblock_info *si;

            si = &superblocks[pc - min_address];
            if (!si->count) {
               superblock_decoder (si, pc);
            }
            pc_end = pc + si->size;
            inst_count += si->count;

            // run the superblock
            if (pc_end != pc) {
#ifdef DEBUG
                printf ("RUNNING: Exec from %08x to %08x\n", pc, pc_end);
                dump_regs (x86_other_context);
#endif
                x86_switch_to_user (pc_end);
                pc = (uint32_t) x86_other_context[REG_EIP];
                if (pc_end != pc) {
                    fprintf (stderr, "Unexpected PC at end of superblock: %08x\n", pc);
                    exit (1);
                }
            }

            // at end of superblock, print out the contents of the superblock
            if (inst_trace || !x86_quiet_mode) {
               uint32_t address = pc_end - si->size;
               ZydisDecodedInstruction instruction;

               while (address <= pc_end) {
                  
                  char buffer[256];
                  if (!ZYDIS_SUCCESS (ZydisDecoderDecodeBuffer
                        (&decoder, (const char *) address, 32, address, &instruction))) {
                     strcpy (buffer, "(unknown)");
                  } else {
                     ZydisFormatterFormatInstruction
                       (&formatter, &instruction, buffer, sizeof(buffer));
                  }
                  if (!x86_quiet_mode) {
                     printf ("RUNNING: %08x: %s\n", address, buffer);
                  }
                  if (inst_trace) {
                     fprintf (inst_trace, "%08x: %s\n", address, buffer);
                  }
                  address += instruction.length;
               }
               if (inst_trace) {
                  fflush (inst_trace);
               }
            }

            // at end of superblock, attempt to interpret the control flow here
            if (!interpret_control_flow ()) {
                // Forced to single step
#ifdef DEBUG
                uint8_t * pc_bytes = (uint8_t *) pc;
                printf ("Non-interpretable code %02x %02x at %08x\n",
                    pc_bytes[0], pc_bytes[1], pc);
#endif
                entry_flag = 1;
                x86_other_context[REG_EFL] |= SINGLE_STEP_FLAG;
                x86_switch_to_user ((uint32_t) fake_endpoint);
                x86_other_context[REG_EFL] &= ~SINGLE_STEP_FLAG;

                branch_taken (pc_end, x86_other_context[REG_EIP]);
            }

        } else {
            // We're outside the program, free run until return
            pc_end = ((uint32_t *) x86_other_context[REG_ESP])[0];
            if ((pc_end < min_address) || (pc_end > max_address)) {
                if (!x86_quiet_mode) {
                    printf ("exit interpreter by return to %08x: stop interpretation\n", pc);
                }
                x86_switch_to_user ((uint32_t) fake_endpoint);
                exit (1);
            }
#ifdef DEBUG
            printf ("free run: IP %08x SP %08x end %08x\n",
                pc, x86_other_context[REG_ESP], pc_end);
#endif
            x86_switch_to_user (pc_end);
        }
    } while (1);
}

static void superblock_decoder (superblock_info * si, uint32_t pc)
{
   uint32_t address;
   char special = 'X';

   if (!x86_quiet_mode) {
      printf ("DECODE: new superblock: %08x\n", pc);
   }

   address = pc;
   while (address < max_address) {
      ZydisDecodedInstruction instruction;

      si->count ++;
      if (!ZYDIS_SUCCESS (ZydisDecoderDecodeBuffer
            (&decoder, (const char *) address, 32, address, &instruction))) {
         // invalid instruction? Assume end of superblock
         special = '?';
         break;
      }
      if (!x86_quiet_mode) {
         char buffer[256];
         ZydisFormatterFormatInstruction
           (&formatter, &instruction, buffer, sizeof(buffer));
         printf ("DECODE: %08x: %s\n", address, buffer);
      }
      special = '\0';
      switch (instruction.meta.category) {
         case ZYDIS_CATEGORY_COND_BR:
         case ZYDIS_CATEGORY_UNCOND_BR:
            special = 'J';
            break;
         case ZYDIS_CATEGORY_CALL:
            special = 'C';
            break;
         case ZYDIS_CATEGORY_RET:
            special = 'R';
            break;
         case ZYDIS_CATEGORY_SYSTEM:
            if (instruction.mnemonic == ZYDIS_MNEMONIC_RDTSC) {
               special = 't';
            }
            break;
         case ZYDIS_CATEGORY_IO:
            if ((instruction.mnemonic == ZYDIS_MNEMONIC_OUT)
            || (instruction.mnemonic == ZYDIS_MNEMONIC_IN)) {
               special = 't';
            }
            break;
      }
      if (special) {
         // End of superblock reached
         break;
      }
      address += instruction.length;
   }
   si->size = address - pc;
   if (!x86_quiet_mode) {
      printf ("DECODE: superblock %08x has %u instructions "
               "and ends at %08x with %c\n", pc, si->count, address, special);
      fflush (stdout);
   }
}

void x86_check_version (CommStruct * pcs)
{
   if (strcmp (pcs->internalVersionCheck, INTERNAL_VERSION) != 0)
   {
      printf ("pcs ivc = '%s' %d\n", pcs->internalVersionCheck, strlen (pcs->internalVersionCheck));
      printf ("internal= '%s' %d\n", INTERNAL_VERSION , strlen (INTERNAL_VERSION));
      fflush (stdout);
      x86_bp_trap (FAILED_VERSION_CHECK, NULL);
   }
}

// entry point
void x86_startup (size_t minPage, size_t maxPage, CommStruct * pcs)
{

   if (superblocks) {
      x86_bp_trap (FAILED_DOUBLE_LOAD, NULL);
   }
   x86_quiet_mode = !pcs->debugEnabled;
   min_address = minPage;
   max_address = maxPage;

   superblocks = calloc (sizeof (superblock_info), max_address + 1 - min_address);

   if (!superblocks) {
      x86_bp_trap (FAILED_MALLOC, NULL);
   }
   if (!x86_quiet_mode) {
      printf ("RUNNING: program address range: %08x .. %08x\n", min_address, max_address);
   }

   if (strlen (pcs->branchTrace)) {
      if (!x86_quiet_mode) {
         printf ("RUNNING: writing branch trace to: %s\n", pcs->branchTrace);
      }
      branch_trace = fopen (pcs->branchTrace, "wt");
      if (!branch_trace) {
         perror ("opening branch trace file");
         x86_bp_trap (FAILED_OPEN_BRANCH_TRACE, NULL);
      }
   }
   if (strlen (pcs->instTrace)) {
      if (!x86_quiet_mode) {
         printf ("RUNNING: writing instruction trace to: %s\n", pcs->instTrace);
      }
      inst_trace = fopen (pcs->instTrace, "wt");
      if (!inst_trace) {
         perror ("opening instruction trace file");
         x86_bp_trap (FAILED_OPEN_INST_TRACE, NULL);
      }
   }
   if (strlen (pcs->outTrace)) {
      if (!x86_quiet_mode) {
         printf ("RUNNING: writing out trace to: %s\n", pcs->outTrace);
      }
      out_trace = fopen (pcs->outTrace, "wt");
      if (!out_trace) {
         perror ("opening out trace file");
         x86_bp_trap (FAILED_OPEN_OUT_TRACE, NULL);
      }
   }
   if (!x86_quiet_mode) {
      fflush (stdout);
   }

   ZydisDecoderInit (&decoder, ZYDIS_MACHINE_MODE_LEGACY_32, ZYDIS_ADDRESS_WIDTH_32);
   if (!x86_quiet_mode) {
      ZydisFormatterInit(&formatter, ZYDIS_FORMATTER_STYLE_INTEL);
   }

   //x86_make_text_writable (min_address, max_address);
   entry_flag = 1;

   // save this context and launch the interpreter
   x86_begin_single_step ();

   // we return to here in user mode only
}


