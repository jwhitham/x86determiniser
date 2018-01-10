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

   loadLibrary = cs->loadLibraryProc;
   getProcAddress = cs->getProcAddressProc;

   hm = loadLibrary (cs->libraryName);
   if (!hm) {
      asm volatile ("int3" : : "eax"(FAILED_LOADLIBRARY));
      return;
   }
   x86DeterminiserStartup = (void *) getProcAddress (hm, cs->procName);
   if (!x86DeterminiserStartup) {
      asm volatile ("int3" : : "eax"(FAILED_GETPROCADDRESS));
      return;
   }
   x86DeterminiserStartup (cs);
   asm volatile ("int3" : : "eax"(FAILED_UNKNOWN));
   return;
}

void RemoteLoaderEnd (void) {}


