#include "particles.h"

int ROW = 0, COL = 0;

int *row_order;
int *col_order;

board initBoard(){
	board b;
	b.particles = malloc(ROW * sizeof(particle *));
	b.fieldx = malloc(ROW * sizeof(float *));
	b.fieldy = malloc(ROW * sizeof(float *));
	for (int i = 0; i < ROW; i++){
		b.particles[i] = calloc(COL, sizeof(particle));
		b.fieldx[i] = calloc(COL, sizeof(float));
		b.fieldy[i] = calloc(COL, sizeof(float));
		for(int j=0; j<COL; j++)
			b.fieldy[i][j] = -0.94/2;
	}
	for (int i = 0; i < ROW; i++)
		for(int j=0; j<COL; j++){
			if(!i) spawnParticle(&b, i, j, 1/20.0, 0x000000);
			else if(i==ROW-1) spawnParticle(&b, i, j, 1/20.0, 0x000000);
			else if(!j) spawnParticle(&b, i, j, 1/20.0, 0x000000);
			else if(j==COL-1) spawnParticle(&b, i, j, 1/20.0, 0x000000);
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

void spawnParticle(board *b, int row, int col, float mass, int color){
	if (col<0)
		col = rand()%COL;
	if (row<0)
		row = rand()%ROW;
	if(b->particles[row][col].exists) return;
	particle baseparticle = {.mass = mass, .speedx = 0, .speedy = 0, .acelx=0, .acely=0, .exists = true, .closed=(color==0x000000), .color=color};

	b->particles[row][col] = baseparticle;
	particle *p = &(b->particles[row][col]);
	float dist;

	for(int i=1; i<4; i++)
		for(int j=1; j<4; j++){
			dist = sqrt(i*i + j*j);
			if(row>=i){
				if(col>=j){
					b->fieldx[row-i][col-j] += p->mass/dist*j;
					b->fieldy[row-i][col-j] += p->mass/dist*i;
				}
				if(col+j<COL){
					b->fieldx[row-i][col+j] -= p->mass/dist*j;
					b->fieldy[row-i][col+j] += p->mass/dist*i;
				}
			}
			if(row+i<ROW){
				if(col>=j){
					b->fieldx[row+i][col-j] += p->mass/dist*j;
					b->fieldy[row+i][col-j] -= p->mass/dist*i;
				}
				if(col+j<COL){
					b->fieldx[row+i][col+j] -= p->mass/dist*j;
					b->fieldy[row+i][col+j] -= p->mass/dist*i;
				}
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
	float dist;
	for(int i=0; i<4; i++)
		for(int j=0; j<4; j++){
			if(!i && !j) continue;
			dist = sqrt(i*i + j*j);
			dist = dist*dist;
			if(row>=i){
				if(col>=j){
					b->fieldx[row-i][col-j] -= p->mass/dist*j;
					b->fieldy[row-i][col-j] -= p->mass/dist*i;
				}
				if(col+j<COL){
					b->fieldx[row-i][col+j] += p->mass/dist*j;
					b->fieldy[row-i][col+j] -= p->mass/dist*i;
				}
			}
			if(row+i<ROW && i){
				if(col>=j){
					b->fieldx[row+i][col-j] -= p->mass/dist*j;
					b->fieldy[row+i][col-j] += p->mass/dist*i;
				}
				if(col+j<COL && j){
					b->fieldx[row+i][col+j] += p->mass/dist*j;
					b->fieldy[row+i][col+j] += p->mass/dist*i;
				}
			}
			if(prow>=i){
				if(pcol>=j){
					b->fieldx[prow-i][pcol-j] += p->mass/dist*j;
					b->fieldy[prow-i][pcol-j] += p->mass/dist*i;
				}
				if(pcol+j<COL && j){
					b->fieldx[prow-i][pcol+j] -= p->mass/dist*j;
					b->fieldy[prow-i][pcol+j] += p->mass/dist*i;
				}
			}
			if(prow+i<ROW && i){
				if(pcol>=j){
					b->fieldx[prow+i][pcol-j] += p->mass/dist*j;
					b->fieldy[prow+i][pcol-j] -= p->mass/dist*i;
				}
					if(pcol+j<COL){
					b->fieldx[prow+i][pcol+j] -= p->mass/dist*j;
					b->fieldy[prow+i][pcol+j] -= p->mass/dist*i;
				}
			}
		}
}

void particleStep(board *b){
	particle *p;
	bool up, down, left, right;
	float speedx, speedy;
	double total_speed;
	int row, col, prow, pcol;
	for(int row_index=0; row_index<ROW; row_index++){
		for(int col_index=0; col_index<COL; col_index++){
			row = row_order[row_index];
			col = col_order[col_index];
			p = &(b->particles[row][col]);
			if(!p->exists) continue;
			if(p->closed) continue;
			prow=row;
			pcol=col;
			speedx = p->speedx;
			speedy = p->speedy;
			total_speed = sqrt(speedx*speedx+speedy*speedy);
			for(; total_speed>1;){
				up=down=left=right=false;
				if(speedy>=1 && prow<ROW-1 && !b->particles[prow+1][pcol].exists) up=true;
				if(speedy<=-1 && prow>0 && !b->particles[prow-1][pcol].exists) down=true;
				if(speedx<=-1 && pcol>0 && !b->particles[prow][pcol-1].exists) left=true;
				if(speedx>=1 && pcol<COL-1 && !b->particles[prow][pcol+1].exists) right=true;

				if((up || down) && (left || right)){
					if((float)rand() / RAND_MAX < fabs(speedy) / (fabs(speedx) + fabs(speedy))) left=right=false;
					else up=down=false;
				}
				prow += up-down;
				pcol += right-left;
				if(!b->particles[prow][pcol].exists){
					swap(p, &(b->particles[prow][pcol]));
					p = &(b->particles[prow][pcol]);
					speedy += down-up;
					speedx += left-right;
					total_speed = sqrt(speedx*speedx+speedy*speedy);
				}
				else if(up||down||left||right){
					if(up||down){
						p->acely=0;
						p->speedy=0;
					}
					else{
						p->acelx=0;
						p->speedx=0;
					}
					break;
				}
				else break;
			}
			if(prow!=row || pcol!=col) updateField(b, row, col, prow, pcol);
			p->acelx = (b->fieldx[prow][pcol]);
			p->acely = (b->fieldy[prow][pcol]);
			p->speedx += p->acelx;
			p->speedy += p->acely;
		}
	}
}
