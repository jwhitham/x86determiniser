#include <stdio.h>
#include <string.h>
#include <stdlib.h>


int X86DeterminiserLoader(int argc, char ** argv);

int main (int argc, char ** argv)
{
   exit (X86DeterminiserLoader (argc - 1, &argv[1]));
}

