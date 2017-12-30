#include <windows.h>

#include "remote_loader.h"

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
      asm volatile ("mov $0x001, %eax\nint3\n");
      return;
   }
   x86DeterminiserStartup = (void *) getProcAddress (hm, cs->procName);
   if (!x86DeterminiserStartup) {
      asm volatile ("mov $0x002, %eax\nint3\n");
      return;
   }
   x86DeterminiserStartup (cs);
   asm volatile ("mov $0x003, %eax\nint3\n");
   return;
}

void RemoteLoaderEnd (void) {}


