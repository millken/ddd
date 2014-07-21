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
	cd objs && $(CC) -fPIC -c ../*.c $(DEBUG) $(INCLUDES);

all: dns

zxc: 
	[ -d objs ] || mkdir objs;
	cd objs && $(CC) -fPIC -c ../*.c $(OPTIMIZATION) $(INCLUDES);
	$(CC) objs/*.o $(CFLAGS) $(OPTIMIZATION) -o dns
	
dns: main.o
	$(CC) objs/*.o $(CFLAGS) $(DEBUG) -o $@

install:all
	[ -d $(PREFIX) ] || mkdir $(PREFIX);
	cp dns $(PREFIX)/
	
clean:
	rm -rf objs;
	rm -rf dns;
	rm -f core.*
