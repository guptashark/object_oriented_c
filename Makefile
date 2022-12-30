
INCLUDE = include

CC = gcc
CFLAGS = -Wall -Werror -Wextra -std=c99 -I $(INCLUDE)

all:
	$(CC) $(CFLAGS) src/main.c src/memory.c src/obj.c
	./a.out

