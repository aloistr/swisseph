/************************************************************
   $Header: /home/dieter/sweph/RCS/sweodef.h,v 1.74 2008/06/16 10:07:20 dieter Exp $
   definitions and constants for all Swiss Ephemeris source files,
   only required for compiling the libraries, not for the external
   interface of the libraries.

   The definitions are a subset of Astrodienst's ourdef.h content
   and must be kept compatible. Everything not used in SwissEph
   has been deleted.

   Does auto-detection of MSDOS (TURBO_C or MS_C),  HPUNIX, Linux.
   Must be extended for more portability; there should be a way
   to detect byte order and file system type.
   
************************************************************/

/* Copyright (C) 1997 - 2008 Astrodienst AG, Switzerland.  All rights reserved.

  License conditions
  ------------------

  This file is part of Swiss Ephemeris.

  Swiss Ephemeris is distributed with NO WARRANTY OF ANY KIND.  No author
  or distributor accepts any responsibility for the consequences of using it,
  or for whether it serves any particular purpose or works at all, unless he
  or she says so in writing.  

  Swiss Ephemeris is made available by its authors under a dual licensing
  system. The software developer, who uses any part of Swiss Ephemeris
  in his or her software, must choose between one of the two license models,
  which are
  a) GNU public license version 2 or later
  b) Swiss Ephemeris Professional License

  The choice must be made before the software developer distributes software
  containing parts of Swiss Ephemeris to others, and before any public
  service using the developed software is activated.

  If the developer choses the GNU GPL software license, he or she must fulfill
  the conditions of that license, which includes the obligation to place his
  or her whole software project under the GNU GPL or a compatible license.
  See http://www.gnu.org/licenses/old-licenses/gpl-2.0.html

  If the developer choses the Swiss Ephemeris Professional license,
  he must follow the instructions as found in http://www.astro.com/swisseph/ 
  and purchase the Swiss Ephemeris Professional Edition from Astrodienst
  and sign the corresponding license contract.

  The License grants you the right to use, copy, modify and redistribute
  Swiss Ephemeris, but only under certain conditions described in the License.
  Among other things, the License requires that the copyright notices and
  this notice be preserved on all copies.

  Authors of the Swiss Ephemeris: Dieter Koch and Alois Treindl

  The authors of Swiss Ephemeris have no control or influence over any of
  the derived works, i.e. over software or services created by other
  programmers which use Swiss Ephemeris functions.

  The names of the authors or of the copyright holder (Astrodienst) must not
  be used for promoting any software, product or service which uses or contains
  the Swiss Ephemeris. This copyright notice is the ONLY place where the
  names of the authors can legally appear, except in cases where they have
  given special permission in writing.

  The trademarks 'Swiss Ephemeris' and 'Swiss Ephemeris inside' may be used
  for promoting such software, products or services.
*/

#ifndef _OURDEF_INCLUDED	/* ourdef.h is a superset of sweodef.h */
#ifndef _SWEODEF_INCLUDED /* allow multiple #includes */
#define _SWEODEF_INCLUDED
 
# define MY_TRUE 1	/* for use in other defines, before TRUE is defined */
# define MY_FALSE 0	/* for use in other defines, before TRUE is defined */

/* TLS support
 *
 * Sun Studio C/C++, IBM XL C/C++, GNU C and Intel C/C++ (Linux systems) -> __thread
 * Borland, VC++ -> __declspec(thread)
 */
#if defined( __GNUC__ )
#define TLS     __thread
#else
#define TLS     __declspec(thread)
#endif

#ifdef _WIN32		/* Microsoft VC 5.0 does not define MSDOS anymore */
# undef MSDOS
# define MSDOS MY_TRUE
#include <wtypes.h>
#include <objbase.h>
#include <wincon.h>
#include <winbase.h>
#include <io.h>
#include <windows.h>
# define sleep(x)	Sleep((x) * 1000)
#endif

#ifdef _MSC_VER
# define MS_VC
#endif

#ifdef WIN32		/* Microsoft VC 5.0 does not define MSDOS anymore */
# define MSDOS MY_TRUE
#endif

#ifdef MSDOS	/* already defined by some DOS compilers */
# undef MSDOS
# define MSDOS MY_TRUE
#endif

#ifdef __TURBOC__	/* defined by  turboc */
# ifndef MSDOS
#   define MSDOS MY_TRUE
# endif
# define TURBO_C
#endif

#ifdef __SC__	/* defined by  Symantec C */
# ifndef MSDOS
#   define MSDOS MY_TRUE
# endif
# define SYMANTEC_C
#endif

#ifdef __WATCOMC__	/* defined by  WatcomC */
# ifndef MSDOS
#   define MSDOS MY_TRUE
# endif
# define WATCOMC
#endif

#ifdef __MWERKS__	/* defined on Macintosh CodeWarrior */
# if macintosh && powerc
#  define MACOS MY_TRUE		/* let it undefined otherwise */
#  define MSDOS MY_FALSE	/* in case one above fired falsely */
# endif
#endif

#ifdef MSDOS
#  define HPUNIX MY_FALSE
#  define INTEL_BYTE_ORDER 1
#  ifndef TURBO_C
#    define MS_C	/* assume Microsoft C compiler */
#  endif
# define UNIX_FS MY_FALSE
#else
# ifdef MACOS
#  define HPUNIX MY_FALSE
#  define UNIX_FS MY_FALSE
# else
#  define MSDOS MY_FALSE
#  define HPUNIX MY_TRUE
#  ifndef _HPUX_SOURCE
#    define _HPUX_SOURCE
#  endif
#  define UNIX_FS MY_TRUE
# endif
#endif

#include <math.h>
#include <stdlib.h>
#ifndef FILE
# include <stdio.h>
#endif

#if HPUNIX
#  include <unistd.h>
#endif

/*
 * if we have 16-bit ints, we define INT_16; we will need %ld to printf an int32
 * if we have 64-bit long, we define LONG_64
 * If none is defined, we have int = long = 32 bit, and use %d to printf an int32
 */
#include <limits.h>
#if INT_MAX < 40000
# define INT_16
#else
# if LONG_MAX > INT_MAX
#   define LONG_64
# endif
#endif

#ifdef BYTE_ORDER
#ifdef LITTLE_ENDIAN
# if BYTE_ORDER == LITTLE_ENDIAN
#  define INTEL_BYTE_ORDER
# endif
#endif
#endif

#ifdef INT_16
  typedef long	int32;
  typedef unsigned long	uint32;
  typedef int	int16;
  typedef double  REAL8;  /* real with at least 64 bit precision */
  typedef long    INT4;   /* signed integer with at least 32 bit precision */
  typedef unsigned long UINT4;
                          /* unsigned integer with at least 32 bit precision */
  typedef int     AS_BOOL;
  typedef unsigned int UINT2;	/* unsigned 16 bits */
# define ABS4	labs		/* abs function for long */ 
#else
  typedef int	int32;
  typedef long long	int64;
  typedef unsigned int	uint32;
  typedef short	int16;
  typedef double  REAL8;  /* real with at least 64 bit precision */
  typedef int     INT4;   /* signed integer with at least 32 bit precision */
  typedef unsigned int UINT4; 
			/* unsigned integer with at least 32 bit precision */
  typedef int     AS_BOOL;
  typedef unsigned short UINT2;	/* unsigned 16 bits */
  # define ABS4	abs		/* abs function for long */
#endif

#if MSDOS 
# ifdef TURBO_C
#   include <alloc.h>		/* MSC needs malloc ! */
# else
#   include <malloc.h>
# endif
# define SIGALRM SIGINT
#endif

#ifndef TRUE 
#  define TRUE 1
#  define FALSE 0
#endif

#ifndef OK 
#  define OK (0)
#  define ERR (-1)
#endif

/* hack because UCHAR is already used by mingw gcc */
#ifdef __GNUC__
#ifdef _WIN32
#define UCHAR SWE_UCHAR
#endif
#endif

typedef unsigned char UCHAR;
#define UCP	(UCHAR*)
#define SCP	(char*)

# define ODEGREE_STRING "°"	/* degree as string, utf8 encoding */
 


#ifndef HUGE
#  define HUGE 1.7E+308     /* biggest value for REAL8 */
#endif
#ifndef M_PI
#  define M_PI 3.14159265358979323846
#endif
 
/* #define forward static  obsolete */

#define AS_MAXCH 256    /* used for string declarations, allowing 255 char+\0 */
 
#define DEGTORAD 0.0174532925199433
#define RADTODEG 57.2957795130823
 
typedef int32    centisec;       /* centiseconds used for angles and times */
#define CS	(centisec)	/* use for casting */
#define CSEC	centisec	/* use for typing */

#define DEG     360000  /* degree expressed in centiseconds */
#define DEG7_30 (2700000)	/* 7.5 degrees */
#define DEG15   (15 * DEG)
#define DEG24   (24 * DEG)
#define DEG30   (30 * DEG)
#define DEG60   (60 * DEG)
#define DEG90   (90 * DEG)
#define DEG120  (120 * DEG)
#define DEG150  (150 * DEG)
#define DEG180  (180 * DEG)
#define DEG270  (270 * DEG)
#define DEG360  (360 * DEG)
 
#define CSTORAD  4.84813681109536E-08 /* centisec to rad: pi / 180 /3600/100 */
#define RADTOCS  2.06264806247096E+07 /* rad to centisec 180*3600*100/pi */
 
#define CS2DEG	(1.0/360000.0)	/* centisec to degree */

/* control strings for fopen()	*/
#if UNIX_FS
#  define BFILE_R_ACCESS "r"	/* open binary file for reading */
#  define BFILE_RW_ACCESS "r+"	/* open binary file for writing and reading */
#  define BFILE_W_CREATE "w"	/* create/open binary file for write*/
#  define BFILE_A_ACCESS "a+"	/* create/open binary file for append*/
#  define FILE_R_ACCESS "r"	/* open text file for reading */
#  define FILE_RW_ACCESS "r+"	/* open text file for writing and reading */
#  define FILE_W_CREATE "w"	/* create/open text file for write*/
#  define FILE_A_ACCESS "a+"	/* create/open text file for append*/
#  define O_BINARY 0		/* for open(), not defined in Unix */
#  define OPEN_MODE 0666	/* default file creation mode */
#  define DIR_GLUE "/"		/* glue string for directory/file */
#  define PATH_SEPARATOR ";:"	/* semicolon or colon may be used */
#else
#  define BFILE_R_ACCESS "rb"	/* open binary file for reading */
#  define BFILE_RW_ACCESS "r+b"	/* open binary file for writing and reading */
#  define BFILE_W_CREATE "wb"	/* create/open binary file for write*/
#  define BFILE_A_ACCESS "a+b"	/* create/open binary file for append*/
#  define PATH_SEPARATOR ";"	/* semicolon as PATH separator */
#  define OPEN_MODE 0666	/* default file creation mode */
# ifdef MACOS
#  define FILE_R_ACCESS "r"	/* open text file for reading */
#  define FILE_RW_ACCESS "r+"	/* open text file for writing and reading */
#  define FILE_W_CREATE "w"	/* create/open text file for write*/
#  define FILE_A_ACCESS "a+"	/* create/open text file for append*/
#  define DIR_GLUE ":"		/* glue string for directory/file */
# else
#  define FILE_R_ACCESS "rt"	/* open text file for reading */
#  define FILE_RW_ACCESS "r+t"	/* open text file for writing and reading */
#  define FILE_W_CREATE "wt"	/* create/open text file for write*/
#  define FILE_A_ACCESS "a+t"	/* create/open text file for append*/
/* attention, all backslashes for msdos directry names must be written as \\,
   because it is the C escape character */
#  define DIR_GLUE "\\"		/* glue string for directory/file */
# endif
#endif

#include <string.h>
#include <ctype.h>

#endif /* _SWEODEF_INCLUDED */
#endif /* _OURDEF_INCLUDED */
