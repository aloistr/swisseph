/* SWISSEPH 
   $Header: swecl.c,v 1.1 98/12/02 19:17:53 dieter Exp $

    Ephemeris computations
    Author: Dieter Koch
*/
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

#include "swejpl.h"
#include "swephexp.h"
#include "sweph.h"
#include "swephlib.h"

#define SEFLG_EPHMASK	(SEFLG_JPLEPH|SEFLG_SWIEPH|SEFLG_MOSEPH)
static int find_maximum(double y00, double y11, double y2, double dx, 
			double *dxret, double *yret);
static int find_zero(double y00, double y11, double y2, double dx, 
			double *dxret, double *dxret2);
static double refrac2(double trualt, double atpress, double attemp);

#if 0
#define DSUN 	(1391978489.9 / AUNIT)	/* this value is consistent with
					 * 959.63 arcsec at AU distance (Astr. Alm.) */
#else
#define DSUN 	(1392000000.0 / AUNIT)
#endif
#define DMOON 	(3476300.0 / AUNIT)
#define DEARTH  (6378140.0 * 2 / AUNIT)
#define RSUN	(DSUN / 2)
#define RMOON	(DMOON / 2)
#define REARTH	(DEARTH / 2)
static int32 eclipse_where( double tjd_ut, int32 ifl, double *geopos, 
	double *dcore, char *serr);
static int32 eclipse_how( double tjd_ut, int32 ifl,
        double geolon, double geolat, double geohgt, 
	double *attr, char *serr);
static int32 eclipse_when_loc(double tjd_start, int32 ifl, double *geopos, 
	double *tret, double *attr, AS_BOOL backward, char *serr);
static int32 lun_eclipse_how(double tjd_ut, int32 ifl, double *attr, 
        double *dcore, char *serr);
static int32 calc_mer_trans(
               double tjd_ut, int32 ipl, int32 epheflag, int32 rsmi,
               double *geopos,
               char *starname, 
               double *tret,
               char *serr);

/* Computes geographic location and type of solar eclipse 
 * for a given tjd 
 * iflag:	to indicate ephemeris to be used
 *			(SEFLG_JPLEPH, SEFLG_SWIEPH, SEFLG_MOSEPH)
 *
 * Algorithms for the central line is taken from Montenbruck, pp. 179ff.,
 * with the exception, that we consider refraction for the maxima of
 * partial and noncentral eclipses.
 * Geographical positions are referred to sea level / the mean ellipsoid.
 *
 * Errors:
 * - from uncertainty of JPL-ephemerides (0.01 arcsec): 
 *	about 40 meters
 * - from displacement of shadow points by atmospheric refraction:
 *      a few meters 
 * - from deviation of the geoid from the ellipsoid 
 *      a few meters
 * - from polar motion
 *      a few meters
 * For geographical locations that are interesting for observation,
 * the error is always < 100 m.
 * However, if the sun is close to the horizon,
 * all of these errors can grow up to a km or more. 
 * 
 * Function returns:
 * -1 (ERR)	on error (e.g. if swe_calc() for sun or moon fails)
 * 0		if there is no solar eclipse at tjd
 * SE_ECL_TOTAL
 * SE_ECL_ANNULAR
 * SE_ECL_TOTAL | SE_ECL_CENTRAL
 * SE_ECL_TOTAL | SE_ECL_NONCENTRAL
 * SE_ECL_ANNULAR | SE_ECL_CENTRAL
 * SE_ECL_ANNULAR | SE_ECL_NONCENTRAL
 * SE_ECL_PARTIAL
 * 
 * geopos[0]:	geographic longitude of central line
 * geopos[1]:	geographic latitude of central line
 *
 * not implemented so far:
 *
 * geopos[2]:	geographic longitude of northern limit of umbra
 * geopos[3]:	geographic latitude of northern limit of umbra
 * geopos[4]:	geographic longitude of southern limit of umbra
 * geopos[5]:	geographic latitude of southern limit of umbra
 * geopos[6]:	geographic longitude of northern limit of penumbra
 * geopos[7]:	geographic latitude of northern limit of penumbra
 * geopos[8]:	geographic longitude of southern limit of penumbra
 * geopos[9]:	geographic latitude of southern limit of penumbra
 *
 * Attention: "northern" and "southern" limits of umbra do not 
 * necessarily correspond to the northernmost or southernmost 
 * geographic position, where the total, annular, or partial
 * phase is visible at a given time. 
 * Imagine a situation in northern summer, when the sun illuminates 
 * the northern polar circle. The southernmost point of the core 
 * shadow may then touch the north pole, and therefore the 
 * northernmost point will be more in the south.
 * Note also that with annular eclipses, the northern edge is 
 * usually geographically the southern one. With annular-total 
 * ones, the two lines cross, usually twice. The maximum is always
 * total in such cases. 
 *
 * attr[0]	fraction of solar diameter covered by moon (magnitude)
 * attr[1]	ratio of lunar diameter to solar one
 * attr[2]	fraction of solar disc covered by moon (obscuration)
 * attr[3]      diameter of core shadow in km
 * attr[4]	azimuth of sun at tjd
 * attr[5]	true altitude of sun above horizon at tjd
 * attr[6]	apparent altitude of sun above horizon at tjd
 * attr[7]	angular distance of moon from sun in degrees
 *         declare as attr[20] at least !
 */
int32 FAR PASCAL_CONV swe_sol_eclipse_where(
		double tjd_ut, 
                int32 ifl,
		double *geopos, 
		double *attr, 
		char *serr) 
{
  int32 retflag, retflag2;
  double dcore[10];
  if ((retflag = eclipse_where(tjd_ut, ifl, geopos, dcore, serr)) < 0)
    return retflag;
  if ((retflag2 = eclipse_how(tjd_ut, ifl, geopos[0], geopos[1], 0, attr, serr)) == ERR)
    return retflag2;
  attr[3] = dcore[0];
  return retflag;
}

/* Used by several swe_sol_eclipse_ functions.
 * Like swe_sol_eclipse_where(), but instead of attr[0], it returns:
 *
 * dcore[0]:	core shadow width in km
 * dcore[2]:	distance of shadow axis from geocenter r0
 * dcore[3]:	diameter of core shadow on fundamental plane d0
 * dcore[4]:	diameter of half-shadow on fundamental plane D0
 */
static int32 eclipse_where( double tjd_ut, int32 ifl, double *geopos, double *dcore, 
		char *serr)
{
  int i;
  int32 retc = 0;
  double e[6], et[6], erm[6], rm[6], rs[6], rmt[6], rst[6], xs[6], xst[6];
  double xssv[16], x[6];
  double lm[6], ls[6], lx[6];
  double dsm, dsmt, d0, D0, s0, r0, d, s, dm;
  double de = 6378140.0 / AUNIT;
  double earthobl = 1 - EARTH_OBLATENESS;
  double deltat, tjd, sidt;
  double sinf1, sinf2, cosf1, cosf2;
  int32 iflag, iflag2;
  double ecce = sqrt(2 * EARTH_OBLATENESS - EARTH_OBLATENESS * EARTH_OBLATENESS);
  AS_BOOL no_eclipse = FALSE;
  struct epsilon *oe = &swed.oec;
  for (i = 0; i < 10; i++) 
    dcore[i] = 0;
  /* nutation need not be in lunar and solar positions,
   * if mean sidereal time will be used */
  iflag = SEFLG_SPEED | SEFLG_EQUATORIAL | ifl;
  iflag2 = iflag | SEFLG_RADIANS;
  iflag  = iflag | SEFLG_XYZ;
  deltat = swe_deltat(tjd_ut);
  tjd = tjd_ut + deltat;
  /* moon in cartesian coordiantes */
  if (swe_calc(tjd, SE_MOON, iflag, rm, serr) == ERR)
    return ERR;
  /* moon in polar coordinates */
  if (swe_calc(tjd, SE_MOON, iflag2, lm, serr) == ERR)
    return ERR;
  if (iflag & SEFLG_NONUT)
    sidt = swe_sidtime0(tjd_ut, oe->eps * RADTODEG, 0) * 15 * DEGTORAD;
  else
    sidt = swe_sidtime(tjd_ut) * 15 * DEGTORAD;
  /* save moon position */
  for (i = 0; i <= 2; i++)
    rmt[i] = rm[i];
  /* Account for oblateness of earth:
   * Instead of flattening the earth, we apply the 
   * correction to the z coordinate of the moon and
   * the sun. This makes the calculation easier.
   */
  for (i = 0; i <= 2; i++)
    lx[i] = lm[i];
  lx[1] = atan(tan(lx[1]) / (1 - ecce * ecce));
  swi_polcart(lx, rm);
  /* distance of moon from geocenter */
  dm = sqrt(square_sum(rm));
  /* sun in cartesian coordiantes */
  if (swe_calc(tjd, SE_SUN, iflag, rs, serr) == ERR)
    return ERR;
  /* sun in polar coordiantes */
  if (swe_calc(tjd, SE_SUN, iflag2, ls, serr) == ERR)
    return ERR;
  /* save sun position */
  for (i = 0; i <= 2; i++)
    rst[i] = rs[i];
  /* Account for oblateness of earth */
  for (i = 0; i <= 2; i++)
    lx[i] = ls[i];
  lx[1] = atan(tan(lx[1]) / (1 - ecce * ecce));
  swi_polcart(lx, rs);
  /* sun - moon vector */
  for (i = 0; i <= 2; i++) {
    e[i] = (rm[i] - rs[i]);
    et[i] = (rmt[i] - rst[i]);
  }
  /* distance sun - moon */
  dsm = sqrt(square_sum(e));
  dsmt = sqrt(square_sum(et));
  /* sun - moon unit vector */
  for (i = 0; i <= 2; i++) {
    e[i] /= dsm;
    et[i] /= dsmt;
    erm[i] = rm[i] / dm;
  }
  sinf1 = ((RSUN - RMOON) / dsm);
  cosf1 = sqrt(1 - sinf1 * sinf1);
  sinf2 = ((RSUN + RMOON) / dsm);
  cosf2 = sqrt(1 - sinf2 * sinf2);
  /* distance of moon from fundamental plane */
  s0 = -dot_prod(rm, e);
  /* distance of shadow axis from geocenter */
  r0 = sqrt(dm * dm - s0 * s0);
  /* diameter of core shadow on fundamental plane */
  d0 = (s0 / dsm * (DSUN - DMOON) - DMOON) / cosf1;
  /* diameter of half-shadow on fundamental plane */
  D0 = (s0 / dsm * (DSUN + DMOON) + DMOON) / cosf2;
  dcore[2] = r0;
  dcore[3] = d0;
  dcore[4] = D0;
  dcore[5] = cosf1;
  dcore[6] = cosf2;
  for (i = 2; i < 5; i++) 
    dcore[i] *= AUNIT / 1000.0;
  /**************************
   * central (total or annular) phase 
   **************************/
  retc = 0;
  if (de * cosf1 >= r0) {
    retc |= SE_ECL_CENTRAL;
  } else if (r0 <= de * cosf1 + fabs(d0) / 2) {
    retc |= SE_ECL_NONCENTRAL;
  } else if (r0 <= de * cosf2 + D0 / 2) {
    retc |= (SE_ECL_PARTIAL | SE_ECL_NONCENTRAL);
  } else {
    if (serr != NULL)
      sprintf(serr, "no solar eclipse at tjd = %f", tjd);
    for (i = 0; i < 10; i++)
      geopos[i] = 0;
    *dcore = 0;
    retc = 0;
    d = 0;
    no_eclipse = TRUE;
    /*return retc;*/
  } 
  /* distance of shadow point from fundamental plane */
  d = s0 * s0 + de * de - dm * dm;
  if (d > 0)
    d = sqrt(d);
  else 
    d = 0;
  /* distance of moon from shadow point on earth */
  s = s0 - d;
  /* next: geographic position of eclipse center.
   * if shadow axis does not touch the earth, 
   * place on earth with maximum occultation is computed.
   */
#if 0	/* the following stuff is meaningless for observations */
  /* 
   * account for refraction at horizon
   */
  if (d == 0) {
    double ds, a, b;
    /* distance of sun from geocenter */
    ds = sqrt(square_sum(rs));
    a = PI - acos(swi_dot_prod_unit(e, erm));
       /* refraction at horizon + sun radius = about 0.83 degrees */
    b = 34.4556 / 60.0 * DEGTORAD + asin(RSUN / ds);
# if 0
     /* at edge of umbra and penumbra 
      * light rays are not parallel to shadow axis.
      * for a short time close to contact of umbra and
      * penumbra, an angle < 0.27 degrees would have
      * to be subtracted from b;
      */ 
    if (retc & SE_ECL_PARTIAL) {
      d = d0;
      sinf = sinf1;
    } else {
      d = D0;
      sinf = sinf2;
    }
    c = (r0 - de) / d * 2 * sinf;
    if (c > sinf1) {
      b -= .....;
    }
      printf("%f %f %f", a * RADTODEG, b * RADTODEG, s);
      printf(" %f\n", s);
# else
    if (retc & SE_ECL_PARTIAL) 
      b -= asin(sinf2);	/* maximum! */
    else
      b -= asin(sinf1);
# endif
    s += tan(b) * cos(PI / 2 - a) * dm;
  }
#endif
  /* geographic position of eclipse center (maximum) */
  for (i = 0; i <= 2; i++)
    xs[i] = rm[i] + s * e[i];
  /* we need geographic position with correct z, as well */
  for (i = 0; i <= 2; i++)
    xst[i] = xs[i];
  swi_cartpol(xst, xst);
  xst[1] = atan(tan(xst[1]) * (1 - ecce * ecce));
  swi_polcart(xst, xst);
  /* to longitude and latitude */
  swi_cartpol(xs, xs);
  /* measure from sidereal time at greenwich */
  xs[0] -= sidt;
  xs[0] *= RADTODEG;
  xs[1] *= RADTODEG;
  xs[0] = swe_degnorm(xs[0]);
  /* west is negative */
  if (xs[0] > 180)
    xs[0] -= 360;
  xssv[0] = xs[0];
  xssv[1] = xs[1];
  geopos[0] = xs[0];
  geopos[1] = xs[1];
  /* diameter of core shadow: 
   * first, distance moon - place of eclipse on earth */
  for (i = 0; i <= 2; i++) 
    x[i] = rmt[i] - xst[i];
  s = sqrt(square_sum(x));
  /* diameter of core shadow at place of maximum eclipse */
  *dcore = (s / dsmt * ( DSUN - DMOON) - DMOON) * cosf1;
  *dcore *= AUNIT / 1000.0;
  /* diameter of penumbra at place of maximum eclipse */
  dcore[1] = (s / dsmt * ( DSUN + DMOON) + DMOON) * cosf2;
  dcore[1] *= AUNIT / 1000.0;
  if (!(retc & SE_ECL_PARTIAL) && !no_eclipse) {
    if (*dcore > 0) {
      /*printf("ringf”rmig\n");*/
      retc |= SE_ECL_ANNULAR;
    } else {
      /*printf("total\n");*/
      retc |= SE_ECL_TOTAL;
    }
  }
  return retc;
}

/* Computes attributes of a solar eclipse for given tjd, geo. longitude, 
 * geo. latitude, and geo. height.
 * 
 * retflag	SE_ECL_TOTAL or SE_ECL_ANNULAR or SE_ECL_PARTIAL
 *              SE_ECL_NONCENTRAL
 *              if 0, no eclipse is visible at geogr. position.
 * 
 * attr[0]	fraction of solar diameter covered by moon (magnitude)
 * attr[1]	ratio of lunar diameter to solar one
 * attr[2]	fraction of solar disc covered by moon (obscuration)
 * attr[3]      diameter of core shadow in km
 * attr[4]	azimuth of sun at tjd
 * attr[5]	true altitude of sun above horizon at tjd
 * attr[6]	apparent altitude of sun above horizon at tjd
 * attr[7]	elongation of moon in degrees
 *         declare as attr[20] at least !
 * 
 */
int32 FAR PASCAL_CONV swe_sol_eclipse_how(
          double tjd_ut, 
          int32 ifl,
          double *geopos,
          double *attr, 
          char *serr)
{
  int32 retflag, retflag2;
  double dcore[10];
  double geopos2[20];
  if ((retflag = eclipse_how(tjd_ut, ifl, geopos[0], geopos[1], geopos[2], attr, serr)) == ERR)
    return retflag;
  if ((retflag2 = eclipse_where(tjd_ut, ifl, geopos2, dcore, serr)) == ERR)
    return retflag2;
  if (retflag)
    retflag |= (retflag2 & (SE_ECL_CENTRAL | SE_ECL_NONCENTRAL));
  attr[3] = dcore[0];
  return retflag;
}

static int32 eclipse_how( double tjd_ut, int32 ifl,
          double geolon, double geolat, double geohgt,
          double *attr, char *serr)
{
  int i;
  int32 retc = 0;
  double te;
  double xs[6], xm[6], ls[6], lm[6], x1[6], x2[6];
  double rmoon, rsun, rsplusrm, rsminusrm;
  double dctr;
  int32 iflag = SEFLG_EQUATORIAL | SEFLG_TOPOCTR | ifl;
  int32 iflagcart = iflag | SEFLG_XYZ;
  double mdd, eps, sidt, armc, xh[6], hmin_appr;
  double lsun, lmoon, lctr, lsunleft, a, b, sc1, sc2;
  for (i = 0; i < 10; i++)
    attr[i] = 0;
  te = tjd_ut + swe_deltat(tjd_ut);
  swe_set_topo(geolon, geolat, geohgt);
  if (swe_calc(te, SE_SUN, iflag, ls, serr) == ERR)
    return ERR;
  if (swe_calc(te, SE_MOON, iflag, lm, serr) == ERR)
    return ERR;
  if (swe_calc(te, SE_SUN, iflagcart, xs, serr) == ERR)
    return ERR;
  if (swe_calc(te, SE_MOON, iflagcart, xm, serr) == ERR)
    return ERR;
  /* 
   * azimuth and altitude of sun 
   */
  eps = swi_epsiln(te);
  if (iflag & SEFLG_NONUT)
    sidt = swe_sidtime0(tjd_ut, eps * RADTODEG, 0) * 15;
  else
    sidt = swe_sidtime(tjd_ut) * 15;
  armc = sidt + geolon;
  mdd = swe_degnorm(ls[0] - armc);
  xh[0] = swe_degnorm(mdd - 90);
  xh[1] = ls[1];
  xh[2] = ls[2];
  swe_cotrans(xh, xh, 90 - geolat);	/* azimuth from east, counterclock */
  /* eclipse description */
  rmoon = asin(RMOON / lm[2]) * RADTODEG;
  rsun = asin(RSUN / ls[2]) * RADTODEG;
  rsplusrm = rsun + rmoon;
  rsminusrm = rsun - rmoon;
  for (i = 0; i < 3; i++) {
    x1[i] = xs[i] / ls[2];
    x2[i] = xm[i] / lm[2];
  }
  dctr = acos(swi_dot_prod_unit(x1, x2)) * RADTODEG;
  /* 
   * phase 
   */
  if (dctr < rsminusrm)
    retc = SE_ECL_ANNULAR;
  else if (dctr < fabs(rsminusrm))
    retc = SE_ECL_TOTAL;
  else if (dctr < rsplusrm)
    retc = SE_ECL_PARTIAL;
  else {
    retc = 0;
    if (serr != NULL)
      sprintf(serr, "no solar eclipse at tjd = %f", tjd_ut);
  }
  /* 
   * percentage of eclipse 
   */
#if 0
  attr[0] = (rsplusrm - dctr) / rsun / 2 * 100;
#else
  /* 
   * eclipse magnitude:
   * fraction of solar diameter covered by moon
   */
  lsun = asin(rsun / 2 * DEGTORAD) * 2;
  lmoon = asin(rmoon / 2 * DEGTORAD) * 2;
  lctr = asin(dctr / 2 * DEGTORAD) * 2;
  lsunleft = asin((-dctr + rsun + rmoon) * DEGTORAD / 2) * 2;
  attr[0] = lsunleft / lsun / 2;
  /*
   * ratio of diameter of moon to that of sun
   */
  attr[1] = lmoon / lsun;
  /*
   * obscuration:
   * fraction of solar disc obscured by moon
   */
  if (retc == 0)
    attr[2] = 0;
  else if (retc == SE_ECL_TOTAL || retc == SE_ECL_ANNULAR)
    attr[2] = lmoon * lmoon / lsun / lsun;
  else {
    a = 2 * lctr * lmoon;
    b = 2 * lctr * lsun;
    if (a < 1e-9) {
      attr[2] = lmoon * lmoon / lsun / lsun;
    } else {
      a = acos((lctr * lctr + lmoon * lmoon - lsun * lsun) / a);
      b = acos((lctr * lctr + lsun * lsun - lmoon * lmoon) / b);
      sc1 = a * lmoon * lmoon / 2;
      sc2 = b * lsun * lsun / 2;
      sc1 -= (cos(a) * sin(a)) * lmoon * lmoon / 2;
      sc2 -= (cos(b) * sin(b)) * lsun * lsun / 2;
      attr[2] = (sc1 + sc2) * 2 / PI / lsun / lsun;
    }
  }
#endif
  attr[7] = dctr;
  /* approximate minimum height for visibility, considering
   * refraction and dip
   * 34.4556': refraction at horizon, from Bennets formulae 
   * 1.75' / sqrt(geohgt): dip of horizon
   * 0.37' / sqrt(geohgt): refraction between horizon and observer */
  hmin_appr = -(34.4556 + (1.75 + 0.37) * sqrt(geohgt)) / 60;	
  if (xh[1] + rsun + fabs(hmin_appr) >= 0 && retc) 
    retc |= SE_ECL_VISIBLE;	/* eclipse visible */
  attr[4] = swe_degnorm(90 - xh[0]);	/* azimuth, from north, clockwise */
  attr[5] = xh[1]; /* height */
  return retc;
}

/* When is the next solar eclipse anywhere on earth?
 *
 * input parameters:
 *
 * tjd_start    start time for search (UT)
 * ifl		ephemeris to be used (SEFLG_SWIEPH, etc.)
 * ifltype	eclipse type to be searched (SE_ECL_TOTAL, etc.)
 *              0, if any type of eclipse is wanted
 *
 * return parameters: 
 *
 * retflag	SE_ECL_TOTAL or SE_ECL_ANNULAR or SE_ECL_PARTIAL
 *              or SE_ECL_ANNULAR_TOTAL
 *              SE_ECL_CENTRAL
 *              SE_ECL_NONCENTRAL
 *
 * tret[0]	time of maximum eclipse
 * tret[1]	time, when eclipse takes place at local apparent noon
 * tret[2]	time of eclipse begin
 * tret[3]	time of eclipse end
 * tret[4]	time of totality begin
 * tret[5]	time of totality end
 * tret[6]	time of center line begin
 * tret[7]	time of center line end
 * tret[8]	time when annular-total eclipse becomes total
 *      	   not implemented so far
 * tret[9]	time when annular-total eclipse becomes annular again
 *      	   not implemented so far
 *         declare as tret[10] at least!
 *
 */
int32 FAR PASCAL_CONV swe_sol_eclipse_when_glob(double tjd_start, int32 ifl, int32 ifltype,
     double *tret, AS_BOOL backward, char *serr)
{
  int i, j, k, m, n, o, i1, i2;
  int32 retflag = 0, retflag2 = 0;
  double de = 6378.140, a;
  double t, tt, tjd, tjds, tjdsv, dt, dtint, dta, dtb;
  double T, T2, T3, T4, K, M, Mm;
  double E, Ff;
  double xs[6], xm[6], ls[6], lm[6];
  double rmoon, rsun, dcore[10];
  double dc[3], dctr;
  double twohr = 2.0 / 24.0;
  double tenmin = 10.0 / 24.0 / 60.0;
  double dt1, dt2;
  double geopos[20], attr[20];
  double dtstart, dtdiv;
  double xa[6], xb[6];
  int direction = 1;
  int32 iflag = SEFLG_EQUATORIAL | ifl;
  int32 iflagtop = SEFLG_EQUATORIAL | SEFLG_TOPOCTR | ifl;
  int32 iflagcart = iflag | SEFLG_XYZ;
  AS_BOOL dont_times = FALSE;
  struct epsilon *oe = &swed.oec;
  if (ifltype == (SE_ECL_PARTIAL | SE_ECL_CENTRAL)) {
    if (serr != NULL)
      strcpy(serr, "central partial eclipses do not exist");
    return ERR;
  }
  if (ifltype == 0)
    ifltype = SE_ECL_TOTAL | SE_ECL_ANNULAR | SE_ECL_PARTIAL
           | SE_ECL_ANNULAR_TOTAL | SE_ECL_NONCENTRAL | SE_ECL_CENTRAL;
  if (backward)
    direction = -1;
  K = (int) ((tjd_start - J2000) / 365.2425 * 12.3685);
  K -= direction;
next_try:
  retflag = 0;
  dont_times = FALSE;
  for (i = 0; i <= 9; i++)
    tret[i] = 0;
  T = K / 1236.85;
  T2 = T * T; T3 = T2 * T; T4 = T3 * T;
  Ff = swe_degnorm(160.7108 + 390.67050274 * K
               - 0.0016341 * T2
               - 0.00000227 * T3
               + 0.000000011 * T4);
  if (Ff > 180)
    Ff -= 180;
  if (Ff > 21 && Ff < 159) { 	/* no eclipse possible */
    K += direction;
    goto next_try;
  }
  /* approximate time of geocentric maximum eclipse
   * formula from Meeus, German, p. 381 */
  tjd = 2451550.09765 + 29.530588853 * K
                      + 0.0001337 * T2
                      - 0.000000150 * T3
                      + 0.00000000073 * T4;
  M = swe_degnorm(2.5534 + 29.10535669 * K
                      - 0.0000218 * T2
                      - 0.00000011 * T3);
  Mm = swe_degnorm(201.5643 + 385.81693528 * K
                      + 0.1017438 * T2
                      + 0.00001239 * T3
                      + 0.000000058 * T4);
  E = 1 - 0.002516 * T - 0.0000074 * T2;
  M *= DEGTORAD;
  Mm *= DEGTORAD;
  tjd = tjd - 0.4075 * sin(Mm)
            + 0.1721 * E * sin(M);
  /* 
   * time of maximum eclipse (if eclipse) = 
   * minimum geocentric angle between sun and moon edges.
   * After this time has been determined, check
   * whether or not an eclipse is taking place with
   * the functions eclipse_where() and _how().
   */
  dtstart = 1;
  if (tjd < 2000000)
    dtstart = 5;
  dtdiv = 4;
  for (dt = dtstart, tjdsv = tjd - 1; 
       dt > 0.0001; 
       dt /= dtdiv) {
    for (i = 0, t = tjd - dt; i <= 2; i++, t += dt) {
      if (swe_calc(t, SE_SUN, iflag, ls, serr) == ERR)
        return ERR;
      if (swe_calc(t, SE_MOON, iflag, lm, serr) == ERR)
        return ERR;
      if (swe_calc(t, SE_SUN, iflagcart, xs, serr) == ERR)
        return ERR;
      if (swe_calc(t, SE_MOON, iflagcart, xm, serr) == ERR)
        return ERR;
      for (m = 0; m < 3; m++) {
        xa[m] = xs[m] / ls[2];
        xb[m] = xm[m] / lm[2];
      }
      dc[i] = acos(swi_dot_prod_unit(xa, xb)) * RADTODEG;
      rmoon = asin(RMOON / lm[2]) * RADTODEG;
      rsun = asin(RSUN / ls[2]) * RADTODEG;
      dc[i] -= (rmoon + rsun);
    }
    find_maximum(dc[0], dc[1], dc[2], dt, &dtint, &dctr);
    tjdsv = tjd;
    tjd += dtint + dt;
  }
  tjds = tjd = tjd - swe_deltat(tjd);
  if ((retflag = eclipse_where(tjd, ifl, geopos, dcore, serr)) == ERR)
    return retflag;
  retflag2 = retflag;
    /* in extreme cases _where() returns no eclipse, where there is
     * actually a very small one, therefore call _how() with the
     * coordinates returned by _where(): */
  if ((retflag2 = eclipse_how(tjd, ifl, geopos[0], geopos[1], 0, attr, serr)) == ERR)
    return retflag2;
  if (retflag2 == 0) {
    K += direction;
    goto next_try;
  }
  tret[0] = tjd;
  if ((backward && tret[0] >= tjd_start - 0.0001) 
    || (!backward && tret[0] <= tjd_start + 0.0001)) {
    K += direction;
    goto next_try;
  }
  /*
   * eclipse type, SE_ECL_TOTAL, _ANNULAR, etc.
   * SE_ECL_ANNULAR_TOTAL will be discovered later
   */
  if ((retflag = eclipse_where(tjd, ifl, geopos, dcore, serr)) == ERR)
    return retflag;
  if (retflag == 0) {	/* can happen with extremely small percentage */
    retflag = SE_ECL_PARTIAL | SE_ECL_NONCENTRAL;
    tret[4] = tret[5] = tjd;	/* fix this ???? */
    dont_times = TRUE;
  } 
  /* 
   * check whether or not eclipse type found is wanted
   */
  /* non central eclipse is wanted: */
  if (!(ifltype & SE_ECL_NONCENTRAL) && (retflag & SE_ECL_NONCENTRAL)) {
    K += direction;
    goto next_try;
  }
  /* central eclipse is wanted: */
  if (!(ifltype & SE_ECL_CENTRAL) && (retflag & SE_ECL_CENTRAL)) {
    K += direction;
    goto next_try;
  }
  /* non annular eclipse is wanted: */
  if (!(ifltype & SE_ECL_ANNULAR) && (retflag & SE_ECL_ANNULAR)) {
    K += direction;
    goto next_try;
  }
  /* non partial eclipse is wanted: */
  if (!(ifltype & SE_ECL_PARTIAL) && (retflag & SE_ECL_PARTIAL)) {
    K += direction;
    goto next_try;
  }
  /* annular-total eclipse will be discovered later */
  if (!(ifltype & (SE_ECL_TOTAL | SE_ECL_ANNULAR_TOTAL)) && (retflag & SE_ECL_TOTAL)) {
    K += direction;
    goto next_try;
  }
  if (dont_times)
    goto end_search_global;
  /* 
   * n = 0: times of eclipse begin and end 
   * n = 1: times of totality begin and end 
   * n = 2: times of center line begin and end 
   */
  if (retflag & SE_ECL_PARTIAL)
    o = 0;
  else if (retflag & SE_ECL_NONCENTRAL)
    o = 1;
  else 
    o = 2;
  dta = twohr;
  dtb = tenmin;
  for (n = 0; n <= o; n++) {
    if (n == 0) {
      /*dc[1] = dcore[3] / 2 + de - dcore[1];*/
      i1 = 2; i2 = 3;
    } else if (n == 1) {
      if (retflag & SE_ECL_PARTIAL)
        continue;
      i1 = 4; i2 = 5;
    } else if (n == 2) {
      if (retflag & SE_ECL_NONCENTRAL)
        continue;
      i1 = 6; i2 = 7;
    }
    for (i = 0, t = tjd - dta; i <= 2; i += 1, t += dta) {
      if ((retflag2 = eclipse_where(t, ifl, geopos, dcore, serr)) == ERR)
        return retflag2;
      if (n == 0)
        dc[i] = dcore[4] / 2 + de / dcore[5] - dcore[2];
      else if (n == 1)
        dc[i] = fabs(dcore[3]) / 2 + de / dcore[6] - dcore[2];
      else if (n == 2)
        dc[i] = de / dcore[6] - dcore[2];
    }
    find_zero(dc[0], dc[1], dc[2], dta, &dt1, &dt2);
    tret[i1] = tjd + dt1 + dta;
    tret[i2] = tjd + dt2 + dta;
    for (m = 0, dt = dtb; m < 3; m++, dt /= 3) {
      for (j = i1; j <= i2; j += (i2 - i1)) {
        for (i = 0, t = tret[j] - dt; i < 2; i++, t += dt) {
          if ((retflag2 = eclipse_where(t, ifl, geopos, dcore, serr)) == ERR)
            return retflag2;
          if (n == 0)
            dc[i] = dcore[4] / 2 + de / dcore[5] - dcore[2];
          else if (n == 1)
            dc[i] = fabs(dcore[3]) / 2 + de / dcore[6] - dcore[2];
          else if (n == 2)
            dc[i] = de / dcore[6] - dcore[2];
        }
        dt1 = dc[1] / ((dc[1] - dc[0]) / dt);
        tret[j] -= dt1;
      }
    }
  }
  /*
   * annular-total eclipses
   */
  if (retflag & SE_ECL_TOTAL) {
    if ((retflag2 = eclipse_where(tret[0], ifl, geopos, dcore, serr)) == ERR)
      return retflag2;
    dc[0] = *dcore;
    if ((retflag2 = eclipse_where(tret[4], ifl, geopos, dcore, serr)) == ERR)
      return retflag2;
    dc[1] = *dcore;
    if ((retflag2 = eclipse_where(tret[5], ifl, geopos, dcore, serr)) == ERR)
      return retflag2;
    dc[2] = *dcore;
    /* the maximum is always total, and there is either one or
     * to times before and after, when the core shadow becomes
     * zero and totality changes into annularity or vice versa.
     */
    if (dc[0] * dc[1] < 0 || dc[0] * dc[2] < 0) {
      retflag |= SE_ECL_ANNULAR_TOTAL;
      retflag &= ~SE_ECL_TOTAL;
    } 
  } 
  /* if eclipse is given but not wanted: */
  if (!(ifltype & SE_ECL_TOTAL) && (retflag & SE_ECL_TOTAL)) {
    K += direction;
    goto next_try;
  }
  /* if annular_total eclipse is given but not wanted: */
  if (!(ifltype & SE_ECL_ANNULAR_TOTAL) && (retflag & SE_ECL_ANNULAR_TOTAL)) {
    K += direction;
    goto next_try;
  }
  /*
   * time of maximum eclipse at local apparent noon
   */
  /* first, find out, if there is a solar transit 
   * between begin and end of eclipse */
  k = 2;
  for (i = 0; i < 2; i++) {
    j = i + k;
    tt = tret[j] + swe_deltat(tret[j]);
    if (swe_calc(tt, SE_SUN, iflag, ls, serr) == ERR)
        return ERR;
    if (swe_calc(tt, SE_MOON, iflag, lm, serr) == ERR)
        return ERR;
    dc[i] = swe_degnorm(ls[0] - lm[0]);
    if (dc[i] > 180)
      dc[i] -= 360;
  }
  if (dc[0] * dc[1] >= 0)	/* no transit */
    tret[1] = 0;
  else {
    tjd = tjds;
    dt = 0.1;
    dt1 = (tret[3] - tret[2]) / 2.0;
    if (dt1 < dt)
      dt = dt1 / 2.0;
    for (j = 0, tjdsv = tjd - 1; 
        dt > 0.01;
        j++, dt /= 3) {
      for (i = 0, t = tjd; i <= 1; i++, t -= dt) {
        tt = t + swe_deltat(t);
        if (swe_calc(tt, SE_SUN, iflag, ls, serr) == ERR)
          return ERR;
        if (swe_calc(tt, SE_MOON, iflag, lm, serr) == ERR)
          return ERR;
        dc[i] = swe_degnorm(ls[0] - lm[0]);
        if (dc[i] > 180)
          dc[i] -= 360;
        if (dc[i] > 180)
          dc[i] -= 360;
      }
      a = (dc[1] - dc[0]) / dt;
      if (a < 1e-10)
        break;
      dt1 = dc[0] / a;
      tjdsv = tjd;
      tjd += dt1;
    }
    tret[1] = tjd;
  }
end_search_global:
  return retflag;
  /*
   * the time of maximum occultation is practically identical
   * with the time of maximum core shadow diameter.
   *
   * the time, when duration of totality is maximal,
   * is not an interesting computation either. Near the maximum
   * occulation, the time of totality can be the same by 
   * a second for hundreds of kilometers (for 10 minutes 
   * or more).
   *  
   * for annular eclipses the maximum duration is close to the
   * beginning and the end of the center lines, where is also
   * the minimum of core shadow diameter.
   */
}

/* When is the next solar eclipse at a given geographical position?
 * Note the uncertainty of Delta T for the remote past and for
 * the future.
 *
 * retflag	SE_ECL_TOTAL or SE_ECL_ANNULAR or SE_ECL_PARTIAL
 *              SE_ECL_VISIBLE, 
 *              SE_ECL_MAX_VISIBLE, 
 *              SE_ECL_1ST_VISIBLE, SE_ECL_2ND_VISIBLE
 *              SE_ECL_3ST_VISIBLE, SE_ECL_4ND_VISIBLE
 *
 * tret[0]	time of maximum eclipse
 * tret[1]	time of first contact
 * tret[2]	time of second contact
 * tret[3]	time of third contact
 * tret[4]	time of forth contact
 * tret[5]	time of sun rise between first and forth contact
                        (not implemented so far)
 * tret[6]	time of sun set beween first and forth contact
                        (not implemented so far)
 *
 * attr[0]	fraction of solar diameter covered by moon (magnitude)
 * attr[1]	ratio of lunar diameter to solar one
 * attr[2]	fraction of solar disc covered by moon (obscuration)
 * attr[3]      diameter of core shadow in km
 * attr[4]	azimuth of sun at tjd
 * attr[5]	true altitude of sun above horizon at tjd
 * attr[6]	apparent altitude of sun above horizon at tjd
 * attr[7]	elongation of moon in degrees
 *         declare as attr[20] at least !
 */
int32 FAR PASCAL_CONV swe_sol_eclipse_when_loc(double tjd_start, int32 ifl,
     double *geopos, double *tret, double *attr, AS_BOOL backward, char *serr)
{
  int32 retflag = 0, retflag2 = 0;
  double geopos2[20], dcore[10];
  if ((retflag = eclipse_when_loc(tjd_start, ifl, geopos, tret, attr, backward, serr)) <= 0)
    return retflag;
  /* 
   * diameter of core shadow
   */
  if ((retflag2 = eclipse_where(tret[0], ifl, geopos2, dcore, serr)) == ERR)
    return retflag2;
  retflag |= (retflag2 & SE_ECL_NONCENTRAL);
  attr[3] = dcore[0];
  return retflag; 
}

static int32 eclipse_when_loc(double tjd_start, int32 ifl, double *geopos, double *tret, double *attr, AS_BOOL backward, char *serr)
{
  int i, j, k, m;
  int32 retflag = 0;
  double t, tjd, dt, dtint, T, T2, T3, T4, K, F, M, Mm;
  double E, Ff, A1, Om;
  double xs[6], xm[6], ls[6], lm[6], x1[6], x2[6], dm, ds;
  double rmoon, rsun, rsplusrm, rsminusrm;
  double dc[3], dctr, dctrmin;
  double twomin = 2.0 / 24.0 / 60.0;
  double tensec = 10.0 / 24.0 / 60.0 / 60.0;
  double twohr = 2.0 / 24.0;
  double tenmin = 10.0 / 24.0 / 60.0;
  double dt1, dt2, dtdiv, dtstart;
  int32 iflag = SEFLG_EQUATORIAL | SEFLG_TOPOCTR | ifl;
  int32 iflagcart = iflag | SEFLG_XYZ;
  K = (int) ((tjd_start - J2000) / 365.2425 * 12.3685);
  if (backward)
    K++;
  else
    K--;
next_try:
  T = K / 1236.85;
  T2 = T * T; T3 = T2 * T; T4 = T3 * T;
  Ff = F = swe_degnorm(160.7108 + 390.67050274 * K
               - 0.0016341 * T2
               - 0.00000227 * T3
               + 0.000000011 * T4);
  if (Ff > 180)
    Ff -= 180;
  if (Ff > 21 && Ff < 159) { 	/* no eclipse possible */
    if (backward)
      K--;
    else 
      K++;
    goto next_try;
  }
  /* approximate time of geocentric maximum eclipse.
   * formula from Meeus, German, p. 381 */
  tjd = 2451550.09765 + 29.530588853 * K
                      + 0.0001337 * T2
                      - 0.000000150 * T3
                      + 0.00000000073 * T4;
  M = swe_degnorm(2.5534 + 29.10535669 * K
                      - 0.0000218 * T2
                      - 0.00000011 * T3);
  Mm = swe_degnorm(201.5643 + 385.81693528 * K
                      + 0.1017438 * T2
                      + 0.00001239 * T3
                      + 0.000000058 * T4);
  Om = swe_degnorm(124.7746 - 1.56375580 * K
                      + 0.0020691 * T2
                      + 0.00000215 * T3);
  E = 1 - 0.002516 * T - 0.0000074 * T2;
  A1 = swe_degnorm(299.77 + 0.107408 * K - 0.009173 * T2);
  M *= DEGTORAD;
  Mm *= DEGTORAD;
  F *= DEGTORAD;
  Om *= DEGTORAD;
  A1 *= DEGTORAD;
  tjd = tjd - 0.4075 * sin(Mm)
            + 0.1721 * E * sin(M);
  swe_set_topo(geopos[0], geopos[1], geopos[2]);
  dtdiv = 2;
  dtstart = 0.5;
  if (tjd < 1900000)	/* because above formula is not good (delta t?) */
    dtstart = 2;
  for (dt = dtstart; 
       dt > 0.00001; 
       dt /= dtdiv) {
    if (dt < 0.1) 
      dtdiv = 3;
    for (i = 0, t = tjd - dt; i <= 2; i++, t += dt) {
      /* this takes some time, but is necessary to avoid
       * missing an eclipse */
      if (swe_calc(t, SE_SUN, iflagcart, xs, serr) == ERR)
        return ERR;
      if (swe_calc(t, SE_SUN, iflag, ls, serr) == ERR)
        return ERR;
      if (swe_calc(t, SE_MOON, iflagcart, xm, serr) == ERR)
        return ERR;
      if (swe_calc(t, SE_MOON, iflag, lm, serr) == ERR)
        return ERR;
      dm = sqrt(square_sum(xm));
      ds = sqrt(square_sum(xs));
      for (k = 0; k < 3; k++) {
        x1[k] = xs[k] / ds /*ls[2]*/;
        x2[k] = xm[k] / dm /*lm[2]*/;
      }
      dc[i] = acos(swi_dot_prod_unit(x1, x2)) * RADTODEG;
    }
    find_maximum(dc[0], dc[1], dc[2], dt, &dtint, &dctr);
    tjd += dtint + dt;
  }
  if (swe_calc(tjd, SE_SUN, iflagcart, xs, serr) == ERR)
        return ERR;
  if (swe_calc(tjd, SE_SUN, iflag, ls, serr) == ERR)
        return ERR;
  if (swe_calc(tjd, SE_MOON, iflagcart, xm, serr) == ERR)
        return ERR;
  if (swe_calc(tjd, SE_MOON, iflag, lm, serr) == ERR)
        return ERR;
  dctr = acos(swi_dot_prod_unit(xs, xm)) * RADTODEG;
  rmoon = asin(RMOON / lm[2]) * RADTODEG;
  rsun = asin(RSUN / ls[2]) * RADTODEG;
  rsplusrm = rsun + rmoon;
  rsminusrm = rsun - rmoon;
  if (dctr > rsplusrm) {
    if (backward)
      K--;
    else
      K++;
    goto next_try;
  }
  tret[0] = tjd - swe_deltat(tjd);
  if ((backward && tret[0] >= tjd_start - 0.0001) 
    || (!backward && tret[0] <= tjd_start + 0.0001)) {
    if (backward)
      K--;
    else
      K++;
    goto next_try;
  }
  if (dctr < rsminusrm)
    retflag = SE_ECL_ANNULAR;
  else if (dctr < fabs(rsminusrm))
    retflag = SE_ECL_TOTAL;
  else if (dctr <= rsplusrm)
    retflag = SE_ECL_PARTIAL;
  dctrmin = dctr;
  /* contacts 2 and 3 */
  if (dctr > fabs(rsminusrm))  /* partial, no 2nd and 3rd contact */
    tret[2] = tret[3] = 0;
  else {
    dc[1] = fabs(rsminusrm) - dctrmin;
    for (i = 0, t = tjd - twomin; i <= 2; i += 2, t = tjd + twomin) {
      if (swe_calc(t, SE_SUN, iflagcart, xs, serr) == ERR)
        return ERR;
      if (swe_calc(t, SE_MOON, iflagcart, xm, serr) == ERR)
        return ERR;
      dm = sqrt(square_sum(xm));
      ds = sqrt(square_sum(xs));
      rmoon = asin(RMOON / dm) * RADTODEG;
      rsun = asin(RSUN / ds) * RADTODEG;
      rsminusrm = rsun - rmoon;
      for (k = 0; k < 3; k++) {
        x1[k] = xs[k] / ds /*ls[2]*/;
        x2[k] = xm[k] / dm /*lm[2]*/;
      }
      dctr = acos(swi_dot_prod_unit(x1, x2)) * RADTODEG;
      dc[i] = fabs(rsminusrm) - dctr;
    }
    find_zero(dc[0], dc[1], dc[2], twomin, &dt1, &dt2);
    tret[2] = tjd + dt1 + twomin;
    tret[3] = tjd + dt2 + twomin;
    for (m = 0, dt = tensec; m < 2; m++, dt /= 10) {
      for (j = 2; j <= 3; j++) {
        if (swe_calc(tret[j], SE_SUN, iflagcart | SEFLG_SPEED, xs, serr) == ERR)
          return ERR;
        if (swe_calc(tret[j], SE_MOON, iflagcart | SEFLG_SPEED, xm, serr) == ERR)
          return ERR;
        for (i = 0; i < 2; i++) {
          if (i == 1) {
            for(k = 0; k < 3; k++) {
              xs[k] -= xs[k+3] * dt;
              xm[k] -= xm[k+3] * dt;
            }
          }
          dm = sqrt(square_sum(xm));
          ds = sqrt(square_sum(xs));
          rmoon = asin(RMOON / dm) * RADTODEG;
          rsun = asin(RSUN / ds) * RADTODEG;
          rsminusrm = rsun - rmoon;
          for (k = 0; k < 3; k++) {
            x1[k] = xs[k] / ds /*ls[2]*/;
            x2[k] = xm[k] / dm /*lm[2]*/;
          }
          dctr = acos(swi_dot_prod_unit(x1, x2)) * RADTODEG;
          dc[i] = fabs(rsminusrm) - dctr;
        }
        dt1 = -dc[0] / ((dc[0] - dc[1]) / dt);
        tret[j] += dt1;
      }
    }
    tret[2] -= swe_deltat(tret[2]);
    tret[3] -= swe_deltat(tret[3]);
  }
  /* contacts 1 and 4 */
  dc[1] = rsplusrm - dctrmin;
  for (i = 0, t = tjd - twohr; i <= 2; i += 2, t = tjd + twohr) {
    if (swe_calc(t, SE_SUN, iflagcart, xs, serr) == ERR)
      return ERR;
    if (swe_calc(t, SE_MOON, iflagcart, xm, serr) == ERR)
      return ERR;
    dm = sqrt(square_sum(xm));
    ds = sqrt(square_sum(xs));
    rmoon = asin(RMOON / dm) * RADTODEG;
    rsun = asin(RSUN / ds) * RADTODEG;
    rsplusrm = rsun + rmoon;
    for (k = 0; k < 3; k++) {
      x1[k] = xs[k] / ds /*ls[2]*/;
      x2[k] = xm[k] / dm /*lm[2]*/;
    }
    dctr = acos(swi_dot_prod_unit(x1, x2)) * RADTODEG;
    dc[i] = rsplusrm - dctr;
  }
  find_zero(dc[0], dc[1], dc[2], twohr, &dt1, &dt2);
  tret[1] = tjd + dt1 + twohr;
  tret[4] = tjd + dt2 + twohr;
  for (m = 0, dt = tenmin; m < 3; m++, dt /= 10) {
    for (j = 1; j <= 4; j += 3) {
      if (swe_calc(tret[j], SE_SUN, iflagcart | SEFLG_SPEED, xs, serr) == ERR)
        return ERR;
      if (swe_calc(tret[j], SE_MOON, iflagcart | SEFLG_SPEED, xm, serr) == ERR)
        return ERR;
      for (i = 0; i < 2; i++) {
        if (i == 1) {
          for(k = 0; k < 3; k++) {
            xs[k] -= xs[k+3] * dt;
            xm[k] -= xm[k+3] * dt;
          }
        }
        dm = sqrt(square_sum(xm));
        ds = sqrt(square_sum(xs));
        rmoon = asin(RMOON / dm) * RADTODEG;
        rsun = asin(RSUN / ds) * RADTODEG;
        rsplusrm = rsun + rmoon;
        for (k = 0; k < 3; k++) {
          x1[k] = xs[k] / ds /*ls[2]*/;
          x2[k] = xm[k] / dm /*lm[2]*/;
        }
        dctr = acos(swi_dot_prod_unit(x1, x2)) * RADTODEG;
        dc[i] = fabs(rsplusrm) - dctr;
      }
      dt1 = -dc[0] / ((dc[0] - dc[1]) / dt);
      tret[j] += dt1;
    }
  }
  tret[1] -= swe_deltat(tret[1]);
  tret[4] -= swe_deltat(tret[4]);
  /*  
   * visibility of eclipse phases 
   */
  for (i = 4; i >= 0; i--) {	/* attr for i = 0 must be kept !!! */
    if (tret[i] == 0)
      continue;
    if (eclipse_how(tret[i], ifl, geopos[0], geopos[1], geopos[2], 
		attr, serr) == ERR)
      return ERR;
    /*if (retflag2 & SE_ECL_VISIBLE) { could be wrong for 1st/4th contact */
    if (attr[5] > 0) {	/* this is save, sun above horizon */
      retflag |= SE_ECL_VISIBLE;
      switch(i) {
      case 0: retflag |= SE_ECL_MAX_VISIBLE; break;
      case 1: retflag |= SE_ECL_1ST_VISIBLE; break;
      case 2: retflag |= SE_ECL_2ND_VISIBLE; break;
      case 3: retflag |= SE_ECL_3RD_VISIBLE; break;
      case 4: retflag |= SE_ECL_4TH_VISIBLE; break;
      default:  break;
      }
    }
  }
#if 1
  if (!(retflag & SE_ECL_VISIBLE)) {
    if (backward)
      K--;
    else
      K++;
    goto next_try;
  }
#endif
  return retflag;
}

/* 
 * swe_azalt()
 * Computes azimut and height, from either ecliptic or 
 * equatorial coordinates
 *
 * input:
 *   tjd_ut	  
 *   iflag        either SE_ECL2HOR or SE_EQU2HOR
 *   geopos[3]    geograph. longitude, latitude, height above sea
 *   atpress	  atmospheric pressure at geopos in millibars (hPa)
 *   attemp	  atmospheric temperature in degrees C
 *   xin[2]       input coordinates polar, in degrees
 * 
 * Horizontal coordinates are returned in
 *   xaz[3]       xaz[0] = azimuth
 *                xaz[1] = true altitude
 *                xaz[2] = apparent altitude
 *
 * If atpress is not given (= 0), the programm assumes 1013.25 mbar;
 * if a non-zero height above sea is given, atpress is estimated.
 *   geohgt	  height of observer above sea (optional)
 */
void FAR PASCAL_CONV swe_azalt(
      double tjd_ut,
      int32  calc_flag,
      double *geopos,
      double atpress,
      double attemp,
      double *xin, 
      double *xaz) 
{
  int i;
  double x[6], xra[3];
  double armc = swe_degnorm(swe_sidtime(tjd_ut) * 15 + geopos[0]);
  double mdd, eps_true, tjd_et;
  for (i = 0; i < 2; i++)
    xra[i] = xin[i];
  xra[2] = 1;
  if (calc_flag == SE_ECL2HOR) {
	tjd_et = tjd_ut + swe_deltat(tjd_ut);
    swe_calc(tjd_et, SE_ECL_NUT, 0, x, NULL);
    eps_true = x[0];
	swe_cotrans(xra, xra, -eps_true);
  }
  mdd = swe_degnorm(xra[0] - armc);
  x[0] = swe_degnorm(mdd - 90);
  x[1] = xra[1];
  x[2] = 1;
  /* azimuth from east, counterclock */
  swe_cotrans(x, x, 90 - geopos[1]);
  /* azimuth from south to west */
  x[0] = swe_degnorm(x[0] + 90);
  xaz[0] = 360 - x[0];
  xaz[1] = x[1];		/* true height */
  if (atpress == 0) {
    /* estimate atmospheric pressure */
    atpress = 1013.25 * pow(1 - 0.0065 * geopos[2] / 288, 5.255);
  } 
  xaz[2] = swe_refrac(x[1], atpress, attemp, SE_TRUE_TO_APP);
}

/* 
 * swe_azalt_rev()
 * computes either ecliptical or equatorial coordinates from
 * azimuth and true altitude in degrees.
 * For conversion between true and apparent altitude, there is
 * the function swe_refrac().
 *
 * input:
 *   tjd_ut	  
 *   iflag        either SE_HOR2ECL or SE_HOR2EQU
 *   xin[2]       azimut and true altitude, in degrees 
 */
void FAR PASCAL_CONV swe_azalt_rev(
      double tjd_ut,
      int32  calc_flag,
      double *geopos,
      double *xin, 
      double *xout) 
{
  int i;
  double x[6], xaz[3];
  double geolon = geopos[0];
  double geolat = geopos[1];
  double armc = swe_degnorm(swe_sidtime(tjd_ut) * 15 + geolon);
  double eps_true, tjd_et;
  for (i = 0; i < 2; i++)
    xaz[i] = xin[i];
  xaz[2] = 1;
  /* azimuth is from south, clockwise. 
   * we need it from east, counterclock */
  xaz[0] = 360 - xaz[0];
  xaz[0] = swe_degnorm(xaz[0] - 90);
  /* equatorial positions */
  swe_cotrans(xaz, xaz, geolat - 90);
  xaz[0] = swe_degnorm(xaz[0] + armc + 90);
  xout[0] = xaz[0];
  xout[1] = xaz[1];
  /* ecliptic positions */
  if (calc_flag == SE_HOR2ECL) {
	tjd_et = tjd_ut + swe_deltat(tjd_ut);
    swe_calc(tjd_et, SE_ECL_NUT, 0, x, NULL);
    eps_true = x[0];
	swe_cotrans(xaz, x, eps_true);
    xout[0] = x[0];
    xout[1] = x[1];
  }
}

/* swe_refrac()
 * Transforms apparent to true altitude and vice-versa.
 * These formulae do not handle the case when the
 * sun is visible below the geometrical horizon 
 * (from a mountain top or an air plane)
 * input:
 * double inalt;        * altitude of object in degrees *
 * double atpress;      * millibars (hectopascal) *
 * double attemp;       * degrees C *
 * int32  calc_flag;	* either SE_CALC_APP_TO_TRUE or 
 *                      *        SE_CALC_TRUE_TO_APP
 */
double FAR PASCAL_CONV swe_refrac(double inalt, double atpress, double attemp, int32 calc_flag)
{
#if 0
 /*
  * -- S. L. Moshier */
  double y, yy0, D0, N, D, P, Q;
  int i;
  double trualt;
  if (calc_flag == SE_TRUE_TO_APP) {
    trualt = inalt;
    if( (trualt < -2.0) || (trualt >= 90.0) )
      return(trualt);
    /* For high altitude angle, AA page B61
     * Accuracy "usually about 0.1' ".
     */
    if( trualt > 15.0 ) {
	  D = 0.00452*atpress/((273.0+attemp)*tan( DEGTORAD*trualt ));
	  return(trualt + D);
	}
    /* Formula for low altitude is from the Almanac for Computers.
     * It gives the correction for observed altitude, so has
     * to be inverted numerically to get the observed from the true.
     * Accuracy about 0.2' for -20C < T < +40C and 970mb < P < 1050mb.
     */
    /* Start iteration assuming correction = 0
     */
    y = trualt;
    D = 0.0;
    /* Invert Almanac for Computers formula numerically
     */
    P = (atpress - 80.0)/930.0;
    Q = 4.8e-3 * (attemp - 10.0);
    yy0 = y;
    D0 = D;
    for( i=0; i<4; i++ ) {
  	  N = y + (7.31/(y+4.4));
	  N = 1.0/tan(DEGTORAD*N);
	  D = N*P/(60.0 + Q * (N + 39.0));
	  N = y - yy0;
	  yy0 = D - D0 - N; /* denominator of derivative */
	  if( (N != 0.0) && (yy0 != 0.0) )
      /* Newton iteration with numerically estimated derivative */
	    N = y - N*(trualt + D - y)/yy0;
	  else
      /* Can't do it on first pass */
	    N = trualt + D;
	  yy0 = y;
	  D0 = D;
	  y = N;
	}
    return( trualt + D );
  } else {
#else
  /* another algorithm, from Meeus, German, p. 114ff.
   */
  double a, refr;
  double pt_factor = atpress / 1010.0 * 283.0 / (273.0 + attemp);
  double trualt, appalt;
  if (calc_flag == SE_TRUE_TO_APP) {
    trualt = inalt;
    if (trualt > 15) {
      a = tan((90 - trualt) * DEGTORAD);
      refr = (58.276 * a - 0.0824 * a * a * a);
      refr *=  pt_factor / 3600.0;
    } else if (trualt > -5) {
      /* the following tan is not defined for a value
       * of trualt near -5.00158 and 89.89158 */
      a = trualt + 10.3 / (trualt + 5.11);
      if (a + 1e-10 >= 90)
    	refr = 0;
      else
        refr = 1.02 / tan(a * DEGTORAD);
      refr *= pt_factor / 60.0;
    } else
      refr = 0;
    appalt = trualt;
    if (appalt + refr > 0)
      appalt += refr;
    return appalt;
  } else {
#endif
    double a, refr;
    double pt_factor = atpress / 1010.0 * 283.0 / (273.0 + attemp);
    double trualt, appalt;
    appalt = inalt;
    if (appalt > 15) {
      a = tan((90 - appalt) * DEGTORAD);
      refr = (58.294 * a - 0.0668 * a * a * a);
      refr *=  pt_factor / 3600.0;
    } else {
      /* the following tan is not defined for a value
       * of inalt near -4.3285 and 89.9225 */
      a = appalt + 7.31 / (appalt + 4.4);
      if (a + 1e-10 >= 90)
    	refr = 0;
      else {
        refr = 1.00 / tan(a * DEGTORAD);
    	refr -= 0.06 * sin(14.7 * refr + 13);
      }
      refr *= pt_factor / 60.0;
    }
    trualt = appalt;
    if (trualt - refr > 0)
      trualt -= refr;
    return trualt;
  }
}

/* Computes attributes of a lunar eclipse for given tjd and geopos
 * 
 * retflag	SE_ECL_TOTAL or SE_ECL_PARTIAL
 *              SE_ECL_PENUMBRAL
 *              if 0, there is no eclipse
 * 
 * attr[0]	umbral magnitude at tjd
 * attr[1]      penumbral magnitude
#if 0	 not implemented so far
 * attr[4]	azimuth of moon at tjd
 * attr[5]	true altitude of moon above horizon at tjd
 * attr[6]	apparent altitude of moon above horizon at tjd
#endif
 * attr[7]	distance of moon from opposition in degrees
 *         declare as attr[20] at least !
 * 
 */
int32 FAR PASCAL_CONV swe_lun_eclipse_how(
          double tjd_ut, 
          int32 ifl,
          double *geopos, 
          double *attr, 
          char *serr)
{
  double dcore[10];
  /* attention: geopos[] is not used so far; may be NULL */
  return lun_eclipse_how(tjd_ut, ifl, attr, dcore, serr);
}

/*
 * attr[]: 	see swe_lun_eclipse_how()
 *
 * dcore[0]:	distance of shadow axis from geocenter r0
 * dcore[1]:	diameter of core shadow on fundamental plane d0
 * dcore[2]:	diameter of half-shadow on fundamental plane D0
 */
static int32 lun_eclipse_how(
          double tjd_ut, 
          int32 ifl,
          double *attr, 
          double *dcore, 
          char *serr)
{
  int i;
  int32 retc = 0;
  double e[6], rm[6], rs[6];
  double dsm, d0, D0, s0, r0, ds, dm;
  double dctr, x1[6], x2[6];
  double f1, f2;
  double deltat, tjd;
  double cosf1, cosf2;
  int32 iflag;
  for (i = 0; i < 10; i++) 
    dcore[i] = 0;
  for (i = 0; i < 20; i++) 
    attr[i] = 0;
  /* nutation need not be in lunar and solar positions,
   * if mean sidereal time will be used */
  iflag = SEFLG_SPEED | SEFLG_EQUATORIAL | ifl;
  iflag  = iflag | SEFLG_XYZ;
  deltat = swe_deltat(tjd_ut);
  tjd = tjd_ut + deltat;
  /* moon in cartesian coordiantes */
  if (swe_calc(tjd, SE_MOON, iflag, rm, serr) == ERR)
    return ERR;
  /* distance of moon from geocenter */
  dm = sqrt(square_sum(rm));
  /* sun in cartesian coordiantes */
  if (swe_calc(tjd, SE_SUN, iflag, rs, serr) == ERR)
    return ERR;
  /* distance of sun from geocenter */
  ds = sqrt(square_sum(rs));
  for (i = 0; i < 3; i++) {
    x1[i] = rs[i] / ds;
    x2[i] = rm[i] / dm;
  }
  dctr = acos(swi_dot_prod_unit(x1, x2)) * RADTODEG;
  /* selenocentric sun */
  for (i = 0; i <= 2; i++)
    rs[i] -= rm[i];
  /* selenocentric earth */
  for (i = 0; i <= 2; i++)
    rm[i] = -rm[i];
  /* sun - earth vector */
  for (i = 0; i <= 2; i++)
    e[i] = (rm[i] - rs[i]);
  /* distance sun - earth */
  dsm = sqrt(square_sum(e));
  /* sun - earth unit vector */
  for (i = 0; i <= 2; i++)
    e[i] /= dsm;
  f1 = ((RSUN - REARTH) / dsm);
  cosf1 = sqrt(1 - f1 * f1);
  f2 = ((RSUN + REARTH) / dsm);
  cosf2 = sqrt(1 - f2 * f2);
  /* distance of earth from fundamental plane */
  s0 = -dot_prod(rm, e);
  /* distance of shadow axis from selenocenter */
  r0 = sqrt(dm * dm - s0 * s0);
  /* diameter of core shadow on fundamental plane */
  d0 = fabs(s0 / dsm * (DSUN - DEARTH) - DEARTH) * (1 + 1.0 / 50) / cosf1;
         /* one 50th is added for effect of atmosphere, AA98, L4 */
  /* diameter of half-shadow on fundamental plane */
  D0 = (s0 / dsm * (DSUN + DEARTH) + DEARTH) * (1 + 1.0 / 50) / cosf2;
  d0 /= cosf1;
  D0 /= cosf2;
  dcore[0] = r0;
  dcore[1] = d0;
  dcore[2] = D0;
  dcore[3] = cosf1;
  dcore[4] = cosf2;
  /**************************
   * phase and umbral magnitude
   **************************/
  retc = 0;
  if (d0 / 2 >= r0 + RMOON / cosf1) {
    retc = SE_ECL_TOTAL;
    attr[0] = (d0 / 2 - r0 + RMOON) / DMOON;
  } else if (d0 / 2 >= r0 - RMOON / cosf1) {
    retc = SE_ECL_PARTIAL;
    attr[0] = (d0 / 2 - r0 + RMOON) / DMOON;
  } else if (D0 / 2 >= r0 - RMOON / cosf2) {
    retc = SE_ECL_PENUMBRAL;
    attr[0] = 0;
  } else {
    if (serr != NULL)
      sprintf(serr, "no lunar eclipse at tjd = %f", tjd);
  }
  /**************************
   * penumbral magnitude
   **************************/
  attr[1] = (D0 / 2 - r0 + RMOON) / DMOON;
  if (retc != 0)
    attr[7] = 180 - fabs(dctr);
  return retc;
}

/* When is the next lunar eclipse?
 *
 * retflag	SE_ECL_TOTAL or SE_ECL_PENUMBRAL or SE_ECL_PARTIAL
 *
 * tret[0]	time of maximum eclipse
 * tret[1]	
 * tret[2]	time of partial phase begin (indices consistent with solar eclipses)
 * tret[3]	time of partial phase end
 * tret[4]	time of totality begin
 * tret[5]	time of totality end
 * tret[6]	time of penumbral phase begin
 * tret[7]	time of penumbral phase end
 */
int32 FAR PASCAL_CONV swe_lun_eclipse_when(double tjd_start, int32 ifl, int32 ifltype,
     double *tret, AS_BOOL backward, char *serr)
{
  int i, j, m, n, o, i1, i2;
  int32 retflag = 0, retflag2 = 0;
  double t, tjd, dt, dtint, dta, dtb;
  double T, T2, T3, T4, K, F, M, Mm;
  double E, Ff, F1, A1, Om;
  double xs[6], xm[6], dm, ds;
  double rsun, rearth, dcore[10];
  double dc[3], dctr;
  double twohr = 2.0 / 24.0;
  double tenmin = 10.0 / 24.0 / 60.0;
  double dt1, dt2;
  double kk;
  double attr[20];
  double dtstart, dtdiv;
  double xa[6], xb[6];
  int direction = 1;
  int32 iflag = SEFLG_EQUATORIAL | ifl;
  int32 iflagcart = iflag | SEFLG_XYZ;
  if (ifltype == 0)
    ifltype = SE_ECL_TOTAL | SE_ECL_PENUMBRAL | SE_ECL_PARTIAL;
  if (backward)
    direction = -1;
  K = (int) ((tjd_start - J2000) / 365.2425 * 12.3685);
  K -= direction;
next_try:
  retflag = 0;
  for (i = 0; i <= 9; i++)
    tret[i] = 0;
  kk = K + 0.5;
  T = kk / 1236.85;
  T2 = T * T; T3 = T2 * T; T4 = T3 * T;
  Ff = F = swe_degnorm(160.7108 + 390.67050274 * kk
               - 0.0016341 * T2
               - 0.00000227 * T3
               + 0.000000011 * T4);
  if (Ff > 180)
    Ff -= 180;
  if (Ff > 21 && Ff < 159) { 	/* no eclipse possible */
    K += direction;
    goto next_try;
  }
  /* approximate time of geocentric maximum eclipse
   * formula from Meeus, German, p. 381 */
  tjd = 2451550.09765 + 29.530588853 * kk
                      + 0.0001337 * T2
                      - 0.000000150 * T3
                      + 0.00000000073 * T4;
  M = swe_degnorm(2.5534 + 29.10535669 * kk
                      - 0.0000218 * T2
                      - 0.00000011 * T3);
  Mm = swe_degnorm(201.5643 + 385.81693528 * kk
                      + 0.1017438 * T2
                      + 0.00001239 * T3
                      + 0.000000058 * T4);
  Om = swe_degnorm(124.7746 - 1.56375580 * kk
                      + 0.0020691 * T2
                      + 0.00000215 * T3);
  E = 1 - 0.002516 * T - 0.0000074 * T2;
  A1 = swe_degnorm(299.77 + 0.107408 * kk - 0.009173 * T2);
  M *= DEGTORAD;
  Mm *= DEGTORAD;
  F *= DEGTORAD;
  Om *= DEGTORAD;
  F1 = F - 0.02665 * sin(Om) * DEGTORAD;
  A1 *= DEGTORAD;
  tjd = tjd - 0.4075 * sin(Mm)
            + 0.1721 * E * sin(M)
            + 0.0161 * sin(2 * Mm)
            - 0.0097 * sin(2 * F1)
            + 0.0073 * E * sin(Mm - M)
            - 0.0050 * E * sin(Mm + M)
            - 0.0023 * sin(Mm - 2 * F1)
            + 0.0021 * E * sin(2 * M)
            + 0.0012 * sin(Mm + 2 * F1)
            + 0.0006 * E * sin(2 * Mm + M)
            - 0.0004 * sin(3 * Mm)
            - 0.0003 * E * sin(M + 2 * F1)
            + 0.0003 * sin(A1)
            - 0.0002 * E * sin(M - 2 * F1)
            - 0.0002 * E * sin(2 * Mm - M)
            - 0.0002 * sin(Om);
  /* 
   * precise computation:
   * time of maximum eclipse (if eclipse) = 
   * minimum selenocentric angle between sun and earth edges.
   * After this time has been determined, check
   * whether or not an eclipse is taking place with
   * the function lun_eclipse_how().
   */
  dtstart = 0.1;
  if (tjd < 2000000)
    dtstart = 5;
  dtdiv = 4;
  for (j = 0, dt = dtstart;
       dt > 0.001; 
       j++, dt /= dtdiv) {
    for (i = 0, t = tjd - dt; i <= 2; i++, t += dt) {
      if (swe_calc(t, SE_SUN, iflagcart, xs, serr) == ERR)
        return ERR;
      if (swe_calc(t, SE_MOON, iflagcart, xm, serr) == ERR)
        return ERR;
      for (m = 0; m < 3; m++) {
        xs[m] -= xm[m];	/* selenocentric sun */
        xm[m] = -xm[m];	/* selenocentric earth */
      }
      ds = sqrt(square_sum(xs));
      dm = sqrt(square_sum(xm));
      for (m = 0; m < 3; m++) {
        xa[m] = xs[m] / ds;
        xb[m] = xm[m] / dm;
      }
      dc[i] = acos(swi_dot_prod_unit(xa, xb)) * RADTODEG;
      rearth = asin(REARTH / dm) * RADTODEG;
      rsun = asin(RSUN / ds) * RADTODEG;
      dc[i] -= (rearth + rsun);
    }
    find_maximum(dc[0], dc[1], dc[2], dt, &dtint, &dctr);
    tjd += dtint + dt;
  }
  tjd = tjd - swe_deltat(tjd);
  if ((retflag = swe_lun_eclipse_how(tjd, ifl, NULL, attr, serr)) == ERR)
    return retflag;
  if (retflag == 0) {
    K += direction;
    goto next_try;
  }
  tret[0] = tjd;
  if ((backward && tret[0] >= tjd_start - 0.0001) 
    || (!backward && tret[0] <= tjd_start + 0.0001)) {
    K += direction;
    goto next_try;
  }
  /* 
   * check whether or not eclipse type found is wanted
   */
  /* non penumbral eclipse is wanted: */
  if (!(ifltype & SE_ECL_PENUMBRAL) && (retflag & SE_ECL_PENUMBRAL)) {
    K += direction;
    goto next_try;
  }
  /* non partial eclipse is wanted: */
  if (!(ifltype & SE_ECL_PARTIAL) && (retflag & SE_ECL_PARTIAL)) {
    K += direction;
    goto next_try;
  }
  /* annular-total eclipse will be discovered later */
  if (!(ifltype & (SE_ECL_TOTAL)) && (retflag & SE_ECL_TOTAL)) {
    K += direction;
    goto next_try;
  }
  /* 
   * n = 0: times of eclipse begin and end 
   * n = 1: times of totality begin and end 
   * n = 2: times of center line begin and end 
   */
  if (retflag & SE_ECL_PENUMBRAL)
    o = 0;
  else if (retflag & SE_ECL_PARTIAL)
    o = 1;
  else 
    o = 2;
  dta = twohr;
  dtb = tenmin;
  for (n = 0; n <= o; n++) {
    if (n == 0) {
      i1 = 6; i2 = 7;
    } else if (n == 1) {
      i1 = 2; i2 = 3;
    } else if (n == 2) {
      i1 = 4; i2 = 5;
    }
#if 1
    for (i = 0, t = tjd - dta; i <= 2; i += 1, t += dta) {
      if ((retflag2 = lun_eclipse_how(t, ifl, attr, dcore, serr)) == ERR)
        return retflag2;
      if (n == 0)
        dc[i] = dcore[2] / 2 + RMOON / dcore[4] - dcore[0];
      else if (n == 1)
        dc[i] = dcore[1] / 2 + RMOON / dcore[3] - dcore[0];
      else if (n == 2)
        dc[i] = dcore[1] / 2 - RMOON / dcore[3] - dcore[0];
    }
    find_zero(dc[0], dc[1], dc[2], dta, &dt1, &dt2);
    dtb = (dt1 + dta) / 2;
    tret[i1] = tjd + dt1 + dta;
    tret[i2] = tjd + dt2 + dta;
#else
    tret[i1] = tjd - dtb;
    tret[i2] = tjd + dtb;
#endif
    for (m = 0, dt = dtb / 2; m < 3; m++, dt /= 2) {
      for (j = i1; j <= i2; j += (i2 - i1)) {
        for (i = 0, t = tret[j] - dt; i < 2; i++, t += dt) {
          if ((retflag2 = lun_eclipse_how(t, ifl, attr, dcore, serr)) == ERR)
            return retflag2;
          if (n == 0)
            dc[i] = dcore[2] / 2 + RMOON / dcore[4] - dcore[0];
          else if (n == 1)
            dc[i] = dcore[1] / 2 + RMOON / dcore[3] - dcore[0];
          else if (n == 2)
            dc[i] = dcore[1] / 2 - RMOON / dcore[3] - dcore[0];
        }
        dt1 = dc[1] / ((dc[1] - dc[0]) / dt);
        tret[j] -= dt1;
      }
    }
  }
  return retflag;
}

/* 
 * function calculates planetary phenomena
 * 
 * attr[0] = phase angle (earth-planet-sun)
 * attr[1] = phase (illumined fraction of disc)
 * attr[2] = elongation of planet
 * attr[3] = apparent diameter of disc
 * attr[4] = apparent magnitude
 *         declare as attr[20] at least !
 *
 * Note: the lunar magnitude is quite a complicated thing,
 * but our algorithm is very simple. 
 * The phase of the moon, its distance from the earth and
 * the sun is considered, but no other factors.
 * 
 */
#define EULER 2.718281828459
#define NMAG_ELEM  (SE_VESTA + 1)
static double mag_elem[NMAG_ELEM][4] = {
                /* DTV-Atlas Astronomie, p. 32 */
                {-26.86, 0, 0, 0},
                {-12.55, 0, 0, 0},
                /* IAU 1986 */
                {-0.42, 3.80, -2.73, 2.00},
                {-4.40, 0.09, 2.39, -0.65},
                {- 1.52, 1.60, 0, 0},   /* Mars */
                {- 9.40, 0.5, 0, 0},    /* Jupiter */
                {- 8.88, -2.60, 1.25, 0.044},   /* Saturn */
                {- 7.19, 0.0, 0, 0},    /* Uranus */
                {- 6.87, 0.0, 0, 0},    /* Neptune */
                {- 1.00, 0.0, 0, 0},    /* Pluto */
                {99, 0, 0, 0},          /* nodes and apogees */
                {99, 0, 0, 0},
                {99, 0, 0, 0},
                {99, 0, 0, 0},
                {99, 0, 0, 0},          /* Earth */
                /* from Bowell data base */
                {6.5, 0.15, 0, 0},      /* Chiron */
                {7.0, 0.15, 0, 0},      /* Pholus */
                {3.34, 0.12, 0, 0},     /* Ceres */
                {4.13, 0.11, 0, 0},     /* Pallas */
                {5.33, 0.32, 0, 0},     /* Juno */
                {3.20, 0.32, 0, 0},     /* Vesta */
                };
#define NDIAM  (SE_VESTA + 1)
static double diam[NDIAM] = {1392000000.0, /* Sun */
                           3476300.0, /* Moon */
                           2493000.0 * 2, /* Mercury */
                           6052000.0 * 2, /* Venus */
                           3397200.0 * 2, /* Mars */
                          71398000.0 * 2, /* Jupiter */
                          60000000.0 * 2, /* Saturn */
                          25400000.0 * 2, /* Uranus */
                          24300000.0 * 2, /* Neptune */
                           2500000.0 * 2, /* Pluto */
                           0, 0, 0, 0,    /* nodes and apogees */
                           6378140.0 * 2, /* Earth */
                                 0.0, /* Chiron */
                                 0.0, /* Pholus */
                            913000.0, /* Ceres */
                            523000.0, /* Pallas */
                            244000.0, /* Juno */
                            501000.0, /* Vesta */
                        };
int32 FAR PASCAL_CONV swe_pheno(double tjd, int32 ipl, int32 iflag, double *attr, char *serr)
{
  int i;
  double xx[6], xx2[6], xxs[6], lbr[6], lbr2[6], dt, dsm, dd;
  double fac;
  double T, in, om, sinB, u1, u2, du;
  double ph1, ph2, me[2];
  int32 iflagp;
  for (i = 0; i < 20; i++)
    attr[i] = 0;
  /* Ceres - Vesta must be SE_CERES etc., not 10001 etc. */
  if (ipl > SE_AST_OFFSET && ipl <= SE_AST_OFFSET + 4)
        ipl = ipl - SE_AST_OFFSET - 1 + SE_CERES;
  iflag = iflag & (SEFLG_EPHMASK | 
                   SEFLG_TRUEPOS | 
                   SEFLG_J2000 | 
                   SEFLG_NONUT |
                   SEFLG_NOGDEFL |
                   SEFLG_NOABERR |
                   SEFLG_TOPOCTR);
  iflagp = iflag & (SEFLG_EPHMASK | 
                   SEFLG_TRUEPOS | 
                   SEFLG_J2000 | 
                   SEFLG_NONUT |
                   SEFLG_NOABERR);
  iflagp |= SEFLG_HELCTR;                
  /*  
   * geocentric planet 
   */
  if (swe_calc(tjd, (int) ipl, iflag | SEFLG_XYZ, xx, serr) == ERR)
    /* int cast can be removed when swe_calc() gets int32 ipl definition */
    return ERR;
  if (swe_calc(tjd, (int) ipl, iflag, lbr, serr) == ERR)
    /* int cast can be removed when swe_calc() gets int32 ipl definition */
    return ERR;
  /* if moon, we need sun as well, for magnitude */
  if (ipl == SE_MOON)
    if (swe_calc(tjd, SE_SUN, iflag | SEFLG_XYZ, xxs, serr) == ERR)
      return ERR;
  if (ipl != SE_SUN && ipl != SE_EARTH &&
    ipl != SE_MEAN_NODE && ipl != SE_TRUE_NODE &&
    ipl != SE_MEAN_APOG && ipl != SE_OSCU_APOG) {
    /*
     * light time planet - earth
     */
    dt = lbr[2] * AUNIT / CLIGHT / 86400.0;     
        if (iflag & SEFLG_TRUEPOS)
      dt = 0;
    /* 
     * heliocentric planet at tjd - dt
     */
    if (swe_calc(tjd - dt, (int) ipl, iflagp | SEFLG_XYZ, xx2, serr) == ERR)
    /* int cast can be removed when swe_calc() gets int32 ipl definition */
      return ERR;
    if (swe_calc(tjd - dt, (int) ipl, iflagp, lbr2, serr) == ERR)
    /* int cast can be removed when swe_calc() gets int32 ipl definition */
      return ERR;
    /*
     * phase angle
     */
    attr[0] = acos(swi_dot_prod_unit(xx, xx2)) * RADTODEG;
    /*
     * phase
     */
    attr[1] = (1 + cos(attr[0] * DEGTORAD)) / 2;
  }
  /*
   * apparent diameter of disk
   */
  if (ipl < NDIAM)
    dd = diam[ipl];
  else if (ipl > SE_AST_OFFSET)
    dd = swed.ast_diam * 1000;	/* km -> m */
  else
    dd = 0;
  if (lbr[2] < dd / 2 / AUNIT)
    attr[3] = 180;  /* assume position on surface of earth */
  else
    attr[3] = asin(dd / 2 / AUNIT / lbr[2]) * 2 * RADTODEG;
  /* 
   * apparent magnitude 
   */
  if (ipl > SE_AST_OFFSET || (ipl < NMAG_ELEM && mag_elem[ipl][0] < 99)) {
    if (ipl == SE_SUN) {
      /* ratio apparent diameter : average diameter */
      fac = attr[3] / (asin(diam[SE_SUN] / 2.0 / AUNIT) * 2 * RADTODEG);
      fac *= fac;
      attr[4] = mag_elem[ipl][0] - 2.5 * log10(fac);
    } else if (ipl == SE_MOON) {
      /* ratio apparent diameter : average diameter */
      fac = attr[3] / (asin(diam[SE_MOON] / 2.0 / 384400000.0) * 2 * RADTODEG);
      /* distance sun - moon */
      for (i = 0; i < 3; i++)
        xxs[i] -= xx[i];
      dsm = sqrt(square_sum(xxs));
      /* account for phase and distance of moon: */
      fac *= fac * attr[1];
      /* account for distance of sun from moon: */
      fac *= dsm * dsm;
      attr[4] = mag_elem[ipl][0] - 2.5 * log10(fac);
    } else if (ipl == SE_SATURN) {
      /* rings are considered according to Meeus, German, p. 329ff. */
      T = (tjd - dt - J2000) / 36525.0;
      in = (28.075216 - 0.012998 * T + 0.000004 * T * T) * DEGTORAD;
      om = (169.508470 + 1.394681 * T + 0.000412 * T * T) * DEGTORAD;
      sinB = fabs(sin(in) * cos(lbr[1] * DEGTORAD) 
                    * sin(lbr[0] * DEGTORAD - om)
                    - cos(in) * sin(lbr[1] * DEGTORAD));
      u1 = atan2(sin(in) * tan(lbr2[1] * DEGTORAD) 
                             + cos(in) * sin(lbr2[0] * DEGTORAD - om), 
                        cos(lbr2[0] * DEGTORAD - om)) * RADTODEG;
      u2 = atan2(sin(in) * tan(lbr[1] * DEGTORAD) 
                             + cos(in) * sin(lbr[0] * DEGTORAD - om), 
                        cos(lbr[0] * DEGTORAD - om)) * RADTODEG;
      du = swe_degnorm(u1 - u2);
      if (du > 10) 
        du = 360 - du;
      attr[4] = 5 * log10(lbr2[2] * lbr[2])
                  + mag_elem[ipl][1] * sinB
                  + mag_elem[ipl][2] * sinB * sinB
                  + mag_elem[ipl][3] * du
                  + mag_elem[ipl][0];
    } else if (ipl < SE_CHIRON) {
      attr[4] = 5 * log10(lbr2[2] * lbr[2])
                  + mag_elem[ipl][1] * attr[0] /100.0
                  + mag_elem[ipl][2] * attr[0] * attr[0] / 10000.0
                  + mag_elem[ipl][3] * attr[0] * attr[0] * attr[0] / 1000000.0
                  + mag_elem[ipl][0];
    } else if (ipl < NMAG_ELEM || ipl > SE_AST_OFFSET) { /* asteroids */
      ph1 = pow(EULER, -3.33 * pow(tan(attr[0] * DEGTORAD / 2), 0.63));
      ph2 = pow(EULER, -1.87 * pow(tan(attr[0] * DEGTORAD / 2), 1.22));
      if (ipl < NMAG_ELEM) {    /* main asteroids */
        me[0] = mag_elem[ipl][0];
        me[1] = mag_elem[ipl][1];
      } else if (ipl == SE_AST_OFFSET + 1566) { 
                  /* Icarus has elements from JPL database */
                me[0] = 16.9;
                me[1] = 0.15;
      } else {      /* other asteroids */
        me[0] = swed.ast_G;
        me[1] = swed.ast_H;
      }
      attr[4] = 5 * log10(lbr2[2] * lbr[2])
          + me[0]
          - 2.5 * log10((1 - me[1]) * ph1 + me[1] * ph2);
    } else { /* ficticious bodies */
      attr[4] = 0;
    }
  }
  if (ipl != SE_SUN && ipl != SE_EARTH) {
    /* 
     * elongation of planet
     */
    if (swe_calc(tjd, SE_SUN, iflag | SEFLG_XYZ, xx2, serr) == ERR)
      return ERR;
    if (swe_calc(tjd, SE_SUN, iflag, lbr2, serr) == ERR)
      return ERR;
    attr[2] = acos(swi_dot_prod_unit(xx, xx2)) * RADTODEG;
  }
  return OK;
}

int32 FAR PASCAL_CONV swe_pheno_ut(double tjd_ut, int32 ipl, int32 iflag, double *attr, char *serr)
{
  return swe_pheno(tjd_ut + swe_deltat(tjd_ut), ipl, iflag, attr, serr);
}

static int find_maximum(double y00, double y11, double y2, double dx, 
                        double *dxret, double *yret)
{
  double a, b, c, x, y;
  c = y11;
  b = (y2 - y00) / 2.0;
  a = (y2 + y00) / 2.0 - c;
  x = -b / 2 / a;
  y = (4 * a * c - b * b) / 4 / a;
  *dxret = (x - 1) * dx;
  if (yret != NULL)
    *yret = y;
  return OK;
}

static int find_zero(double y00, double y11, double y2, double dx, 
                        double *dxret, double *dxret2)
{
  double a, b, c, x1, x2;
  c = y11;
  b = (y2 - y00) / 2.0;
  a = (y2 + y00) / 2.0 - c;
  if (b * b - 4 * a * c < 0) 
    return ERR;
  x1 = (-b + sqrt(b * b - 4 * a * c)) / 2 / a;
  x2 = (-b - sqrt(b * b - 4 * a * c)) / 2 / a;
    *dxret = (x1 - 1) * dx;
    *dxret2 = (x2 - 1) * dx;
  return OK;
}

/* rise, set, and meridian transits of sun, moon, planets, and stars
 *
 * tjd_ut	universal time from when on search ought to start
 * ipl		planet number, neglected, if starname is given
 * starname	pointer to string. if a planet, not a star, is
 *              wanted, starname must be NULL or ""
 * epheflag	used for ephemeris only
 * rsmi		SE_CALC_RISE, SE_CALC_SET, SE_CALC_MTRANSIT, SE_CALC_ITRANSIT
 * geopos	array of doubles for geogr. long., lat. and height above sea
 * atpress	atmospheric pressure
 * attemp	atmospheric temperature
 *
 * return variables:
 * tret         time of rise, set, meridian transits
 * serr[256]	error string
 */
#define SEFLG_EPHMASK	(SEFLG_JPLEPH|SEFLG_SWIEPH|SEFLG_MOSEPH)

int32 FAR PASCAL_CONV swe_rise_trans(
               double tjd_ut, int32 ipl, char *starname,
	       int32 epheflag, int32 rsmi,
               double *geopos, 
	       double atpress, double attemp,
               double *tret,
               char *serr)
{
  int i, j, k, ii, calc_culm, nculm = -1;
  double tjd_et = tjd_ut + swe_deltat(tjd_ut);
  double xc[6], xh[20][6], ah[6], aha;
  double tculm[4], tcu, tc[20], h[20], t2[6], dc[6], dtint, dx, rdi, dd;
  int32 iflag = epheflag;
  int jmax = 14;
  AS_BOOL has_set = FALSE, has_rise = FALSE;
  double t, te, tt, dt, twohrs = 1.0 / 12.0;
  AS_BOOL do_fixstar = (starname != NULL && *starname != '\0');
  iflag &= SEFLG_EPHMASK;
  *tret = 0;
  iflag |= (SEFLG_EQUATORIAL | SEFLG_TOPOCTR);
  swe_set_topo(geopos[0], geopos[1], geopos[2]);
  if (rsmi & (SE_CALC_MTRANSIT | SE_CALC_ITRANSIT))
    return calc_mer_trans(tjd_ut, ipl, epheflag, rsmi, 
		geopos, starname, tret, serr);
  if (!(rsmi & (SE_CALC_RISE | SE_CALC_SET))) {
    rsmi |= SE_CALC_RISE;
  }
  /* find culmination points within 28 hours from t0 - twohrs.
   * culminations are required in case there are maxima or minima
   * in height slightly above or below the horizon.
   * we do not use meridian transits, because in polar regions
   * the culmination points may considerably deviate from
   * transits. also, there are cases where the moon rises in the
   * western half of the sky for a short time. 
   */
  if (do_fixstar) {
    if (swe_fixstar(starname, tjd_et, iflag, xc, serr) == ERR)
      return ERR;
  } 
  for (ii = 0, t = tjd_ut - twohrs; ii <= jmax; ii++, t += twohrs) {
    tc[ii] = t;
    if (!do_fixstar) {
      te = t + swe_deltat(t);
      if (swe_calc(te, ipl, iflag, xc, serr) == ERR)
        return ERR;
    }
    /* diameter of object in km */
    if (ii == 0) {
      if (do_fixstar) 
        dd = 0;
      else if (rsmi & SE_BIT_DISC_CENTER)
        dd = 0;
      else if (ipl < NDIAM)
        dd = diam[ipl];
      else if (ipl > SE_AST_OFFSET)
        dd = swed.ast_diam * 1000;	/* km -> m */
      else
        dd = 0;
    }
    /* apparent radius of disc */
    rdi = asin(dd / 2 / AUNIT / xc[2]) * RADTODEG;
    /* true height of center of body */
    swe_azalt(t, SE_EQU2HOR, geopos, atpress, attemp, xc, xh[ii]);
    /* true height of uppermost point of body */
    xh[ii][1] += rdi;
    /* apparent height of uppermost point of body */
    if (rsmi & SE_BIT_NO_REFRACTION) {
      h[ii] = xh[ii][1];
    } else {
      swe_azalt_rev(t, SE_HOR2EQU, geopos, xh[ii], xc);
      swe_azalt(t, SE_EQU2HOR, geopos, atpress, attemp, xc, xh[ii]);
      h[ii] = xh[ii][2];
    }
    calc_culm = 0;
    if (ii > 1) {
      dc[0] = xh[ii-2][1];
      dc[1] = xh[ii-1][1];
      dc[2] = xh[ii][1];
      if (dc[1] > dc[0] && dc[1] > dc[2])
        calc_culm = 1;
      if (dc[1] < dc[0] && dc[1] < dc[2])
        calc_culm = 2;
    }
    if (calc_culm) {
      dt = twohrs;
      tcu = t - dt;
      find_maximum(dc[0], dc[1], dc[2], dt, &dtint, &dx);
      tcu += dtint + dt;
      dt /= 3;
      for (; dt > 0.0001; dt /= 3) {
        for (i = 0, tt = tcu - dt; i < 3; tt += dt, i++) {
          te = tt + swe_deltat(tt);
          if (!do_fixstar)
            if (swe_calc(te, ipl, iflag, xc, serr) == ERR)
              return ERR;
          swe_azalt(tt, SE_EQU2HOR, geopos, atpress, attemp, xc, ah);
          dc[i] = ah[1];
        }
        find_maximum(dc[0], dc[1], dc[2], dt, &dtint, &dx);
        tcu += dtint + dt;
      }
      nculm++;
      tculm[nculm] = tcu;
    }
  }
  /* note: there can be a rise or set on the poles, even if 
   * there is no culmination. So, we must not leave here
   * in any case. */
  /* insert culminations into array of heights */
  for (i = 0; i <= nculm; i++) {
    for (j = 1; j <= jmax; j++) {
      if (tculm[i] < tc[j]) {
        for (k = jmax; k >= j; k--) {
          tc[k+1] = tc[k];
          h[k+1] = h[k];
        }
        tc[j] = tculm[i];
        if (!do_fixstar) {
          te = tc[j] + swe_deltat(tc[j]);
          if (swe_calc(te, ipl, iflag, xc, serr) == ERR)
            return ERR;
        }
        /* apparent radius of disc */
        rdi = asin(dd / 2 / AUNIT / xc[2]) * RADTODEG;
        /* true height of center of body */
        swe_azalt(tc[j], SE_EQU2HOR, geopos, atpress, attemp, xc, ah);
        /* true height of uppermost point of body */
        ah[1] += rdi;
        /* apparent height of uppermost point of body */
	if (rsmi & SE_BIT_NO_REFRACTION) {
	  h[j] = ah[1];
	} else {
	  swe_azalt_rev(tc[j], SE_HOR2EQU, geopos, ah, xc);
	  swe_azalt(tc[j], SE_EQU2HOR, geopos, atpress, attemp, xc, ah);
	  h[j] = ah[2];
	}
        jmax++;
        break;
      }
    }
  }
  *tret = 0;
  /* find points with zero height. 
   * binary search */
  for (ii = 1; ii <= jmax; ii++) {
    if (h[ii-1] * h[ii] >= 0)
      continue;
    if (h[ii-1] < h[ii] && !(rsmi & SE_CALC_RISE))
      continue;
    if (h[ii-1] > h[ii] && !(rsmi & SE_CALC_SET))
      continue;
    dc[0] = h[ii-1];
    dc[1] = h[ii];
    t2[0] = tc[ii-1];
    t2[1] = tc[ii];
    for (i = 0; i < 20; i++) {
      t = (t2[0] + t2[1]) / 2;
      if (!do_fixstar) {
        te = t + swe_deltat(t);
        if (swe_calc(te, ipl, iflag, xc, serr) == ERR)
          return ERR;
      }
      /* apparent radius of disc */
      rdi = asin(dd / 2 / AUNIT / xc[2]) * RADTODEG;
      /* true height of center of body */
      swe_azalt(t, SE_EQU2HOR, geopos, atpress, attemp, xc, ah);
      /* true height of uppermost point of body */
      ah[1] += rdi;
      /* apparent height of uppermost point of body */
      if (rsmi & SE_BIT_NO_REFRACTION) {
	aha = ah[1];
      } else {
	swe_azalt_rev(t, SE_HOR2EQU, geopos, ah, xc);
	swe_azalt(t, SE_EQU2HOR, geopos, atpress, attemp, xc, ah);
	aha = ah[2];
      }
      if (aha * dc[0] <= 0) {
        dc[1] = aha;
        t2[1] = t;
      } else {
        dc[0] = aha;
        t2[0] = t;
      }
    }
    if (t > tjd_ut) {
     *tret = t;
     return OK;
    }
  }
  return OK;
}

static int32 calc_mer_trans(
               double tjd_ut, int32 ipl, int32 epheflag, int32 rsmi,
               double *geopos,
               char *starname, 
               double *tret,
               char *serr)
{
  int i;
  double tjd_et = tjd_ut + swe_deltat(tjd_ut);
  double armc, armc0, arxc, x0[6], x[6], t, te;
  double r0;
  double mdd;
  int32 iflag = epheflag;
  int mmax = 3;
  AS_BOOL do_fixstar = (starname != NULL && *starname != '\0');
  iflag &= SEFLG_EPHMASK;
  *tret = 0;
  iflag |= (SEFLG_EQUATORIAL | SEFLG_TOPOCTR);
  armc0 = swe_sidtime(tjd_ut) + geopos[0] / 15;
  if (armc0 >= 24)
    armc0 -= 24;
  if (armc0 < 0)
    armc0 += 24;
  armc0 *= 15;
  if (do_fixstar) {
    if (swe_fixstar(starname, tjd_et, iflag, x0, serr) == ERR)
      return ERR;
  } else {
    if (swe_calc(tjd_et, ipl, iflag, x0, serr) == ERR)
      return ERR;
  }
  r0 = x0[2];
  /*
   * meridian transits
   */
    x[0] = x0[0];
    x[1] = x0[1];
    t = tjd_ut;
    arxc = armc0;
    if (rsmi & SE_CALC_ITRANSIT)
      arxc = swe_degnorm(arxc + 180);
    for (i = 0; i < 4; i++) {
      mdd = swe_degnorm(x[0] - arxc);
      if (i > 0 && mdd > 180)
        mdd -= 360;
      t += mdd / 361;
      armc = swe_sidtime(t) + geopos[0] / 15;
      if (armc >= 24)
        armc -= 24;
      if (armc < 0)
        armc += 24;
      armc *= 15;
	  arxc = armc;
      if (rsmi & SE_CALC_ITRANSIT)
        arxc = swe_degnorm(arxc + 180);
      if (!do_fixstar) {
        te = t + swe_deltat(t);
        if (swe_calc(te, ipl, iflag, x, serr) == ERR)
          return ERR;
      }
    }
  *tret = t;
  return OK;
}

/*
Nodes and apsides of planets and moon

Planetary nodes can be defined in three different ways:
a) They can be understood as a direction or as an axis 
  defined by the intersection line of two orbital planes. 
  E.g., the nodes of Mars are defined by the intersection 
  line of Mars' orbital plane with the ecliptic (= the 
  Earths orbit heliocentrically or the solar orbit 
  geocentrically). However, as Michael Erlewine points 
  out in his elaborate web page on this topic 
  (http://thenewage.com/resources/articles/interface.html),
  planetary nodes can be defined for any couple of 
  planets. E.g. there is also an intersection line for the 
  two orbital planes of Mars and Saturn.
  Because such lines are, in principle, infinite, the 
  heliocentric and the geocentric positions of the 
  planetary nodes will be the same. There are astrologers 
  that use such heliocentric planetary nodes in geocentric 
  charts.
  The ascending and the descending node will, in this 
  case, be in precise opposition.

b) The planetary nodes can also be understood in a 
  different way, not as an axis, but as the two points on a 
  planetary orbit that are located precisely on the 
  intersection line of the two planes.
  This second definition makes no difference for the moon or 
  for heliocentric positions of planets, but it does so for 
  geocentric positions. There are two possibilities for 
  geocentric planetary nodes based on this definition. 
  1) The common solution is that the points on the 
    planets orbit are transformed to the geocenter. The 
    two points will not be in opposition anymore, or 
    they will only roughly be so with the outer planets. The 
    advantage of these nodes is that when a planet is in 
    conjunction with its node, then its ecliptic latitude 
    will be zero. This is not true when a planet is in 
    geocentric conjunction with its heliocentric node. 
    (And neither is it always true for the inner planets, 
    i.e. Mercury and Venus.)
  2) The second possibility that nobody seems to have 
    thought of so far: One may compute the points of 
    the earth's orbit that lie exactly on another planet's 
    orbital plane and transform it to the geocenter. The two 
    points will always be in an approximate square.

c) Third, the planetary nodes could be defined as the 
  intersection points of the plane defined by their 
  momentary geocentric position and motion with the 
  plane of the ecliptic. Such points would move very fast 
  around the planetary stations. Here again, as in b)1), 
  the planet would cross the ecliptic and its ecliptic 
  latitude would be 0 exactly when it were in 
  conjunction with one of its nodes.

The Swiss Ephemeris supports the solutions a) and b) 1).

Possible definitions for apsides

a) The planetary apsides can be defined as the perihelion and
  aphelion points on a planetary orbit. For a
  geocentric chart, these points could be transformed 
  from the heliocenter to the geocenter.
b) However, one might consider these points as 
  astrologically relevant axes rather than as points on a 
  planetary orbit. Again, this would allow heliocentric 
  positions in a geocentric chart.

Note: For the "Dark Moon" or "Lilith", which I usually 
define as the lunar apogee, some astrologers give a 
different definition. They understand it as the second focal 
point of the moon's orbital ellipse. This definition does not 
make a difference for geocentric positions, because the 
apogee and the second focus are in exactly the same geocentric 
direction. However, it makes a difference with topocentric 
positions, because the two points do not have same distance. 
Analogous "black planets" have been proposed: they would be the 
second focal points of the planets' orbital ellipses. The 
heliocentric positions of these "black planets" are identical 
with the heliocentric positions of the aphelia, but geocentric 
positions are not identical, because the focal points are 
much closer to the sun than the aphelia.

The Swiss Ephemeris allows to compute the "black planets" as well.

Mean positions

Mean nodes and apsides can be computed for the Moon, the 
Earth and the planets Mercury - Neptune. They are taken 
from the planetary theory VSOP87. Mean points can not be 
calculated for Pluto and the asteroids, because there is no 
planetary theory for them.
 
Osculating nodes and apsides

Nodes and apsides can also be derived from the osculating 
orbital elements of a body, the paramaters that define an  
ideal unperturbed elliptic (two-body) orbit. 
For astrology, note that this is a simplification and 
idealization. 
Problem with Neptune: Neptune's orbit around the sun does not 
have much in common with an ellipse. There are often two 
perihelia and two aphelia within one revolution. As a result, 
there is a wild oscillation of the osculating perihelion (and 
aphelion). 
In actuality, Neptune's orbit is not heliocentric orbit at all. 
The twofold perihelia and aphelia are an effect of the motion of 
the sun about the solar system barycenter. This motion is 
much faster than the motion of Neptune, and Neptune 
cannot react on such fast displacements of the Sun. As a 
result, Neptune seems to move around the barycenter (or a 
mean sun) rather than around the true sun. In fact, 
Neptune's orbit around the barycenter is therefore closer to 
an ellipse than the his orbit around the sun. The same 
statement is also true for Saturn, Uranus and Pluto, but not 
for Jupiter and the inner planets.

This fundamental problem about osculating ellipses of 
planetary orbits does of course not only affect the apsides 
but also the nodes.

Two solutions can be thought of for this problem: 
1) The one would be to interpolate between actual 
  passages of the planets through their nodes and 
  apsides. However, this works only well with Mercury. 
  With all other planets, the supporting points are too far 
  apart as to make an accurate interpolation possible. 
  This solution is not implemented, here.
2) The other solution is to compute the apsides of the 
  orbit around the barycenter rather than around the sun. 
  This procedure makes sense for planets beyond Jupiter, 
  it comes closer to the mean apsides and nodes for 
  planets that have such points defined. For all other 
  transsaturnian planets and asteroids, this solution yields 
  a kind of "mean" nodes and apsides. On the other hand, 
  the barycentric ellipse does not make any sense for 
  inner planets and Jupiter.

The Swiss Ephemeris supports solution 2) for planets and 
asteroids beyond Jupiter.

Anyway, neither the heliocentric nor the barycentric ellipse 
is a perfect representation of the nature of a planetary orbit, 
and it will not yield the degree of precision that today's 
astrology is used to.
The best choice of method will probably be:
- For Mercury - Neptune: mean nodes and apsides
- For asteroids that belong to the inner asteroid belt: 
osculating nodes/apsides from a heliocentric ellipse
- For Pluto and outer asteroids: osculating nodes/apsides 
from a barycentric ellipse

The Moon is a special case: A "lunar true node" makes 
more sense, because it can be defined without the idea of an 
ellipse, e.g. as the intersection axis of the momentary lunar 
orbital plane with the ecliptic. Or it can be said that the 
momentary motion of the moon points to one of the two 
ecliptic points that are called the "true nodes".  So, these 
points make sense. With planetary nodes, the situation is 
somewhat different, at least if we make a difference 
between heliocentric and geocentric positions. If so, the 
planetary nodes are points on a heliocentric orbital ellipse, 
which are transformed to the geocenter. An ellipse is 
required here, because a solar distance is required. In 
contrast to the planetary nodes, the lunar node does not 
require a distance, therefore manages without the idea of an 
ellipse and does not share its weaknesses. 
On the other hand, the lunar apsides DO require the idea of 
an ellipse. And because the lunar ellipse is actually 
extremely distorted, even more than any other celestial 
ellipse, the "true Lilith" (apogee), for which printed 
ephemerides are available, does not make any sense at all. 
(See the chapter on the lunar node and apogee.)

Special case: the Earth

The Earth is another special case. Instead of the motion of 
the Earth herself, the heliocentric motion of the Earth-
Moon-Barycenter (EMB) is used to determine the 
osculating perihelion. 
There is no node of the earth orbit itself. However, there is 
an axis around which the earth's orbital plane slowly rotates 
due to planetary precession. The position points of this axis 
are not calculated by the Swiss Ephemeris.

Special case: the Sun

In addition to the Earth (EMB) apsides, the function 
computes so-to-say "apsides" of the sun, i.e. points on the 
orbit of the Sun where it is closest to and where it is farthest 
from the Earth. These points form an opposition and are 
used by some astrologers, e.g. by the Dutch astrologer 
George Bode or the Swiss astrologer Liduina Schmed. The 
perigee, located at about 13 Capricorn, is called the 
"Black Sun", the other one, in Cancer, the "Diamond".
So, for a complete set of apsides, one ought to calculate 
them for the Sun and the Earth and all other planets. 

The modes of the Swiss Ephemeris function 
swe_nod_aps()

The  function swe_nod_aps() can be run in the following 
modes:
1) Mean positions are given for nodes and apsides of Sun, 
  Moon, Earth, and the up to Neptune. Osculating 
  positions are given with Pluto and all asteroids. This is 
  the default mode.
2) Osculating positions are returned for nodes and apsides 
  of all planets.
3) Same as 2), but for planets and asteroids beyond 
  Jupiter, a barycentric ellipse is used.
4) Same as 1), but for Pluto and asteroids beyond Jupiter, 
  a barycentric ellipse is used.

In all of these modes, the second focal point of the ellipse 
can be computed instead of the aphelion.
Like the planetary function swe_calc(), swe_nod_aps() is 
able to return geocentric, topocentric, heliocentric, or 
barycentric position.
 *
 * tjd_ut	 julian day, ephemeris time
 * ipl		 planet number
 * iflag	 as usual, SEFLG_HELCTR, etc.
 * xnasc         an array of 6 doubles: ascending node
 * xndsc         an array of 6 doubles: ascending node
 * xperi         an array of 6 doubles: perihelion
 * xaphe         an array of 6 doubles: aphelion
 * method        see below
 * serr          error message
 *
 * method        can have the following values:
 *               - 0 or SE_NODBIT_MEAN. MEAN positions are given for
 *                 nodes and apsides of Sun, Moon, Earth, and the 
 *                 planets up to Neptune. Osculating positions are
 *                 given with Pluto and all asteroids.
 *               - SE_NODBIT_OSCU. Osculating positions are given
 *                 for all nodes and apsides.
 *               - SE_NODBIT_OSCU_BAR. Osculating nodes and apsides
 *                 are computed from barycentric ellipses, for planets
 *                 beyond Jupiter, but from heliocentric ones for
 *                 ones for Jupiter and inner planets.
 *               - SE_NODBIT_MEAN and SE_NODBIT_OSCU_BAR can be combined.
 *                 The program behaves the same way as with simple 
 *                 SE_NODBIT_MEAN, but uses barycentric ellipses for
 *                 planets beyond Neptune and asteroids beyond Jupiter.
 *               - SE_NODBIT_FOCAL can be combined with any of the other
 *                 bits. The second focal points of the ellipses will 
 *                 be returned instead of the aphelia.
 */
/* mean elements for Mercury - Neptune from VSOP87 (mean equinox of date) */
static double el_node[8][4] = 
  {{ 48.330893,  1.1861890,  0.00017587,  0.000000211,}, /* Mercury */
  { 76.679920,  0.9011190,  0.00040665, -0.000000080,}, /* Venus   */
  {  0       ,  0        ,  0         ,  0          ,}, /* Earth   */
  { 49.558093,  0.7720923,  0.00001605,  0.000002325,}, /* Mars    */
  {100.464441,  1.0209550,  0.00040117,  0.000000569,}, /* Jupiter */
  {113.665524,  0.8770970, -0.00012067, -0.000002380,}, /* Saturn  */
  { 74.005947,  0.5211258,  0.00133982,  0.000018516,}, /* Uranus  */
  {131.784057,  1.1022057,  0.00026006, -0.000000636,}, /* Neptune */
  };
static double el_peri[8][4] = 
  {{ 77.456119,  1.5564775,  0.00029589,  0.000000056,}, /* Mercury */
  {131.563707,  1.4022188, -0.00107337, -0.000005315,}, /* Venus   */
  {102.937348,  1.7195269,  0.00045962,  0.000000499,}, /* Earth   */
  {336.060234,  1.8410331,  0.00013515,  0.000000318,}, /* Mars    */
  { 14.331309,  1.6126668,  0.00103127, -0.000004569,}, /* Jupiter */
  { 93.056787,  1.9637694,  0.00083757,  0.000004899,}, /* Saturn  */
  {173.005159,  1.4863784,  0.00021450,  0.000000433,}, /* Uranus  */
  { 48.123691,  1.4262677,  0.00037918, -0.000000003,}, /* Neptune */
  };
static double el_incl[8][4] = 
  {{  7.004986,  0.0018215, -0.00001809,  0.000000053,}, /* Mercury */
  {  3.394662,  0.0010037, -0.00000088, -0.000000007,}, /* Venus   */
  {  0,         0,          0,           0          ,}, /* Earth   */
  {  1.849726, -0.0006010,  0.00001276, -0.000000006,}, /* Mars    */
  {  1.303270, -0.0054966,  0.00000465, -0.000000004,}, /* Jupiter */
  {  2.488878, -0.0037363, -0.00001516,  0.000000089,}, /* Saturn  */
  {  0.773196,  0.0007744,  0.00003749, -0.000000092,}, /* Uranus  */
  {  1.769952, -0.0093082, -0.00000708,  0.000000028,}, /* Neptune */
  };
static double el_ecce[8][4] = 
  {{  0.20563175,  0.000020406, -0.0000000284, -0.00000000017,}, /* Mercury */
  {  0.00677188, -0.000047766,  0.0000000975,  0.00000000044,}, /* Venus   */
  {  0.01670862, -0.000042037, -0.0000001236,  0.00000000004,}, /* Earth   */
  {  0.09340062,  0.000090483, -0.0000000806, -0.00000000035,}, /* Mars    */
  {  0.04849485,  0.000163244, -0.0000004719, -0.00000000197,}, /* Jupiter */
  {  0.05550862, -0.000346818, -0.0000006456,  0.00000000338,}, /* Saturn  */
  {  0.04629590, -0.000027337,  0.0000000790,  0.00000000025,}, /* Uranus  */
  {  0.00898809,  0.000006408, -0.0000000008, -0.00000000005,}, /* Neptune */
  };
static double el_sema[8][4] = 
  {{  0.387098310,  0.0,  0.0,  0.0,}, /* Mercury */
  {  0.723329820,  0.0,  0.0,  0.0,}, /* Venus   */
  {  1.000001018,  0.0,  0.0,  0.0,}, /* Earth   */
  {  1.523679342,  0.0,  0.0,  0.0,}, /* Mars    */
  {  5.202603191,  0.0000001913,  0.0,  0.0,}, /* Jupiter */
  {  9.554909596,  0.0000021389,  0.0,  0.0,}, /* Saturn  */
  { 19.218446062, -0.0000000372,  0.00000000098,  0.0,}, /* Uranus  */
  { 30.110386869, -0.0000001663,  0.00000000069,  0.0,}, /* Neptune */
  };
/* Ratios of mass of Sun to masses of the planets */
static double plmass[9] = {
    6023600,        /* Mercury */
     408523.5,      /* Venus */
     328900.5,      /* Earth and Moon */
    3098710,        /* Mars */
       1047.350,    /* Jupiter */
       3498.0,      /* Saturn */
      22960,        /* Uranus */
      19314,        /* Neptune */
  130000000,        /* Pluto */
};
static int ipl_to_elem[15] = {2, 0, 0, 1, 3, 4, 5, 6, 7, 0, 0, 0, 0, 0, 2,};
int32 FAR PASCAL_CONV swe_nod_aps(double tjd_et, int32 ipl, int32 iflag, 
                      int32  method,
                      double *xnasc, double *xndsc, 
                      double *xperi, double *xaphe, 
                      char *serr)
{
  int ij, i, j;
  int32 iplx;
  int32 ipli;
  int istart, iend;
  int32 iflJ2000;
  double plm;
  double t = (tjd_et - J2000) / 36525, dt;
  double x[6], xx[24], *xp, xobs[6], x2000[6]; 
  double xpos[3][6], xnorm[6];
  double xposm[6];
  double xn[3][6], xs[3][6];
  double xq[3][6], xa[3][6];
  double xobs2[6], x2[6];
  double *xna = xx, *xnd = xx+6, *xpe = xx+12, *xap = xx+18;
  double incl, sema, ecce, parg, ea, vincl, vsema, vecce, pargx, eax;
  struct plan_data *pedp = &swed.pldat[SEI_EARTH];
  struct plan_data *psdp = &swed.pldat[SEI_SUN];
  struct plan_data *psbdp = &swed.pldat[SEI_SUNBARY];
  struct plan_data pldat;
  double *xsun = psbdp->x;
  double *xear = pedp->x;
  double *ep;
  double Gmsm, dzmin;
  double rxy, rxyz, fac, sgn;
  double sinnode, cosnode, sinincl, cosincl, sinu, cosu, sinE, cosE, cosE2;
  double uu, ny, ny2, c2, v2, pp, ro, ro2, rn, rn2;
  struct epsilon *oe;
  AS_BOOL is_true_nodaps = FALSE, is_mean_nodaps = FALSE;
  AS_BOOL do_aberr = !(iflag & (SEFLG_TRUEPOS | SEFLG_NOABERR));
  AS_BOOL do_defl = !(iflag & SEFLG_TRUEPOS) && !(iflag & SEFLG_NOGDEFL);
  AS_BOOL do_focal_point = method & SE_NODBIT_FOPOINT;
  AS_BOOL ellipse_is_bary = FALSE;
  int32 iflg0;
  /* to get control over the save area: */
  swi_force_app_pos_etc();
  method %= SE_NODBIT_FOPOINT;
  ipli = ipl;
  if (ipl == SE_SUN) 
    ipli = SE_EARTH;
  if (ipl == SE_MOON) {
    do_defl = FALSE;
    if (!(iflag & SEFLG_HELCTR))
      do_aberr = FALSE;
  }
  iflg0 = (iflag & (SEFLG_EPHMASK|SEFLG_NONUT)) | SEFLG_SPEED | SEFLG_TRUEPOS;
  if (ipli != SE_MOON)
    iflg0 |= SEFLG_HELCTR;
  if (ipl == SE_MEAN_NODE || ipl == SE_TRUE_NODE || 
	  ipl == SE_MEAN_APOG || ipl == SE_OSCU_APOG || 
	  ipl < 0 || 
	  (ipl >= SE_NPLANETS && ipl <= SE_AST_OFFSET)) {
	  /*(ipl >= SE_FICT_OFFSET && ipl - SE_FICT_OFFSET < SE_NFICT_ELEM)) */
    if (serr != NULL)
      sprintf(serr, "nodes/apsides for planet %5.0f are not implemented", (double) ipl);
    if (xnasc != NULL)
      for (i = 0; i <= 5; i++)
	xnasc[i] = 0;
    if (xndsc != NULL)
      for (i = 0; i <= 5; i++)
	xndsc[i] = 0;
    if (xaphe != NULL)
      for (i = 0; i <= 5; i++)
	xaphe[i] = 0;
    if (xperi != NULL)
      for (i = 0; i <= 5; i++)
	xperi[i] = 0;
    return ERR;
  }
  for (i = 0; i < 24; i++)
    xx[i] = 0;
  /***************************************
   * mean nodes and apsides 
   ***************************************/
  /* mean points only for Sun - Neptune */
  if ((method == 0 || (method & SE_NODBIT_MEAN)) &&
        ((ipl >= SE_SUN && ipl <= SE_NEPTUNE) || ipl == SE_EARTH)) {
    if (ipl == SE_MOON) {
      swi_mean_lunar_elements(tjd_et, &xna[0], &xna[3], &xpe[0], &xpe[3]);
      incl = MOON_MEAN_INCL;
      vincl = 0;
      ecce = MOON_MEAN_ECC;
      vecce = 0;
      sema = MOON_MEAN_DIST / AUNIT;
      vsema = 0;
    } else {
      iplx = ipl_to_elem[ipl];
      ep = el_incl[iplx];
      incl = ep[0] + ep[1] * t + ep[2] * t * t + ep[3] * t * t * t;
      vincl = ep[1] / 36525;
      ep = el_sema[iplx];
      sema = ep[0] + ep[1] * t + ep[2] * t * t + ep[3] * t * t * t;
      vsema = ep[1] / 36525;
      ep = el_ecce[iplx];
      ecce = ep[0] + ep[1] * t + ep[2] * t * t + ep[3] * t * t * t;
      vecce = ep[1] / 36525;
      ep = el_node[iplx];
      /* ascending node */
      xna[0] = ep[0] + ep[1] * t + ep[2] * t * t + ep[3] * t * t * t;
      xna[3] = ep[1] / 36525;
      /* perihelion */
      ep = el_peri[iplx];
      xpe[0] = ep[0] + ep[1] * t + ep[2] * t * t + ep[3] * t * t * t;
      xpe[3] = ep[1] / 36525;
    }
    /* descending node */
    xnd[0] = swe_degnorm(xna[0] + 180);
    xnd[3] = xna[3];
    /* angular distance of perihelion from node */
    parg = xpe[0] = swe_degnorm(xpe[0] - xna[0]);
    pargx = xpe[3] = swe_degnorm(xpe[0] + xpe[3]  - xna[3]);
    /* transform from orbital plane to mean ecliptic of date */
    swe_cotrans(xpe, xpe, -incl);
    /* xpe+3 is aux. position, not speed!!! */ 
    swe_cotrans(xpe+3, xpe+3, -incl-vincl);	
    /* add node again */
    xpe[0] = swe_degnorm(xpe[0] + xna[0]);
    /* xpe+3 is aux. position, not speed!!! */ 
    xpe[3] = swe_degnorm(xpe[3] + xna[0] + xna[3]);
    /* speed */
    xpe[3] = swe_degnorm(xpe[3] - xpe[0]);
    /* heliocentric distance of perihelion and aphelion */
    xpe[2] = sema * (1 - ecce);
    xpe[5] = (sema + vsema) * (1 - ecce - vecce) - xpe[2];
    /* aphelion */
    xap[0] = swe_degnorm(xpe[0] + 180);
    xap[1] = -xpe[1];
    xap[3] = xpe[3];
    xap[4] = -xpe[4];
    if (do_focal_point) {
      xap[2] = sema * ecce * 2;
      xap[5] = (sema + vsema) * (ecce + vecce) * 2 - xap[2];
    } else {
      xap[2] = sema * (1 + ecce);
      xap[5] = (sema + vsema) * (1 + ecce + vecce) - xap[2];
    }
    /* heliocentric distance of nodes */
    ea = atan(tan(-parg * DEGTORAD / 2) * sqrt((1-ecce)/(1+ecce))) * 2;
    eax = atan(tan(-pargx * DEGTORAD / 2) * sqrt((1-ecce-vecce)/(1+ecce+vecce))) * 2;
    xna[2] = sema * (cos(ea) - ecce) / cos(parg * DEGTORAD);
    xna[5] = (sema+vsema) * (cos(eax) - ecce - vecce) / cos(pargx * DEGTORAD);
    xna[5] -= xna[2];
    ea = atan(tan((180 - parg) * DEGTORAD / 2) * sqrt((1-ecce)/(1+ecce))) * 2;
    eax = atan(tan((180 - pargx) * DEGTORAD / 2) * sqrt((1-ecce-vecce)/(1+ecce+vecce))) * 2;
    xnd[2] = sema * (cos(ea) - ecce) / cos((180 - parg) * DEGTORAD);
    xnd[5] = (sema+vsema) * (cos(eax) - ecce - vecce) / cos((180 - pargx) * DEGTORAD);
    xnd[5] -= xnd[2];
    /* no light-time correction because speed is extremely small */
    for (i = 0, xp = xx; i < 4; i++, xp += 6) {
      /* to cartesian coordinates */
      xp[0] *= DEGTORAD;
      xp[1] *= DEGTORAD;
      xp[3] *= DEGTORAD;
      xp[4] *= DEGTORAD;
      swi_polcart_sp(xp, xp);
    }
    is_mean_nodaps = TRUE;
  /***************************************
   * "true" or osculating nodes and apsides 
   ***************************************/
  } else {
    /* first, we need a heliocentric distance of the planet */
    if (swe_calc(tjd_et, ipli, iflg0, x, serr) == ERR)
      return ERR;
    iflJ2000 = (iflag & SEFLG_EPHMASK)|SEFLG_J2000|SEFLG_EQUATORIAL|SEFLG_XYZ|SEFLG_TRUEPOS|SEFLG_NONUT|SEFLG_SPEED;
    ellipse_is_bary = FALSE;
    if (ipli != SE_MOON) {
      if ((method & SE_NODBIT_OSCU_BAR) && x[2] > 6) {
        iflJ2000 |= SEFLG_BARYCTR; /* only planets beyond Jupiter */
        ellipse_is_bary = TRUE;
      } else {
        iflJ2000 |= SEFLG_HELCTR;
      }
    }
    /* we need three positions and three speeds
     * for three nodes/apsides. from the three node positions,
     * the speed of the node will be computed. */
    if (ipli == SE_MOON) {
      dt = NODE_CALC_INTV;
      dzmin = 1e-15;
      Gmsm = GEOGCONST * (1 + 1 / EARTH_MOON_MRAT) /AUNIT/AUNIT/AUNIT*86400.0*86400.0;
    } else {
      if ((ipli >= SE_MERCURY && ipli <= SE_PLUTO) || ipli == SE_EARTH)
        plm = 1 / plmass[ipl_to_elem[ipl]];
      else
        plm = 0;
      dt = NODE_CALC_INTV * 10 * x[2];
      dzmin = 1e-15 * dt / NODE_CALC_INTV;
      Gmsm = HELGRAVCONST * (1 + plm) /AUNIT/AUNIT/AUNIT*86400.0*86400.0;
    }
    if (iflag & SEFLG_SPEED) {
      istart = 0;
      iend = 2;
    } else {
      istart = iend = 0;
    }
    for (i = istart, t = tjd_et - dt; i <= iend; i++, t += dt) {
      if (istart == iend)
        t = tjd_et;
      if (swe_calc(t, ipli, iflJ2000, xpos[i], serr) == ERR)
        return ERR;
      /* the EMB is used instead of the earth */
      if (ipli == SE_EARTH) {
        if (swe_calc(t, SE_MOON, iflJ2000 & ~(SEFLG_BARYCTR|SEFLG_HELCTR), xposm, serr) == ERR)
          return ERR;
        for (j = 0; j <= 2; j++)
          xpos[i][j] += xposm[j] / (EARTH_MOON_MRAT + 1.0);
      }
      swi_plan_for_osc_elem(iflg0, t, xpos[i]);
    }
    for (i = istart; i <= iend; i++) {
      if (fabs(xpos[i][5]) < dzmin)
        xpos[i][5] = dzmin;
      fac = xpos[i][2] / xpos[i][5];
      sgn = xpos[i][5] / fabs(xpos[i][5]);
      for (j = 0; j <= 2; j++) {
        xn[i][j] = (xpos[i][j] - fac * xpos[i][j+3]) * sgn;
        xs[i][j] = -xn[i][j];
      }
    }
    for (i = istart; i <= iend; i++) {
      /* node */
      rxy =  sqrt(xn[i][0] * xn[i][0] + xn[i][1] * xn[i][1]);
      cosnode = xn[i][0] / rxy;	
      sinnode = xn[i][1] / rxy;
      /* inclination */
      swi_cross_prod(xpos[i], xpos[i]+3, xnorm);
      rxy =  xnorm[0] * xnorm[0] + xnorm[1] * xnorm[1];
      c2 = (rxy + xnorm[2] * xnorm[2]);
      rxyz = sqrt(c2);
      rxy = sqrt(rxy);
      sinincl = rxy / rxyz;
      cosincl = sqrt(1 - sinincl * sinincl);
      /* argument of latitude */
      cosu = xpos[i][0] * cosnode + xpos[i][1] * sinnode;
      sinu = xpos[i][2] / sinincl;	
      uu = atan2(sinu, cosu);	
      /* semi-axis */
      rxyz = sqrt(square_sum(xpos[i]));
      v2 = square_sum((xpos[i]+3));
      sema = 1 / (2 / rxyz - v2 / Gmsm);	
      /* eccentricity */
      pp = c2 / Gmsm;
      ecce = sqrt(1 - pp / sema);	
      /* eccentric anomaly */
      cosE = 1 / ecce * (1 - rxyz / sema);	
      sinE = 1 / ecce / sqrt(sema * Gmsm) * dot_prod(xpos[i], (xpos[i]+3));
      /* true anomaly */
      ny = 2 * atan(sqrt((1+ecce)/(1-ecce)) * sinE / (1 + cosE));
      /* distance of perihelion from ascending node */
      xq[i][0] = swi_mod2PI(uu - ny);
      xq[i][1] = 0;			/* latitude */
      xq[i][2] = sema * (1 - ecce);	/* distance of perihelion */
      /* transformation to ecliptic coordinates */
      swi_polcart(xq[i], xq[i]);
      swi_coortrf2(xq[i], xq[i], -sinincl, cosincl);
      swi_cartpol(xq[i], xq[i]);
      /* adding node, we get perihelion in ecl. coord. */
      xq[i][0] += atan2(sinnode, cosnode);
      xa[i][0] = swi_mod2PI(xq[i][0] + PI);
      xa[i][1] = -xq[i][1];
      if (do_focal_point) {
        xa[i][2] = sema * ecce * 2;	/* distance of aphelion */
      } else {
        xa[i][2] = sema * (1 + ecce);	/* distance of aphelion */
      }
      swi_polcart(xq[i], xq[i]);
      swi_polcart(xa[i], xa[i]);
      /* new distance of node from orbital ellipse:
       * true anomaly of node: */
      ny = swi_mod2PI(ny - uu);
      ny2 = swi_mod2PI(ny - uu + PI);
      /* eccentric anomaly */
      cosE = cos(2 * atan(tan(ny / 2) / sqrt((1+ecce) / (1-ecce))));
      cosE2 = cos(2 * atan(tan(ny2 / 2) / sqrt((1+ecce) / (1-ecce))));
      /* new distance */
      ro = sema * (1 - ecce * cosE);
      ro2 = sema * (1 - ecce * cosE2);
      /* old node distance */
      rn = sqrt(square_sum(xn[i]));
      rn2 = sqrt(square_sum(xs[i]));
      /* correct length of position vector */
      for (j = 0; j <= 2; j++) {
        xn[i][j] *= ro / rn;
        xs[i][j] *= ro2 / rn2;
      }
    }
    for (i = 0; i <= 2; i++) {
      if (iflag & SEFLG_SPEED) {
        xpe[i] = xq[1][i];
        xpe[i+3] = (xq[2][i] - xq[0][i]) / dt / 2;
        xap[i] = xa[1][i];
        xap[i+3] = (xa[2][i] - xa[0][i]) / dt / 2;
        xna[i] = xn[1][i];
        xna[i+3] = (xn[2][i] - xn[0][i]) / dt / 2;
        xnd[i] = xs[1][i];
        xnd[i+3] = (xs[2][i] - xs[0][i]) / dt / 2;
      } else {
        xpe[i] = xq[0][i];
        xpe[i+3] = 0;
        xap[i] = xa[0][i];
        xap[i+3] = 0;
        xna[i] = xn[0][i];
        xna[i+3] = 0;
        xnd[i] = xs[0][i];
        xnd[i+3] = 0;
      }
    }
    is_true_nodaps = TRUE;
  }
  /* to set the variables required in the save area,
   * i.e. ecliptic, nutation, barycentric sun, earth
   * we compute the planet */
  if (ipli == SE_MOON && (iflag & (SEFLG_HELCTR | SEFLG_BARYCTR))) {
    swi_force_app_pos_etc();
    if (swe_calc(tjd_et, SE_SUN, iflg0, x, serr) == ERR)
      return ERR;
  } else {
    if (swe_calc(tjd_et, ipli, iflg0 | (iflag & SEFLG_TOPOCTR), x, serr) == ERR)
      return ERR;
  }
  /***********************
   * position of observer
   ***********************/
  if (iflag & SEFLG_TOPOCTR) {
    /* geocentric position of observer */
    if (swi_get_observer(tjd_et, iflag, FALSE, xobs, serr) != OK)
      return ERR;
    /*for (i = 0; i <= 5; i++)
      xobs[i] = swed.topd.xobs[i];*/
  } else {
    for (i = 0; i <= 5; i++)
      xobs[i] = 0;
  }
  if (iflag & (SEFLG_HELCTR | SEFLG_BARYCTR)) {
    if ((iflag & SEFLG_HELCTR) && !(iflag & SEFLG_MOSEPH))
      for (i = 0; i <= 5; i++)
        xobs[i] = xsun[i];
  } else if (ipl == SE_SUN && !(iflag & SEFLG_MOSEPH)) {
    for (i = 0; i <= 5; i++)
      xobs[i] = xsun[i];
  } else {
    /* barycentric position of observer */
    for (i = 0; i <= 5; i++)
      xobs[i] += xear[i];
  }
  /* ecliptic obliqity */
  if (iflag & SEFLG_J2000)
    oe = &swed.oec2000;
  else
    oe = &swed.oec;
  /*************************************************
   * conversions shared by mean and osculating points
   *************************************************/
  for (ij = 0, xp = xx; ij < 4; ij++, xp += 6) {
    /* no nodes for earth */
    if (ipli == SE_EARTH && ij <= 1) {
      for (i = 0; i <= 5; i++)
    	xp[i] = 0;
      continue;
    }
    /*********************
     * to equator 
     *********************/
    if (is_true_nodaps && !(iflag & SEFLG_NONUT)) {
      swi_coortrf2(xp, xp, -swed.nut.snut, swed.nut.cnut);
      if (iflag & SEFLG_SPEED)
        swi_coortrf2(xp+3, xp+3, -swed.nut.snut, swed.nut.cnut);
    }
    swi_coortrf2(xp, xp, -oe->seps, oe->ceps);
    swi_coortrf2(xp+3, xp+3, -oe->seps, oe->ceps);
    if (is_true_nodaps) {
      /****************************
       * to mean ecliptic of date 
       ****************************/
      if (!(iflag & SEFLG_NONUT))
	swi_nutate(xp, iflag, TRUE);
    }
    /*********************
     * to J2000 
     *********************/
    swi_precess(xp, tjd_et, J_TO_J2000);
    if (iflag & SEFLG_SPEED)
      swi_precess_speed(xp, tjd_et, J_TO_J2000);
    /*********************
     * to barycenter 
     *********************/
    if (ipli == SE_MOON) {
      for (i = 0; i <= 5; i++)
        xp[i] += xear[i];
    } else {
      if (!(iflag & SEFLG_MOSEPH) && !ellipse_is_bary)
        for (j = 0; j <= 5; j++)
          xp[j] += xsun[j];
    }
    /*********************
     * to correct center 
     *********************/
    for (j = 0; j <= 5; j++)
      xp[j] -= xobs[j];
        /* geocentric perigee/apogee of sun */
    if (ipl == SE_SUN && !(iflag & (SEFLG_HELCTR | SEFLG_BARYCTR)))
      for (j = 0; j <= 5; j++)
        xp[j] = -xp[j];
    /*********************
     * light deflection 
     *********************/
    dt = sqrt(square_sum(xp)) * AUNIT / CLIGHT / 86400.0;     
    if (do_defl)
      swi_deflect_light(xp, dt, iflag);
    /*********************
     * aberration 
     *********************/
    if (do_aberr) {
      swi_aberr_light(xp, xobs, iflag);
      /* 
       * Apparent speed is also influenced by
       * the difference of speed of the earth between t and t-dt. 
       * Neglecting this would result in an error of several 0.1"
       */
      if (iflag & SEFLG_SPEED) {
        /* get barycentric sun and earth for t-dt into save area */
        if (swe_calc(tjd_et - dt, ipli, iflg0 | (iflag & SEFLG_TOPOCTR), x2, serr) == ERR)
          return ERR;
        if (iflag & SEFLG_TOPOCTR) {
          /* geocentric position of observer */
          /* if (swi_get_observer(tjd_et - dt, iflag, FALSE, xobs, serr) != OK)
            return ERR;*/
          for (i = 0; i <= 5; i++)
            xobs2[i] = swed.topd.xobs[i];
        } else {
          for (i = 0; i <= 5; i++)
            xobs2[i] = 0;
        }
        if (iflag & (SEFLG_HELCTR | SEFLG_BARYCTR)) {
          if ((iflag & SEFLG_HELCTR) && !(iflag & SEFLG_MOSEPH))
            for (i = 0; i <= 5; i++)
              xobs2[i] = xsun[i];
        } else if (ipl == SE_SUN && !(iflag & SEFLG_MOSEPH)) {
          for (i = 0; i <= 5; i++)
            xobs2[i] = xsun[i];
        } else {
          /* barycentric position of observer */
          for (i = 0; i <= 5; i++)
            xobs2[i] += xear[i];
        }
        for (i = 3; i <= 5; i++) 
          xp[i] += xobs[i] - xobs2[i];
        /* The above call of swe_calc() has destroyed the
         * parts of the save area 
         * (i.e. bary sun, earth nutation matrix!). 
         * to restore it:
         */
        if (swe_calc(tjd_et, SE_SUN, iflg0 | (iflag & SEFLG_TOPOCTR), x2, serr) == ERR)
          return ERR;
      }
    }
    /*********************
     * precession
     *********************/
    /* save J2000 coordinates; required for sidereal positions */
    for (j = 0; j <= 5; j++)
      x2000[j] = xp[j];
    if (!(iflag & SEFLG_J2000)) {
      swi_precess(xp, tjd_et, J2000_TO_J);
      if (iflag & SEFLG_SPEED)
        swi_precess_speed(xp, tjd_et, J2000_TO_J);
    }
    /*********************
     * nutation           
     *********************/
    if (!(iflag & SEFLG_NONUT))
      swi_nutate(xp, iflag, FALSE);
    /* now we have equatorial cartesian coordinates; keep them */
    for (j = 0; j <= 5; j++)
      pldat.xreturn[18+j] = xp[j];
    /************************************************
     * transformation to ecliptic.                  *
     * with sidereal calc. this will be overwritten *
     * afterwards.                                  *
     ************************************************/
    swi_coortrf2(xp, xp, oe->seps, oe->ceps);
    if (iflag & SEFLG_SPEED)
      swi_coortrf2(xp+3, xp+3, oe->seps, oe->ceps);
    if (!(iflag & SEFLG_NONUT)) {
      swi_coortrf2(xp, xp, swed.nut.snut, swed.nut.cnut);
      if (iflag & SEFLG_SPEED)
        swi_coortrf2(xp+3, xp+3, swed.nut.snut, swed.nut.cnut);
    }
    /* now we have ecliptic cartesian coordinates */
    for (j = 0; j <= 5; j++)
      pldat.xreturn[6+j] = xp[j];
    /************************************
     * sidereal positions               *
     ************************************/
    if (iflag & SEFLG_SIDEREAL) {
      /* project onto ecliptic t0 */
      if (swed.sidd.sid_mode & SE_SIDBIT_ECL_T0) {
        if (swi_trop_ra2sid_lon(x2000, pldat.xreturn+6, pldat.xreturn+18, iflag, serr) != OK)
          return ERR;
      /* project onto solar system equator */
      } else if (swed.sidd.sid_mode & SE_SIDBIT_SSY_PLANE) {
        if (swi_trop_ra2sid_lon_sosy(x2000, pldat.xreturn+6, pldat.xreturn+18, iflag, serr) != OK)
          return ERR;
      } else {
      /* traditional algorithm */
        swi_cartpol_sp(pldat.xreturn+6, pldat.xreturn); 
        pldat.xreturn[0] -= swe_get_ayanamsa(tjd_et) * DEGTORAD;
        swi_polcart_sp(pldat.xreturn, pldat.xreturn+6); 
      }
    } 
    if ((iflag & SEFLG_XYZ) && (iflag & SEFLG_EQUATORIAL)) {
      for (j = 0; j <= 5; j++)
        xp[j] = pldat.xreturn[18+j];
      continue;
    }
    if (iflag & SEFLG_XYZ) {
      for (j = 0; j <= 5; j++)
        xp[j] = pldat.xreturn[6+j];
      continue;
    }
    /************************************************
     * transformation to polar coordinates          *
     ************************************************/
    swi_cartpol_sp(pldat.xreturn+18, pldat.xreturn+12); 
    swi_cartpol_sp(pldat.xreturn+6, pldat.xreturn); 
    /********************** 
     * radians to degrees *
     **********************/
    for (j = 0; j < 2; j++) {
      pldat.xreturn[j] *= RADTODEG;		/* ecliptic */
      pldat.xreturn[j+3] *= RADTODEG;
      pldat.xreturn[j+12] *= RADTODEG;	/* equator */
      pldat.xreturn[j+15] *= RADTODEG;
    }
    if (iflag & SEFLG_EQUATORIAL) {
      for (j = 0; j <= 5; j++)
        xp[j] = pldat.xreturn[12+j];
      continue;
    } else {
      for (j = 0; j <= 5; j++)
        xp[j] = pldat.xreturn[j];
      continue;
    }
  }
  for (i = 0; i <= 5; i++) {
    if (i > 2 && !(iflag & SEFLG_SPEED))
      xna[i] = xnd[i] = xpe[i] = xap[i] = 0;
    if (xnasc != NULL)
      xnasc[i] = xna[i];
    if (xndsc != NULL)
      xndsc[i] = xnd[i];
    if (xperi != NULL)
      xperi[i] = xpe[i];
    if (xaphe != NULL)
      xaphe[i] = xap[i];
  }
  return OK;
}

int32 FAR PASCAL_CONV swe_nod_aps_ut(double tjd_ut, int32 ipl, int32 iflag, 
                      int32  method,
                      double *xnasc, double *xndsc, 
                      double *xperi, double *xaphe, 
                      char *serr) {
  return swe_nod_aps(tjd_ut + swe_deltat(tjd_ut),
                      ipl, iflag, method, xnasc, xndsc, xperi, xaphe,
                      serr);
}
