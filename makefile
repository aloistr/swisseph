# keep only a few active development files at the front, the rest 
# should be sorted alphabetically
#CFLAGS = -Aa -O -I/users/alois/lib +DA1.1
#CFLAGS = -Aa -I/users/alois/lib +O4 
#CFLAGS = -Aa -I/users/alois/lib -g -z +FPVZUO +DA1.1 -DNO_JPL -DNO_MOSHIER
#CFLAGS = -Aa -I/users/alois/lib -g -z +FPVZUO -DTRACE
#CFLAGS = -Aa -g -z
#CFLAGS = -I/users/alois/lib -ggdb -Wall
#CFLAGS = -ggdb -Wall
#note, -DTLSOFF defines TLS as ''. This makes static data such as swed visible in gdb.
#CFLAGS = -g -Wall -Wextra -DTLSOFF -fPIC -O2
#CFLAGS = -g -Wall -DTLSOFF -fPIC 
CFLAGS = -g -Wall -fPIC -O2
CFLAGS = -g -Wall -fPIC -O2 -Wunused-but-set-variable
#CFLAGS = -g -Wall -fPIC -Wunused-but-set-variable 

OP=$(CFLAGS)
REV=1.30
SWEVERSION=2.10
SWEVERSIONOLD=2.09.03

MOSH = swemmoon.o swemplan.o
SWEPH = sweph.o swephlib.o swejpl.o swedate.o swemmoon.o swemplan.o swehouse.o swecl.o swehel.o
SWEPHA = swepha.o swephlib.o swejpl.o swedate.o swemmoon.o swemplan.o
SWEOBJ = swecl.o sweph.o swephlib.o swejpl.o \
	swemmoon.o swemplan.o swedate.o swehouse.o swehel.o
# extra modules needed by Astrodienst internal programs
#ASWEOBJ = $(SWEOBJ)  sweephe4.o astrolib.o d2l.o
ASWEOBJ = $(SWEOBJ)  sweephe4.o astrolib.o d2l.o

LIBSWESOURCE = swedll.h swemmoon.c sweph.h swephlib.h \
	swehouse.c swephexp.h swetest.c \
	swehouse.h swemplan.c sweph.c swecl.c \
	swedate.c swejpl.c swemptab.h swehel.c \
	swedate.h swejpl.h sweodef.h swephlib.c swemini.c \
	swenut2000a.h 

PUBSOURCE = LICENSE swedll.h swemmoon.c sweph.h swephlib.h \
	swehouse.c swephexp.h swetest.c \
	swehouse.h swemplan.c sweph.c swecl.c \
	swedate.c swejpl.c swemptab.h swehel.c \
	swedate.h swejpl.h sweodef.h swephlib.c swemini.c seorbel.txt \
	sefstars.txt swenut2000a.h seleapsec.txt \
	sedeltat.txt.inactive \
	perl_swisseph/PerlSwissEph-*.tar.gz

PUBDOC = doc/swephin.cdr\
	doc/swephin.gif\
	doc/sweph.cdr\
	doc/sweph.gif \
	doc/swephprg.docx \
	doc/swephprg.htm \
	doc/swephprg.pdf \
	doc/swisseph.docx \
	doc/swisseph.htm \
	doc/swisseph.pdf 

INTSOURCE = $(PUBSOURCE) 

EXE = swetest swevents

.o :  
	cc $(OP) $@ $?  -lm -ldl
.c.o :  
	cc $(OP) -c $< 

swetest: swetest.o $(SWEPH)
	cc $(OP) -o swetest swetest.o $(SWEPH) -lm -ldl

clean:
	rm -f *.o $(EXE)


#
# make a local library libswe.so
libswe: $(ASWEOBJ) 
	cc -shared -Wl,-soname,libswe.so.1 -o libswe.so.$(SWEVERSION) $(ASWEOBJ) -lm 



# statically linked library libsweph.a
libsweph: $(ASWEOBJ) 
	ar r libsweph.a $(ASWEOBJ)


