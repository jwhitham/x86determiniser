
PLATFORM=UNDEFINED
ROOT=../..
include ../../cfg/$(PLATFORM).cfg

all: tests.$(PLATFORM).ok

clean:
	-rm -rf args.$(PLATFORM)$(EXE) outs.$(PLATFORM)$(EXE) \
      ud.$(PLATFORM)$(EXE) pipetest.$(PLATFORM)$(EXE) \
      red_zone.$(PLATFORM)$(EXE) example.$(PLATFORM)$(EXE) \
      *.trace *.ok tmp.*

tests.$(PLATFORM).ok: args.$(PLATFORM)$(EXE) outs.$(PLATFORM)$(EXE) \
      ud.$(PLATFORM)$(EXE) pipetest.$(PLATFORM)$(EXE) \
      red_zone.$(PLATFORM)$(EXE) example.$(PLATFORM)$(EXE)
	python3 ./tests.py $(PLATFORM)

args.$(PLATFORM)$(EXE): args.c
	$(CC) -o $@ $(CFLAGS) $<

outs.$(PLATFORM)$(EXE): outs.S
	$(CC) -o $@ $(CFLAGS) $(ASFLAGS) $<

red_zone.$(PLATFORM)$(EXE): red_zone.S
	$(CC) -o $@ $(CFLAGS) $(ASFLAGS) $<

ud.$(PLATFORM)$(EXE): ud.c
	$(CC) -o $@ $(CFLAGS) $<

pipetest.$(PLATFORM)$(EXE): pipetest.c
	$(CC) -o $@ $(CFLAGS) $<

example.$(PLATFORM)$(EXE): ../example.c
	$(CC) -o $@ $(CFLAGS) $<

