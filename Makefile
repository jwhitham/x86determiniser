CC=gcc
LIBNAME=libx86determiniser
LDFLAGS=-L. -lx86determiniser
CFLAGS=-m32 -Wall -O0

all:
	echo 'Use "make linux32" or "make win32"'

linux32: $(LIBNAME).so example.c
	gcc -o example.elf example.c $(CFLAGS) $(LDFLAGS)
	LD_LIBRARY_PATH=$(PWD) ./example.elf 0
	LD_LIBRARY_PATH=$(PWD) ./example.elf 1

win32: $(LIBNAME).dll example.c
	gcc -o example.exe example.c $(CFLAGS) $(LDFLAGS)
	./example.exe 0
	./example.exe 1

$(LIBNAME).so: src/$(LIBNAME).so
	cp src/$(LIBNAME).so $(LIBNAME).so

$(LIBNAME).dll: src/$(LIBNAME).dll
	cp src/$(LIBNAME).dll $(LIBNAME).dll

src/$(LIBNAME).so:
	make -C src $(LIBNAME).so

src/$(LIBNAME).dll:
	make -C src $(LIBNAME).dll

clean:
	make -C src clean
	rm -f $(LIBNAME).so $(LIBNAME).dll

