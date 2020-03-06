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
   set_pc(context, (uintptr_t) singleStepProc);
   clear_single_step_flag(context);
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
   if (fread (elfHeader, 1, sizeof (elfHeader), testFd) != sizeof (elfHeader)) {
      err_printf (1, "program '%s' not readable", argv[0]);
      exit (1);
   }
   fclose (testFd);
   dbg_fprintf (stderr, "INITIAL: program is '%s'\n", argv[0]);

   if (memcmp (elfHeader, "\x7f" "ELF", 4) != 0) {
      err_printf (0, "program '%s' must be an ELF executable", argv[0]);
      exit (1);
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
         struct user_regs_struct context;

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
         struct user_regs_struct context;
         uintptr_t remoteLoaderCS;
         CommStruct check_copy;

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
         struct user_regs_struct context;

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
            struct user_regs_struct context;

            if (ptrace (PTRACE_GETREGS, childPid, NULL, &context) != 0) {
               err_printf (1, "RUNNING: PTRACE_GETREGS");
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
               StartSingleStepProc
                 (singleStepProc,
                  childPid,
                  &context);
               if (ptrace (PTRACE_SETREGS, childPid, NULL, &context) != 0) {
                  err_printf (1, "RUNNING: PTRACE_SETREGS");
                  exit (1);
               }
            }
            // continue to next event (from determiniser)
            ptrace (PTRACE_CONT, childPid, NULL, NULL);

         } else if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGSEGV) {
            // Stopped by segfault - is this caused by re-entering the text section
            // after running library code?
            struct user_regs_struct context;
            siginfo_t siginfo;

            if (ptrace (PTRACE_GETREGS, childPid, NULL, &context) != 0) {
               err_printf (1, "RUNNING: PTRACE_GETREGS (2)");
               exit (1);
            }
            if (ptrace (PTRACE_GETSIGINFO, childPid, NULL, &siginfo) != 0) {
               err_printf (1, "RUNNING: PTRACE_GETSIGINFO");
               exit (1);
            }
            if (((uintptr_t) siginfo.si_addr >= pcs->minAddress)
            && ((uintptr_t) siginfo.si_addr < pcs->maxAddress)) {
               // returned to text
               if (pcs->debugEnabled) {
                  dbg_fprintf
                    (stderr, "RUNNING: Re-enter text section at %p, go to handler at %p\n", 
                     (void *) get_pc (&context), (void *) singleStepProc);
               }
               run = 0;
               StartSingleStepProc
                 (singleStepProc,
                  childPid,
                  &context);
               if (ptrace (PTRACE_SETREGS, childPid, NULL, &context) != 0) {
                  err_printf (1, "RUNNING: PTRACE_SETREGS (2)");
                  exit (1);
               }
            } else {
               err_printf (0, "RUNNING: seg fault for some reason? addr %p PC %p code %d\n",
                        siginfo.si_addr, (void *) get_pc (&context), siginfo.si_code);
               exit (1);
            }

            // continue to next event (from determiniser)
            ptrace (PTRACE_CONT, childPid, NULL, NULL);
            
         } else if (WIFSTOPPED(status)) {
            err_printf (0, "RUNNING: child process stopped unexpectedly (%d)", (int) (WSTOPSIG(status)));
            exit (1);

         } else if (WIFEXITED(status)) {
            // normal exit
            exit (WEXITSTATUS(status));
         } else {
            err_printf (0, "RUNNING: child process waitpid?");
            exit (1);
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
            struct user_regs_struct context;

            if (ptrace (PTRACE_GETREGS, childPid, NULL, &context) != 0) {
               err_printf (1, "SINGLE_STEP: PTRACE_GETREGS");
               exit (1);
            }

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
            if (ptrace (PTRACE_SETREGS, childPid, NULL, &context) != 0) {
               err_printf (1, "RUNNING: PTRACE_SETREGS");
               exit (1);
            }
            ptrace (PTRACE_CONT, childPid, NULL, NULL);
            run = 1;
         } else if (WIFEXITED(status)) {
            err_printf (1, "SINGLE_STEP: exit in single step handler?");
            // normal exit
            exit (WEXITSTATUS(status));
         }
      }
   }
}

