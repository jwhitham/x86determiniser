# Migration from version 1.x

When upgrading from version 1.x to version 2.0, some changes
may be required.

## In version 2.0, there's no DLL or startup function

x86determiniser version 1.x required each program to link against
a DLL (or shared object) named `libx86determiniser.dll`. This DLL
exported one function, `startup_x86_determiniser`, which had to
be called from the program. It would normally be called within
`main`:

```
   #include <stdint.h>
   void startup_x86_determiniser (uint32_t * ER);

   int main (int argc, char ** argv)
   {
      uint32_t ER[2];
      startup_x86_determiniser (ER);

      // ... stuff to be determinised ...

      return 0;
    }

```

In version 2.0, x86determiniser no longer has a DLL or any
`startup_x86_determiniser` function. Instead, the program is
'determinised' from the point where the executable actually begins
to run. The first instruction to be determinised is the entry point of the
executable, often named `_start` or `_mainCRTStartup`. As before, any
shared library code and system calls are ignored, so the dynamic loader
(e.g. `ld-linux.so.2`) is not determinised. 

## In version 2.0, run the program within x86determiniser

In version 1.x, programs are linked against x86determiniser's DLL
and then started normally.

In version 2.0, x86determiniser acts like a loader or debugger, so programs
are started as follows:

  `C> x64determiniser.exe [options] program.exe`

## In version 2.0, the X86D environment variables have no effect

Version 1.x used the `X86D_QUIET_MODE` to turn off verbose output
and `X86D_BRANCH_TRACE` to allow a branch trace file to be specified.

Version 2.0 does not have these environment variables. Instead,
specify the required options on the command line. `--branch-trace`
replaces `X86D_BRANCH_TRACE` and `X86D_QUIET_MODE` is the default:
specify `--debug` for extra output.

## In version 2.0, tracing starts at the entry to the executable

In version 1.x, code would be 'determinised' from the point where 
`startup_x86_determiniser` returned, to the point where `main` exited.
Any shared library or system calls would be ignored. 

In version 2.0, the program is
'determinised' from the point where the executable actually begins
to run. Even in small C programs, there is plenty of initialisation 
before `main` is reached.

Some applications may depend on x86determiniser's version 1.x behaviour:
usually because a branch trace is being captured.
In version 1.x, branch traces are not produced until
`startup_x86_determiniser` returns.

If an application depends on version 1.x behaviour, then it may be
restored using the `--await` option, which produces no instruction
or branch trace output until the first `in` or `out` instruction
is reached. Use `--await` on the x86determiniser command line,
and replace the call to `startup_x86_determiniser` with assembly code
such as the following:
  
```
   int main (int argc, char ** argv)
   {
      unsigned ignore;
      __asm__ volatile ("in $0x30,%%eax" : "=a"(ignore));

      // ... stuff to be determinised ...

      return 0;
   }
```   

The determinising process still runs from the program entry point,
but no trace is produced until this `in` instruction is reached,
restoring version 1.x branch trace behaviour.



