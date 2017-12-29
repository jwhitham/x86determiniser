#include <windows.h>

#include "remote_loader.h"

void RemoteLoaderStart (void) {}

void RemoteLoader (CommStruct * cs)
{
   void * hm;
   HMODULE WINAPI (* loadLibrary) (LPCSTR lpLibFileName);
   FARPROC WINAPI (* getProcAddress) (HMODULE hModule, LPCSTR lpProcName);
   int (* Loader) (CommStruct *);

   loadLibrary = cs->loadLibraryProc;
   getProcAddress = cs->getProcAddressProc;

   hm = loadLibrary (cs->libraryName);
   if (!hm) {
      asm volatile ("mov 0x001, %eax\nint3\n");
      return;
   }
   Loader = (void *) getProcAddress (hm, cs->procName);
   if (!Loader) {
      asm volatile ("mov 0x002, %eax\nint3\n");
      return;
   }
   Loader (cs);
   asm volatile ("mov 0x003, %eax\nint3\n");
   return;
}

void RemoteLoaderEnd (void) {}


