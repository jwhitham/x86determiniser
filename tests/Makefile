
PLATFORM=UNDEFINED
ROOT=..
include ../cfg/$(PLATFORM).cfg

OK_FILES=complex_test/ctest.$(PLATFORM).ok simple_tests/tests.$(PLATFORM).ok

all:
	rm -f $(OK_FILES)
	make $(OK_FILES)

clean:
	make -C complex_test clean PLATFORM=$(PLATFORM)
	make -C simple_tests clean PLATFORM=$(PLATFORM)

# ctest: branch trace of C program
complex_test/ctest.$(PLATFORM).ok:
	make -C complex_test ctest.$(PLATFORM).ok PLATFORM=$(PLATFORM)

simple_tests/tests.$(PLATFORM).ok:
	make -C simple_tests tests.$(PLATFORM).ok PLATFORM=$(PLATFORM)

version1:
	make -C version1shim PLATFORM=$(PLATFORM)

