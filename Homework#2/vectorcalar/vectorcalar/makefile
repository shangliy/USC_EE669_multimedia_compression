# Uncomment the following for SunOS-4.1.x (Solaris-1.x) Sun's
#  AR = /bin/ar
#  RANLIB = /bin/ranlib
#  CC = cc

# Uncomment the following for Solaris-2.x Sun's
# Note that "ar" subsumes the "ranlib" functions on Solaris2.x machines
AR = /usr/ccs/bin/ar
RANLIB = /bin/true
CC = gcc

CFLAGS= -O # for debugging, change -O to -g
LDFLAGS = -L. -s # for debugging, comment out -s
LOADLIBS = -lstdvq -lm

#  Suffix rules
.SUFFIXES:
.SUFFIXES: .o .c
.c.o:	;	cc $(CFLAGS) -c $<
.o:	;	cc $(CFLAGS) $(LDFLAGS) $< $(LOADLIBS) -o $@

all:  stdvq stdvqe voronoi_fs

clean:
	rm *.o *.a

LIBSTDVQ_OBJ =	stdvq_util.o	\
		lloyd0.o	\
		lloyd1.o	\
		lloyd2.o

libstdvq.a: $(LIBSTDVQ_OBJ)
	$(AR) rv $@ $(LIBSTDVQ_OBJ)
	$(RANLIB) $@

stdvq: libstdvq.a vq.h stdvq.def stdvq.h stdvq.c
	cc $(CFLAGS) $(LDFLAGS) $@.c $(LOADLIBS) -o $@

stdvqe: libstdvq.a vq.h stdvq.def stdvq.h stdvqe.c
	cc $(CFLAGS) $(LDFLAGS) $@.c  $(LOADLIBS) -o $@

stdvq_util.o lloyd0.o lloyd1.o lloyd2.o: vq.h stdvq.def stdvq.h

voronoi_fs: vq.h voronoi_fs.c
	cc $(CFLAGS) $(LDFLAGS) $@.c -lm -o $@
