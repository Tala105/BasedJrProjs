#include "particles.h"

typedef struct board{
	particle **particles;
	float **fieldx;
	float **fieldy;
}board;


int ROW = 1060, COL = 1920;

void spawnParticle(particle **particles, int row, int col, unsigned int mass, int color){
	if (col<0)
		col = rand()%COL;
	if (row<0)
		row = rand()%ROW;
	if (particles[row][col].exists) return;
	particle baseparticle = {.mass = mass, .speedx = 0, .speedy = 0, .acelx=0, .acely=-1, .exists = true, .color=color};

	particles[row][col] = baseparticle;
}

void swap(particle *a, particle *b, int row){
	particle t = *a;
	*a = *b;
	*b = t;
}

void particleStep(particle **particles, bool slides){
	bool down, left, right;
	particle *p, *d, *l, *r;
	for(int row=1; row<ROW; row++){
		for(int col=0; col<COL; col++){
			down=right=left=false;
			d=l=r=&(particle){.exists=false, .mass=0};

			p = &particles[row][col];
			d = &particles[row-1][col];
			if(col>0) l = &particles[row-1][col-1];
			if(col<COL-1) r = &particles[row-1][col+1];

			if(slides && p->exists){
				if (!d->exists || p->mass > d->mass) down = true;
				if(col>0 && (!l->exists || p->mass > l->mass)) left = true;
				if(col<COL-1 && (!r->exists || p->mass > r->mass)) right = true;
			}
			else if(p->exists){
				if(!d->exists || p->mass > d->mass) down = true;
				if(d->color == 0x000000){
					if(col>0 && (!l->exists|| p->mass > l->mass)) left = true;
					if(col<COL-1 && (!r->exists || p->mass > r->mass)) right = true;
				}
			}
			if(down) swap(p, d, row);
			else{
				if(right && left){
				if(rand()%2) right = false;
				else left = false;
				}
			
				if(left) swap(p, l, row);
				else if(right) swap(p, r, row);
				}
		}
	}
}

