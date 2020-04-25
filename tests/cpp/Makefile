
PLATFORM=UNDEFINED
ROOT=../..
include ../../cfg/$(PLATFORM).cfg

X86DRUN=$(LOADER_EXE) --out-trace x.txt --
TCPP=tcpp.$(PLATFORM)$(EXE)

all: tests.$(PLATFORM).ok

clean:
	-rm -f *.o $(TCPP) tests.$(PLATFORM).ok

$(TCPP):
	$(GCC_DIR)/g++ -O2 -g -o $(TCPP) test.cc -static-libstdc++ -static-libgcc

tests.$(PLATFORM).ok: $(TCPP)
	echo 'Two runs produce the same timings'
	$(X86DRUN) $(TCPP) > 1.$(PLATFORM).csv
	$(X86DRUN) $(TCPP) > 2.$(PLATFORM).csv
	cmp 1.$(PLATFORM).csv 2.$(PLATFORM).csv
	echo 'Good'
	touch test.$(PLATFORM).ok

