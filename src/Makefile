
PLATFORM=win32
include ../$(PLATFORM).cfg

LOADER_LIBS=-lpsapi
ZYDIS=determiniser/zydis-2.0.2

VERSION=2.0

LOADER_COMMON=loader/common
LOADER_PLATFORM=loader/$(PLATFORM)
DETERMINISER_COMMON=determiniser/common
DETERMINISER_PLATFORM=determiniser/$(PLATFORM)
GENERATED=generated

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
   $(ZYDIS)/src/Decoder.c \
   $(ZYDIS)/src/DecoderData.c \
   $(ZYDIS)/src/Formatter.c \
   $(ZYDIS)/src/MetaInfo.c \
   $(ZYDIS)/src/Mnemonic.c \
   $(ZYDIS)/src/Register.c \
   $(ZYDIS)/src/SharedData.c \
   $(ZYDIS)/src/String.c \
   $(ZYDIS)/src/Utils.c \
   $(ZYDIS)/src/Zydis.c
HEADERS=\
   $(LOADER_COMMON)/remote_loader.h \
   $(GENERATED)/x86d_version.h
MAKE_H_SRCS=$(DETERMINISER_COMMON)/make_h.c
MAKE_H=$(GENERATED)/make_h$(EXE)
OFFSETS_H=$(GENERATED)/offsets.h
INCS=-I$(ZYDIS)/include \
   -I$(ZYDIS)/msvc \
   -I$(ZYDIS)/src \
   -I$(GENERATED) -I$(LOADER_COMMON) -I$(DETERMINISER_COMMON) \
   -I$(LOADER_PLATFORM) -I$(DETERMINISER_PLATFORM)



all: $(LOADER32_EXE) $(DETERMINISER32_DLL)

clean:
	rm -f $(LOADER32_EXE) $(DETERMINISER32_DLL)  \
      $(OFFSETS_H) $(MAKE_H) \
      $(GENERATED)/x86d_version.h

$(GENERATED)/x86d_version.h:
	echo '#define X86D_VERSION "'$(VERSION)'"' > $(GENERATED)/x86d_version.h
	echo -n '#define INTERNAL_VERSION "X86D '$(VERSION)' ' >> $(GENERATED)/x86d_version.h
	git rev-parse HEAD | tr -d '\n' >> $(GENERATED)/x86d_version.h
	echo ' !"' >> $(GENERATED)/x86d_version.h

$(LOADER32_EXE): $(LOADER_SRCS) $(HEADERS) ../bin
	$(CC) -o $@ $(LOADER_SRCS) $(CFLAGS32) $(LOADER_LIBS) $(INCS)

$(DETERMINISER32_DLL): $(DETERMINISER_SRCS) $(HEADERS) $(OFFSETS_H)
	$(CC) -o $@ -shared $(DETERMINISER_SRCS) $(CFLAGS32) \
      $(INCS)

$(OFFSETS_H): $(MAKE_H_SRCS)
	$(CC) -o $(MAKE_H) $(MAKE_H_SRCS) $(CFLAGS32)
	$(MAKE_H) > $(OFFSETS_H)

../bin:
	mkdir -p ../bin


