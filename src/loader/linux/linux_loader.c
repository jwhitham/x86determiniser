#define _XOPEN_SOURCE 600
#define _GNU_SOURCE

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
#include "linux_context.h"

#define USER_ERROR      1
#define INTERNAL_ERROR  2

#define dbg_fprintf if (pcs->debugEnabled) fprintf

typedef struct SingleStepStruct {
   void * unused;
   LINUX_CONTEXT * pcontext;
   LINUX_CONTEXT context;
} SingleStepStruct;

static void err_printf (unsigned err_code, const char * fmt, ...)
{
   int err = errno;
   va_list ap;

   va_start (ap, fmt);
   vfprintf (stderr, fmt, ap);
   va_end (ap);

   if (err_code == 1) {
      char * buf = strerror (err);
      fprintf (stderr, ": Linux error code 0x%x: %s\n", (unsigned) err, buf);

   } else if ((err_code >= X86D_FIRST_ERROR) && (err_code <= X86D_LAST_ERROR)) {
      const char * buf = X86Error (err_code);
      fprintf (stderr, ": code 0x%x: %s\n", err_code - X86D_FIRST_ERROR, buf);
   } else {
      fprintf (stderr, "\n");
   }
}

static uintptr_t get_stack_ptr (LINUX_CONTEXT * context)
{
#ifdef LINUX64
   return context->regs.rsp;
#else
   return context->regs.esp;
#endif
}

static void set_stack_ptr (LINUX_CONTEXT * context, uintptr_t sp)
{
#ifdef LINUX64
   context->regs.rsp = sp;
#else
   context->regs.esp = sp;
#endif
}

static uintptr_t get_xax (LINUX_CONTEXT * context)
{
#ifdef LINUX64
   return context->regs.rax;
#else
   return context->regs.eax;
#endif
}

static uintptr_t get_xbx (LINUX_CONTEXT * context)
{
#ifdef LINUX64
   return context->regs.rbx;
#else
   return context->regs.ebx;
#endif
}

static uintptr_t get_pc (LINUX_CONTEXT * context)
{
#ifdef LINUX64
   return context->regs.rip;
#else
   return context->regs.eip;
#endif
}

static void set_pc (LINUX_CONTEXT * context, uintptr_t pc)
{
#ifdef LINUX64
   context->regs.rip = pc;
#else
   context->regs.eip = pc;
#endif
}

static void dbg_state(LINUX_CONTEXT * context)
{
   fprintf (stderr, "xpc = %p\n", (void *) get_pc (context));
   fprintf (stderr, "xax = %p\n", (void *) get_xax (context));
   fprintf (stderr, "xbx = %p\n", (void *) get_xbx (context));
}

static void clear_single_step_flag (LINUX_CONTEXT * context)
{
   context->regs.eflags &= ~SINGLE_STEP_FLAG;
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

static int IsSingleStep (pid_t childPid, LINUX_CONTEXT * context)
{
   // https://sourceware.org/gdb/wiki/LinuxKernelWishList
   // "It would be useful for the kernel to tell us whether a SIGTRAP corresponds to a
   //  breakpoint hit or a finished single-step (or something else), without having to
   //  resort to heuristics. Checking for the existence of an int3 in memory at PC-1
   //  is not a complete solution, as the breakpoint might be gone already...
   //  Some archs started encoding trap discrimination on SIGTRAPs si_code..."

   uint8_t previous_byte[1];

   if ((get_xax (context) >= X86D_FIRST_ERROR)
   && (get_xax (context) <= X86D_LAST_ERROR)) {
      // May be a breakpoint
      GetData (childPid, get_pc (context) - 1, previous_byte, 1);
      if (previous_byte[0] == 0xcc) {
         return 0; // breakpoint
      }
   }
   return 1; // single step
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
   LINUX_CONTEXT * context)
{
   SingleStepStruct localCs;
   uintptr_t new_sp;

   memcpy (&localCs.context, context, sizeof (LINUX_CONTEXT));

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
   // Linux x64 calling convention: rdi is first parameter
   context->regs.rdi = (uintptr_t) localCs.pcontext;
#endif

   // fill remote stack
   // return address is NULL (1st item in struct: don't return!!)
   // first parameter is "pcontext" (2nd item in struct)
   PutData (childPid, get_stack_ptr(context), (const void *) &localCs, sizeof (localCs));

   // run single step handler until breakpoint
   set_pc (context, (uintptr_t) singleStepProc);
   clear_single_step_flag(context);
}

// Library is unlinked in the event of a normal exit or USER_ERROR
static void UnlinkLibrary (CommStruct * pcs)
{
   if (unlink (pcs->libraryName) != 0) {
      err_printf (1, "unable to unlink '%s'", pcs->libraryName);
   }
}

// Handle a signal or other event from the child process which is not related
// to x86determiniser's operation
static void DefaultHandler (CommStruct * pcs, pid_t childPid, int status, const char * state)
{
   if (WIFSTOPPED(status)) {
      // This means a signal was received, other than the one we are using (SIGTRAP).
      // It's normal for the user program to receive signals.
      // Pass them onwards to the program.
      ptrace (PTRACE_CONT, childPid, NULL, (void *) (uintptr_t) WSTOPSIG(status));

   } else if (WIFEXITED(status)) {
      // normal exit
      UnlinkLibrary (pcs);
      exit (WEXITSTATUS(status));

   } else if (WIFSIGNALED(status)) {
      // terminated by a signal
      switch (WTERMSIG(status)) {
         case SIGILL:
            err_printf (0, "Illegal instruction");
            UnlinkLibrary (pcs);
            exit (USER_ERROR);
            break;
         case SIGSEGV:
            err_printf (0, "Segmentation fault");
            UnlinkLibrary (pcs);
            exit (USER_ERROR);
            break;
         case SIGFPE:
            err_printf (0, "Divide by zero");
            UnlinkLibrary (pcs);
            exit (USER_ERROR);
            break;
         default:
            // pass through
            err_printf (0, "%s: terminated by signal %d\n",
                     state, WTERMSIG(status));
            UnlinkLibrary (pcs);
            exit (USER_ERROR);
      }
   } else {
      err_printf (0, "%s: waitpid returned for unknown reason: status = 0x%x\n", state, status);
      exit (INTERNAL_ERROR);
   }
}

// Test the crucial GetData and PutData functions for correctness
// by reading/writing various data sizes
static void TestGetPutData (pid_t childPid, uintptr_t base)
{
   const size_t max_size = 24;
   uint8_t outgoing[max_size];
   uint8_t incoming[max_size + 1];
   uint8_t expected[max_size];
   uint8_t current_data = 1;
   size_t offset, size, i;

   // reset all
   memset (expected, 0x00, max_size);
   memset (outgoing, 0x00, max_size);
   memset (incoming, 0x00, max_size + 1);
   PutData (childPid, base, outgoing, max_size);

   for (offset = 0; offset < 12; offset++) {
      for (size = 1; size < (max_size - offset); size++) {
         // generate test data
         memset (outgoing, 0xee, max_size);
         for (i = 0; i < size; i++) {
            outgoing[i] = current_data;
            current_data++;
         }

         // Write test
         PutData (childPid, base + offset, outgoing, size);

         // Expected effect of this write
         memcpy (&expected[offset], outgoing, size);

         // Readback 1: readback with offset
         memset (incoming, 0xaa, max_size + 1);
         GetData (childPid, base + offset, incoming, size);
         if (memcmp (incoming, outgoing, size) != 0) {
            err_printf (0, "readback 1a test failed: read something unexpected back");
            exit (INTERNAL_ERROR);
         }
         if (incoming[size] != 0xaa) {
            err_printf (0, "readback 1b test failed: GetData overwrote sentinel value");
            exit (INTERNAL_ERROR);
         }

         // Readback 2: readback without offset (check the entire expected area)
         memset (incoming, 0xaa, max_size + 1);
         GetData (childPid, base, incoming, max_size);
         if (memcmp (incoming, expected, max_size) != 0) {
            err_printf (0, "readback 2a test failed: memory area changed in an unexpected way");
            exit (INTERNAL_ERROR);
         }
         if (incoming[max_size] != 0xaa) {
            err_printf (0, "readback 2b test failed: GetData overwrote sentinel value");
            exit (INTERNAL_ERROR);
         }
      }
   }
}

// GetContext: do PTRACE_GETREGS and PTRACE_GETFPREGS
static void GetContext (pid_t childPid, LINUX_CONTEXT * context, const char * state)
{
   memset (context, 0, sizeof (LINUX_CONTEXT));
   if (ptrace (PTRACE_GETREGS, childPid, NULL, &context->regs) != 0) {
      err_printf (1, "%s: PTRACE_GETREGS", state);
      exit (INTERNAL_ERROR);
   }
   if (ptrace (PTRACE_GETFPREGS, childPid, NULL, &context->fpregs) != 0) {
      err_printf (1, "%s: PTRACE_GETFPREGS", state);
      exit (INTERNAL_ERROR);
   }
}

// PutContext: do PTRACE_SETREGS and PTRACE_SETFPREGS
static void PutContext (pid_t childPid, LINUX_CONTEXT * context, const char * state)
{
   if (ptrace (PTRACE_SETREGS, childPid, NULL, &context->regs) != 0) {
      err_printf (1, "%s: PTRACE_SETREGS", state);
      exit (INTERNAL_ERROR);
   }
   if (ptrace (PTRACE_SETFPREGS, childPid, NULL, &context->fpregs) != 0) {
      err_printf (1, "%s: PTRACE_SETFPREGS", state);
      exit (INTERNAL_ERROR);
   }
}


// Create and fill the x86determiniser shared object file
static void SetupLibrary (CommStruct * pcs)
{
   extern uint8_t _x86d__binary_determiniser_so_start[];
   extern uint8_t _x86d__binary_determiniser_so_end[];
   size_t size = ((uintptr_t) _x86d__binary_determiniser_so_end) -
                  ((uintptr_t) _x86d__binary_determiniser_so_start);
   int fd;
   const char * tmp_dir = getenv("TMPDIR");

   if (!tmp_dir) {
      tmp_dir = "/tmp";
   }
   snprintf (pcs->libraryName, MAX_FILE_NAME_SIZE, "%s/%sdeterminiser-%d-XXXXXX.so",
         tmp_dir, X86_OR_X64, (int) getpid ());
   pcs->libraryName[MAX_FILE_NAME_SIZE - 1] = '\0';

   fd = mkostemps (pcs->libraryName, 3, O_RDWR | O_CREAT); // 3 == strlen(".so")
   if (fd < 0) {
      // User should set a different TMPDIR
      err_printf (1, "Unable to create '%s'", pcs->libraryName);
      exit (USER_ERROR);
   }

   if ((ssize_t) size != write (fd, _x86d__binary_determiniser_so_start, size)) {
      err_printf (1, "Unable to fill '%s'", pcs->libraryName);
      exit (INTERNAL_ERROR);
   }
   close (fd);

   dbg_fprintf (stderr, "INITIAL: library is '%s'\n", pcs->libraryName);
}


// Entry point from main
// Args already parsed into CommStruct
int X86DeterminiserLoader(CommStruct * pcs, int argc, char ** argv)
{
   unsigned    trapCount = 0;
   pid_t       childPid = -1;
   char        binFolder[BUFSIZ];
   int         status = 0;
   FILE *      testFd = 0;
   int         run = 1;
   int         elfType = 0;
   char        elfHeader[5];
   LINUX_CONTEXT    context;
   uintptr_t   enterSSContext_xss = 0;
   uintptr_t   pcsInChild = 0;
   siginfo_t   siginfo;

   memset (&context, 0, sizeof (LINUX_CONTEXT));
   memset (&siginfo, 0, sizeof (siginfo_t));
   memset (&elfHeader, 0, sizeof (elfHeader));
   pcs->singleStepHandlerAddress = 0;

   (void) argc;
   (void) enterSSContext_xss;

   if (readlink ("/proc/self/exe", binFolder, sizeof (binFolder) - 1) > 0) {
      char * finalSlash = strrchr (binFolder, '/');
      if (!finalSlash) {
         finalSlash = binFolder;
      }
      *finalSlash = '\0';
   } else {
      err_printf (1, "readlink('" X86_OR_X64 "determiniser')");
      exit (USER_ERROR);
   }

   // check child process executable exists and has the correct format
   testFd = fopen (argv[0], "rb");
   if (!testFd) {
      err_printf (1, "program '%s' not found", argv[0]);
      exit (USER_ERROR);
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
         exit (USER_ERROR);
   }
   if (elfType != (PTR_SIZE * 8)) {
      fprintf (stderr, "%s: executable appears to be %d-bit, try x%ddeterminiser instead\n",
         argv[0],
         (PTR_SIZE == 4) ? 64 : 32,
         (PTR_SIZE == 4) ? 64 : 86);
      exit (USER_ERROR);
   }
   SetupLibrary (pcs);

   // run the subprocess, preloading the library
   setenv ("LD_PRELOAD", pcs->libraryName, 1);
   childPid = fork ();
   if (childPid < 0) {
      err_printf (1, "fork");
      exit (INTERNAL_ERROR);
   } else if (childPid == 0) {
      // Run the child process: wait for the parent to attach via ptrace
      uint32_t error = FAILED_EXEC;

      ptrace (PTRACE_TRACEME, 0, NULL, NULL);
      execv (argv[0], argv);

      // If exec'ing the child process failed for some reason, report this using FAILED_EXEC message
      __asm__ volatile ("int3" : : "a"(error));
      exit (INTERNAL_ERROR);
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
         exit (INTERNAL_ERROR);
      }

      if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGTRAP) {
         // child process stopped - this is the initial stop, after the execv call
         GetContext (childPid, &context, "INITIAL");
         dbg_fprintf (stderr, "INITIAL: entry PC = %p\n", (void *) get_pc (&context));

         // Discover the bounds of the executable .text section,
         // initially excluding the current address, which is probably in ld-linux.so.2 or an equivalent
         if (!ReadMaps (childPid, get_pc (&context), pcs)) {
            // Did not find the bounds, so maybe there is no ld-linux.so.2? Try again.
            if (!ReadMaps (childPid, 0, pcs)) {
               // Nothing can be found - give up
               err_printf (0, "INITIAL: unable to determine bounds of .text section for '%s'", argv[0]);
               exit (INTERNAL_ERROR);
            }
         }
         dbg_fprintf (stderr, "INITIAL: minAddress = %p\n", (void *) pcs->minAddress);
         dbg_fprintf (stderr, "INITIAL: maxAddress = %p\n", (void *) pcs->maxAddress);

         // continue to breakpoint
         ptrace (PTRACE_CONT, childPid, NULL, NULL);
         trapCount = 1;

      } else if (WIFSTOPPED(status)) {
         err_printf (0, "INITIAL: child process stopped unexpectedly (signal %d)",
                     (int) (WSTOPSIG(status)));
         exit (INTERNAL_ERROR);

      } else if (WIFSIGNALED(status)) {
         err_printf (0, "INITIAL: child process was unexpectedly terminated by "
                     "signal %d", (int) (WTERMSIG(status)), status);
         exit (INTERNAL_ERROR);
      } else if (WIFEXITED(status)) {
         err_printf (0, "INITIAL: child process exited immediately");
         exit (INTERNAL_ERROR);

      } else {
         err_printf (0, "INITIAL: child process did something unexpected, "
                        "status = 0x%x", status);
         exit (INTERNAL_ERROR);
      }
   }

   // AWAIT_FIRST_STAGE STAGE
   // Wait for the breakpoint indicating that the RemoteLoader procedure is executing.
   while (trapCount == 1) {
      if (waitpid (childPid, &status, 0) != childPid) {
         err_printf (1, "AWAIT_FIRST_STAGE: waitpid");
         exit (INTERNAL_ERROR);
      }

      if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGTRAP) {
         // child process stopped - this should be the second stop, in RemoteLoader
         // The eax/rax register should contain the address of CommStruct in the child process
         CommStruct check_copy;

         GetContext (childPid, &context, "AWAIT_FIRST_STAGE");

         dbg_fprintf (stderr, "AWAIT_FIRST_STAGE: breakpoint at %p\n", (void *) get_pc (&context));

         // check we stopped in the right place: EAX/RAX contains the expected code
         if (get_xax (&context) != COMPLETED_REMOTE) {
            err_printf (0, "AWAIT_FIRST_STAGE: breakpoint was not in RemoteLoader procedure");
            exit (INTERNAL_ERROR);
         }

         // copy CommStruct data to the child: EBX/RBX contains the address
         pcsInChild = get_xbx (&context);
         dbg_fprintf (stderr, "AWAIT_FIRST_STAGE: pcsInChild = %p\n", (void *) pcsInChild);
         if (pcs->debugEnabled) {
            dbg_fprintf (stderr, "AWAIT_FIRST_STAGE: running TestGetPutData\n");
            TestGetPutData (childPid, pcsInChild);
         }
         PutData (childPid, pcsInChild, (const void *) pcs, sizeof (CommStruct));

         // check for correct data transfer
         memset (&check_copy, 0xaa, sizeof (CommStruct));
         GetData (childPid, pcsInChild, (void *) &check_copy, sizeof (CommStruct));
         if (memcmp (&check_copy, pcs, sizeof (CommStruct) != 0)) {
            err_printf (0, "AWAIT_FIRST_STAGE: readback of CommStruct failed");
            exit (INTERNAL_ERROR);
         }
         dbg_fprintf (stderr, "AWAIT_FIRST_STAGE: readback ok (%d bytes)\n", (int) sizeof (CommStruct));

         // continue to next event (from determiniser)
         ptrace (PTRACE_CONT, childPid, NULL, NULL);
         trapCount = 2;

      } else if (WIFSTOPPED(status)) {
         err_printf (0, "AWAIT_FIRST_STAGE: child process stopped unexpectedly (%d)", (int) (WSTOPSIG(status)));
         exit (INTERNAL_ERROR);

      } else if (WIFEXITED(status)) {
         err_printf (0, "AWAIT_FIRST_STAGE: child process exited immediately");
         exit (INTERNAL_ERROR);
      }
   }

   // AWAIT_REMOTE_LOADER_BP STAGE
   // Wait for the breakpoint indicating that the remote loader has finished
   // and the x86 determiniser is ready to run. Get the address of the single step
   // procedure.
   while (trapCount == 2) {
      if (waitpid (childPid, &status, 0) != childPid) {
         err_printf (1, "AWAIT_REMOTE_LOADER_BP: waitpid");
         exit (INTERNAL_ERROR);
      }

      if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGTRAP) {
         // child process stopped - this should be the second stop, in RemoteLoader
         // The eax/rax register should contain the address of CommStruct in the child process
         GetContext (childPid, &context, "AWAIT_REMOTE_LOADER_BP");

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
            if (get_xax (&context) == COMPLETED_LOADER) {
               // EBX confirms location of CommStruct in child process
               dbg_fprintf (stderr, "AWAIT_REMOTE_LOADER_BP: completed\n");
               if (get_xbx (&context) != pcsInChild) {
                  err_printf (0, "AWAIT_REMOTE_LOADER_BP: unexpected EBX value");
                  exit (INTERNAL_ERROR);
               }

               // read back the CommStruct, now updated with singleStepHandlerAddress
               GetData (childPid, pcsInChild, pcs, sizeof (CommStruct));

               // Execution continues
               ptrace (PTRACE_CONT, childPid, NULL, NULL);
               trapCount = 3;
            } else if ((get_xax (&context) >= X86D_FIRST_ERROR)
            && (get_xax (&context) <= X86D_LAST_ERROR)) {
               // User error reported at lower level
               err_printf (get_xax (&context), "AWAIT_REMOTE_LOADER");
               UnlinkLibrary (pcs);
               exit (USER_ERROR);
            } else {
               // Breakpoint for some other reason
               if (pcs->debugEnabled) {
                  dbg_fprintf (stderr, "AWAIT_REMOTE_LOADER_BP: other bp\n");
                  dbg_state (&context);
               }
               DefaultHandler (pcs, childPid, status, "AWAIT_REMOTE_LOADER_BP");
            }
         }
      } else if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGSEGV) {
         // Stopped by segfault (unexpectedly)
         memset (&siginfo, 0, sizeof (siginfo_t));

         GetContext (childPid, &context, "AWAIT_REMOTE_LOADER_BP: segv");
         if (ptrace (PTRACE_GETSIGINFO, childPid, NULL, &siginfo) != 0) {
            err_printf (1, "AWAIT_REMOTE_LOADER_BP: PTRACE_GETSIGINFO: segv");
            exit (INTERNAL_ERROR);
         }
         fprintf (stderr, "AWAIT_REMOTE_LOADER_BP: segfault at PC 0x%p "
               "fault address %p\n",
                     (void *) get_pc (&context),
                     (void *) siginfo.si_addr);
         ReadMaps (childPid, 0, pcs);
         exit (INTERNAL_ERROR);

      } else if (WIFSTOPPED(status)) {
         GetContext (childPid, &context, "AWAIT_REMOTE_LOADER_BP");
         err_printf (0, "AWAIT_REMOTE_LOADER_BP: child process stopped unexpectedly (signal %d)", (int) (WSTOPSIG(status)));
         exit (INTERNAL_ERROR);

      } else if (WIFEXITED(status)) {
         err_printf (0, "AWAIT_REMOTE_LOADER_BP: child process exited immediately");
         exit (INTERNAL_ERROR);
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
            exit (INTERNAL_ERROR);
         }

         if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGTRAP) {
            // Stopped by single-step or stopped by breakpoint?
            GetContext (childPid, &context, "RUNNING: single_step");

            if (IsSingleStep (childPid, &context)) {
               // Reached single step; run single step handler.
               if (pcs->debugEnabled) {
                  dbg_fprintf
                    (stderr, "RUNNING: Single step at %p, go to handler at %p\n",
                     (void *) get_pc (&context), (void *) pcs->singleStepHandlerAddress);
               }
               run = 0;
#ifdef LINUX32
               enterSSContext_xss = context.regs.xss;
#endif
               StartSingleStepProc
                 (pcs->singleStepHandlerAddress,
                  childPid,
                  &context);
               PutContext (childPid, &context, "RUNNING: single step");
               // continue to next event (from determiniser)
               ptrace (PTRACE_CONT, childPid, NULL, NULL);
            } else {
               // Breakpoint for some other reason
               // continue to next event (from determiniser)
               ptrace (PTRACE_CONT, childPid, NULL, NULL);
            }

         } else if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGSEGV) {
            // Stopped by segfault - is this caused by re-entering the text section
            // after running library code?
            memset (&siginfo, 0, sizeof (siginfo_t));

            GetContext (childPid, &context, "RUNNING: segv");
            if (ptrace (PTRACE_GETSIGINFO, childPid, NULL, &siginfo) != 0) {
               err_printf (1, "RUNNING: PTRACE_GETSIGINFO: segv");
               exit (INTERNAL_ERROR);
            }
            if (((uintptr_t) siginfo.si_addr >= pcs->minAddress)
            && ((uintptr_t) siginfo.si_addr < pcs->maxAddress)
            && (get_pc (&context) == (uintptr_t) siginfo.si_addr)) {
               // The segfault should have been caused by jumping to an address in the
               // program .text section which has been marked non-executable
               if (pcs->debugEnabled) {
                  dbg_fprintf
                    (stderr, "RUNNING: Re-enter text section at %p, go to handler at %p\n",
                     (void *) get_pc (&context),
                     (void *) pcs->singleStepHandlerAddress);
               }
               run = 0;
#ifdef LINUX32
               enterSSContext_xss = context.regs.xss;
#endif
               StartSingleStepProc
                 (pcs->singleStepHandlerAddress,
                  childPid,
                  &context);
               PutContext (childPid, &context, "RUNNING: segv");

               // eat this signal and continue to next event (from determiniser)
               ptrace (PTRACE_CONT, childPid, NULL, NULL);

            } else {
               // allow the signal to reach the program
               DefaultHandler (pcs, childPid, status, "RUNNING");
            }

         } else {
            DefaultHandler (pcs, childPid, status, "RUNNING");
         }
      }

      // SINGLE_STEP STATE
      // Single step handler runs within x86 determiniser.
      // Waiting for a breakpoint at the end of the handler.

      while (!run) {
         if (waitpid (childPid, &status, 0) != childPid) {
            err_printf (1, "SINGLE_STEP: waitpid");
            exit (INTERNAL_ERROR);
         }

         if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGTRAP) {
            // single step procedure finished
            GetContext (childPid, &context, "SINGLE_STEP");

            if (IsSingleStep (childPid, &context)) {
               err_printf (1, "SINGLE_STEP: single-stepping in single step handler?");
               exit (INTERNAL_ERROR);
            }

            // EAX contains an error code, or 0x102 on success
            // EBX is pointer to context, altered by remote
            if (get_xax (&context) != COMPLETED_SINGLE_STEP_HANDLER) {
               err_printf (get_xax (&context), "SINGLE_STEP");
               exit (INTERNAL_ERROR);
            }

            // context restored
            dbg_fprintf (stderr, "SINGLE_STEP: location of context: %p\n", (void *) get_xbx (&context));
            GetData (childPid, get_xbx (&context), (void *) &context, sizeof (LINUX_CONTEXT));
            dbg_fprintf (stderr, "SINGLE_STEP: flags word is %x\n", (unsigned) context.regs.eflags);

            // this register appears difficult to restore in user space, but we have a copy
            // of the expected value
#ifdef LINUX32
            context.regs.xss = enterSSContext_xss;
#endif

            PutContext (childPid, &context, "SINGLE_STEP");
            ptrace (PTRACE_CONT, childPid, NULL, NULL);
            run = 1;

         } else {
            DefaultHandler (pcs, childPid, status, "SINGLE_STEP");
         }
      }
   }
}

