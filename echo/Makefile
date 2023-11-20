# Makefile for Proxy Lab 
#
# You may modify this file any way you like (except for the handin
# rule). You instructor will type "make" on your specific Makefile to
# build your proxy from sources.

CC = gcc
CFLAGS = -g -Wall
LDFLAGS = -lpthread

all: echoclient echoserveri

echoclient: echoclient.c csapp.o
	$(CC) $(CFLAGS) -o echoclient echoclient.c csapp.o $(LDFLAGS)

echoserveri: echoserveri.c echo.c csapp.o $(LDFLAGS)
	$(CC) $(CFLAGS) -o echoserveri echoserveri.c echo.c csapp.o $(LDFLAGS)

csapp.o: csapp.c csapp.h
	$(CC) $(CFLAGS) -c csapp.c

clean:
	rm -f echoclient echoserveri echoclient.o echoserveri.o csapp.o echo.o
