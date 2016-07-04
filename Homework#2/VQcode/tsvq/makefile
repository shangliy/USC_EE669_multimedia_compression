# Uncomment the following for SunOS-4.1.x (Solaris-1.x) Sun's
# AR = /bin/ar
# RANLIB = /bin/ranlib
# CC = cc

# Uncomment the following for Solaris-2.x Sun's
# Note that "ar" subsumes the "ranlib" functions on Solaris2.x machines
 AR = /usr/ccs/bin/ar
 RANLIB = /bin/true
 CC = gcc

CFLAGS= -O # for debugging, change -O to -g
LDFLAGS = -L. -s # for debugging, comment out -s
LOADLIBES = -ltree -lm

# Suffix rules
.SUFFIXES:
.SUFFIXES: .o .c

.c:	;	$(CC) $(CFLAGS) $(LDFLAGS) $*.c $(LOADLIBES) -o $@

.c.o:	;	$(CC) $(CFLAGS) -c $<

# the next two lines were added for SUN and GNU make functions
%: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) $*.c $(LOADLIBES) -o $@

all:  tsvqe tsvq prune select voronoi_ts

clean:
	rm *.o *.a

#
LIBTREE_OBJ =	node_util.o	\
		tsvq_util.o	\
		slope_util.o	\
		tsvqe_util.o	\
		read_util.o	\
		write_util.o	\
		prune_util.o	\
		voronoi_util.o
libtree.a: $(LIBTREE_OBJ)
	$(AR) rv $@ $(LIBTREE_OBJ)
	$(RANLIB) $@

tsvqe tsvq prune select voronoi_ts : libtree.a tsvq.h
node_util.o tsvq_util.o slope_util.o tsvqe_util.o read_util.o write_util.o prune_util.o voronoi_util.o : tsvq.h

