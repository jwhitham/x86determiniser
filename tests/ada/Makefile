
PLATFORM=UNDEFINED
ROOT=../..
include ../../cfg/$(PLATFORM).cfg

X86DRUN=$(LOADER_EXE) --out-trace x.txt --
TQ=test_queue.$(PLATFORM)$(EXE)

all: tests.$(PLATFORM).ok

clean:
	-rm -f *.ali *.o $(TQ) tests.$(PLATFORM).ok

$(TQ):
	-rm -f *.ali *.o
	$(GCC_DIR)/gcc -c rdtsc.S 
	$(GCC_DIR)/gnatmake -O2 -g -o $(TQ) test_queue.adb -largs rdtsc.o

tests.$(PLATFORM).ok: $(TQ)
	echo 'Two runs produce the same timings'
	$(X86DRUN) $(TQ) > 1.txt
	$(X86DRUN) $(TQ) > 2.txt
	cmp 1.txt 2.txt
	echo 'Good'
	touch test.$(PLATFORM).ok

