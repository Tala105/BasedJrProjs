#include "particles.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#define FPS 20
#define FRAME_TIME (CLOCKS_PER_SEC / FPS)

__constant__ int ROW, COL;
unsigned int colors[] = {0xFFFFFF, 0xF4A460, 0xDEB887, 0xD2B48C, 0xDAA520,
                         0xFF7F50, 0xFF8C00, 0xCD853F, 0xD2691E, 0xA0522D,
                         0xFF6347, 0xB22222, 0x800000};
char COLOR_AMOUNT = sizeof(colors) / sizeof(colors[0]);
int total;

void build_cdf(int *cdf) {
  total = 0;
  for (int k = 0; k < COLOR_AMOUNT; k++) {
    total += pow(M_E, k + 1);
    cdf[k] = total;
  }
}

int sample_cdf(int *cdf, int idx_max) {
  int r = rand() % cdf[(idx_max > 0) * (idx_max - 1)];
  for (int k = 0; k < idx_max; k++)
    if (r < cdf[k])
      return k;
  return (idx_max > 0) * (idx_max - 1);
}

unsigned int setColor(int i, int *cdf) {
  int pam = 2500 / SIZE;
  int idx = ((i % (pam * COLOR_AMOUNT)) / pam) % COLOR_AMOUNT;
  return colors[sample_cdf(cdf, idx)];
}

unsigned int getColorIndex(unsigned int color) {
  for (int i = 0; i < COLOR_AMOUNT; i++)
    if (colors[i] == color)
      return i;
  return 0;
}

__global__ void renderKernel(board b, unsigned int *pixel_buffer,
                             unsigned int *d_colors, int color_amount,
                             int full_col, int full_row) {
  int row = blockIdx.y * blockDim.y + threadIdx.y;
  int col = blockIdx.x * blockDim.x + threadIdx.x;
  if (row >= ROW || col >= COL)
    return;
  particle *p = &b.particles[row * COL + col];
  unsigned int px = p->exists
                        ? d_colors[(int)fminf(p->mass / SOFT_CAP * color_amount,
                                              color_amount - 1)]
                        : 0x000000;

  for (int dy = 0; dy < SIZE; dy++)
    for (int dx = 0; dx < SIZE; dx++) {
      int pr = (ROW - row - 1) * SIZE + dy;
      int pc = col * SIZE + dx;
      if (pr < full_row && pc < full_col)
        pixel_buffer[pr * full_col + pc] = px;
    }
}

int main(int argc, char *argv[]) {
  int h_ROW = 1080;
  int h_COL = 1920;
  if (argc > 2) {
    h_COL = atoi(argv[1]);
    h_ROW = atoi(argv[2]);
  }
  int full_COL = h_COL;
  int full_ROW = h_ROW;
  h_COL /= SIZE;
  h_ROW = (h_ROW - 25) / SIZE;

  initDimensions(h_ROW, h_COL);
  cudaMemcpyToSymbol(ROW, &h_ROW, sizeof(int), 0, cudaMemcpyHostToDevice);
  cudaMemcpyToSymbol(COL, &h_COL, sizeof(int), 0, cudaMemcpyHostToDevice);

  initOrders();
  board b = initBoard();
  unsigned int *d_colors;
  cudaMalloc((void **)&d_colors, COLOR_AMOUNT * sizeof(unsigned int));
  cudaMemcpy(d_colors, colors, COLOR_AMOUNT * sizeof(unsigned int),
             cudaMemcpyHostToDevice);
  srand(time(NULL));

  unsigned int *pixel_buffer;
  cudaMallocHost((void **)&pixel_buffer,
                 full_ROW * full_COL * sizeof(unsigned int));

  unsigned int *d_pixel_buffer;
  cudaMalloc((void **)&d_pixel_buffer,
             full_ROW * full_COL * sizeof(unsigned int));

  Display *dpy = XOpenDisplay(NULL);
  if (!dpy)
    return 1;
  Window root = DefaultRootWindow(dpy);
  GC gc = XCreateGC(dpy, root, 0, NULL);

  XImage *img =
      XCreateImage(dpy, DefaultVisual(dpy, DefaultScreen(dpy)),
                   DefaultDepth(dpy, DefaultScreen(dpy)), ZPixmap, 0,
                   (char *)pixel_buffer, full_COL, full_ROW, 32, full_COL * 4);

  int *cdf = (int *)malloc(sizeof(int) * COLOR_AMOUNT);
  build_cdf(cdf);

  dim3 block(16, 16);
  dim3 grid((h_COL + 15) / 16, (h_ROW + 15) / 16);

  int i = 0, spawn_amount = 0;
  unsigned int color = colors[0];
  float mass = 0.0;
  clock_t last = clock();

  while (i < 2 * 1e6) {
    if (i < 2 * 1e6)
      spawn_amount = rand() % (h_COL / SIZE / 128);
    else
      spawn_amount = 0;
    color = setColor(i, cdf);
    mass = (getColorIndex(color) + 1) / 10.0;
    spawnParticles(b, h_ROW - 2, mass, color, spawn_amount);

    particleStep(&b);

    renderKernel<<<grid, block>>>(b, d_pixel_buffer, d_colors, COLOR_AMOUNT,
                                  full_COL, full_ROW);
    cudaDeviceSynchronize();

    cudaMemcpy(pixel_buffer, d_pixel_buffer,
               full_ROW * full_COL * sizeof(unsigned int),
               cudaMemcpyDeviceToHost);

    XPutImage(dpy, root, gc, img, 0, 0, 0, 0, full_COL, full_ROW);
    XFlush(dpy);

    clock_t now = clock();
    clock_t elapsed = now - last;
    if (elapsed < FRAME_TIME)
      usleep((FRAME_TIME - elapsed) * 1000000 / CLOCKS_PER_SEC);
    last = clock();
    i += spawn_amount;
  }

  free(b.particles);
  free(cdf);
  img->data = NULL;
  XDestroyImage(img);
  cudaFreeHost(pixel_buffer);
  cudaFree(d_pixel_buffer);
  XFreeGC(dpy, gc);
  XCloseDisplay(dpy);
  return 0;
}
