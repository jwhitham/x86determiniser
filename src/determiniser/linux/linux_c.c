
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
   uintptr_t minPage, maxPage;
   FILE * fd;

   minPage = maxPage = 0;

   // Here is the entry point from the RemoteLoader procedure
   // Check internal version first
   x86_check_version (pcs);

   // Discover the bounds of the executable .text segment
   // which is known to contain pcs->startAddress

   fd = fopen ("/proc/self/maps", "rt");
   if (!fd) {
      x86_bp_trap (FAILED_MEMORY_BOUND_DISCOVERY, NULL);
   }

   while (1) {
      unsigned long long tmp1, tmp2;
      int c;

      tmp1 = tmp2 = 0;
      if (2 != fscanf (fd, "%llx-%llx ", &tmp1, &tmp2)) {
         // did not find the min/max bounds
         x86_bp_trap (FAILED_MEMORY_BOUND_DISCOVERY, NULL);
      }

      if (((uintptr_t) tmp1 <= (uintptr_t) pcs->startAddress)
      && ((uintptr_t) pcs->startAddress < (uintptr_t) tmp2)) {
         // Found the min/max bounds for .text
         minPage = (uintptr_t) tmp1;
         maxPage = (uintptr_t) tmp2;
         break;
      }

      do {
         // read to end of line/EOF
         c = fgetc (fd);
      } while ((c != EOF) && (c != '\n'));

      if (c == EOF) {
         // did not find the min/max bounds
         x86_bp_trap (FAILED_MEMORY_BOUND_DISCOVERY, NULL);
      }
   }
   fclose (fd);

   if (0 != mprotect ((void *) minPage, (size_t) (maxPage - minPage),
                     PROT_READ | PROT_WRITE | PROT_EXEC)) {
      x86_bp_trap (FAILED_MEMORY_PERMISSIONS, NULL);
   }

   x86_startup (minPage, maxPage, pcs);

   // Now ready for the user program
   // Breakpoint with EAX = 0x101 and EBX = pointer to single_step_handler
   x86_bp_trap (COMPLETED_LOADER, single_step_handler);
}



