
all:
	@echo 'Use an appropriate make target for your platform:'
	@echo '   "make linux" - Linux, 32-bit and 64-bit'
	@echo '   "make linux32" - Linux, 32-bit only'
	@echo '   "make linux64" - Linux, 64-bit only'
	@echo '   "make win" - Windows, 32-bit and 64-bit'
	@echo '   "make win32" - Windows, 32-bit only'
	@echo '   "make win64" - Windows, 64-bit only'

clean:
	-make -C src PLATFORM=win32 clean
	-make -C tests PLATFORM=win32 clean
	-rm -rf bin

deepclean:
	git clean -f -d -x

win:
	make win32 win64

linux:
	make linux32 linux64

win32:
	make -C src PLATFORM=win32 all
	make -C tests PLATFORM=win32 most

linux32:
	make -C src PLATFORM=linux32 all
	make -C tests PLATFORM=linux32 most

win64:
	make -C src PLATFORM=win64 all
	make -C tests PLATFORM=win64 most

linux64:
	make -C src PLATFORM=linux64 all
	make -C tests PLATFORM=linux64 most

win.test:
	make deepclean
	make win32 win64
	make -C tests PLATFORM=win32 all
	make -C tests PLATFORM=win64 all

linux.test:
	make deepclean
	make linux32 linux64
	make -C tests PLATFORM=linux32 all
	make -C tests PLATFORM=linux64 all

