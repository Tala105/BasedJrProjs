#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <time.h>

#define true 1
#define false 0

typedef char bool;
typedef struct particle{
	float mass;
	float acelx;
	float acely;
	float speedx;
	float speedy;
	bool exists;
	bool closed;
	unsigned long color;
}particle;

typedef struct board{
	particle **particles;
	float **fieldx;
	float **fieldy;
}board;

extern int ROW, COL;

board initBoard();
void spawnParticle(board *b, int row, int col, float mass, int color);
void particleStep(board *b);
void initOrders();
