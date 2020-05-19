CC=gcc
CFLAGS=-Werror=vla -Wextra -Wall -Wshadow -Wswitch-default -std=c11 -pthread
CFLAG_SAN=$(CFLAGS) -fsanitize=address -g
DEPS=hashmap.h linkedlist.h dyn_array.h
OBJ=linkedlist.o dyn_array.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAG_SAN)

hashmap.o: hashmap.c $(OBJ)
	$(CC) -c -o $@ $< $(CFLAG_SAN)

test_fsan: test.c
	$(CC) -o test test.c linkedlist.c hashmap.c dyn_array.c -pthread $(CFLAG_SAN)
	./test test_safe_complex
test_val: test.c
	$(CC) -o test test.c linkedlist.c hashmap.c dyn_array.c -pthread $(CFLAGS) -g
	valgrind --tool=helgrind ./test test_safe_complex
clean:
	rm -f *.o
	rm -f test_fsan
	rm -f test_val
