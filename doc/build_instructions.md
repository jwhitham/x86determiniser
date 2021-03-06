
# Build instructions

There are four supported platforms: win32, win64, linux32, linux64.
You can pick one or more. 

## Windows

The build requirements are:

* [Git for Windows](https://gitforwindows.org/).
* GNU make - you can install MSYS2, or find the 
  [make package](http://repo.msys2.org/msys/x86_64/). Extract `make.exe` with `tar xf`
  and put it somewhere on the PATH.
* GCC - I have tested using the GCC versions
  provided by [Adacore](https://www.adacore.com/) so I recommend you
  [use those](https://www.adacore.com/download/more)
  though there's no reason other GCC versions won't also work. See "Tested GCC versions" below.
* [Python](https://www.python.org/) - needed for running test cases. Python 3 is
  preferred. Make sure `python.exe` is on the PATH.
 
When these requirements are installed, edit `paths.cfg` to configure the location of the
compilers for the platforms you wish to build, then use `make win32` or `make win64`.

A program is built within the `bin` directory and then tested as part of the build process.
If the tests complete successfully, you will see a message such as
`simple_tests completed ok for win64`.

## Ubuntu 18.04 64-bit (64-bit build)

Use the package manager to install the required build tools:

```
   sudo apt install git make build-essential
```

Edit `paths.cfg` to set the location of the compiler, and set the
`-no-pie` flag where it's required:

```
   LINUX64_GCC_DIR=/usr/bin
   ADDITIONAL_GCC_FLAGS=-no-pie
```
   
(Ubuntu's GCC produces position-independent executables by default,
but two test cases don't support this.)

Having done this, you can build the 64-bit version with `make linux64`.
A program is built within the `bin` directory and then tested as part of the build process.
If the tests complete successfully, you will see a message such as
`simple_tests completed ok for linux64`.

## Ubuntu 18.04 64-bit (32-bit build)

If you want to build the 32-bit version on Ubuntu 18.04, you will need 32-bit compatibility
libraries, i.e. `sudo apt install gcc-multilib libc6-dev:i386`.
I recommend you do a 64-bit build first.

The 32-bit compatibility libraries do not install a 32-bit version of GCC, they just enable
32-bit support in the 64-bit version of GCC, so I found it easiest to install a
32-bit only version of GCC (GNAT GPL 2014). I also needed to delete the `ld` program from 
`gnat-gpl-2014-x86-linux-bin/libexec/gcc/i686-pc-linux-gnu/4.7.4/ld` as it
appears to be incompatible with Ubuntu's (newer) libraries. `-no-pie` is not required for
this compiler, but you must still set the compiler location in `paths.cfg` before
running `make linux32`.


## Generic Linux

The build requirements are the same as Windows. As for Windows, I suggest installing
the GNAT compilers. Aside from those, you require Python (ideally Python 3),
GNU make and Git.

Edit `paths.cfg` to configure the location of the
compilers for the platforms you wish to build, then use `make linux32` or `make linux64`.

## Tested GCC versions

x86determiniser has been tested with the following GCC versions
provided by [Adacore](https://www.adacore.com/):

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


