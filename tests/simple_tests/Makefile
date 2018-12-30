
PLATFORM=win32
ROOT=../..
include ../../$(PLATFORM).cfg

all: tests.ok

clean:
	-rm -rf args$(EXE) outs$(EXE) ud$(EXE) pipetest$(EXE) example$(EXE) \
	   args64$(EXE) outs64$(EXE) ud64$(EXE) pipetest64$(EXE) example64$(EXE) \
      *.trace *.ok tmp.*

tests.ok: args$(EXE) outs$(EXE) ud$(EXE) pipetest$(EXE) example$(EXE) \
      args64$(EXE) outs64$(EXE) ud64$(EXE) pipetest64$(EXE) example64$(EXE)
	python ./tests.py

args$(EXE): args.c
	$(CC) -o $@ $(CFLAGS32) $<

outs$(EXE): outs.S
	$(CC) -o $@ $(CFLAGS32) $<

ud$(EXE): ud.c
	$(CC) -o $@ $(CFLAGS32) $<

pipetest$(EXE): pipetest.c
	$(CC) -o $@ $(CFLAGS32) $<

example$(EXE): ../example.c
	$(CC) -o $@ $(CFLAGS32) $<

args64$(EXE): args.c
	$(CC) -o $@ $(CFLAGS64) $<

outs64$(EXE): outs.S
	$(CC) -o $@ $(CFLAGS64) $<

ud64$(EXE): ud.c
	$(CC) -o $@ $(CFLAGS64) $<

pipetest64$(EXE): pipetest.c
	$(CC) -o $@ $(CFLAGS64) $<

example64$(EXE): ../example.c
	$(CC) -o $@ $(CFLAGS64) $<

