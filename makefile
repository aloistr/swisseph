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
	cc $(OP) $@ $?  -lastro  -lm -ldl
.c.o :  
	cc $(OP) -c $< 

swetest: swetest.o $(SWEPH)
	cc $(OP) -o swetest swetest.o $(SWEPH) -lastro -lm -ldl

# a version of swetest with dynamical library
swetestx: swetest.o 
	cc $(OP) -o swetestx swetest.o -lswex -lastro -lm -ldl

st9:	st9.o
	cc $(OP) -o st9 st9.o  $(SWEOBJ) -lcheck -lm -ldl

swetd: swetd.o $(SWEPH)
	cc $(OP) -o swetd swetd.o $(SWEPH) -lastro -lm -ldl

clean:
	rm -f *.o $(EXE)


# make a local library libswe.so
test: $(ASWEOBJ) sweephe4.h
	cc -shared -Wl -o libswe.so.2.06.01b04 $(ASWEOBJ) -lm 
#
# make a local library libswe.so
#libswe: $(ASWEOBJ) sweephe4.h astrolib.h
libswe: $(ASWEOBJ) 
	cc -shared -Wl,-soname,libswe.so.1 -o libswe.so.$(SWEVERSION) $(ASWEOBJ) -lm 

libswe_install: libswe
	rsync -av libswe.so.$(SWEVERSION) /usr/local/lib64/swe
	ln -sf /usr/local/lib64/swe/libswe.so.$(SWEVERSION) /usr/local/lib64/swe/libswe.so.1
	ln -sf /usr/local/lib64/swe/libswe.so.$(SWEVERSION) /usr/local/lib64/swe/libswe.so
	echo "/usr/local/lib64/swe/libswe.so.$(SWEVERSION)" >> /users/dieter/sweph/perl_swisseph/MANIFEST

# release to wwx
LOCAL_LIB_DIR = /usr/local/lib64/swe
libswer: 
	rsync -av  $(LOCAL_LIB_DIR)/libswe.so.$(SWEVERSION) wwx:$(LOCAL_LIB_DIR)
	ssh wwx ln -sf /usr/local/lib64/swe/libswe.so.$(SWEVERSION) /usr/local/lib64/swe/libswe.so.1
	ssh wwx ln -sf /usr/local/lib64/swe/libswe.so.$(SWEVERSION) /usr/local/lib64/swe/libswe.so

libsweold: 
	ln -sf /usr/local/lib64/swe/libswe.so.$(SWEVERSIONOLD) /usr/local/lib64/swe/libswe.so.1
	ln -sf /usr/local/lib64/swe/libswe.so.$(SWEVERSIONOLD) /usr/local/lib64/swe/libswe.so
	rsync -av --no-links $(LOCAL_LIB_DIR)/libswe* wwx:$(LOCAL_LIB_DIR)
	rsync -av $(LOCAL_LIB_DIR)/libswe* wwx:$(LOCAL_LIB_DIR)

libsweoldi: 
	ln -sf /usr/local/lib64/swe/libswe.so.$(SWEVERSIONOLD) /usr/local/lib64/swe/libswe.so.1
	ln -sf /usr/local/lib64/swe/libswe.so.$(SWEVERSIONOLD) /usr/local/lib64/swe/libswe.so

#libswe: $(SWEOBJ) sweephe4.h
#	cc -shared -Wl,-soname,libswe.so.1 -o libswe.so.$(SWEVERSION) $(SWEOBJ) -lm 
#	ln -sf ./libswe.so.$(SWEVERSION) ./libswe.so.1
#	ln -sf ./libswe.so.$(SWEVERSION) ./libswe.so

#libswex: $(ASWEOBJ) sweephe4.h astrolib.h
# make a dynamical library libswex.so
# a symbolic link /usr/lib64/libswex.so -> /usr/local/lib64/swe/libswex.so
# must exist already
libswex: $(ASWEOBJ) 
	cc -shared -Wl,-soname,libswex.so.1 -o libswex.so.$(SWEVERSION) $(ASWEOBJ) -lm 
	cp -p libswex.so.$(SWEVERSION) /usr/local/lib64/swe
	ln -sf /usr/local/lib64/swe/libswex.so.$(SWEVERSION) /usr/local/lib64/swe/libswex.so.1
	ln -sf /usr/local/lib64/swe/libswex.so.$(SWEVERSION) /usr/local/lib64/swe/libswex.so
#	echo "/usr/local/lib64/swe/libswe.so.$(SWEVERSION)" >> /users/dieter/sweph/perl_swisseph/MANIFEST

# statically linked library libsweph.a
libsweph: $(ASWEOBJ) 
	ar r libsweph.a $(ASWEOBJ)
	cp libsweph.a /usr/lib64

swesubdir:
	mkdir sweph$(SWEVERSION)
	cp -p swe*.c *.h swetest sefstar* seleapsec* seorbel* sedeltat* sweph$(SWEVERSION)
	cp -p swe*.c *.h swetest sefstar* seleapsec* seorbel* sedeltat* sweph$(SWEVERSION)
	cp -p makefile sweph$(SWEVERSION)
	cp -p sweph_release.memo sweph$(SWEVERSION)
	cp -p LICENSE sweph$(SWEVERSION)
	cp -p todo*.memo sweph$(SWEVERSION)
	cd sweph$(SWEVERSION)
	ln -s /home/alois/lib/sweephe4.o .
	ln -s /home/alois/lib/astrolib.o .
	ln -s /home/alois/lib/d2l.o .

swet: swet.o $(SWEPH)
	cc $(OP) -o swet swet.o $(SWEPH) -lastro -lm -ldl

t2: t2.o $(SWEPH)
	cc $(OP) -o t2 t2.o $(SWEPH) -lastro -lm -ldl

swet2: swet2.o $(SWEPH)
	cc $(OP) -o swet2 swet2.o $(SWEPH) -lastro -lm -ldl

swets: swetest.o 
	cc $(OP) -o swets swetest.o -L/users/alois/lib -lswe -lm

swetsx: swetest.o 
	cc $(OP) -o swets swetest.o -L. -lswe

testutc: testutc.o $(SWEPH)
	cc $(OP) -o testutc testutc.o $(SWEPH) -lastro -lm

jplhor4chopt: jplhor4chopt.o  $(SWEPH) 
	cc $(OP) -o jplhor4chopt jplhor4chopt.o  $(SWEPH) -lastro -lm -ldl

testx: testx.o $(SWEPH)
	cc $(OP) -o testx testx.o $(SWEPH) -lastro -lm -ldl

testx2: testx2.o $(SWEPH)
	cc $(OP) -o testx2 testx2.o  $(SWEPH) -lastro -lm -ldl

planetary_hours: planetary_hours.o $(SWEPH)
	cc $(OP) -o planetary_hours planetary_hours.o $(SWEPH) -lastro -lm -ldl

testrise: testrise.o $(SWEPH)
	cc $(OP) -o testrise testrise.o $(SWEPH) -lastro -lm -ldl

st2: st2.o $(SWEPH)
	cc $(OP) -o st2 st2.o $(SWEPH) -lastro -lm -ldl

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

swehel: sweh.o $(SWEPH)
	cc $(OP) -o swehel sweh.o $(SWEPH) -lastro -lm

swehelx: sweh.o swehelx.o $(SWEPH)
	cc $(OP) -o swehelx sweh.o swehelx.o $(SWEPH) -lastro -lm

swehely: swehelx.o $(SWEPH)
	cc $(OP) -o swehelx swehelx.o $(SWEPH) -lastro -lm

chcal: chcal.o $(SWEPH)
	cc $(OP) -o chcal chcal.o $(SWEPH) -lastro -lm -ldl

sweasp: sweasp.o $(SWEPH)
	cc $(OP) -o sweasp sweasp.o $(SWEPH) -lastro -lm

xtest: xtest.o $(SWEPH)
	cc $(OP) -o xtest xtest.o $(SWEPH) -lastro -lm

voc: voc.o $(SWEPH)
	cc $(OP) -o voc voc.o $(SWEPH) -lastro -lm -ldl

tvoc: tvoc_neu.o $(SWEPH)
	cc $(OP) -o tvoc tvoc_neu.o $(SWEPH) -lastro -lm -ldl

cccrypt: cccrypt.o $(SWEPH)
	cc $(OP) -o cccrypt cccrypt.o $(SWEPH) -lastro -lm

sweclips: sweclips.o $(SWEPH)
	cc $(OP) -o sweclips sweclips.o $(SWEPH) -lastro -lm

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
	rsync  -ptgoLv $(PUBSOURCE) wwx:/ufs/src
	rsync  -ptgoLv sefstars.txt wwx:/ufs/ephe
	rsync -ptgoLv $(PUBDOC) wwx:/ufs/doc
	#chmod 664 /ufs/doc/*
	#-cp -p $(INTSOURCE) /users/alois/lib

srci:	$(LIBSWESOURCE)
	-chmod ug+w $(LIBSWESOURCE)
	-cp -p $(LIBSWESOURCE) /users/alois/lib

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
	cc $(OP) -o swevents swevents.o $(SWEPH) -lastro -lm -ldl


sweht: sweht.o swehouse.o swephlib.o swedate.o
	cc $(OP) -o sweht sweht.o swehouse.o swephlib.o swedate.o -lastro -lm

se: 
	nice -20 chopt -p0 -makeph -whole
	nice -20 chopt -p9 -makeph -whole

sepltest: sepltest.o 
	cc $(OP) -o sepltest sepltest.o /users/alois/lib/libastros.a /users/alois/lib/libaswe.a -lm

sehtest: sehtest.o 
	cc $(OP) -o sehtest sehtest.o /users/alois/lib/libastros.a /users/alois/lib/libaswe.a -lm

#sepltest: sepltest.o sweph.o swephlib.o swejpl.o swedate.o $(MOSH)
#	cc $(OP) -o sepltest sepltest.o sweph.o swephlib.o swejpl.o swedate.o $(MOSH) /users/alois/lib/libastros.a -lm

swephgen: swephgen.o sweephe4.o sweph.o swephlib.o swejpl.o swemmoon.o swemplan.o swedate.o
	cc $(OP) -o swephgen swephgen.o sweephe4.o sweph.o swephlib.o swejpl.o swemmoon.o swemplan.o swedate.o -lastros  -lm

ts:	venus
	venus -mscreen -p3 -b1.1.1996 -s1 -n100000 >s

u:
	make
	make ts

t:
	setest/setestx setest/t

swecomp: swecomp.o sweph.o swephlib.o swejpl.o swedate.o $(MOSH)
	cc $(OP) -o swecomp swecomp.o sweph.o swephlib.o swejpl.o swedate.o $(MOSH) -lastros -lm

chopt: chopt.o
	cc $(OP) -o chopt chopt.o $(LIBS) -lm

swdeltat: swdeltat.o 
	cc $(OP) -o swdeltat swdeltat.o -lastros -lm

earth: earth.o de4sub.o moshmoon.o
	cc $(OP) -o earth earth.o de4sub.o moshmoon.o -lastros -lm

d4mo: d4mo.o de4sub.o 
	cc $(OP) -o d4mo d4mo.o de4sub.o  -lastros -lm

de4test2: de4test2.o de4sub2.o
	cc $(OP) -o $@ $? -lastros -lm

###
chopt.o: sweodef.h de4sub.h swephexp.h swedll.h sweph.h swephlib.h
de4sub.o: sweodef.h de4sub.h
moshmoon.o: sweodef.h swephexp.h swedll.h sweph.h swephlib.h
printmod.o: /users/alois/lib/astrolib.h swephexp.h sweodef.h \
	swedll.h /users/alois/lib/iso12ps.h
st.o: ourdef.h swephexp.h sweodef.h swedll.h
swecl.o: swejpl.h sweodef.h swephexp.h swedll.h sweph.h swephlib.h
sweclips.o: sweodef.h swephexp.h swedll.h
swedate.o: swephexp.h sweodef.h swedll.h
sweephe4.o: swephexp.h sweodef.h swedll.h sweephe4.h \
	/users/alois/lib/sweephe4.h ourfiles.h
swehouse.o: swephexp.h sweodef.h swedll.h swephlib.h swehouse.h
sweht.o: swephexp.h sweodef.h swedll.h swehouse.h
swejpl.o: swephexp.h sweodef.h swedll.h sweph.h swejpl.h
swemini.o: swephexp.h sweodef.h swedll.h
swemmoon.o: swephexp.h sweodef.h swedll.h sweph.h swephlib.h
swemplan.o: swephexp.h sweodef.h swedll.h sweph.h swephlib.h swemptab.h
sweph.o: swejpl.h sweodef.h swephexp.h swedll.h sweph.h swephlib.h
swephgen.o: swephexp.h sweodef.h swedll.h sweephe4.h ourfiles.h \
	/users/alois/lib/sweephe4.h
swephlib.o: swephexp.h sweodef.h swedll.h sweph.h swephlib.h
swetest.o: swephexp.h sweodef.h swedll.h
swetrace.o: sweodef.h swephexp.h swedll.h
swevents.o: swephexp.h sweodef.h swedll.h 
venus.o: swephexp.h sweodef.h swedll.h sweph.h
venusold.o: ourdef.h swephexp.h sweodef.h swedll.h swephlib.h sweph.h
