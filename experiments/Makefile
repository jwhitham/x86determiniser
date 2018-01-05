

CC=/c/TDM-GCC-64/bin/gcc
#CC=gcc
CFLAGS=-g -Wall -m32 -DWIN32
BINUTILS=distest/binutils-2.29.1

LOADER_SRCS=main.c loader.c remote_loader.c
TARGET_SRCS=target.c
REMOTE_SRCS=x86_common_c.c x86_common_asm.S \
   win32_asm.S win32_c.c $(BINUTILS)/opcodes/i386-dis.c
HEADERS=remote_loader.h



all: loader.exe target.exe remote.dll

clean:
	rm -f loader.exe target.exe remote.dll \
      win32_offsets.h win32_make_h.exe

loader.exe: $(LOADER_SRCS) $(HEADERS)
	$(CC) -o $@ $(LOADER_SRCS) $(CFLAGS) -lpsapi

target.exe: $(TARGET_SRCS) $(HEADERS)
	$(CC) -o $@ $(TARGET_SRCS) $(CFLAGS)

remote.dll: $(REMOTE_SRCS) $(HEADERS) win32_offsets.h 
	$(CC) -o $@ -shared $(REMOTE_SRCS) $(CFLAGS) \
      -I$(BINUTILS)/include -Idistest 

win32_offsets.h: make_h.c
	$(CC) -o win32_make_h.exe make_h.c $(CFLAGS) -DWIN32
	./win32_make_h.exe > win32_offsets.h

