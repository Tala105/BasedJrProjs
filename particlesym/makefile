CC = gcc
NVCC = nvcc
NVCCFLAGS = -O2 --expt-relaxed-constexpr
LIBS = -lm -lX11
all: render
render: render.o particles.o
	$(NVCC) $(NVCCFLAGS) -o render render.o particles.o $(LIBS)
render.o: render.cu particles.h
	$(NVCC) $(NVCCFLAGS) -c render.cu -o render.o
particles.o: particles.cu particles.h
	$(NVCC) $(NVCCFLAGS) -c particles.cu -o particles.o
clean:
	rm -f render.o particles.o render
