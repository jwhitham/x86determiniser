#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>


#define _WIN32_WINNT 0x0500

#include <windows.h>
#include <excpt.h>

#include "win32_offsets.h"

void x86_trap_handler (uint32_t * gregs, uint32_t trapno);
void x86_startup (const char * objdump_cmd);

void x86_make_text_writable (uint32_t min_address, uint32_t max_address)
{
    MEMORY_BASIC_INFORMATION meminfo;
    ZeroMemory (&meminfo, sizeof (MEMORY_BASIC_INFORMATION));

    if (!VirtualProtect ((void *) min_address, max_address - min_address,
            PAGE_EXECUTE_READWRITE, &meminfo.Protect)) {
        fputs ("VirtualProtect .text READ_WRITE setting failed\n", stderr);
        exit (1);
    }
}

EXCEPTION_DISPOSITION __cdecl
new_handler (PEXCEPTION_RECORD ExceptionRecord,
              void *EstablisherFrame,
              PCONTEXT ContextRecord,
              void *DispatcherContext)
{
   if (ExceptionRecord->ExceptionCode == STATUS_SINGLE_STEP) {
      uint32_t * gregs = (uint32_t *) ContextRecord;
      x86_trap_handler (gregs, 1);
      return ExceptionContinueExecution;
   } else {
      return ExceptionContinueSearch;
   }
}

void startup_x86_determiniser (uint32_t * ER)
{
    char filename[BUFSIZ];
    char objdump_cmd[BUFSIZ + 128];
    unsigned rc;
    uint32_t ptr = 0;
    const char * x86_disabled = getenv ("X86D_DISABLED");

    if (x86_disabled && (atoi (x86_disabled) != 0)) {
        printf ("libx86determiniser is disabled by environment variable; running code natively\n");
        return; // skip initialisation, run program natively
    }

    if (!ER) {
        fputs ("startup_x86_determiniser must be passed an SEH "
               "EXCEPTION_RECORD pointer\n", stderr);
        exit (1);
    }

    // put current handler in ptr
    asm ("mov %%fs:(0),%0" : "=r" (ptr));
    ER[0] = (uint32_t)ptr;          /* previous handler */
    ER[1] = (uint32_t)new_handler;  /* new handler */

    /* ER is the new handler, set fs:(0) with this value */
    ptr = (uint32_t) &ER[0];
    asm volatile ("mov %0,%%fs:(0)": : "r" (ptr));

    rc = GetModuleFileName (NULL, filename, sizeof (filename));
    if (rc >= sizeof (filename)) {
        fputs ("GetModuleFileName failed\n", stderr);
        exit (1);
    }
    snprintf (objdump_cmd, sizeof (objdump_cmd), "objdump -d \"%s\"", filename);
    x86_startup (objdump_cmd);
}



