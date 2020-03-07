#define _XOPEN_SOURCE 600

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>

#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/signal.h>
#include <sys/stat.h>

#include "remote_loader.h"
#include "x86_flags.h"
#include "common.h"

#define dbg_fprintf if (pcs->debugEnabled) fprintf

typedef struct SingleStepStruct {
   void * unused;
   struct user_regs_struct * pcontext;
   struct user_regs_struct context;
} SingleStepStruct;

static void err_printf (unsigned err_code, const char * fmt, ...)
{
   int err = errno;
   va_list ap;

   va_start (ap, fmt);
   vfprintf (stderr, fmt, ap);
   va_end (ap);

   if (err_code == 1) {
      char buf[BUFSIZ];
      buf[0] = '\0';
      strerror_r (err, buf, sizeof (buf));
      buf[BUFSIZ - 1] = '\0';
      fprintf (stderr, ": Linux error code 0x%x: %s\n", (unsigned) err, buf);

   } else if ((err_code >= X86D_FIRST_ERROR) && (err_code <= X86D_LAST_ERROR)) {
      const char * buf = X86Error (err_code);
      fprintf (stderr, ": code 0x%x: %s\n", err_code - X86D_FIRST_ERROR, buf);
   } else {
      fprintf (stderr, "\n");
   }
}

static uintptr_t get_stack_ptr (struct user_regs_struct * context)
{
#ifdef LINUX64
   return context->rsp;
#else
   return context->esp;
#endif
}

static void set_stack_ptr (struct user_regs_struct * context, uintptr_t sp)
{
#ifdef LINUX64
   context->rsp = sp;
#else
   context->esp = sp;
#endif
}

static uintptr_t get_xax (struct user_regs_struct * context)
{
#ifdef LINUX64
   return context->rax;
#else
   return context->eax;
#endif
}

static uintptr_t get_xbx (struct user_regs_struct * context)
{
#ifdef LINUX64
   return context->rbx;
#else
   return context->ebx;
#endif
}

/*
static uintptr_t get_xsp (struct user_regs_struct * context)
{
#ifdef LINUX64
   return context->rsp;
#else
   return context->esp;
#endif
}
*/

static uintptr_t get_pc (struct user_regs_struct * context)
{
#ifdef LINUX64
   return context->rip;
#else
   return context->eip;
#endif
}

static void set_pc (struct user_regs_struct * context, uintptr_t pc)
{
#ifdef LINUX64
   context->rip = pc;
#else
   context->eip = pc;
#endif
}

static void clear_single_step_flag (struct user_regs_struct * context)
{
   context->eflags &= ~SINGLE_STEP_FLAG;
}

static uintptr_t align_64 (uintptr_t v)
{
   return v & ~63;
}

static void DebugUserRegs (struct user_regs_struct * context)
{
   size_t i;
   for (i = 0; i < sizeof (struct user_regs_struct); i += 4) {
      if ((i % 16) == 0) {
         fprintf (stderr, "\nraw data:  ");
      }
      fprintf (stderr, "  %08x", ((uint32_t *) &context)[i / 4]);
   }
   fprintf (stderr, "\n");
   fprintf (stderr, "ebx = %08x  ", (uint32_t) context->ebx);
   fprintf (stderr, "ecx = %08x  ", (uint32_t) context->ecx);
   fprintf (stderr, "edx = %08x  ", (uint32_t) context->edx);
   fprintf (stderr, "esi = %08x\n", (uint32_t) context->esi);
   fprintf (stderr, "edi = %08x  ", (uint32_t) context->edi);
   fprintf (stderr, "ebp = %08x  ", (uint32_t) context->ebp);
   fprintf (stderr, "eax = %08x  ", (uint32_t) context->eax);
   fprintf (stderr, "xds = %08x\n", (uint32_t) context->xds);
   fprintf (stderr, "xes = %08x  ", (uint32_t) context->xes);
   fprintf (stderr, "xfs = %08x  ", (uint32_t) context->xfs);
   fprintf (stderr, "xgs = %08x  ", (uint32_t) context->xgs);
   fprintf (stderr, "0ax = %08x\n", (uint32_t) context->orig_eax);
   fprintf (stderr, "eip = %08x  ", (uint32_t) context->eip);
   fprintf (stderr, "xcs = %08x  ", (uint32_t) context->xcs);
   fprintf (stderr, "efl = %08x  ", (uint32_t) context->eflags);
   fprintf (stderr, "esp = %08x\n", (uint32_t) context->esp);
   fprintf (stderr, "xss = %08x\n", (uint32_t) context->xss);
}

// parent receives data from child
static void GetData
  (pid_t childPid,
   uintptr_t childAddress,
   void * parentAddress,
   size_t size)
{
   uint8_t * parentAddressCopy = (uint8_t *) parentAddress;

   // copy full words
   while (size >= sizeof (uintptr_t)) {
      *((uintptr_t *) parentAddressCopy) = ptrace (PTRACE_PEEKDATA, childPid, childAddress, NULL);
      parentAddressCopy += sizeof (uintptr_t);
      childAddress += sizeof (uintptr_t);
      size -= sizeof (uintptr_t);
   }
   // copy final word if any
   if (size > 0) {
      union u {
         uintptr_t word;
         char bytes[sizeof(uintptr_t)];
      } data;
      data.word = ptrace (PTRACE_PEEKDATA, childPid, childAddress, NULL);
      memcpy (parentAddressCopy, &data.bytes, size);
   }
}

// parent sends data to child
static void PutData
  (pid_t childPid,
   uintptr_t childAddress,
   const void * parentAddress,
   size_t size)
{
   const uint8_t * parentAddressCopy = (uint8_t *) parentAddress;

   // copy full words
   while (size >= sizeof (uintptr_t)) {
      ptrace (PTRACE_POKEDATA, childPid, childAddress, *((uintptr_t *) parentAddressCopy));
      parentAddressCopy += sizeof (uintptr_t);
      childAddress += sizeof (uintptr_t);
      size -= sizeof (uintptr_t);
   }
   // copy final word if any
   if (size > 0) {
      union u {
         uintptr_t word;
         char bytes[sizeof(uintptr_t)];
      } data;
      data.word = ptrace (PTRACE_PEEKDATA, childPid, childAddress, NULL);
      memcpy (&data.bytes, parentAddressCopy, size);
      ptrace (PTRACE_POKEDATA, childPid, childAddress, data.word);
   }
}

static int IsSingleStep (pid_t childPid, struct user_regs_struct * context)
{ 
   // https://sourceware.org/gdb/wiki/LinuxKernelWishList
   // "It would be useful for the kernel to tell us whether a SIGTRAP corresponds to a
   //  breakpoint hit or a finished single-step (or something else), without having to
   //  resort to heuristics. Checking for the existence of an int3 in memory at PC-1
   //  is not a complete solution, as the breakpoint might be gone already...
   //  Some archs started encoding trap discrimination on SIGTRAPs si_code..."

   uint8_t previous_byte[1];

   GetData (childPid, get_pc (context) - 1, previous_byte, 1);
   if (previous_byte[0] == 0xcc) {
      return 0; // breakpoint
   } else {
      return 1; // single step
   }
}


// Read the memory maps 'file' for a particular process.
// Find the first .text section that excludes the specified address
static int ReadMaps (pid_t childPid, uintptr_t excludeAddress, CommStruct * pcs)
{
   FILE * fd;
   char mapFileName[BUFSIZ];

   pcs->minAddress = pcs->maxAddress = 0;
   snprintf (mapFileName, sizeof (mapFileName), "/proc/%d/maps", (int) childPid);
   mapFileName[BUFSIZ - 1] = '\0';

   fd = fopen (mapFileName, "rt");
   if (!fd) {
      return 0;
   }

   if (pcs->debugEnabled) {
      // copy map contents to stderr
      char buf[BUFSIZ];
      while (fgets (buf, sizeof (buf), fd)) {
         buf[sizeof(buf) - 1] = '\0';
         fprintf (stderr, "map for %d: %s", childPid, buf);
      }
      fclose (fd);
      fd = fopen (mapFileName, "rt");
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
            pcs->minAddress = ((uintptr_t) tmp1);
            pcs->maxAddress = ((uintptr_t) tmp2);
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


static void StartSingleStepProc
  (uintptr_t singleStepProc,
   pid_t childPid,
   struct user_regs_struct * context)
{
   SingleStepStruct localCs;
   uintptr_t new_sp;

   memcpy (&localCs.context, context, sizeof (struct user_regs_struct));

   // reserve stack space for SingleStepStruct
   new_sp = get_stack_ptr (context) - sizeof (localCs);
   new_sp = align_64 (new_sp);
   set_stack_ptr(context, new_sp);

   // generate pointer to context within localCs
   // This will be the first parameter
   localCs.pcontext = (void *) (new_sp +
      ((uintptr_t) &localCs.context - (uintptr_t) &localCs));
   localCs.unused = NULL;

#ifdef IS_64_BIT
   context->rcx = (uintptr_t) localCs.pcontext;
#endif

   // fill remote stack
   // return address is NULL (1st item in struct: don't return!!)
   // first parameter is "pcontext" (2nd item in struct)
   PutData (childPid, get_stack_ptr(context), (const void *) &localCs, sizeof (localCs));

   // run single step handler until breakpoint
   set_pc (context, (uintptr_t) singleStepProc);
   clear_single_step_flag(context);
}

// Handle a signal or other event from the child process which is not related
// to x86determiniser's operation
static void DefaultHandler (pid_t childPid, int status, const char * state)
{
   if (WIFSTOPPED(status)) {
      // This means a signal was received, other than the one we are using (SIGTRAP).
      // It's normal for the user program to receive signals.
      // Pass them onwards to the program.
      ptrace (PTRACE_CONT, childPid, NULL, (void *) WSTOPSIG(status));

   } else if (WIFEXITED(status)) {
      // normal exit
      exit (WEXITSTATUS(status));

   } else if (WIFSIGNALED(status)) {
      // terminated by a signal
      switch (WTERMSIG(status)) {
         case SIGILL:
            err_printf (0, "Illegal instruction");
            exit (1);
            break;
         case SIGSEGV:
            err_printf (0, "Segmentation fault");
            exit (1);
            break;
         case SIGFPE:
            err_printf (0, "Divide by zero");
            exit (1);
            break;
         default:
            // pass through
            err_printf (0, "%s: terminated by signal %d\n",
                     state, WTERMSIG(status));
            exit (1);
      }
   } else {
      err_printf (0, "%s: waitpid returned for unknown reason: status = 0x%x\n", state, status);
      exit (1);
   }
}


// Entry point from main
// Args already parsed into CommStruct
int X86DeterminiserLoader(CommStruct * pcs, int argc, char ** argv)
{
   uintptr_t   singleStepProc = 0;
   unsigned    trapCount = 0;
   pid_t       childPid = -1;
   char        binFolder[BUFSIZ];
   int         status = 0;
   FILE *      testFd = 0;
   int         run = 1;
   int         elfType = 0;
   char        elfHeader[5];
   struct user_regs_struct context;
   struct user_regs_struct enterSSContext;
   siginfo_t   siginfo;

   memset (&context, 0, sizeof (struct user_regs_struct));
   memset (&enterSSContext, 0, sizeof (struct user_regs_struct));
   memset (&siginfo, 0, sizeof (siginfo_t));
   memset (&elfHeader, 0, sizeof (elfHeader));

   (void) argc;

   if (readlink ("/proc/self/exe", binFolder, sizeof (binFolder) - 1) > 0) {
      char * finalSlash = strrchr (binFolder, '/');
      if (!finalSlash) {
         finalSlash = binFolder;
      }
      *finalSlash = '\0';
   } else {
      err_printf (1, "readlink('" X86_OR_X64 "determiniser')");
      exit (1);
   }

   // library name
   snprintf (pcs->libraryName, MAX_FILE_NAME_SIZE, "%s/%sdeterminiser.so",
      binFolder, X86_OR_X64);
   dbg_fprintf (stderr, "INITIAL: library is '%s'\n", pcs->libraryName);

   // check library exists
   testFd = fopen (pcs->libraryName, "rb");
   if (!testFd) {
      err_printf (1, "open('%s')", pcs->libraryName);
      exit (1);
   }
   fclose (testFd);

   // check child process executable exists and has the correct format
   testFd = fopen (argv[0], "rb");
   if (!testFd) {
      err_printf (1, "program '%s' not found", argv[0]);
      exit (1);
   }
   // read errors here will be detected by memcmp below:
   (void) fread (elfHeader, 1, sizeof (elfHeader), testFd);
   fclose (testFd);
   dbg_fprintf (stderr, "INITIAL: program is '%s'\n", argv[0]);

   if (memcmp (elfHeader, "\x7f" "ELF", 4) != 0) {
      // make ELF header check report an error below:
      elfHeader[4] = '\0';
   }

   // an ELF header is fairly complex but apparently there is an easy method to determine 32/64 bit:
   // https://unix.stackexchange.com/questions/106234/determine-if-a-specific-process-is-32-or-64-bit
   switch (elfHeader[4]) {
      case 1:
         elfType = 32;
         break;
      case 2:
         elfType = 64;
         break;
      default:
         err_printf (0, "program '%s' must be an x86 ELF executable", argv[0]);
         exit (1);
   }
   if (elfType != (PTR_SIZE * 8)) {
      fprintf (stderr, "%s: executable appears to be %d-bit, try x%ddeterminiser instead\n",
         argv[0],
         (PTR_SIZE == 4) ? 64 : 32,
         (PTR_SIZE == 4) ? 64 : 86);
      exit (1);
   }

   // run the subprocess, preloading the library
   setenv ("LD_PRELOAD", pcs->libraryName, 1);
   childPid = fork ();
   if (childPid < 0) {
      err_printf (1, "fork");
      exit (1);
   } else if (childPid == 0) {
      // Run the child process: wait for the parent to attach via ptrace
      ptrace (PTRACE_TRACEME, 0, NULL, NULL);
      execv (argv[0], argv);

      // TODO If exec'ing the child process failed for some reason, here is where
      // we should provide a way to report that.
      exit (1);
   }
   dbg_fprintf (stderr, "INITIAL: child pid is %d\n", (int) childPid);

   // INITIAL STAGE
   // Wait for the child process to attach
   // (execv ought to generate a SIGTRAP when called after PTRACE_TRACEME.)
   // Obtain the start address for the child process executable
   // Run until the first system call which would be in ld-linux.
   while (trapCount == 0) {
      if (waitpid (childPid, &status, 0) != childPid) {
         err_printf (1, "INITIAL: waitpid");
         exit (1);
      }

      if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGTRAP) {
         // child process stopped - this is the initial stop, after the execv call
         memset (&context, 0, sizeof (struct user_regs_struct));

         if (ptrace (PTRACE_GETREGS, childPid, NULL, &context) != 0) {
            err_printf (1, "INITIAL: PTRACE_GETREGS");
            exit (1);
         }
         dbg_fprintf (stderr, "INITIAL: entry PC = %p\n", (void *) get_pc (&context));

         // Discover the bounds of the executable .text section,
         // initially excluding the current address, which is probably in ld-linux.so.2 or an equivalent
         if (!ReadMaps (childPid, get_pc (&context), pcs)) {
            // Did not find the bounds, so maybe there is no ld-linux.so.2? Try again.
            if (!ReadMaps (childPid, 0, pcs)) {
               // Nothing can be found - give up
               err_printf (0, "INITIAL: unable to determine bounds of .text section for '%s'", argv[0]);
               exit (1);
            }
         }
         dbg_fprintf (stderr, "INITIAL: minAddress = %p\n", (void *) pcs->minAddress);
         dbg_fprintf (stderr, "INITIAL: maxAddress = %p\n", (void *) pcs->maxAddress);

         // continue to breakpoint
         ptrace (PTRACE_CONT, childPid, NULL, NULL);
         trapCount = 1;

      } else if (WIFSTOPPED(status)) {
         err_printf (0, "INITIAL: child process stopped unexpectedly (%d)", (int) (WSTOPSIG(status)));
         exit (1);

      } else if (WIFEXITED(status)) {
         err_printf (0, "INITIAL: child process exited immediately");
         exit (1);
      }
   }


   // AWAIT_FIRST_STAGE STAGE
   // Wait for the breakpoint indicating that the RemoteLoader procedure is executing.
   while (trapCount == 1) {
      if (waitpid (childPid, &status, 0) != childPid) {
         err_printf (1, "AWAIT_FIRST_STAGE: waitpid");
         exit (1);
      }

      if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGTRAP) {
         // child process stopped - this should be the second stop, in RemoteLoader
         // The eax/rax register should contain the address of CommStruct in the child process
         uintptr_t remoteLoaderCS;
         CommStruct check_copy;

         memset (&context, 0, sizeof (struct user_regs_struct));

         if (ptrace (PTRACE_GETREGS, childPid, NULL, &context) != 0) {
            err_printf (1, "AWAIT_FIRST_STAGE: PTRACE_GETREGS");
            exit (1);
         }

         dbg_fprintf (stderr, "AWAIT_FIRST_STAGE: breakpoint at %p\n", (void *) get_pc (&context));

         // check we stopped in the right place: EAX/RAX contains the expected code
         if (get_xax (&context) != COMPLETED_REMOTE) {
            err_printf (0, "AWAIT_FIRST_STAGE: breakpoint was not in RemoteLoader procedure");
            exit (1);
         }

         // copy CommStruct data to the child: EBX/RBX contains the address
         remoteLoaderCS = get_xbx (&context);
         dbg_fprintf (stderr, "AWAIT_FIRST_STAGE: remoteLoaderCS = %p\n", (void *) remoteLoaderCS);
         PutData (childPid, remoteLoaderCS, (const void *) pcs, sizeof (CommStruct));

         // check for correct data transfer
         memset (&check_copy, 0xaa, sizeof (CommStruct));
         GetData (childPid, remoteLoaderCS, (void *) &check_copy, sizeof (CommStruct));
         if (memcmp (&check_copy, pcs, sizeof (CommStruct) != 0)) {
            err_printf (0, "AWAIT_FIRST_STAGE: readback of CommStruct failed");
            exit (1);
         }
         dbg_fprintf (stderr, "AWAIT_FIRST_STAGE: readback ok (%d bytes)\n", sizeof (CommStruct));

         // continue to next event (from determiniser)
         ptrace (PTRACE_CONT, childPid, NULL, NULL);
         trapCount = 2;

      } else if (WIFSTOPPED(status)) {
         err_printf (0, "AWAIT_FIRST_STAGE: child process stopped unexpectedly (%d)", (int) (WSTOPSIG(status)));
         exit (1);

      } else if (WIFEXITED(status)) {
         err_printf (0, "AWAIT_FIRST_STAGE: child process exited immediately");
         exit (1);
      }
   }

   // AWAIT_REMOTE_LOADER_BP STAGE
   // Wait for the breakpoint indicating that the remote loader has finished
   // and the x86 determiniser is ready to run. Get the address of the single step
   // procedure.
   while (trapCount == 2) {
      if (waitpid (childPid, &status, 0) != childPid) {
         err_printf (1, "AWAIT_REMOTE_LOADER_BP: waitpid");
         exit (1);
      }

      if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGTRAP) {
         // child process stopped - this should be the second stop, in RemoteLoader
         // The eax/rax register should contain the address of CommStruct in the child process
         memset (&context, 0, sizeof (struct user_regs_struct));

         if (ptrace (PTRACE_GETREGS, childPid, NULL, &context) != 0) {
            err_printf (1, "AWAIT_REMOTE_LOADER_BP: PTRACE_GETREGS");
            exit (1);
         }

         // check we stopped in the right place (got the correct message after a breakpoint)
         if (IsSingleStep (childPid, &context)) {
            // There is some single-stepping at the end of the loader,
            // this is normal, let it continue
            dbg_fprintf (stderr, "AWAIT_REMOTE_LOADER_BP: single step at %p\n",
                  (void *) get_pc (&context));
            ptrace (PTRACE_CONT, childPid, NULL, NULL);
         } else {
            dbg_fprintf (stderr, "AWAIT_REMOTE_LOADER_BP: breakpoint at %p\n",
                  (void *) get_pc (&context));

            // EAX contains an error code, or 0x101 on success
            if (get_xax (&context) != COMPLETED_LOADER) {
               err_printf (get_xax (&context), "AWAIT_REMOTE_LOADER_BP");
               exit (1);
            }

            // EBX contains the address of the single step handler
            singleStepProc = get_xbx (&context);

            // Execution continues
            ptrace (PTRACE_CONT, childPid, NULL, NULL);
            trapCount = 3;
         }
      } else if (WIFSTOPPED(status)) {
         err_printf (0, "AWAIT_REMOTE_LOADER_BP: child process stopped unexpectedly (%d)", (int) (WSTOPSIG(status)));
         exit (1);

      } else if (WIFEXITED(status)) {
         err_printf (0, "AWAIT_REMOTE_LOADER_BP: child process exited immediately");
         exit (1);
      }
   }

   while (1) {
      // RUNNING STATE
      // Child process runs within x86 determiniser.
      // Waiting for a single-step event, or exit.

      run = 1;
      while (run) {
         if (waitpid (childPid, &status, 0) != childPid) {
            err_printf (1, "RUNNING: waitpid");
            exit (1);
         }

         if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGTRAP) {
            // Stopped by single-step or stopped by breakpoint?
            memset (&context, 0, sizeof (struct user_regs_struct));

            if (ptrace (PTRACE_GETREGS, childPid, NULL, &context) != 0) {
               err_printf (1, "RUNNING: PTRACE_GETREGS: single step");
               exit (1);
            }

            if (IsSingleStep (childPid, &context)) {
               // Reached single step; run single step handler.
               if (pcs->debugEnabled) {
                  dbg_fprintf
                    (stderr, "RUNNING: Single step at %p, go to handler at %p\n", 
                     (void *) get_pc (&context), (void *) singleStepProc);
               }
               run = 0;
               memcpy (&enterSSContext, &context, sizeof (struct user_regs_struct));
               StartSingleStepProc
                 (singleStepProc,
                  childPid,
                  &context);
               if (ptrace (PTRACE_SETREGS, childPid, NULL, &context) != 0) {
                  err_printf (1, "RUNNING: PTRACE_SETREGS: single step");
                  exit (1);
               }
               // continue to next event (from determiniser)
               ptrace (PTRACE_CONT, childPid, NULL, NULL);
            } else {
               // A breakpoint: ignore it
               ptrace (PTRACE_CONT, childPid, NULL, NULL);
            }

         } else if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGSEGV) {
            // Stopped by segfault - is this caused by re-entering the text section
            // after running library code?
            memset (&context, 0, sizeof (struct user_regs_struct));
            memset (&siginfo, 0, sizeof (siginfo_t));

            if (ptrace (PTRACE_GETREGS, childPid, NULL, &context) != 0) {
               err_printf (1, "RUNNING: PTRACE_GETREGS: segv");
               exit (1);
            }
            if (ptrace (PTRACE_GETSIGINFO, childPid, NULL, &siginfo) != 0) {
               err_printf (1, "RUNNING: PTRACE_GETSIGINFO: segv");
               exit (1);
            }
            if (((uintptr_t) siginfo.si_addr >= pcs->minAddress)
            && ((uintptr_t) siginfo.si_addr < pcs->maxAddress)) {
               // The segfault should have been caused by jumping to an address in the
               // program .text section which has been marked non-executable
               if (pcs->debugEnabled) {
                  dbg_fprintf
                    (stderr, "RUNNING: Re-enter text section at %p, go to handler at %p\n", 
                     (void *) get_pc (&context),
                     (void *) singleStepProc);
               }
               if (get_pc (&context) != (uintptr_t) siginfo.si_addr) {
                  err_printf (1, "RUNNING: segfault address %p is not equal to PC %p",
                              siginfo.si_addr, (void *) get_pc (&context));
                  exit (1);
               }
               run = 0;
               memcpy (&enterSSContext, &context, sizeof (struct user_regs_struct));
               StartSingleStepProc
                 (singleStepProc,
                  childPid,
                  &context);
               if (ptrace (PTRACE_SETREGS, childPid, NULL, &context) != 0) {
                  err_printf (1, "RUNNING: PTRACE_SETREGS: segv");
                  exit (1);
               }
               // eat this signal and continue to next event (from determiniser)
               ptrace (PTRACE_CONT, childPid, NULL, NULL);
           
            } else {
               // allow the signal to reach the program
               DefaultHandler (childPid, status, "RUNNING");
            }

         } else {
            DefaultHandler (childPid, status, "RUNNING");
         }
      }

      // SINGLE_STEP STATE
      // Single step handler runs within x86 determiniser.
      // Waiting for a breakpoint at the end of the handler.

      while (!run) {
         if (waitpid (childPid, &status, 0) != childPid) {
            err_printf (1, "SINGLE_STEP: waitpid");
            exit (1);
         }

         if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGTRAP) {
            // single step procedure finished
            struct user_regs_struct c2;
            memset (&context, 0, sizeof (struct user_regs_struct));

            if (ptrace (PTRACE_GETREGS, childPid, NULL, &context) != 0) {
               err_printf (1, "SINGLE_STEP: PTRACE_GETREGS");
               exit (1);
            }
            memcpy (&c2, &context, sizeof (struct user_regs_struct));

            if (IsSingleStep (childPid, &context)) {
               err_printf (1, "SINGLE_STEP: single-stepping in single step handler?");
               exit (1);
            }

            // EAX contains an error code, or 0x102 on success
            // EBX is pointer to context, altered by remote
            if (get_xax (&context) != COMPLETED_SINGLE_STEP_HANDLER) {
               err_printf (get_xax (&context), "SINGLE_STEP");
               exit (1);
            }

            // context restored
            dbg_fprintf (stderr, "SINGLE_STEP: location of context: %p\n", (void *) get_xbx (&context));
            GetData (childPid, get_xbx (&context), (void *) &context, sizeof (struct user_regs_struct));
            dbg_fprintf (stderr, "SINGLE_STEP: flags word is %x\n", (unsigned) context.eflags);
            context.xss = enterSSContext.xss;
            if (ptrace (PTRACE_SETREGS, childPid, NULL, &context) != 0) {
               err_printf (1, "SINGLE_STEP: PTRACE_SETREGS");
               fprintf (stderr, "enter SS (COMPLETED_SINGLE_STEP_HANDLER)\n");
               DebugUserRegs (&enterSSContext);
               fprintf (stderr, "before GetData (COMPLETED_SINGLE_STEP_HANDLER)\n");
               DebugUserRegs (&c2);
               fprintf (stderr, "after GetData (COMPLETED_SINGLE_STEP_HANDLER)\n");
               DebugUserRegs (&context);
               exit (1);
            }
            ptrace (PTRACE_CONT, childPid, NULL, NULL);
            run = 1;

         } else {
            DefaultHandler (childPid, status, "SINGLE_STEP");
         }
      }
   }
}

