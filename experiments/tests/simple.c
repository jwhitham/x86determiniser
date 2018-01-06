#include <stdio.h>


int main (int argc, char ** argv)
{
   const char x[] = "hello world\n";
   int i;

   for (i = 0; x[i] != '\0'; i++) {
      fputc (x[i], stdout);
      fflush (stdout);
   }
   for (i = 0; i < argc; i++) {
      printf ("[%s]\n", argv[i]);
   }
   return 0;
}

