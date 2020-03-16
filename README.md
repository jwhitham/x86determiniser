# x86determiniser
x86determiniser is a
"simulator" with [branch tracing](https://en.wikipedia.org/wiki/Branch_trace),
instruction tracing
and deterministic timing for x86 32-bit and 64-bit programs
on Windows and Linux. It operates by "system call passthrough"
and loads native executables.

## Example:

  `C> x64determiniser.exe --inst-trace test.txt ..\tests\simple_tests\args.win64.exe`

test.txt will contain a list of all instructions executed by the
"args.win64.exe" program, excluding those in library code (not within
the .exe file).

`--help` prints out a list of other supported options.


# About x86determiniser

x86determiniser was developed to allow x86 programs to be simulated in
a deterministic environment. That is,
no matter how many times a program is executed
with particular inputs, it will always take the same time to run. Here,
"time" is execution time measured using the RDTSC instruction, rather than
any external clock. Instructions are only counted if they are part of
the main thread of the executable: instructions in a DLL, other threads
and the operating system are ignored.

## Branch traces

x86determiniser also generates
[branch traces](https://en.wikipedia.org/wiki/Branch_trace). These capture the
execution path through a program. The branch trace records the source
and destination address of every branch that is taken, including
unconditional branches, calls and returns. It also records the address
of every branch that is not taken. Each trace record is "timestamped"
with an instruction count. The `doc` subdirectory contains a
description of the branch trace format.

## Repeatable and predictable tests

x86determiniser was created to help test a timing analysis tool named
[RapiTime](https://www.rapitasystems.com/products/rapitime),
made by [Rapita Systems](https://www.rapitasystems.com/).
Programs running on x86 CPUs do not have precisely predictable
execution times, but deterministic execution was needed in order to make
reliable and repeatable test cases for continuous integration.

Other simulators did not meet the requirements for this, because
we wanted to compile programs on both Windows and Linux hosts, and then
run them in simulation. Usually, simulators do not support
"[syscall emulation](https://qemu.weilnetz.de/doc/qemu-doc.html#QEMU-User-space-emulator)", also known as "system call passthrough"
or "userspace emulation". Instead, simulators normally simulate
an entire system, which means that we cannot take native programs from
the host and run them in the simulator, because we must also simulate the
OS and the hardware. [QEMU](https://www.qemu.org/)
does "userspace emulation" on Linux
but not on Windows, while the [valgrind](https://www.valgrind.org/)
family of simulators do
not work on Windows at all. A search for Windows-based simulators
with such features came up with nothing.

I set out to try to build a simulator that did as little simulating as
possible, because x86 is a very complex architecture and simulators are
hard to write. x86determiniser is the result. It avoids simulation as
far as possible by executing code directly - instrumentation is added
(and removed) dynamically so that the program appears to execute normally,
but the instructions are counted, and instructions such as RDTSC and OUT
are treated specially.

## Limitations

x86determiniser is unlikely to be very fast. There is a lot of context
switching. There is nothing like QEMU or valgrind's system for
JIT translation of machine code. It is much faster than single-stepping,
though I recognise that this is a low barrier.

The interpreter will work with 32-bit and 64-bit x86 code. It uses the Zydis
x86 disassembler. It has only been tested with code compiled with GCC.

x86determiniser versions 1.0 to 1.2 were extensively used for internal
testing at Rapita while I was working there. Version 2.0 was completed
after I had left, but regression tests in the `tests` directory check
the current behaviour against reference output from 1.2.

The interpreter can be trivially extended to produce other traces.

## Licensing and credits

Aside from the Zydis disassembler,
the code is copyright (c) 2015-2020 by Jack Whitham.

x86determiniser has been relicensed under the MIT License (same as Zydis)
so that non-GPL software may be linked to it and distributed.


# Version History

## 1.0 (January 2016)

Version 1.0 relied on GNU objdump to disassemble code. It supported
32-bit Windows and Linux only. Programs had to link against the x86determiniser
DLL and call the `startup_x86_determiniser` function during `main`. This
version used the GNU General Public License.

## 1.1 (June 2018)

Version 1.1 changed the branch trace format in order to provide more information
and support testing for
[another tool](https://www.rapitasystems.com/products/features/zero-footprint-timing-analysis)
being developed at Rapita, allowing repeatable tests to be written.

## 1.2 (December 2018)

Version 1.2 changed the behaviour of IN and OUT instructions to allow custom events
to be written to a branch trace file. The license was changed to MIT.

## 2.0 (March 2020)

Version 2.0 added support for 64-bit platforms. Programs no longer needed
to link against any x86determiniser DLL or call any library function during `main`:
x86determiniser was now an application rather than a DLL, and could run
unmodified Linux and Windows programs when executed as follows:

   `C> x64determiniser.exe --inst-trace test.txt example.exe`

GNU objdump was no longer required, as the [Zydis](https://zydis.re) disassembler
was now embedded within x86determiniser. (Zydis and x86determiniser share the MIT
License.)

x86determiniser became able to generate an instruction trace (with disassembly)
as well as branch traces.

# Build instructions

There are four supported platforms: win32, win64, linux32, linux64. You can pick
one or more. The supported compiler is GCC, and I have tested using the GCC versions
provided by [Adacore](https://www.adacore.com/), which are:

Platform | Tested GCC version | Adacore compiler name
-------- | ------------------ | ---------------------
linux32  | 4.7.4 20140401     | x86 GNU Linux (32 bits) 2014
linux64  | 6.3.1 20170510     | x86 GNU Linux (64 bits) 2017
win32    | 6.3.1 20170510     | x86 Windows (32 bits) 2017
win64    | 8.3.1 20190518     | x86 Windows (64 bits) 2019

You can install one of these compilers (they are 
[free downloads](https://www.adacore.com/download/more)) or you can use
whichever GCC you already have. There is no particular need to use an Adacore
compiler, but I have used this configuration anyway in order to test x86determiniser
with programs written in Ada.

Edit `paths.cfg` to configure the location of the
compilers for the platforms you wish to build, then use `make win32`, `make linux64` etc.
A program is built within the `bin` directory and then tested as part of the build process.
If the tests complete successfully, you will see a message such as
`simple_tests completed ok for win64`.


