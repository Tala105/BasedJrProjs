#include "particles.h"

int ROW = 0, COL = 0;

int surrouding_area=5;
int near_area = 1;

int *row_order;
int *col_order;

board initBoard(){
	board b;
	b.particles = malloc(ROW * sizeof(particle *));
	b.field = malloc(ROW * sizeof(double complex *));
	for (int i = 0; i < ROW; i++){
		b.particles[i] = calloc(COL, sizeof(particle));
		b.field[i] = calloc(COL, sizeof(double complex));
		for(int j=0; j<COL; j++)
			b.field[i][j] = -0.5*I;
	}
	for (int i = 0; i < ROW; i++)
		for(int j=0; j<COL; j++){
			if(!i) spawnParticle(&b, i, j, 0.0, 0x444444);
			else if(i==ROW-1) spawnParticle(&b, i, j, 0.0, 0x444444);
			else if(!j) spawnParticle(&b, i, j, 0.0, 0x444444);
			else if(j==COL-1) spawnParticle(&b, i, j, 00.0, 0x444444);
	}
	return b;
}

void initOrders() {
    row_order = malloc(sizeof(int) * ROW);
    col_order = malloc(sizeof(int) * COL);
    for(int i = 0; i < ROW; i++) row_order[i] = i;
    for(int i = 0; i < COL; i++) col_order[i] = i;

    for(int i = ROW - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int t = row_order[i];
        row_order[i] = row_order[j];
        row_order[j] = t;
    }

    for(int i = COL - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int t = col_order[i];
        col_order[i] = col_order[j];
        col_order[j] = t;
    }
}

double complex clampPos(double complex pos){
	double row = creal(pos), col = cimag(pos);
	if (row<0) row*=-1;
	if (col<0) col*=-1;
	if (row>=ROW) row = row-ROW;
	if (col>=COL) col = col-COL;
	return row + I*col;
}

void spawnParticle(board *b, int row, int col, float mass, int color){
	if (col<0)
		col = rand()%COL;
	if (row<0)
		row = rand()%ROW;
	if(b->particles[row][col].exists) return;
	particle baseparticle = {.mass = mass, .speed = 0, .acel=0, .exists = true, .closed=(color==0x444444), .color=color};

	b->particles[row][col] = baseparticle;
	particle *p = &(b->particles[row][col]);
	float dist, effect_dist, pressure = cabs(b->field[row][col]);

	for(int i=0; i<surrouding_area; i++)
		for(int j=0; j<surrouding_area; j++){
			dist = i*i + j*j;
			effect_dist = (-0.3/dist*(i<near_area || j<near_area) + 1)*p->mass/dist;
			if(!i && !j) continue;
			if(row>=i){
				if(col>=j) b->field[row-i][col-j] += pressure*effect_dist*j + pressure*effect_dist*i*I;

				if(col+j<COL) b->field[row-i][col+j] -= pressure*effect_dist*j + pressure*effect_dist*i*I;
			}
			if(row+i<ROW){
				if(col>=j) b->field[row+i][col-j] += pressure*effect_dist*j-pressure*effect_dist*i*I;

				if(col+j<COL) b->field[row+i][col+j] -= pressure*effect_dist*j-pressure*effect_dist*i*I;
			}
		}
}

void swap(particle *a, particle *b){
	particle t = *a;
	*a = *b;
	*b = t;
}

void updateField(board *b, int row, int col, int prow, int pcol){
	particle *p = &(b->particles[prow][pcol]);
	float dist, effect_dist, pressure = cabs(b->field[row][col]); 
	for(int i=0; i<surrouding_area; i++)
		for(int j=0; j<surrouding_area; j++){
			if(!i && !j) continue;
			dist = i*i + j*j;
			effect_dist = (-0.3/dist*(i<near_area || j<near_area) + 1)*p->mass/dist;
			if(row>=i){
				if(col>=j) b->field[row-i][col-j] -= pressure*effect_dist*j-pressure*effect_dist*i*I;

				if(col+j<COL && j) b->field[row-i][col+j] += pressure*effect_dist*j-pressure*effect_dist*i*I;
			}
			if(row+i<ROW && i){
				if(col>=j) b->field[row+i][col-j] -= pressure*effect_dist*j+pressure*effect_dist*i*I;

				if(col+j<COL && j) b->field[row+i][col+j] += pressure*effect_dist*j+pressure*effect_dist*i*I;
			}
			if(prow>=i){
				if(pcol>=j) b->field[prow-i][pcol-j] += pressure*effect_dist*j+pressure*effect_dist*i*I;

				if(pcol+j<COL && j) b->field[prow-i][pcol+j] -= pressure*effect_dist*j+pressure*effect_dist*i*I;
			}
			if(prow+i<ROW && i){
				if(pcol>=j) b->field[prow+i][pcol-j] += pressure*effect_dist*j-pressure*effect_dist*i*I;

				if(pcol+j<COL && j) b->field[prow+i][pcol+j] -= pressure*effect_dist*j-pressure*effect_dist*i*I;
			}
		}
}

void collision(particle *a, particle *b){
}

void particleStep(board *b){
	particle *p, *f;
	double complex pos;
	for(int row=0; row<ROW; row++){
		for(int col=0; col<COL; col++){
			p = &(b->particles[row][col]);
			if(!p->exists) continue;
			pos = clampPos(row + I*col + p->speed);
			f = &(b->particles[(int)(creal(pos))][(int)(cimag(pos))]);
			if(f->exists) collision(p,f);
		}
	}
}
