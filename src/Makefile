
PLATFORM=win32
ROOT=..
include ../$(PLATFORM).cfg

ZYDIS=determiniser/zydis-2.0.2

VERSION=2.0

LOADER_COMMON=loader/common
DETERMINISER_COMMON=determiniser/common

LOADER_SRCS=\
   $(LOADER_COMMON)/main.c \
   $(LOADER_COMMON)/x86_error.c \
   $(LOADER_PLATFORM_SRCS)
DETERMINISER_SRCS=\
   $(DETERMINISER_COMMON)/x86_common_c.c \
   $(DETERMINISER_PLATFORM_SRCS) \
   $(DETERMINISER_PLATFORM_CPU_SRCS) \
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



all: $(LOADER_EXE)

clean:
	rm -f $(LOADER_EXE) $(DETERMINISER_DLL)  \
      $(OFFSETS_H) $(MAKE_H) \
      $(GENERATED)/x86d_version.h

$(GENERATED)/x86d_version.h:
	echo '#define X86D_VERSION "'$(VERSION)'"' > $(GENERATED)/x86d_version.h
	echo -n '#define INTERNAL_VERSION "X86D '$(VERSION)' ' >> $(GENERATED)/x86d_version.h
	git rev-parse HEAD | tr -d '\n' >> $(GENERATED)/x86d_version.h
	echo ' !"' >> $(GENERATED)/x86d_version.h

$(DETERMINISER_OBJECT): $(DETERMINISER_DLL)
	cd $(GENERATED); $(OBJCOPY) $(OBJCOPY_FLAGS) \
      $(SHORT_DETERMINISER_DLL) ../$(DETERMINISER_OBJECT)

$(LOADER_EXE): $(LOADER_SRCS) $(HEADERS) ../bin $(DETERMINISER_OBJECT)
	$(CC) -o $@ $(LOADER_SRCS) $(CFLAGS) $(LOADER_LIBS) $(INCS) $(LDFLAGS) \
      $(DETERMINISER_OBJECT)

$(DETERMINISER_DLL): $(DETERMINISER_SRCS) $(HEADERS) $(OFFSETS_H)
	$(CC) -o $@ -shared $(DETERMINISER_SRCS) $(CFLAGS) \
      $(INCS) $(LDFLAGS)

$(OFFSETS_H): $(MAKE_H_SRCS)
	$(CC) -o $(MAKE_H) $(MAKE_H_SRCS) $(INCS) $(CFLAGS)
	$(MAKE_H) > $(OFFSETS_H)

../bin:
	mkdir -p ../bin


