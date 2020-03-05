
#include "remote_loader.h"
#include "common.h"



void X86DeterminiserStartup (CommStruct * pcs);


void RemoteLoader (CommStruct * pcs)
{
   unsigned code = COMPLETED_REMOTE;

   // Get a copy of the CommStruct from the parent process.
   // We send a message with EAX = COMPLETED_REMOTE and EBX = pcs
   __asm__ volatile ("int3" : : "a"(code), "b"(pcs));

   X86DeterminiserStartup (pcs);
}


static void init(void) __attribute__((constructor));

static CommStruct cs;

static void init(void)
{
   RemoteLoader (&cs);
}

