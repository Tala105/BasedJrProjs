#include <stdlib.h>
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
	char color;
}particle;

typedef struct board{
	particle **particles;
	float **fieldx;
	float **fieldy;
}board;

extern int ROW, COL;

void spawnParticle(particle **particles, int row, int col, unsigned int mass, int color);
void particleStep(board *b);
