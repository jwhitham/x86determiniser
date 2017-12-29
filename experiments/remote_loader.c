#include <windows.h>

#include "remote_loader.h"

void RemoteLoaderStart (void) {}

void RemoteLoader (CommStruct * cs)
{
   void * hm;
   HMODULE WINAPI (* loadLibrary) (LPCSTR lpLibFileName);
   FARPROC WINAPI (* getProcAddress) (HMODULE hModule, LPCSTR lpProcName);
   int (* Loader) (CommStruct *);
   void (* Error) (int errorCode);

   loadLibrary = cs->loadLibraryProc;
   getProcAddress = cs->getProcAddressProc;
   Error = cs->errorProc;

   hm = loadLibrary (cs->libraryName);
   if (!hm) {
      Error (1);
      return;
   }
   Loader = (void *) getProcAddress (hm, cs->procName);
   if (!Loader) {
      Error (2);
      return;
   }
   Error (Loader (cs));
   return;
}

void RemoteLoaderBP (int errorCode)
{
   asm volatile ("mov %%eax, %0\nint3\n" : : "r"(errorCode) );
}

void RemoteLoaderEnd (void) {}


