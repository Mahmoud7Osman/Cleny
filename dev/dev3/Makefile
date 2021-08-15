exe = ts3d
exe-dir = $(HOME)/bin
version-file = version
version = `cat $(version-file)`
tests = tests
windows-zip = ts3d.zip
data-dir = data
man-page = ts3d.6.gz
man-dir = /usr/share/man/man6
TS3D_ROOT ?= $(HOME)/.ts3d
TS3D_DATA ?= $(TS3D_ROOT)/data
exe-install = $(exe-dir)/$(exe)
man-install = $(man-dir)/$(man-page)
data-install = $(TS3D_DATA)
sources = src/*.cpp
headers = src/*.h
man-page-input = ts3d.6.in

cflags = -std=c99 -Wall -Wextra -Wpedantic -D_POSIX_C_SOURCE=200112L\
 -DJSON_WITH_STDIO -DTS3D_VERSION="\"$(version)\"" ${CFLAGS}
linkage = -lm -lcurses
test-flags = -shared -fPIC -O0 -g3 -DCTF_TESTS_ENABLED

CC ?= g++
RM ?= rm -f

$(exe): $(sources) $(headers) $(version-file)
	$(CC) $(cflags) -o $@ $(sources) $(linkage)

$(tests): $(sources) $(headers)
	$(CC) $(cflags) $(test-flags) -o $@ $(sources) $(linkage)

$(windows-zip): $(exe)
	./zip-windows

$(man-page): $(man-page-input) $(version-file)
	sed "s/@@VERSION@@/$(version)/g" $< | gzip > $@

.PHONY: install
install: $(exe) $(man-page)
	MAKEFILE=yes EXE="$(exe)" EXE_INSTALL="$(exe-install)" \
	DATA_DIR="$(data-dir)" DATA_INSTALL="$(data-install)" \
	MAN_PAGE="$(man-page)" MAN_INSTALL="$(man-install)" \
	./installer install

.PHONY: uninstall
uninstall:
	MAKEFILE=yes EXE_INSTALL="$(exe-install)" \
	DATA_INSTALL="$(data-install)" MAN_INSTALL="$(man-install)" \
	./installer uninstall

.PHONY: run-tests
run-tests: $(tests)
	ceeteef -t8 $(tests)

.PHONY: clean
clean:
	$(RM) $(exe) $(tests) $(windows-zip) $(man-page)
