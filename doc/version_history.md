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

## 2.1 (April 2020)

Version 2.1 was a bug fix release. It fixed a critical bug for 64-bit Linux
which could cause stack corruption in programs that use the "red zone". A new test
checks for this specific bug.

The "complex test" was redesigned to avoid the need for a checked-in executable,
making it more portable across Linux distributions and across CPU microarchitectures
(the test failed on CPUs with AVX). Finally, the last dependency on Python 2 was
removed. Python 3 is now preferred.




