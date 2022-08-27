# this Makefile creates a SwissEph library and a swetest sample on 64-bit
# Redhat Enterprise Linux RHEL 7.

# The mode marked as 'Linux' should also work with the GNU C compiler
# gcc on other systems. 

# If you modify this makefile for another compiler, please
# let us know. We would like to add as many variations as possible.
# If you get warnings and error messages from your compiler, please
# let us know. We like to fix the source code so that it compiles
# free of warnings.
# send email to the Swiss Ephemeris mailing list.
# 

CFLAGS =  -g -Wall -fPIC # for Linux and other gcc systems
CFLAGS =  -O2 -Wall -fPIC # for Linux and other gcc systems
OP=$(CFLAGS)  
CC=cc	#for Linux

# compilation rule for general cases
.o :
	$(CC) $(OP) -o $@ $? -lm
.c.o:
	$(CC) -c $(OP) $<     

SWEOBJ = swedate.o swehouse.o swejpl.o swemmoon.o swemplan.o sweph.o\
	 swephlib.o swecl.o swehel.o

all:	swetest swetests swevents swemini

# build swetest with SE linked in, using dynamically linked system libraries libc, libm, libdl.
swetest: swetest.o libswe.a
	$(CC) $(OP) -o swetest swetest.o -L. -lswe -lm -ldl

# build a statically linked version of swetest. first find out where libc.a and libm.a reside,
# and add this path with -L like below
# a statically linked program will run on any Linux variant, independent of dynamic system libraries.
swetests: swetest.o $(SWEOBJ)
	$(CC)  $(OP) -static -L/usr/lib/x86_64-redhat-linux6E/lib64/ -o swetests swetest.o $(SWEOBJ) -lm -ldl

swevents: swevents.o libswe.a
	$(CC) $(OP) -o swevents swevents.o -L. -lswe -lm -ldl

swemini: swemini.o libswe.a
	$(CC) $(OP) -o swemini swemini.o -L. -lswe -lm -ldl

# create an archive and a dynamic link libary fro SwissEph
# a user of this library will inlcude swephexp.h  and link with -lswe

libswe.a: $(SWEOBJ)
	ar r libswe.a	$(SWEOBJ)

libswe.so: $(SWEOBJ)
	$(CC) -shared -o libswe.so $(SWEOBJ)

test:
	cd setest && make && ./setest t

test.exp:
	cd setest && make && ./setest -g t

clean:
	rm -f *.o swetest libswe*
	cd setest && make clean
	
###
swecl.o: swejpl.h sweodef.h swephexp.h swedll.h sweph.h swephlib.h
sweclips.o: sweodef.h swephexp.h swedll.h
swedate.o: swephexp.h sweodef.h swedll.h
swehel.o: swephexp.h sweodef.h swedll.h
swehouse.o: swephexp.h sweodef.h swedll.h swephlib.h swehouse.h
swejpl.o: swephexp.h sweodef.h swedll.h sweph.h swejpl.h
swemini.o: swephexp.h sweodef.h swedll.h
swemmoon.o: swephexp.h sweodef.h swedll.h sweph.h swephlib.h
swemplan.o: swephexp.h sweodef.h swedll.h sweph.h swephlib.h swemptab.h
sweph.o: swejpl.h sweodef.h swephexp.h swedll.h sweph.h swephlib.h
swephlib.o: swephexp.h sweodef.h swedll.h sweph.h swephlib.h
swetest.o: swephexp.h sweodef.h swedll.h
swevents.o: swephexp.h sweodef.h swedll.h
