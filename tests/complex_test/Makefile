
PLATFORM=win32
ROOT=../..
include ../../$(PLATFORM).cfg

all: ctest.ok

clean:
	-rm -rf ctest.trace \
      *.trace *.ok

SRC=main.c t1.c t2.S t3.S t4.S t5.S t6.c t7.c
ctest$(EXE): $(SRC)
	$(CC) -o $@ $(CFLAGS32) $(SRC) -O2

ctest.trace: ctest$(EXE) $(LOADER32_EXE)
	$(LOADER32_EXE) --branch-trace $@ $<
	./chop_at_markers.sh ctest.trace

ctest.ok: ctest.trace ctest.ref
	diff -w ctest.trace ctest.ref
	touch $@

ctest.ref: ctest$(EXE) libx86determiniser$(DLL)
	X86D_BRANCH_TRACE=ctest.ref ./ctest$(EXE) ./libx86determiniser$(DLL)
	./chop_at_markers.sh ctest.ref

