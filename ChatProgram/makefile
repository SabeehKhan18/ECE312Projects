# Makefile for the ChatServer project

CC = gcc
CFLAGS = -Wall
LDFLAGS = -lpthread

all: server client

server:
	$(CC) $(CFLAGS) -o $@ $@.c $(LDFLAGS)
	
client:
	$(CC) $(CFLAGS) -o $@ $@.c $(LDFLAGS)
	
clean:
	rm -f server client
