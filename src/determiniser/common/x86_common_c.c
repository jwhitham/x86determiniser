//#define DEBUG
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

#include <Zydis/Zydis.h>

#include "offsets.h"
#include "x86_flags.h"
#include "common.h"
#include "x86_common.h"

#define TRIGGER_LEVEL   0x1000000
#define AFTER_FLAG      0x80000000U
#define MARKER_FLAG     0x40000000U
#define PC_MASK         0x3fffffffU

static uint8_t          entry_flag = 1;
uint8_t                 x86_quiet_mode = 0;

static uint8_t          fake_endpoint[8];
static uint32_t         min_address = 0;
static uint32_t         max_address = 0;

static FILE *           branch_trace = NULL;
static FILE *           out_trace = NULL;

static uint64_t         inst_count = 0;
static ZydisDecoder     decoder;
static ZydisFormatter   formatter;


extern uint32_t x86_other_context[];

void x86_switch_to_user (uint32_t endpoint);
void x86_make_text_writable (uint32_t min_address, uint32_t max_address);
void x86_begin_single_step (void);

extern uint8_t x86_switch_from_user[];
extern uint32_t x86_size_of_indirect_call_instruction;

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

typedef struct superblock_info {
    uint32_t size;
    uint32_t count;
} superblock_info;

static superblock_info * superblocks;

static void superblock_decoder (superblock_info * si, uint32_t pc);

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
    printf ("RUNNING: stepping EIP %08x ESP %08x entry_flag %u\n", pc, gregs[REG_ESP], entry_flag);
#endif

    if (!entry_flag) {
        // We have now stepped one instruction in the program, time to leave!
        // Fake an indirect call (to match the other way to exit from user code)
        gregs[REG_ESP] -= 4;
        ((uint32_t *) gregs[REG_ESP])[0] = gregs[REG_EIP] + x86_size_of_indirect_call_instruction; 
        gregs[REG_EIP] = (uint32_t) x86_switch_from_user;
#ifdef DEBUG
        printf ("RUNNING: switch from user: new EIP %08x ESP %08x\n", gregs[REG_EIP], gregs[REG_ESP]);
#endif
        gregs[REG_EFL] &= ~FLAG_TF;
    } else if ((pc < min_address) || (pc > max_address)) {
        // Still outside program (probably completing the switch from super -> user)
        // Keep stepping
        gregs[REG_EFL] |= FLAG_TF;
    } else {
        // Stepped one instruction
        entry_flag = 0;
        gregs[REG_EFL] |= FLAG_TF;
    }
}

static int interpret_control_flow (void)
{
    uint32_t pc = x86_other_context[REG_EIP];
    uint8_t * pc_bytes = (uint8_t *) pc;
    uint32_t flags = x86_other_context[REG_EFL];
    uint32_t branch = 0;
    uint32_t rm = 0;
    uint32_t * offset = NULL;
    uint32_t * stack = NULL;

    switch (pc_bytes[0]) {
        // opcodes 0x70 - 0x7f decoding based on i8086emu - see README.md
        case 0x70: //JO
            branch = (flags & FLAG_OF);
            goto short_relative_branch;
        case 0x71: //JNO
            branch = ( !(flags & FLAG_OF) );
            goto short_relative_branch;
        case 0x72: //JB/JNAE
            branch = (flags & FLAG_CF);
            goto short_relative_branch;
        case 0x73: //JNB/JAE
            branch = (!(flags & FLAG_CF));
            goto short_relative_branch;
        case 0x74: //JE/JZ
            branch = (flags & FLAG_ZF);
            goto short_relative_branch;
        case 0x75: //JNE/JNZ
            branch = (!(flags & FLAG_ZF));
            goto short_relative_branch;
        case 0x76: //JBE/JNA
            branch = ((flags & FLAG_CF) || (flags & FLAG_ZF));
            goto short_relative_branch;
        case 0x77: //JNBE/JA
            branch = ( (!(flags & FLAG_CF)) && (!(flags & FLAG_ZF)) );
            goto short_relative_branch;
        case 0x78: //JS
            branch = (flags & FLAG_SF);
            goto short_relative_branch;
        case 0x79: //JNS
            branch = ( !(flags & FLAG_SF) );
            goto short_relative_branch;
        case 0x7a: //JP/JPE
            branch = (flags & FLAG_PF);
            goto short_relative_branch;
        case 0x7b: //JNP/JPO
            branch = ( !(flags & FLAG_PF) );
            goto short_relative_branch;
        case 0x7c: //JL/JNGE
            branch = (((flags & FLAG_SF) << 4) != (flags & FLAG_OF));
            goto short_relative_branch;
        case 0x7d: //JNL/JGE
            branch = (((flags & FLAG_SF) << 4) == (flags & FLAG_OF));
            goto short_relative_branch;
        case 0x7e: //JLE/JNG
            branch = ((flags & FLAG_ZF) || (((flags & FLAG_SF) << 4) != (flags & FLAG_OF)));
            goto short_relative_branch;
        case 0x7f: //JNLE/JG
            branch = ( (!(flags & FLAG_ZF)) && (((flags & FLAG_SF) << 4) == (flags & FLAG_OF)) );
            goto short_relative_branch;
        case 0xeb: // JMP (short)
            branch = 1;
        short_relative_branch:
            pc += 2;
            if (branch) {
                pc += (int8_t) pc_bytes[1];
            }
            break;
        case 0xe4: // IN imm8, AL (special instruction; reset counter)
            pc += 2;
            inst_count = 0;
            break;
        case 0xe5: // IN imm8, EAX (special instruction; read and reset counter)
            pc += 2;
            x86_other_context[REG_EAX] = inst_count;
            inst_count = 0;
            break;
        case 0xe6: // OUT imm8, AL (special instruction; generate a marker)
        case 0xe7: // OUT imm8, EAX
            pc += 2;
            if (branch_trace) {
               fprintf (branch_trace, "%08x %08x\n",
                     MARKER_FLAG | (uint32_t) pc_bytes[1],
                     (uint32_t) inst_count);
            }
            if (out_trace) {
               // fields:
               //  port number
               //  inst count
               //  value (AL or EAX)
               fprintf (out_trace, "%02x %08x %08x\n",
                     (uint32_t) pc_bytes[1],
                     (uint32_t) inst_count,
                     ((pc_bytes[0] == 0xe6) ?
                        (x86_other_context[REG_EAX] & 0xff) :
                        x86_other_context[REG_EAX]));
            }
            if (!x86_quiet_mode) {
               printf ("MARKER: %u\n", (uint32_t) pc_bytes[1]);
               fflush (stdout);
            }
            break;
        case 0x0f: // Two-byte instructions
            switch (pc_bytes[1]) {
                // opcodes 0x80 - 0x8f decoding based on i8086emu - see README.md
                case 0x80: //JO
                    branch = (flags & FLAG_OF);
                    goto long_relative_branch;
                case 0x81: //JNO
                    branch = ( !(flags & FLAG_OF) );
                    goto long_relative_branch;
                case 0x82: //JB/JNAE
                    branch = (flags & FLAG_CF);
                    goto long_relative_branch;
                case 0x83: //JNB/JAE
                    branch = (!(flags & FLAG_CF));
                    goto long_relative_branch;
                case 0x84: //JE/JZ
                    branch = (flags & FLAG_ZF);
                    goto long_relative_branch;
                case 0x85: //JNE/JNZ
                    branch = (!(flags & FLAG_ZF));
                    goto long_relative_branch;
                case 0x86: //JBE/JNA
                    branch = ((flags & FLAG_CF) || (flags & FLAG_ZF));
                    goto long_relative_branch;
                case 0x87: //JNBE/JA
                    branch = ( (!(flags & FLAG_CF)) && (!(flags & FLAG_ZF)) );
                    goto long_relative_branch;
                case 0x88: //JS
                    branch = (flags & FLAG_SF);
                    goto long_relative_branch;
                case 0x89: //JNS
                    branch = ( !(flags & FLAG_SF) );
                    goto long_relative_branch;
                case 0x8a: //JP/JPE
                    branch = (flags & FLAG_PF);
                    goto long_relative_branch;
                case 0x8b: //JNP/JPO
                    branch = ( !(flags & FLAG_PF) );
                    goto long_relative_branch;
                case 0x8c: //JL/JNGE
                    branch = (((flags & FLAG_SF) << 4) != (flags & FLAG_OF));
                    goto long_relative_branch;
                case 0x8d: //JNL/JGE
                    branch = (((flags & FLAG_SF) << 4) == (flags & FLAG_OF));
                    goto long_relative_branch;
                case 0x8e: //JLE/JNG
                    branch = ((flags & FLAG_ZF) || (((flags & FLAG_SF) << 4) != (flags & FLAG_OF)));
                    goto long_relative_branch;
                case 0x8f: //JNLE/JG
                    branch = ( (!(flags & FLAG_ZF)) && (((flags & FLAG_SF) << 4) == (flags & FLAG_OF)) );
                long_relative_branch:
                    pc += 6;
                    if (branch) {
                        offset = (uint32_t *) &pc_bytes[2];
                        pc += offset[0];
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
            break;

        case 0xe8: // CALL
            offset = (uint32_t *) &pc_bytes[1];
            pc += 5;
            stack = (uint32_t *) x86_other_context[REG_ESP];
            stack--;
            stack[0] = pc;
            x86_other_context[REG_ESP] = (intptr_t) stack;
            pc += offset[0];
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
            break;

        case 0xff:
            stack = (uint32_t *) x86_other_context[REG_ESP];
            stack--;
#ifdef DEBUG
            printf ("RUNNING: Code ff %02x at %08x, mode %u, R/M %u, opcode %u\n",
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
                    break;
                case 3: // CALLF
                    return 0;
                case 4: // JMP
                    pc = rm;
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
        printf ("RUNNING: interpreter startup...\n");
    }

    // Startup: run until reaching the program
    pc = x86_other_context[REG_EIP];
#ifdef DEBUG
    printf ("stepping from EIP %08x\n", pc);
#endif
    entry_flag = 1;
    x86_other_context[REG_EFL] |= FLAG_TF;
    x86_switch_to_user ((uint32_t) fake_endpoint);
    x86_other_context[REG_EFL] &= ~FLAG_TF;
    pc = x86_other_context[REG_EIP];
    if ((pc < min_address) || (pc > max_address)) {
        printf ("RUNNING: Startup did not reach program (at %08x)\n", pc);
        exit (1);
    }

    if (!x86_quiet_mode) {
        printf ("RUNNING: interpreter ok, entry EIP %08x ESP %08x, program running:\n",
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

            // at end of superblock, attempt to interpret the control flow here
            if (!interpret_control_flow ()) {
                // Forced to single step
#ifdef DEBUG
                uint8_t * pc_bytes = (uint8_t *) pc;
                printf ("RUNNING: Non-interpretable code %02x %02x at %08x\n",
                    pc_bytes[0], pc_bytes[1], pc);
#endif
                entry_flag = 1;
                x86_other_context[REG_EFL] |= FLAG_TF;
                x86_switch_to_user ((uint32_t) fake_endpoint);
                x86_other_context[REG_EFL] &= ~FLAG_TF;
            }

            // branch taken
            if (branch_trace
            && (pc_end >= min_address)
            && (pc_end <= max_address)
            && ((uint32_t) x86_other_context[REG_EIP] >= min_address)
            && ((uint32_t) x86_other_context[REG_EIP] <= max_address)) {
               fprintf (branch_trace, "%08x %08x\n",     // before
                     PC_MASK & pc_end,
                     (uint32_t) inst_count);
               fprintf (branch_trace, "%08x %08x\n",     // after
                     AFTER_FLAG | (PC_MASK & (uint32_t) x86_other_context[REG_EIP]),
                     (uint32_t) inst_count);
            }

        } else {
            // We're outside the program, free run until return
            pc_end = ((uint32_t *) x86_other_context[REG_ESP])[0];
            if ((pc_end < min_address) || (pc_end > max_address)) {
                if (!x86_quiet_mode) {
                    printf ("RUNNING: exit interpreter by return to %08x: stop interpretation\n", pc);
                }
                x86_switch_to_user ((uint32_t) fake_endpoint);
                exit (1);
            }
#ifdef DEBUG
            printf ("RUNNING: free run: IP %08x SP %08x end %08x\n",
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


