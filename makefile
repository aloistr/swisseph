#CFLAGS = -Aa -I/users/alois/lib -g +z 
#CFLAGS = -O2 -Wall -I/users/alois/lib
# for building perl modules parameter -fPIC is needed
#CFLAGS = -g -O2 -Wall -fPIC  
#CFLAGS = -g -O2    -fPIC -Wall 
CFLAGS = -O2  -fPIC -Wall 
CFLAGS = -g  -fPIC -Wall 
#CFLAGS = -g  -fPIC -Wall 
CC = cc
# Linux flags:
# -pedantic     check all Ansi stuff, lots of messages
# -Wall         check about as much as lint    
# -g is debug mode
# -O2 is optimizing
OP=$(CFLAGS)  -I/users/alois/lib -I/usr/include/cairo -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include -I/usr/include/pango-1.0  -I/usr/lib64/glib-2.0/include
SWEVERSION=2.05c
.SUFFIXES: .o  .c  .ts

# compilation rule for general cases
.o :  
	$(CC) $(OP) -o $@ $?  $(SWEOBJ) -lcheck -lm -ldl
.c.o:
	$(CC) -c $(OP) $<

# compilation rule for test-scripts .ts
.ts.c:
	checkmk $< > $*.c

OBJECTS = astrolib.o asyslib.o atlclps.o atls.o awd.o \
	combin.o composit.o csec.o ctype256.o cutstr.o cutstr2.o \
	d2l.o degstr.o\
	ecl2equ0.o ecl2equ.o \
	filemv.o \
	get_lpb.o gettext.o \
	interpod.o interpol.o \
	juldays.o kdvclps.o langdate.o \
	makepath.o mapcotra.o msgchild.o \
	ourtime.o outdeg.o outll.o outmerid.o outtime.o outnord.o \
	packbli.o parsecmd.o plotmod.o printmod.o \
	readbmap.o random.o revjuls.o rmuscore.o \
	sdegstr.o solcross.o splitcs.o sread.o stristr.o \
	strmap.o \
	ttbs.o  ttbs2.o ttbs2localtime.o timestr.o titsub.o upcase1.o \
	utf8.o writelog.o

# for libastros, the placalc-less version of libastro
SOBJECTS = astrolib.o asyslib.o atlclps.o atls.o awd.o \
	combin.o composit.o  csec.o ctype256.o cutstr.o cutstr2.o \
	d2l.o degstr.o\
	ecl2equ0.o ecl2equ.o \
	filemv.o \
	get_lpb.o gettext.o \
	interpod.o interpol.o \
	juldays.o \
	kdvclps.o langdate.o makepath.o mapcotra.o msgchild.o \
	ourtime.o outdeg.o outll.o outmerid.o outtime.o outnord.o \
	packbli.o parsecmd.o plotmod.o   printmod.o \
	readbmap.o random.o revjuls.o rmuscore.o \
	sdegstr.o solcross.o splitcs.o sread.o stristr.o \
	strmap.o \
	ttbs.o ttbs2.o ttbs2localtime.o  timestr.o titsub.o upcase1.o \
	utf8.o writelog.o \
	sweephe4.o 

# for libswe, the standard version of SwissEph
SWESRC = swecl.c sweph.c swephlib.c swejpl.c \
	swemmoon.c swemplan.c swedate.c swehouse.c swehel.c \
	swedate.h swehouse.h swejpl.h swemptab.h sweodef.h \
	swenut2000a.h swephexp.h sweph.h swephlib.h
SWEOBJ = swecl.o sweph.o swephlib.o swejpl.o \
	swemmoon.o swemplan.o swedate.o swehouse.o swehel.o 
ASWEOBJ = $(SWEOBJ)  sweephe4.o 

LIBSWE = -lswe

all:	libastro libsweph libastros

clean:
	rm *.o lib*.a

test:	swe_check
	swe_check

wwx:
	rsync -avL *.c *.h wwx:/users/alois/lib

wwu:
	rsync -avL *.c *.h wwu:/users/alois/lib

# build libraries for new SE release
libswe: swe_src libswe_so libastros

MYDIR = $(shell pwd)
swe_src: 
	cd /users/dieter/sweph/ && rsync -avq $(SWESRC) $(MYDIR)
	chmod ug+w $(SWESRC)

libswe_so: $(SWEOBJ) 
	cc -shared -Wl,-soname,libswe.so.1 -o libswe.so.$(SWEVERSION) $(SWEOBJ) -lm 
	cp -p libswe.so.$(SWEVERSION) /usr/local/lib64/swe
	ln -sf /usr/local/lib64/swe/libswe.so.$(SWEVERSION) /usr/local/lib64/swe/libswe.so.1
	ln -sf /usr/local/lib64/swe/libswe.so.$(SWEVERSION) /usr/local/lib64/swe/libswe.so
	echo "/usr/local/lib64/swe/libswe.so.$(SWEVERSION)" >> /users/dieter/sweph/perl_swisseph/MANIFEST

# release to wwx
LOCAL_LIBDIR = /usr/local/lib64/swe
libswer: 
	rsync -av --no-links $(LOCAL_LIB_DIR)/libswe* wwx:$(LOCAL_LIB_DIR)
	rsync -av $(LOCAL_LIB_DIR)/libswe* wwx:$(LOCAL_LIB_DIR)

# statically linked library libastro.c
libastro: $(OBJECTS) astrolib.h
	ar r libastro.a $(OBJECTS) 
	cp libastro.a /usr/lib64
#
# statically linked library libsweph.a
libsweph: $(SWEOBJ) 
	ar r libsweph.a $(SWEOBJ)
	# cp libsweph.a /usr/lib64

# libastros is obsolete: we use either statically libsweph or dynamically libswe
# together with libastro
libastros: $(SOBJECTS) $(ASWEOBJ) astrolib.h
	ar r libastros.a $(SOBJECTS) $(ASWEOBJ)
	cp -p libastros.a /usr/lib64

astrolib.tar: 
	tar -chvf astrolib.tar *.h *.c

# needs extrawurst because of include mysql.h
awd.o: awd.c
	$(CC) $(OP) -c awd.c

# swetest using dynamic library
swetestd: swetest.o 
	cc $(OP) -o swetestd swetest.o -lswe -lastro -lm -ldl

# swetest statically linked
swetest: swetest.o $(SWEOBJ)
	cc $(OP) -o swetest swetest.o $(SWEOBJ) -lastro -lm -ldl

# solcross statically linked
solcross: solcross.o $(SWEOBJ)
	$(CC) -c $(OP) -DSOLMAIN solcross.c
	cc $(OP) -o solcross solcross.o $(SWEOBJ) -lastros -lm -ldl


###
asyslib.o: ourdef.h astrolib.h asyslib.h printmod.h ourfiles.h plotmod.h
atlclps.o: ourdef.h astrolib.h ctype256.h
atls.o: ourdef.h ourfiles.h atls.h astrolib.h
combin.o: ourdef.h astrolib.h atls.h composit.h swepcalc.h swephexp.h \
	sweodef.h kdvexpor.h
composit.o: ourdef.h astrolib.h atls.h composit.h swepcalc.h swephexp.h \
	sweodef.h kdvexpor.h swephlib.h
csec.o: ourdef.h astrolib.h 
ctype256.o: ourdef.h astrolib.h ctype256.h
cutstr.o: ourdef.h astrolib.h
cutstr2.o: ourdef.h
d2l.o: ourdef.h
dateconv.o: ourdef.h astrolib.h
degstr.o: ourdef.h astrolib.h
ecl2equ.o: ourdef.h astrolib.h
ecl2equ0.o: ourdef.h astrolib.h
ephe.o: ephe.h ourdef.h astrolib.h ourfiles.h
filemv.o: ourdef.h ourfiles.h astrolib.h
formio.o: ourdef.h astrolib.h formio.h /usr/include/curses.h \
	ctype256.h
gdcache.o: gdcache.h ourdef.h
get_lpb.o: ourdef.h ourfiles.h
gettext.o: ourdef.h ourfiles.h astrolib.h
hp2iso1.o: ourdef.h astrolib.h
interpod.o: ourdef.h astrolib.h
interpol.o: ourdef.h astrolib.h
jobcsub.o: ourdef.h ourfiles.h astrolib.h jobcon.h atls.h \
	swephexp.h sweodef.h 
julday.o: ourdef.h astrolib.h
juldays.o: ourdef.h astrolib.h
kdvclps.o: ourdef.h astrolib.h ctype256.h
makepath.o: ourdef.h ourfiles.h astrolib.h
mapcotra.o: mapcotra.h sweodef.h swephexp.h sweph.h
msgchild.o: ourdef.h ourfiles.h astrolib.h atls.h kdvcom.h kdvexpor.h \
	msgchild.h
ourtime.o: ourdef.h astrolib.h
outdeg.o: ourdef.h astrolib.h
outll.o: ourdef.h
outmerid.o: ourdef.h
outnord.o: ourdef.h astrolib.h
outtime.o: ourdef.h astrolib.h
parsecmd.o: ourdef.h astrolib.h
plotmod.o: ourdef.h ourfiles.h astrolib.h plotmod.h printmod.h 
printmod.o: ourdef.h printmod.h ourfiles.h astrolib.h swepcalc.h \
	swephexp.h sweodef.h printtab.c
random.o: ourdef.h astrolib.h
readbmap.o: ourdef.h astrolib.h
revjul.o: ourdef.h astrolib.h
revjuls.o: ourdef.h astrolib.h
rmuscore.o: ourdef.h astrolib.h
sdegstr.o: ourdef.h
solcross.o: ourdef.h astrolib.h swepcalc.h swephexp.h sweodef.h 
splitcs.o: ourdef.h astrolib.h
sread.o: ourdef.h astrolib.h
stristr.o: ourdef.h astrolib.h
swecl.o: swejpl.h sweodef.h swephexp.h sweph.h swephlib.h
sweclips.o: sweodef.h swephexp.h 
swedate.o: swephexp.h sweodef.h 
sweephe4.o: swephexp.h sweodef.h sweephe4.h swepcalc.h astrolib.h \
	ourfiles.h
swehouse.o: swephexp.h sweodef.h sweph.h swephlib.h swehouse.h
swejpl.o: swephexp.h sweodef.h sweph.h swejpl.h
swemini.o: swephexp.h sweodef.h 
swemmoon.o: swephexp.h sweodef.h sweph.h swephlib.h
swemplan.o: swephexp.h sweodef.h sweph.h swephlib.h 
swepcalc.o: swepcalc.h swephexp.h sweodef.h 
swepdate.o: swepcalc.h swephexp.h sweodef.h 
sweph.o: swejpl.h sweodef.h swephexp.h sweph.h swephlib.h
swephlib.o: swephexp.h sweodef.h sweph.h swephlib.h
swetest.o: swephexp.h sweodef.h 
timestr.o: ourdef.h astrolib.h
titsub.o: ourdef.h printmod.h ourfiles.h astrolib.h titsub.h
transeph.o: ourdef.h transeph.h
ttbs.o: ourdef.h ourfiles.h astrolib.h atls.h ttbs.h
ttbs2.o: ourdef.h ourfiles.h astrolib.h atls.h ttbs.h ttbs2.h
upcase1.o: ourdef.h astrolib.h
writelog.o: ourdef.h astrolib.h
