#include "remote_loader.h"

void RemoteLoaderStart (void) {}

void RemoteLoader (CommStruct * cs)
{
   void * hm;
   int (* loaderProc) (CommStruct *);

   hm = cs->loadLibraryProc (cs->libraryName);
   if (!hm) {
      cs->errorProc (1);
      return;
   }
   loaderProc = (void *) cs->getProcAddressProc (hm, cs->procName);
   if (!loaderProc) {
      cs->errorProc (2);
      return;
   }
   cs->errorProc (loaderProc (cs));
   return;
}

void RemoteLoaderBP (int errorCode)
{
   asm volatile ("mov %%eax, %0\nint3\n" : : "r"(errorCode) );
}

void RemoteLoaderEnd (void) {}


