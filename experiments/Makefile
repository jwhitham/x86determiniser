

CC=/c/TDM-GCC-64/bin/gcc
#CC=gcc
CFLAGS32=-g -Wall -m32 -DWIN32
CFLAGS64=-g -Wall -m64 -DWIN32
LOADER_LIBS=-lpsapi
BINUTILS=distest/binutils-2.29.1


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
GENERATED=determiniser/generated
T=tests
TB=$(T)/bin

LOADER_SRCS=\
   $(LOADER_COMMON)/main.c \
   $(LOADER_PLATFORM)/win32_loader.c \
   $(LOADER_PLATFORM)/win32_remote.c
DETERMINISER_SRCS=\
   $(DETERMINISER_COMMON)/x86_common_c.c \
   $(DETERMINISER_COMMON)/x86_common_asm.S \
   $(DETERMINISER_PLATFORM)/win32_asm.S \
   $(DETERMINISER_PLATFORM)/win32_c.c \
   $(BINUTILS)/opcodes/i386-dis.c
HEADERS=\
   $(LOADER_COMMON)/remote_loader.h
MAKE_H_SRCS=$(DETERMINISER_COMMON)/make_h.c
MAKE_H=$(GENERATED)/make_h$(EXE)
OFFSETS_H=$(GENERATED)/offsets.h
INCS=-I$(GENERATED) -I$(LOADER_COMMON) -I$(DETERMINISER_COMMON) \
   -I$(LOADER_PLATFORM) -I$(DETERMINISER_PLATFORM)



all: $(LOADER32_EXE) $(DETERMINISER32_DLL) \
      $(TB)/simple32$(EXE) $(TB)/simple64$(EXE)

clean:
	rm -f loader.exe target.exe remote.dll \
      win32_offsets.h win32_make_h.exe

$(LOADER32_EXE): $(LOADER_SRCS) $(HEADERS)
	$(CC) -o $@ $(LOADER_SRCS) $(CFLAGS32) $(LOADER_LIBS) $(INCS)

$(DETERMINISER32_DLL): $(DETERMINISER_SRCS) $(HEADERS) $(OFFSETS_H)
	$(CC) -o $@ -shared $(DETERMINISER_SRCS) $(CFLAGS32) \
      -I$(BINUTILS)/include -I$(BINUTILS)/binutils $(INCS)

$(OFFSETS_H): $(MAKE_H_SRCS)
	$(CC) -o $(MAKE_H) $(MAKE_H_SRCS) $(CFLAGS32)
	$(MAKE_H) > $(OFFSETS_H)


$(TB)/simple32$(EXE): $(T)/simple.c
	$(CC) -o $@ $(CFLAGS32) $<

$(TB)/simple64$(EXE): $(T)/simple.c
	$(CC) -o $@ $(CFLAGS64) $<

