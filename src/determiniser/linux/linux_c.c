
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


static void single_step_handler (struct user_regs_struct * context)
{
   // Convert user_regs_struct, which is used by ptrace, to gregset_t, which is used by
   // signal handlers.
   // "struct user_regs_struct" -> /usr/include/i386-linux-gnu/sys/user.h
   // "gregset_t" -> /usr/include/i386-linux-gnu/sys/ucontext.h
   
   uintptr_t * gregs = (uintptr_t *) context;

   // Run single step handler
   x86_trap_handler (gregs, 1);

   // Completed the single step handler, go back to normal execution
   // Breakpoint with EAX = 0x102 and EBX = pointer to updated context
   x86_bp_trap (COMPLETED_SINGLE_STEP_HANDLER, gregs);
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

   if (0 != mprotect ((void *) pcs->minAddress, (size_t) (pcs->maxAddress - pcs->minAddress),
                     PROT_READ | PROT_WRITE | PROT_EXEC)) {
      x86_bp_trap (FAILED_MEMORY_PERMISSIONS, NULL);
   }

   x86_startup (pcs);

   // Now ready for the user program
   // Breakpoint with EAX = 0x101 and EBX = pointer to single_step_handler
   x86_bp_trap (COMPLETED_LOADER, single_step_handler);
}


