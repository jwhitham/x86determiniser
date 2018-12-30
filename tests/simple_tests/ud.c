
volatile char * null_ptr = (char *) 0;

int main (int argc, char ** argv)
{
   if (argc < 2) {
      return 0;
   }
   switch (argv[1][0]) {
      case '1':
         __asm__ volatile ("ud1");
         return 1;
      case '2':
         __asm__ volatile ("ud2");
         return 1;
      case '3':
         __asm__ volatile ("hlt");
         return 1;
      case '4':
         *null_ptr = '\0';
         return 1;
      case '5':
         if (*null_ptr) {
            return 2;
         }
         return 1;
      default:
         return 0;
   }
}

