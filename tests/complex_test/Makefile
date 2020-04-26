
PLATFORM=UNDEFINED
ROOT=../..
include ../../cfg/$(PLATFORM).cfg
ALLOW_REBUILD=0

all: ctest.$(PLATFORM).ok

clean:
	-rm -rf ctest.trace \
      *.trace *.ok


ctest.$(PLATFORM).ok: $(LOADER_EXE) ctest.$(PLATFORM)$(EXE) ctest.$(PLATFORM).ref
	$(LOADER_EXE) --branch-trace ctest.$(PLATFORM).raw_trace ctest.$(PLATFORM)$(EXE)
	$(PYTHON) normalise_trace.py ctest.$(PLATFORM).raw_trace ctest.$(PLATFORM).trace
	diff -q ctest.$(PLATFORM).trace ctest.$(PLATFORM).ref

ctest.$(PLATFORM)$(EXE): main.c all_tests.$(PLATFORM).S
	$(CC) -o $@ $(CFLAGS) $(ASFLAGS) \
      all_tests.$(PLATFORM).S main.c $(LDFLAGS)

maintainer_rebuild:
	make ALLOW_REBUILD=1 PLATFORM=linux64 all_tests.linux64.S ctest.linux64.ref -B
	make ALLOW_REBUILD=1 PLATFORM=linux32 all_tests.linux32.S ctest.linux32.ref -B

ifeq ($(ALLOW_REBUILD),1)

# These rules are only used when the test is created.
# Do this on Linux. "make maintainer_rebuild". Recreates reference files.

ASM=t0.S t1.$(PLATFORM).S t2.S t3.S t4.S t5.S t6.$(PLATFORM).S

t1.$(PLATFORM).S: t1.c
	$(CC) -S -O2 -o $@ $<
	sed -i 's/[.]L/t1_L/g' $@

t6.$(PLATFORM).S: t6.c
	$(CC) -S -O2 -o $@ $<
	sed -i 's/[.]L/t6_L/g' $@

all_tests.$(PLATFORM).S: $(ASM)
	# All assembly files together
	cat $(ASM) > $@
	# Global symbols removed (treated differently on Linux/Windows)
	sed -i 's/^.*[.]glob.*//g' $@
	# Special section markers squashed to .text
	sed -i 's/[.]section.*note[.].*/.text/g' $@
	sed -i 's/[.]section.*text[.].*/.text/g' $@
	# Directives that are only supported for ELF are removed
	sed -i 's/^.*[.]\(size\|type\|local\).*//' $@
	sed -i 's/^\(.*[.]section.*\),".*".*/\1/' $@
	# .comm works for Win32 but there were issues for Win64; convert it to .bss
	sed -i 's/^.*[.]comm[\t ]*\(.*\),\([0-9]*\),\([0-9]*\)/.bss\n.align \3\n\1: .space \2/' $@
	# Only the entry point (all_tests) is global
	echo '.global all_tests' >> $@
	echo '.global _all_tests' >> $@


ifeq ($(PLATFORM),linux32)

# Here is a reference file made with the old version.
# Only possible to build this with 32-bit Linux.

ctest.$(PLATFORM).ref: main.c all_tests.$(PLATFORM).S
	cp /tmp/x86determiniser/libx86determiniser$(DLL) .
	$(CC) -o tmp$(EXE) $(CFLAGS) $(ASFLAGS) \
      all_tests.$(PLATFORM).S main.c \
      -L. -lx86determiniser -DUSE_OLD_VERSION_TO_MAKE_A_REFERENCE_FILE
	LD_LIBRARY_PATH=$(PWD) X86D_BRANCH_TRACE=tmp.txt ./tmp$(EXE)
	$(PYTHON) normalise_trace.py tmp.txt ctest.$(PLATFORM).ref

else
ctest.$(PLATFORM).ref: $(LOADER_EXE) ctest.$(PLATFORM)$(EXE)
	$(LOADER_EXE) --branch-trace tmp.txt ctest.$(PLATFORM)$(EXE)
	$(PYTHON) normalise_trace.py tmp.txt ctest.$(PLATFORM).ref

endif

else

all_tests.linux%.S:
	@echo all_tests.$(PLATFORM).S should already be present

all_tests.win%.S: all_tests.linux%.S
	cp $< $@

ctest.win%.ref: ctest.linux%.ref
	@echo $@ should be identical to $<
	cp $< $@

endif

