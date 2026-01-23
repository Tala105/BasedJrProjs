#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

#define true 1
#define false 0
#define SIZE 16

typedef int bool;

typedef struct particle{
	float speed;
	float offset;
	int exists;
	int closed;
}particle;

void printParticles(particle particles[SIZE][SIZE]){
	for(int row=SIZE-1; row>=0; row--){
		for(int col=0; col<SIZE; col++){
			if (particles[row][col].exists) printf("o");
			else printf(" ");
		}
		printf("\n");
	}
}

void spawnParticle(particle particles[SIZE][SIZE]){
	int pos = rand()%SIZE;
	particle baseparticle = {.speed = 1, .exists = true, .closed = false};

	if (rand()%3==1) particles[SIZE-1][pos] = baseparticle;
}

void particleStep(particle particles[SIZE][SIZE]){
	int left, right;
	for(int row=1; row<SIZE; row++){
		for(int col=0; col<SIZE; col++){
			if(particles[row][col].exists && !particles[row][col].closed){
				right=left=false;
				if (!particles[row-1][col].exists){
					particles[row-1][col] = particles[row][col];
					particles[row][col] = (particle){0};
				}
				else if(col>0 && !particles[row-1][col-1].exists) left = true;
				else if(col<SIZE-1 && !particles[row-1][col+1].exists) right = true;
				else particles[row][col].closed = true;

				if(right && left){
					if(rand()%2) right = false;
					else left = false;
				}
				else if(left){
					particles[row-1][col-1] = particles[row][col];
					particles[row][col] = (particle){0};
				}
				else if(right){
					particles[row-1][col+1] = particles[row][col];
					particles[row][col] = (particle){0};
				}

				if(row==0)
					particles[row][col].closed = true;
			}
		}
	}
}

int main(){
	particle particles[SIZE][SIZE] = {0};
	int stuckparticles = 0, pos;
	char stop;
	srand(time(NULL));
	spawnParticle(particles);

	printParticles(particles);
	while(true){
		system("clear");
		particleStep(particles);
		spawnParticle(particles);
		printParticles(particles);
		usleep(250000);
	}
	return 0;
}
