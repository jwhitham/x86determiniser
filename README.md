# x86determiniser
"Simulator" with instruction tracing and deterministic timing for x86 32-bit programs.

This is a interpreter for x86 programs running on Linux and Windows. The
purpose of the interpreter is to make the RDTSC instruction produce the exact
instruction count rather than a clock cycle count. Using this interpreter,
and given the same input, any two runs of the same program will always obtain
the same values from RDTSC.

Library calls and system calls do not form part of the instruction count:
they are executed normally, at full speed, by native code.

In order to write this interpreter quickly, I have made heavy use of the x86 CPU
itself. Only branches and jumps are interpreted: straight-line code is executed
natively. Also, if a branch or jump cannot be interpreted, because its encoding
is not supported by my simple instruction decoder, then it will be executed in
single-step mode.

As a result of this design, the interpreter will only work with 32-bit x86 code.
It requires the "objdump" program to be present. It has only been tested with code
compiled with GCC.

The interpreter can be trivially extended to produce a trace of executed instructions,
register values, memory accesses and so forth.

All of the code is copyright (c) 2015-2018 by Jack Whitham.

x86determiniser has been relicenced under the MIT License
so that non-GPL software may be linked to it and distributed.

