
ROOT=../..
PLATFORM=UNDEFINED

include ../../cfg/$(PLATFORM).cfg

LIBNAME=libx86determiniser$(DLL)
COPY_LIBNAME=$(ROOT)/bin/version1shim/libx86determiniser$(DLL)
LDFLAGS2=-L. -lx86determiniser
CFLAGS2=-m32 -Wall -O0

all: tests.$(PLATFORM).ok

tests.$(PLATFORM).ok: example.exe example2.exe
	python ./tests.py $(PLATFORM)

example.exe: example.c $(LIBNAME)
	$(CC) -o example.exe example.c $(CFLAGS2) $(LDFLAGS2)

example2.exe: example2.adb $(LIBNAME)
	$(GCC_DIR)/gnatmake -o example2.exe example2.adb -largs $(LDFLAGS2)

$(LIBNAME): $(COPY_LIBNAME)
	cp $(COPY_LIBNAME) .

$(COPY_LIBNAME):
	make -C $(ROOT)/src version1shim

	
clean:
	rm -f $(LIBNAME)
	rm -f example.exe example2.exe
	rm -f *.ali

