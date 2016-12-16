# Makefile for the ChatServer project

CC = gcc
CFLAGS = -Wall
LDFLAGS = -lpthread

all: server client

debug: CFLAGS += -g
debug: all

server:
	$(CC) $(CFLAGS) -o $@ $@.c $(LDFLAGS)
	
client:
	$(CC) $(CFLAGS) -o $@ $@.c $(LDFLAGS)
	
clean:
	rm -f server client
