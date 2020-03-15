# "complex test"
This regression test mixes C and x86 assembly code with the intention
of exposing bugs in the x86determiniser simulation. The reference trace,
ctest.ref, was produced using the previous version and the current version
is expected to produce exactly the same output after minimal processing.

The test includes the following:
* t1.c - recursion and looping in C
* t2.S - tail recursion in assembly
* t3.S - test of the CALL + POP sequence for PIC on x86
* t4.S - branch pattern and some unusual instructions
* t5.S - memset written in assembly code with unusual instructions
* t6.c - C functions from zlib, tiny-AES-C, libgcc
* t7.c - math functions statically linked from mingw

The test uses code from:
* https://github.com/kokke/tiny-AES-C/
* https://github.com/madler/zlib/blob/master/contrib/puff/


