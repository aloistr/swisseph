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
CFLAGS = -g -Wall -DTLSOFF -fPIC 
LIBS= /usr/lib/libastros.a \
		/usr/lib/libc.a \
		/usr/lib/libm.a

OP=$(CFLAGS)
REV=1.30
SWEVERSION=2.05

MOSH = swemmoon.o swemplan.o
SWEPH = sweph.o swephlib.o swejpl.o swedate.o swemmoon.o swemplan.o 
SWEPHA = swepha.o swephlib.o swejpl.o swedate.o swemmoon.o swemplan.o
SWEOBJ = sweph.o swephlib.o swejpl.o \
	swemmoon.o swemplan.o swedate.o 

PUBSOURCE = LICENSE swedll.h swemmoon.c swepcalc.h sweph.h swephlib.h \
	swepcalc.c swepdate.c swephexp.h swetest.c \
	swemplan.c sweph.c \
	swedate.c swejpl.c swemptab.c \
	swedate.h swejpl.h sweodef.h swephlib.c swemini.c seorbel.txt \
	sefstars.txt swenut2000a.h seleapsec.txt \
	sedeltat.txt.inactive \
	perl_swisseph/PerlSwissEph-*.tar.gz

PUBDOC = doc/swephin.cdr\
	doc/swephin.gif\
	doc/sweph.cdr\
	doc/sweph.gif \
	doc/swephprg.doc \
	doc/swephprg.htm \
	doc/swephprg.pdf \
	doc/swisseph.doc \
	doc/swisseph.htm \
	doc/swisseph.pdf 

INTSOURCE = $(PUBSOURCE) 

.o :  
	cc $(OP) -o $@ $? -lastros -lf2c -lm
.c.o :  
	cc $(OP) -c $< 

swetest: swetest.o $(SWEPH)
	cc $(OP) -o swetest swetest.o $(SWEPH) -lastros -lm -ldl

# a version of swetest with dynamical library
swetestx: swetest.o 
	cc $(OP) -o swetestx swetest.o -lswex -lastro -lm -ldl

# make a dynamical library libswex.so
# a symbolic link /usr/lib64/libswex.so -> /usr/local/lib64/swe/libswex.so
# must exist already
libswex: $(SWEOBJ) astrolib.h
	cc -shared -Wl,-soname,libswex.so.1 -o libswex.so.$(SWEVERSION) $(SWEOBJ) -lm 
	cp -p libswex.so.$(SWEVERSION) /usr/local/lib64/swe
	ln -sf /usr/local/lib64/swe/libswex.so.$(SWEVERSION) /usr/local/lib64/swe/libswex.so.1
	ln -sf /usr/local/lib64/swe/libswex.so.$(SWEVERSION) /usr/local/lib64/swe/libswex.so
#	echo "/usr/local/lib64/swe/libswe.so.$(SWEVERSION)" >> /users/dieter/sweph/perl_swisseph/MANIFEST

swet: swet.o $(SWEPH)
	cc $(OP) -o swet swet.o $(SWEPH) -lastro -lm

swet2: swet2.o $(SWEPH)
	cc $(OP) -o swet2 swet2.o $(SWEPH) -lastro -lm

swets: swetest.o 
	cc $(OP) -o swets swetest.o -L/users/alois/lib -lswe -lm

swetsx: swetest.o 
	cc $(OP) -o swets swetest.o -L. -lswe

testutc: testutc.o $(SWEPH)
	cc $(OP) -o testutc testutc.o $(SWEPH) -lastro -lm

testx: testx.o $(SWEPH)
	cc $(OP) -o testx testx.o $(SWEPH) -lastro -lm

st2: st2.o $(SWEPH)
	cc $(OP) -o st2 st2.o $(SWEPH) -lastro -lm

sofatot: sofatot.o $(SWEPH)
	cc $(OP) -o sofatot sofatot.o $(SWEPH) -lastro -lm

threadtest: threadtest.c
	gcc -std=c99 -Wall $(SWEPH) -o threadtest threadtest.c  -lm -pthread  

teo: teo.o $(SWEPH)
	cc $(OP) -o teo teo.o $(SWEPH) -lastro -lm

gen_frame_bias: gen_frame_bias.o $(SWEPH)
	cc $(OP) -o gen_frame_bias gen_frame_bias.o $(SWEPH) -lastro -lm

tsid: tsid.o $(SWEPH)
	cc $(OP) -o tsid tsid.o $(SWEPH) -lastro -lm

testbug: testbug.o $(SWEPH)
	cc $(OP) -o testbug testbug.o $(SWEPH) -lastro -lm

testlat: testlat.o $(SWEPH)
	cc $(OP) -o testlat testlat.o $(SWEPH) -lastro -lm

swetesth: swetesth.o $(SWEPH)
	cc $(OP) -o swetesth swetesth.o $(SWEPH) -lastro -lm

chcal: chcal.o $(SWEPH)
	cc $(OP) -o chcal chcal.o $(SWEPH) -lastro -lm

sweasp: sweasp.o $(SWEPH)
	cc $(OP) -o sweasp sweasp.o $(SWEPH) -lastro -lm

xtest: xtest.o $(SWEPH)
	cc $(OP) -o xtest xtest.o $(SWEPH) -lastro -lm

voc: voc.o $(SWEPH)
	cc $(OP) -o voc voc.o $(SWEPH) -lastro -lm

cccrypt: cccrypt.o $(SWEPH)
	cc $(OP) -o cccrypt cccrypt.o $(SWEPH) -lastro -lm

swetest1: swetest1.o $(SWEPH)
	cc $(OP) -o swetest1 swetest1.o $(SWEPH) -lastro -lm

rectdate: rectdate.o swedate.o
	cc $(OP) -o rectdate rectdate.o swedate.o -lastro -lm

julday: julday.o swedate.o
	cc $(OP) -o julday julday.o swedate.o -lastro -lm

st:	st.o $(SWEPH)
	cc $(OP) -o st st.o $(SWEPH) -lastro -lm

swetest2: swetest.o $(SWEPH)
	cc $(OP) -DMOSH_MOON_200 -c swemmoon.c
	cc $(OP) -o swetest2 swetest.o $(SWEPH) -lastro -lm

swetesta: swetest.o $(SWEPHA)
	cc $(OP) -o swetesta swetest.o $(SWEPHA) -lastro -lm

src:	$(INTSOURCE)
	-chmod ug+w $(INTSOURCE)
	-cp -p $(INTSOURCE) /users/alois/lib
	rsync  -ptgoLv $(PUBSOURCE) wwx:/ufs/src
	rsync -ptgoLv $(PUBDOC) wwx:/ufs/doc
	#chmod 664 /ufs/doc/*

srci:	$(INTSOURCE)
	-chmod ug+w $(INTSOURCE)
	-cp -p $(INTSOURCE) /users/alois/lib

as75:	$(INTSOURCE)
	-rcp -p $(INTSOURCE) as75:/users/dieter/sweph
	-rcp -p $(INTSOURCE) as75:/users/alois/lib

ci:	$(INTSOURCE)
	ci -u -r$(REV) $(INTSOURCE)
	chmod 664 $(INTSOURCE)

co:	$(INTSOURCE)
	co -l -r$(REV) $(INTSOURCE)
	chmod 664 $(INTSOURCE)

swetrace: swetrace.o $(SWEPH) swehouse.o
	cc $(OP) -o swetrace swetrace.o $(SWEPH) -lastro -lm

swete3: swete3.o $(SWEPH)
	cc $(OP) -o swete3 swete3.o $(SWEPH) -lastro -lm

swete2: swete2.o $(SWEPH)
	cc $(OP) -o swete2 swete2.o $(SWEPH) -lastro -lm

venus: venus.o $(SWEPH)
	cc $(OP) -lcairo -lpango-1.0 -lpangocairo-1.0 -o venus venus.o  $(SWEPH) -lastro -lm

venusold: venusold.o $(SWEPH) 
	cc $(OP) -o venusold venusold.o  $(SWEPH) -lastro -lm

swevents: swevents.o $(SWEPH)
	cc $(OP) -o swevents swevents.o $(SWEPH) -lastro -lm


sweht: sweht.o swehouse.o swephlib.o swedate.o
	cc $(OP) -o sweht sweht.o swehouse.o swephlib.o swedate.o -lastro -lm

se: 
	nice -20 chopt -p0 -makeph -whole
	nice -20 chopt -p9 -makeph -whole

sepltest: sepltest.o 
	cc $(OP) -o sepltest sepltest.o /users/alois/lib/libastros.a /users/alois/lib/libaswe.a -lm

sehtest: sehtest.o 
	cc $(OP) -o sehtest sehtest.o /users/alois/lib/libastros.a /users/alois/lib/libaswe.a -lm

#sepltest: sepltest.o swepcalc.o sweph.o swephlib.o swejpl.o swedate.o $(MOSH)
#	cc $(OP) -o sepltest sepltest.o swepcalc.o sweph.o swephlib.o swejpl.o swedate.o $(MOSH) /users/alois/lib/libastros.a -lm

swephgen: swephgen.o sweephe4.o sweph.o swephlib.o swejpl.o swemmoon.o swemplan.o swedate.o
	cc $(OP) -o swephgen swephgen.o sweephe4.o sweph.o swephlib.o swejpl.o swemmoon.o swemplan.o swedate.o -lastro  -lm

ts:	venus
	venus -mscreen -p3 -b1.1.1996 -s1 -n100000 >s

u:
	make
	make ts

swecomp: swecomp.o sweph.o swephlib.o swejpl.o swedate.o $(MOSH)
	cc $(OP) -o swecomp swecomp.o sweph.o swephlib.o swejpl.o swedate.o $(MOSH) -lastro -lm

chopt: chopt.o
	cc $(OP) -o chopt chopt.o $(LIBS) -lm

swdeltat: swdeltat.o 
	cc $(OP) -o swdeltat swdeltat.o -lastro -lm

earth: earth.o de4sub.o moshmoon.o
	cc $(OP) -o earth earth.o de4sub.o moshmoon.o -lastro -lm

d4mo: d4mo.o de4sub.o 
	cc $(OP) -o d4mo d4mo.o de4sub.o  -lastro -lm

de4test2: de4test2.o de4sub2.o
	cc $(OP) -o $@ $? -lastro -lm

###
chopt.o: sweodef.h de4sub.h swephexp.h swedll.h sweph.h swephlib.h
de4sub.o: sweodef.h de4sub.h
moshmoon.o: sweodef.h swephexp.h swedll.h sweph.h swephlib.h
printmod.o: /users/alois/lib/astrolib.h swepcalc.h swephexp.h sweodef.h \
	swedll.h /users/alois/lib/iso12ps.h
st.o: ourdef.h swephexp.h sweodef.h swedll.h
swedate.o: swephexp.h sweodef.h swedll.h
sweephe4.o: swephexp.h sweodef.h swedll.h sweephe4.h swepcalc.h \
	/users/alois/lib/astrolib.h ourfiles.h
swehouse.o: swephexp.h sweodef.h swedll.h swephlib.h swehouse.h
sweht.o: swephexp.h sweodef.h swedll.h swehouse.h
swejpl.o: swephexp.h sweodef.h swedll.h sweph.h swejpl.h
swemini.o: swephexp.h sweodef.h swedll.h
swemmoon.o: swephexp.h sweodef.h swedll.h sweph.h swephlib.h
wemplan.o: swephexp.h sweodef.h swedll.h sweph.h swephlib.h swemptab.c
swepcalc.o: swepcalc.h swephexp.h sweodef.h swedll.h
swepdate.o: swepcalc.h swephexp.h sweodef.h swedll.h
sweph.o: swejpl.h sweodef.h swephexp.h swedll.h sweph.h swephlib.h
swephgen.o: swephexp.h sweodef.h swedll.h sweephe4.h swepcalc.h ourfiles.h \
	/users/alois/lib/astrolib.h
swephlib.o: swephexp.h sweodef.h swedll.h sweph.h swephlib.h
swetest.o: swephexp.h sweodef.h swedll.h
swetrace.o: sweodef.h swephexp.h swedll.h
swevents.o: swephexp.h sweodef.h swedll.h swepcalc.h
venus.o: swephexp.h sweodef.h swedll.h sweph.h
venusold.o: ourdef.h swephexp.h sweodef.h swedll.h swephlib.h sweph.h
