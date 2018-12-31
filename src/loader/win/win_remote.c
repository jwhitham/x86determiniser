#include <windows.h>

#include "remote_loader.h"
#include "common.h"

void RemoteLoaderStart (void) {}

void RemoteLoader (CommStruct * cs)
{
   void * hm;
   HMODULE WINAPI (* loadLibrary) (LPCSTR lpLibFileName);
   FARPROC WINAPI (* getProcAddress) (HMODULE hModule, LPCSTR lpProcName);
   void (* x86DeterminiserStartup) (CommStruct *);
   int rc = FAILED_UNKNOWN;

   loadLibrary = cs->loadLibraryProc;
   getProcAddress = cs->getProcAddressProc;

   hm = loadLibrary (cs->libraryName);
   if (!hm) {
      rc = FAILED_LOADLIBRARY;
      goto error;
   }
   x86DeterminiserStartup = (void *) getProcAddress (hm, cs->procName);
   if (!x86DeterminiserStartup) {
      rc = FAILED_GETPROCADDRESS;
      goto error;
   }
   x86DeterminiserStartup (cs);
error:
   /* similar to x86_bp_trap but we can't use that from this procedure */
   __asm__ volatile ("mov %0, %%eax\nmov $0, %%ebx\nint3" : : "r"(rc));
}

void RemoteLoaderEnd (void) {}


