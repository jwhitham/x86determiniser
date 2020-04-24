
PLATFORM=UNDEFINED
ROOT=../..
include ../../cfg/$(PLATFORM).cfg

all: ctest.$(PLATFORM).ok

clean:
	-rm -rf ctest.trace \
      *.trace *.ok


ctest.$(PLATFORM).ok: $(LOADER_EXE)
	$(LOADER_EXE) --branch-trace ctest.$(PLATFORM).trace ctest.$(PLATFORM)$(EXE)
	python tests.py $(PLATFORM)

ctest.$(PLATFORM)$(EXE): main.c all_tests.$(PLATFORM).S
	$(CC) -o $@ $(CFLAGS) all_tests.$(PLATFORM).S main.c -ldl


SRC32=t0.S t1.c t2.S t3.S t4.S t5.S t6.c
SRC64=t0.S t1.S t2_x64.S t3_x64.S t4.S t5_x64.S t6.S

# These rules are only used when the test is created.
t1.S: t1.c
	$(CC) -S -O2 -o $@ $<
	sed -i 's/[.]L/t1_L/g' $@

t6.S: t6.c
	$(CC) -S -O2 -o $@ $<
	sed -i 's/[.]L/t6_L/g' $@

all_tests.$(PLATFORM).S: $(SRC64)
	cat $(SRC64) > $@
	sed -i 's/^[.]glob.*//g' $@
	sed -i 's/[.]section.*note[.].*/.text/g' $@
	sed -i 's/[.]section.*text[.].*/.text/g' $@
	echo '.global all_tests' >> $@
	echo '.global _all_tests' >> $@

ctest.$(PLATFORM).ref: ctest.$(PLATFORM)$(EXE)
	cp /tmp/x86determiniser/libx86determiniser$(DLL) .
	X86D_BRANCH_TRACE=ctest.$(PLATFORM).ref \
      ./ctest.$(PLATFORM)$(EXE) ./libx86determiniser$(DLL)

