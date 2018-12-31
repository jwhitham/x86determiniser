
PLATFORM=win32
ROOT=../..
include ../../$(PLATFORM).cfg

all: ctest.$(PLATFORM).ok

clean:
	-rm -rf ctest.trace \
      *.trace *.ok

ctest.win32.ok: $(LOADER_EXE)
	$(LOADER_EXE) --branch-trace ctest.trace ctest$(EXE)
	python tests.py

ctest.win64.ok: $(LOADER_EXE)
	touch ctest.win64.ok

# These rules are only used when the test is created.
SRC=main.c t1.c t2.S t3.S t4.S t5.S t6.c t7.c

ctest$(EXE): $(SRC)
	$(CC) -o $@ $(CFLAGS) $(SRC) -O2

ctest.ref: ctest$(EXE)
	cp ~/x86dx/libx86determiniser$(DLL) .
	X86D_BRANCH_TRACE=ctest.ref ./ctest$(EXE) ./libx86determiniser$(DLL)
	./chop_at_markers.sh ctest.ref

