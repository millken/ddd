SYS := $(shell gcc -dumpmachine)
CC = gcc
OPTIMIZATION = -O3 

CFLAGS =  -lm  -lpthread $(HARDMODE)
ifeq (, $(findstring linux, $(SYS)))
CFLAGS = 
endif

DEBUG = -g 

ifneq ($(SMPDEBUG),)
DEBUG = -g -ggdb -D SMPDEBUG
endif

ifndef $(PREFIX)
PREFIX = /usr/local/dns
endif

INCLUDES=-I$(PWD)/include/

main.o:
	[ -d objs ] || mkdir objs;
	[ -d objs/3rd ] || mkdir objs/3rd;
	cd objs && $(CC) -fPIC -c ../*.c $(DEBUG) $(INCLUDES);

all: dns
	
dns: main.o
	$(CC) objs/3rd/*.o objs/*.o $(CFLAGS) $(DEBUG) -o $@

install:all
	[ -d $(PREFIX) ] || mkdir $(PREFIX);
	cp dns $(PREFIX)/
	
clean:
	rm -rf objs;
	rm -rf dns;
	rm -f core.*