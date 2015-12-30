CC=gcc
LIBNAME=libx86determiniser
COMMON=x86_common_c.c x86_common_asm.S
LINUX32=linux32_asm.S linux32_offsets.h linux32_c.c 
WIN32=win32_asm.S win32_offsets.h win32_c.c
CFLAGS=-g -O2 -Wall -m32

$(LIBNAME).so: $(COMMON) $(LINUX32)
	$(CC) -shared -o $(LIBNAME).so $(COMMON) $(LINUX32) -fPIC -DLINUX32 $(CFLAGS)

$(LIBNAME).dll: $(COMMON) $(WIN32)
	$(CC) -shared -o $(LIBNAME).dll $(COMMON) $(WIN32) -DWIN32 $(CFLAGS)

linux32_offsets.h: make_h.c
	$(CC) -o linux32_make_h make_h.c $(CFLAGS) -DLINUX32
	./linux32_make_h > linux32_offsets.h

win32_offsets.h: make_h.c
	$(CC) -o win32_make_h.exe make_h.c $(CFLAGS) -DWIN32
	./win32_make_h.exe > win32_offsets.h

clean:
	rm -f *.o $(LIBNAME).so $(LIBNAME).dll \
        linux32_make_h win32_make_h linux32_offsets.h win32_offsets.h

