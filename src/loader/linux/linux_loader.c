#define _XOPEN_SOURCE 600

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>

#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/signal.h>
#include <sys/stat.h>

#include "remote_loader.h"
#include "x86_flags.h"
#include "common.h"

#define dbg_printf if (pcs->debugEnabled) printf

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

/*static void err_wrong_architecture (const char * exe)
{
   fprintf (stderr, "%s: executable appears to be %d-bit, try x%ddeterminiser instead\n",
      exe,
      PTR_SIZE * 8,
      (PTR_SIZE == 4) ? 86 : 64);
} */

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

/*
static void set_single_step_flag (struct user_regs_struct * context)
{
   context->eflags |= SINGLE_STEP_FLAG;
}
*/

static void clear_single_step_flag (struct user_regs_struct * context)
{
   context->eflags &= ~SINGLE_STEP_FLAG;
}

static int is_single_step (struct user_regs_struct * context)
{
   return !!(context->eflags & SINGLE_STEP_FLAG);
}

static uintptr_t align_64 (uintptr_t v)
{
   return v & ~63;
}

// parent receives data from child
static void getdata
  (pid_t childPid,
   uintptr_t childAddress,
   char * parentAddress,
   size_t size)
{
   // copy full words
   while (size >= sizeof (uintptr_t)) {
      *((uintptr_t *) parentAddress) = ptrace (PTRACE_PEEKDATA, childPid, childAddress, NULL);
      parentAddress += sizeof (uintptr_t);
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
      memcpy (parentAddress, &data.bytes, size);
   }
}

// parent sends data to child
static void putdata
  (pid_t childPid,
   uintptr_t childAddress,
   const char * parentAddress,
   size_t size)
{
   // copy full words
   while (size >= sizeof (uintptr_t)) {
      ptrace (PTRACE_POKEDATA, childPid, childAddress, *((uintptr_t *) parentAddress));
      parentAddress += sizeof (uintptr_t);
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
      memcpy (&data.bytes, parentAddress, size);
      ptrace (PTRACE_POKEDATA, childPid, childAddress, data.word);
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
   putdata (childPid, get_stack_ptr(context), (const void *) &localCs, sizeof (localCs));

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
   int         testFd = 0;
   int         run = 1;

   (void) argc;

   if (readlink ("/proc/self/exe", binFolder, sizeof (binFolder) - 1) > 0) {
      char * finalSlash = strrchr (binFolder, '/');
      if (!finalSlash) {
         finalSlash = binFolder;
      }
      *finalSlash = '\0';
   } else {
      err_printf (1, "readlink('" X86_OR_X64 "determiniser')");
      return 1;
   }

   // library name
   snprintf (pcs->libraryName, MAX_FILE_NAME_SIZE, "%s/%sdeterminiser.so",
      binFolder, X86_OR_X64);

   // check library exists
   testFd = open (pcs->libraryName, O_RDONLY);
   if (!testFd) {
      err_printf (1, "open('%s')", pcs->libraryName);
      return 1;
   }
   close (testFd);
  
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

         // TODO check that the process is 32-bit or 64-bit, matching expectations

         // capture entry point for the program: this tells us where the .text segment is
         if (ptrace (PTRACE_GETREGS, childPid, NULL, &context) != 0) {
            err_printf (1, "AWAIT_FIRST_STAGE: PTRACE_GETREGS");
         }
         pcs->startAddress = get_pc (&context);

         // continue to breakpoint
         ptrace (PTRACE_CONT, childPid, NULL, NULL);
         trapCount = 1;

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
         char buf[12];
         uintptr_t remoteLoaderPC;
         uintptr_t remoteLoaderCS;

         if (ptrace (PTRACE_GETREGS, childPid, NULL, &context) != 0) {
            err_printf (1, "AWAIT_FIRST_STAGE: PTRACE_GETREGS");
         }

         // check we stopped in the right place (look for magic string in executable code)
         remoteLoaderPC = get_pc (&context);
         getdata (childPid, remoteLoaderPC + 3, buf, sizeof (buf));
         if (memcmp (buf, "RemoteLoader", sizeof (buf)) != 0) {
            err_printf (0, "AWAIT_FIRST_STAGE: breakpoint was not in RemoteLoader procedure");
            exit (1);
         }

         // copy CommStruct data to the child: eax/rax contains the address
         remoteLoaderCS = get_xax (&context);
         putdata (childPid, remoteLoaderCS, (const void *) pcs, sizeof (CommStruct));

         // continue to next event (from determiniser)
         ptrace (PTRACE_CONT, childPid, NULL, NULL);
         trapCount = 2;
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
         }

         // check we stopped in the right place (look for magic string in executable code)
         if (is_single_step (&context)) {
            // There is some single-stepping at the end of the loader,
            // this is normal, let it continue
            dbg_printf ("AWAIT_REMOTE_LOADER_BP: single step at %p\n",
                  (void *) get_pc (&context));
            ptrace (PTRACE_CONT, childPid, NULL, NULL);
         } else {
            dbg_printf ("AWAIT_REMOTE_LOADER_BP: breakpoint at %p\n",
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
            // TODO how do I tell?
            struct user_regs_struct context;

            if (ptrace (PTRACE_GETREGS, childPid, NULL, &context) != 0) {
               err_printf (1, "RUNNING: PTRACE_GETREGS");
            }

            if (is_single_step (&context)) {
               // Reached single step; run single step handler.
               if (pcs->debugEnabled) {
                  dbg_printf
                    ("RUNNING: Single step at %p, go to handler at %p\n", 
                     (void *) get_pc (&context), (void *) singleStepProc);
                  fflush (stdout);
               }
               run = 0;
               StartSingleStepProc
                 (singleStepProc,
                  childPid,
                  &context);
               if (ptrace (PTRACE_SETREGS, childPid, NULL, &context) != 0) {
                  err_printf (1, "RUNNING: PTRACE_SETREGS");
               }
            }
            // continue to next event (from determiniser)
            ptrace (PTRACE_CONT, childPid, NULL, NULL);

         } else if (WIFEXITED(status)) {
            // normal exit
            exit (WEXITSTATUS(status));
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
            }

            if (is_single_step (&context)) {
               err_printf (1, "SINGLE_STEP: single-stepping in single step handler?");
            }

            // EAX contains an error code, or 0x102 on success
            // EBX is pointer to context, altered by remote
            if (get_xax (&context) != COMPLETED_SINGLE_STEP_HANDLER) {
               err_printf (get_xax (&context), "SINGLE_STEP");
               return 1;
            }

            // context restored
            dbg_printf ("SINGLE_STEP: location of context: %p\n", (void *) get_xbx (&context));
            getdata (childPid, get_xbx (&context), (void *) &context, sizeof (struct user_regs_struct));
            dbg_printf ("SINGLE_STEP: flags word is %x\n", (unsigned) context.eflags);
            if (ptrace (PTRACE_SETREGS, childPid, NULL, &context) != 0) {
               err_printf (1, "RUNNING: PTRACE_SETREGS");
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
   return 0;
}

