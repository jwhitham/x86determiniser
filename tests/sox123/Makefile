
PLATFORM=UNDEFINED
ROOT=../..
include ../../cfg/$(PLATFORM).cfg

HERE=$(ROOT)/tests/sox123
INSTALL=$(HERE)/install.$(PLATFORM)
X86DRUN=$(LOADER_EXE) --out-trace x.txt --
LD=$(GCC_DIR)/ld

export CC
export LD
export RANLIB
export AR

all: tests.$(PLATFORM).ok

clean:
	-rm -rf sox-14.4.2
	-rm -rf mpg123-1.25.13
	-rm -rf $(INSTALL) tests.$(PLATFORM).ok

$(INSTALL)/bin/sox$(EXE):
	rm -rf sox-14.4.2
	tar xf sox-14.4.2.tar.bz2
	cd sox-14.4.2; $(WIN_BASH) ./configure \
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
	cd sox-14.4.2; make CC=$(CC) all
	mkdir -p $(INSTALL)/bin
	cp	sox-14.4.2/src/sox$(EXE) $(INSTALL)/bin

$(INSTALL)/bin/mpg123$(EXE):
	rm -rf mpg123-1.25.13
	tar xf mpg123-1.25.13.tar.bz2
	cd mpg123-1.25.13; $(WIN_BASH) ./configure \
		--with-audio=dummy \
		--with-cpu=generic --enable-static --disable-shared
	cd mpg123-1.25.13; make CC=$(CC) all
	mkdir -p $(INSTALL)/bin
	cp	mpg123-1.25.13/src/mpg123$(EXE) $(INSTALL)/bin

tests.$(PLATFORM).ok: $(INSTALL)/bin/mpg123$(EXE) $(INSTALL)/bin/sox$(EXE) 
	echo 'First run without determiniser'
	$(INSTALL)/bin/mpg123 -w ref1.wav input.mp3
	$(INSTALL)/bin/sox -R ref1.wav ref2.wav \
          phaser 0.89 0.85 1.0 0.24 2.0 -t
	echo 'Second run with determiniser - should produce same outputs'
	$(X86DRUN) $(INSTALL)/bin/mpg123 -w test1.wav input.mp3
	$(X86DRUN) $(INSTALL)/bin/sox -R test1.wav test2.wav \
          phaser 0.89 0.85 1.0 0.24 2.0 -t
	echo 'Compare outputs'
	cmp test1.wav ref1.wav
	cmp test2.wav ref2.wav
	echo 'Good'
	touch test.$(PLATFORM).ok

