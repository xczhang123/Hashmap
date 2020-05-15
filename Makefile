CC=gcc
CFLAGS=-Werror=vla -Wextra -Wall -Wshadow -Wswitch-default -std=c11
CFLAG_SAN=$(CFLAGS) -fsanitize=address -g
DEPS=linkedlist.c linkedlist.h dyn_array.c dyn_array.h
OBJ=hashmap.o

hashmap.o: hashmap.c hashmap.h $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)
	
clean:
	rm *.o
