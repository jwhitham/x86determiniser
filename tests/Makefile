
PLATFORM=win32
include ../$(PLATFORM).cfg

T=../tests
TB=$(T)/bin

all: $(T)/simple32.ok $(T)/outs.ok

clean:
	-rm -rf $(TB)/simple32$(EXE) $(TB)/outs$(EXE) \
      $(T)/outs.txt $(T)/outs.ok $(T)/simple32.ok

$(TB):
	mkdir -p $(TB)

$(TB)/simple32$(EXE): $(T)/simple.c $(TB)
	$(CC) -o $@ $(CFLAGS32) $<

$(TB)/outs$(EXE): $(T)/outs.s $(TB)
	$(CC) -o $@ $(CFLAGS32) $<

$(T)/outs.txt: $(TB)/outs$(EXE) $(LOADER32_EXE)
	$(LOADER32_EXE) --out-trace $@ $<

$(T)/outs.ok: $(T)/outs.txt
	diff -w $(T)/outs.txt $(T)/outs.ref
	touch $@

$(T)/simple32.ok: $(TB)/simple32$(EXE) $(LOADER32_EXE)
	$(LOADER32_EXE) --branch-trace $(T)/simple32.ok $(TB)/simple32$(EXE)

