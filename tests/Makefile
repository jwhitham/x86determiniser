
PLATFORM=UNDEFINED
ROOT=..
include ../cfg/$(PLATFORM).cfg

OK_FILES=complex_test/ctest.$(PLATFORM).ok simple_tests/tests.$(PLATFORM).ok
MORE_OK_FILES=sox123/tests.$(PLATFORM).ok

most:
	rm -f $(OK_FILES)
	make $(OK_FILES)

all:
	rm -f $(MORE_OK_FILES) $(OK_FILES)
	make $(OK_FILES) $(MORE_OK_FILES)

clean:
	make -C complex_test clean PLATFORM=$(PLATFORM)
	make -C simple_tests clean PLATFORM=$(PLATFORM)
	make -C sox123 clean PLATFORM=$(PLATFORM)

# ctest: branch trace of C program
complex_test/ctest.$(PLATFORM).ok:
	make -C complex_test ctest.$(PLATFORM).ok PLATFORM=$(PLATFORM)

simple_tests/tests.$(PLATFORM).ok:
	make -C simple_tests tests.$(PLATFORM).ok PLATFORM=$(PLATFORM)

sox123/tests.$(PLATFORM).ok:
	make -C sox123 tests.$(PLATFORM).ok PLATFORM=$(PLATFORM)

