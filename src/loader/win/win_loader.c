#define _WIN32_WINNT 0x600
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <Windows.h>
#include <psapi.h>
#include <shlwapi.h>
#include <stdint.h>

#include "remote_loader.h"
#include "x86_flags.h"
#include "common.h"

int _CRT_glob = 0; /* don't expand wildcards when parsing command-line args */

#define USER_ERROR      1
#define INTERNAL_ERROR  2

#define dbg_fprintf if (pcs->debugEnabled) fprintf

static DWORD DefaultHandler (
      CommStruct * pcs,
      const char * state, DEBUG_EVENT * pDebugEvent,
      PROCESS_INFORMATION * pProcessInformation);

typedef struct DetermineOffsetsStruct {
   uintptr_t getProcAddressOffset;
   uintptr_t loadLibraryOffset;
   uintptr_t getLastErrorOffset;
   LPVOID kernel32Base;
   CommStruct * pcs;
   char * argv0;
} DetermineOffsetsStruct;

typedef struct SingleStepStruct {
   void * unused;
   PCONTEXT pcontext;
   CONTEXT context;
} SingleStepStruct;

static void err_printf (unsigned err_code, const char * fmt, ...)
{
   DWORD err = GetLastError ();
   va_list ap;

   va_start (ap, fmt);
   vfprintf (stderr, fmt, ap);
   va_end (ap);

   if (err_code == 1) {
      char * buf = NULL;
      FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER |
                        FORMAT_MESSAGE_FROM_SYSTEM |
                        FORMAT_MESSAGE_IGNORE_INSERTS,
                     NULL, err, 0, (char *) &buf, 0, NULL);
      fprintf (stderr, ": Windows error code 0x%x: %s", (unsigned) err, buf ? buf : "??");

   } else if ((err_code >= X86D_FIRST_ERROR) && (err_code <= X86D_LAST_ERROR)) {
      const char * buf = X86Error (err_code);
      fprintf (stderr, ": code 0x%x: %s\n", err_code - X86D_FIRST_ERROR, buf);
   } else {
      fprintf (stderr, "\n");
   }
}

static void err_wrong_architecture (const char * exe)
{
   fprintf (stderr, "%s: executable appears to be %d-bit, try x%ddeterminiser instead\n",
      exe,
      (PTR_SIZE == 4) ? 64 : 32,
      (PTR_SIZE == 4) ? 64 : 86);
}

static uintptr_t get_stack_ptr (PCONTEXT context)
{
#ifdef WIN64
   return context->Rsp;
#else
   return context->Esp;
#endif
}

static void set_stack_ptr (PCONTEXT context, uintptr_t sp)
{
#ifdef WIN64
   context->Rsp = sp;
#else
   context->Esp = sp;
#endif
}

static uintptr_t get_xax (PCONTEXT context)
{
#ifdef WIN64
   return context->Rax;
#else
   return context->Eax;
#endif
}

static uintptr_t get_xbx (PCONTEXT context)
{
#ifdef WIN64
   return context->Rbx;
#else
   return context->Ebx;
#endif
}

static uintptr_t get_xsp (PCONTEXT context)
{
#ifdef WIN64
   return context->Rsp;
#else
   return context->Esp;
#endif
}

static uintptr_t get_pc (PCONTEXT context)
{
#ifdef WIN64
   return context->Rip;
#else
   return context->Eip;
#endif
}

static void set_pc (PCONTEXT context, uintptr_t pc)
{
#ifdef WIN64
   context->Rip = pc;
#else
   context->Eip = pc;
#endif
}

static void dbg_state(PCONTEXT context)
{
   uint8_t * c;
   fprintf (stderr, "xpc = %p\n", (void *) get_pc (context));
   fprintf (stderr, "xax = %p\n", (void *) get_xax (context));
   fprintf (stderr, "xbx = %p\n", (void *) get_xbx (context));
   fprintf (stderr, "xsp = %p\n", (void *) get_xsp (context));

   c = (uint8_t *) get_pc(context);
   fprintf (stderr, "pc = %02x %02x %02x %02x\n",
      c[0], c[1], c[2], c[3]);
}

static void set_single_step_flag (PCONTEXT context)
{
   context->EFlags |= SINGLE_STEP_FLAG;
}

static void clear_single_step_flag (PCONTEXT context)
{
   context->EFlags &= ~SINGLE_STEP_FLAG;
}

static uintptr_t align_64 (uintptr_t v)
{
   return v & ~63;
}


static void StartSingleStepProc
  (void * singleStepHandler,
   HANDLE hProcess,
   PCONTEXT context)
{
   SingleStepStruct localCs;
   uintptr_t new_sp;

   memcpy (&localCs.context, context, sizeof (CONTEXT));

   // reserve stack space for SingleStepStruct
   new_sp = get_stack_ptr(context) - sizeof (localCs) - RED_ZONE_SIZE;
   new_sp = align_64 (new_sp);
   set_stack_ptr(context, new_sp);

   // generate pointer to context within localCs
   // This will be the first parameter
   localCs.pcontext = (void *) (new_sp +
      ((uintptr_t) &localCs.context - (uintptr_t) &localCs));
   localCs.unused = NULL;

#ifdef IS_64_BIT
   context->Rcx = (uintptr_t) localCs.pcontext;
#endif

   // fill remote stack
   // return address is NULL (1st item in struct: don't return!!)
   // first parameter is "pcontext" (2nd item in struct)
   WriteProcessMemory
     (hProcess,
      (void *) get_stack_ptr(context),
      &localCs,
      sizeof (localCs),
      NULL);

   // run single step handler until breakpoint
   set_pc(context, (uintptr_t) singleStepHandler);
   clear_single_step_flag(context);
}

void StartRemoteLoader
  (CommStruct * pcs,
   DetermineOffsetsStruct * dos,
   uintptr_t startAddress,
   HANDLE hProcess,
   PCONTEXT context)
{
   ssize_t space;
   void * remoteBuf;
   uintptr_t new_sp;
   BOOL rc;

   space = (char *) RemoteLoaderEnd - (char *) RemoteLoaderStart;
   if (space <= 0) {
      err_printf (0, "REMOTE_LOADER: No valid size for remote loader");
      exit (INTERNAL_ERROR);
   }

   // allocates space in debugged process for executable code
   // RemoteLoaderStart .. RemoteLoaderEnd
   remoteBuf = VirtualAllocEx
     (hProcess,
      NULL,
      (DWORD) space,
      MEM_RESERVE | MEM_COMMIT,
      PAGE_EXECUTE_READWRITE);
   if (!remoteBuf) {
      err_printf (0, "REMOTE_LOADER: Unable to allocate %d bytes for remote loader", (int) space);
      exit (INTERNAL_ERROR);
   }

   // writes remote loader into memory within debugged process
   rc = WriteProcessMemory
     (hProcess,
      remoteBuf,
      RemoteLoaderStart,
      (DWORD) space,
      NULL);
   if (!rc) {
      err_printf (0, "REMOTE_LOADER: Unable to inject %d bytes", (int) space);
      exit (INTERNAL_ERROR);
   }

   // reserve the right amount of stack space
   // Round down to 64 byte boundary, then add bytes for the return address,
   // to avoid alignment violations if the loader uses instructions
   // such as movaps.
   new_sp = get_stack_ptr(context) - sizeof (CommStruct);
   new_sp -= sizeof (void *);
   new_sp = align_64 (new_sp);
   new_sp += sizeof (void *);
   set_stack_ptr(context, new_sp);

   // build data structure to load into the remote stack
   pcs->myself = (void *) new_sp;
   strncpy (pcs->procName, "X86DeterminiserStartup", MAX_PROC_NAME_SIZE);
   pcs->loadLibraryProc =
      (void *) ((char *) dos->kernel32Base + dos->loadLibraryOffset);
   pcs->getProcAddressProc =
      (void *) ((char *) dos->kernel32Base + dos->getProcAddressOffset);
   pcs->getLastErrorProc =
      (void *) ((char *) dos->kernel32Base + dos->getLastErrorOffset);
   pcs->startAddress = startAddress;

   // fill remote stack
   // return address is NULL (1st item in struct: don't return!!)
   // first parameter is "myself" (2nd item in struct)
   WriteProcessMemory
     (hProcess,
      pcs->myself,
      pcs,
      sizeof (CommStruct),
      NULL);
   if (!rc) {
      err_printf (0, "REMOTE_LOADER: Unable to fill remote stack");
      exit (INTERNAL_ERROR);
   }

#ifdef IS_64_BIT
   // first parameter is "myself"
   context->Rcx = (uintptr_t) pcs->myself;
#endif

   // run RemoteLoader procedure until breakpoint
   set_pc(context, (uintptr_t) ((void *) ((char *) remoteBuf +
         ((char *) RemoteLoader - (char *) RemoteLoaderStart))));
   clear_single_step_flag(context);
}


// Based on
// https://blogs.msdn.microsoft.com/twistylittlepassagesallalike/2011/04/23/everyone-quotes-command-line-arguments-the-wrong-way/
static char * AppendArg (char * output, const char * input)
{
   size_t inputIndex = 0;
   size_t outputIndex = 0;
   char ch = '\0';
   BOOL quotesNeeded = FALSE;

   output[outputIndex++] = '"';
   do {
      size_t i, numberBackslashes = 0;

      while (input[inputIndex] == '\\') {
         inputIndex++;
         numberBackslashes++;
      }
      ch = input[inputIndex];
      if (ch == '\0') {
         // End of string: escape backslashes and end
         for (i = 0; i < numberBackslashes; i++) {
            output[outputIndex++] = '\\';
            output[outputIndex++] = '\\';
         }
         // quotes needed if the input string is empty
         if (inputIndex == 0) {
            quotesNeeded = TRUE;
         }

      } else if (ch == '"') {
         // Escape backslashes and following " mark
         for (i = 0; i < numberBackslashes; i++) {
            output[outputIndex++] = '\\';
            output[outputIndex++] = '\\';
         }
         output[outputIndex++] = '\\';
         output[outputIndex++] = ch;

         // quotes needed as " is present
         quotesNeeded = TRUE;
      } else {
         // Backslashes aren't special here
         for (i = 0; i < numberBackslashes; i++) {
            output[outputIndex++] = '\\';
         }
         output[outputIndex++] = ch;

         // quotes needed if whitespace chars are present
         if ((ch == '\t') || (ch == ' ') || (ch == '\n') || (ch == '\v')) {
            quotesNeeded = TRUE;
         }
      }
      inputIndex++;
   } while (ch != '\0');

   // No quotes required? Just copy the whole string
   if (!quotesNeeded) {
      inputIndex--;
      outputIndex = inputIndex;
      memcpy (output, input, outputIndex);
      output[outputIndex] = '\0';
      return &output[outputIndex];
   }

   output[outputIndex++] = '"';
   output[outputIndex] = '\0';
   return &output[outputIndex];
}

static char * GenerateArgs (CommStruct * pcs, size_t argc, char ** argv)
{
   size_t max_space = 0;
   size_t i = 0;
   char * output = NULL;
   char * tmp = NULL;

   if (argc < 1) {
      err_printf (0, "GenerateArgs: no args provided");
      exit (INTERNAL_ERROR);
   }

   // Generate upper bound on required space for args
   for (i = 0; i < argc; i++) {
      max_space += strlen (argv[i]) * 2;
      max_space += 10;
      dbg_fprintf (stderr, "[%s]\n", argv[i]);
   }
   tmp = output = calloc (1, max_space);
   if (!output) {
      err_printf (0, "GenerateArgs: out of memory");
      exit (INTERNAL_ERROR);
   }

   // append first arg
   tmp = AppendArg (tmp, argv[0]);

   for (i = 1; i < argc; i++) {
      // add a space
      *tmp = ' ';
      tmp++;
      *tmp = '\0';

      // append next arg
      tmp = AppendArg (tmp, argv[i]);
   }
   return output;
}

// Library is unlinked in the event of a normal exit or USER_ERROR
static void UnlinkLibrary (CommStruct * pcs, DEBUG_EVENT * pDebugEvent, PROCESS_INFORMATION * pProcessInformation)
{
   unsigned failure_count = 0;

   while (!DeleteFile (pcs->libraryName)) {
      // Deleting the file failed
      switch (GetLastError()) {
         case ERROR_FILE_NOT_FOUND:
            // File is gone - all done!
            return;
         case ERROR_ACCESS_DENIED:
            // Subprocess is still holding a handle on the library - try again
            if (failure_count == 0) {
               // On the first failure, send a terminate signal, and then continue from
               // the current debug event to allow it to terminate
               TerminateProcess (pProcessInformation->hProcess, 0);
               ContinueDebugEvent (pDebugEvent->dwProcessId, pDebugEvent->dwThreadId, DBG_CONTINUE);
            } else if (failure_count < 100) {
               // On subsequent failures, we eat up any remaining debug events
               TerminateProcess (pProcessInformation->hProcess, 0);
               if (WaitForDebugEvent (pDebugEvent, 10)) {
                  ContinueDebugEvent (pDebugEvent->dwProcessId, pDebugEvent->dwThreadId, DBG_CONTINUE);
               }
            } else {
               // That's enough failures. Give up.
               err_printf (1, "unable to unlink '%s'", pcs->libraryName);
               exit (INTERNAL_ERROR);
               return;
            }
            failure_count ++;
            Sleep (10);
            break;
         default:
            // unlink failed for some other reason
            // Don't keep trying
            err_printf (1, "unable to unlink '%s'", pcs->libraryName);
            exit (INTERNAL_ERROR);
            return;
      }
   }
}

// Create and fill the x86determiniser DLL
static void SetupLibrary (CommStruct * pcs)
{
   extern uint8_t _x86d__binary_determiniser_dll_start[];
   extern uint8_t _x86d__binary_determiniser_dll_end[];
   size_t size = ((uintptr_t) _x86d__binary_determiniser_dll_end) -
                  ((uintptr_t) _x86d__binary_determiniser_dll_start);
   FILE * fd;
   char tmp_dir[MAX_PATH + 1];
   char tmp_file[MAX_PATH + 1];
   DWORD tmp_dir_size;

   tmp_dir_size = GetTempPath (sizeof (tmp_dir), tmp_dir);
   if ((tmp_dir_size == 0) || (tmp_dir_size >= sizeof (tmp_dir))) {
      err_printf (1, "Unable to GetTempPath");
      exit (INTERNAL_ERROR);
   }
   if (GetTempFileName (tmp_dir, "x8d", 0, tmp_file) == 0) {
      err_printf (1, "Unable to GetTempFileName");
      exit (INTERNAL_ERROR);
   }
   snprintf (pcs->libraryName, MAX_FILE_NAME_SIZE,
         "%s.%sdeterminiser-%d.dll", tmp_file, X86_OR_X64,
         (int) GetCurrentProcessId());
   pcs->libraryName[MAX_FILE_NAME_SIZE - 1] = '\0';

   fd = fopen (pcs->libraryName, "wb");
   if (!fd) {
      // User should set a different TMP/TEMP value?
      err_printf (1, "Unable to create '%s'", pcs->libraryName);
      exit (USER_ERROR);
   }

   if (size != fwrite (_x86d__binary_determiniser_dll_start, 1, size, fd)) {
      err_printf (1, "Unable to fill '%s'", pcs->libraryName);
      exit (INTERNAL_ERROR);
   }
   fclose (fd);

   // delete original tmp file created by the OS
   DeleteFile (tmp_file);

   dbg_fprintf (stderr, "INITIAL: library is '%s'\n", pcs->libraryName);
}

static int DetermineOffsets(DetermineOffsetsStruct * dos,
                            const char * kernel32_name,
                            LPVOID load_address)
{
   // Determine offset of LoadLibrary and GetProcAddress
   // within kernel32.dll, and the load address of the DLL
   HMODULE kernel32;
   MODULEINFO modinfo;
   LPVOID pa;
   CommStruct * pcs = dos->pcs;
  
   if (strncmp (kernel32_name, "\\\\?\\", 4) == 0) {
      // remove prefix of path (not allowed for LoadLibrary)
      kernel32_name += 4;
   }
   kernel32 = LoadLibraryA (kernel32_name);
   if (!kernel32) {
      if (GetLastError () == ERROR_BAD_EXE_FORMAT) {
         err_wrong_architecture (dos->argv0);
         return 1;
      }
      err_printf (1, "LoadLibrary ('%s')", kernel32_name);
      return 1;
   }
   if (!GetModuleInformation
         (GetCurrentProcess(), kernel32, &modinfo, sizeof(MODULEINFO))) {
      err_printf (1, "GetModuleInformation ('%s')", kernel32_name);
      return 1;
   }
   dbg_fprintf (stderr, "AWAIT_FIRST: %s: base %p size %u\n", kernel32_name,
         modinfo.lpBaseOfDll, (unsigned) modinfo.SizeOfImage);
  
   pa = GetProcAddress (kernel32, "GetProcAddress");
   if (!pa) {
      err_printf (1, "GetProcAddress ('GetProcAddress') within '%s'", kernel32_name);
      return 1;
   }
   dos->getProcAddressOffset = (char *) pa - (char *) modinfo.lpBaseOfDll;
  
   pa = GetProcAddress (kernel32, "GetLastError");
   if (!pa) {
      err_printf (1, "GetProcAddress ('GetLastError') within '%s'", kernel32_name);
      return 1;
   }
   dos->getLastErrorOffset = (char *) pa - (char *) modinfo.lpBaseOfDll;
  
   pa = GetProcAddress (kernel32, "LoadLibraryA");
   if (!pa) {
      err_printf (1, "GetProcAddress ('LoadLibraryA') within '%s'", kernel32_name);
      return 1;
   }
   dos->loadLibraryOffset = (char *) pa - (char *) modinfo.lpBaseOfDll;
  
   // We also have the load address of the the DLL
   dos->kernel32Base = load_address;
   dbg_fprintf (stderr, "AWAIT_FIRST: kernel32base = %p\n", (void *) dos->kernel32Base);
   return 0;
}

// Entry point from main
// Args already parsed into CommStruct
int X86DeterminiserLoader(CommStruct * pcs, int argc, char ** argv)
{
   STARTUPINFO startupInfo;
   PROCESS_INFORMATION processInformation;
   DEBUG_EVENT debugEvent;
   DWORD dwCreationFlags;
   BOOL rc, run = TRUE;
   char buf[BUFSIZ];
   SIZE_T len, exeLen;
   CONTEXT startContext;
   LPVOID startAddress = NULL;
   DetermineOffsetsStruct dos[1];
   char startInstruction = 0;
   char *commandLine;
   const char *exeText = ".exe";
   DWORD todo = DBG_CONTINUE;

   memset (&startupInfo, 0, sizeof (startupInfo));
   memset (&processInformation, 0, sizeof (processInformation));
   memset (&debugEvent, 0, sizeof (debugEvent));
   memset (&startContext, 0, sizeof (startContext));
   memset (&dos, 0, sizeof (dos));
   startupInfo.cb = sizeof (startupInfo);
   pcs->singleStepHandlerAddress = 0;
   dos->pcs = pcs;

   // add .exe extension if missing
   dos->argv0 = argv[0];
   len = strlen (dos->argv0);
   exeLen = strlen (exeText);
   if ((len < exeLen) || (strcasecmp (&dos->argv0[len - exeLen], exeText) != 0)) {
      // No .exe extension present: add it
      dos->argv0 = calloc (1, len + exeLen + 1);
      if (!dos->argv0) {
         err_printf (0, "Memory allocation error");
         exit (INTERNAL_ERROR);
      }
      strcpy (dos->argv0, argv[0]);
      strcat (dos->argv0, exeText);
   }

   // Deploy .dll in temporary directory
   SetupLibrary (pcs);

   dwCreationFlags = DEBUG_PROCESS | DEBUG_ONLY_THIS_PROCESS;
   commandLine = GenerateArgs (pcs, (size_t) argc, argv);
   dbg_fprintf (stderr, "[[%s]]\n", commandLine);
   rc = CreateProcess(
     /* _In_opt_    LPCTSTR               */ dos->argv0,
     /* _Inout_opt_ LPTSTR                */ commandLine /* lpCommandLine */,
     /* _In_opt_    LPSECURITY_ATTRIBUTES */ NULL /* lpProcessAttributes */,
     /* _In_opt_    LPSECURITY_ATTRIBUTES */ NULL /* lpThreadAttributes */,
     /* _In_        BOOL                  */ FALSE /* bInheritHandles */,
     /* _In_        DWORD                 */ dwCreationFlags,
     /* _In_opt_    LPVOID                */ NULL /* lpEnvironment */,
     /* _In_opt_    LPCTSTR               */ NULL /* lpCurrentDirectory */,
     /* _In_        LPSTARTUPINFO         */ &startupInfo,
     /* _Out_       LPPROCESS_INFORMATION */ &processInformation
   );
   if (!rc) {
      if ((PTR_SIZE == 4) && (GetLastError () == ERROR_NOT_SUPPORTED)) {
         /* Loading 64-bit .exe using x86determiniser */
         err_wrong_architecture (argv[0]);
         DeleteFile (pcs->libraryName);
         exit (USER_ERROR);
      }
      err_printf (1, "CreateProcess ('%s')", argv[0]);
      DeleteFile (pcs->libraryName);
      exit (USER_ERROR);
   }

   // INITIAL STAGE
   // Wait for the child process to attach
   // Obtain the start address for the child process executable
   // Put a breakpoint at the start address, so we see when it's
   // actually reached
   while (!startAddress) {
      rc = WaitForDebugEvent (&debugEvent, INFINITE);
      todo = DBG_CONTINUE;
      if (!rc) {
         err_printf (1, "INITIAL: WaitForDebugEvent");
         exit (INTERNAL_ERROR);
      }
      switch (debugEvent.dwDebugEventCode) {
         case CREATE_PROCESS_DEBUG_EVENT:
            if ((debugEvent.dwProcessId != processInformation.dwProcessId)
            || (debugEvent.dwThreadId != processInformation.dwThreadId)) {
               err_printf (0, "INITIAL: CREATE_PROCESS_DEBUG_EVENT from unexpected process");
               exit (INTERNAL_ERROR);
            }
            startAddress = debugEvent.u.CreateProcessInfo.lpStartAddress;

            // First instruction is saved
            rc = ReadProcessMemory
              (processInformation.hProcess,
               (void *) startAddress,
               &startInstruction,
               1,
               &len);
            if ((!rc) || (len != 1)) {
               err_printf (1, "INITIAL: Unable to read first instruction");
               exit (INTERNAL_ERROR);
            }

            // Install breakpoint
            rc = WriteProcessMemory
              (processInformation.hProcess,
               (void *) startAddress,
               "\xcc",
               1,
               &len);
            if ((!rc) || (len != 1)) {
               err_printf (1, "INITIAL: Unable to replace first instruction with breakpoint");
               exit (INTERNAL_ERROR);
            }
            break;
         default:
            todo = DefaultHandler (pcs, "INITIAL", &debugEvent, &processInformation);
            break;
      }
      ContinueDebugEvent
         (debugEvent.dwProcessId, debugEvent.dwThreadId, todo);
   }
   dbg_fprintf (stderr, "INITIAL: startAddress = %p\n", startAddress);

   // AWAIT_FIRST STAGE
   // Await breakpoint at first instruction
   // Capture startContext once it's reached
   // Also capture the kernel32.dll base address, which we should see
   // before the first instruction
   while (!get_pc(&startContext)) {
      rc = WaitForDebugEvent (&debugEvent, INFINITE);
      todo = DBG_CONTINUE;
      if (!rc) {
         err_printf (1, "AWAIT_FIRST: WaitForDebugEvent");
         exit (INTERNAL_ERROR);
      }
      dbg_fprintf (stderr, "AWAIT_FIRST: debug event %x\n",
            (unsigned) debugEvent.dwDebugEventCode);
      switch (debugEvent.dwDebugEventCode) {
         case EXCEPTION_DEBUG_EVENT:
            if ((debugEvent.dwProcessId == processInformation.dwProcessId)
            && (debugEvent.dwThreadId == processInformation.dwThreadId)) {
               CONTEXT context;

               context.ContextFlags = CONTEXT_FULL;
               rc = GetThreadContext (processInformation.hThread, &context);
               if (!rc) {
                  err_printf (1, "AWAIT_FIRST: GetThreadContext");
                  exit (INTERNAL_ERROR);
               }
               dbg_fprintf (stderr, "AWAIT_FIRST: exception code %x\n",
                     (unsigned) debugEvent.u.Exception.ExceptionRecord.ExceptionCode);
               switch (debugEvent.u.Exception.ExceptionRecord.ExceptionCode) {
                  case STATUS_BREAKPOINT:
                     // Are we in the right place?
                     set_pc (&context, get_pc (&context) - 1);
                     if ((void *) get_pc (&context) != startAddress) {
                        // This is a breakpoint in some system code.
                        // Its purpose is not known.
                        break;
                     }

                     // REMOTE_LOADER STAGE
                     // Remove breakpoint
                     // Inject code needed to load the DLL and run the loader
                     memcpy (&startContext, &context, sizeof (CONTEXT));

                     if (!dos->kernel32Base) {
                        // We haven't got a name, so let's enumerate
                        HMODULE hMods[1024];
                        DWORD cbNeeded;
                        unsigned int i;
                        if( EnumProcessModules(processInformation.hProcess, hMods, sizeof(hMods), &cbNeeded)) {
                           for ( i = 0; i < (cbNeeded / sizeof(HMODULE)); i++ ) {
                              TCHAR szModName[MAX_PATH];
                              // Get the full path to the module's file.
                              GetModuleFileNameEx( processInformation.hProcess, hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR));
                              dbg_fprintf (stderr, "AWAIT_FIRST: enum DLL = %s base = %p\n", szModName, (void *) hMods[i]);

                              if (StrStrI (szModName, "\\kernel32.dll") != NULL) {
                                 // Determine offset of LoadLibrary and GetProcAddress
                                 // within kernel32.dll, and the load address of the DLL
                                 if (DetermineOffsets (dos, szModName, hMods[i])) {
                                    return 1;
                                 }
                              }
                           }
                        }
                     }

                     if (!dos->kernel32Base) {
                        err_printf (0, "REMOTE_LOADER: don't know the kernel32.dll base address");
                        exit (INTERNAL_ERROR);
                     }

                     // Uninstall breakpoint
                     rc = WriteProcessMemory
                       (processInformation.hProcess,
                        (void *) startAddress,
                        &startInstruction,
                        1,
                        &len);
                     if ((!rc) || (len != 1)) {
                        err_printf (1, "REMOTE_LOADER: Unable to restore first instruction");
                        exit (INTERNAL_ERROR);
                     }

                     StartRemoteLoader
                       (pcs,
                        dos,
                        (uintptr_t) startAddress,
                        processInformation.hProcess,
                        &context);
                     if (pcs->remoteDebugEnabled) {
                        set_single_step_flag(&context);
                     }
                     SetThreadContext (processInformation.hThread, &context);

                     break;
                  default:
                     todo = DefaultHandler (pcs, "AWAIT_FIRST", &debugEvent, &processInformation);
                     break;
               }
            } else {
               todo = DefaultHandler (pcs, "AWAIT_FIRST", &debugEvent, &processInformation);
            }
            break;
         case LOAD_DLL_DEBUG_EVENT:
            len = GetFinalPathNameByHandle
              (debugEvent.u.LoadDll.hFile,
               buf,
               sizeof (buf) - 1,
               0);
            buf[len] = 0;
            CloseHandle (debugEvent.u.LoadDll.hFile);
            dbg_fprintf (stderr, "AWAIT_FIRST: load DLL = %s base = %p\n", buf,
                  (void *) debugEvent.u.LoadDll.lpBaseOfDll);

            /* if buf == kernel32.dll, we have the addresses for LoadLibrary
             * and GetProcAddress, so we can switch the process over to use
             * the determiniser. */

            if (strstr (buf, "\\kernel32.dll") != NULL) {
               // Determine offset of LoadLibrary and GetProcAddress
               // within kernel32.dll, and the load address of the DLL
               // Note: debugEvent.u.LoadDll.lpBaseOfDll may be NULL in some cases,
               // see https://github.com/jwhitham/x86determiniser/issues/3 . In such cases
               // we have a second chance to compute these offsets at the first breakpoint.
               if (DetermineOffsets (dos, buf, debugEvent.u.LoadDll.lpBaseOfDll)) {
                  return 1;
               }
            }

            break;
         default:
            todo = DefaultHandler (pcs, "AWAIT_FIRST", &debugEvent, &processInformation);
            break;
      }
      ContinueDebugEvent
         (debugEvent.dwProcessId, debugEvent.dwThreadId, todo);
   }

   // AWAIT_REMOTE_LOADER_BP STAGE
   // Wait for the breakpoint indicating that the remote loader has finished
   // and the x86 determiniser is ready to run. Get the address of the single step
   // procedure.
   while (!pcs->singleStepHandlerAddress) {
      rc = WaitForDebugEvent (&debugEvent, INFINITE);
      todo = DBG_CONTINUE;
      if (!rc) {
         err_printf (1, "AWAIT_REMOTE_LOADER_BP: WaitForDebugEvent");
         exit (INTERNAL_ERROR);
      }
      dbg_fprintf (stderr, "AWAIT_REMOTE_LOADER_BP: debug event %x\n",
            (unsigned) debugEvent.dwDebugEventCode);
      switch (debugEvent.dwDebugEventCode) {
         case EXCEPTION_DEBUG_EVENT:
            if ((debugEvent.dwProcessId == processInformation.dwProcessId)
            && (debugEvent.dwThreadId == processInformation.dwThreadId)) {
               CONTEXT context;
               context.ContextFlags = CONTEXT_FULL;
               rc = GetThreadContext (processInformation.hThread, &context);
               if (!rc) {
                  err_printf (1, "AWAIT_REMOTE_LOADER_BP: GetThreadContext");
                  exit (INTERNAL_ERROR);
               }
               dbg_fprintf (stderr, "AWAIT_REMOTE_LOADER_BP: exception code %x\n",
                     (unsigned) debugEvent.u.Exception.ExceptionRecord.ExceptionCode);
               switch (debugEvent.u.Exception.ExceptionRecord.ExceptionCode) {
                  case STATUS_BREAKPOINT:
                     dbg_fprintf (stderr, "AWAIT_REMOTE_LOADER_BP: breakpoint at %p\n",
                           (void *) get_pc (&context));
                     if (pcs->remoteDebugEnabled) {
                        dbg_state(&context);
                     }

                     // EAX contains an error code, or 0x101 on success
                     if (get_xax (&context) != COMPLETED_LOADER) {
                        // Not completed the loader yet, this is some
                        // other breakpoint
                        todo = DefaultHandler
                           (pcs, "AWAIT_REMOTE_LOADER_BP", &debugEvent, &processInformation);
                     } else {
                        // EBX confirms location of CommStruct in child process
                        if (get_xbx (&context) != (uintptr_t) pcs->myself) {
                           err_printf (0, "AWAIT_REMOTE_LOADER_BP: unexpected EBX value");
                           exit (INTERNAL_ERROR);
                        }

                        // read back the CommStruct, now updated with
                        // new minAddress/maxAddress and singleStepHandlerAddress
                        ReadProcessMemory
                          (processInformation.hProcess,
                           pcs->myself,
                           pcs,
                           sizeof (CommStruct),
                           NULL);

                        // REMOTE_LOADER_BP STAGE
                        // "reboot" into the original context and continue, initially
                        // single stepping
                        memcpy (&context, &startContext, sizeof (CONTEXT));
                        set_single_step_flag (&context);
                        rc = SetThreadContext (processInformation.hThread, &context);
                        if (!rc) {
                           err_printf (1, "REMOTE_LOADER_BP: unable to SetThreadContext");
                           exit (INTERNAL_ERROR);
                        }
                     }

                     break;
                  case STATUS_SINGLE_STEP:
                     // There is some single-stepping at the end of the loader,
                     // this is normal, let it continue
                     dbg_fprintf (stderr, "AWAIT_REMOTE_LOADER_BP: single step at %p\n",
                           (void *) get_pc (&context));
                     if (pcs->remoteDebugEnabled) {
                        dbg_state(&context);
                     }
                     set_single_step_flag(&context);
                     SetThreadContext (processInformation.hThread, &context);
                     break;
                  default:
                     todo = DefaultHandler (pcs, "AWAIT_REMOTE_LOADER_BP", &debugEvent, &processInformation);
                     break;
               }
            } else {
               todo = DefaultHandler (pcs, "AWAIT_REMOTE_LOADER_BP", &debugEvent, &processInformation);
            }
            break;
         default:
            todo = DefaultHandler (pcs, "AWAIT_REMOTE_LOADER_BP", &debugEvent, &processInformation);
            break;
      }
      ContinueDebugEvent
         (debugEvent.dwProcessId, debugEvent.dwThreadId, todo);
   }

   while (TRUE) {
      // RUNNING STATE
      // Child process runs within x86 determiniser.
      // Waiting for a single-step event, or exit.

      run = TRUE;
      while (run) {
         rc = WaitForDebugEvent (&debugEvent, INFINITE);
         todo = DBG_CONTINUE;
         if (!rc) {
            err_printf (1, "RUNNING: WaitForDebugEvent");
            exit (INTERNAL_ERROR);
         }
         dbg_fprintf (stderr, "RUNNING: debug event %x\n",
               (unsigned) debugEvent.dwDebugEventCode);
         switch (debugEvent.dwDebugEventCode) {
            case EXCEPTION_DEBUG_EVENT:
               dbg_fprintf (stderr, "RUNNING: exception code %x\n",
                     (unsigned) debugEvent.u.Exception.ExceptionRecord.ExceptionCode);
               if ((debugEvent.dwProcessId == processInformation.dwProcessId)
               && (debugEvent.dwThreadId == processInformation.dwThreadId)) {
                  CONTEXT context;
                  uintptr_t address;

                  context.ContextFlags = CONTEXT_FULL;
                  rc = GetThreadContext (processInformation.hThread, &context);
                  if (!rc) {
                     err_printf (1, "RUNNING: GetThreadContext");
                     exit (INTERNAL_ERROR);
                  }
                  switch (debugEvent.u.Exception.ExceptionRecord.ExceptionCode) {
                     case STATUS_SINGLE_STEP:
                        // Reached single step; run single step handler.
                        dbg_fprintf
                          (stderr, "RUNNING: Single step at %p, go to handler at %p\n",
                           (void *) get_pc (&context), (void *) pcs->singleStepHandlerAddress);
                        run = FALSE;
                        StartSingleStepProc
                          ((void *) pcs->singleStepHandlerAddress,
                           processInformation.hProcess,
                           &context);
                        context.ContextFlags = CONTEXT_FULL;
                        SetThreadContext (processInformation.hThread, &context);
                        break;
                     case STATUS_BREAKPOINT:
                        dbg_fprintf (stderr, "RUNNING: breakpoint at %p\n",
                              (void *) get_pc (&context));
                        todo = DefaultHandler (pcs, "RUNNING", &debugEvent, &processInformation);
                        break;
                     case STATUS_ACCESS_VIOLATION:
                        // The segfault may have been caused by jumping to an address in the
                        // program .text section which has been marked non-executable: this
                        // happens in "free run" mode.
                        address = (uintptr_t) debugEvent.u.Exception.ExceptionRecord.ExceptionAddress;
                        if (((uintptr_t) address >= pcs->minAddress)
                        && ((uintptr_t) address < pcs->maxAddress)
                        && (get_pc (&context) == (uintptr_t) address)) {

                           // We know that the segfault happened in the .text section at the PC address
                           // so it's probably a result of finishing "free run"... but let's make sure!
                           uint8_t free_run_flag = 0;
                           ReadProcessMemory
                             (processInformation.hProcess,
                              (void *) pcs->freeRunFlagAddress,
                              &free_run_flag,
                              sizeof (uint8_t),
                              NULL);

                           if (free_run_flag) {
                              dbg_fprintf
                                (stderr, "RUNNING: Re-entry to text at %p, go to handler at %p\n",
                                 (void *) get_pc (&context), (void *) pcs->singleStepHandlerAddress);
                              run = FALSE;
                              StartSingleStepProc
                                ((void *) pcs->singleStepHandlerAddress,
                                 processInformation.hProcess,
                                 &context);
                              context.ContextFlags = CONTEXT_FULL;
                              SetThreadContext (processInformation.hThread, &context);
                           } else {
                              todo = DefaultHandler (pcs, "RUNNING", &debugEvent, &processInformation);
                           }
                        } else {
                           todo = DefaultHandler (pcs, "RUNNING", &debugEvent, &processInformation);
                        }
                        break;
                     default:
                        todo = DefaultHandler (pcs, "RUNNING", &debugEvent, &processInformation);
                        break;
                  }
               } else {
                  todo = DefaultHandler (pcs, "RUNNING", &debugEvent, &processInformation);
               }
               break;
            default:
               todo = DefaultHandler (pcs, "RUNNING", &debugEvent, &processInformation);
               break;
         }
         ContinueDebugEvent
            (debugEvent.dwProcessId, debugEvent.dwThreadId, todo);
      }

      // SINGLE_STEP STATE
      // Single step handler runs within x86 determiniser.
      // Waiting for a breakpoint at the end of the handler.

      while (!run) {
         rc = WaitForDebugEvent (&debugEvent, INFINITE);
         todo = DBG_CONTINUE;
         if (!rc) {
            err_printf (1, "SINGLE_STEP: WaitForDebugEvent");
            exit (INTERNAL_ERROR);
         }
         dbg_fprintf (stderr, "SINGLE_STEP: debug event %x\n",
               (unsigned) debugEvent.dwDebugEventCode);
         switch (debugEvent.dwDebugEventCode) {
            case EXCEPTION_DEBUG_EVENT:
               dbg_fprintf (stderr, "SINGLE_STEP: exception code %x\n",
                     (unsigned) debugEvent.u.Exception.ExceptionRecord.ExceptionCode);
               if ((debugEvent.dwProcessId == processInformation.dwProcessId)
               && (debugEvent.dwThreadId == processInformation.dwThreadId)) {
                  CONTEXT context;
                  context.ContextFlags = CONTEXT_FULL;
                  rc = GetThreadContext (processInformation.hThread, &context);
                  if (!rc) {
                     err_printf (1, "SINGLE_STEP: GetThreadContext");
                     exit (INTERNAL_ERROR);
                  }
                  switch (debugEvent.u.Exception.ExceptionRecord.ExceptionCode) {
                     case STATUS_BREAKPOINT:
                        dbg_fprintf (stderr, "SINGLE_STEP: breakpoint at %p\n",
                              (void *) get_pc (&context));
                        // single step procedure finished
                        // EAX contains an error code, or 0x102 on success
                        // EBX is pointer to context, altered by remote
                        if (get_xax (&context) != COMPLETED_SINGLE_STEP_HANDLER) {
                           err_printf (get_xax (&context), "SINGLE_STEP");
                           return 1;
                        }
                        // context restored
                        dbg_fprintf (stderr, "SINGLE_STEP: location of context: %p\n", (void *) get_xbx (&context));
                        ReadProcessMemory
                          (processInformation.hProcess,
                           (void *) get_xbx (&context),
                           (void *) &context,
                           sizeof (CONTEXT),
                           NULL);
                        dbg_fprintf (stderr, "SINGLE_STEP: flags word is %x\n", (unsigned) context.EFlags);
                        context.ContextFlags = CONTEXT_FULL;
                        SetThreadContext (processInformation.hThread, &context);
                        run = TRUE;
                        break;
                     case STATUS_SINGLE_STEP:
                        dbg_fprintf (stderr, "SINGLE_STEP: single step at %p\n",
                              (void *) get_pc (&context));
                        todo = DefaultHandler (pcs, "SINGLE_STEP", &debugEvent, &processInformation);
                        break;
                     default:
                        todo = DefaultHandler (pcs, "SINGLE_STEP", &debugEvent, &processInformation);
                        break;
                  }
               } else {
                  todo = DefaultHandler (pcs, "SINGLE_STEP", &debugEvent, &processInformation);
               }
               break;
            default:
               todo = DefaultHandler (pcs, "SINGLE_STEP", &debugEvent, &processInformation);
               break;
         }
         ContinueDebugEvent
            (debugEvent.dwProcessId, debugEvent.dwThreadId, todo);
      }
   }
   return 0;
}

/* Handle a debug event without doing anything special */
static DWORD DefaultHandler (
      CommStruct * pcs,
      const char * state, DEBUG_EVENT * pDebugEvent,
      PROCESS_INFORMATION * pProcessInformation)
{
   DWORD todo = DBG_CONTINUE;
   switch (pDebugEvent->dwDebugEventCode) {
      case CREATE_PROCESS_DEBUG_EVENT:
         err_printf (0, "%s: received a second CREATE_PROCESS_DEBUG_EVENT "
               "from an unexpected process\n", state);
         exit (INTERNAL_ERROR);
         break;
      case EXIT_PROCESS_DEBUG_EVENT:
         if (pDebugEvent->dwProcessId == pProcessInformation->dwProcessId) {
            dbg_fprintf (stderr, "%s: Process exited! %d\n", state, (int) pDebugEvent->dwProcessId);
            UnlinkLibrary (pcs, pDebugEvent, pProcessInformation);
            exit (pDebugEvent->u.ExitProcess.dwExitCode);
         }
         break;
      case EXCEPTION_DEBUG_EVENT:
         if ((pDebugEvent->dwProcessId == pProcessInformation->dwProcessId)
         && (pDebugEvent->dwThreadId == pProcessInformation->dwThreadId)) {
            CONTEXT context;
            memset (&context, 0, sizeof (CONTEXT));
            context.ContextFlags = CONTEXT_FULL;
            GetThreadContext (pProcessInformation->hThread, &context);
            switch (pDebugEvent->u.Exception.ExceptionRecord.ExceptionCode) {
               case STATUS_BREAKPOINT:
                  if ((get_xax (&context) >= X86D_FIRST_ERROR)
                  && (get_xax (&context) <= X86D_LAST_ERROR)) {
                     // Error from the remote loader:
                     // Parse as a Win32 error if possible (Windows error code in xBX register)
                     switch (get_xax (&context)) {
                        case FAILED_LOADLIBRARY:
                           SetLastError ((DWORD) get_xbx (&context));
                           err_printf (1, "x86determiniser LoadLibrary('%s')", pcs->libraryName);
                           break;
                        case FAILED_GETPROCADDRESS:
                           SetLastError ((DWORD) get_xbx (&context));
                           err_printf (1, "x86determiniser GetProcAddress('%s')", pcs->procName);
                           break;
                        default:
                           err_printf (get_xax (&context), "x86determiniser");
                           UnlinkLibrary (pcs, pDebugEvent, pProcessInformation);
                           exit (USER_ERROR);
                           break;
                     }
                  } else {
                     err_printf (0, "Breakpoint instruction at %p", (void *) get_pc (&context));
                  }
                  exit (INTERNAL_ERROR);
                  break;
               case STATUS_SINGLE_STEP:
                  err_printf (0, "%s: Unexpected single step at %p", state, (void *) get_pc (&context));
                  exit (INTERNAL_ERROR);
                  break;
               case STATUS_PRIVILEGED_INSTRUCTION:
               case STATUS_ILLEGAL_INSTRUCTION:
                  err_printf (0, "Illegal instruction at %p", (void *) get_pc (&context));
                  UnlinkLibrary (pcs, pDebugEvent, pProcessInformation);
                  exit (USER_ERROR);
                  break;
               case STATUS_ACCESS_VIOLATION:
                  err_printf (0, "Segmentation fault at %p", (void *) get_pc (&context));
                  UnlinkLibrary (pcs, pDebugEvent, pProcessInformation);
                  exit (USER_ERROR);
                  break;
               case STATUS_INTEGER_DIVIDE_BY_ZERO:
                  err_printf (0, "Divide by zero at %p", (void *) get_pc (&context));
                  UnlinkLibrary (pcs, pDebugEvent, pProcessInformation);
                  exit (USER_ERROR);
                  break;
               case STATUS_STACK_OVERFLOW:
                  err_printf (0, "Stack overflow at %p, sp = %p",
                     (void *) get_pc (&context),
                     (void *) get_xsp (&context));
                  UnlinkLibrary (pcs, pDebugEvent, pProcessInformation);
                  exit (USER_ERROR);
                  break;
               default:
                  // pass through
                  dbg_fprintf (stderr, "%s: Exception at %p code 0x%0x (first chance %d)\n",
                     state, (void *) get_pc (&context),
                     (unsigned) pDebugEvent->u.Exception.ExceptionRecord.ExceptionCode,
                     !!pDebugEvent->u.Exception.dwFirstChance);
                  todo = DBG_EXCEPTION_NOT_HANDLED;
                  break;
            }
         }
         break;
      case CREATE_THREAD_DEBUG_EVENT:
         if (pDebugEvent->dwProcessId == pProcessInformation->dwProcessId) {
            dbg_fprintf (stderr, "%s: New thread %p\n", state, (void *) pDebugEvent->u.CreateThread.hThread);
         }
         break;
      case EXIT_THREAD_DEBUG_EVENT:
         if (pDebugEvent->dwProcessId == pProcessInformation->dwProcessId) {
            dbg_fprintf (stderr, "%s: Exit thread 0x%x\n", state, (unsigned) pDebugEvent->dwThreadId);
         }
         break;
      case LOAD_DLL_DEBUG_EVENT:
      case UNLOAD_DLL_DEBUG_EVENT:
      case OUTPUT_DEBUG_STRING_EVENT:
      case RIP_EVENT:
         break;
      default:
         break;
   }
   return todo;
}
