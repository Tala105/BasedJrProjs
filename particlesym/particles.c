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
		for(int col=0; col<16; col++){
			if (particles[row][col].exists) printf(".");
			else printf(" ");
		}
		printf("\n");
	}
}

void particleStep(particle particles[SIZE][SIZE]){
	int left, right;
	for(int row=1; row<SIZE; row++){
		for(int col=0; col<SIZE; col++){
			if(particles[row][col].exists){
				while (!particles[row][col].closed){
					right=left=false;
					system("clear");
					printParticles(particles);
					if (!particles[row-1][col].exists){
						particles[row-1][col] = particles[row][col];
						particles[row][col] = (particle){0};
						row--;
					}
					else if(col>0 && !particles[row-1][col-1].exists) left = true;
					else if(col<SIZE-1 && !particles[row-1][col+1].exists) right = true;
					else{
						particles[row][col].closed = true;
						break;
					}
					if(right && left){
						if(rand()%2) right = false;
						else left = false;
					}
					if(left){
						particles[row-1][col+1] = particles[row][col];
						particles[row][col] = (particle){0};
						row--; col++;
					}
					if(right){
						particles[row-1][col-1] = particles[row][col];
						particles[row][col] = (particle){0};
						row--; col--;
					}
					if(row==0)
							particles[row][col].closed = true;
					usleep(250000);
				}
			}
		}
	}
}

int main(){
	particle particles[SIZE][SIZE] = {0}, baseparticle = {.speed = 1, .exists = true, .closed = false};
	int stuckparticles = 0, pos;
	char stop;
	srand(time(NULL));

	while(true){
		system("clear");
		printParticles(particles);
		pos = rand()%SIZE;
		if (rand()%2>-1) particles[SIZE-1][pos] = baseparticle;
		particleStep(particles);
		// scanf(" %c", &stop);
		// if(stop == 'q') break;
	}
	return 0;
}
