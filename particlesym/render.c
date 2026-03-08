#include "particles.h"
#include <X11/Xlib.h>

#define FPS 15
#define FRAME_TIME (CLOCKS_PER_SEC / FPS)

unsigned int colors[] = {
    0xFFFFFF,
    0xF4A460,
    0xDEB887,
    0xD2B48C,
    0xDAA520,
    0xFF7F50,
    0xFF8C00,
    0xCD853F,
    0xD2691E,
    0xA0522D,
    0xFF6347,
    0xB22222,
    0x800000
};
char COLOR_AMOUNT = sizeof(colors)/sizeof(colors[0]);
int total;

void build_cdf(int *cdf) {
    total = 0;
    for(int k=0;k<COLOR_AMOUNT;k++){
        total += pow(M_E, k+1);
        cdf[k] = total;
    }
}

int sample_cdf(int *cdf, int idx_max){
    int r = rand() % cdf[(idx_max>0)*(idx_max-1)];
    for(int k=0;k<idx_max;k++)
        if(r < cdf[k]) return k;
    return (idx_max>0)*(idx_max-1);
}

unsigned int setColor(int i, int *cdf){
	int pam = 250/SIZE;
	int idx = ((i%(pam*COLOR_AMOUNT))/pam)%COLOR_AMOUNT;
	return colors[sample_cdf(cdf, idx)];
}

unsigned int getColorIndex(unsigned int color){
	for(int i=0; i<COLOR_AMOUNT; i++)
		if(colors[i] == color) return i;
	return 0;
}

int main(int argc, char *argv[]){

	int spawn_col=-1;
    clock_t last = clock();
	unsigned int color = colors[0];
	int *cdf = malloc(sizeof(int) * COLOR_AMOUNT);
	build_cdf(cdf);

	if(argc > 2){
		COL = atoi(argv[1]);
		ROW = atoi(argv[2]);
	}
	COL /= SIZE;
	ROW = ROW/SIZE-25;
	initOrders();

	board b = initBoard();
	srand(time(NULL));

    Display *dpy = XOpenDisplay(NULL);
	if (!dpy) return 1;
	Window root = DefaultRootWindow(dpy);
	GC gc = XCreateGC(dpy, root, 0, NULL);
	Pixmap buffer = XCreatePixmap(dpy, root, SIZE*COL, SIZE*ROW+1, DefaultDepth(dpy, DefaultScreen(dpy)));

	int i = 0, spawn_amount = 0;
	float mass = 0.0;
	while(i<2*1e5){
		XSetForeground(dpy, gc, 0x000000);
		XFillRectangle(dpy, buffer, gc, 0, 0, SIZE*COL+1, SIZE*ROW+1);
		for (int row = 0; row < ROW; row++)
			for (int col = 0; col < COL; col++)
				if (b.particles[row][col].exists){
					XSetForeground(dpy, gc, b.particles[row][col].color);
					XFillRectangle(dpy, buffer, gc, col*SIZE+1, (ROW-row-1)*SIZE+1, SIZE, SIZE);
					}
		particleStep(&b);
		FILE *log = fopen("stuck.csv", "a");
		for(int col = 0; col < COL; col++){
		particle *p = &(b.particles[ROW-1][col]);
		if(p->exists)
			fprintf(log, "%d,%d,%f,%f,%f,%f\n", ROW-1, col, p->speed[0], p->speed[1], p->acel[0], p->acel[1]);
		}
		fclose(log);

		color = setColor(i, cdf);
		mass = (getColorIndex(color)+1)/30.0;

		if(i<3*1e4) spawn_amount = rand()%(COL/SIZE/128);
		else spawn_amount = 0;
		for(int j=0; j<spawn_amount; j++)
			spawnParticle(&b, ROW-2, spawn_col, mass, color);

		clock_t now = clock();
        clock_t elapsed = now - last;
        if(elapsed < FRAME_TIME){
            usleep((FRAME_TIME - elapsed) * 1000000 / CLOCKS_PER_SEC);
        }
        last = clock();
		i+=spawn_amount;
			XCopyArea(dpy, buffer, root, gc, 0, 0, SIZE*COL, SIZE*ROW+1, 0, 0);
		XFlush(dpy);
	}

	for(int i=0; i<ROW; i++) free(b.particles[i]);
	free(b.particles);
	free(cdf);

	XFreePixmap(dpy, buffer);
    XFreeGC(dpy, gc);
    XCloseDisplay(dpy);
	return 0;
}
