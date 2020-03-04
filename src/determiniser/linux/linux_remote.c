
#include "remote_loader.h"
#include "common.h"


static void RemoteLoader(void) __attribute__((constructor));

static CommStruct cs;



static void RemoteLoader (void)
{

   /* Get a copy of the CommStruct from the parent process */
   asm volatile
     ("mov %0, %%eax\n"
      "int3\n"
      "jmp 0f\n"
      ".ascii \"RemoteLoader\"\n"
      "0:\n" : : "r"(cs));

   x86DeterminiserStartup (cs);
}


