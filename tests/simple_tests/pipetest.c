#include <stdio.h>
#include <ctype.h>


int main (void)
{
   while (1) {
      int ch = fgetc (stdin);

      if (ch == EOF) {
         return 0;
      }

      if (isalpha (ch)) {
         fputc (ch, stdout);
      } else if (isdigit (ch)) {
         fputc (ch, stderr);
      } else {
         fputc (ch, stdout);
         fputc (ch, stderr);
      }
      fflush (stdout);
      fflush (stderr);
   }
}

