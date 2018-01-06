#define _WIN32_WINNT 0x600
#include <stdio.h>
#include <string.h>
#include <Windows.h>
#include <Psapi.h>

#include "remote_loader.h"
#include "x86_flags.h"


static BOOL debugEnabled = FALSE;

#define dbg_printf if (debugEnabled) printf
#define err_printf printf

void DefaultHandler (const char * state, DEBUG_EVENT * pDebugEvent,
      PROCESS_INFORMATION * pProcessInformation);

typedef struct SingleStepStruct {
   void * unused;
   PCONTEXT pcontext;
   CONTEXT context;
} SingleStepStruct;


void StartSingleStepProc
  (void * singleStepProc,
   HANDLE hProcess,
   PCONTEXT context)
{
   SingleStepStruct localCs;

   memcpy (&localCs.context, context, sizeof (CONTEXT));

   // reserve stack space for SingleStepStruct
   context->Esp -= sizeof (localCs);
   localCs.pcontext = (void *) (context->Esp + 8);
   localCs.unused = NULL;
   // dbg_printf ("location of context: %p\n", (void *) localCs.pcontext);

   // fill remote stack
   // return address is NULL (1st item in struct: don't return!!)
   // first parameter is "pcontext" (2nd item in struct)
   WriteProcessMemory
     (hProcess,
      (void *) context->Esp,
      &localCs,
      sizeof (localCs),
      NULL);

   // run single step handler until breakpoint
   context->Eip = (DWORD) singleStepProc;
   context->EFlags &= ~SINGLE_STEP_FLAG;
}

void StartRemoteLoader
  (size_t getProcAddressOffset,
   size_t loadLibraryOffset,
   void * kernel32Base,
   void * startAddress,
   HANDLE hProcess,
   const char * binFolder,
   PCONTEXT context)
{
   ssize_t space;
   void * remoteBuf;
   CommStruct localCs;
   BOOL rc;

   memset (&localCs, 0, sizeof (localCs));

   space = (char *) RemoteLoaderEnd - (char *) RemoteLoaderStart;
   if (space <= 0) {
      err_printf ("REMOTE_LOADER: No valid size for remote loader\n");
      exit (1);
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
      err_printf ("REMOTE_LOADER: Unable to allocate %d bytes for remote loader\n", (int) space);
      exit (1);
   }

   // writes remote loader into memory within debugged process
   rc = WriteProcessMemory
     (hProcess,
      remoteBuf,
      RemoteLoaderStart,
      (DWORD) space,
      NULL);
   if (!rc) {
      err_printf ("REMOTE_LOADER: Unable to inject %d bytes\n", (int) space);
      exit (1);
   }

   // reserve the right amount of stack space
   context->Esp -= sizeof (localCs);

   // build data structure to load into the remote stack
   localCs.myself = (void *) context->Esp;
   snprintf (localCs.libraryName, MAX_LIBRARY_NAME_SIZE, "%s/x86determiniser.dll", binFolder);
   strncpy (localCs.procName, "X86DeterminiserStartup", MAX_PROC_NAME_SIZE);
   localCs.loadLibraryProc = 
      (void *) ((char *) kernel32Base + loadLibraryOffset);
   localCs.getProcAddressProc =
      (void *) ((char *) kernel32Base + getProcAddressOffset);
   localCs.startAddress = startAddress;

   // fill remote stack
   // return address is NULL (1st item in struct: don't return!!)
   // first parameter is "myself" (2nd item in struct)
   WriteProcessMemory
     (hProcess,
      localCs.myself,
      &localCs,
      sizeof (localCs),
      NULL);
   if (!rc) {
      err_printf ("REMOTE_LOADER: Unable to fill remote stack\n");
      exit (1);
   }

   // run RemoteLoader procedure until breakpoint
   context->Eip = 
      (DWORD) ((void *) ((char *) remoteBuf +
         ((char *) RemoteLoader - (char *) RemoteLoaderStart)));
   context->EFlags &= ~SINGLE_STEP_FLAG;
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

static char * GenerateArgs (int argc, char ** argv)
{
   size_t max_space = 0;
   size_t i = 0;
   char * output = NULL;
   char * tmp = NULL;

   if (argc < 1) {
      err_printf ("GenerateArgs: no args provided\n");
      exit (1);
   }

   // Generate upper bound on required space for args
   for (i = 0; i < argc; i++) {
      max_space += strlen (argv[i]) * 2;
      max_space += 10;
      dbg_printf ("[%s]\n", argv[i]);
   }
   tmp = output = calloc (1, max_space);
   if (!output) {
      err_printf ("GenerateArgs: out of memory\n");
      exit (1);
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


int X86DeterminiserLoader(int argc, char ** argv)
{
   STARTUPINFO startupInfo;
   PROCESS_INFORMATION processInformation;
   DEBUG_EVENT debugEvent;
   DWORD dwCreationFlags;
   BOOL rc, run = TRUE;
   char buf[128];
   SIZE_T len;
   CONTEXT startContext;
   //CONTEXT stepContext;
   size_t getProcAddressOffset = 0;
   size_t loadLibraryOffset = 0;
   LPVOID kernel32Base = NULL;
   LPVOID startAddress = NULL;
   LPVOID singleStepProc = NULL;
   char startInstruction = 0;
   char *commandLine;
   char binFolder[BUFSIZ];

   memset (&startupInfo, 0, sizeof (startupInfo));
   memset (&processInformation, 0, sizeof (processInformation));
   memset (&debugEvent, 0, sizeof (debugEvent));
   memset (&startContext, 0, sizeof (startContext));
   startupInfo.cb = sizeof (startupInfo);

   if (GetModuleFileName(NULL, binFolder, sizeof (binFolder)) != 0) {
      char * finalSlash = strrchr (binFolder, '\\');
      if (!finalSlash) {
         finalSlash = binFolder;
      }
      *finalSlash = '\0';
   } else {
      err_printf ("GetModuleFileName: error %d\n", (int) GetLastError());
      return 1;
   }



   dwCreationFlags = DEBUG_PROCESS | DEBUG_ONLY_THIS_PROCESS;
   commandLine = GenerateArgs (argc, argv);
   dbg_printf ("[[%s]]\n", commandLine);
   rc = CreateProcess(
     /* _In_opt_    LPCTSTR               */ argv[0],
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
      err_printf ("CreateProcess: error %d\n", (int) GetLastError());
      return 1;
   }

   // Determine offset of LoadLibrary and GetProcAddress
   // within kernel32.dll
   {
      HMODULE kernel32 = LoadLibrary ("kernel32.dll");
      MODULEINFO modinfo;
      LPVOID pa;

      if (!kernel32) {
         err_printf ("LoadLibrary: error %d\n", (int) GetLastError());
         return 1;
      }
      if (!GetModuleInformation
            (GetCurrentProcess(), kernel32, &modinfo, sizeof(MODULEINFO))) {
         err_printf ("GetModuleInformation: error %d\n", (int) GetLastError());
         return 1;
      }
      dbg_printf ("kernel32.dll: base %p size %u\n", modinfo.lpBaseOfDll, (unsigned) modinfo.SizeOfImage);

      pa = GetProcAddress (kernel32, "GetProcAddress");
      if (!pa) {
         err_printf ("GetProcAddress: error %d\n", (int) GetLastError());
         return 1;
      }
      getProcAddressOffset = (char *) pa - (char *) modinfo.lpBaseOfDll;

      pa = GetProcAddress (kernel32, "LoadLibraryA");
      if (!pa) {
         err_printf ("GetProcAddress: error %d\n", (int) GetLastError());
         return 1;
      }
      loadLibraryOffset = (char *) pa - (char *) modinfo.lpBaseOfDll;
   }

   // INITIAL STAGE
   // Wait for the child process to attach
   // Obtain the start address for the child process executable
   // Put a breakpoint at the start address, so we see when it's
   // actually reached
   while (!startAddress) {
      rc = WaitForDebugEvent (&debugEvent, INFINITE);
      if (!rc) {
         err_printf ("INITIAL: WaitForDebugEvent: error %d\n", (int) GetLastError());
         exit (1);
      }
      switch (debugEvent.dwDebugEventCode) {
         case CREATE_PROCESS_DEBUG_EVENT:
            if ((debugEvent.dwProcessId != processInformation.dwProcessId)
            || (debugEvent.dwThreadId != processInformation.dwThreadId)) {
               err_printf ("INITIAL: CREATE_PROCESS_DEBUG_EVENT from unexpected process\n");
               exit (1);
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
               err_printf ("INITIAL: Unable to read first instruction\n");
               exit (1);
            }

            // Install breakpoint
            rc = WriteProcessMemory
              (processInformation.hProcess,
               (void *) startAddress,
               "\xcc",
               1,
               &len);
            if ((!rc) || (len != 1)) {
               err_printf ("INITIAL: Unable to replace first instruction with breakpoint\n");
               exit (1);
            }
            break;
         default:
            DefaultHandler ("INITIAL", &debugEvent, &processInformation);
            break;
      }
      ContinueDebugEvent
         (debugEvent.dwProcessId, debugEvent.dwThreadId, DBG_CONTINUE);
   }
   dbg_printf ("INITIAL: startAddress = %p\n", startAddress);

   // AWAIT_FIRST STAGE
   // Await breakpoint at first instruction
   // Capture startContext once it's reached
   // Also capture the kernel32.dll base address, which we should see
   // before the first instruction
   while (!startContext.Eip) {
      rc = WaitForDebugEvent (&debugEvent, INFINITE);
      if (!rc) {
         err_printf ("AWAIT_FIRST: WaitForDebugEvent: error %d\n", (int) GetLastError());
         exit (1);
      }
      switch (debugEvent.dwDebugEventCode) {
         case EXCEPTION_DEBUG_EVENT:
            if ((debugEvent.dwProcessId == processInformation.dwProcessId)
            && (debugEvent.dwThreadId == processInformation.dwThreadId)) {
               CONTEXT context;

               context.ContextFlags = CONTEXT_FULL;
               rc = GetThreadContext (processInformation.hThread, &context);
               if (!rc) {
                  err_printf ("AWAIT_FIRST: GetThreadContext: error %d\n",
                     (int) GetLastError());
                  exit (1);
               }
               switch (debugEvent.u.Exception.ExceptionRecord.ExceptionCode) {
                  case STATUS_BREAKPOINT:
                     // Are we in the right place?
                     context.Eip --;
                     if ((void *) context.Eip != startAddress) {
                        // This is a breakpoint in some system code.
                        // Its purpose is not known.
                        break;
                     }

                     // REMOTE_LOADER STAGE
                     // Remove breakpoint
                     // Inject code needed to load the DLL and run the loader
                     memcpy (&startContext, &context, sizeof (CONTEXT));

                     if (!kernel32Base) {
                        err_printf ("REMOTE_LOADER: don't know the kernel32.dll base address\n");
                        exit (1);
                     }

                     // Uninstall breakpoint
                     rc = WriteProcessMemory
                       (processInformation.hProcess,
                        (void *) startAddress,
                        &startInstruction,
                        1,
                        &len);
                     if ((!rc) || (len != 1)) {
                        err_printf ("REMOTE_LOADER: Unable to restore first instruction\n");
                        exit (1);
                     }

                     StartRemoteLoader
                       (getProcAddressOffset,
                        loadLibraryOffset,
                        kernel32Base,
                        (void *) startAddress,
                        processInformation.hProcess,
                        binFolder,
                        &context);
                     SetThreadContext (processInformation.hThread, &context);

                     break;
                  default:
                     DefaultHandler ("AWAIT_FIRST", &debugEvent, &processInformation);
                     break;
               }
            } else {
               DefaultHandler ("AWAIT_FIRST", &debugEvent, &processInformation);
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

            /* if buf == kernel32.dll, we have the addresses for LoadLibrary
             * and GetProcAddress, so we can switch the process over to use
             * the determiniser. */

            if (strstr (buf, "\\kernel32.dll") != NULL) {
               kernel32Base = debugEvent.u.LoadDll.lpBaseOfDll;
            }
            break;
         default:
            DefaultHandler ("AWAIT_FIRST", &debugEvent, &processInformation);
            break;
      }
      ContinueDebugEvent
         (debugEvent.dwProcessId, debugEvent.dwThreadId, DBG_CONTINUE);
   }

   // AWAIT_REMOTE_LOADER_BP STAGE
   // Wait for the breakpoint indicating that the remote loader has finished
   // and the x86 determiniser is ready to run. Get the address of the single step
   // procedure.
   while (!singleStepProc) {
      rc = WaitForDebugEvent (&debugEvent, INFINITE);
      if (!rc) {
         err_printf ("AWAIT_REMOTE_LOADER_BP: WaitForDebugEvent: error %d\n", (int) GetLastError());
         exit (1);
      }
      switch (debugEvent.dwDebugEventCode) {
         case EXCEPTION_DEBUG_EVENT:
            if ((debugEvent.dwProcessId == processInformation.dwProcessId)
            && (debugEvent.dwThreadId == processInformation.dwThreadId)) {
               CONTEXT context;
               context.ContextFlags = CONTEXT_FULL;
               rc = GetThreadContext (processInformation.hThread, &context);
               if (!rc) {
                  err_printf ("AWAIT_REMOTE_LOADER_BP: GetThreadContext: error %d\n",
                     (int) GetLastError());
                  exit (1);
               }
               switch (debugEvent.u.Exception.ExceptionRecord.ExceptionCode) {
                  case STATUS_BREAKPOINT:
                     // EAX contains an error code, or 0x101 on success
                     if (context.Eax != 0x101) {
                        DefaultHandler ("AWAIT_REMOTE_LOADER_BP", &debugEvent, &processInformation);
                     }
                     // EBX contains the address of the single step handler
                     singleStepProc = (void *) context.Ebx;

                     // REMOTE_LOADER_BP STAGE
                     // "reboot" into the original context and continue, initially
                     // single stepping
                     memcpy (&context, &startContext, sizeof (CONTEXT));
                     context.EFlags |= SINGLE_STEP_FLAG;
                     rc = SetThreadContext (processInformation.hThread, &context);
                     if (!rc) {
                        err_printf ("REMOTE_LOADER_BP: unable to SetThreadContext\n");
                        exit (1);
                     }

                     break;
                  case STATUS_SINGLE_STEP:
                     // There is some single-stepping at the end of the loader,
                     // this is normal, let it continue
                     context.EFlags |= SINGLE_STEP_FLAG;
                     SetThreadContext (processInformation.hThread, &context);
                     break;
                  default:
                     DefaultHandler ("AWAIT_REMOTE_LOADER_BP", &debugEvent, &processInformation);
                     break;
               }
            } else {
               DefaultHandler ("AWAIT_REMOTE_LOADER_BP", &debugEvent, &processInformation);
            }
            break;
         default:
            DefaultHandler ("AWAIT_REMOTE_LOADER_BP", &debugEvent, &processInformation);
            break;
      }
      ContinueDebugEvent
         (debugEvent.dwProcessId, debugEvent.dwThreadId, DBG_CONTINUE);
   }

   while (TRUE) {
      // RUNNING STATE
      // Child process runs within x86 determiniser.
      // Waiting for a single-step event, or exit.

      run = TRUE;
      while (run) {
         rc = WaitForDebugEvent (&debugEvent, INFINITE);
         if (!rc) {
            err_printf ("RUNNING: WaitForDebugEvent: error %d\n", (int) GetLastError());
            exit (1);
         }
         switch (debugEvent.dwDebugEventCode) {
            case EXCEPTION_DEBUG_EVENT:
               if ((debugEvent.dwProcessId == processInformation.dwProcessId)
               && (debugEvent.dwThreadId == processInformation.dwThreadId)) {
                  CONTEXT context;
                  context.ContextFlags = CONTEXT_FULL;
                  rc = GetThreadContext (processInformation.hThread, &context);
                  if (!rc) {
                     err_printf ("RUNNING: GetThreadContext: error %d\n",
                        (int) GetLastError());
                     exit (1);
                  }
                  switch (debugEvent.u.Exception.ExceptionRecord.ExceptionCode) {
                     case STATUS_SINGLE_STEP:
                        // Reached single step; run single step handler.
                        dbg_printf
                          ("RUNNING: Single step at %p\n", 
                           (void *) context.Eip);
                        fflush (stdout);
                        run = FALSE;
                        StartSingleStepProc
                          (singleStepProc,
                           processInformation.hProcess,
                           &context);
                        context.ContextFlags = CONTEXT_FULL;
                        SetThreadContext (processInformation.hThread, &context);
                        break;
                     default:
                        DefaultHandler ("RUNNING", &debugEvent, &processInformation);
                        break;
                  }
               } else {
                  DefaultHandler ("RUNNING", &debugEvent, &processInformation);
               }
               break;
            default:
               DefaultHandler ("RUNNING", &debugEvent, &processInformation);
               break;
         }
         ContinueDebugEvent
            (debugEvent.dwProcessId, debugEvent.dwThreadId, DBG_CONTINUE);
      }

      // SINGLE_STEP STATE
      // Single step handler runs within x86 determiniser.
      // Waiting for a breakpoint at the end of the handler.

      while (!run) {
         rc = WaitForDebugEvent (&debugEvent, INFINITE);
         if (!rc) {
            err_printf ("SINGLE_STEP: WaitForDebugEvent: error %d\n", (int) GetLastError());
            exit (1);
         }
         switch (debugEvent.dwDebugEventCode) {
            case EXCEPTION_DEBUG_EVENT:
               if ((debugEvent.dwProcessId == processInformation.dwProcessId)
               && (debugEvent.dwThreadId == processInformation.dwThreadId)) {
                  CONTEXT context;
                  context.ContextFlags = CONTEXT_FULL;
                  rc = GetThreadContext (processInformation.hThread, &context);
                  if (!rc) {
                     err_printf ("SINGLE_STEP: GetThreadContext: error %d\n",
                        (int) GetLastError());
                     exit (1);
                  }
                  switch (debugEvent.u.Exception.ExceptionRecord.ExceptionCode) {
                     case STATUS_BREAKPOINT:
                        // single step procedure finished
                        // EAX contains an error code, or 0x102 on success
                        // EBX is pointer to context, altered by remote
                        if (context.Eax != 0x102) {
                           err_printf ("SINGLE_STEP: error code 0x%x\n", (int) context.Eax);
                           return 1;
                        }
                        // context restored
                        //dbg_printf ("location of context: %p\n", (void *) context.Ebx);
                        ReadProcessMemory
                          (processInformation.hProcess,
                           (void *) context.Ebx,
                           (void *) &context,
                           sizeof (CONTEXT),
                           NULL);
                        context.ContextFlags = CONTEXT_FULL;
                        SetThreadContext (processInformation.hThread, &context);
                        run = TRUE;
                        break;
                     default:
                        DefaultHandler ("SINGLE_STEP", &debugEvent, &processInformation);
                        break;
                  }
               } else {
                  DefaultHandler ("SINGLE_STEP", &debugEvent, &processInformation);
               }
               break;
            default:
               DefaultHandler ("SINGLE_STEP", &debugEvent, &processInformation);
               break;
         }
         ContinueDebugEvent
            (debugEvent.dwProcessId, debugEvent.dwThreadId, DBG_CONTINUE);
      }
   }
   return 0;
}

/* Handle a debug event without doing anything special */
void DefaultHandler (const char * state, DEBUG_EVENT * pDebugEvent,
      PROCESS_INFORMATION * pProcessInformation)
{
   switch (pDebugEvent->dwDebugEventCode) {
      case CREATE_PROCESS_DEBUG_EVENT:
         err_printf ("%s: received a second CREATE_PROCESS_DEBUG_EVENT "
               "from an unexpected process\n", state);
         exit (1);
         break;
      case EXIT_PROCESS_DEBUG_EVENT:
         if (pDebugEvent->dwProcessId == pProcessInformation->dwProcessId) {
            dbg_printf ("%s: Process exited! %d\n", state, (int) pDebugEvent->dwProcessId);
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
                  err_printf
                    ("%s: Reached "
                     "unexpected breakpoint at %p, error code 0x%x\n",
                     state, (void *) context.Eip, (int) context.Eax);
                  exit (1);
                  break;
               case STATUS_SINGLE_STEP:
                  err_printf ("%s: Unexpected single step at %p\n", state, (void *) context.Eip);
                  exit (1);
                  break;
               default:
                  // pass through
                  dbg_printf ("%s: Exception at %p\n", state, (void *) context.Eip);
                  break;
            }
         }
         break;
      case CREATE_THREAD_DEBUG_EVENT:
         if (pDebugEvent->dwProcessId == pProcessInformation->dwProcessId) {
            dbg_printf ("%s: New thread %p\n", state, (void *) pDebugEvent->u.CreateThread.hThread);
         }
         break;
      case EXIT_THREAD_DEBUG_EVENT:
         if (pDebugEvent->dwProcessId == pProcessInformation->dwProcessId) {
            dbg_printf ("%s: Exit thread %p\n", state, (void *) pDebugEvent->dwThreadId);
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
}

