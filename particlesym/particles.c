#include "particles.h"

int ROW = 1080, COL = 1920;

int surrouding_area=5;
int near_area = 1;

int *row_order;
int *col_order;

board initBoard(){
	board b;
	b.particles = malloc(ROW * sizeof(particle *));
	b.field = malloc(ROW * sizeof(double (*)[2]));
	for (int i = 0; i < ROW; i++){
		b.particles[i] = calloc(COL, sizeof(particle));
		b.field[i] = calloc(COL, sizeof(double [2]));
		for(int j=0; j<COL; j++){
			b.field[i][j][0] = -0.05;
		}
	}
	return b;
}

void shuffle(void *base, size_t n, size_t size)
{
    if (n < 2) return;

    char *a = base;
    char tmp[size];

    for (size_t i = n - 1; i > 0; i--) {
        size_t j = rand() % (i + 1);

        memcpy(tmp, a + i * size, size);
        memcpy(a + i * size, a + j * size, size);
        memcpy(a + j * size, tmp, size);
    }
}

void initOrders() {
    row_order = malloc(sizeof(int) * ROW);
    col_order = malloc(sizeof(int) * COL);

    for (int i = 0; i < ROW; i++) row_order[i] = i;
    for (int i = 0; i < COL; i++) col_order[i] = i;

    shuffle(row_order, ROW, sizeof(int));
    shuffle(col_order, COL, sizeof(int));
}

void addField(board *b, int row, int col, bool remove){
	particle *p = &(b->particles[row][col]);
	float dist, effect_dist, fieldi = 0, fieldj = 0, pressure = 1;
	int sign = 1 - 2*remove;
	for(int i=0; i<surrouding_area; i++)
		for(int j=0; j<surrouding_area; j++){
			dist = i*i + j*j;
			if(!i && !j) continue;
			effect_dist = -(-1.05*(i<near_area || j<near_area) + 1)*p->mass/dist/100.0;
			fieldi = pressure*effect_dist*i;
			fieldj = pressure*effect_dist*j;
			if(row>=i){
				if(col>=j){
					b->field[row-i][col-j][0] += sign * fieldi;
					b->field[row-i][col-j][1] += sign * fieldj;
				}

				if(col+j<COL && j){
					b->field[row-i][col+j][0] += sign * fieldi;
					b->field[row-i][col+j][1] -= sign * fieldj;
				}
			}
			if(row+i<ROW && i){
				if(col>=j){
					b->field[row+i][col-j][0] -= sign * fieldi;
					b->field[row+i][col-j][1] += sign * fieldj;
				}
				if(col+j<COL && j){
					b->field[row+i][col+j][0] -= sign * fieldi;
					b->field[row+i][col+j][1] -= sign * fieldj;
				}
			}
		}
}

void spawnParticle(board *b, int row, int col, float mass, int color){
	if (col<0)
		col = rand()%COL;
	if (row<0)
		row = rand()%ROW;
	if(b->particles[row][col].exists) return;
	particle p = {.mass = mass, .exists = true, .closed=(color==0x444444), .color=color};
	memcpy(p.speed, b->field[row][col], sizeof(p.speed));
	memcpy(p.acel, b->field[row][col], sizeof(p.acel));

	b->particles[row][col] = p;
	addField(b, row, col, false);

}

void swap(particle *a, particle *b){
	particle t = *a;
	*a = *b;
	*b = t;
}

double dotProd(double v1[2], double v2[2]){
	return v1[0]*v2[0] + v1[1]*v2[1];
}

double norm(double v1[2]){
	return sqrt(dotProd(v1,v1));
}

void differece(double v1[2], double v2[2], double *out){
	out[0] = v1[0] - v2[0];
	out[1] = v1[1] - v2[1];
}

void clampPos(double *pos, particle *p){
	if (pos[0] < 0){
		pos[0] = fmax(fmin(pos[0], ROW-1), 0);
		p->speed[0] = 0;
	}
	if (pos[0] > ROW-1){
		pos[0] = fmax(fmin(pos[0], ROW-1), 0);
		p->speed[0] *= -0.5;
	}
	if(pos[1] < 0 || pos[1] > COL-1){
		pos[1] = fmax(fmin(pos[1], COL-1), 0);
		p->speed[1] *= -0.5;
	}
}

void projection(double *vector, double *direction, double *out){
	direction[0] = direction[0]/norm(direction);
	direction[1] = direction[1]/norm(direction);

	out[0] = dotProd(vector,direction)*direction[0];
	out[1] = dotProd(vector,direction)*direction[1];
}

void collision(particle *a, particle *b, int pos[2]){
	double cv1[2], cv2[2], pos1[2], pos2[2], proj[2],
		diffc[2], diffp[2], nudge[2] = {(float)rand()/INT_MAX, (float)rand()/INT_MAX};

	memcpy(cv1, a->speed, sizeof(a->speed));
	memcpy(cv2, b->speed, sizeof(b->speed));

	double m1 = a->mass, m2 = b->mass,
		ang1 = atan(cv1[1]/cv1[0]), ang2 = atan(cv2[1]/cv2[0]),
		v1 = norm(cv1), v2 = norm(cv2),
		time = 2.0/sqrt(v1*v1+v2*v2-2*v1*v2*cos(ang2-ang1));

	pos1[0] = pos[0]-cv1[0]*time, pos1[1] = pos[1]-cv1[1]*time;
	pos2[0] = pos[0]-cv2[0]*time, pos2[1] = pos[1]-cv2[1]*time;

	differece(cv1, cv2, diffc);
	differece(pos1, pos2, diffp);
	projection(diffc, diffp, proj);

	time = fmax(time, 0.1);
	a->speed[0] = cv1[0] - 2*m2/(m1+m2)*proj[0] + nudge[0];
	a->speed[1] = cv1[1] - 2*m2/(m1+m2)*proj[1] + nudge[1];
	b->speed[0] = cv2[0] - 2*m1/(m1+m2)*proj[0] - nudge[0];
	b->speed[1] = cv2[1] - 2*m1/(m1+m2)*proj[1] - nudge[1];
}

void particleStep(board *b){
    particle *p, *f;
    double pos[2];
    int row, col;
    char lateral[3] = {-1,0,1};
    int r, c;
    int found;

    for(int crow=0; crow<ROW; crow++){
        for(int ccol=0; ccol<COL; ccol++){
            row = row_order[crow];
            col = col_order[ccol];
            p = &(b->particles[row][col]);
            if(!p->exists) continue;

			pos[0] = row + p->speed[0];
			pos[1] = col + p->speed[1];
            clampPos(pos, p);

            int target_row = (int)floor(pos[0]);
            int target_col = (int)floor(pos[1]);
            f = &(b->particles[target_row][target_col]);

            found = 0;
            if(f->exists){
				// collision(p, f, pos);
                for(r = target_row; r <= row ; r++){
                    shuffle(lateral, 3, sizeof(char));
                    for(int i = 0; i < 3; i++){
                        c = col + lateral[i];
                        if(c < 0 || c >= COL) continue;
                        if(!b->particles[r][c].exists){
                            f = &(b->particles[r][c]);
                            found = 1;
                            break;
                        }
                    }
                    if(found) break;
                }
            }

			p->speed[0] += p->acel[0];
			p->speed[1] += p->acel[1];
            memcpy(p->acel, b->field[(int)floor(pos[0])][(int)floor(pos[1])], sizeof(p->acel));
            addField(b, row, col, true);
            swap(p, f);
            addField(b, pos[0], pos[1], false);
        }
    }
}
