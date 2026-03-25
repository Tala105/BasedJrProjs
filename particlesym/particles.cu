#include "particles.h"

__constant__ int ROW, COL;
int h_ROW = 1080, h_COL = 1920;

int *row_order;
int *col_order;

void initDimensions(int row, int col) {
  h_ROW = row;
  h_COL = col;
  cudaMemcpyToSymbol(ROW, &row, sizeof(int));
  cudaMemcpyToSymbol(COL, &col, sizeof(int));
}

board initBoard() {
  board b;
  cudaMalloc((void **)&b.particles, h_ROW * h_COL * sizeof(*b.particles));
  cudaMalloc((void **)&b.field, h_ROW * h_COL * 2 * sizeof(*b.field));
  cudaMemset(b.particles, 0, h_ROW * h_COL * sizeof(*b.particles));
  cudaMemset(b.field, 0, h_ROW * h_COL * 2 * sizeof(*b.field));

  float *tmp_field = (float *)calloc(h_ROW * h_COL * 2, sizeof(float));
  for (int i = 0; i < h_ROW; i++)
    for (int j = 0; j < h_COL; j++)
      tmp_field[(i * h_COL + j) * 2] = -GRAVITY;
  cudaMemcpy(b.field, tmp_field, h_ROW * h_COL * 2 * sizeof(float),
             cudaMemcpyHostToDevice);
  free(tmp_field);

  return b;
}

void shuffle(void *base, size_t n, size_t size) {
  if (n < 2)
    return;
  char *a = (char *)base;
  char *tmp = (char *)malloc(size);
  for (size_t i = n - 1; i > 0; i--) {
    size_t j = rand() % (i + 1);
    memcpy(tmp, a + i * size, size);
    memcpy(a + i * size, a + j * size, size);
    memcpy(a + j * size, tmp, size);
  }
  free(tmp);
}

void initOrders() {
  row_order = (int *)malloc(sizeof(int) * h_ROW);
  col_order = (int *)malloc(sizeof(int) * h_COL);
  for (int i = 0; i < h_ROW; i++)
    row_order[i] = i;
  for (int i = 0; i < h_COL; i++)
    col_order[i] = i;
  shuffle(row_order, h_ROW, sizeof(int));
  shuffle(col_order, h_COL, sizeof(int));
}

__global__ void spawnParticlesKernel(board b, int row, float mass, int color,
                                     int count) {
  int idx = blockIdx.x * blockDim.x + threadIdx.x;
  if (idx >= count)
    return;

  int col = ((idx * 2654435761u) ^ (clock64() * 2246822519u)) % COL;
  if (b.particles[row * COL + col].exists)
    return;

  particle p = {};
  p.mass = mass;
  p.exists = true;
  p.closed = (color == 0x444444);
  p.color = color;
  p.speed[0] = b.field[(row * COL + col) * 2];
  p.speed[1] = b.field[(row * COL + col) * 2 + 1];
  p.acel[0] = p.speed[0];
  p.acel[1] = p.speed[1];

  b.particles[row * COL + col] = p;
}

void spawnParticles(board b, int row, float mass, int color, int count) {
  if (count <= 0)
    return;
  spawnParticlesKernel<<<1, count>>>(b, row, mass, color, count);
}

__device__ float dotProd(float v1[2], float v2[2]) {
  return v1[0] * v2[0] + v1[1] * v2[1];
}

__device__ float norm(float v1[2]) { return sqrt(dotProd(v1, v1)); }

__device__ void differece(float v1[2], float v2[2], float *out) {
  out[0] = v1[0] - v2[0];
  out[1] = v1[1] - v2[1];
}

__device__ void clampPos(float *pos, particle *p) {
  if (pos[0] < 0) {
    pos[0] = fmax(fmin(pos[0], ROW - 1), 0);
    p->speed[0] *= -0.1;
  }
  if (pos[0] > ROW - 1) {
    pos[0] = fmax(fmin(pos[0], ROW - 1), 0);
    p->speed[0] *= -0.5;
  }
  if (pos[1] < 0 || pos[1] > COL - 1) {
    pos[1] = fmax(fmin(pos[1], COL - 1), 0);
    p->speed[1] *= -0.5;
  }
}

__device__ void projection(float *vector, float *direction, float *out) {
  direction[0] = direction[0] / norm(direction);
  direction[1] = direction[1] / norm(direction);
  out[0] = dotProd(vector, direction) * direction[0];
  out[1] = dotProd(vector, direction) * direction[1];
}

__device__ void collision(particle *a, particle *b, int pos[2]) {
  float cv1[2], cv2[2], pos1[2], pos2[2], proj[2];
  float nudge[2] = {
      (float)(((int)pos[0] * 2654435761u) ^ ((int)pos[1] * 2246822519u)) /
          UINT_MAX,
      (float)(((int)pos[0] * 2246822519u) ^ ((int)pos[1] * 2654435761u)) /
          UINT_MAX};

  cv1[0] = a->speed[0];
  cv1[1] = a->speed[1];
  cv2[0] = b->speed[0];
  cv2[1] = b->speed[1];

  float m1 = a->mass, m2 = b->mass, ang1 = atan(cv1[1] / cv1[0]),
        ang2 = atan(cv2[1] / cv2[0]), v1 = norm(cv1), v2 = norm(cv2),
        time = 2.0 / sqrt(v1 * v1 + v2 * v2 - 2 * v1 * v2 * cos(ang2 - ang1));

  pos1[0] = pos[0] - cv1[0] * time;
  pos1[1] = pos[1] - cv1[1] * time;
  pos2[0] = pos[0] - cv2[0] * time;
  pos2[1] = pos[1] - cv2[1] * time;

  float diffc[2], diffp[2];
  differece(cv1, cv2, diffc);
  differece(pos1, pos2, diffp);
  projection(diffc, diffp, proj);

  time = fmax(time, 0.1);
  a->speed[0] = cv1[0] - 2 * m2 / (m1 + m2) * proj[0] + nudge[0];
  a->speed[1] = cv1[1] - 2 * m2 / (m1 + m2) * proj[1] + nudge[1];
  b->speed[0] = cv2[0] - 2 * m1 / (m1 + m2) * proj[0] - nudge[0];
  b->speed[1] = cv2[1] - 2 * m1 / (m1 + m2) * proj[1] - nudge[1];
}

__global__ void field(board dst) {
  int row = blockIdx.y * blockDim.y + threadIdx.y;
  int col = blockIdx.x * blockDim.x + threadIdx.x;
  if (row >= ROW || col >= COL)
    return;

  dst.field[(row * COL + col) * 2] = -GRAVITY;
  particle *p = &dst.particles[row * COL + col];
  if (!p->exists)
    return;

  for (int i = -FIELD_RADIUS; i <= FIELD_RADIUS; i++)
    for (int j = -FIELD_RADIUS; j <= FIELD_RADIUS; j++) {
      if (!i && !j)
        continue;
      int nr = row + i;
      int nc = col + j;
      if (nr < 0 || nr >= ROW || nc < 0 || nc >= COL)
        continue;

      float dist = sqrt((float)(i * i + j * j));
      float magnitude = FIELD_CONSTANT * p->mass / (dist * dist) +
                        ELASTIC_CONSTANT * (dist - 2) * (dist <= 4);

      atomicAdd(&dst.field[(nr * COL + nc) * 2], magnitude * i / dist);
      atomicAdd(&dst.field[(nr * COL + nc) * 2 + 1], magnitude * j / dist);
    }
}

__global__ void movement(board src, board dst) {
  int row = blockIdx.y * blockDim.y + threadIdx.y;
  int col = blockIdx.x * blockDim.x + threadIdx.x;
  if (row >= ROW || col >= COL)
    return;

  particle *p = &src.particles[row * COL + col];
  if (!p->exists)
    return;

  float pos[2] = {row + p->speed[0], col + p->speed[1]};
  clampPos(pos, p);

  int tr = (int)roundf(pos[0]);
  int tc = (int)roundf(pos[1]);

  float new_speed_x = p->speed[0] + src.field[(row * COL + col) * 2];
  float new_speed_y = p->speed[1] + src.field[(row * COL + col) * 2 + 1];

  atomicAdd(&dst.particles[tr * COL + tc].mass, p->mass);
  atomicAdd(&dst.particles[tr * COL + tc].count, 1);
  atomicAdd(&dst.particles[tr * COL + tc].speed[0], new_speed_x);
  atomicAdd(&dst.particles[tr * COL + tc].speed[1], new_speed_y);
  dst.particles[tr * COL + tc].exists = true;
  dst.particles[tr * COL + tc].color = p->color;
}

__global__ void redistribution(board dst, int *d_overflow) {
  int row = blockIdx.y * blockDim.y + threadIdx.y;
  int col = blockIdx.x * blockDim.x + threadIdx.x;
  if (row >= ROW || col >= COL)
    return;

  particle *p = &dst.particles[row * COL + col];
  if (!p->exists || p->mass <= SOFT_CAP)
    return;
  if (p->count > 1) {
    p->speed[0] /= p->count;
    p->speed[1] /= p->count;
  }
  p->count = 0;

  float excess = p->mass - SOFT_CAP, actually_pushed = 0;

  int neighbors[8][2] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1},
                         {0, 1},   {1, -1}, {1, 0},  {1, 1}};

  int available = 0;
  for (int i = 0; i < 8; i++) {
    int nr = row + neighbors[i][0];
    int nc = col + neighbors[i][1];
    if (nr < 0 || nr >= ROW || nc < 0 || nc >= COL)
      continue;
    if (dst.particles[nr * COL + nc].mass < SOFT_CAP)
      available++;
  }

  if (available == 0)
    return;

  float push_per_neighbor = excess / available;

  for (int i = 0; i < 8; i++) {
    int nr = row + neighbors[i][0];
    int nc = col + neighbors[i][1];
    if (nr < 0 || nr >= ROW || nc < 0 || nc >= COL)
      continue;

    particle *n = &dst.particles[nr * COL + nc];
    if (n->mass >= SOFT_CAP)
      continue;

    float ratio = push_per_neighbor / p->mass;

    atomicAdd(&n->mass, push_per_neighbor);
    actually_pushed += push_per_neighbor;
    atomicAdd(&n->speed[0], p->speed[0] * ratio);
    atomicAdd(&n->speed[1], p->speed[1] * ratio);
    atomicAdd(&n->count, 1);
    n->exists = true;
  }

  p->mass -= actually_pushed;
  if (p->mass > HARD_CAP)
    atomicOr(d_overflow, 1);
}

void particleStep(board *b) {
  static board dst = {NULL, NULL};
  static int *d_overflow = NULL;

  if (dst.particles == NULL) {
    cudaMalloc((void **)&dst.particles, h_ROW * h_COL * sizeof(*dst.particles));
    cudaMalloc((void **)&dst.field, h_ROW * h_COL * 2 * sizeof(*dst.field));
    cudaMalloc((void **)&d_overflow, sizeof(int));
  }

  dim3 block(16, 16);
  dim3 grid((h_COL + 15) / 16, (h_ROW + 15) / 16);

  cudaMemset(b->field, 0, h_ROW * h_COL * 2 * sizeof(*b->field));
  field<<<grid, block>>>(*b);
  cudaDeviceSynchronize();

  cudaMemset(dst.particles, 0, h_ROW * h_COL * sizeof(*dst.particles));
  cudaMemset(dst.field, 0, h_ROW * h_COL * 2 * sizeof(*dst.field));

  movement<<<grid, block>>>(*b, dst);
  cudaDeviceSynchronize();

  int overflow, max_iter = HARD_CAP;
  do {
    cudaMemset(d_overflow, 0, sizeof(int));
    redistribution<<<grid, block>>>(dst, d_overflow);
    cudaDeviceSynchronize();
    cudaMemcpy(&overflow, d_overflow, sizeof(int), cudaMemcpyDeviceToHost);
  } while (overflow && --max_iter);

  particle *tmp_p = b->particles;
  float *tmp_f = b->field;
  b->particles = dst.particles;
  b->field = dst.field;
  dst.particles = tmp_p;
  dst.field = tmp_f;
}
