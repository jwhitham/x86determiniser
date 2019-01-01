

#ifdef LINUX32

static const char api_name[] = "LINUX32";
#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <ucontext.h>

#define REG_LIMIT (sizeof (gregset_t))
#define REGISTER_PREFIX 'E'

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
#define REG_XFL (oof(EFlags))
#define REG_R8 (oof(R8))
#define REG_R9 (oof(R9))
#define REG_R10 (oof(R10))
#define REG_R11 (oof(R11))
#define REG_R12 (oof(R12))
#define REG_R13 (oof(R13))
#define REG_R14 (oof(R14))
#define REG_R15 (oof(R15))
#define REG_LIMIT (sizeof (CONTEXT))
#define REGISTER_PREFIX 'R'
#define IS_64_BIT

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
#define REG_XFL (oof(EFlags))
#define REG_LIMIT (sizeof (CONTEXT))
#define REGISTER_PREFIX 'E'

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
   table ("XFL", REG_XFL);
#ifdef IS_64_BIT
   table ("R8", REG_R8);
   table ("R9", REG_R9);
   table ("R10", REG_R10);
   table ("R11", REG_R11);
   table ("R12", REG_R12);
   table ("R13", REG_R13);
   table ("R14", REG_R14);
   table ("R15", REG_R15);
#endif
   table ("LIMIT", REG_LIMIT);
   printf ("#define REGISTER_PREFIX '%c'\n", REGISTER_PREFIX);
   return 0;
}

