TARGETS := setest \
           $(patsubst %.c,%.o,$(wildcard suite_*.c)) 

# Tĥe prerequisites needed for setest alone
OBJECTS := setest.o \
           generated_tests.o \
           checkpoints.o \
           constants.o \
           testdata.o \
           multivalues.o \
           reader.o \
           globals.o

all: $(TARGETS)

clean:
	rm -f $(TARGETS) ${OBJECTS} generated_tests.c

test:	all
	@cd t; ./test.sh 

# Include path to a recent swisseph directory, the include file swephexp.h is needed
#INCL = -I../sweph
INCL = -I..

CFLAGS = -std=gnu99 -fms-extensions -g -Wall -pthread -fPIC 
LIBS = ../libswe.a -lm  
LIBSX = ../libswe.a -lm  
CC=gcc	

setest: $(OBJECTS)
	$(CC) $(CFLAGS) -o setest $(OBJECTS) $(LIBS) -ldl  -lcrypto

setestx: $(OBJECTS)
	$(CC) $(CFLAGS) -o setestx $(OBJECTS) $(LIBSX) -ldl  -lcrypto

generated_tests.c: suite_*.c testsuite.m4 testsuite_end.m4 globals_suite.c
	m4 testsuite.m4 globals_suite.c suite_*.c testsuite_end.m4 >generated_tests.c

# Generate a template for a fixture file 
# (use it only the first time, or if file has been removed)
%.fix: $(TARGETS) gen_fix_templ.m4
	cp --backup=numbered $@ $@.bak 2>/dev/null || :
	m4 gen_fix_templ.m4 suite_*.c >$@

# Generate expectation file from fixture with `make mytest.exp`
%.exp:	
	./setest -g $*

# compilation rule for general cases
.o :
	$(CC) $(INCL) $(CFLAGS) -o $@ $? $(LIBS)
.c.o:
	$(CC) $(INCL) -c $(CFLAGS) $<     


### Dependencies
generated_tests.o: generated_tests.c setest.h checkpoints.h testsuite_final.h globals.h
setest.o: setest.c setest.h globals.h 
suite_2.o: suite_2.c testsuite_facade.h
suite_1.o: suite_1.c testsuite_facade.h
checkpoints.o: checkpoints.c checkpoints.h setest.h
testdata.o: testdata.c testdata.h globals.h multivalues.h setest.h
multivalues.o: testdata.h globals.h multivalues.h setest.h
reader.o: reader.h globals.h

.PHONY: all clean test
