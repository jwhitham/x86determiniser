# x86determiniser
x86determiniser is a 
"simulator" with branch tracing and deterministic timing for x86 32-bit
and 64-bit programs, with system call passthrough.

# Example:

   C> x64determiniser.exe --inst-trace test.txt ..\\tests\\simple\_tests\\args.win64.exe

test.txt will contain a list of all instructions executed by the
"args.win64.exe" program, excluding those in library code (not within
the .exe file).

# "Determinising"

x86determiniser was developed to allow x86 programs to be simulated in
a deterministic environment. That is,
no matter how many times a program is executed
with particular inputs, it will always take the same time to run. Here,
"time" is execution time measured using the RDTSC instruction, rather than
any external clock.

Generally, programs running on x86 CPUs do not have precisely predictable
execution times, because the execution time is affected by the state of
the CPU caches, pipeline, branch predictor and virtual memory system, and
also the activities of other CPU cores. This state is not predictable
at the beginning of the program, and every system call or context switch
introduces more entropy.

This was a problem, because the timing analysis tool made by my company
must ship with example software and tutorials which demonstrate how a 
program's execution time is measured. It's convenient if the programs
being measured can run natively on the user's PC - however, if we do that,
then we can't have predictable timing. The initial solution was to run
the example programs in a simulator for a simple embedded system, but this meant
shipping a cross compiler with our tools, which was not practical,
particularly as the complexity of the examples grew to include the
Ada runtime. I found that I really wanted an x86 simulator, but one with
system call passthrough, so that a native x86 program could be
executed in simulation and produce a predictable execution time. This
would be easy on Linux - we might use the "qemu-i386" program, for example,
and modify QEMU so that RDTSC produces an instruction count. Or we might
make a frontend for "valgrind". But on Windows, for whatever reason,
nobody had made a simulator like that. There are full-system simulators
for x86 (QEMU is one) but nothing has system call passthrough. If you want
to simulate a Windows program, you must also simulate Windows.

I set out to try to build a simulator that did as little simulating as
possible, because x86 is a very complex architecture and simulators are
hard to write. x86determiniser is the result. It avoids simulation as
far as possible by executing code directly - instrumentation is added
(and removed) dynamically so that the program appears to execute normally,
but the instructions are counted, and instructions such as RDTSC and OUT
are treated specially.

Library calls and system calls do not form part of the instruction count.
They are executed normally, at full speed, by native code.

x86determiniser initially relied on GNU objdump to disassemble code,
but now uses the Zydis disassembler. It also once required programs
to link directly against x86determiniser.dll and execute a setup function
on startup, but now, programs do not have to be modified because they
are started by x86determiniser.exe, which acts as a debugger and program
loader. x86determiniser is now also able to capture instruction traces (in
text format) and branch traces (in an encoded format) and this may be
useful as a way to debug and analyse any program, not necessarily one
requiring deterministic execution.

# Limitations

x86determiniser is unlikely to be very fast. There is a lot of context
switching. There is nothing like QEMU or valgrind's system for
JIT translation of machine code. It is however faster than single-stepping.

The interpreter will work with 32-bit and 64-bit x86 code. It uses the Zydis
x86 disassembler. It has only been tested with code compiled with GCC.

The interpreter can be trivially extended to produce a trace of
executed instructions, register values, memory accesses and so forth.

# Licensing and credits

Aside from the Zydis disassembler,
the code is copyright (c) 2015-2019 by Jack Whitham.

x86determiniser has been relicenced under the MIT License (same as Zydis)
so that non-GPL software may be linked to it and distributed.

