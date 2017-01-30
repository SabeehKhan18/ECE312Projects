# Makefile for PacketEncapsulation project

CC = gcc
CFLAGS =

all: rhpClient
	
debug: CFLAGS += -g
debug: all

rhpClient: rhpClient.c
	$(CC) $(CFLAGS) -o $@ $@.c

clean:
	rm -f rhpClient
