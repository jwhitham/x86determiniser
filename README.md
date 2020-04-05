# x86determiniser
x86determiniser is a
"simulator" with [branch tracing](https://en.wikipedia.org/wiki/Branch_trace),
instruction tracing
and deterministic timing for x86 32-bit and 64-bit programs
on Windows and Linux. It operates by "syscall emulation"
and runs native executables. There's a longer description
[on my website](https://www.jwhitham.org/2020/04/x86determiniser.html)
along with [precompiled binaries](https://www.jwhitham.org/x86determiniser/bin/).

## Example:

  `C> x64determiniser.exe --inst-trace test.txt example.exe`

test.txt will contain a list of all instructions executed by the
"example.exe" program, excluding those in library code (not within
the .exe file).

`x64determiniser.exe --help` prints out a list of other options.

# Documentation

[About x86determiniser](doc/about_x86determiniser.md)

[Version history](doc/version_history.md)

[Build instructions](doc/build_instructions.md)

[Migration from version 1.x](doc/migration.md)

