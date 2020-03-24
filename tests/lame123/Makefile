
PLATFORM=UNDEFINED
ROOT=../..
include ../../cfg/$(PLATFORM).cfg

HERE=$(ROOT)/tests/lame123
INSTALL=$(PWD)/install

all: test.$(PLATFORM).ok

clean:
	-rm -rf $(CLEANUP)

$(INSTALL)/bin/lame$(EXE):
	rm -rf lame-3.100
	tar xf lame-3.100.tar.bz2
	cd lame-3.100; ./configure --prefix=$(INSTALL)
	cd lame-3.100; make all install

$(INSTALL)/bin/mpg123$(EXE):
	rm -rf mpg123-1.25.13
	tar xf mpg123-1.25.13.tar.bz2
	cd mpg123-1.25.13; ./configure \
		--with-audio=dummy --prefix=$(INSTALL) \
		--with-cpu=generic --enable-static --disable-shared
	cd mpg123-1.25.13; make all install

test.$(PLATFORM).ok: $(INSTALL)/bin/lame$(EXE) $(INSTALL)/bin/mpg123$(EXE)
	$(LOADER_EXE) --out-trace x1.txt -- \
		$(INSTALL)/bin/mpg123 -w test.wav input.mp3
	$(LOADER_EXE) --out-trace x2.txt -- \
		$(INSTALL)/bin/lame test.wav output.mp3
	md5sum -c expect.txt
	touch test.$(PLATFORM).ok

