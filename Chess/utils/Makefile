CC=gcc

all: treelogic.so treelogic

treelogic.so: treelogic.c
	$(CC) -shared -fPIC -o treelogic.so treelogic.c

treelogic: treelogic.c
	$(CC) -o treelogic treelogic.c

clear:
	rm treelogic treelogic.so
