
PLATFORM=win32
ROOT=..
include ../$(PLATFORM).cfg

all: complex_test/ctest.ok simple_tests/tests.ok

clean:
	make -C complex_test clean
	make -C simple_tests clean

# ctest: branch trace of C program
complex_test/ctest.ok: 
	make -C complex_test ctest.ok

simple_tests/ctest.ok: 
	make -C simple_tests tests.ok


