#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>


#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

#include <windows.h>
#include <excpt.h>

#include "win32_offsets.h"
#include "remote_loader.h"

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

__declspec(dllexport) __cdecl void startup_x86_determiniser (CommStruct * cs)
{
    uint32_t ER[2];
    char filename[BUFSIZ];
    char objdump_cmd[BUFSIZ + 128];
    unsigned rc;
    uint32_t ptr = 0;
    void (* Entry) (void);

    asm volatile ("int3\n");

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

    Entry = cs->continueEntry;
    Entry();
}



