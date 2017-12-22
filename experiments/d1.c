#define _WIN32_WINNT 0x600
#include <stdio.h>
#include <string.h>
#include <Windows.h>

/*
DWORD WINAPI GetFinalPathNameByHandle(
  HANDLE hFile,
  LPTSTR lpszFilePath,
  DWORD  cchFilePath,
  DWORD  dwFlags
); */
#define SINGLE_STEP_FLAG 0x100

int main(void)
{
   STARTUPINFO startupInfo;
   PROCESS_INFORMATION processInformation;
   DEBUG_EVENT debugEvent;
   DWORD dwCreationFlags;
   BOOL rc, run = TRUE;
   char buf[128];
   SIZE_T len;
   LPVOID ptr;
   CONTEXT context;
   BOOL do_single_step = FALSE;

   memset (&startupInfo, 0, sizeof (startupInfo));
   memset (&processInformation, 0, sizeof (processInformation));
   memset (&debugEvent, 0, sizeof (debugEvent));
   memset (&context, 0, sizeof (context));
   startupInfo.cb = sizeof (startupInfo);

   dwCreationFlags = DEBUG_PROCESS | DEBUG_ONLY_THIS_PROCESS;
   rc = CreateProcess(
     /* _In_opt_    LPCTSTR               */ "d0.exe",
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

   while (run) {
      do_single_step = FALSE;
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
            do_single_step = TRUE;
            break;
         case CREATE_THREAD_DEBUG_EVENT:
            // create an additional thread (no single stepping in this thread, runs freely)
            break;
         case EXCEPTION_DEBUG_EVENT:
            if ((debugEvent.dwProcessId == processInformation.dwProcessId)
            && (debugEvent.dwThreadId == processInformation.dwThreadId)) {
               switch (debugEvent.u.Exception.ExceptionRecord.ExceptionCode) {
                  case STATUS_SINGLE_STEP:
                     do_single_step = TRUE;
                     break;
                  default:
                     break;
               }
            }
            break;
            
   
         // case CREATE_THREAD_DEBUG_EVENT:

#if 0
            // debugEvent.u.CreateProcessInfo.lpStartAddress,

            // allocates space in debugged process
            remoteBuf = VirtualAllocEx
              (processInformation.hProcess,
               NULL,
               dwLength,
               MEM_RESERVE | MEM_COMMIT,
               PAGE_EXECUTE_READWRITE);
            // writes stuff into memory within debugged process
            WriteProcessMemory
              (processInformation.hProcess,
               remoteBuf,
               localBuf,
               dwLength,
               NULL);
            
            // Run code in remote process
            CreateRemoteThread
              (processInformation.hProcess,
               NULL,
               /* stack size */,
               remoteBuf,
               /* lpParameter */,
               /* */,
               &dwThreadId);
#endif
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
            break;
         case UNLOAD_DLL_DEBUG_EVENT:
            printf ("UnloadDll: %p\n", debugEvent.u.UnloadDll.lpBaseOfDll);
#if 0
            if (debugEvent.u.LoadDll.lpImageName == NULL) {
               printf ("Process loads DLL with null name\n");
               break;
            } 
            len = sizeof (ptr);
            ptr = NULL;
            rc = ReadProcessMemory
              (processInformation.hProcess,
               debugEvent.u.LoadDll.lpImageName,
               &ptr,
               sizeof (ptr),
               &len);
            if (!rc) {
               printf ("ReadProcessMemory: error %d (2)\n", (int) GetLastError());
               return 1;
            }
            if (!ptr) {
               printf ("Process loads DLL with null name (2)\n");
               break;
            }
            len = sizeof (buf);
            rc = ReadProcessMemory
              (processInformation.hProcess,
               ptr,
               buf,
               sizeof (buf) - 1,
               &len);
            if (!rc) {
               printf ("ReadProcessMemory: error %d (3)\n", (int) GetLastError());
               return 1;
            }
            if (!len) {
               printf ("Process loads DLL with null name (3)\n");
               break;
            }
            buf[len - 1] = '\0';
            printf ("process loads DLL '%s'\n", buf);
#endif
            break;
         case EXIT_PROCESS_DEBUG_EVENT:
            printf ("Process exited! %d\n", (int) debugEvent.dwProcessId);
            run = FALSE;
            break;
         default:
            printf("Event %d\n", (int) debugEvent.dwDebugEventCode);
            break;
      }

      if (do_single_step) {
         SuspendThread (processInformation.hThread);
         context.ContextFlags = CONTEXT_CONTROL;
         rc = GetThreadContext (processInformation.hThread, &context);
         if (!rc) {
            printf ("GTC: error %d\n", (int) GetLastError());
            return 1;
         }
         unsigned char opcode = 0;
         const DWORD current_pc = context.Eip;

         if (get_classification (previous_pc) == UNKNOWN) {
            if ((current_pc > (previous_pc + 15))
            || (current_pc < previous_pc))
         }
         if (get_classification (current_pc) == UNKNOWN) {
            rc = ReadProcessMemory
              (processInformation.hProcess,
               (void *) current_pc,
               &opcode,
               1,
               NULL);
            if (!rc) {
               printf ("ReadProcessMemory: error %d\n", (int) GetLastError());
               return 1;
            }
            if (opcode >= 0x70 && opcode <= 0x7f) {
               // It's a conditional jump opcode
               classification = CONDITIONAL_JUMP;
               set_classification (current_pc, classification);
            }
         }


         } else if (classification == FIRST_TRY) {
            rc = ReadProcessMemory
              (processInformation.hProcess,
               (void *) previous_pc,
               &opcode,
               1,
               NULL);
         }

               } else {
                  set_classification (current_pc, CONDITIONAL_JUMP);
               }
               printf ("%02x ", v);

         

         previous_pc = context.Eip;
         context.ContextFlags = CONTEXT_CONTROL;
         context.EFlags |= SINGLE_STEP_FLAG;
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
