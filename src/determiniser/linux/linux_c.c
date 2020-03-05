
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

static int read_maps (const void * excludeAddress, uintptr_t * minPage, uintptr_t * maxPage)
{
   FILE * fd;

   (* minPage) = (* maxPage) = 0;

   fd = fopen ("/proc/self/maps", "rt");
   if (!fd) {
      return 0;
   }

   while (1) {
      unsigned long long tmp1, tmp2;
      char rwxp_flags[5];
      int c;

      tmp1 = tmp2 = 0;
      if (3 != fscanf (fd, "%llx-%llx %4s ", &tmp1, &tmp2, rwxp_flags)) {
         // did not find the min/max bounds
         fclose (fd);
         return 0;
      }

      rwxp_flags[4] = '\0';

      if (strcmp (rwxp_flags, "r-xp") == 0) {
         // This is a .text section
         if (((uintptr_t) tmp1 <= (uintptr_t) excludeAddress)
         && ((uintptr_t) excludeAddress < (uintptr_t) tmp2)) {
            // This section is excluded
         } else {
            // Found usable min/max bounds for .text
            (* minPage) = (uintptr_t) tmp1;
            (* maxPage) = (uintptr_t) tmp2;
            fclose (fd);
            return 1;
         }
      }

      do {
         // read to end of line/EOF
         c = fgetc (fd);
      } while ((c != EOF) && (c != '\n'));

      if (c == EOF) {
         // did not find the min/max bounds
         fclose (fd);
         return 0;
      }
   }
}

void X86DeterminiserStartup (CommStruct * pcs)
{
   uintptr_t minPage, maxPage;

   // Here is the entry point from the RemoteLoader procedure
   // Check internal version first
   x86_check_version (pcs);

   // Discover the bounds of the executable .text segment,
   // initially excluding pcs->startAddress, which is probably in ld-linux.so.2 or an equivalent
   if (!read_maps (pcs->startAddress, &minPage, &maxPage)) {
      // Did not find the bounds, so maybe there is no ld-linux.so.2? Try again.
      if (!read_maps (NULL, &minPage, &maxPage)) {
         // Nothing can be found - give up
         x86_bp_trap (FAILED_MEMORY_BOUND_DISCOVERY, NULL);
      }
   }

   if (0 != mprotect ((void *) minPage, (size_t) (maxPage - minPage),
                     PROT_READ | PROT_WRITE | PROT_EXEC)) {
      x86_bp_trap (FAILED_MEMORY_PERMISSIONS, NULL);
   }

   x86_startup (minPage, maxPage, pcs);

   // Now ready for the user program
   // Breakpoint with EAX = 0x101 and EBX = pointer to single_step_handler
   x86_bp_trap (COMPLETED_LOADER, single_step_handler);
}



