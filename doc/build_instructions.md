
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



