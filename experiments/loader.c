#define _WIN32_WINNT 0x600
#include <stdio.h>
#include <string.h>
#include <Windows.h>
#include <Psapi.h>

#include "remote_loader.h"

/*
DWORD WINAPI GetFinalPathNameByHandle(
  HANDLE hFile,
  LPTSTR lpszFilePath,
  DWORD  cchFilePath,
  DWORD  dwFlags
); */
#define SINGLE_STEP_FLAG 0x100

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
   // printf ("location of context: %p\n", (void *) localCs.pcontext);

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
   PCONTEXT context)
{
   ssize_t space;
   void * remoteBuf;
   CommStruct localCs;
   BOOL rc;

   memset (&localCs, 0, sizeof (localCs));

   space = (char *) RemoteLoaderEnd - (char *) RemoteLoaderStart;
   if (space <= 0) {
      printf ("REMOTE_LOADER: No valid size for remote loader\n");
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
      printf ("REMOTE_LOADER: Unable to allocate %d bytes for remote loader\n", (int) space);
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
      printf ("REMOTE_LOADER: Unable to inject %d bytes\n", (int) space);
      exit (1);
   }

   // reserve the right amount of stack space
   context->Esp -= sizeof (localCs);

   // build data structure to load into the remote stack
   localCs.myself = (void *) context->Esp;
   strncpy (localCs.libraryName, "remote.dll", MAX_LIBRARY_NAME_SIZE);
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
      printf ("REMOTE_LOADER: Unable to fill remote stack\n");
      exit (1);
   }

   // run RemoteLoader procedure until breakpoint
   context->Eip = 
      (DWORD) ((void *) ((char *) remoteBuf +
         ((char *) RemoteLoader - (char *) RemoteLoaderStart)));
   context->EFlags &= ~SINGLE_STEP_FLAG;
}


int main(void)
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

   memset (&startupInfo, 0, sizeof (startupInfo));
   memset (&processInformation, 0, sizeof (processInformation));
   memset (&debugEvent, 0, sizeof (debugEvent));
   memset (&startContext, 0, sizeof (startContext));
   startupInfo.cb = sizeof (startupInfo);

   dwCreationFlags = DEBUG_PROCESS | DEBUG_ONLY_THIS_PROCESS;
   rc = CreateProcess(
     /* _In_opt_    LPCTSTR               */ "target.exe",
     /* _Inout_opt_ LPTSTR                */ NULL /* lpCommandLine */,
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
      printf ("CreateProcess: error %d\n", (int) GetLastError());
      return 1;
   }

   // Determine offset of LoadLibrary and GetProcAddress
   // within kernel32.dll
   {
      HMODULE kernel32 = LoadLibrary ("kernel32.dll");
      MODULEINFO modinfo;
      LPVOID pa;

      if (!kernel32) {
         printf ("LoadLibrary: error %d\n", (int) GetLastError());
         return 1;
      }
      if (!GetModuleInformation
            (GetCurrentProcess(), kernel32, &modinfo, sizeof(MODULEINFO))) {
         printf ("GetModuleInformation: error %d\n", (int) GetLastError());
         return 1;
      }
      printf ("kernel32.dll: base %p size %u\n", modinfo.lpBaseOfDll, (unsigned) modinfo.SizeOfImage);

      pa = GetProcAddress (kernel32, "GetProcAddress");
      if (!pa) {
         printf ("GetProcAddress: error %d\n", (int) GetLastError());
         return 1;
      }
      getProcAddressOffset = (char *) pa - (char *) modinfo.lpBaseOfDll;

      pa = GetProcAddress (kernel32, "LoadLibraryA");
      if (!pa) {
         printf ("GetProcAddress: error %d\n", (int) GetLastError());
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
         printf ("INITIAL: WaitForDebugEvent: error %d\n", (int) GetLastError());
         exit (1);
      }
      switch (debugEvent.dwDebugEventCode) {
         case CREATE_PROCESS_DEBUG_EVENT:
            if ((debugEvent.dwProcessId != processInformation.dwProcessId)
            || (debugEvent.dwThreadId != processInformation.dwThreadId)) {
               printf ("INITIAL: CREATE_PROCESS_DEBUG_EVENT from unexpected process\n");
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
               printf ("INITIAL: Unable to read first instruction\n");
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
               printf ("INITIAL: Unable to replace first instruction with breakpoint\n");
               exit (1);
            }
            break;
         default:
            break;
      }
      ContinueDebugEvent
         (debugEvent.dwProcessId, debugEvent.dwThreadId, DBG_CONTINUE);
   }
   printf ("INITIAL: startAddress = %p\n", startAddress);

   // AWAIT_FIRST STAGE
   // Await breakpoint at first instruction
   // Capture startContext once it's reached
   // Also capture the kernel32.dll base address, which we should see
   // before the first instruction
   while (!startContext.Eip) {
      rc = WaitForDebugEvent (&debugEvent, INFINITE);
      if (!rc) {
         printf ("AWAIT_FIRST: WaitForDebugEvent: error %d\n", (int) GetLastError());
         exit (1);
      }
      switch (debugEvent.dwDebugEventCode) {
         case CREATE_PROCESS_DEBUG_EVENT:
            printf ("AWAIT_FIRST: CREATE_PROCESS_DEBUG_EVENT from unexpected process\n");
            exit (1);
            break;
         case EXCEPTION_DEBUG_EVENT:
            if ((debugEvent.dwProcessId == processInformation.dwProcessId)
            && (debugEvent.dwThreadId == processInformation.dwThreadId)) {
               CONTEXT context;

               context.ContextFlags = CONTEXT_FULL;
               rc = GetThreadContext (processInformation.hThread, &context);
               if (!rc) {
                  printf ("AWAIT_FIRST: GetThreadContext: error %d\n",
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
                        printf ("REMOTE_LOADER: don't know the kernel32.dll base address\n");
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
                        printf ("REMOTE_LOADER: Unable to restore first instruction\n");
                        exit (1);
                     }

                     StartRemoteLoader
                       (getProcAddressOffset,
                        loadLibraryOffset,
                        kernel32Base,
                        (void *) startAddress,
                        processInformation.hProcess,
                        &context);
                     SetThreadContext (processInformation.hThread, &context);

                     break;
                  default:
                     printf ("AWAIT_FIRST: unhandled debug event 0x%08x EIP %p\n",
                        (int) debugEvent.u.Exception.ExceptionRecord.ExceptionCode,
                        (void *) context.Eip);
                     exit (1);
                     break;
               }
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
         printf ("AWAIT_REMOTE_LOADER_BP: WaitForDebugEvent: error %d\n", (int) GetLastError());
         exit (1);
      }
      switch (debugEvent.dwDebugEventCode) {
         case CREATE_PROCESS_DEBUG_EVENT:
            printf ("AWAIT_REMOTE_LOADER_BP: CREATE_PROCESS_DEBUG_EVENT from unexpected process\n");
            exit (1);
            break;
         case EXCEPTION_DEBUG_EVENT:
            if ((debugEvent.dwProcessId == processInformation.dwProcessId)
            && (debugEvent.dwThreadId == processInformation.dwThreadId)) {
               CONTEXT context;
               context.ContextFlags = CONTEXT_FULL;
               rc = GetThreadContext (processInformation.hThread, &context);
               if (!rc) {
                  printf ("AWAIT_REMOTE_LOADER_BP: GetThreadContext: error %d\n",
                     (int) GetLastError());
                  exit (1);
               }
               switch (debugEvent.u.Exception.ExceptionRecord.ExceptionCode) {
                  case STATUS_BREAKPOINT:
                     // EAX contains an error code, or 0x101 on success
                     if (context.Eax != 0x101) {
                        printf
                          ("AWAIT_REMOTE_LOADER_BP: Reached "
                           "unexpected breakpoint at %p, error code %d\n",
                           (void *) context.Eip, (int) context.Eax);
                        exit (1);
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
                        printf ("REMOTE_LOADER_BP: unable to SetThreadContext\n");
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
                     printf ("AWAIT_REMOTE_LOADER_BP: unhandled debug event 0x%08x EIP %p\n",
                        (int) debugEvent.u.Exception.ExceptionRecord.ExceptionCode,
                        (void *) context.Eip);
                     exit (1);
                     break;
               }
            }
            break;
         case EXIT_PROCESS_DEBUG_EVENT:
            if (debugEvent.dwProcessId == processInformation.dwProcessId) {
               printf ("AWAIT_REMOTE_LOADER_BP: Process exited! %d\n", (int) debugEvent.dwProcessId);
               exit (0);
            }
            break;
         default:
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
            printf ("RUNNING: WaitForDebugEvent: error %d\n", (int) GetLastError());
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
                     printf ("RUNNING: GetThreadContext: error %d\n",
                        (int) GetLastError());
                     exit (1);
                  }
                  switch (debugEvent.u.Exception.ExceptionRecord.ExceptionCode) {
                     case STATUS_BREAKPOINT:
                        printf
                          ("RUNNING: Reached "
                           "unexpected breakpoint at %p\n",
                           (void *) context.Eip);
                        exit (1);
                        break;
                     case STATUS_SINGLE_STEP:
                        // Reached single step; run single step handler.
                        printf
                          ("RUNNING: Single step at %p\n", 
                           (void *) context.Eip);
                        run = FALSE;
                        StartSingleStepProc
                          (singleStepProc,
                           processInformation.hProcess,
                           &context);
                        context.ContextFlags = CONTEXT_FULL;
                        SetThreadContext (processInformation.hThread, &context);
                        break;
                     default:
                        printf ("RUNNING: unhandled debug event 0x%08x\n",
                           (int) debugEvent.u.Exception.ExceptionRecord.ExceptionCode);
                        exit (1);
                        break;
                  }
               }
               break;
            case EXIT_PROCESS_DEBUG_EVENT:
               if (debugEvent.dwProcessId == processInformation.dwProcessId) {
                  printf ("RUNNING: Process exited! %d\n", (int) debugEvent.dwProcessId);
                  exit (0);
               }
               break;
            default:
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
            printf ("SINGLE_STEP: WaitForDebugEvent: error %d\n", (int) GetLastError());
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
                     printf ("SINGLE_STEP: GetThreadContext: error %d\n",
                        (int) GetLastError());
                     exit (1);
                  }
                  switch (debugEvent.u.Exception.ExceptionRecord.ExceptionCode) {
                     case STATUS_SINGLE_STEP:
                        printf ("SINGLE_STEP: unexpectedly stepped within handler\n");
                        exit (1);
                        break;
                     case STATUS_BREAKPOINT:
                        // single step procedure finished
                        // EAX contains an error code, or 0x102 on success
                        // EBX is pointer to context, altered by remote
                        if (context.Eax != 0x102) {
                           printf ("SINGLE_STEP: error code %d\n", (int) context.Eax);
                           return 1;
                        }
                        // context restored
                        //printf ("location of context: %p\n", (void *) context.Ebx);
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
                        printf ("SINGLE_STEP: unhandled debug event 0x%08x EIP %p\n",
                           (int) debugEvent.u.Exception.ExceptionRecord.ExceptionCode,
                           (void *) context.Eip);
                        exit (1);
                        break;
                  }
               }
               break;
            case EXIT_PROCESS_DEBUG_EVENT:
               if (debugEvent.dwProcessId == processInformation.dwProcessId) {
                  printf ("SINGLE_STEP: Process exited! %d\n", (int) debugEvent.dwProcessId);
                  exit (0);
               }
               break;
            default:
               break;
         }
         ContinueDebugEvent
            (debugEvent.dwProcessId, debugEvent.dwThreadId, DBG_CONTINUE);
      }
   }
   return 0;
}

#if 0
               
               SuspendThread (processInformation.hThread);
               context.ContextFlags = CONTEXT_FULL;
               rc = GetThreadContext (processInformation.hThread, &context);
               if (!rc) {
                  printf ("GTC: error %d\n", (int) GetLastError());
                  return 1;
               }
               printf ("single step eip %p: ", (void *) context.Eip);
               context.ContextFlags = CONTEXT_FULL;
               context.EFlags |= 0x100; // single step flag
               rc = SetThreadContext (processInformation.hThread, &context);
               if (!rc) {
                  printf ("STC: error %d\n", (int) GetLastError());
                  return 1;
               }
               int i;
               for (i = 0; i < 6; i++) {
                  unsigned char v = 0;
                  rc = ReadProcessMemory
                    (processInformation.hProcess,
                     (void *) (context.Eip + i),
                     &v,
                     1,
                     &len);
                  if (!rc) {
                     printf ("ReadProcessMemory: error %d\n", (int) GetLastError());
                     return 1;
                  }
                  printf ("%02x ", v);
               }
               printf ("\n");
               ResumeThread (processInformation.hThread);

#endif
