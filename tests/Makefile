
PLATFORM=win32
ROOT=..
include ../$(PLATFORM).cfg

all: complex_test/ctest.ok simple_tests/tests.ok

clean:
	make -C complex_test clean PLATFORM=$(PLATFORM)
	make -C simple_tests clean PLATFORM=$(PLATFORM)

# ctest: branch trace of C program
complex_test/ctest.ok: 
	make -C complex_test ctest.ok PLATFORM=$(PLATFORM)

simple_tests/tests.ok: 
	make -C simple_tests tests.ok PLATFORM=$(PLATFORM)


