#define DEBUG
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

// branch trace encoding data (see doc/branch_trace_format.pdf)
#define WORD_DATA_MASK  0x0fffffffU
#define CEM             0x00000000U
#define BNT             0x10000000U
#define SA              0x20000000U
#define BT              0x30000000U
#define SM              0x40000000U

#define BRANCH_TRACE_REFRESH_INTERVAL 10000


static uint8_t          entry_flag = 1;
uint8_t                 x86_quiet_mode = 0;

static uint8_t          fake_endpoint[16];
static uintptr_t        min_address = 0;
static uintptr_t        max_address = 0;

static uint32_t         branch_trace_temp = 0;
static uint32_t         branch_trace_refresh = BRANCH_TRACE_REFRESH_INTERVAL;
static FILE *           branch_trace = NULL;
static FILE *           out_trace = NULL;
static FILE *           inst_trace = NULL;

static uint64_t         inst_count = 0;
static ZydisDecoder     decoder;
static ZydisFormatter   formatter;

typedef struct superblock_info {
    size_t size;
    size_t count;
} superblock_info;

static superblock_info * superblocks;

extern uintptr_t x86_other_context[];
extern uint8_t x86_switch_from_user[];
extern uint32_t x86_size_of_call_instruction;

void x86_switch_to_user (uintptr_t endpoint);
void x86_make_text_writable (uintptr_t min_address, uintptr_t max_address);
void x86_begin_single_step (void);
int x86_is_branch_taken (uintptr_t flags, uint8_t opcode);

static uint32_t * get_flags_ptr (uintptr_t * gregs)
{
   uint8_t * gregs_start = (uint8_t *) gregs;
   uint8_t * eflags_start = gregs_start + OFF_EFL;
   return (uint32_t *) eflags_start;
}

static uint32_t get_flags (uintptr_t * gregs)
{
   return get_flags_ptr (gregs)[0];
}

static void set_single_step_flag (uintptr_t * gregs)
{
   get_flags_ptr (gregs)[0] |= SINGLE_STEP_FLAG;
}

static void clear_single_step_flag (uintptr_t * gregs)
{
   get_flags_ptr (gregs)[0] &= ~SINGLE_STEP_FLAG;
}

static void dump_regs (uintptr_t * gregs)
{
   char c = REGISTER_PREFIX;
   printf (" %cAX = %p  ", c, (void *) gregs[REG_XAX]);
   printf (" %cBX = %p  ", c, (void *) gregs[REG_XBX]);
   printf (" %cCX = %p  ", c, (void *) gregs[REG_XCX]);
   printf (" %cDX = %p  ", c, (void *) gregs[REG_XDX]);
   printf (" EFL = %x\n", get_flags (gregs));
   printf (" %cIP = %p  ", c, (void *) gregs[REG_XIP]);
   printf (" %cSI = %p  ", c, (void *) gregs[REG_XSI]);
   printf (" %cDI = %p  ", c, (void *) gregs[REG_XDI]);
   printf (" %cBP = %p  ", c, (void *) gregs[REG_XBP]);
   printf (" %cSP = %p\n", c, (void *) gregs[REG_XSP]);
}

static void superblock_decoder (superblock_info * si, uintptr_t pc);

// encode some element into the branch trace, with correct handling
// for values greater than 1 << 28
static void branch_trace_encode (uint32_t trace_opcode, uintptr_t addr)
{
   if (branch_trace) {
      uint32_t top_nibble = addr >> 28;

      if ((top_nibble != branch_trace_temp)
      || (branch_trace_refresh == 0)) {
         /* set the value of the top nibble used in the branch trace using
          * the SM trace opcode */
         branch_trace_temp = top_nibble;
         branch_trace_refresh = BRANCH_TRACE_REFRESH_INTERVAL;
         fprintf (branch_trace, "%08x %08x\n",
            SM | top_nibble, (uint32_t) inst_count);
      }
      /* write an element to the branch trace */
      fprintf (branch_trace, "%08x %08x\n",
          (uint32_t) (trace_opcode | (addr & WORD_DATA_MASK)),
          (uint32_t) inst_count);
      branch_trace_refresh --;
#ifdef DEBUG
      fflush (branch_trace);
#endif
   }
}

// write taken branch to branch trace
static inline void branch_taken (uintptr_t src, uintptr_t dest)
{
   if (branch_trace
   && (src >= min_address) && (src <= max_address)
   && (dest >= min_address) && (dest <= max_address)) {
       branch_trace_encode (SA, src);
       branch_trace_encode (BT, dest);
   }
}

// write not taken branch to branch trace
static inline void branch_not_taken (uintptr_t src)
{
   if (branch_trace
   && (src >= min_address) && (src <= max_address)) {
       branch_trace_encode (BNT, src);
   }
}

// single step print_trigger_counted
void x86_trap_handler (uintptr_t * gregs, uint32_t trapno)
{
   uintptr_t pc;

   if (trapno != 1) {
      fprintf (stderr, "trapno %u not known\n", trapno);
      dump_regs (gregs);
      x86_bp_trap (FAILED_BAD_TRAP_NUMBER, NULL);
      return;
   }
        
   pc = gregs[REG_XIP];
#ifdef DEBUG
   if (!x86_quiet_mode) {
      printf ("X86D: stepping %cIP %p %cSP %p entry_flag %u context %p\n",
            REGISTER_PREFIX, (void *) gregs[REG_XIP],
            REGISTER_PREFIX, (void *) gregs[REG_XSP], entry_flag,
            (void *) gregs);
   }
#endif

   if (!entry_flag) {
      // We have now stepped one instruction in the program, time to leave!
      // Fake a call (to match the other way to exit from user code)
      uintptr_t sp = (uintptr_t) gregs[REG_XSP];
      uintptr_t * tos;
       
      sp -= PTR_SIZE;
      gregs[REG_XSP] = sp;
      tos = (uintptr_t *) sp;
      tos[0] = pc + x86_size_of_call_instruction;
      gregs[REG_XIP] = (uintptr_t) x86_switch_from_user;

#ifdef DEBUG
      if (!x86_quiet_mode) {
         printf ("X86D: switch from user: new %cIP %p %cSP %p\n",
               REGISTER_PREFIX, (void *) gregs[REG_XIP],
               REGISTER_PREFIX, (void *) gregs[REG_XSP]);
      }
#endif
      clear_single_step_flag (gregs);
   } else if ((pc < min_address) || (pc > max_address)) {
      // Still outside program (probably completing the switch from super -> user)
      // Keep stepping
      set_single_step_flag (gregs);
      if (!x86_quiet_mode) {
         printf ("X86D: keep stepping: %x\n", get_flags (gregs));
      }
   } else {
      // Stepped one instruction
      entry_flag = 0;
      set_single_step_flag (gregs);
      if (!x86_quiet_mode) {
         printf ("X86D: initial step: %x\n", get_flags (gregs));
      }
   }
}

static ptrdiff_t get_disp32 (uint8_t * imm_value_ptr)
{
   return (ptrdiff_t) (((int32_t *) imm_value_ptr)[0]);
}

static int interpret_control_flow (void)
{
   uintptr_t pc = x86_other_context[REG_XIP];
   uintptr_t src = pc;
   uint8_t * pc_bytes = (uint8_t *) pc;
   uint32_t flags = get_flags (x86_other_context);
   ptrdiff_t rm = 0;
   uintptr_t * stack = NULL;
   uintptr_t v = 0;

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
      case 0xe5: // IN imm8, EAX (get counter value and then reset counter)
         pc += 2;
         x86_other_context[REG_XAX] = (uint32_t) inst_count;
         inst_count = 0;
         break;
      case 0xe6: // OUT imm8, AL (special instruction; generate a marker)
      case 0xe7: // OUT imm8, EAX
         pc += 2;
         v = x86_other_context[REG_XAX];
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
                (uint32_t) inst_count, (uint32_t) v);
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
                  pc += get_disp32 (&pc_bytes[2]);
                  branch_taken (src, pc);
               } else {
                  branch_not_taken (src);
               }
               break;
            case 0x31: // RDTSC
               x86_other_context[REG_XAX] = inst_count;
               x86_other_context[REG_XDX] = (uint32_t) (inst_count >> 32);
               pc += 2;
               break;
            default:
               return 0;
         }
         break;

      case 0xe9: // JMP (long)
         pc += 5;
         pc += get_disp32 (&pc_bytes[1]);
         branch_taken (src, pc);
         break;

      case 0xe8: // CALL
         pc += 5;
         stack = (uintptr_t *) x86_other_context[REG_XSP];
         stack--;
         stack[0] = pc;
         x86_other_context[REG_XSP] = (uintptr_t) stack;
         pc += get_disp32 (&pc_bytes[1]);
         branch_taken (src, pc);
         break;

      case 0xf3: // REPZ
         if (pc_bytes[1] != 0xc3) {
            return 0; // not REPZ RET
         }
         // REPZ RET - fall through:
      case 0xc3: // RET
         stack = (uintptr_t *) x86_other_context[REG_XSP];
         pc = stack[0];
         stack++;
         x86_other_context[REG_XSP] = (uintptr_t) stack;
         branch_taken (src, pc);
         break;

      case 0xff:
#ifdef DEBUG
         if (!x86_quiet_mode) {
            printf ("X86D: Code ff %02x at %p, mode %u, R/M %u, opcode %u\n",
               pc_bytes[1], pc_bytes, pc_bytes[1] >> 6, pc_bytes[1] & 7, (pc_bytes[1] >> 3) & 7);
            fflush (stdout);
         }
#endif

         // Decode R/M field: says which register is used for effective address
         switch (pc_bytes[1] & 7) {
            case 0: rm = x86_other_context[REG_XAX]; break;
            case 1: rm = x86_other_context[REG_XCX]; break;
            case 2: rm = x86_other_context[REG_XDX]; break;
            case 3: rm = x86_other_context[REG_XBX]; break;
            case 6: rm = x86_other_context[REG_XSI]; break;
            case 7: rm = x86_other_context[REG_XDI]; break;
            case 5:
               if ((pc_bytes[1] >> 6) == 0) {
                  // disp32
                  rm = get_disp32 (&pc_bytes[2]);
                  pc += 4;
               } else {
                  // XBP
                  rm = x86_other_context[REG_XBP];
               }
               break;
            default: // SIB: don't decode that!
               return 0;
         }

#ifdef DEBUG
         if (!x86_quiet_mode) {
            printf ("X86D: R/M value: (before mode) %p ", (void *) rm);
            fflush (stdout);
         }
#endif

         // Decode the Mode field: says how register should be used
         switch (pc_bytes[1] >> 6) {
            case 0:
               if ((PTR_SIZE == 8) && ((pc_bytes[1] & 7) == 5)) {
                  // dereference with PC offset in 64-bit mode
                  pc += 2; // relative to PC at end of instruction
                  rm += pc;
#ifdef DEBUG
                  if (!x86_quiet_mode) {
                     printf (" (before deref) %p ", (void *) rm);
                     fflush (stdout);
                  }
#endif
                  rm = ((uintptr_t *) rm)[0];
               } else {
                  // dereference with zero offset
                  rm = ((uintptr_t *) rm)[0];
                  pc += 2;
               }
               break;
            case 1: // dereference with short offset
               rm += (int8_t) pc_bytes[2];
               rm = ((uintptr_t *) rm)[0];
               pc += 3;
               break;
            case 2: // dereference with long offset
               rm += get_disp32 (&pc_bytes[2]);
               rm = ((uintptr_t *) rm)[0];
               pc += 6;
               break;
            default: // direct
               pc += 2;
               break;
         }

#ifdef DEBUG
         if (!x86_quiet_mode) {
            printf ("(after mode) %p\n", (void *) rm);
            fflush (stdout);
         }
#endif

         // Decode Opcode field
         switch ((pc_bytes[1] >> 3) & 7) {
            case 0: // INC
               return 0;
            case 1: // DEC
               return 0;
            case 2: // CALL
               stack = (uintptr_t *) x86_other_context[REG_XSP];
               stack--;
               x86_other_context[REG_XSP] = (uintptr_t) stack;
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
   x86_other_context[REG_XIP] = pc;
   return 1;
}

// interpreter loop
void x86_interpreter (void)
{
    uintptr_t pc, pc_end;

    if (!x86_quiet_mode) {
        printf ("X86D: interpreter startup... pc = %p\n", (void *) x86_other_context[REG_XIP]);
    }

    // Startup: run until reaching the program
    pc = x86_other_context[REG_XIP];
    entry_flag = 1;
    set_single_step_flag (x86_other_context);
    x86_switch_to_user ((uintptr_t) fake_endpoint);
    clear_single_step_flag (x86_other_context);
    pc = x86_other_context[REG_XIP];
    if ((pc < min_address) || (pc > max_address)) {
        fprintf (stderr, "Startup did not reach program (at %p)\n", (void *) pc);
        fflush (stdout);
        fflush (stderr);
        x86_bp_trap (FAILED_TO_REACH_PROGRAM, NULL);
    }

    if (!x86_quiet_mode) {
        printf ("X86D: interpreter ok, entry %cIP %p %cSP %p, program running:\n",
                    REGISTER_PREFIX, (void *) pc,
                    REGISTER_PREFIX, (void *) x86_other_context[REG_XSP]);
    }

    // here is the main loop
    do {
        pc = x86_other_context[REG_XIP];

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

#ifdef DEBUG
             if (!x86_quiet_mode) {
                void ** address = (void **) 0x408178;
                printf ("X86D: probe %p = %p\n", (void *) address, (void *) address[0]);
                fflush (stdout);
             }
#endif

            // run the superblock
            if (pc_end != pc) {
#ifdef DEBUG
                if (!x86_quiet_mode) {
                   printf ("X86D: Exec from %p to %p\n", (void *) pc, (void *) pc_end);
                   dump_regs (x86_other_context);
                   fflush (stdout);
                }
#endif
                x86_switch_to_user (pc_end);
                pc = (uintptr_t) x86_other_context[REG_XIP];
                if (pc_end != pc) {
                    fprintf (stderr, "Unexpected PC at end of superblock: %p\n",
                        (void *) pc);
                    x86_bp_trap (FAILED_SUPERBLOCK_DECODE_ERR, NULL);
                }
            }

            // at end of superblock, print out the contents of the superblock
            if (inst_trace || !x86_quiet_mode) {
               uintptr_t address = pc_end - si->size;
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
                     printf ("X86D: %p: %s\n", (void *) address, buffer);
                  }
                  if (inst_trace) {
#ifdef IS_64_BIT
                     fprintf (inst_trace, "%08x", (uint32_t) ((uint64_t) address >> (uint64_t) 32));
#endif
                     fprintf (inst_trace, "%08x: %s\n", (uint32_t) address, buffer);
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
                if (!x86_quiet_mode) {
                   uint8_t * pc_bytes = (uint8_t *) pc;
                   printf ("Non-interpretable code %02x %02x at %p\n",
                       pc_bytes[0], pc_bytes[1], pc_bytes);
                }
#endif
                entry_flag = 1;
                set_single_step_flag (x86_other_context);
                x86_switch_to_user ((uintptr_t) fake_endpoint);
                clear_single_step_flag (x86_other_context);

                branch_taken (pc_end, x86_other_context[REG_XIP]);
            }

        } else {
            // We're outside the program, free run until return
            pc_end = ((uintptr_t *) x86_other_context[REG_XSP])[0];
            if ((pc_end < min_address) || (pc_end > max_address)) {
                if (!x86_quiet_mode) {
                    printf ("exit interpreter by return to %p: stop interpretation\n", (void *) pc);
                }
                x86_switch_to_user ((uintptr_t) fake_endpoint);
                exit (1);
            }
#ifdef DEBUG
            if (!x86_quiet_mode) {
               printf ("X86D: free run: %cIP %p %cSP %p end %p\n",
                   REGISTER_PREFIX, (void *) pc,
                   REGISTER_PREFIX, (void *) x86_other_context[REG_XSP], (void *) pc_end);
            }
#endif
            x86_switch_to_user (pc_end);
        }
    } while (1);
}

static void superblock_decoder (superblock_info * si, uintptr_t pc)
{
   uintptr_t address;
   char special = 'X';

   if (!x86_quiet_mode) {
      printf ("DECODE: new superblock: %p\n", (void *) pc);
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
         case ZYDIS_CATEGORY_X87_ALU:
            if (instruction.mnemonic == ZYDIS_MNEMONIC_FWAIT) {
               si->count --; // fwait does not count
               // (this matches objdump which will combine it into the following
               // floating point instruction)
            }
            break;
         case ZYDIS_CATEGORY_MISC:     // UD2
            if ((instruction.mnemonic == ZYDIS_MNEMONIC_UD0)
            || (instruction.mnemonic == ZYDIS_MNEMONIC_UD1)
            || (instruction.mnemonic == ZYDIS_MNEMONIC_UD2)) {
               special = 'u';
            }
            break;
         case ZYDIS_CATEGORY_INVALID:
         case ZYDIS_CATEGORY_INTERRUPT:
            special = 'i';
            break;
         case ZYDIS_CATEGORY_IO:       // IN, OUT
         case ZYDIS_CATEGORY_SYSTEM:   // RDTSC
            special = 't';
            break;
      }
      if (!x86_quiet_mode) {
         char buffer[256];
         ZydisFormatterFormatInstruction
           (&formatter, &instruction, buffer, sizeof(buffer));
         printf ("DECODE: %p: %s\n", (void *) address, buffer);
      }
      if (special) {
         // End of superblock reached
         break;
      }
      address += instruction.length;
   }
   si->size = address - pc;
   if (!x86_quiet_mode) {
      printf ("DECODE: superblock %p has %u instructions "
               "and ends at %p with %c\n",
                  (void *) pc, (unsigned) si->count, (void *) address, special);
      fflush (stdout);
   }
}

void x86_check_version (CommStruct * pcs)
{
   if (strcmp (pcs->internalVersionCheck, INTERNAL_VERSION) != 0)
   {
      printf ("pcs ivc = '%s' %d\n", pcs->internalVersionCheck, (int) strlen (pcs->internalVersionCheck));
      printf ("internal= '%s' %d\n", INTERNAL_VERSION , (int) strlen (INTERNAL_VERSION));
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
      printf ("X86D: program address range: %p .. %p\n",
         (void *) min_address, (void *) max_address);
   }

   if (strlen (pcs->branchTrace)) {
      if (!x86_quiet_mode) {
         printf ("X86D: writing branch trace to: %s\n", pcs->branchTrace);
      }
      branch_trace = fopen (pcs->branchTrace, "wt");
      if (!branch_trace) {
         x86_bp_trap (FAILED_OPEN_BRANCH_TRACE, NULL);
      }
   }
   if (strlen (pcs->instTrace)) {
      if (!x86_quiet_mode) {
         printf ("X86D: writing instruction trace to: %s\n", pcs->instTrace);
      }
      inst_trace = fopen (pcs->instTrace, "wt");
      if (!inst_trace) {
         x86_bp_trap (FAILED_OPEN_INST_TRACE, NULL);
      }
   }
   if (strlen (pcs->outTrace)) {
      if (!x86_quiet_mode) {
         printf ("X86D: writing out trace to: %s\n", pcs->outTrace);
      }
      out_trace = fopen (pcs->outTrace, "wt");
      if (!out_trace) {
         x86_bp_trap (FAILED_OPEN_OUT_TRACE, NULL);
      }
   }
   if (!x86_quiet_mode) {
      fflush (stdout);
   }

#ifdef IS_64_BIT
   ZydisDecoderInit (&decoder, ZYDIS_MACHINE_MODE_LONG_64, ZYDIS_ADDRESS_WIDTH_64);
#else
   ZydisDecoderInit (&decoder, ZYDIS_MACHINE_MODE_LEGACY_32, ZYDIS_ADDRESS_WIDTH_32);
#endif
   ZydisFormatterInit(&formatter, ZYDIS_FORMATTER_STYLE_INTEL);

   //x86_make_text_writable (min_address, max_address);
   entry_flag = 1;

   // save this context and launch the interpreter
   x86_begin_single_step ();

   // we return to here in user mode only
}


