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
        total += (k+1)*(k+1);
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
	int idx = ((i%(50000*COLOR_AMOUNT))/50000)%COLOR_AMOUNT;
	return colors[sample_cdf(cdf, idx)];
}

unsigned int getColorIndex(unsigned int color){
	for(int i=0; i<COLOR_AMOUNT; i++)
		if(colors[i] == color) return i;
	return 0;
}

int main(int argc, char *argv[]){

	bool bean = false;
	int spawn_col=-1;
    clock_t last = clock();
	unsigned int color = colors[0];
	int *cdf = malloc(sizeof(int) * COLOR_AMOUNT);
	build_cdf(cdf);

	if(argc > 2){
		COL = atoi(argv[1]);
		ROW = atoi(argv[2]);
		if(argc > 3){
			bean = true;
			spawn_col = COL/2;
		}
	}
	else exit(-1);

	particle **particles = malloc(ROW * sizeof(particle *));
	for (int i = 0; i < ROW; i++)
		particles[i] = calloc(COL, sizeof(particle));
	srand(time(NULL));

	if(bean)
		for(int row=ROW-2; row>ROW/2; row--)
			for(int col=0; col<COL; col++)
				if(row%2==col%2)
					spawnParticle(particles, row, col, 0, 0x000000);

    Display *dpy = XOpenDisplay(NULL);
	if (!dpy) return 1;
	Window root = DefaultRootWindow(dpy);
	GC gc = XCreateGC(dpy, root, 0, NULL);
	Pixmap buffer = XCreatePixmap(dpy, root, COL, ROW, DefaultDepth(dpy, DefaultScreen(dpy)));

	int i = 0, spawn_amount = 0;
	unsigned int mass = 0;
	while(i<2*1e7){
		XSetForeground(dpy, gc, 0x000000);
		XFillRectangle(dpy, buffer, gc, 0, 0, COL+1, ROW+1);
		for (int i = 0; i < ROW; i++)
			for (int j = 0; j < COL; j++)
				if (particles[i][j].exists){
					XSetForeground(dpy, gc, particles[i][j].color);
                    XFillRectangle(dpy, buffer, gc, j, ROW-i-1, 1, 1);
				}
		particleStep(particles, !bean);
		color = setColor(i, cdf);
		mass = getColorIndex(color)+1;
		spawn_amount = rand()%(COL/96);
		for(int j=0; j<spawn_amount; j++)
			spawnParticle(particles, ROW-1, spawn_col, mass, color);

		clock_t now = clock();
        clock_t elapsed = now - last;
        if(elapsed < FRAME_TIME){
            usleep((FRAME_TIME - elapsed) * 1000000 / CLOCKS_PER_SEC);
        }
        last = clock();
		i+=spawn_amount;
		XCopyArea(dpy, buffer, root, gc, 0, 0, COL, ROW, 1920, 0);
		XFlush(dpy);
	}

	for(int i=0; i<ROW; i++) free(particles[i]);
	free(particles);
	free(cdf);

	XFreePixmap(dpy, buffer);
    XFreeGC(dpy, gc);
    XCloseDisplay(dpy);
	return 0;
}
