# Makefile for PacketEncapsulation project

CC = gcc
CFLAGS =

all: rhpClient
	
debug: CFLAGS += -g
debug: all

rhpClient:
	$(CC) $(CFLAGS) -o $@ $@.c

clean:
	rm -f rhpClient
