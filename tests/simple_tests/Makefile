
PLATFORM=win32
ROOT=../..
include ../../$(PLATFORM).cfg

all: tests.ok

clean:
	-rm -rf args$(EXE) outs$(EXE) ud$(EXE) pipetest$(EXE) \
      example$(EXE) *.trace *.ok

tests.ok: args$(EXE) outs$(EXE) ud$(EXE) pipetest$(EXE) example$(EXE)
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

