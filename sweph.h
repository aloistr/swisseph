/************************************************************
   definitions and constants SWISSEPH

  Authors: Dieter Koch and Alois Treindl, Astrodienst Zurich

************************************************************/
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

/*
 * move over from swephexp.h
 */

#define SE_VERSION      "2.10.03" 

#define J2000           2451545.0  	/* 2000 January 1.5 */
#define B1950           2433282.42345905  	/* 1950 January 0.923 */
#define J1900           2415020.0  	/* 1900 January 0.5 */
#define B1850           2396758.2035810  	/* 1850 January 16:53 */

#define MPC_CERES       1
#define MPC_PALLAS      2
#define MPC_JUNO        3
#define MPC_VESTA       4
#define MPC_CHIRON      2060
#define MPC_PHOLUS      5145

#define SE_NAME_SUN             "Sun"
#define SE_NAME_MOON            "Moon"
#define SE_NAME_MERCURY         "Mercury"
#define SE_NAME_VENUS           "Venus"
#define SE_NAME_MARS            "Mars"
#define SE_NAME_JUPITER         "Jupiter"
#define SE_NAME_SATURN          "Saturn"
#define SE_NAME_URANUS          "Uranus"
#define SE_NAME_NEPTUNE         "Neptune"
#define SE_NAME_PLUTO           "Pluto"
#define SE_NAME_MEAN_NODE       "mean Node"
#define SE_NAME_TRUE_NODE       "true Node"
#define SE_NAME_MEAN_APOG       "mean Apogee"
#define SE_NAME_OSCU_APOG       "osc. Apogee"
#define SE_NAME_INTP_APOG       "intp. Apogee"
#define SE_NAME_INTP_PERG       "intp. Perigee"
#define SE_NAME_EARTH           "Earth"
#define SE_NAME_CERES           "Ceres"
#define SE_NAME_PALLAS          "Pallas"
#define SE_NAME_JUNO            "Juno"
#define SE_NAME_VESTA           "Vesta"
#define SE_NAME_CHIRON          "Chiron"
#define SE_NAME_PHOLUS          "Pholus"

 
#define SE_NAME_CUPIDO          "Cupido"
#define SE_NAME_HADES           "Hades"
#define SE_NAME_ZEUS            "Zeus"
#define SE_NAME_KRONOS          "Kronos"
#define SE_NAME_APOLLON         "Apollon"
#define SE_NAME_ADMETOS         "Admetos"
#define SE_NAME_VULKANUS        "Vulkanus"
#define SE_NAME_POSEIDON        "Poseidon"
#define SE_NAME_ISIS            "Isis"
#define SE_NAME_NIBIRU          "Nibiru"
#define SE_NAME_HARRINGTON      "Harrington"
#define SE_NAME_NEPTUNE_LEVERRIER       "Leverrier"
#define SE_NAME_NEPTUNE_ADAMS   "Adams"
#define SE_NAME_PLUTO_LOWELL    "Lowell"
#define SE_NAME_PLUTO_PICKERING "Pickering"
#define SE_NAME_VULCAN          "Vulcan"
#define SE_NAME_WHITE_MOON      "White Moon"

/*
 * earlier content
 */

#define PI              M_PI	/* 3.14159265358979323846, math.h */
#define TWOPI           (2.0 * PI)

#define ENDMARK         -99

#define SEI_EPSILON     -2
#define SEI_NUTATION    -1
#define SEI_EMB		0	
#define SEI_EARTH	0	
#define SEI_SUN  	0	
#define	SEI_MOON	1	
#define	SEI_MERCURY	2	
#define	SEI_VENUS	3	
#define	SEI_MARS	4	
#define	SEI_JUPITER	5	
#define	SEI_SATURN	6	
#define	SEI_URANUS	7	
#define	SEI_NEPTUNE	8	
#define	SEI_PLUTO	9	
#define	SEI_SUNBARY	10	/* barycentric sun */
#define	SEI_ANYBODY	11	/* any asteroid */
#define	SEI_CHIRON	12	
#define	SEI_PHOLUS	13	
#define	SEI_CERES	14	
#define	SEI_PALLAS	15	
#define	SEI_JUNO	16	
#define	SEI_VESTA	17	

#define SEI_NPLANETS    18

#define SEI_MEAN_NODE   0
#define SEI_TRUE_NODE   1
#define SEI_MEAN_APOG   2
#define SEI_OSCU_APOG   3
#define SEI_INTP_APOG   4
#define SEI_INTP_PERG   5

#define SEI_NNODE_ETC    6

#define SEI_FLG_HELIO   1
#define SEI_FLG_ROTATE  2
#define SEI_FLG_ELLIPSE 4
#define SEI_FLG_EMBHEL  8   	/* TRUE, if heliocentric earth is given
				 * instead of barycentric sun 
				 * i.e. bary sun is computed from 
				 * barycentric and heliocentric earth */

#define SEI_FILE_PLANET	  0
#define SEI_FILE_MOON	  1 
#define SEI_FILE_MAIN_AST 2
#define SEI_FILE_ANY_AST  3
#define SEI_FILE_FIXSTAR  4
#define SEI_FILE_PLMOON   5

#if 0
#define SEI_FILE_TEST_ENDIAN     (97L * 65536L + 98L * 256L + 99L) /*abc*/
#endif
#define SEI_FILE_TEST_ENDIAN     (0x616263L) 	/* abc*/ 
#define SEI_FILE_BIGENDIAN	0
#define SEI_FILE_NOREORD	0
#define SEI_FILE_LITENDIAN	1
#define SEI_FILE_REORD  	2

#define SEI_FILE_NMAXPLAN	50
#define SEI_FILE_EFPOSBEGIN      500

#define SE_FILE_SUFFIX	"se1"

#define SEI_NEPHFILES   7
#define SEI_CURR_FPOS   -1
#define SEI_NMODELS 8

#define SEI_ECL_GEOALT_MAX   25000.0
#define SEI_ECL_GEOALT_MIN   (-500.0)

/* Chiron's orbit becomes chaotic 
 * before 720 AD and after 4606 AD, because of close encounters
 * with Saturn. Accepting a maximum error of 5 degrees, 
 * the ephemeris is good between the following dates:
 */
/*#define CHIRON_START    1958470.5  	* 1.1.650 old limit until v. 2.00 */
#define CHIRON_START    1967601.5  	/* 1.1.675 */
#define CHIRON_END      3419437.5  	/* 1.1.4650 */

/* Pholus's orbit is unstable as well, because he sometimes
 * approaches Saturn.
 * Accepting a maximum error of 5 degrees,
 * the ephemeris is good after the following date:
 */
/* #define PHOLUS_START    314845.5  	* 1.1.-3850  old limit until v. 2.00 */
#define PHOLUS_START    640648.5	/* 1.1.-2958 jul */
#define PHOLUS_END      4390617.5  	/* 1.1.7309 */

#define MOSHPLEPH_START	 625000.5
#define MOSHPLEPH_END  	2818000.5
#define MOSHLUEPH_START	 625000.5
#define MOSHLUEPH_END  	2818000.5
/*#define MOSHNDEPH_START	-254900.5 */	/* 14 Feb -5410 00:00 ET jul.cal.*/
/*#define MOSHNDEPH_END  	3697000.5 */	/* 11 Dec 5409 00:00 ET, greg. cal */
#define MOSHNDEPH_START	-3100015.5	/* 15 Aug -13200 00:00 ET jul.cal.*/
#define MOSHNDEPH_END  	8000016.5       /* 15 Mar 17191 00:00 ET, greg. cal */
/*
#define MOSHPLEPH_START	 -225000.5
#define MOSHPLEPH_END  	3600000.5
#define MOSHLUEPH_START	 -225000.5
#define MOSHLUEPH_END  	3600000.5
*/
#define JPL_DE431_START -3027215.5 
#define JPL_DE431_END    7930192.5

#if FALSE	/*	Alois commented out, not used anywhere  */
#define JPLEPH_START	 625307.5	/* about -3000 (DE406) */
#define JPLEPH_END	2816848.5	/* about  3000 (DE406) */
#define SWIEPH_START	 625614.927151
#define SWIEPH_END	2813641.5
#define ALLEPH_START	MOSHPLEPH_START
#define ALLEPH_END	MOSHPLEPH_END
#define BEG_YEAR       (-3000)
#define END_YEAR       3000
#endif

#define MAXORD          40

#define NCTIES         6.0     /* number of centuries per eph. file */

#define OK (0)
#define ERR (-1)
#define NOT_AVAILABLE (-2)
#define BEYOND_EPH_LIMITS (-3)

#define J_TO_J2000   	1
#define J2000_TO_J   	-1

/* we always use Astronomical Almanac constants, if available */
#define MOON_MEAN_DIST  384400000.0		/* in m, AA 1996, F2 */
#define MOON_MEAN_INCL  5.1453964		/* AA 1996, D2 */
#define MOON_MEAN_ECC   0.054900489		/* AA 1996, F2 */
/* #define SUN_EARTH_MRAT  328900.561400           Su/(Ea+Mo) AA 2006 K7 */
#define SUN_EARTH_MRAT  332946.050895           /* Su / (Ea only) AA 2006 K7 */   
#define EARTH_MOON_MRAT (1 / 0.0123000383)	/* AA 2006, K7 */
#if 0
#define EARTH_MOON_MRAT 81.30056907419062	/* de431 */
#endif
#if 0
#define EARTH_MOON_MRAT 81.30056		/* de406 */
#endif
//#define AUNIT       	1.49597870691e+11  	/* au in meters, AA 2006 K6 */
#define AUNIT       	1.49597870700e+11  	/* au in meters, DE431 */
#define CLIGHT       	2.99792458e+8   	/* m/s, AA 1996 K6 / DE431 */
#if 0
#define HELGRAVCONST    1.32712438e+20		/* G * M(sun), m^3/sec^2, AA 1996 K6 */
#endif
#define HELGRAVCONST    1.32712440017987e+20	/* G * M(sun), m^3/sec^2, AA 2006 K6 */
#define GEOGCONST       3.98600448e+14 		/* G * M(earth) m^3/sec^2, AA 1996 K6 */
#define KGAUSS		0.01720209895		/* Gaussian gravitational constant K6 */
#define SUN_RADIUS      (959.63 / 3600 * DEGTORAD)  /*  Meeus germ. p 391 */
#define EARTH_RADIUS	6378136.6		/* AA 2006 K6 */
//#define EARTH_OBLATENESS (1.0/ 298.257223563)	/* AA 1998 K13 */
#define EARTH_OBLATENESS (1.0/ 298.25642)	/* AA 2006 K6 */
#define EARTH_ROT_SPEED (7.2921151467e-5 * 86400) /* in rad/day, expl. suppl., p 162 */

#define LIGHTTIME_AUNIT  (499.0047838362/3600.0/24.0) 	/* 8.3167 minutes (days) */
#define PARSEC_TO_AUNIT  206264.8062471         /* 648000/PI, according to IAU Resolution B2, 2016 */

/* node of ecliptic measured on ecliptic 2000 */
#define SSY_PLANE_NODE_E2000    (107.582569 * DEGTORAD)
/* node of ecliptic measured on solar system rotation plane */
#define SSY_PLANE_NODE          (107.58883388 * DEGTORAD)
/* inclination of ecliptic against solar system rotation plane */
#define SSY_PLANE_INCL          (1.578701 * DEGTORAD)

#define KM_S_TO_AU_CTY	 21.095			/* km/s to AU/century */
#define MOON_SPEED_INTV  0.00005 		/* 4.32 seconds (in days) */
#define PLAN_SPEED_INTV  0.0001 	        /* 8.64 seconds (in days) */
#define MEAN_NODE_SPEED_INTV  0.001		
#define NODE_CALC_INTV  0.0001		
#define NODE_CALC_INTV_MOSH   0.1		
#define NUT_SPEED_INTV   0.0001
#define DEFL_SPEED_INTV  0.0000005

#define SE_LAPSE_RATE        0.0065  /* deg K / m, for refraction */

#define square_sum(x)   (x[0]*x[0]+x[1]*x[1]+x[2]*x[2])
#define dot_prod(x,y)   (x[0]*y[0]+x[1]*y[1]+x[2]*y[2])

#define PNOINT2JPL {J_EARTH, J_MOON, J_MERCURY, J_VENUS, J_MARS, J_JUPITER, J_SATURN, J_URANUS, J_NEPTUNE, J_PLUTO, J_SUN, }

/* planetary radii in meters */
#define NDIAM  (SE_VESTA + 1)
static const double pla_diam[NDIAM] = {1392000000.0, /* Sun */
                           3475000.0, /* Moon */
                           2439400.0 * 2, /* Mercury */
                           6051800.0 * 2, /* Venus */
                           3389500.0 * 2, /* Mars */
                          69911000.0 * 2, /* Jupiter */
                          58232000.0 * 2, /* Saturn */
                          25362000.0 * 2, /* Uranus */
                          24622000.0 * 2, /* Neptune */
                           1188300.0 * 2, /* Pluto */
                           0, 0, 0, 0,    /* nodes and apogees */
                           6371008.4 * 2, /* Earth */
                            271370.0, /* Chiron */
                            290000.0, /* Pholus */
                            939400.0, /* Ceres */
                            545000.0, /* Pallas */
                            246596.0, /* Juno */
                            525400.0, /* Vesta */
                        };


/* Ayanamsas 
 * For each ayanamsa, there are the following values:
 * t0       epoch of ayanamsa, TDT (can be ET or UT)
 * ayan_t0  ayanamsa value at epoch
 * t0_is_UT true, if t0 is UT
 * prec_offset is the precession model for which the ayanamsha
 *          has to be corrected by adding/subtracting a constant offset. 
 *          0, if no correction is needed
 *          -1, if correction is unclear or has not been investigated
 *              and therefore is not applied
 */
struct aya_init {double t0;
                 double ayan_t0; 
		 AS_BOOL t0_is_UT;
		 int prec_offset;};
static const struct aya_init ayanamsa[SE_NSIDM_PREDEF] = {
/* 0: Fagan/Bradley (Default) 
     "The American Sidereal Ephemeris, 1976-2000" (Astro Computing Services, 1981)
     states on S.V.P. ("Synetic Vernal Point"):
     "The S.V.P. is the Sidereal longitude of the Vernal Equinox (the
     Tropical zero-point) in the Fagan-Bradley school of Western Sidereal
     astrology. It was determined empirically, its mean value being defined
     as 335°57'28".64 for the epoch 1950.0."
     Fagan/Firebrace, "Primer of Sidereal Astrology", p. 13:
     "It was during 1957 that Garth Allen .... experimenting ... But when
     progressed for the dates of the calamities, all were found by him to be 
     slightly out, the mean error being equivalent to an increase of 0°06'05"
     in the then-adopted sidereal longitude of the vernal point, determined
     from Spica in 29 Virgo (i.e. 29°06'05" Virgo; D.K.), and the proper motion
     having been allowed for. In short, for the epoch 1950.0 he proposed as the 
     mean longitude of the vernal point 335°57'28.64", proper motion being 
     disregarded."
     If "1950.0" means the standard epoch B1950 = JD 2433282.423, and based 
     on the then-used precession model of Newcomb, this ayanamsha leads to 
     a true position of 29°06'05.965" Virgo, based on Hipparcos position of 
     the star. */
{2433282.42346, 24.042044444, FALSE, SEMOD_PREC_NEWCOMB}, // 0: Fagan/Bradley
/*************************/
/* 1: Standard Lahiri 
     according to program NOVA by Robert Hand: 
     {J1900, 360 - 337.53953},   
     This corresponds to an ayanamsha 22°27'37.69 as given in
     Indian Ephemeris and Nautical Almanac" 1965, p. 459.
     Note, however, this value should only with a precession formula 
     where T is measured in tropical centuries. Swiss Ephemeris always
     uses Julian centuries.
     The following definition is according to:
     Calendar Reform Committee 1956; the subtracted value is nutation:
     {2435553.5, 23.25 - 0.00464207, FALSE}, 
     Lahiri (derived from: Indian Astronomical Ephemeris 1989, p. 556;
     the subtracted value is nutation, according to Wahr 1980) */
{2435553.5, 23.250182778 - 0.004658035, FALSE, SEMOD_PREC_IAU_1976}, // 1: Lahiri
/*************************/
/* 2: Robert DeLuce (Constellational Astrology ... p. 5; 
     birth of Jesus assumed on 1 Jan 1 BC (= 0) jul.,
     based on Newcomb precession. 
     {J1900, 360 - 333.58695, FALSE, 0}, 
     Ayanamsha was corrected with SE 2.09 as follows:
     Started at zero ayanamsha epoch with value 0 and 
     run with standard precession.
     This makes a difference of 22" compared with previous version: */
{1721057.5, 0, TRUE, 0}, // 2: DeLuce
/*************************/
/* 3: B.V. Raman (Robert Hand) 
     See B.V. Raman, "Hindu Predictive Astrology" (1938, Introduction), 
     pp. 279, 287.
     This ayanamsha is apparently not based on a valid precession theory (e.g.
     Newcomb). We cannot reproduce precisely the ayanamsha values on p. 287. */
{J1900, 360 - 338.98556, FALSE, SEMOD_PREC_NEWCOMB}, // 3: Raman
/*************************/
/* 4: Usha/Shashi (Robert Hand) 
     Usha and Shashi, "Hindu Astrological Calculations" (1978, 
     Sagar Publications, New Delhi).
     We do not have this book. */
{J1900, 360 - 341.33904, FALSE, -1}, // 4: Usha/Shashi
/*************************/
/* 5: Krishnamurti (Robert Hand)
     K.S. Krishnamurti, "Reader 1", pp. 55-59.
     Autor does not give precise information. Zero ayanamsha year is said to 
     be 291 CE, and there is an ayanamsha table with arc min precision for
     1840 to 2000 on p. 58.
     This ayanamsha reproduces the table quite well, if 1 Jan of each year
     is taken. (Turn off Newcomb precession in order to verify.)
     However, D. Senthilathiban believes the values are given for the date
     of sidereal Aries ingress of each year. ("Study of KP Ayanamsa with
     Modern Precession Theories", pp. 126f. */
{J1900, 360 - 337.636111, FALSE, SEMOD_PREC_NEWCOMB}, // 5: Krishnamurti
/*************************/
/* 6: Djwhal Khool (Graham Dawson), 
     "Channeled" information: Aquarius ingress of VP on 1 July 2117
     See Philipp Lindsay, “The Beginning of the Age of Aquarius: 2,117 A.D.”,
     http://esotericastrologer.org/newsletters/the-age-of-aquarius-ray-and-zodiac-cycles/ */
/*************************/
{J1900, 360 - 333.0369024, FALSE, 0}, // 6: Djwhal Khool
/* 7: Shri Yukteshwar; (David Cochrane) 
     This ayanamsha seems to be wrong.
     Swami Sri Yukteswar, "The Holy Science", 1920 (1949, 1957 and 1977, 
     partly revised), Yogoda Satsanga Society of India.
     Ayanamsha on the spring equinox 1893 was 20°54'36" (1894 according to 
     the revised edition of 1977) At the same time he believed that this was the
     distance of the spring equinox from the star Revati, which he put at the
     initial point of Aries.  Unfortunately, this is wrong, because on that date
     Revati was actually 18°23' away from the vernal point. The error is
     explained from the fact that Yukteshwar used the zero ayanamsha year 499 CE
     and an inaccurate Suryasiddhantic precession rate of 360°/24'000 years = 54
     arcsec/year. It is obvious that Yukteshwar actually intended an ayanamsha
     that starts at the star Revati.  */
{J1900, 360 - 338.917778, FALSE, -1},  // 7: Shri Yukteshwar
//{2412543.5, 20.91, TRUE, -1},        // 7: Shri Yukteshwar; (Holy Science, p. xx) 
/*************************/
/* 8: J.N. Bhasin; (David Cochrane) 
     We don't have any sources or detailed information about this ayanamsha. */
{J1900, 360 - 338.634444, FALSE, -1}, // 8: Bhasin
/*************************/
/* 14 Sept. 2018: the following three ayanamshas have been wrong for
   many years */
/* 9 - 11: Babylonian, Kugler */
{1684532.5, -5.66667, TRUE, -1},     //  9: Babylonian, Kugler 1
{1684532.5, -4.26667, TRUE, -1},     // 10: Babylonian, Kugler 2
{1684532.5, -3.41667, TRUE, -1},     // 11: Babylonian, Kugler 3 
/*************************/
/* 12: Babylonian, Huber 
      P. Huber, "Über den Nullpunkt der babylonischen Ekliptik", in: Centaurus
      1958, 5, p. 192-208. 
      This ayanamsha had a wrong initial value until 14 Sept. 2018. */
{1684532.5, -4.46667, TRUE, -1},     // 12: Baylonian, Huber 
/*************************/
/* 13: Babylonian, Mercier; eta Piscium culminates with zero point */
{1673941, -5.079167, TRUE, -1},      // 13: Babylonian, Mercier
/*************************/
/* 14: t0 is defined by Aldebaran at 15 Taurus in year -100 */
{1684532.5, -4.44138598, TRUE, 0},  // 14: Babylonian/Aldebaran = 15 Tau
/*************************/
/* 15: Hipparchos */
{1674484.0, -9.33333, TRUE, -1},       // 15: Hipparchos
/*************************/
/* 16: Sassanian */
{1927135.8747793, 0, TRUE, -1},      // 16: Sassanian 
/*************************/
/* 17: Galactic Center at 0 Sagittarius */
{0, 0, FALSE, 0},                    // 17: Galactic Center at 0 Sagittarius 
/*************************/
/* 18: J2000 */
{J2000, 0, FALSE, 0},	             // 18: J2000
/*************************/
/* 19: J1900 */
{J1900, 0, FALSE, 0},	             // 19: J1900
/*************************/
/* 20: B1950 */
{B1950, 0, FALSE, 0},	             // 20: B1950
/*************************/
/* 21: Suryasiddhanta, assuming ingress of mean Sun into Aries at point of mean
      equinox of date on 21.3.499, near noon, Ujjain (75.7684565 E) 
      = 7:30:31.57 UT = 12:33:36 LMT*/
{1903396.8128654, 0, TRUE, 0},	     // 21: Suryasiddhanta
/*************************/
/* 22: Suryasiddhanta, assuming ingress of mean Sun into Aries at true position
      of mean Sun at same epoch */
{1903396.8128654,-0.21463395, TRUE, 0}, // 22: Suryasiddhanta, mean Sun
/*************************/
/* 23: Aryabhata, same date, but UT 6:56:55.57 analogous to 21 */
{1903396.7895321, 0, TRUE, 0},	     // 23: Aryabhata
/*************************/
/* 24: Aryabhata, analogous 22 */
{1903396.7895321,-0.23763238, TRUE, 0}, // 24: Aryabhata, mean Sun
/*************************/
/* 25: Suryasiddhanta, Revati/zePsc at polar long. 359°50'*/
{1903396.8128654,-0.79167046, TRUE, 0}, // 25: SS Revati
/*************************/
/* 26: Suryasiddhanta, Citra/Spica at polar long. 180° */
{1903396.8128654, 2.11070444, TRUE, 0}, // 26: SS Citra
/*************************/
/* 27: True Citra (Spica exactly at 0 Libra) */
{0, 0, FALSE, 0},	             // 27: True Citra 
/*************************/
/* 28: True Revati (zeta Psc exactly at 29°50' Pisces) */
{0, 0, FALSE, 0},	             // 28: True Revati 
/*************************/
/* 29: True Pushya (delta Cnc exactly a 16 Cancer */
{0, 0, FALSE, 0},		     // 29: True Pushya 
/*************************/
/* 30: R. Gil Brand; Galactic Center at golden section between 0 Sco and 0 Aqu;
      note: 0° Aqu/Leo is the symmetric axis of rulerships */
{0, 0, FALSE, 0},                    // 30: Gil Brand
/*************************/
/* 31: Galactic Equator IAU 1958, i.e. galactic/ecliptic intersection point
      based on galactic coordinate system */
{0, 0, FALSE, 0},	             // 31: GE IAU 1958
/*************************/
/* 32: Galactic Equator True, i.e. galactic/ecliptic intersection point based
     on the galactic pole as given in: Liu/Zhu/Zhang, „Reconsidering the
     galactic coordinate system“, A & A No. AA2010, Oct. 2010 */
{0, 0, FALSE, 0},	             // 32: GE true
/*************************/
/* 33: Galactic Equator Mula, i.e. galactic/ecliptic intersection point in the
      middle of lunar mansion Mula */
{0, 0, FALSE, 0},	             // 33: GE Mula
/*************************/
/* 34: Skydram/Galactic Alignment (R. Mardyks); autumn equinox aligned with
      Galactic Equator/Pole */
{2451079.734892000, 30, FALSE, 0},   // 34: Skydram/Mardyks
/*************************/
/* 35: Chandra Hari */
{0, 0, FALSE, 0},	             // 35: Chandra Hari 
/*************************/
/* 36: Dhruva Galactic Centre Middle of Mula (Ernst Wilhelm) */
{0, 0, FALSE, 0},	             // 36: Ernst Wilhelm
/*************************/
/* 37: Kali 3623 = 522 CE, Ujjain (75.7684565), based on Kali midnight and 
      year length of Suryasiddhanta */
{1911797.740782065, 0, TRUE, 0},     // 36: 0 ayanamsha in year 522
/*************************/
/* 38: Babylonian (Britton 2010) 
      John P. Britton, "Studies in Babylonian lunar theory: part III. The
      introduction of the uniform zodiac", in Arch. Hist. Exact. Sci.
      (2010)64:617-663, p. 630. */
{1721057.5, -3.2, TRUE, -1},         // 38: Babylonian (Britton 2010)
/*************************/
/* 39: Sunil Sheoran ("Vedic") 
      S. Sheoran, "The Science of Time and Timeline of World History", 2017. */
{0, 0, FALSE, 0},                    // 39: Sunil Sheoran ("Vedic") 
/*************************/
/* 40: Galactic Center at 0 Capricon (Cochrane) */
{0, 0, FALSE, 0},                    // 40: Cochrane 
/*************************/
/* 41: "Galactic Equatorial" (N.A. Fiorenza) */
{2451544.5, 25.0, TRUE, 0},          // 41: N.A. Fiorenza */
/*************************/
/* 42: Vettius Valens (Moon; derived from Holden 1995 p. 12 for epoch of Valens
      1 Jan. 150 CE julian) */
{1775845.5, -2.9422, TRUE, -1},      // 42: Vettius Valens
/*************************/
/* 43: Lahiri (1940), book "Panchanga darpan": 
      22°26'45".50 + 50".25748T + 0".00011115T^2 */
{J1900, 22.44597222, FALSE, SEMOD_PREC_NEWCOMB}, // 43: Lahiri (1940)
/*************************/
/* 44: Lahiri (VP285), mean sun at 360° in 285CE; epoch for mean sun at 0 acc.
      to Simon 1994, corrected for Vondrak precession 
      (Preface to Lahiri's "Indian Ephemeris" 1980) */
{1825235.2458513028, 0.0, FALSE, 0}, // 44: Lahiri VP285 (1980)
/*************************/
/* 45: Krishnamurti from mean equinox 291, based on Newcomb precession,
      according to D. Senthilathiban, "Study of KP Ayanamsa with Modern
      Precession Theories" (2019), but using precession Vondrak 2011 and
      correction base on Newcomb precession. */
//{1827424.752255678, 0.0, FALSE, SEMOD_PREC_NEWCOMB}, // 45: Krishnamurti VP291
{1827424.752255678, 0.0, FALSE, 0}, // 45: Krishnamurti VP291
/*************************/
/* 46: Lahiri original: Calendar Reform Committee 1956, 
      before the correction by 0.658" in IAE 1985.
      The subtracted value is nutation according to Woolard 1953.
      However, nutation Woolard was used by IENA/IAE only from 1960 on,
      so this value is not correct. In order to reproduce mean ayanamshas
      of IENA >=1960, we could choose 23.25 - 0.00464207 + 0.07 / 3600.0
      as initial value in 1956. However this will not help to reproduce 
      true ayanamshas. A deviation of around 0.1" remains,
      for unknown reasons. The difference between Lahiri (1) and
      Lahiri ICRC (45) amounts to 1.1". */
{2435553.5, 23.25 - 0.00464207, FALSE, SEMOD_PREC_NEWCOMB}, // 46: SE_SIDM_LAHIRI_ICRC
/*************************/
    };

#define PLAN_DATA struct plan_data

/* obliquity of ecliptic */
struct epsilon {
  double teps, eps, seps, ceps; 	/* jd, eps, sin(eps), cos(eps) */
};

/*
extern struct epsilon oec2000;
extern struct epsilon oec;
*/

struct plan_data {
  /* the following data are read from file only once, immediately after 
   * file has been opened */
  int ibdy;		/* internal body number */
  int32 iflg;		/* contains several bit flags describing the data:	
			 * SEI_FLG_HELIO: true if helio, false if bary
			 * SEI_FLG_ROTATE: TRUE if coefficients are referred 
			 *      to coordinate system of orbital plane 
			 * SEI_FLG_ELLIPSE: TRUE if reference ellipse */
  int ncoe;		/* # of coefficients of ephemeris polynomial,
			   is polynomial order + 1  */
  /* where is the segment index on the file */
  int32 lndx0;   	/* file position of begin of planet's index */
  int32 nndx;		/* number of index entries on file: computed */
  double tfstart;	/* file contains ephemeris for tfstart thru tfend */
  double tfend;         /*      for this particular planet !!!            */
  double dseg;		/* segment size (days covered by a polynomial)  */
  /* orbital elements: */
  double telem;		/* epoch of elements */
  double prot;		
  double qrot; 
  double dprot;
  double dqrot; 
  double rmax;		/* normalisation factor of cheby coefficients */
  /* in addition, if reference ellipse is used: */
  double peri;		
  double dperi; 
  double *refep;	/* pointer to cheby coeffs of reference ellipse,
			 * size of data is 2 x ncoe */
  /* unpacked segment information, only updated when a segment is read: */
  double tseg0, tseg1;	/* start and end jd of current segment */
  double *segp;         /* pointer to unpacked cheby coeffs of segment;
			 * the size is 3 x ncoe */
  int neval;		/* how many coefficients to evaluate. this may
			 * be less than ncoe */
  /* result of most recent data evaluation for this body: */
  double teval;		/* time for which previous computation was made */
  int32 iephe;            /* which ephemeris was used */
  double x[6];		/* position and speed vectors equatorial J2000 */
  int32 xflgs;		/* hel., light-time, aberr., prec. flags etc. */
  double xreturn[24];   /* return positions:
			 * xreturn+0	ecliptic polar coordinates
			 * xreturn+6	ecliptic cartesian coordinates
			 * xreturn+12	equatorial polar coordinates
			 * xreturn+18	equatorial cartesian coordinates
			 */
};

/*
 * stuff exported from swemplan.c and swemmoon.c 
 * and constants used inside these functions.
************************************************************/

#define STR             4.8481368110953599359e-6 /* radians per arc second */

/* moon, s. moshmoon.c */
extern int swi_mean_node(double jd, double *x, char *serr);
extern int swi_mean_apog(double jd, double *x, char *serr);
extern int swi_moshmoon(double tjd, AS_BOOL do_save, double *xpm, char *serr) ;
extern int swi_moshmoon2(double jd, double *x);
extern int swi_intp_apsides(double J, double *pol, int ipli);

/* planets, s. moshplan.c */
extern int swi_moshplan(double tjd, int ipli, AS_BOOL do_save, double *xpret, double *xeret, char *serr);
extern int swi_moshplan2(double J, int iplm, double *pobj);
extern int swi_osc_el_plan(double tjd, double *xp, int ipl, int ipli, double *xearth, double *xsun, char *serr);
extern FILE *swi_fopen(int ifno, char *fname, char *ephepath, char *serr);
extern int32 swi_init_swed_if_start(void);
extern int32 swi_set_tid_acc(double tjd_ut, int32 iflag, int32 denum, char *serr);
extern int32 swi_get_tid_acc(double tjd_ut, int32 iflag, int32 denum, int32 *denumret, double *tid_acc, char *serr);

extern int32 swi_get_ayanamsa_ex(double tjd_et, int32 iflag, double *daya, char *serr);
extern int32 swi_get_ayanamsa_ex_ut(double tjd_ut, int32 iflag, double *daya, char *serr);
extern int32 swi_get_ayanamsa_with_speed(double tjd_et, int32 iflag, double *daya, char *serr);

extern double swi_armc_to_mc(double armc, double eps);

extern int32 swi_get_denum(int32 ipli, int32 iflag);


/* nutation */
struct nut {
  double tnut;
  double nutlo[2];	/* nutation in longitude and obliquity */
  double snut, cnut;	/* sine and cosine of nutation in obliquity */
  double matrix[3][3];	
};

struct plantbl {
  char max_harmonic[9];
  char max_power_of_t;
  signed char *arg_tbl;
  double *lon_tbl;
  double *lat_tbl;
  double *rad_tbl;
  double distance;
};

struct file_data {
  char fnam[AS_MAXCH];	/* ephemeris file name */
  int fversion;		/* version number of file */
  char astnam[50];	/* asteroid name, if asteroid file */ 
  int32 sweph_denum;     /* DE number of JPL ephemeris, which this file
			 * is derived from. */
  FILE *fptr;		/* ephemeris file pointer */
  double tfstart;       /* file may be used from this date */
  double tfend;         /*      through this date          */
  int32 iflg; 		/* byte reorder flag and little/bigendian flag */
  short npl;		/* how many planets in file */
  int ipl[SEI_FILE_NMAXPLAN];	/* planet numbers */
};
 
struct gen_const {
 double clight, 
	aunit, 
	helgravconst, 
	ratme, 
	sunradius;
};

struct save_positions {
  int ipl;
  double tsave;
  int32 iflgsave;
  /* position at t = tsave,
   * in ecliptic polar (offset 0),
   *    ecliptic cartesian (offset 6), 
   *    equatorial polar (offset 12),
   *    and equatorial cartesian coordinates (offset 18).
   * 6 doubles each for position and speed coordinates.
   */
  double xsaves[24];    
};

struct node_data {
  /* result of most recent data evaluation for this body: */
  double teval;		/* time for which last computation was made */
  int32 iephe;            /* which ephemeris was used */
  double x[6];		/* position and speed vectors equatorial J2000 */
  int32 xflgs;		/* hel., light-time, aberr., prec. flags etc. */
  double xreturn[24];   /* return positions: 
			 * xreturn+0	ecliptic polar coordinates
			 * xreturn+6	ecliptic cartesian coordinates
			 * xreturn+12	equatorial polar coordinates
			 * xreturn+18	equatorial cartesian coordinates
			 */
};

struct topo_data {
  double geolon, geolat, geoalt;
  double teval;
  double tjd_ut;
  double xobs[6];
};

struct sid_data {
  int32 sid_mode;
  double ayan_t0;
  double t0;
  AS_BOOL t0_is_UT;
};

#define SWI_STAR_LENGTH 40
struct fixed_star {
  char skey[SWI_STAR_LENGTH + 2]; // may be prefixed with comma, one char more
  char starname[SWI_STAR_LENGTH + 1];
  char starbayer[SWI_STAR_LENGTH + 1];
  char starno[10];
  double epoch, ra, de, ramot, demot, radvel, parall, mag;
};

/* dpsi and deps loaded for 100 years after 1962 */
#define SWE_DATA_DPSI_DEPS  36525   

struct interpol {
  double tjd_nut0, tjd_nut2;
  double nut_dpsi0, nut_dpsi1, nut_dpsi2;
  double nut_deps0, nut_deps1, nut_deps2;
};

/* if this is changed, then also update initialisation in sweph.c */
struct swe_data {
  AS_BOOL ephe_path_is_set;
  AS_BOOL jpl_file_is_open;
  FILE *fixfp;		/* fixed stars file pointer */
  char ephepath[AS_MAXCH];
  char jplfnam[AS_MAXCH];
  int32 jpldenum;
  int32 last_epheflag;
  AS_BOOL geopos_is_set;
  AS_BOOL ayana_is_set;
  AS_BOOL is_old_starfile;
  double eop_tjd_beg;
  double eop_tjd_beg_horizons;
  double eop_tjd_end;
  double eop_tjd_end_add;
  int eop_dpsi_loaded;
  double tid_acc;
  AS_BOOL is_tid_acc_manual;
  AS_BOOL init_dt_done;
  AS_BOOL swed_is_initialised;
  AS_BOOL delta_t_userdef_is_set;
  double delta_t_userdef;
  double ast_G;
  double ast_H;
  double ast_diam;
  char astelem[AS_MAXCH * 10];
  int i_saved_planet_name;
  char saved_planet_name[80];
  //double dpsi[36525];  /* works for 100 years after 1962 */
  //double deps[36525];
  double *dpsi;
  double *deps;
  int32 timeout;
  int32 astro_models[SEI_NMODELS];
  AS_BOOL do_interpolate_nut;
  struct interpol interpol;
  struct file_data fidat[SEI_NEPHFILES];
  struct gen_const gcdat;
  struct plan_data pldat[SEI_NPLANETS];
#if 0
  struct node_data nddat[SEI_NNODE_ETC];
#else
  struct plan_data nddat[SEI_NNODE_ETC];
#endif
  struct save_positions savedat[SE_NPLANETS+1];
  struct epsilon oec;
  struct epsilon oec2000;
  struct nut nut;
  struct nut nut2000;
  struct nut nutv;
  struct topo_data topd;
  struct sid_data sidd;
  AS_BOOL n_fixstars_real;   // real number of fixed stars in sefstars.txt
  AS_BOOL n_fixstars_named;  // number of fixed stars with tradtional name
  AS_BOOL n_fixstars_records;// number of fixed stars records in fixed_stars
  struct fixed_star *fixed_stars;
};

extern TLS struct swe_data swed;
