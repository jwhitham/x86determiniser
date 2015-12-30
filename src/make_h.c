

#ifdef LINUX32

static const char api_name[] = "LINUX32";
#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <ucontext.h>

#define REG_LIMIT (sizeof (gregset_t))

#elif WIN32

static const char api_name[] = "WIN32";
#include <stdint.h>
#include <windows.h>
#include <winnt.h>

static CONTEXT c;
#define oof(x) (((uint32_t *) (&(c.x))) - ((uint32_t *) (&c)))

#define REG_EDI (oof(Edi))
#define REG_ESI (oof(Esi))
#define REG_EBP (oof(Ebp))
#define REG_ESP (oof(Esp))
#define REG_EBX (oof(Ebx))
#define REG_EDX (oof(Edx))
#define REG_ECX (oof(Ecx))
#define REG_EAX (oof(Eax))
#define REG_EIP (oof(Eip))
#define REG_EFL (oof(EFlags))
#define REG_LIMIT (sizeof (CONTEXT))

#endif

#include <stdio.h>

void table (const char * name, unsigned value)
{
    printf ("#define REG_%s (%u)\n", name, value);      /* dword offset */
    printf ("#define OFF_%s (%u)\n", name, value * 4);  /* byte offset */
}

int main (void)
{
    printf ("#define API_NAME \"%s\"\n", api_name);
    table ("EDI", REG_EDI);
    table ("ESI", REG_ESI);
    table ("EBP", REG_EBP);
    table ("ESP", REG_ESP);
    table ("EBX", REG_EBX);
    table ("EDX", REG_EDX);
    table ("ECX", REG_ECX);
    table ("EAX", REG_EAX);
    table ("EIP", REG_EIP);
    table ("EFL", REG_EFL);
    table ("LIMIT", REG_LIMIT);
    return 0;
}

