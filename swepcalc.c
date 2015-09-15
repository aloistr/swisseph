/*****************************************************
$Header: /home/dieter/sweph/RCS/swepcalc.c,v 1.74 2008/06/16 10:07:20 dieter Exp $
Placalc compatibility interface for Swiss Ephemeris.

*******************************************************/
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

/*
 * This file is the PLACALC compatibility interface for Swiss Ephemeris.
 * It allows very easy porting of older Placalc application to the SwissEph.
 * A user has to replace #include "placalc.h" and "housasp.h" with
 * #include "swepcalc.h"
 * If he has used "ourdef.h" he replaces it with "sweodef.h".
 * Then he links his application with swepcalc.o and runs it against the
 * Swiss Ephemeris DLL or linkable library.
 *
 * All calls which were present in the placalc sources are contained 
 * here, and either implemented directly or translated into Swiss Ephemeris
 * calls.
 *
 * 
 */

#include "swepcalc.h"
#include "swephexp.h"

/************************************************************
local globals, not exported 
************************************************************/

static const int plac2swe[] = {SE_SUN, SE_MOON, SE_MERCURY, SE_VENUS, SE_MARS, SE_JUPITER, SE_SATURN, SE_URANUS, SE_NEPTUNE, SE_PLUTO, SE_MEAN_NODE, SE_TRUE_NODE, SE_CHIRON, SE_MEAN_APOG, SE_CERES, SE_PALLAS, SE_JUNO, SE_VESTA,};

/* If there occurs an internal error in placalc, a message is 
 * written into the string variable perrtx.
 * The message can be read with placalc_get_errtext();
 */
static TLS char perrtx[AS_MAXCH];
static double ekl, nut;

/*
 * mimimum and maximum distances computed over 1000 years with plamimax,
 * required for relative distances rgeo, where the distance is given
 * as 100 when a planet is closest and as 0 when farthest from earth.
 */
static const double rmima[CALC_N][2] = {	
	{ 0.98296342,  1.01704665},
	{ 0.00238267,  0.00271861},
	{ 0.54900496,  1.45169607},
	{ 0.26411287,  1.73597885},
	{ 0.37289847,  2.67626927},
	{ 3.94877993,  6.45627627},
	{ 7.99362824, 11.09276636},
	{17.28622633, 21.10714104},
	{28.81374786, 31.33507284},
	{28.67716748, 50.29208774},
	{ 0.00,	 0.00259553},	/* nodes don't get a real value*/
	{ 0.00,  0.00259553},
	{ 7.36277475, 19.86585062}};

/**********************************************************
function nacalc	()
calculates an array of planet longitudes and speeds,
as needed for complete nathan data records.
The function knows itself how many planets and in which mode
they have to be calculated for Nathan. 

return OK or ERR

The returned positions are in centiseconds, our standard
coordinate format for fast mathematics with planetary positions.

This function is just a template of how the calc() package
can be used. 
**********************************************************/
int nacalc (double	jd_ad,	/* universal time relative  julian date */
	    centisec *plon,	/* returned longitudes */
	    centisec *pspe	/* returned speeds, if not NULL pointer */
	   )
{
  char err[AS_MAXCH];
  int planet, flag;
  double rlng, rrad, rlat, rspeed;
  int result = OK;
  flag = CALC_BIT_SPEED;	/* same, with speed */
  jd_ad += deltat( jd_ad );	/* ET = UT + Delta_T */
  for (planet = SUN; planet <= MAXPL_NACALC; planet++) {
    if (calc (planet, jd_ad, flag, &rlng, &rrad, &rlat, &rspeed) == OK) {
      plon [planet] = swe_csnorm(swe_d2l (rlng * DEG));
      if (pspe != NULL) pspe [planet] = swe_d2l (rspeed * DEG);
    } else {
      plon [planet] = -1;
      if (pspe != NULL) pspe [planet] = 0;
      if (result != ERR) {	/* save first error message */
	strcpy(err, placalc_get_errtext());
      }
      result = ERR;
    }
  }
  if (result == ERR)
    strcpy(perrtx, err);
  return result;
}	/* end nacalc */

/******************************************************************
 * calculation server
 * delivers positions in string format which can be sent easily
 * over a communication line to the calculation client.
 * if plalist = 0, only SUN .. CHIRON are delivered, no LILITH
 ******************************************************************/
int calcserv(int id,	/* request id, random number to prevent phase err */
	    double jd_ad,	/* time as relative Astrodienst julian date */
	    int	flag,	/* a set of CALC_BIT_ bitflags */
	    int  plalist,/* bit list of planets to be computed, 0 = all */
	    char *so)	/* output string, MUST BE LONG ENOUGH (800 bytes)*/
{
  int p, planet, so_len;
  double rlng, rrad, rlat, rspeed, rau[CALC_N];
  centisec lcs[CALC_N], lpcs[CALC_N], betcs[CALC_N]; 
  char s[AS_MAXCH];
  if (plalist == 0) plalist = (1 << 13) - 1;	/* sun .. chiron */;
  /*
   * flag determines whether deltat is added to t;
   * if CALC_BIT_EPHE is set, jd_ad is considered as ephemeris time,
   * otherwise as universal time.
   */
  if ((flag & CALC_BIT_EPHE) == 0) {
    jd_ad += deltat (jd_ad);
  }
  for (p = SUN; p < CALC_N; p++) {
    if (! check_bit(plalist, p)) continue;
    if (calc (p, jd_ad, flag, &rlng, &rrad, &rlat, &rspeed) == OK) {
      lcs [p] = swe_d2l (rlng * DEG);
      lpcs [p] = swe_d2l (rspeed * DEG);
      betcs [p] = swe_d2l (rlat * DEG);
      rau [p] = rrad;
    } else {
      sprintf(so,"error at planet %d", p);
      return ( ERR);
    }
  }
  /*
   * format comma separated list: id,teph,flag,plalist,ekl,nut
   * double is given with 8 digits precision after decimal point, 
   * all angles are given in centiseconds.
   * then for each requested planet: longitude (csec)
   * then for each requested planet, if wanted: speed (csec/day)
   * then for each requested planet, if wanted: latitude (csec)
   * then for each requested planet, if wanted: rgeo (units 0..999)
   * then for each requested planet, if wanted: rau  (A.U.)
   */
  sprintf (so, "%d,%.8f,%d,%d,%d,%d", id, jd_ad, flag, plalist, 
	       swe_d2l(ekl * DEG), swe_d2l (nut * DEG) );
  so_len = strlen (so);
  for (planet = SUN; planet < CALC_N; planet++) {
    if (! check_bit(plalist, planet)) continue;
    sprintf (s ,",%d", lcs[planet]);
    strcat (so + so_len, s);
    so_len += strlen (s);
  }
  if (flag & CALC_BIT_SPEED) {
    for (planet = SUN; planet < CALC_N; planet++)  {
      if (! check_bit(plalist, planet)) continue;
      sprintf (s ,",%d", lpcs[planet]);
      strcat (so + so_len, s);
      so_len += strlen (s);
    }
  }
  if (flag & CALC_BIT_BETA) {
    for (planet = SUN; planet < CALC_N; planet++)  {
      if (! check_bit(plalist, planet)) continue;
      sprintf (s ,",%d", betcs[planet]);
      strcat (so + so_len, s);
      so_len += strlen (s);
    }
  }
  if (flag & CALC_BIT_RGEO) {
    for (planet = SUN; planet < CALC_N; planet++)  {
      if (! check_bit(plalist, planet)) continue;
      sprintf (s ,",%d", rel_geo(planet,rau[planet]));
      strcat (so + so_len, s);
      so_len += strlen (s);
    }
  }
  if (flag & CALC_BIT_RAU) {
    for (planet = SUN; planet < CALC_N; planet++)  {
      if (! check_bit(plalist, planet)) continue;
      sprintf (s ,",%.8f", rau[planet]);
      strcat (so + so_len, s);
      so_len += strlen (s);
    }
  }
  return (OK);
}	/* end calcserv */

/******************************************************************
   function calc(): 
   This is the main routine for computing a planets position.
   The function has several modes, which are controlled by bits in
   the parameter 'flag'. The normal mode (flag == 0) computes
   a planets apparent geocentric position in ecliptic coordinates relative to
   the true equinox of date, without speed

   Explanation of the arguments: see the functions header.

   Returns OK or ERR (if some planet out of time range). OK and ERR are
   defined in ourdef.h and must not be confused with TRUE and FALSE.
   OK and ERR are of type int, not of type AS_BOOL.

   Bits used in flag:
   CALC_BIT_HELIO  		0 = geocentric, 1 = heliocentric
   CALC_BIT_NOAPP 	   	0 = apparent positions, 1 = true positions
   CALC_BIT_NONUT 		0 = do nutation (true equinox of date)
				1 = don't do nutation (mean equinox of date).

   CALC_BIT_SPEED 		0 = don't calc speed,
				1 = calc speed

   Time range:
   The function can be used savely in the time range 3000 BC to
   3000 AD. 

   Getting ecliptic and nutation:
   calc(CALC_ONLY_ECL_NUT,teph,0,&nutv,&meaneklv,&eklv,NULL);
   will return the values for time teph.

******************************************************************/
int calc(int  planet,  	/* planet index as defined in placalc.h,
			 SUN = 0, MOON = 1 etc.
			 planet == -1 calc calculates only nut, ekl, meanekl */
	 double jd_ad,	/* relative Astrodienst Juldate, ephemeris time.
			 Astrodienst Juldate is relative 31 Dec 1949, noon. */
	 int  flag,	/* See definition of flag bits above */
	 double *alng,
	 double *arad,
	 double *alat,
	 double *alngspeed)
      /* pointers to the return variables:
	 alng = ecliptic longitude in degrees
	 arad = radius vector in AU (astronomic units)
	 alat = ecliptic latitude in degrees
	 alngspeed = speed of planet in degrees per day
       */
{
  double tjd = jd_ad + JUL_OFFSET;
  double x[6];
  int32 iflagret = 0, iflag = 0;
  int ipl;
  /* planet number 
   */
  /* ecliptic and nutation */
  if (planet == CALC_ONLY_ECL_NUT)
    ipl = SE_ECL_NUT;
  /* earth: placalc makes no difference between sun and earth, 
   *        swisseph does */
  else if (planet == SUN && (flag & CALC_BIT_HELIO))
    ipl = SE_EARTH;	
  else if (planet >= SUN && planet <= VESTA)
    ipl = plac2swe[planet];
  else {
    sprintf(perrtx, "invalid planet number %d. ", planet);
    return ERR;
  }
  /* flag */
  if (flag & CALC_BIT_HELIO)
    if (ipl != SE_MEAN_NODE && ipl != SE_TRUE_NODE && ipl != SE_MEAN_APOG)
      iflag |= SEFLG_HELCTR; /* lunar node and apogee is always geocentric */
  if (flag & CALC_BIT_NOAPP)
    iflag |= SEFLG_TRUEPOS;
  if (flag & CALC_BIT_NONUT)
    iflag |= SEFLG_NONUT;
  if (flag & CALC_BIT_SPEED)
    iflag |= SEFLG_SPEED;
  /* ecliptic and nutation */
  if ((iflagret = swe_calc(tjd, ipl, iflag, x, perrtx)) == ERR)
    return iflagret;
  if (ipl == SE_ECL_NUT) {
    *alng = nut = x[2];
    *arad = x[1];
    *alat = ekl = x[0];
  } else {
    *alng = x[0];
    *arad = x[2];
    *alat = x[1];
    *alngspeed = x[3];
  }
  return (OK);
} /* end calc */

int rel_geo(int planet, double rau)
{
  /*
   * get relative earth distance in range 0..1000:
   * To compute the relative distance we use fixed values of 
   * mimimum and maximum distance measured empirically between
   * 1300 AD and 2300 AD (see helconst.c). 
   * This approach is certainly fine for the
   * outer planets, but not the best for Sun and Moon, where it
   * would be better to look at the mean anomaly, i.e. the progress
   * the planet makes on it's Kepler orbit.
   * Considering the low importance astrologers give to the relative
   * distance, we consider the effort not worthwile.
   * Now we compare real radius with longtime-averaged distances.
   */
  int rgeo;
  if (planet == MEAN_NODE || planet == TRUE_NODE || planet == LILITH) {
    return 0;
  } else {
    rgeo = 1000 * (1.0 - (rau - rmima[planet][0]) / (rmima[planet][1] - rmima[planet][0]));
  }
  if (rgeo < 0)
    rgeo = 0;
  else if (rgeo > 999)
    rgeo = 999;
  return rgeo;
}

/*
 * get the planet index for an AFL letter
 * returns -1 if the letter does not correspond to a planet.
 */
int afl2planet(int afl)
{
  int p;
  switch (afl) {
    case AFL_SUN : p = SUN; break;
    case AFL_MON : p = MOON; break;
    case AFL_MER : p = MERCURY; break;
    case AFL_VEN : p = VENUS; break;
    case AFL_MAR : p = MARS; break;
    case AFL_JUP : p = JUPITER; break;
    case AFL_SAT : p = SATURN; break;
    case AFL_URA : p = URANUS; break;
    case AFL_NEP : p = NEPTUNE; break;
    case AFL_PLU : p = PLUTO; break;
    case AFL_MNODE :  p = MEAN_NODE; break;
    case AFL_TNODE :  p = TRUE_NODE; break;
    case AFL_CHI : p = CHIRON; break;
    case AFL_LIL : p = LILITH; break;
    case AFL_CER  : p = CERES; break;
    case AFL_PAL  : p = PALLAS; break;
    case AFL_JUN  : p = JUNO; break;
    case AFL_VES  : p = VESTA; break;
    case AFL_AC  : p = AC; break;
    case AFL_MC  : p = MC; break;
    default : p = -1; break;
  }  
  return p;
}

/*
 * get the AFL letter for a planet
 * returns -1 if no letter corresponds to a planet.
 */
int planet2afl(int planet)
{
  switch (planet) {
    case SUN:		return AFL_SUN;
    case MOON:		return AFL_MON;
    case MERCURY:	return AFL_MER;
    case VENUS:		return AFL_VEN;
    case MARS:		return AFL_MAR;
    case JUPITER:	return AFL_JUP;
    case SATURN:	return AFL_SAT;
    case URANUS:	return AFL_URA;
    case NEPTUNE:	return AFL_NEP;
    case PLUTO:		return AFL_PLU;
    case MEAN_NODE:	return AFL_MNODE;
    case TRUE_NODE:	return AFL_TNODE;
    case CHIRON:	return AFL_CHI;
    case LILITH:	return AFL_LIL;
    case CERES:		return AFL_CER;
    case PALLAS:	return AFL_PAL;
    case JUNO:		return AFL_JUN;
    case VESTA:		return AFL_VES;
    case AC:		return AFL_AC;
    case MC:		return AFL_MC;
  }  
  if (planet >= FIRST_HSNR && planet <= LAST_HSNR)
    return AFL_HOUSE;
  else
    return -1;
}

/*
 * get the 2-letter abbreviation for a planet
 * returns ?? if not defined
 */
char *planet2abbr2(int planet)
{
  switch (planet) {
    case SUN:		return "su";
    case MOON:		return "mo";
    case MERCURY:	return "me";
    case VENUS:		return "ve";
    case MARS:		return "ma";
    case JUPITER:	return "ju";
    case SATURN:	return "sa";
    case URANUS:	return "ur";
    case NEPTUNE:	return "ne";
    case PLUTO:		return "pl";
    case MEAN_NODE:	return "mn";
    case TRUE_NODE:	return "tn";
    case CHIRON:	return "ch";
    case LILITH:	return "li";
    case CERES:		return "ce";
    case PALLAS:	return "pa";
    case JUNO:		return "jn";
    case VESTA:		return "vs";
    case AC:		return "ac";
    case MC:		return "mc";
  }  
  return "??";
}

/*
 * get the 3-letter abbreviation for a planet
 * returns ??? if not defined
 */
char *planet2abbr3(int planet)
{
  switch (planet) {
    case SUN:		return "sun";
    case MOON:		return "mon";
    case MERCURY:	return "mer";
    case VENUS:		return "ven";
    case MARS:		return "mar";
    case JUPITER:	return "jup";
    case SATURN:	return "sat";
    case URANUS:	return "ura";
    case NEPTUNE:	return "nep";
    case PLUTO:		return "plu";
    case MEAN_NODE:	return "mno";
    case TRUE_NODE:	return "tno";
    case CHIRON:	return "chi";
    case LILITH:	return "lil";
    case CERES:		return "cer";
    case PALLAS:	return "pal";
    case JUNO:		return "jun";
    case VESTA:		return "ves";
    case AC:		return "asc";
    case MC:		return "mc ";
  }  
  return "???";
}

char *placalc_set_ephepath(char *path)
{
  static TLS char *epath;
  if (path == NULL) return epath;
  if (epath != NULL)
    free((void *) epath);
  epath =  malloc(strlen(path) + 1);
  if (epath != NULL) {
    strcpy(epath, path);
    swe_set_ephe_path(epath);
  }
  return epath;
}

void placalc_close_files()
{
  swe_close();
}

char *placalc_get_errtext()
{
  return perrtx;
}

/*****************************************************
deltat(t): returns delta t (in julian days) from universal time t
is included by users
ET = UT +  deltat
******************************************************/
double deltat (double jd_ad) /* Astrodienst relative julian date */
{ 
  return swe_deltat(jd_ad + JUL_OFFSET);
}

/**********************************************************
 * get fixstar positions
 * parameters:
 * star
 *    if string star contains a name, this star is searched.
 *    if it contains a number, the n'th star of the file
 *    is returned, starting with 0.
 *    In any case the name of the star is returned.
 * jd absolute julian day
 * lon, lat	pointer for returning the ecliptic coordinates
 *              (mean ecliptic and equinox of date)
**********************************************************/
int fixstar(char *star, double jd, double *lon, double *lat)
{
  double x[6];
  int i;
  int32 retflag;
  /* if call by number, fixstar() is 0-based, 
   * whereas swe_fixstar starts with 1 */
  if (isdigit((int) *star)) {
    i = atoi(star);
    sprintf(star, "%d", i+1);
  }
  retflag = swe_fixstar(star, jd, 0, x, perrtx);
  *lon = x[0];
  *lat = x[1];
  return((int) retflag);
}

/******************************************************************/
double diff8360 (double a, double b) 
     /* a - b on a 360 degree circle, result -180..180*/
{
  double d;
  d = a - b;
  if ( d >= 180.0 ) return( d - 360.0 );
  if ( d < -180.0 ) return( d + 360.0 );
  return( d );
}    /* diff8360 */

/*
 * originally in swephous.c
 */

/*************************************
return in which house pp is;
houses are numbered from 1 .. 12
*************************************/ 
int HouseNr(struct houses *hsp, CSEC pp)
{
  CSEC cx;
  int i = 2;
  cx = swe_difcsn(pp, hsp->cusp [1]); 	/* distance from cusp 1 */
  while (i < 13 && cx >= difcsn(hsp->cusp [i], hsp->cusp [1])) i++;
  return (i - 1);
} 

/************************************
returns the inp-house number, where pp is in 
houses are numbered from 1 .. 12
************************************/
int InpHouseNr (struct houses *hsp, CSEC pp, CSEC *coff)
{ 
  CSEC cx;
  int i = 2;
  cx = swe_difcsn(pp, hsp->cusp [1] + coff [1]); 
  while(i<13 &&  cx >= swe_difcsn(hsp->cusp[i] + coff[i], hsp->cusp[1] + coff[1])) 
    i++;
  return (i - 1);
}

/* variation of InpHouseNr(). Able to handle house pre-orbs that are
 * proportional to house size.
 * value 1 in doff[0] means that the offset is proportional to house size,
 * e.g. doff[ihs] = -5 means here:
 * doff[ihs] = -5 / 30 * preceding_house_size; 
 * We first calculate the absolute offsets for each house of our birth chart, 
 * then call the function InpHouseNr() with those values.
 */
int InpHouseNr2 (struct houses *hsp, CSEC pp, CSEC *coff)
{
  int i, j;
  CSEC myoff[13];
  for (i = 0; i < 13; i++)
    myoff[i] = coff[i];
  if (myoff[0] == 1) {
    for (i = 1; i < 13; i++) {
      j = i + 1;
      if (j > 12) j = 1;
      myoff[j] = swe_degnorm((hsp->cusp[j] - hsp->cusp[i]) / 360000.0) / 30.0 * myoff[j];
    }
  }
  return InpHouseNr(hsp, pp, myoff);
}

/* ********************************************************* */
/*  Houses:						     */
/* ********************************************************* */
/*  Koch and Placidus don't work in the polar circle.        */
/*  We swap MC/IC so that MC is always before AC in the zodiac */
/*  We than divide the quadrants into 3 equal parts.         */
/* ********************************************************* */
/*  All angles are expressed in centiseconds (1/100th of a   */
/*  second of arc) and integer arithmetic is used for these. */
/*  Special trigonometric functions dsin, dcos etc. are im-  */
/*  plemented for arguments in centiseconds.                 */
/* ********************************************************* */
/*  Arguments: th = sidereal time (angle 0..360 degrees      */
/*             hsy = letter code for house system; implemen- */
/*                   ted are P,K,C,R,E,V.                    */
/*             fi = geographic latitude                      */
/*             ekl = obliquity of the ecliptic               */
/*             iteration_count = number of iterations in     */
/*             Placidus calculation; can be 1 or 2.          */
/* ********************************************************* */
void CalcHouses(CSEC th, CSEC fi, CSEC mekl, char hsy, int iteration_count,
	struct houses *hsp )
{
  int retc = 0, i;
  double cs2deg = 360000;
  double cusps[13];
  double ascmc[10];
  /* iteration_count is always 2 */
  retc = swe_houses_armc(th / cs2deg, fi / cs2deg, mekl / cs2deg, (int) hsy, 
			cusps, ascmc);
  for (i = 0; i < 13; i++)
    hsp->cusp[i] = swe_d2l(cusps[i] * cs2deg);
  hsp->ac = swe_d2l(ascmc[0] * cs2deg);
  hsp->mc = swe_d2l(ascmc[1] * cs2deg);
  /*
   * this is just to shut up lint
   */
  retc += iteration_count;
  iteration_count = retc;
} /* procedure houses */

/******************************/
void RecalcAspects(struct AspectType *a)
{
  centisec diff,orbis;
  int p1, p2, i;
  struct aspRec *arp;
  if (a->ppos2 == NULL) {	/* no set ppos2, no interaspects */
    for (p1 = 0; p1 < a->NrOfPlanets; p1++) {
      a->Asp[p1][p1].index = 0;		/* ignore p1 conjunct p1 */
      for (p2 = p1 + 1; p2 < a->NrOfPlanets; p2++) {
	arp = &(a->Asp[p1][p2]);
	diff =  a->PlanetPos [p2] - a->PlanetPos [p1];
	if (diff >= DEG180) 
	  diff -= DEG360;
	else if (diff <  - DEG180) 
	  diff += DEG360;
	i = 1;                  
	arp->index = 0;
	while (i <= a->NrOfAspects) {
	  orbis = ABS4 (diff) - a->Angle [i];
	  if (ABS4 (orbis) <= a->Maxorb [i]) {
	    arp->index = i;
	    arp->orb = orbis;
	    break; 	/* out of while */
	  }
	  i++;
	}
	a->Asp [p2][p1].index = arp->index;
	a->Asp [p2][p1].orb = arp->orb;
      } /* for p2 */
    } /* for p1 */ 
  } else {	/* interaspects between set 1 and set 2 */
    for (p1 = 0; p1 < a->NrOfPlanets; p1++) {
      for (p2 = 0; p2 < a->NrOfPlanets; p2++) {
	arp = &(a->Asp[p1][p2]);
	diff =  a->ppos2 [p2] - a->PlanetPos [p1];
	if (diff >= DEG180) 
	  diff -= DEG360;
	else if (diff <  - DEG180) 
	  diff += DEG360;
	i = 1;                  
	arp->index = 0;
	while (i <= a->NrOfAspects) {
	  orbis = ABS4 (diff) - a->Angle [i];
	  if (ABS4 (orbis) <= a->Maxorb [i]) {
	    arp->index = i;
	    arp->orb = orbis;
	    break; 	/* out of while */
	  }
	  i++;
	}
      } /* for p2 */
    } /* for p1 */ 
  } 	/* else */
  a->dataValid = TRUE;
} 

/***********************************************************
function sidtime (t): returns sidereal time at greenwich;
Parameters differ from ASYS version! after AESuppl. 1961, page 75
version 24-oct-87
***********************************************************/
double  sidtime (double jd_ad, double ecl, double nuta)
	/* jd_ad relative julian date */
        /* ecl, nuta  ecliptic and nutation of date, in degrees */
{
  return swe_sidtime0(jd_ad + JUL_OFFSET, ecl, nuta);
}

# ifdef INTEL_BYTE_ORDER
/********************************************************************/
void longreorder (UCHAR *p, int n) 
			   /* p points to memory filled with int32 values; for
                           each of the values the seqeuence of the four bytes
                           has to be reversed, to translate HP-UX and VAX
			   ordering to MSDOS/Turboc ordering */
{
  int i;
  unsigned char c0, c1, c2, c3;
  for (i = 0; i < n; i += 4, p += 4) {
    c0 = *p;
    c1 = *(p + 1);
    c2 = *(p + 2);
    c3 = *(p + 3);
    *p = c3;
    *(p + 1) = c2;
    *(p + 2) = c1;
    *(p + 3) = c0;
  }
}
# endif
