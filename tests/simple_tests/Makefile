
PLATFORM=win32
ROOT=../..
include ../../$(PLATFORM).cfg

all: tests.$(PLATFORM).ok

clean:
	-rm -rf args.$(PLATFORM)$(EXE) outs.$(PLATFORM)$(EXE) \
      ud.$(PLATFORM)$(EXE) pipetest.$(PLATFORM)$(EXE) \
      example.$(PLATFORM)$(EXE) \
      *.trace *.ok tmp.*

tests.$(PLATFORM).ok: args.$(PLATFORM)$(EXE) outs.$(PLATFORM)$(EXE) \
      ud.$(PLATFORM)$(EXE) pipetest.$(PLATFORM)$(EXE) \
      example.$(PLATFORM)$(EXE)
	python ./tests.py $(PLATFORM)

args.$(PLATFORM)$(EXE): args.c
	$(CC) -o $@ $(CFLAGS) $<

outs.$(PLATFORM)$(EXE): outs.S
	$(CC) -o $@ $(CFLAGS) $<

ud.$(PLATFORM)$(EXE): ud.c
	$(CC) -o $@ $(CFLAGS) $<

pipetest.$(PLATFORM)$(EXE): pipetest.c
	$(CC) -o $@ $(CFLAGS) $<

example.$(PLATFORM)$(EXE): ../example.c
	$(CC) -o $@ $(CFLAGS) $<

