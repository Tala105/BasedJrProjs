CC = gcc
CFLAGS = -Wall -O2
LIBS = -lX11

all: render

render: render.o particles.o
	$(CC) $(CFLAGS) -o render render.o particles.o $(LIBS)

render.o: render.c particles.h
	$(CC) $(CFLAGS) -c render.c -o render.o

particles.o: particles.c particles.h
	$(CC) $(CFLAGS) -c particles.c -o particles.o

clean:
	rm -f render.o particles.o render

