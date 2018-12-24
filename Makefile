
all:
	@echo 'Use "make linux32" or "make win32"'

clean:
	-make -C src PLATFORM=win32 clean
	-make -C tests PLATFORM=win32 clean
	-make -C src PLATFORM=linux32 clean
	-make -C tests PLATFORM=linux32 clean
	-rm -rf bin

deepclean:
	git clean -f -d -x

win32: 
	make -C src PLATFORM=win32 all
	make -C tests PLATFORM=win32 all

