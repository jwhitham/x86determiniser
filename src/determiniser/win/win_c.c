#define _WIN32_WINNT 0x0600

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include <windows.h>
#include <excpt.h>

#include "offsets.h"
#include "remote_loader.h"
#include "x86_common.h"
#include "common.h"


extern uint8_t x86_free_run_flag;

static void single_step_handler (PCONTEXT ContextRecord)
{
   uintptr_t * gregs = (uintptr_t *) ContextRecord;

   // Run single step handler
   x86_trap_handler (gregs, 1);

   // Completed the single step handler, go back to normal execution
   // Breakpoint with EAX = 0x102 and EBX = pointer to updated context
   x86_bp_trap (COMPLETED_SINGLE_STEP_HANDLER, gregs);
}

void x86_make_text_writable (uintptr_t minAddress, uintptr_t maxAddress)
{
   MEMORY_BASIC_INFORMATION mbi;

   if (!VirtualProtect ((void *) minAddress, maxAddress - minAddress,
         PAGE_EXECUTE_READWRITE, &mbi.Protect)) {
      x86_bp_trap (FAILED_MEMORY_PERMISSIONS, NULL);
   }
}

void x86_make_text_noexec (uintptr_t minAddress, uintptr_t maxAddress)
{
   MEMORY_BASIC_INFORMATION mbi;
   DWORD flags = 0;
   BOOL permanent = FALSE;

   // If DEP is enabled, then we can use PAGE_READWRITE to detect attempts to execute
   // the program without also preventing reading/writing the code. But if DEP is not
   // enabled, then PAGE_READWRITE means the same thing as PAGE_EXECUTE_READWRITE, so
   // we can't use it :(
   GetProcessDEPPolicy (GetCurrentProcess (), &flags, &permanent);

   if (!VirtualProtect ((void *) minAddress, maxAddress - minAddress,
         (flags & PROCESS_DEP_ENABLE) ? PAGE_READWRITE : PAGE_NOACCESS,
         &mbi.Protect)) {
      x86_bp_trap (FAILED_MEMORY_PERMISSIONS, NULL);
   }
}
__declspec(dllexport) void X86DeterminiserStartup (CommStruct * pcs)
{
   SYSTEM_INFO systemInfo;
   MEMORY_BASIC_INFORMATION mbi;
   size_t pageSize, pageMask;

   // Here is the entry point from the RemoteLoader procedure
   // Check internal version first
   x86_check_version (pcs);

   // Discover the bounds of the executable .text segment
   // which is known to contain pcs->startAddress
   // (On Linux the parent process works out the min/max address for the .text section
   // and sets minAddress/maxAddress, and startAddress is not available.)
   GetSystemInfo (&systemInfo);
   pageSize = systemInfo.dwPageSize;
   pageMask = ~ (pageSize - 1);
   pcs->minAddress = (uintptr_t) (((size_t) pcs->startAddress) & pageMask);
   ZeroMemory (&mbi, sizeof (mbi));

   // find minimum page
   while ((VirtualQuery ((void *) (pcs->minAddress - pageSize), &mbi, sizeof (mbi)) != 0)
   && mbi.RegionSize > pageSize) {
      pcs->minAddress -= pageSize;
   }

   // calculate maximum page
   if (VirtualQuery ((void *) pcs->minAddress, &mbi, sizeof (mbi)) == 0) {
      // Error code EAX = 0x104
      x86_bp_trap (FAILED_MEMORY_BOUND_DISCOVERY, NULL);
   }
   pcs->maxAddress = mbi.RegionSize + pcs->minAddress;

   pcs->singleStepHandlerAddress = (uintptr_t) single_step_handler;
   pcs->freeRunFlagAddress = (uintptr_t) &x86_free_run_flag;
   x86_startup (pcs);

   // Now ready for the user program
   // Breakpoint with EAX = 0x101 and EBX = pointer to comm struct
   x86_bp_trap (COMPLETED_LOADER, pcs);
}



