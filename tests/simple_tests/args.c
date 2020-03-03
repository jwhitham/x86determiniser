#include <stdio.h>
#include <string.h>

int _CRT_glob = 0; /* don't expand wildcards when parsing command-line args */

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

