#ifdef WIN32
#include <windows.h>
#define WIN
#elif WIN64
#include <windows.h>
#define WIN
#else
#include <dlfcn.h>
#endif

#include <stdint.h>
#include <stdio.h>


void all_tests (void);

int main(int argc, char ** argv)
{
   uint32_t ER[2];
   void (* startup_x86_determiniser) (uint32_t * ER) = NULL;

   if (argc == 2) {
#ifdef WIN
      HMODULE lib = LoadLibrary (argv[1]);
      if (!lib) {
         return 1;
      }
      startup_x86_determiniser = (void *) GetProcAddress (lib, "startup_x86_determiniser");
#else
      void * lib = dlopen (argv[1], RTLD_NOW);
      if (!lib) {
         return 1;
      }
      startup_x86_determiniser = (void *) dlsym (lib, "startup_x86_determiniser");
#endif
      if (!startup_x86_determiniser) {
         return 2;
      }
      startup_x86_determiniser (ER);
   }

   printf ("start\n");
   all_tests ();
   printf ("stop\n");
   return 0;
}


