CPUC      = gcc
CFLAGS = -O3 --std=c11 -m64 -Werror -Wall -Wextra	-Wno-unused-parameter #     -Wunused-variable #-Wno-conversion-null -Wdelete-non-virtual-dtor
DEFS      =
INCLUDES  =
LIBDIR   = -L/usr/lib
LIBS     = -lm
LINK     =  $(LIBDIR) $(LIBS)
CPU_COMPILE_C  = $(CPUC) $(DEFS) $(INCLUDES) $(CFLAGS)

all:
	$(CPU_COMPILE_C) main-mdf.c  $(LINK)   -o mdf
	$(CPU_COMPILE_C) main-mdf-parallel.c  $(LINK)   -o mdf-parallel

clean:
	rm mdf
