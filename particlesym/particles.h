#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <time.h>
#include <limits.h>
#include <stdbool.h>
#include <cuda_runtime.h>
#include <device_launch_parameters.h>

#define SIZE 2
#define GRAVITY 0.5f
#define SOFT_CAP 2
#define HARD_CAP 4
#define FIELD_RADIUS 16
#define FIELD_CONSTANT 0.01
#define ELASTIC_CONSTANT 0.6f

typedef struct particle{
    float mass;
    float acel[2];
    float speed[2];
	int count;
    bool exists;
    bool closed;
    unsigned long color;
}particle;

typedef struct board{
    particle *particles;
    float *field;
}board;

void initDimensions(int row, int col);
board initBoard();
void initOrders();
void spawnParticles(board b, int row, float mass, int color, int count);
void particleStep(board *b);
