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

typedef enum {
   AWAIT_START,
   AWAIT_KERNEL32_LOAD,
   AWAIT_FIRST_INSTRUCTION,
   AWAIT_REMOTE_LOADER_BP,
   REMOTE_LOADER_BP,
   RUNNING,
   SINGLE_STEP,
   AWAIT_SINGLE_STEP_BP,
   SINGLE_STEP_BP,
} STATE;

typedef struct SingleStepStruct {
   void * unused;
   struct SingleStepStruct * myself;
   CONTEXT context;
} SingleStepStruct;


void StartSingleStepProc
  (void * singleStepProc,
   HANDLE hProcess,
   CONTEXT * context)
{
   SingleStepStruct localCs;

   memcpy (&localCs.context, context, sizeof (CONTEXT));

   // reserve stack space for SingleStepStruct
   context->Esp -= sizeof (localCs);
   localCs.myself = (void *) context->Esp;
   localCs.unused = NULL;

   // fill remote stack
   // return address is NULL (1st item in struct: don't return!!)
   // first parameter is "myself" (2nd item in struct)
   WriteProcessMemory
     (hProcess,
      localCs.myself,
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
   HANDLE hProcess,
   CONTEXT * context)
{
   ssize_t space;
   void * remoteBuf;
   CommStruct localCs;

   memset (&localCs, 0, sizeof (localCs));

   space = (char *) RemoteLoaderEnd - (char *) RemoteLoaderStart;
   if (space <= 0) {
      printf ("No valid size for remote loader\n");
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
      printf ("Unable to allocate %d bytes for remote loader\n", (int) space);
      exit (1);
   }

   // writes remote loader into memory within debugged process
   WriteProcessMemory
     (hProcess,
      remoteBuf,
      RemoteLoaderStart,
      (DWORD) space,
      NULL);

   // reserve the right amount of stack space
   context->Esp -= sizeof (localCs);

   // build data structure to load into the remote stack
   localCs.myself = (void *) context->Esp;
   strncpy (localCs.libraryName, "remote.dll", MAX_LIBRARY_NAME_SIZE);
   strncpy (localCs.procName, "startup_x86_determiniser", MAX_PROC_NAME_SIZE);
   localCs.loadLibraryProc = 
      (void *) ((char *) kernel32Base + loadLibraryOffset);
   localCs.getProcAddressProc =
      (void *) ((char *) kernel32Base + getProcAddressOffset);

   // fill remote stack
   // return address is NULL (1st item in struct: don't return!!)
   // first parameter is "myself" (2nd item in struct)
   WriteProcessMemory
     (hProcess,
      localCs.myself,
      &localCs,
      sizeof (localCs),
      NULL);

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
   CONTEXT stepContext;
   size_t getProcAddressOffset = 0;
   size_t loadLibraryOffset = 0;
   LPVOID kernel32Base = NULL;
   LPVOID startAddress = NULL;
   LPVOID singleStepProc = NULL;
   STATE state = AWAIT_START;

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
   printf ("Spawned %d\n", (int) processInformation.dwProcessId);

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
      printf ("gpa: offset %u\n", (unsigned) getProcAddressOffset);

      pa = GetProcAddress (kernel32, "LoadLibraryA");
      if (!pa) {
         printf ("GetProcAddress: error %d\n", (int) GetLastError());
         return 1;
      }
      loadLibraryOffset = (char *) pa - (char *) modinfo.lpBaseOfDll;
      printf ("ll: offset %u\n", (unsigned) loadLibraryOffset);
   }


   while (run) {
      rc = WaitForDebugEvent (&debugEvent, INFINITE);
      if (!rc) {
         printf ("WaitForDebugEvent: error %d\n", (int) GetLastError());
         return 1;
      }
      switch (debugEvent.dwDebugEventCode) {
         case CREATE_PROCESS_DEBUG_EVENT:
            if ((debugEvent.dwProcessId != processInformation.dwProcessId)
            || (debugEvent.dwThreadId != processInformation.dwThreadId)) {
               printf ("CREATE_PROCESS_DEBUG_EVENT from unexpected process\n");
               return 1;
            }
            if (state == AWAIT_START) {
               startAddress = debugEvent.u.CreateProcessInfo.lpStartAddress;
               state = AWAIT_KERNEL32_LOAD;
               printf ("state == AWAIT_KERNEL32_LOAD: %p\n", (void *) startAddress);
            }
            break;
         case EXCEPTION_DEBUG_EVENT:
            if ((debugEvent.dwProcessId == processInformation.dwProcessId)
            && (debugEvent.dwThreadId == processInformation.dwThreadId)) {
               switch (debugEvent.u.Exception.ExceptionRecord.ExceptionCode) {
                  case STATUS_SINGLE_STEP:
                     if (state == RUNNING) {
                        state = SINGLE_STEP;
                     } else if (state == AWAIT_FIRST_INSTRUCTION) {
                        // ok
                     } else {
                        printf ("single step in unexpected state\n");
                        exit (1);
                     }
                     break;
                  case STATUS_BREAKPOINT:
                     if (state == AWAIT_REMOTE_LOADER_BP) {
                        state = REMOTE_LOADER_BP;
                        printf ("state == REMOTE_LOADER_BP\n");
                     } else if (state == AWAIT_SINGLE_STEP_BP) {
                        state = SINGLE_STEP_BP;
                        printf ("state == SINGLE_STEP_BP\n");
                     } else {
                        printf ("breakpoint in unexpected state\n");
                        //exit (1);
                     }
                     break;
                  default:
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
            printf ("LoadDll: %s at %p\n", buf, debugEvent.u.LoadDll.lpBaseOfDll);
            CloseHandle (debugEvent.u.LoadDll.hFile);

            if (state == AWAIT_KERNEL32_LOAD) {
               /* if buf == kernel32.dll, we have the addresses for LoadLibrary
                * and GetProcAddress, so we can switch the process over to use
                * the determiniser. */

               if (strstr (buf, "\\kernel32.dll") != NULL) {
                  kernel32Base = debugEvent.u.LoadDll.lpBaseOfDll;
                  state = AWAIT_FIRST_INSTRUCTION;
                  printf ("state == AWAIT_FIRST_INSTRUCTION\n");
               }
            }
            break;
         case UNLOAD_DLL_DEBUG_EVENT:
            printf ("UnloadDll: %p\n", debugEvent.u.UnloadDll.lpBaseOfDll);
            break;
         case EXIT_PROCESS_DEBUG_EVENT:
            printf ("Process exited! %d\n", (int) debugEvent.dwProcessId);
            run = FALSE;
            break;
         default:
            printf("Event %d\n", (int) debugEvent.dwDebugEventCode);
            break;
      }

      {
         CONTEXT context;
         memset (&context, 0, sizeof (context));

         SuspendThread (processInformation.hThread);
         context.ContextFlags = CONTEXT_FULL;
         rc = GetThreadContext (processInformation.hThread, &context);
         if (!rc) {
            printf ("GTC: error %d\n", (int) GetLastError());
            return 1;
         }

         switch (state) {
            case AWAIT_START:
            case AWAIT_KERNEL32_LOAD:
               // single step until started
               // context.EFlags |= SINGLE_STEP_FLAG;
               break;
            case AWAIT_REMOTE_LOADER_BP:
            case AWAIT_SINGLE_STEP_BP:
            case RUNNING:
               // No special action
               break;
            case AWAIT_FIRST_INSTRUCTION:
               // continue single-stepping
               context.EFlags |= SINGLE_STEP_FLAG;
               if ((void *) context.Eip == startAddress) {
                  // we have single-stepped to the start address and should
                  // now run the RemoteLoader procedure
                  memcpy (&startContext, &context, sizeof (CONTEXT));
                  StartRemoteLoader
                    (getProcAddressOffset,
                     loadLibraryOffset,
                     kernel32Base,
                     processInformation.hProcess,
                     &context);
                  state = AWAIT_REMOTE_LOADER_BP;
                  printf ("state == AWAIT_REMOTE_LOADER_BP: %p\n", (void *) context.Eip);
               }
               break;
            case REMOTE_LOADER_BP:
               // reached breakpoint in remote loader
               // EAX contains an error code, or 0x101 on success
               if (context.Eax != 0x101) {
                  printf ("error code %d\n", (int) context.Eax);
                  return 1;
               }
               fflush (stdout);

               // EBX contains the address of the single step handler
               singleStepProc = (void *) context.Ebx;
               state = RUNNING;
               printf ("state == RUNNING: %p\n", (void *) context.Eip);
               // "reboot" into the original context
               memcpy (&context, &startContext, sizeof (CONTEXT));
               
               break;
            case SINGLE_STEP:
               // single step event while running
               memcpy (&stepContext, &context, sizeof (CONTEXT));
               StartSingleStepProc
                 (singleStepProc,
                  processInformation.hProcess,
                  &context);
               state = AWAIT_SINGLE_STEP_BP;
               printf ("state == AWAIT_SINGLE_STEP_DONE: %p\n", (void *) context.Eip);
               break;
            case SINGLE_STEP_BP:
               // single step procedure finished
               // EAX contains an error code, or 0x102 on success
               if (context.Eax != 0x102) {
                  printf ("error code %d\n", (int) context.Eax);
                  return 1;
               }
               // context restored
               memcpy (&context, &stepContext, sizeof (CONTEXT));
               state = RUNNING;
               printf ("state == RUNNING: %p\n", (void *) context.Eip);
               break;
         }
         context.ContextFlags = CONTEXT_FULL;
         rc = SetThreadContext (processInformation.hThread, &context);
         if (!rc) {
            printf ("STC: error %d\n", (int) GetLastError());
            return 1;
         }
         ResumeThread (processInformation.hThread);
      }
      fflush (stdout);

      ContinueDebugEvent
         (debugEvent.dwProcessId, debugEvent.dwThreadId, DBG_CONTINUE);
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
