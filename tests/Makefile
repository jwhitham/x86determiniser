
PLATFORM=win32
include ../$(PLATFORM).cfg

all: simple32.ok outs.ok ud.ok

clean:
	-rm -rf simple32$(EXE) outs$(EXE) ud$(EXE) \
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

# ud: undefined instruction handling
ud$(EXE): ud.s
	$(CC) -o $@ $(CFLAGS32) $<

ud.trace: ud$(EXE) $(LOADER32_EXE)
	$(LOADER32_EXE) --branch-trace $@ $<

ud.ok: ud.trace
	diff -w ud.trace ud.ref
	touch $@
