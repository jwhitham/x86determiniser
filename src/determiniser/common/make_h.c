

#ifdef LINUX32

static const char api_name[] = "LINUX32";
#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/user.h>
#include <unistd.h>
#include <stdint.h>

#include "linux_context.h"

static LINUX_CONTEXT c;
#define oof(x) (((uintptr_t *) (&(c.x))) - ((uintptr_t *) (&c)))

#define REG_XDI (oof(regs.edi))
#define REG_XSI (oof(regs.esi))
#define REG_XBP (oof(regs.ebp))
#define REG_XSP (oof(regs.esp))
#define REG_XBX (oof(regs.ebx))
#define REG_XDX (oof(regs.edx))
#define REG_XCX (oof(regs.ecx))
#define REG_XAX (oof(regs.eax))
#define REG_XIP (oof(regs.eip))
#define REG_LIMIT (sizeof (LINUX_CONTEXT))
#define REGISTER_PREFIX 'E'
#define OFF_EFL (4 * oof(regs.eflags))


#else
#ifdef LINUX64

static const char api_name[] = "LINUX64";
#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/user.h>
#include <unistd.h>
#include <stdint.h>

#include "linux_context.h"

static LINUX_CONTEXT c;
#define oof(x) (((uintptr_t *) (&(c.x))) - ((uintptr_t *) (&c)))

#define REG_XDI (oof(regs.rdi))
#define REG_XSI (oof(regs.rsi))
#define REG_XBP (oof(regs.rbp))
#define REG_XSP (oof(regs.rsp))
#define REG_XBX (oof(regs.rbx))
#define REG_XDX (oof(regs.rdx))
#define REG_XCX (oof(regs.rcx))
#define REG_XAX (oof(regs.rax))
#define REG_XIP (oof(regs.rip))
#define REG_R8 (oof(regs.r8))
#define REG_R9 (oof(regs.r9))
#define REG_R10 (oof(regs.r10))
#define REG_R11 (oof(regs.r11))
#define REG_R12 (oof(regs.r12))
#define REG_R13 (oof(regs.r13))
#define REG_R14 (oof(regs.r14))
#define REG_R15 (oof(regs.r15))
#define REG_Xmm0 (oof(fpregs.xmm_space[0]))
#define REG_Xmm1 (oof(fpregs.xmm_space[2]))
#define REG_Xmm2 (oof(fpregs.xmm_space[4]))
#define REG_Xmm3 (oof(fpregs.xmm_space[6]))
#define REG_Xmm4 (oof(fpregs.xmm_space[8]))
#define REG_Xmm5 (oof(fpregs.xmm_space[10]))
#define REG_Xmm6 (oof(fpregs.xmm_space[12]))
#define REG_Xmm7 (oof(fpregs.xmm_space[14]))
#define REG_Xmm8 (oof(fpregs.xmm_space[16]))
#define REG_Xmm9 (oof(fpregs.xmm_space[18]))
#define REG_Xmm10 (oof(fpregs.xmm_space[20]))
#define REG_Xmm11 (oof(fpregs.xmm_space[22]))
#define REG_Xmm12 (oof(fpregs.xmm_space[24]))
#define REG_Xmm13 (oof(fpregs.xmm_space[26]))
#define REG_Xmm14 (oof(fpregs.xmm_space[28]))
#define REG_Xmm15 (oof(fpregs.xmm_space[30]))
#define REG_LIMIT (sizeof (LINUX_CONTEXT))
#define REGISTER_PREFIX 'R'
#define IS_64_BIT
#define OFF_EFL (((uint8_t *) (&(c.regs.eflags))) - ((uint8_t *) (&c)))

#else
#ifdef WIN64

static const char api_name[] = "WIN64";
#include <stdint.h>
#include <windows.h>
#include <winnt.h>

static CONTEXT c;
#define oof(x) (((uintptr_t *) (&(c.x))) - ((uintptr_t *) (&c)))

#define REG_XDI (oof(Rdi))
#define REG_XSI (oof(Rsi))
#define REG_XBP (oof(Rbp))
#define REG_XSP (oof(Rsp))
#define REG_XBX (oof(Rbx))
#define REG_XDX (oof(Rdx))
#define REG_XCX (oof(Rcx))
#define REG_XAX (oof(Rax))
#define REG_XIP (oof(Rip))
#define REG_R8 (oof(R8))
#define REG_R9 (oof(R9))
#define REG_R10 (oof(R10))
#define REG_R11 (oof(R11))
#define REG_R12 (oof(R12))
#define REG_R13 (oof(R13))
#define REG_R14 (oof(R14))
#define REG_R15 (oof(R15))
#define REG_Xmm0 (oof(Xmm0))
#define REG_Xmm1 (oof(Xmm1))
#define REG_Xmm2 (oof(Xmm2))
#define REG_Xmm3 (oof(Xmm3))
#define REG_Xmm4 (oof(Xmm4))
#define REG_Xmm5 (oof(Xmm5))
#define REG_Xmm6 (oof(Xmm6))
#define REG_Xmm7 (oof(Xmm7))
#define REG_Xmm8 (oof(Xmm8))
#define REG_Xmm9 (oof(Xmm9))
#define REG_Xmm10 (oof(Xmm10))
#define REG_Xmm11 (oof(Xmm11))
#define REG_Xmm12 (oof(Xmm12))
#define REG_Xmm13 (oof(Xmm13))
#define REG_Xmm14 (oof(Xmm14))
#define REG_Xmm15 (oof(Xmm15))
#define REG_LIMIT (sizeof (CONTEXT))
#define REGISTER_PREFIX 'R'
#define IS_64_BIT
#define OFF_EFL (((uint8_t *) (&(c.EFlags))) - ((uint8_t *) (&c)))

#else
#ifdef WIN32

static const char api_name[] = "WIN32";
#include <stdint.h>
#include <windows.h>
#include <winnt.h>

static CONTEXT c;
#define oof(x) (((uintptr_t *) (&(c.x))) - ((uintptr_t *) (&c)))

#define REG_XDI (oof(Edi))
#define REG_XSI (oof(Esi))
#define REG_XBP (oof(Ebp))
#define REG_XSP (oof(Esp))
#define REG_XBX (oof(Ebx))
#define REG_XDX (oof(Edx))
#define REG_XCX (oof(Ecx))
#define REG_XAX (oof(Eax))
#define REG_XIP (oof(Eip))
#define REG_LIMIT (sizeof (CONTEXT))
#define REGISTER_PREFIX 'E'
#define OFF_EFL (4 * oof(EFlags))

#endif
#endif
#endif
#endif

#include <stdio.h>

void table (const char * name, unsigned value)
{
   /* ptr offset */
   printf ("#define REG_%s (%u)\n", name, value);
   /* byte offset */
   printf ("#define OFF_%s (%u)\n", name, (unsigned) (value * sizeof (void *)));
}

int main (void)
{
   printf ("#define API_NAME \"%s\"\n", api_name);
   table ("XDI", REG_XDI);
   table ("XSI", REG_XSI);
   table ("XBP", REG_XBP);
   table ("XSP", REG_XSP);
   table ("XBX", REG_XBX);
   table ("XDX", REG_XDX);
   table ("XCX", REG_XCX);
   table ("XAX", REG_XAX);
   table ("XIP", REG_XIP);
#ifdef IS_64_BIT
   table ("R8", REG_R8);
   table ("R9", REG_R9);
   table ("R10", REG_R10);
   table ("R11", REG_R11);
   table ("R12", REG_R12);
   table ("R13", REG_R13);
   table ("R14", REG_R14);
   table ("R15", REG_R15);
   table ("Xmm0", REG_Xmm0);
   table ("Xmm1", REG_Xmm1);
   table ("Xmm2", REG_Xmm2);
   table ("Xmm3", REG_Xmm3);
   table ("Xmm4", REG_Xmm4);
   table ("Xmm5", REG_Xmm5);
   table ("Xmm6", REG_Xmm6);
   table ("Xmm7", REG_Xmm7);
   table ("Xmm8", REG_Xmm8);
   table ("Xmm9", REG_Xmm9);
   table ("Xmm10", REG_Xmm10);
   table ("Xmm11", REG_Xmm11);
   table ("Xmm12", REG_Xmm12);
   table ("Xmm13", REG_Xmm13);
   table ("Xmm14", REG_Xmm14);
   table ("Xmm15", REG_Xmm15);
#endif
   table ("LIMIT", REG_LIMIT);
   printf ("#define OFF_EFL (%u)\n", (unsigned) OFF_EFL);
   printf ("#define REGISTER_PREFIX '%c'\n", REGISTER_PREFIX);
   return 0;
}

