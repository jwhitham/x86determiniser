CC=gcc

linux32: libicounter.so

win32: libicounter.dll

libicounter.so: linux32_c.c x86_common_c.c x86_common_asm.S linux32_asm.S linux32_offsets.h
	$(CC) -shared -o libicounter.so x86_common_c.c x86_common_asm.S linux32_c.c linux32_asm.S -g -O2 -fPIC -Wall -DLINUX32

libicounter.dll: win32_c.c x86_common_c.c x86_common_asm.S win32_asm.s win32_offsets.h
	$(CC) -shared -o libicounter.dll win32_asm.S x86_common_c.c x86_common_asm.S win32_c.c -g -O2 -Wall -DWIN32

linux32_offsets.h: make_h.c
	$(CC) -o linux32_make_h make_h.c -O0 -g -DLINUX32
	./linux32_make_h > linux32_offsets.h

win32_offsets.h: make_h.c
	$(CC) -o win32_make_h make_h.c -O0 -g -DWIN32
	./win32_make_h > win32_offsets.h

clean:
	rm -f *.o libicounter.so libicounter.dll linux32_make_h win32_make_h linux32_offsets.h win32_offsets.h

