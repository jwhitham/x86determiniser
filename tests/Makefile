
PLATFORM=win32
ROOT=..
include ../$(PLATFORM).cfg

all: complex_test/ctest.$(PLATFORM).ok simple_tests/tests.$(PLATFORM).ok

clean:
	make -C complex_test clean PLATFORM=$(PLATFORM)
	make -C simple_tests clean PLATFORM=$(PLATFORM)

# ctest: branch trace of C program
complex_test/ctest.$(PLATFORM).ok: 
	make -C complex_test ctest.$(PLATFORM).ok PLATFORM=$(PLATFORM)

simple_tests/tests.$(PLATFORM).ok: 
	make -C simple_tests tests.$(PLATFORM).ok PLATFORM=$(PLATFORM)


