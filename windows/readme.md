## Swiss Ephemeris for Windows

The folder 'windows'
https://github.com/aloistr/swisseph/windows
contains

file sweph.zip
which packages Swiss Ephemeris for Windows

- sweph/bin	compiled 32-bit and 64-bit DLLs, compiled binary sample programs
- sweph/src	the C source code used for compilation of DLLs and binaries. This is usually an older version of the source code than the one in the mail git repository.
- sweph/src/projects the project files for Visual Studio to build the binaries
- sweph/doc 	documentation files, often older than the ones in the main /doc folder
- sweph/vb	sample and support files for VB.  Note: Visual Basic files may not have been updated and
  tested, as we currently do not possess a working copy of VB at Astrodienst.

file swephzip.txt 	containing an overview of content of the sweph.zip package

folder programs/	compiled binary samples, for Windows command line,
and the DLLs swedll32.dll (32 bit) and swedll64.dll (64 bit) of version 2.10.04
(22 Nov 2026, for testing), built with clang-cl/LLVM from the sources of this
repository, with the settings of the official Visual Studio projects. They
export the same functions as the DLLs in sweph.zip, plus swe_get_denum().

folder swewin/	compiled (and outdated) Windows sample programs

folder vb/	declarations files for VB