/************************************************************
  $Header: swephexp.h,v 1.27 98/11/30 22:13:27 dieter Exp $
  SWISSEPH: exported definitions and constants 

  This file represents the standard application interface (API)
  to the Swiss Ephemeris.

  A C programmer needs only to include this file, and link his code
  with the SwissEph library.

  The function calls are documented in the Programmer's documentation,
  which is online in HTML format.

  Structure of this file:
    Public API definitions
    Internal developer's definitions
    Public API functions.

  Authors: Dieter Koch and Alois Treindl, Astrodienst Zürich

************************************************************/
/* Copyright (C) 1997, 1998 Astrodienst AG, Switzerland.  All rights reserved.
  
  This file is part of Swiss Ephemeris Free Edition.
  
  Swiss Ephemeris is distributed with NO WARRANTY OF ANY KIND.  No author
  or distributor accepts any responsibility for the consequences of using it,
  or for whether it serves any particular purpose or works at all, unless he
  or she says so in writing.  Refer to the Swiss Ephemeris Public License
  ("SEPL" or the "License") for full details.
  
  Every copy of Swiss Ephemeris must include a copy of the License,
  normally in a plain ASCII text file named LICENSE.  The License grants you
  the right to copy, modify and redistribute Swiss Ephemeris, but only
  under certain conditions described in the License.  Among other things, the
  License requires that the copyright notices and this notice be preserved on
  all copies.

  For uses of the Swiss Ephemeris which do not fall under the definitions
  laid down in the Public License, the Swiss Ephemeris Professional Edition
  must be purchased by the developer before he/she distributes any of his
  software or makes available any product or service built upon the use of
  the Swiss Ephemeris.

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


#ifndef _SWEPHEXP_INCLUDED      /* allow multiple #includes of swephexp.h */
#define _SWEPHEXP_INCLUDED

#include "sweodef.h"

/***********************************************************
 * definitions for use also by non-C programmers
 ***********************************************************/

/* values for gregflag in swe_julday() and swe_revjul() */
# define SE_JUL_CAL	0
# define SE_GREG_CAL	1

/*
 * planet numbers for the ipl parameter in swe_calc()
 */
#define SE_ECL_NUT      -1      

#define SE_SUN          0       
#define SE_MOON         1       
#define SE_MERCURY      2       
#define SE_VENUS        3       
#define SE_MARS         4       
#define SE_JUPITER      5       
#define SE_SATURN       6       
#define SE_URANUS       7       
#define SE_NEPTUNE      8       
#define SE_PLUTO        9       
#define SE_MEAN_NODE    10      
#define SE_TRUE_NODE    11
#define SE_MEAN_APOG    12      
#define SE_OSCU_APOG    13    
#define SE_EARTH        14      
#define SE_CHIRON       15      
#define SE_PHOLUS       16      
#define SE_CERES        17      
#define SE_PALLAS       18      
#define SE_JUNO         19      
#define SE_VESTA        20      

#define SE_NPLANETS     21      

#define SE_AST_OFFSET   10000

#define SE_FICT_OFFSET  	40
#define SE_NFICT_ELEM           15

#define SE_NALL_NAT_POINTS      (SE_NPLANETS + SE_NFICT_ELEM)

/* Hamburger or Uranian "planets" */
#define SE_CUPIDO       	40
#define SE_HADES        	41
#define SE_ZEUS         	42
#define SE_KRONOS       	43
#define SE_APOLLON      	44
#define SE_ADMETOS      	45
#define SE_VULKANUS     	46
#define SE_POSEIDON     	47
/* other ficticious bodies */
#define SE_ISIS         	48
#define SE_NIBIRU       	49
#define SE_HARRINGTON           50
#define SE_NEPTUNE_LEVERRIER    51
#define SE_NEPTUNE_ADAMS        52
#define SE_PLUTO_LOWELL         53
#define SE_PLUTO_PICKERING      54

#define SE_FIXSTAR      -10

/*
 * flag bits for parameter iflag in function swe_calc()
 * The flag bits are defined in such a way that iflag = 0 delivers what one
 * usually wants:
 *    - the default ephemeris (SWISS EPHEMERIS) is used,
 *    - apparent geocentric positions referring to the true equinox of date
 *      are returned.
 * If not only coordinates, but also speed values are required, use 
 * flag = SEFLG_SPEED.
 *
 * The 'L' behind the number indicates that 32-bit integers (Long) are used.
 */
#define SEFLG_JPLEPH    1L       /* use JPL ephemeris */
#define SEFLG_SWIEPH    2L       /* use SWISSEPH ephemeris */
#define SEFLG_MOSEPH    4L       /* use Moshier ephemeris */

#define SEFLG_HELCTR	8L      /* return heliocentric position */
#define SEFLG_TRUEPOS	16L     /* return true positions, not apparent */
#define SEFLG_J2000	32L     /* no precession, i.e. give J2000 equinox */
#define SEFLG_NONUT	64L     /* no nutation, i.e. mean equinox of date */
#define SEFLG_SPEED3	128L     /* speed from 3 positions (do not use it,
                                  SEFLG_SPEED is faster and more precise.) */
#define SEFLG_SPEED	256L     /* high precision speed  */
#define SEFLG_NOGDEFL	512L     /* turn off gravitational deflection */
#define SEFLG_NOABERR	1024L    /* turn off 'annual' aberration of light */
#define SEFLG_EQUATORIAL (2*1024L)    /* equatorial positions are wanted */
#define SEFLG_XYZ	(4*1024L)    /* cartesian, not polar, coordinates */
#define SEFLG_RADIANS	(8*1024L)    /* coordinates in radians, not degrees */
#define SEFLG_BARYCTR	(16*1024L)   /* barycentric positions */
#define SEFLG_TOPOCTR	(32*1024L)   /* topocentric positions */

/* default ephemeris used when no ephemeris flagbit is set */
#define SEFLG_DEFAULTEPH SEFLG_SWIEPH

#define SE_MAX_STNAME		20	/* maximum size of fixstar name;
                                         * the parameter star in swe_fixstar
					 * must allow twice this space for
				         * the returned star name.
					 */


/*
 * only used for experimenting with various JPL ephemeris files
 * which are available at Astrodienst's internal network
 */
#define SE_DE_NUMBER    406
#define SE_FNAME_DE200  "de200.eph"
#define SE_FNAME_DE403  "de403.eph"
#define SE_FNAME_DE404  "de404.eph"
#define SE_FNAME_DE405  "de405.eph"
#define SE_FNAME_DE406  "de406.eph"
#define SE_FNAME_DFT    SE_FNAME_DE406
#define SE_STARFILE     "fixstars.cat"

/*
 * ephemeris path
 * this defines where ephemeris files are expected if the function
 * swe_set_ephe_path() is not called by the application.
 * Normally, every application should make this call to define its
 * own place for the ephemeris files.
 */

#if MSDOS
#ifdef PAIR_SWEPH
#  define SE_EPHE_PATH    "\\pair\\ephe\\"
#else
#  define SE_EPHE_PATH    "\\sweph\\ephe\\"
#endif
#else
#  define SE_EPHE_PATH    "/users/ephe/:/users/ephe2/"
			/* At Astrodienst, we maintain two ephemeris areas for
			   the thousands of asteroid files: 
			   the short files in /users/ephe/ast*,
			   the long file in /users/ephe2/ast*. */
#endif

/*
 * by compiling with -DPAIR_SWEPH in he compiler options it
 * is possible to create a more compact version of SwissEph which
 * contains no code for the JPL ephemeris file and for the builtin
 * Moshier ephemeris.
 * This is quite useful for MSDOS real mode applications which need to
 * run within 640 kb.
 * The option is called PAIR_SWEPH because it was introduced for
 * Astrodienst's partner software PAIR.
 */
#ifdef PAIR_SWEPH	
# define NO_JPL
# define NO_MOSHIER
#endif

/**************************************************************
 * here follow some ugly definitions which are only required
 * if SwissEphemeris is compiled on Windows, either to use a DLL
 * or to create a DLL.
 * Unix users can savely ignore his section
 * and skip to the export function decarations below.
 ************************************************************/
#if defined(MAKE_DLL) || defined(USE_DLL) || defined(_WINDOWS)
#  include <windows.h>
#endif

#ifdef USE_DLL
#  include "swedll.h"
#endif

#if defined(DOS32) || !MSDOS || defined(WIN32)
			/* use compiler switch to define DOS32 */
#  ifndef FAR 
#    define FAR
#  endif
#  define MALLOC malloc  
#  define CALLOC calloc  
#  define FREE free  
#else
#  ifndef FAR
#    define FAR far
#  endif
#  ifdef __BORLANDC__
#    include <alloc.h>
#    define MALLOC farmalloc  
#    define CALLOC farcalloc  
#    define FREE farfree  
#  else
#    define MALLOC _fmalloc
#    define CALLOC _fcalloc
#    define FREE _ffree
#  endif
#endif

/* DLL defines */
#ifdef MAKE_DLL
  #if defined (PASCAL)
    #define PASCAL_CONV PASCAL 
  #else
    #define PASCAL_CONV 
  #endif
  #ifdef MAKE_DLL16 /* 16bit DLL */
    /* We compiled the 16bit DLL for Windows 3.x using Borland C/C++ Ver:3.x
       and the -WD or -WDE compiler switch. */
    #define EXP16 __export 
    #define EXP32 
  #else /* 32bit DLL */
    /* To export symbols in the new DLL model of Win32, Microsoft 
       recommends the following approach */ 
    #define EXP16 
    #define EXP32  __declspec( dllexport )
  #endif
#else 
  #define PASCAL_CONV 
  #define EXP16 
  #define EXP32 
#endif  

#ifndef _SWEDLL_H

/***********************************************************
 * exported functions
 ***********************************************************/

#define ext_def(x)	extern EXP32 x FAR PASCAL_CONV EXP16
			/* ext_def(x) evaluates to x on Unix */

/**************************** 
 * exports from sweph.c 
 ****************************/

/* planets, moon, nodes etc. */
ext_def( long ) swe_calc(
        double tjd, int ipl, long iflag, 
        double *xx,
        char *serr);

/* fixed stars */
ext_def( long ) swe_fixstar(
        char *star, double tjd, long iflag, 
        double *xx,
        char *serr);

/* close Swiss Ephemeris */
ext_def( void ) swe_close(void);

/* set directory path of ephemeris files */
ext_def( void ) swe_set_ephe_path(char *path);

/* set file name of JPL file */
ext_def( void ) swe_set_jpl_file(char *fname);

/* get planet name */
ext_def( char *) swe_get_planet_name(int ipl, char *spname);

/* set geographic position of observer */
ext_def (void) swe_set_topo(double geolon, double geolat, double geoalt);

/**************************** 
 * exports from swedate.c 
 ****************************/

ext_def( int ) swe_date_conversion(
        int y , int m , int d ,         /* year, month, day */
        double utime,   /* universal time in hours (decimal) */
        char c,         /* calendar g[regorian]|j[ulian] */
        double *tjd);

ext_def( double ) swe_julday(
        int year, int month, int day, double hour, 
        int gregflag);

ext_def( void ) swe_revjul (
        double jd, 
        int gregflag,
        int *jyear, int *jmon, int *jday, double *jut);

/**************************** 
 * exports from swehouse.c 
 ****************************/

ext_def( int ) swe_houses(
        double tjd_ut, double geolat, double geolon, int hsys, 
	double *cusps, double *ascmc);

ext_def( int ) swe_houses_armc(
        double armc, double geolat, double eps, int hsys, 
	double *cusps, double *ascmc);

ext_def(double) swe_house_pos(
	double armc, double geolat, double eps, int hsys, double *xpin, char *serr);

/**************************** 
 * exports from swephlib.c 
 ****************************/

/* delta t */
ext_def( double ) swe_deltat(double tjd);

/* equation of time */
ext_def( int ) swe_time_equ(double tjd, double *te, char *serr);

/* sidereal time */
ext_def( double ) swe_sidtime0(double tjd_ut, double eps, double nut);
ext_def( double ) swe_sidtime(double tjd_ut);

/* coordinate transformation polar -> polar */
ext_def( void ) swe_cotrans(double *xpo, double *xpn, double eps);
ext_def( void ) swe_cotrans_sp(double *xpo, double *xpn, double eps);

/* tidal acceleration to be used in swe_deltat() */
ext_def( double ) swe_get_tid_acc(void);
ext_def( void ) swe_set_tid_acc(double t_acc);

ext_def( double ) swe_degnorm(double x);

/******************************************************* 
 * other functions from swephlib.c;
 * they are not needed for Swiss Ephemeris,
 * but may be useful to former Placalc users.
 ********************************************************/

/* normalize argument into interval [0..DEG360] */
ext_def( centisec ) swe_csnorm(centisec p);

/* distance in centisecs p1 - p2 normalized to [0..360[ */
ext_def( centisec ) swe_difcsn (centisec p1, centisec p2);

ext_def( double ) swe_difdegn (double p1, double p2);

/* distance in centisecs p1 - p2 normalized to [-180..180[ */
ext_def( centisec ) swe_difcs2n(centisec p1, centisec p2);

ext_def( double ) swe_difdeg2n(double p1, double p2);

/* round second, but at 29.5959 always down */
ext_def( centisec ) swe_csroundsec(centisec x);

/* double to long with rounding, no overflow check */
ext_def( long ) swe_d2l(double x);

/* monday = 0, ... sunday = 6 */
ext_def( int ) swe_day_of_week(double jd);

ext_def( char *) swe_cs2timestr(CSEC t, int sep, AS_BOOL suppressZero, char *a);

ext_def( char *) swe_cs2lonlatstr(CSEC t, char pchar, char mchar, char *s);

ext_def( char *) swe_cs2degstr(CSEC t, char *a);

#endif  /* #ifndef _SWEDLL_H */

#endif  /* #ifndef _SWEPHEXP_INCLUDED */
