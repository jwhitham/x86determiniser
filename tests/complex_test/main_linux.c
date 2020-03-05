
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>

void test_1 (void);
void test_3 (void);
void test_4 (void);
void test_5 (void);
void test_6 (void);
void test_7 (void);
void start_test (void);
void stop_test (void);

int main(int argc, char ** argv)
{
   uint32_t ER[2];
   void (* startup_x86_determiniser) (uint32_t * ER) = NULL;

   if (argc == 2) {
      void * lib = dlopen (argv[1], RTLD_NOW);
      if (!lib) {
         return 1;
      }
      startup_x86_determiniser = (void *) dlsym (lib, "startup_x86_determiniser");
      if (!startup_x86_determiniser) {
         return 2;
      }
      startup_x86_determiniser (ER);
   }

   printf ("start\n");
   start_test ();
   test_1 ();
   test_3 ();
   test_4 ();
   test_5 ();
   test_6 ();
   test_7 ();
   stop_test ();
   printf ("stop\n");
   return 0;
}

