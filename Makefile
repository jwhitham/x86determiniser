
all:
	@echo 'Use an appropriate make target for your platform:'
	@echo '   "make linux32" - Linux, 32-bit only'
	@echo '   "make linux64" - Linux, 32 and 64-bit, requires 64-bit host'
	@echo '   "make win32" - Windows, 32-bit only'
	@echo '   "make win64" - Windows, 32 and 64-bit, requires 64-bit host'

clean:
	-make -C src PLATFORM=win32 clean
	-make -C tests PLATFORM=win32 clean
	-rm -rf bin

deepclean:
	git clean -f -d -x

win32: 
	make -C src PLATFORM=win32 all
	make -C tests PLATFORM=win32 all

linux32:
	make -C src PLATFORM=linux32 all
	make -C tests PLATFORM=linux32 all

win64:
	make -C src PLATFORM=win64 all
	make -C tests PLATFORM=win64 all

linux64:
	make -C src PLATFORM=linux64 all
	make -C tests PLATFORM=linux64 all

