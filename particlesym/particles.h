#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <time.h>
#include <limits.h>

#define true 1
#define false 0
#define SIZE 1

typedef char bool;
typedef struct particle{
	float mass;
	double acel[2];
	double speed[2];
	double pressure;
	bool exists;
	bool closed;
	unsigned long color;
}particle;

typedef struct board{
	particle **particles;
	double (**field)[2];
}board;

extern int ROW, COL;

board initBoard();
void spawnParticle(board *b, int row, int col, float mass, int color);
void particleStep(board *b);
void initOrders();
