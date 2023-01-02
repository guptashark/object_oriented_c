
INCLUDE = include

CC = gcc
CFLAGS = -Wall -Werror -Wextra -std=c99 -I $(INCLUDE)

all:
	$(CC) $(CFLAGS) src/main.c src/memory.c src/obj.c
	$(CC) $(CFLAGS) -o ch_04_main.out src/ch_04/main.c
	$(CC) $(CFLAGS) -o ch_06_main.out src/ch_06/main.c
	./ch_06_main.out
