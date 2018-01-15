

CC=/c/TDM-GCC-64/bin/gcc
#CC=gcc
CFLAGS32=-g -Wall -m32 -DWIN32
CFLAGS64=-g -Wall -m64 -DWIN32
LOADER_LIBS=-lpsapi
BINUTILS=distest/binutils-2.29.1

VERSION=2.0

PLATFORM=win32
EXE=.exe
DLL=.dll

LOADER32_EXE=bin/x86determiniser$(EXE)
# LOADER64_EXE=bin/x64determiniser$(EXE)
DETERMINISER32_DLL=bin/x86determiniser$(DLL)
# DETERMINISER64_DLL=bin/x64determiniser$(DLL)

LOADER_COMMON=loader/common
LOADER_PLATFORM=loader/$(PLATFORM)
DETERMINISER_COMMON=determiniser/common
DETERMINISER_PLATFORM=determiniser/$(PLATFORM)
GENERATED=generated
T=tests
TB=$(T)/bin

LOADER_SRCS=\
   $(LOADER_COMMON)/main.c \
   $(LOADER_COMMON)/x86_error.c \
   $(LOADER_PLATFORM)/win32_loader.c \
   $(LOADER_PLATFORM)/win32_remote.c
DETERMINISER_SRCS=\
   $(DETERMINISER_COMMON)/x86_common_c.c \
   $(DETERMINISER_COMMON)/x86_common_asm.S \
   $(DETERMINISER_PLATFORM)/win32_asm.S \
   $(DETERMINISER_PLATFORM)/win32_c.c \
   $(BINUTILS)/opcodes/i386-dis.c
HEADERS=\
   $(LOADER_COMMON)/remote_loader.h \
   $(GENERATED)/x86d_version.h
MAKE_H_SRCS=$(DETERMINISER_COMMON)/make_h.c
MAKE_H=$(GENERATED)/make_h$(EXE)
OFFSETS_H=$(GENERATED)/offsets.h
INCS=-I$(BINUTILS)/bfd -I$(BINUTILS)/include  \
   -I$(GENERATED) -I$(LOADER_COMMON) -I$(DETERMINISER_COMMON) \
   -I$(LOADER_PLATFORM) -I$(DETERMINISER_PLATFORM)



all: $(LOADER32_EXE) $(DETERMINISER32_DLL) \
      $(TB)/simple32$(EXE) $(T)/outs.ok

clean:
	rm -f loader.exe target.exe remote.dll \
      win32_offsets.h win32_make_h.exe $(GENERATED)/x86d_version.h

$(GENERATED)/x86d_version.h:
	echo '#define X86D_VERSION "'$(VERSION)'"' > $(GENERATED)/x86d_version.h
	echo -n '#define INTERNAL_VERSION "X86D '$(VERSION)' ' >> $(GENERATED)/x86d_version.h
	git rev-parse HEAD | tr -d '\n' >> $(GENERATED)/x86d_version.h
	echo ' !"' >> $(GENERATED)/x86d_version.h

$(LOADER32_EXE): $(LOADER_SRCS) $(HEADERS) bin
	$(CC) -o $@ $(LOADER_SRCS) $(CFLAGS32) $(LOADER_LIBS) $(INCS)

$(DETERMINISER32_DLL): $(DETERMINISER_SRCS) $(HEADERS) $(OFFSETS_H) bin
	$(CC) -o $@ -shared $(DETERMINISER_SRCS) $(CFLAGS32) \
      $(INCS)

$(OFFSETS_H): $(MAKE_H_SRCS)
	$(CC) -o $(MAKE_H) $(MAKE_H_SRCS) $(CFLAGS32)
	$(MAKE_H) > $(OFFSETS_H)

bin:
	mkdir -p bin

$(TB):
	mkdir -p $(TB)

$(TB)/simple32$(EXE): $(T)/simple.c $(TB)
	$(CC) -o $@ $(CFLAGS32) $<

$(TB)/outs$(EXE): $(T)/outs.s $(TB)
	$(CC) -o $@ $(CFLAGS32) $<

$(T)/outs.txt: $(TB)/outs$(EXE)
	$(LOADER32_EXE) --out-trace $@ $<

$(T)/outs.ok: $(T)/outs.txt
	diff -w $(T)/outs.txt $(T)/outs.ref
	touch $@


