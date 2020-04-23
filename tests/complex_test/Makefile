
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

ctest.$(PLATFORM)$(EXE): main.c all_tests.S ctest.$(PLATFORM).bin
	cp ctest.$(PLATFORM).bin tmp.bin
	$(CC) -o $@ $(CFLAGS) all_tests.S main.c -ldl -Ttest.ld


# These rules are only used when the test is created.
SRC32=t0.S t1.c t2.S t3.S t4.S t5.S t6.c
SRC64=t0.S t1.c t2_x64.S t3_x64.S t4.S t5_x64.S t6.c

# First create an ELF containing all of the code
ctest64.elf: $(SRC64)
	$(CC) -o $@ $(SRC64) -O2 -nostdlib -Tblob.ld
ctest32.elf: $(SRC32)
	$(CC) -o $@ $(SRC32) -O2 -nostdlib -Tblob.ld

# Then create a binary file, again with all of the code
ctest.linux64.bin: ctest64.elf
	$(OBJCOPY) -O binary $< $@
ctest.linux32.bin: ctest32.elf
	$(OBJCOPY) -O binary $< $@
ctest.win64.bin: ctest.linux64.bin
	cp $< $@
ctest.win32.bin: ctest.linux32.bin
	cp $< $@

ctest.$(PLATFORM).ref: ctest.$(PLATFORM)$(EXE)
	cp /tmp/x86determiniser/libx86determiniser$(DLL) .
	X86D_BRANCH_TRACE=ctest.$(PLATFORM).ref \
      ./ctest.$(PLATFORM)$(EXE) ./libx86determiniser$(DLL)

