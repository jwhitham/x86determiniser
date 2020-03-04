
#include "remote_loader.h"
#include "common.h"



void x86DeterminiserStartup (CommStruct * pcs);


void RemoteLoader (CommStruct * pcs)
{

   /* Get a copy of the CommStruct from the parent process */
   __asm__ volatile
     ("mov %0, %%eax\n"
      "int3\n"
      "jmp 0f\n"
      ".ascii \"RemoteLoader\"\n"
      "0:\n" : : "r"(pcs));

   x86DeterminiserStartup (pcs);
}


static void init(void) __attribute__((constructor));

static CommStruct cs;

static void init(void)
{
   RemoteLoader (&cs);
}

