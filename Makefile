# Makefile
CC = gcc
CFLAGS = -std=c11 -Wall -O2
LIBS = -lcurl

all: uniprofile

uniprofile: src/main_curl.c
	$(CC) $(CFLAGS) -o uniprofile src/main_curl.c $(LIBS)

clean:
	rm -f uniprofile
