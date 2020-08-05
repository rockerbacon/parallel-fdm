CPUC      = gcc
CFLAGS = -O3 --std=c11 -m64 -Wall -Wextra	-Wno-unused-parameter -fopt-info-vec-all=vectorize.log #     -Wunused-variable #-Wno-conversion-null -Wdelete-non-virtual-dtor
DEFS      =
INCLUDES  =
LIBDIR   = -L/usr/lib
LIBS     = -lm
LINK     =  $(LIBDIR) $(LIBS)
CPU_COMPILE_C  = $(CPUC) $(DEFS) $(INCLUDES) $(CFLAGS)

.PHONY: all

all: original parallel

original:
	$(CPU_COMPILE_C) main-mdf.c  $(LINK)   -o mdf

parallel:
	$(CPU_COMPILE_C) -fopenmp main-mdf-parallel.c  $(LINK)   -o mdf-parallel

clean:
	rm mdf
