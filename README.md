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

# Documentation

[About x86determiniser](doc/about_x86determiniser.md)

[Version history](doc/version_history.md)

[Build instructions](doc/build_instructions.md)

[Migration from version 1.x](doc/migration.md)

