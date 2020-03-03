
#include "remote_loader.h"
#include "common.h"

void RemoteLoaderStart (void) {}

void RemoteLoader (CommStruct * cs)
{
   void * hm;
   HMODULE WINAPI (* loadLibrary) (LPCSTR lpLibFileName);
   FARPROC WINAPI (* getProcAddress) (HMODULE hModule, LPCSTR lpProcName);
   DWORD WINAPI (* getLastError) (void);
   void (* x86DeterminiserStartup) (CommStruct *);
   int xax = FAILED_UNKNOWN;
   int xbx = 0;

   loadLibrary = cs->loadLibraryProc;
   getProcAddress = cs->getProcAddressProc;
   getLastError = cs->getLastErrorProc;

   hm = loadLibrary (cs->libraryName);
   if (!hm) {
      xax = FAILED_LOADLIBRARY;
      xbx = getLastError();
      goto error;
   }
   x86DeterminiserStartup = (void *) getProcAddress (hm, cs->procName);
   if (!x86DeterminiserStartup) {
      xax = FAILED_GETPROCADDRESS;
      xbx = getLastError();
      goto error;
   }
   x86DeterminiserStartup (cs);
error:
   /* similar to x86_bp_trap but we can't use that from this procedure */
   __asm__ volatile ("mov %0, %%eax\nmov %1, %%ebx\nint3" : : "r"(xax), "r"(xbx));
}

void RemoteLoaderEnd (void) {}


