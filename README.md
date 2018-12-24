# x86determiniser
"Simulator" with branch tracing and deterministic timing for x86 32-bit
programs, with system call passthrough.

This is a interpreter for x86 programs running on Linux and Windows. The
primary purpose of the interpreter is to make the RDTSC instruction produce
the exact instruction count rather than a clock cycle count. Using this
interpreter, and given the same input, any two runs of the same program will
always obtain the same values from RDTSC.

The secondary purpose of the interpreter is to generate a branch trace. This
records the source and target address of every taken branch, including
calls, returns and unconditional branches. It also records the source address
of each not-taken conditional branch.

Library calls and system calls do not form part of the instruction count.
They are executed normally, at full speed, by native code.

In order to write this interpreter quickly, I have made heavy use of the x86
CPU itself. Only branches are interpreted: straight-line code is executed
natively. Special control flow instructions may be executed by single-stepping.

The interpreter will only work with 32-bit x86 code. It uses the Zydis
x86 disassembler. It has only been tested with code compiled with GCC.

The interpreter can be trivially extended to produce a trace of
executed instructions, register values, memory accesses and so forth.

Aside from Zydis, the code is copyright (c) 2015-2018 by Jack Whitham.

x86determiniser has been relicenced under the MIT License 
so that non-GPL software may be linked to it and distributed.

