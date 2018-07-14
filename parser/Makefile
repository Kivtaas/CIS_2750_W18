CC = gcc
CFLAGS = -Wall -std=c11 -g -Iinclude
LDFLAGS = -L.

all: library


parser.o: ./src/GEDCOMparser.c ./include/GEDCOMparser.h ./include/GEDCOMutilities.h
	$(CC) $(CFLAGS) -c -fpic ./src/GEDCOMparser.c -o ./bin/libparser.o

list.o: ./src/LinkedListAPI.c ./include/LinkedListAPI.h
	$(CC) $(CFLAGS) -c -fpic ./src/LinkedListAPI.c -o ./bin/liblist.o

library: parser.o list.o
	gcc -shared -o gedcomLib.so ./bin/liblist.o ./bin/libparser.o 
test: all
	gcc ./bin/libparser.o ./bin/liblist.o main.c -Iinclude

clean: 
	rm -f ./bin/*.o  ./bin/*.a 
