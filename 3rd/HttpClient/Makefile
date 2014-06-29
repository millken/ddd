CC = gcc
CFLAGS = -ansi -pedantic -Wall -O2 -lm
DEBUG = -ansi -pedantic -Wall -g -lm

test: 
	gcc test.c HttpClient.c -I. -g -o a.out

.PHONY = clean
clean:
	rm -f a.out
	rm -f *.o
