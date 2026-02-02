#include "particles.h"
#include <math.h>

int ROW = 0, COL = 0;

void spawnParticle(particle **particles, int row, int col, unsigned int mass, int color){
	if (col<0)
		col = rand()%COL;
	if (row<0)
		row = rand()%ROW;
	particle baseparticle = {.mass = mass, .speedx = 0, .speedy = 0, .acelx=0, .acely=-0.1, .exists = true, .color=color};

	particles[row][col] = baseparticle;
}

void swap(particle *a, particle *b){
	particle t = *a;
	*a = *b;
	*b = t;
}

void particleStep(board *b){
	particle *p;
	bool up, down, left, right;
	float speedx, speedy;
	double total_speed;
	int prow, pcol;
	for(int row=0; row<ROW; row++){
		for(int col=0; col<COL; col++){
			prow=row;
			pcol=col;
			p = &(b->particles[row][col]);
			speedx = p->speedx;
			speedy = p->speedy;
			if(!p->exists) continue;
			total_speed = sqrt((pow(p->speedx,2)+pow(p->speedy,2)));
			for(; total_speed>0; total_speed--){
				up=down=left=right=false;
				if(speedy>=1 && prow<prow-1) up=true;
				if(speedy<=-1 && prow>0) down=true;
				if(speedx<=-1 && pcol>0) left=true;
				if(speedx>=1 && pcol<pcol-1) right=true;

				if((up || down) && (left || right)){
					if(rand()%(abs(speedy)+abs(speedx))<abs(speedy)) left=right=false;
					else up=down=false;
				}
				prow += up-down;
				pcol += right-left;
				swap(p, &(b->particles[prow][pcol]));
				p = &(b->particles[prow][pcol]);
				speedy += down-up;
				speedx += left-right;
			}
			p->speedx += p->acelx;
			p->speedy += p->acely;
			p->acelx += (b->fieldx[row][col])/(p->mass);
			p->acely += (b->fieldy[row][col])/(p->mass);
		}
	}
}
