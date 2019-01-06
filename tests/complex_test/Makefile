
PLATFORM=win32
ROOT=../..
include ../../$(PLATFORM).cfg

all: ctest.$(PLATFORM).ok

clean:
	-rm -rf ctest.trace \
      *.trace *.ok

ctest.win32.ok: $(LOADER_EXE)
	$(LOADER_EXE) --branch-trace ctest.$(PLATFORM).trace ctest.$(PLATFORM)$(EXE)
	python tests.py $(PLATFORM)

ctest.win64.ok: $(LOADER_EXE)
	$(LOADER_EXE) --branch-trace ctest.$(PLATFORM).trace ctest.$(PLATFORM)$(EXE)
	python tests.py $(PLATFORM)

# These rules are only used when the test is created.
SRC32=main.c t1.c t2.S t3.S t4.S t5.S t6.c t7.c
SRC64=main.c t1.c t2_x64.S t3_x64.S t4.S t5_x64.S t6.c t7.c

ctest.win32$(EXE): $(SRC32)
	$(CC) -o $@ $(CFLAGS) $(SRC32) -O2

ctest.win64$(EXE): $(SRC64)
	$(CC) -o $@ $(CFLAGS) $(SRC64) -O2

ctest.$(PLATFORM).ref: ctest.$(PLATFORM)$(EXE)
	cp ~/x86dx/libx86determiniser$(DLL) .
	X86D_BRANCH_TRACE=ctest.$(PLATFORM).ref \
      ./ctest.$(PLATFORM)$(EXE) ./libx86determiniser$(DLL)

