
PLATFORM=win32
ROOT=..
include ../$(PLATFORM).cfg

all: simple32.ok outs.ok ud.ok ctest.ok

clean:
	-rm -rf simple32$(EXE) outs$(EXE) ud$(EXE) ctest.trace \
      *.trace *.ok

# simple32: "Hello World"
simple32$(EXE): simple.c
	$(CC) -o $@ $(CFLAGS32) $<

simple32.ok: simple32$(EXE) $(LOADER32_EXE)
	$(LOADER32_EXE) --branch-trace simple32.ok simple32$(EXE)

# outs: OUT test, make a trace of OUT instructions, compare to reference
outs$(EXE): outs.s
	$(CC) -o $@ $(CFLAGS32) $<

outs.trace: outs$(EXE) $(LOADER32_EXE)
	$(LOADER32_EXE) --out-trace $@ $<

outs.ok: outs.trace
	diff -w outs.trace outs.ref
	touch $@

# ctest: branch trace of C program
ctest$(EXE): ctest.c
	$(CC) -o $@ $(CFLAGS32) $< -O2

ctest.trace: ctest$(EXE) $(LOADER32_EXE)
	$(LOADER32_EXE) --branch-trace $@ $<

ctest.ok: ctest.trace
	diff -w ctest.trace ctest.ref
	touch $@

# ud: undefined instruction handling
ud$(EXE): ud.s
	$(CC) -o $@ $(CFLAGS32) $<

ud.trace: ud$(EXE) $(LOADER32_EXE)
	$(LOADER32_EXE) --branch-trace $@ $<

ud.ok: ud.trace
	diff -w ud.trace ud.ref
	touch $@

