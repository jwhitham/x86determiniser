
# Build instructions

There are four supported platforms: win32, win64, linux32, linux64.
You can pick one or more. 

## Windows

The build requirements are:

* [Git for Windows](https://gitforwindows.org/).
* GNU make - you can install MSYS2, or find the 
  [make package](http://repo.msys2.org/msys/x86_64/). Extract make.exe with `tar xf`
  and put it somewhere on the PATH.
* GCC - I have tested using the GCC versions
  provided by [Adacore](https://www.adacore.com/): see below.
* [Python](https://www.python.org/) - needed for running test cases. Python 3 is
  preferred but Python 2 should work too.
 
When these requirements are installed, edit `paths.cfg` to configure the location of the
compilers for the platforms you wish to build, then use `make win32` or `make win64`.
A program is built within the `bin` directory and then tested as part of the build process.
If the tests complete successfully, you will see a message such as
`simple_tests completed ok for win64`.

## Linux

The build requirements are the same as Windows. Use your package manager to
install them if they are not already present. `apt-get install build-essential git`
will probably get everything you need.

As for Windows, edit `paths.cfg` to configure the location of the
compilers for the platforms you wish to build, then use `make linux32` or `make linux64`.
A program is built within the `bin` directory and then tested as part of the build process.
If the tests complete successfully, you will see a message such as
`simple_tests completed ok for linux64`.

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




