TARGETS := setest \
           $(patsubst %.c,%.o,$(wildcard suite_*.c)) 

# Generated C files: since 2026-11-22 one per suite (see testsuite.m4), the
# helper functions of globals_suite.c, and run_tests() (testsuite_runner.m4).
# The suites run in the order of their file names.
SUITES := $(sort $(wildcard suite_*.c))
GENERATED := gen_run_tests.c gen_globals_suite.c $(patsubst suite_%.c,gen_suite_%.c,$(SUITES))
# keep the generated files (make would delete them as intermediate files)
.SECONDARY: $(GENERATED)

# Tĥe prerequisites needed for setest alone
OBJECTS := setest.o \
           $(GENERATED:.c=.o) \
           checkpoints.o \
           constants.o \
           testdata.o \
           multivalues.o \
           reader.o \
           globals.o \
           tolerances.o \
           environment.o

all: $(TARGETS)

clean:
	rm -f $(TARGETS) ${OBJECTS} $(GENERATED) generated_tests.c

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

gen_suite_%.c: suite_%.c testsuite.m4
	m4 testsuite.m4 $< >$@

gen_globals_suite.c: globals_suite.c testsuite.m4
	m4 testsuite.m4 globals_suite.c >$@

gen_run_tests.c: $(SUITES) testsuite_runner.m4 testsuite_end.m4
	m4 testsuite_runner.m4 $(SUITES) testsuite_end.m4 >$@

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
$(GENERATED:.c=.o): setest.h checkpoints.h testsuite_final.h globals.h globals_suite.h
setest.o: setest.c setest.h globals.h 
suite_2.o: suite_2.c testsuite_facade.h
suite_1.o: suite_1.c testsuite_facade.h
checkpoints.o: checkpoints.c checkpoints.h setest.h
testdata.o: testdata.c testdata.h globals.h multivalues.h setest.h
multivalues.o: testdata.h globals.h multivalues.h setest.h
reader.o: reader.h globals.h
tolerances.o: tolerances.c setest.h
environment.o: environment.c setest.h

.PHONY: all clean test
