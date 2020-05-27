# build
VERSION = 1
LIBVERSION = 1.0.0

CC = gcc -std=c99

INSTALL_DIR ?= /usr/local
HEADER_PATH ?= include/fraux
LIBRARY_PATH ?= lib

DEBUG = -g -DDEBUG

COMPILE_CMD ?= $(CC) -c $? 
BUILD_CMD ?= $(CC) $? -o $@	

all: fraux.o test

clean:
	rm -rf ./*.o ./*.gch
	rm -rf $(LIBNAME).*
	rm -rf ./test

fraux.o: fraux.h fraux.c
	$(COMPILE_CMD) -fPIC

test: test.c fraux.h fraux.c
	$(BUILD_CMD) $(DEBUG)

check: test
	./test

# install
NAME = fraux
LIBNAME = lib$(NAME)
LIB_HEADER = fraux.h
LIB_STATIC = $(LIBNAME).a
LIB_SHARED = $(LIBNAME).so
LIB_SHARED_V = $(LIBNAME).so.$(VERSION)
LIB_SHARED_VERSION = $(LIBNAME).so.$(LIBVERSION)

$(LIB_STATIC): fraux.o
	ar -crv $(LIB_STATIC) $?

$(LIB_SHARED_VERSION): fraux.o
	$(CC) $? --shared -o $@ -fPIC

$(LIB_SHARED_V): $(LIB_SHARED_VERSION)
	ln -s $(LIB_SHARED_VERSION) $(LIB_SHARED_V)

$(LIB_SHARED): $(LIB_SHARED_VERSION)
	ln -s $(LIB_SHARED_VERSION) $(LIB_SHARED)

PREFIX = /usr/local

INSTALL_HEADER_PATH = $(PREFIX)/include/$(NAME)
INSTALL_LIB_PATH = $(PREFIX)/lib

INSTALL = cp -a

install: $(LIB_STATIC) $(LIB_SHARED_VERSION) $(LIB_SHARED_V) $(LIB_SHARED)
	mkdir -p $(INSTALL_HEADER_PATH)
	cp -a $(LIB_HEADER) $(INSTALL_HEADER_PATH)
	cp -a $(LIB_STATIC) $(LIBNAME_SHARED_VERSION) $(LIBNAME_SHARED_V) $(LIB_SHARED) $(INSTALL_LIB_PATH)

uninstall:
	rm -rf $(INSTALL_HEADER_PATH)
	rm $(INSTALL_LIB_PATH)/$(LIBNAME).*