# Makefile for PacketEncapsulation project

CC = gcc
CFLAGS = -Wall

all: rhpClient
	
debug: CFLAGS += -g
debug: all

rhpClient: rhpClient.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f rhpClient
