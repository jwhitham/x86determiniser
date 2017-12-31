//#define DEBUG
#include <config.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>

#include <dis-asm.h>

#ifdef LINUX32
#include "linux32_offsets.h"
#elif WIN32
#include "win32_offsets.h"
#endif

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

static uint64_t         inst_count = 0;

/* These definitions are from i8086emu - see README.md */
#define FLAG_CF  1    /* Carry-Flag */
#define FLAG_PF  4    /* Parity-Flag */
#define FLAG_ACF 16   /* Auxillary-Carry-Flag */
#define FLAG_ZF  64   /* Zero-Flag */
#define FLAG_SF  128  /* Sign-Flag */
#define FLAG_TF  256  /* Trap-Flag */
#define FLAG_IF  512  /* Interrupt-Flag */
#define FLAG_DF  1024 /* Direction-Flag */
#define FLAG_OF  2048 /* Overflow-Flag */

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
        case 0xe6: // OUT imm8, AL (special instruction; generate a marker)
        case 0xe7: // OUT imm8, EAX
            pc += 2;
            if (branch_trace) {
               fprintf (branch_trace, "%08x %08x\n",
                     MARKER_FLAG | (uint32_t) pc_bytes[1],
                     (uint32_t) inst_count);
            }
            printf ("marker %u\n", (uint32_t) pc_bytes[1]);
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
        printf ("interpreter startup...\n");
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
                printf ("Exec from %08x to %08x\n", pc, pc_end);
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
                printf ("Non-interpretable code %02x %02x at %08x\n",
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

static int startswith (const char * line, const char * search)
{ 
    unsigned i;

    for (i = 0; (line[i] == search[i]) && (line[i] != '\0'); i++) {}

    if (line[i] == search[i]) {
        return 1; // both strings are the same
    } else if (search[i] == '\0') {
        return 1; // line starts with search
    } else {
        return 0; // line does not start with search
    }
}

int print_insn_i386 (bfd_vma pc, disassemble_info *info);

//enum bfd_architecture bfd_get_arch (bfd * x) { (void) x; return 0; }
//unsigned long bfd_get_mach (bfd * x) { (void) x; return 0; }

/* Function called to check if a SYMBOL is can be displayed to the user.
 This is used by some ports that want to hide special symbols when
 displaying debugging outout.  */
static bfd_boolean symbol_is_valid (asymbol *s, struct disassemble_info *dinfo)
{
    (void) s;
    (void) dinfo;
    return FALSE;
}

/* Function called to determine if there is a symbol at the given ADDR.
 If there is, the function returns 1, otherwise it returns 0.
 This is used by ports which support an overlay manager where
 the overlay number is held in the top part of an address.  In
 some circumstances we want to include the overlay number in the
 address, (normally because there is a symbol associated with
 that address), but sometimes we want to mask out the overlay bits.  */
static int symbol_at_address_func (bfd_vma addr, struct disassemble_info *dinfo)
{
    (void) addr;
    (void) dinfo;
    return 0;
}

/* Function called to print ADDR.  */
static void print_address_func (bfd_vma addr, struct disassemble_info *dinfo)
{
    dinfo->fprintf_func (dinfo->stream, "%p", (void *) addr);
}

/* Function which should be called if we get an error that we can't
 recover from.  STATUS is the errno value from read_memory_func and
 MEMADDR is the address that we were trying to read.  INFO is a
 pointer to this struct.  */
static void memory_error_func (int status, bfd_vma memaddr, struct disassemble_info *dinfo)
{
   (void) dinfo;
   printf ("memory_error_func status %d addr %p\n", status, (void *) memaddr);
}

/* Function used to get bytes to disassemble.  MEMADDR is the
 address of the stuff to be disassembled, MYADDR is the address to
 put the bytes in, and LENGTH is the number of bytes to read.
 INFO is a pointer to this struct.
 Returns an errno value or 0 for success.  */
static int read_memory_func (bfd_vma memaddr, bfd_byte *myaddr, unsigned int length, struct disassemble_info *dinfo)
{
   unsigned i;
   for (i = 0; (i < length) && (memaddr < max_address); i++) {
      *myaddr = *((char *) memaddr);
      myaddr ++;
      memaddr ++;
   }
   for (; i < length; i++) {
      *myaddr = 0x90;
      myaddr ++;
   }
   (void) dinfo;
   return 0;
}

typedef struct stream_struct {
   unsigned size;
   char data[BUFSIZ];
} stream_struct;

//typedef int (*fprintf_ftype) (void *, const char*, ...) ATTRIBUTE_FPTR_PRINTF_2;
static int scan_printf (void *s, const char *formatstring, ...)
{
   va_list args;
   stream_struct * stream = (stream_struct *) s;
   int rc;
   va_start (args, formatstring);
   rc = vsnprintf (&stream->data[stream->size],
                   BUFSIZ - stream->size,
                   formatstring,
                   args);
   if (rc > 0) {
      stream->size += rc;
   }
   stream->data[stream->size] = '\0';
   return rc;
}

char * stpcpy (char * dest, const char * src)
{
   while (*src) {
      *dest = *src;
      dest ++;
      src ++;
   }
   *dest = '\0';
   return dest;
}

static void superblock_decoder (superblock_info * si, uint32_t pc)
{
   struct disassemble_info di;
   struct stream_struct stream;
   uint32_t address;
   char special = 'X';

   if (!x86_quiet_mode) {
      printf ("new superblock: %08x\n", pc);
   }
   memset (&stream, 0, sizeof (stream));
   memset (&di, 0, sizeof (di));
   di.read_memory_func = read_memory_func;
   di.memory_error_func = memory_error_func;
   di.print_address_func = print_address_func;
   di.symbol_at_address_func = symbol_at_address_func;
   di.symbol_is_valid = symbol_is_valid;
   di.mach = bfd_mach_i386_i386;     /* 32 bit */
   di.fprintf_func = (fprintf_ftype) scan_printf;
   di.stream = (void *) &stream;
   si->count = 0;

   address = pc;
   while (address < max_address) {
      char *      scan = stream.data;
      int         rc;

      stream.data[0] = '\0';
      stream.size = 0;
      si->count ++;
      rc = print_insn_i386 (address, &di);
      if (rc <= 0) {
         // invalid instruction? Assume end of superblock
         special = '?';
         break;
      }
      if (!x86_quiet_mode) {
         printf ("%08x: %s\n", address, scan);
      }
      special = '\0';

      switch (scan[0]) {
         case 'j':
            special = 'J';
            break;
         case 'c':
            if (startswith (scan, "call")) {
               special = 'C';
            }
            break;
         case 'l':
            if (startswith (scan, "loop")) {
               special = 'J';
            }
            break;
         case 'r':
            if (startswith (scan, "ret") || startswith (scan, "repz ret")) {
               special = 'R';
            } else if (startswith (scan, "rdtsc")) {
               special = 't';
            }
            break;
         case 'o':
            if (startswith (scan, "out")) {
               special = 't';
            }
            break;
      }
      if (special) {
         // End of superblock reached
         break;
      }
      address += rc;
   }
   si->size = address - pc;
   if (!x86_quiet_mode) {
      printf ("superblock %08x has %u instructions "
               "and ends at %08x with %c\n", pc, si->count, address, special);
      fflush (stdout);
   }
}

// entry point
int x86_startup (size_t minPage, size_t maxPage)
{
   const char * tmp;

   if (superblocks) {
      return 0x301;
   }
   tmp = getenv ("X86D_QUIET_MODE");
   x86_quiet_mode = 0 && (tmp && (atoi (tmp) != 0));
   min_address = minPage;
   max_address = maxPage;

   superblocks = calloc (sizeof (superblock_info), max_address + 1 - min_address);

   if (!superblocks) {
      return 0x302;
   }
   if (!x86_quiet_mode) {
      printf ("program address range: %08x .. %08x\n", min_address, max_address);
   }

   tmp = getenv ("X86D_BRANCH_TRACE");
   if (tmp && strlen (tmp)) {
      if (!x86_quiet_mode) {
         printf ("writing branch trace to: %s\n", tmp);
      }
      branch_trace = fopen (tmp, "wt");
      if (!branch_trace) {
         perror ("opening branch trace file");
      }
   }
   if (!x86_quiet_mode) {
      fflush (stdout);
   }

   //x86_make_text_writable (min_address, max_address);
   entry_flag = 1;

   // save this context and launch the interpreter
   x86_begin_single_step ();

   // we return to here in user mode only
   return 0;
}


