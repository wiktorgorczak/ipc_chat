CC=gcc

main:
	$(CC) inf141223_k.c inf141223_k.h -o client.o -lpthread -Wall
	$(CC) inf141223_s.c inf141223_s.h -o server.o -Wall

all: main

server:
	$(CC) inf141223_s.c inf141223_s.h -o server.o -Wall

client:
	$(CC) inf141223_k.c inf141223_k.h -o client.o -lpthread -Wall

clean:
	rm -rf client.o server.o

