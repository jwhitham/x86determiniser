#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include <windows.h>

#include "win32_offsets.h"

void x86_trap_handler (uint32_t * gregs, uint32_t trapno);
void x86_startup (const char * objdump_cmd);

// landed at breakpoint / single step
LONG WINAPI breakpoint (LPEXCEPTION_POINTERS uc)
{
    PEXCEPTION_RECORD er = uc->ExceptionRecord;
    PCONTEXT cr = uc->ContextRecord;
    uint32_t * gregs = (uint32_t *) cr;

    switch (er->ExceptionCode) {
        case EXCEPTION_BREAKPOINT:
            // Linux: PC is now after breakpoint instruction
            // Windows: PC is pointing at breakpoint instruction
            // Match Linux behaviour
            gregs[REG_EIP] ++;
            x86_trap_handler (gregs, 3);
            return EXCEPTION_CONTINUE_EXECUTION;
        case EXCEPTION_SINGLE_STEP:
            x86_trap_handler (gregs, 1);
            return EXCEPTION_CONTINUE_EXECUTION;
        default: // unsupported
            printf ("Another sort of exception, code = %08x\n", (unsigned) er->ExceptionCode);
            printf ("EIP = %08x\n", (unsigned) gregs[REG_EIP]);
            printf ("ESP = %08x\n", (unsigned) gregs[REG_ESP]);
            printf ("EA  = %p\n", er->ExceptionAddress);
            x86_trap_handler (gregs, 0);
            return EXCEPTION_CONTINUE_EXECUTION;
    }
}

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


void startup_x86_determiniser (void)
{
    char filename[BUFSIZ];
    char objdump_cmd[BUFSIZ + 128];
    unsigned rc;

    SetUnhandledExceptionFilter (breakpoint);
    rc = GetModuleFileName (NULL, filename, sizeof (filename));
    if (rc >= sizeof (filename)) {
        fputs ("GetModuleFileName failed\n", stderr);
        exit (1);
    }
    snprintf (objdump_cmd, sizeof (objdump_cmd), "objdump -d \"%s\"", filename);
    x86_startup (objdump_cmd);
}



