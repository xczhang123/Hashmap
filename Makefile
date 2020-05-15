CC=clang
CFLAGS=-Werror=vla -Wextra -Wall -Wshadow -Wswitch-default -std=c11
CFLAG_SAN=$(CFLAGS) -fsanitize=address -g
DEPS=hashmap.h linkedlist.h dyn_array.h
OBJ=linkedlist.o dyn_array.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAG_SAN)

hashmap.o: hashmap.c $(OBJ)
	$(CC) -c -o $@ $< $(CFLAG_SAN)
	
clean:
	rm *.o
