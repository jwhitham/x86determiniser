
PLATFORM=UNDEFINED
ROOT=../..
include ../../cfg/$(PLATFORM).cfg

HERE=$(ROOT)/tests/sox123
INSTALL=$(PWD)/install

export CC

all: test.$(PLATFORM).ok

clean:
	-rm -rf $(CLEANUP)

$(INSTALL)/bin/sox$(EXE):
	rm -rf sox-14.4.2
	tar xf sox-14.4.2.tar.bz2
	cd sox-14.4.2; ./configure --prefix=$(INSTALL) \
      --enable-static --disable-shared \
      --disable-stack-protector --disable-openmp \
      --without-magic \
      --without-png \
      --without-ladspa \
      --without-mad \
      --without-id3tag \
      --without-lame \
      --without-twolame \
      --without-oggvorbis \
      --without-opus \
      --without-flac \
      --without-amrwb \
      --without-amrnb \
      --without-wavpack \
      --without-sndio \
      --without-coreaudio \
      --without-alsa \
      --without-ao \
      --without-pulseaudio \
      --without-waveaudio \
      --without-sndfile \
      --without-oss \
      --without-sunaudio \
      --without-mp3 \
      --without-gsm \
      --without-lpc10
	cd sox-14.4.2; make CC=$(CC) all install

$(INSTALL)/bin/mpg123$(EXE):
	rm -rf mpg123-1.25.13
	tar xf mpg123-1.25.13.tar.bz2
	cd mpg123-1.25.13; ./configure \
		--with-audio=dummy --prefix=$(INSTALL) \
		--with-cpu=generic --enable-static --disable-shared
	cd mpg123-1.25.13; make CC=$(CC) all install

test.$(PLATFORM).ok: $(INSTALL)/bin/sox$(EXE) $(INSTALL)/bin/mpg123$(EXE)
	$(LOADER_EXE) --out-trace x1.txt -- \
      $(INSTALL)/bin/mpg123 -w test.wav input.mp3
	$(LOADER_EXE) --out-trace x3.txt -- \
      $(INSTALL)/bin/sox -R test.wav test2.wav \
          phaser 0.89 0.85 1.0 0.24 2.0 -t
	md5sum -c expect.txt
	touch test.$(PLATFORM).ok

