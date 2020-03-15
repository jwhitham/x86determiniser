
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <sys/user.h>
#include <sys/mman.h>

#include "offsets.h"
#include "remote_loader.h"
#include "x86_common.h"
#include "common.h"
#include "linux_context.h"


static void single_step_handler (LINUX_CONTEXT * context)
{
   uintptr_t * gregs = (uintptr_t *) context;

   // Run single step handler
   x86_trap_handler (gregs, 1);

   // Completed the single step handler, go back to normal execution
   // Breakpoint with EAX = 0x102 and EBX = pointer to updated context
   x86_bp_trap (COMPLETED_SINGLE_STEP_HANDLER, gregs);
}

static uintptr_t page_round (uintptr_t address, int round_up)
{
   int page_size = sysconf(_SC_PAGE_SIZE);
   uintptr_t mask;

   if (page_size <= 0) {
      page_size = 4096;
   }
   mask = (uintptr_t) page_size;
   mask --;
   if (round_up) {
      address += (uintptr_t) mask;
   }
   return address & ~mask;
}

void x86_make_text_writable (uintptr_t minAddress, uintptr_t maxAddress)
{
   minAddress = page_round (minAddress, 0);
   maxAddress = page_round (maxAddress, 1);
   if (0 != mprotect ((void *) minAddress, (size_t) (maxAddress - minAddress),
                     PROT_READ | PROT_WRITE | PROT_EXEC)) {
      x86_bp_trap (FAILED_MEMORY_PERMISSIONS, NULL);
   }
}

void x86_make_text_noexec (uintptr_t minAddress, uintptr_t maxAddress)
{
   minAddress = page_round (minAddress, 0);
   maxAddress = page_round (maxAddress, 1);
   // Text must still be writable so that relocations can be added by ld-linux.so.2
   if (0 != mprotect ((void *) minAddress, (size_t) (maxAddress - minAddress),
                     PROT_WRITE)) {
      x86_bp_trap (FAILED_MEMORY_PERMISSIONS, NULL);
   }
}

void X86DeterminiserStartup (CommStruct * pcs)
{
   // Here is the entry point from the RemoteLoader procedure
   // Check internal version first
   x86_check_version (pcs);

   // In Linux the parent process works out the min/max address for the .text section
   // and sets minAddress/maxAddress.
   // (On Windows this is determined within X86DeterminiserStartup based on pcs->startAddress,
   // which is not available on Linux as it points into the ld-linux.so.2 library.)

   x86_startup (pcs);
   pcs->singleStepHandlerAddress = (uintptr_t) single_step_handler;

   // Now ready for the user program
   // Breakpoint with EAX = 0x101 and EBX = pointer to comm struct
   x86_bp_trap (COMPLETED_LOADER, pcs);
}



