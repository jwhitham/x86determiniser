
PLATFORM=UNDEFINED
ROOT=../..
include ../../cfg/$(PLATFORM).cfg
ALLOW_REBUILD=0

all: ctest.$(PLATFORM).ok

clean:
	-rm -rf ctest.trace \
      *.trace *.ok


ctest.$(PLATFORM).ok: $(LOADER_EXE) ctest.$(PLATFORM)$(EXE)
	$(LOADER_EXE) --branch-trace ctest.$(PLATFORM).trace ctest.$(PLATFORM)$(EXE)
	python tests.py $(PLATFORM)

ctest.$(PLATFORM)$(EXE): main.c all_tests.$(PLATFORM).S
	$(CC) -o $@ $(CFLAGS) $(ASFLAGS) \
      all_tests.$(PLATFORM).S main.c -ldl $(ASFLAGS)

ifeq ($(ALLOW_REBUILD),1)

# These rules are only used when the test is created.
# Do this on Linux.

ASM=t0.S t1.$(PLATFORM).S t2.S t3.S t4.S t5.S t6.$(PLATFORM).S

t1.$(PLATFORM).S: t1.c
	$(CC) -S -O2 -o $@ $<
	sed -i 's/[.]L/t1_L/g' $@

t6.$(PLATFORM).S: t6.c
	$(CC) -S -O2 -o $@ $<
	sed -i 's/[.]L/t6_L/g' $@

all_tests.$(PLATFORM).S: $(ASM)
	cat $(ASM) > $@
	sed -i 's/^[.]glob.*//g' $@
	sed -i 's/[.]section.*note[.].*/.text/g' $@
	sed -i 's/[.]section.*text[.].*/.text/g' $@
	echo '.global all_tests' >> $@
	echo '.global _all_tests' >> $@

# Here is a reference file made with the old version.
# Only possible with 32-bit Linux.

ctest.$(PLATFORM).ref: ctest.$(PLATFORM)$(EXE)
	cp /tmp/x86determiniser/libx86determiniser$(DLL) .
	X86D_BRANCH_TRACE=tmp.txt \
      ./ctest.$(PLATFORM)$(EXE) ./libx86determiniser$(DLL)
	python3 normalise_trace.py tmp.txt ctest.$(PLATFORM).ref

else

all_tests.linux%.S:
	@echo all_tests.$(PLATFORM).S should already be present
all_tests.win%.S: all_tests.linux%.S
	cp $< $@

endif

