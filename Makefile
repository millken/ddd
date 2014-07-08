SYS := $(shell gcc -dumpmachine)
CC = gcc
OPTIMIZATION = -O3

CFLAGS = -lm  -lpthread $(HARDMODE)
ifeq (, $(findstring linux, $(SYS)))
CFLAGS = 
endif

DEBUG = -g -ggdb

ifneq ($(SMPDEBUG),)
DEBUG = -g -ggdb -D SMPDEBUG
endif

ifndef $(PREFIX)
PREFIX = /usr/local/ddd
endif

INCLUDES=-I$(PWD)/include/

main.o:
	[ -d objs ] || mkdir objs;
	[ -d objs/3rd ] || mkdir objs/3rd;
	cd objs && $(CC) -fPIC -c ../*.c $(DEBUG) $(INCLUDES);
	cd objs/3rd && $(CC) -fPIC -c ../../3rd/inih/*.c $(DEBUG) $(INCLUDES);

all: ddd
	
ddd: main.o
	$(CC) objs/3rd/*.o objs/*.o $(CFLAGS) $(DEBUG) -o $@

install:all
	[ -d $(PREFIX) ] || mkdir $(PREFIX);
	cp ddd $(PREFIX)/
	[ -f $(PREFIX)/config.ini ] || cp config.ini $(PREFIX)/
	
clean:
	rm -rf objs;
	rm -rf ddd;
