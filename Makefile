################################################################################
# Swiss Ephemeris Multi-platform Build Makefile
#
# This Makefile builds the Swiss Ephemeris project on both Linux and macOS.
#
# Features:
#  - Automatically detects the operating system using `uname`
#  - Sets appropriate compiler flags, library linking options, and shared
#    library creation flags for Linux and macOS.
#  - Builds dynamically linked executables (swetest, swevents, swemini) on both
#    platforms.
#  - Builds a fully statically linked executable (swetests) on Linux only.
#  - Creates both static (libswe.a) and shared libraries (libswe.so on Linux,
#    libswe.dylib on macOS) from the source object files.
#
# Targets:
#    all         - Build all executables (swetest, swevents, swemini, and swetests on Linux)
#    swetest     - Build the swetest executable using libswe.a (dynamic linking)
#    swetests    - Build a fully statically linked swetest (Linux only)
#    swevents    - Build the swevents executable
#    swemini     - Build the swemini executable using libswe.a (dynamic linking)
#    libswe.a    - Create the static library archive from object files
#    libswe.$(DYLIB_EXT)
#                - Create the shared library (extension depends on OS)
#    test        - Run tests from the setest directory (requires a Makefile in setest)
#    clean       - Remove all generated files and clean the setest directory
#
# To customize, modify the CFLAGS, LIBS, or any other variables as needed.
################################################################################

# Detect OS type via uname
OS := $(shell uname)

ifeq ($(OS), Darwin)
  # macOS settings
  CC               = cc
  CFLAGS           = -g -Wall -fPIC
  LIBS             = -lm
  DYLIB_FLAG       = -dynamiclib
  DYLIB_EXT        = dylib
  STATIC_SUPPORTED = false
else
  # Assume Linux settings
  CC               = cc
  CFLAGS           = -g -Wall -fPIC
  LIBS             = -lm -ldl
  DYLIB_FLAG       = -shared
  DYLIB_EXT        = so
  STATIC_SUPPORTED = true
  STATIC_LINK_FLAGS= -Wl,-Bstatic
  DYNAMIC_LINK_FLAGS= -Wl,-Bdynamic
endif

# Object files for the Swiss Ephemeris library
SWEOBJ = swedate.o swehouse.o swejpl.o swemmoon.o swemplan.o sweph.o \
         swephlib.o swecl.o swehel.o

# Define overall targets. On Linux, include the static swetests target.
ifeq ($(STATIC_SUPPORTED),true)
ALL_TARGETS = swetest swetests swevents swemini
else
ALL_TARGETS = swetest swevents swemini
endif

all: $(ALL_TARGETS)

# Compile .c files to .o files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Build swetest: link swetest.o with the static library libswe.a
swetest: swetest.o libswe.a
	$(CC) $(CFLAGS) -o swetest swetest.o -L. -lswe $(LIBS)

# Build swetests: fully statically linked version (Linux only)
ifeq ($(STATIC_SUPPORTED),true)
swetests: swetest.o $(SWEOBJ)
	$(CC) $(CFLAGS) $(STATIC_LINK_FLAGS) -o swetests swetest.o $(SWEOBJ) $(DYNAMIC_LINK_FLAGS) $(LIBS)
endif

# Build swevents
swevents: swevents.o $(SWEOBJ)
	$(CC) $(CFLAGS) -o swevents swevents.o $(SWEOBJ) $(LIBS)

# Build swemini
swemini: swemini.o libswe.a
	$(CC) $(CFLAGS) -o swemini swemini.o -L. -lswe $(LIBS)

# Create a static library from the object files
libswe.a: $(SWEOBJ)
	ar r libswe.a $(SWEOBJ)

# Create a shared library
libswe.$(DYLIB_EXT): $(SWEOBJ)
	$(CC) $(DYLIB_FLAG) -o libswe.$(DYLIB_EXT) $(SWEOBJ)

# Test targets (requires a "setest" subdirectory with its own Makefile)
test:
	cd setest && make && ./setest t

test.exp:
	cd setest && make && ./setest -g t

# Clean up build artifacts
clean:
	rm -f *.o swetest libswe.* swetests swevents swemini
	cd setest && make clean

# Dependency rules
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
