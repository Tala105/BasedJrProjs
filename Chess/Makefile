CC=gcc

all: board.so board

board.so: board.c
	$(CC) -shared -fPIC -o board.so board.c

board: board.c
	$(CC) -o board board.c

clear:
	rm board board.so
