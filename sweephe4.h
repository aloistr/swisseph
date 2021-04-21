/*******************************************************

 header file for structures and functions in module sweephe.c
 for reading and writing stored ephemerides in format ep4
 a fast precomputed ephemeris used in some Astrodienst applications.

 The design of ephemeris type ep4:
 In all ASYS and transit application of stored ephemerides
 except Progressed Declinations Type 56 we need only the
 longitudes of the planets or nodes.
 The old EP3 format contains also latitudes, and uses ephemeris time.
 Therefore we define a new ephemeris format, which is capable of
 replacing EP3, when some ASYS programs are changed.
 The ASYS programs requiring different data can receive them
 by asking the calcserv module.

 We therefore store now a daily ephemeris with only logitudes, ecl and nut.
 The ephemeris is computed and stored for midnight ephemeris time, i.e.
 for jd = 2400000.5, 2400001.5 etc.
 In the ephemeris record for this date, only floor(jd) is kept.

 In many cases universal time (UT) is desired, not ephemeris time.
 Because computation with our current computers is very cheap for
 everything except trigonometrci functions, we can afford to
 build a good interpolation into the ephemeris package.

 The user can request from ephread() ephemeris positions for
 any (double) jd, not only for midnight ephemeris time.
 Inside the package the same quick Everett 5th-order interpolator
 is used as in placalc.
 It delivers positions within 0.01" for all planets except moon, mercury
 and true node. Moon and Mercury suffer, because their positions are
 stored only with a precision of 0.1"; true node suffers because
 it oscillates quickly with the fastest moon terms.
 The maximum observed differences between placalc and ephread for 12.00 noon
 are 0.25" for moon and true node and 0.1" for Mercury; in 80% of the days
 the differences are less than 0.1". This is significantly better than
 the implemented precision of the placalc package itself.

 The Everett interpolator delivers without any additional cost also
 the speed of the planets. This speed is very much better than the
 speed derived for the inner planets from the mean orbits.

 The returned data from ephread are in an array of centisec,
 with ecl and nut behind the planets.
 The default, pflag = 0, returns all.
 The speeds are returned in the second half of the array;
 the speed is always there, even when the speed bit has not been set.
***********************************************************/
/* Copyright (C) 1997 - 2021 Astrodienst AG, Switzerland.  All rights reserved.

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
  a) GNU Affero General Public License (AGPL)
  b) Swiss Ephemeris Professional License

  The choice must be made before the software developer distributes software
  containing parts of Swiss Ephemeris to others, and before any public
  service using the developed software is activated.

  If the developer choses the AGPL software license, he or she must fulfill
  the conditions of that license, which includes the obligation to place his
  or her whole software project under the AGPL or a compatible license.
  See https://www.gnu.org/licenses/agpl-3.0.html

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

# ifndef _EPHE_INCLUDED
# define _EPHE_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

// definitions must agree wit astrolib.h
#define PLACALC_SUN	0		/* used synonymously for earth too */
#define PLACALC_EARTH	0
#define PLACALC_MOON	1
#define PLACALC_MERCURY 2
#define PLACALC_VENUS	3
#define PLACALC_MARS	4
#define PLACALC_JUPITER	5
#define PLACALC_SATURN	6
#define PLACALC_URANUS	7
#define PLACALC_NEPTUNE 8
#define PLACALC_PLUTO	9
#define PLACALC_LASTPLANET PLUTO
#define PLACALC_MEAN_NODE  10
#define PLACALC_TRUE_NODE  11
#define PLACALC_CHIRON	   12
#define PLACALC_LILITH	   13	
#define PLACALC_CALC_N	  14	/* number of planets in placalc module */
#define PLACALC_CERES     14
#define PLACALC_PALLAS    15
#define PLACALC_JUNO      16
#define PLACALC_VESTA     17
#define PLACALC_EARTHHEL  18	/* heliocentric earth */
#define PLACALC_PFORTUNAE 19
// next numbers after 19 are reserved for AC, MC, houses, signs;
# define PLACALC_AC	   20
# define PLACALC_ASC	   20
# define PLACALC_MC	   21
# define PLACALC_CALC_N_MC  22	/* number of normal natal factors */

# define EP4_BLOCKSIZE  sizeof(struct ep4)
# if HPUNIX
#   define EP4_PATH "/home/ephe/"
# else
#   define EP4_PATH  "ephe\\"
# endif
# define EP4_FILE	"sep4_"		/* packed ephemeris */
# define EP4_NDAYS	10000L		/* days  per EP4_ file */
# define NDB		10L		/* 10 days per block */

/*
 * bits for plalist in ephread():
 * the planet flag bits SUN .. PLACALC_CHIRON, ECl, NUT can be set individually.
 * plalist = 0 is equivalent to all planets and ecl,nut
 * EP_ALL_PLANETS sets all planet bits SUN .. PLACALC_CHIRON
 * EP_ALL_BITS sets all bits.
 */

# define EP_NP (PLACALC_CHIRON + 3)		/* total number of factors in ep4 */
					/* sun .. chiron, ecl, nut */

# define EP_ALL_PLANETS  ((1 << (PLACALC_CHIRON + 1)) - 1)	/* bits 0..12 set */
# define EP_CALC_N	(PLACALC_CHIRON+1)	/* 13 planets, SUN .. PLACALC_CHIRON */
# define EP_ECL_INDEX  (PLACALC_CHIRON + 1)	/* index for ecliptic centisec */
# define EP_NUT_INDEX  (PLACALC_CHIRON + 2)	/* index for nutation centisec */
# define EP_ECL_BIT   (1 << EP_ECL_INDEX)
# define EP_NUT_BIT   (1 << EP_NUT_INDEX)
# define EP_ALL_BITS   (EP_ALL_PLANETS|EP_ECL_BIT|EP_NUT_BIT)

// bits for flag in ephread(), values come from placalc compatibility
# define EP_BIT_SPEED	16	// must get speed
# define EP_BIT_MUST_USE_EPHE 256


struct elon  {			/* longitudes for 10 days */
	short	p0m;		/* longitude at day 0, minutes */
	short	p0s;		/* 			0.01" */
	short	pd1m;		/* delta of days 1, 0.01" */
	short	pd1s;		/* 			0.01" */
	short	pd2[NDB-2];		/* second differences, day 2 .. 9, 
				   0.1" moon, mercury, 0.01" others*/
	};

/*
 * ep4 is the new  ephemeris format for longitude only, ephemeris time
 */
struct ep4  {
  short   j_10000;	/* floor(jd - 0.5) / 10000L; */
  short   j_rest;	/* (jd - 0.5 ) - 10000L * j_10000 
			   j_rest is always positive;
			   jd = j_10000 * 10000L + j_rest + 0.5	*/
  short	ecl0m;		/* true ecliptic day 0, min ; */
  short ecl0s;		/*                      0.01" */
  short ecld1[NDB-1];	/* first differences 0.01", day 1..9 */
  short	nuts[NDB];	/* nutation in 0.01", day 0..9 */
  struct  elon elo[PLACALC_CHIRON +1];	 	/* longitude sun...chiron */
};


/******************************************
 globals exported by module ephe.c
********************************************/
extern FILE *ephfp;

/******************************************
 functions exported by module ephe.c
********************************************/

extern centisec *ephread(double jd, int plalist, int flag, char *errtext);
/*
 * This is the only function normally used by applications.
 * ATTENTION: jd is an absolute Julian date, whereas calc() and deltat()
 *	      require Astrodienst-relative Julian dates.
 * plalist can consist of individual planet bit flags, to indicate that
 * only these planets are wanted. 
 * plalist = 0 returns all planets, and ecl and nut.
 * Because the computation is so fast, it is recommended to use pflag = 0.
 * flag recognizes only the bits CALC_BIT_SPEED and CALC_MUST_USE_EPHE.
 * If CALC_BIT_SPEED is set, of the planets are returned in the result array
 * after all longitudes (speeds for ecl and nut are always set to zero).
 * If CALC_BIT_MUST_USE_EPHE is NOT set, calc() will be used if reading
 * the ephemeris fails.
 * cp is the returned pointer, the longitude of the planet p is
 * at cp[p] and the speed is at cp[p + EP_NP].
 * The returned longitudes are always normalized into [0..DEG360[,
 * except for nut, which is small and close to zero, negative or positive.
 */
extern double *dephread2(double jd, int plalist, int flag, char *errtext);

extern int eph4_posit (int jlong, AS_BOOL writeflag, char *errtext);

extern int ephe_plac2swe(int p);

extern void shortreorder (UCHAR *p, int n);

#ifdef __cplusplus
}
#endif

# endif	/* _EPHE_INCLUDED */
