#include <stdint.h>
#include <stdio.h>


void all_tests (void);

int main(void)
{
#ifdef USE_OLD_VERSION_TO_MAKE_A_REFERENCE_FILE
   uint32_t ER[2];
   void startup_x86_determiniser (uint32_t * ER);
   startup_x86_determiniser (ER);
#endif

   printf ("start\n");
   all_tests ();
   printf ("stop\n");
   return 0;
}


