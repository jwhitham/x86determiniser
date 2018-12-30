#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>


#include "common.h"
#include "remote_loader.h"

int X86DeterminiserLoader(CommStruct * cs, int argc, char ** argv);

int main (int argc, char ** argv)
{
   CommStruct cs;
   memset (&cs, 0, sizeof (cs));
   strcpy (cs.internalVersionCheck, INTERNAL_VERSION);

   while (1) {
      static const char version[] =
         "X86Determiniser version " X86D_VERSION "\n"
         "copyright (c) 2015-2018 by Jack Whitham\n"
         "https://github.com/jwhitham/x86determiniser\n";
      static const char help[] =
         "\n"
        //1                                                                              80 
         "X86Determiniser will run an x86 program in a 'simulation' by rewriting basic\n"
         "blocks, emulating some instructions and single-stepping others as required to\n"
         "act as an interpreter for x86 machine code. RDTSC execution timestamps are\n"
         "always deterministic, and system/library calls are executed at full speed.\n"
         "Optionally, a branch trace may be produced, and OUT instructions can be\n"
         "recorded.\n\n"
         "Basic usage:\n"
         "   x86determiniser [options] -- <program name> [program args...]\n\n"
         "Options:\n"
        //1                                                                              80 
         "   --out-trace <file name>     write values of OUT instructions to <file name>\n"
         "   --branch-trace <file name>  write addresses of branches to <file name>\n"
         "   --inst-trace <file name>    disassemble + write instructions to <file name>\n"
         "   --debug                     debug x86determiniser itself (more output)\n"
         "   --version / --help          print information about x86determiniser itself\n\n";
      static struct option long_options[] = {
         {"out-trace", 1, 0, 'o'},
         {"branch-trace", 1, 0, 'b'},
         {"inst-trace", 1, 0, 'i'},
         {"debug", 0, 0, 'd'},
         {"version", 0, 0, 'v'},
         {"help", 0, 0, '?'},
         {NULL, 0, 0, 0},
      };
      int option_index = 0;
      int c = getopt_long (argc, argv, "o:b:dv",
         long_options, &option_index);

      switch (c) {
         case 'o':
            strncpy (cs.outTrace, optarg, MAX_FILE_NAME_SIZE - 1);
            break;
         case 'b':
            strncpy (cs.branchTrace, optarg, MAX_FILE_NAME_SIZE - 1);
            break;
         case 'i':
            strncpy (cs.instTrace, optarg, MAX_FILE_NAME_SIZE - 1);
            break;
         case 'd':
            cs.debugEnabled = 1;
            break;
         case 'v':
            fputs (version, stdout);
            exit (0);
            break;
         case -1:
            // Reached end of options: launch now
            if ((optind >= argc) || (argc < 2)) {
               fputs (version, stdout);
               fputs (help, stdout);
               exit (1);
            }
            exit (X86DeterminiserLoader (&cs, argc - optind, &argv[optind]));
         default:
            fputs (version, stdout);
            fputs (help, stdout);
            exit (1);
      }
   }
}

