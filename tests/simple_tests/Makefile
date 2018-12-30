
PLATFORM=win32
ROOT=../..
include ../../$(PLATFORM).cfg

all: args$(EXE) outs$(EXE) ud$(EXE) tests.ok

clean:
	-rm -rf args$(EXE) outs$(EXE) ud$(EXE) \
      *.trace *.ok
	make -C complex_test clean

tests.ok:
	python ./tests.py

args$(EXE): args.c
	$(CC) -o $@ $(CFLAGS32) $<

outs$(EXE): outs.s
	$(CC) -o $@ $(CFLAGS32) $<

ud$(EXE): ud.c
	$(CC) -o $@ $(CFLAGS32) $<

