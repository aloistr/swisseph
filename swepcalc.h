/************************************************************
   $Header: /home/dieter/sweph/RCS/swepcalc.h,v 1.74 2008/06/16 10:07:20 dieter Exp $
   Definitions and constants for Placalc interface to SwissEph

   This should only be used for porting older Placalc applications.
   All new applications should directly use the Swiss Ephemeris API.

   A programmer using the Placalc API needs only to include this file
   in his code, and link his application with the SwissEph library.

   There is no documentation, as it is only intended for porting old
   existing Placalc applications.

   The Placalc API is not supported in the Windows DLL versions of
   Swiss Ephemeris.
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

#ifndef _SWEPCALC_INCLUDED
#define _SWEPCALC_INCLUDED

#include "swephexp.h"

#define degtocs(x)	(d2l((x) * DEG))
#define cstodeg(x)	(double)((x) * CS2DEG)

/*
 * Some functions which came in the source files csec.c and d2l.c
 * are by default not included in the compatibility interface because
 * they have an identical counterpart in the SwissEph library.
 * It is very simple to adapt your code to it.
 * If you want to adapt your source code, turn the next TRUE into FALSE
 */
#if TRUE	/* replace by if FALSE to deactivate macros */
#  define d2l(x)	swe_d2l(x)
#  define difcsn(x,y)	swe_difcsn(x,y)
#  define difcs2n(x,y)	swe_difcs2n(x,y)
#  define difdegn(x,y)	swe_difdegn(x,y)
#  define difdeg2n(x,y)	swe_difdeg2n(x,y)
#  define csnorm(x)	swe_csnorm(x)
#  define degnorm(x)	swe_degnorm(x)
#  define roundsec(x)	swe_csroundsec(x)
#endif

/*************************************************************
 Exported functions, originally from placalc.c

 In all functions the variable jd_ad indicates the use of
 Astrodienst relative julian days, and jd the use of absolute
 julian days.
*************************************************************/
extern int nacalc(double jd_ad, centisec *plon, centisec *pspe);
extern int calcserv(int id, double t, int flag, int plalist, char *so);
extern void helup(double jd_ad);
extern void togeo(double le, double re, double l, double r, double z, double *alg, double *arg);
extern int  calc(int p,
		 double jd_ad,
		 int flag, 
		 double *alng,
		 double *arad,
		 double *alat,
		 double *alngspeed);
extern int rel_geo(int p, double rau);
extern int hel( int	p,	/* planet index as defined by placalc.h */
		double	jd_ad,	/* relative juliand date, ephemeris time */
				/* Now come 6 pointers to return values. */
		double	*al,	/* longitude in degrees */
		double   *ar,	/* radius in AU */
		double   *az,	/* distance from ecliptic in AU */
		double   *alp, 	/* speed in longitude, degrees per day */
		double   *arp,	/* speed in radius, AU per day */
		double   *azp);   /* speed in z, AU per day */
extern int  moon(double *al, double *ar, double *az);
extern double fraction(double t);
extern double sidtime(double jd_ad, double ecl, double nuta);
extern double smod8360(double x);
extern double mod8360(double x);
extern double diff8360(double x, double y);
extern double test_near_zero(double x);
extern double deltat(double jd_ad);
extern void to_mean_ekl (double jd, double xyz[], double lrz[]);
extern void placalc_close_files();
extern int fixstar(char *star, double jd, double *lon, double *lat);
extern char *placalc_get_errtext();
extern char *placalc_set_ephepath(char *new_path);	/* sets ephepath;
				if called with NULL, returns current path */

/*
 * because deltat() required a relative Julian date due to historical reasons,
 * we define a function deltatjd() with absolute Juliand date argument.
 */
# define deltatjd(x) deltat((x) - JUL_OFFSET)

/*
 * get the planet index for an AFL letter
 * returns -1 if the letter does not correspond to a planet.
 */
extern int afl2planet(int afl);
/*
 * get the AFL letter for a planet
 * returns -1 if planet has no letter.
 */
extern int planet2afl(int p);
extern char *planet2abbr2(int planet);
extern char *planet2abbr3(int planet);

/*************************************************************
	exported variables
	(these cannot be used by DLL clients)
*************************************************************/

  
/*************************************************************
	definitions
*************************************************************/

/*
 * planet index numbers, used to identify a planet in calc() and
 * other related functions.
 */
#define CALC_ONLY_ECL_NUT -1	/* pseudo planet index for calls to calc */
#define SUN	0		/* used synonymously for earth too */
#define EARTH	0
#define MOON	1
#define MERCURY 2
#define VENUS	3
#define MARS	4
#define JUPITER	5
#define SATURN	6
#define URANUS	7
#define NEPTUNE 8
#define PLUTO	9
#define LASTPLANET PLUTO
#define MEAN_NODE  10
#define TRUE_NODE  11
#define CHIRON	   12
#define LILITH	   13	
#define CALC_N	   14	/* number of planets in placalc module */

#define CERES     14
#define PALLAS    15
#define JUNO      16
#define VESTA     17

#define MAXPL_NACALC	(LILITH)	/* nacalc computes SUN..LILITH */
# define PROG_PLANET_OFFSET  50      /* progressed sun */

/*
 * houses and axes get also a 'planet' index number, but they
 * are not used by placalc itself
 * between chiron and AC we leave 6 places unused for some other celestial
 * bodies or chart factors.
 * Axes and houses cannot be computed with calls to calc(); they must
 * be computed with the housasp module functions.
 */
# define AC	   19
# define ASC	   19
# define MC	   20
# define CALC_N_MC  21	/* number of normal natal factors */

# define FIRST_HSNR 21
# define LAST_HSNR 32
# define NO_OF_HOUSES 12
#define MAX_PL_INDEX 32
/*
 * in a bitlist flag each planet is represented by a bit;
 * all 14 defined planets can be called at once with
 */
#define CALC_ALL_PLANET_BITS  ((1 << CALC_N) - 1)	/* bits 0..13 set */

/*
 * AFL: Astrological factor letters for use in selections strings.
 * Each factor (planet, house cusp etc) has a typical letter which
 * can be combined in a selection string for specifying a certain
 * sequence of factors for a table or other kind of display.
 * The function afl2planet() can be used to translate the AFL letters
 * into planet indices.
 * The function planet2afl translates a planext index into the AFL letter.
 */

# define AFL_SUN	'0'
# define AFL_MON	'1'
# define AFL_MER	'2'
# define AFL_VEN	'3'
# define AFL_MAR	'4'
# define AFL_JUP	'5'
# define AFL_SAT	'6'
# define AFL_URA	'7'
# define AFL_NEP	'8'
# define AFL_PLU	'9'
# define AFL_CHI	'c'
# define AFL_LIL	'i'	/* mean Lilith: direction of lunar aphel */
# define AFL_AC		'A'
# define AFL_MC		'M'
# define AFL_TNODE	'N'	/* TRUE_NODE */
# define AFL_MNODE	'n'	/* MEAN_NODE */
# define AFL_CER	'C'	
# define AFL_PAL	'P'
# define AFL_JUN	'J'
# define AFL_VES	'V'

/*
 * other AFL definitions not recognized by afl2planet()
 */
# define AFL_SIDT	's'	/* sidereal time */
# define AFL_WDAY	'd'	/* day of week column */
# define AFL_HOUSE	'H'	/* any house cusp */

# define apl2planet	afl2planet	/* change of original name */


# define J2000	2451545.0	/* Epoch of JPL ephemeris DE200, absolute */
# define J1950  2433282.423	/* Epoch of JPL ephemeris DE102 */
# define JUL_OFFSET 2433282.0	/* offset of Astrodienst relative Julian date */

#ifndef GREG_CAL
# define GREG_CAL	SE_GREG_CAL
# define JUL_CAL	SE_JUL_CAL
#endif

/*
 * flag bits used in calc and calcserv
 */
# define CALC_BIT_HELIO	1	/* geo/helio */
# define CALC_BIT_NOAPP	2	/* apparent/true positions */
# define CALC_BIT_NONUT	4	/* true eq. of date/ mean equ. of date */
# define CALC_BIT_EPHE	8	/* universal/ephemeris time */
# define CALC_BIT_SPEED	16	/* without/with speed */
# define CALC_BIT_BETA	32	/* without/with latitude */
# define CALC_BIT_RGEO	64	/* without/with relative rgeo */
# define CALC_BIT_RAU	128	/* without/with real radius */
# define CALC_BIT_MUST_USE_EPHE	256	/* epheserv may not use calc */
# define CALC_BIT_MAY_USE_EPHE	512	/* calcserv may use ephread */

/*
 * stuff from astrolib.h
 */

#ifndef ADATE	/* this must be bracketed because users of swepcalc
		   may also include astrolib.h for other reasons */
#define ADATE struct adate

/* makros for bit operations */
# define clear_bit(v,bit_nr) 	((v) & ~(1L << (bit_nr)))
# define set_bit(v,bit_nr) 	((v) | (1L << (bit_nr)))
# define bit(bit_nr)		(1L << (bit_nr))
# define check_bit(v,bit_nr)	((v) & (1L << (bit_nr)))

ADATE {	/* date structure used by revjuls and juldays */
       int day, month, year;
       centisec csec;
       };

#endif	/* ADATE */

/*
 * functions exported by swepdate.c
 */
extern double julday(int month, int day, int year, double hour, int gregflag);
extern double juldays(int gregflag, ADATE *adp); 
extern void revjul (double u, int gregflag,
	      int *jmon, int *jday, int *jyear, double *jut);
extern void revjuls(double u, int gregflag, ADATE *adp); 
extern int day_of_week(double t);
/*
 * end swpdate.c
 */

/*
 * stuff from housasp.h
 */
#ifndef ASP_144		/* allow including housasp wihout conflict */
#define MAXPLANETS  16

/*
 * definitions for aspect numbering; we always have a name and an angle 
 * due to historical reasons index 0 is unused, conjunct is 1.
 * We define three different names for the aspects:
 * ASP_CONJ (always 4 letters), ASP_0 and CONJ.
 */
# define ASP_CONJ	1
# define ASP_0		1
# define ASP_OPPO	2
# define ASP_180	2
# define ASP_SQUA	3
# define ASP_90		3
# define ASP_TRIN	4
# define ASP_120	4
# define ASP_SEXT	5
# define ASP_60		5
# define ASP_SMSX	6
# define ASP_30		6
# define ASP_QCNX	7
# define ASP_150	7
# define ASP_SMSQ	8
# define ASP_45 	8
# define ASP_SQSQ	9
# define ASP_135	9
# define ASP_QINT	10
# define ASP_72 	10
# define ASP_BQIN	11
# define ASP_144	11

# define CONJ	ASP_CONJ	
# define OPPO	ASP_OPPO	
# define SQUA	ASP_SQUA	
# define TRIN	ASP_TRIN	
# define SEXT	ASP_SEXT	
# define SMSX	ASP_SMSX	
# define QCNX	ASP_QCNX	
# define SMSQ	ASP_SMSQ	
# define SQSQ	ASP_SQSQ	
# define QINT	ASP_QINT	
# define BQIN	ASP_BQIN	

#define MAXASPECTS      ASP_BQIN

/*
 * for compact encoding of aspect lists we set bit 1 for CONJ, bit 2 for OPPO
 * and so on. asp_bit(asp) deleivers the mask.
 */
#define ALL_ASP_BITS	1022	/* bit mask with all aspect bits set */
#define STRONG_ASP_BITS	62	/* bit mask with strong aspect bits set */
#define HARD_ASP_BITS	14	/* bit mask with hard aspect bits set */

#define asp_bit(asp)	(1 << (asp))

/*
 * sometimes it is desirable to have a compact way to express planet-aspect
 * combinations.
 * We define PASP(planet,aspect) as a constant,  100*planet  + aspect.
 * We can then write things like:
 * case PASP(PLUTO, JONJ): ...
 * A definition like p << 8 + a would be faster but the combinations would
 * be harder to read.
 */
#define PASP(p,a)	(100*(p) + (a))

/*
 * used to initialize an array centisec angles[MAXASPECTS+1]
 */
# define ASP_ANGLES {0,	0*DEG,	180*DEG, 90*DEG, 120*DEG,\
	60*DEG, 30*DEG, 150*DEG, 45*DEG, 135*DEG, 72*DEG, 144*DEG}

struct  AspectType { 
  	AS_BOOL	dataValid;	/* used as boolean */
	int     NrOfPlanets,
	        NrOfAspects;
	centisec *PlanetPos;	/* pointer to an array of planet positions 
				   [0..NrOfPlanets-1]; the user of the
				   structure must set this pointer to his
				   array of planet positions */
        centisec *ppos2;	/* second set for mutual aspects only; if
				   not NULL, interaspects are calculated */
	centisec *Maxorb;	/* pointer to an array [0..NrOfAspects] of
				   maximum orbes; element[0] not used;
				   the user of the structure must set this 
				   pointer to his array of maxorbs */
	centisec *Angle;	/* pointer to Angle[0..NrOfAspects] of the
				   angles of aspects; [0] not used; the
				   user must set this pointer to his array
				   of angles */
	struct aspRec { 
		int  index; 	/* number of the found aspect */
		centisec orb;
	       }
		Asp [MAXPLANETS] [MAXPLANETS];
	     };

struct  houses {
	  centisec cusp[13];
	  centisec ac;
	  centisec mc;
	};

# define HOUSES 	struct houses
#endif	/* ifndef ASP_144 */
/**********************************
  functions exported originally from housasp.c 
***********************************/

extern int HouseNr(HOUSES *h, CSEC p);
  /*
  return in which house pp is,
  The caller is responsible for proper initialization of cusps
  */

extern int InpHouseNr(HOUSES *h, CSEC p, CSEC *cuspoff);
  /* returns the interpretation-house number, where pp is in;
   * an interpretation house is defined differently from a normal
   * house: the cusps are offset, so that it begins and ends
   * a little earlier.
   * cusp[1..12] and  cuspoff[1..12]  must be initialized 
   */
extern int InpHouseNr2(HOUSES *h, CSEC p, CSEC *cuspoff);
  /* variation of InpHouseNr(). Comment in swepcalc.c */

extern void CalcHouses(CSEC th, CSEC fi, CSEC ekl, char hsy, int icnt,
	struct houses *h);

extern void RecalcAspects(struct AspectType *a);

extern void longreorder (UCHAR *p, int n);


#endif /* _SWEPCALC_INCLUDED */
