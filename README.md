# x86determiniser
x86determiniser is a 
"simulator" with branch tracing, instruction tracing
and deterministic timing for x86 32-bit and 64-bit programs
on Windows and Linux. It operates by "system call passthrough"
and loads native executables.

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
any external clock. Instructions are only counted if they are part of
the main thread of the executable: instructions in a DLL, other threads
and the operating system are ignored.

# Branch tracing

x86determiniser also generates branch traces. These capture the
execution path through a program. The branch trace records the source
and destination address of every branch that is taken, including
unconditional branches, calls and returns. It also records the address
of every branch that is not taken. Each trace record is "timestamped"
with an instruction count. The "doc" subdirectory contains a
description of the branch trace format.

# Purposes of x86determiniser

x86determiniser was created to help test a timing analysis tool named
RapiTime, made by [Rapita Systems](https://www.rapitasystems.com/).
Programs running on x86 CPUs do not have precisely predictable
execution times, but deterministic execution was needed in order to make
reliable test cases for continuous integration.

Other simulators did not meet the requirements for this, because
we wanted to compile programs on both Windows and Linux hosts, and then
run them in simulation. Usually, simulators do not support
"[syscall emulation](https://qemu.weilnetz.de/doc/qemu-doc.html#QEMU-User-space-emulator)", also known as "system call passthrough"
or "userspace emulation". Instead, it simulates
an entire system, which means that we cannot take native programs from
the host and run them in the simulator, because we must also simulate the
OS and the hardware. [QEMU](https://www.qemu.org/)
does "userspace emulation" on Linux
but not on Windows, while the [valgrind](https://www.valgrind.org/)
family of simulators do
not work on Windows at all. A search for Windows-based simulators
with such features came up with nothing.

x86determiniser was subsequently extended to generate branch traces in order
to support another tool being developed at Rapita, allowing
repeatable tests to be written.

I set out to try to build a simulator that did as little simulating as
possible, because x86 is a very complex architecture and simulators are
hard to write. x86determiniser is the result. It avoids simulation as
far as possible by executing code directly - instrumentation is added
(and removed) dynamically so that the program appears to execute normally,
but the instructions are counted, and instructions such as RDTSC and OUT
are treated specially.

x86determiniser initially relied on GNU objdump to disassemble code,
but now uses the [Zydis](https://zydis.re) disassembler. 
It also once required programs
to link directly against x86determiniser.dll and execute a setup function
on startup, but now, programs do not have to be modified because they
are started by x86determiniser.exe, which acts as a debugger and program
loader. x86determiniser is now also able to capture instruction traces (in
text format) and branch traces (in an encoded format) and this may be
useful as a way to debug and analyse any program, not necessarily one
requiring deterministic execution.

Initially x86determiniser only supported 32-bit code but it now supports
both 32-bit and 64-bit programs via two different entry points:
x86determiniser.exe (32-bit) and x64determiniser.exe (64-bit).


# Limitations

x86determiniser is unlikely to be very fast. There is a lot of context
switching. There is nothing like QEMU or valgrind's system for
JIT translation of machine code. It is much faster than single-stepping,
though I recognise that this is a low barrier.

The interpreter will work with 32-bit and 64-bit x86 code. It uses the Zydis
x86 disassembler. It has only been tested with code compiled with GCC.

The interpreter can be trivially extended to produce other traces.

# Licensing and credits

Aside from the Zydis disassembler,
the code is copyright (c) 2015-2020 by Jack Whitham.

x86determiniser has been relicenced under the MIT License (same as Zydis)
so that non-GPL software may be linked to it and distributed.

