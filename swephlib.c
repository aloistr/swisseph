
/* SWISSEPH
   $Header: swephlib.c,v 1.65 2003/06/14 13:02:20 alois Exp $

   SWISSEPH modules that may be useful for other applications
   e.g. chopt.c, venus.c, swetest.c

  Authors: Dieter Koch and Alois Treindl, Astrodienst Zürich

   coordinate transformations
   obliquity of ecliptic
   nutation
   precession
   delta t
   sidereal time
   setting or getting of tidal acceleration of moon
   chebyshew interpolation
   ephemeris file name generation
   cyclic redundancy checksum CRC
   modulo and normalization functions
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

#include <string.h>
#include <ctype.h>
#include "swephexp.h"
#include "sweph.h"
#include "swephlib.h"
#if MSDOS
# include <process.h>
#endif

#ifdef TRACE
void swi_open_trace(char *serr);
FILE *swi_fp_trace_c = NULL;
FILE *swi_fp_trace_out = NULL;
int32 swi_trace_count = 0;
#endif

static double tid_acc = SE_TIDAL_DEFAULT;
static AS_BOOL init_dt_done = FALSE;
static void init_crc32(void);
static int init_dt(void);
static double adjust_for_tidacc(double ans, double Y);

/* Reduce x modulo 360 degrees
 */
double FAR PASCAL_CONV swe_degnorm(double x)
{
  double y;
  y = fmod(x, 360.0);
  if (fabs(y) < 1e-13) y = 0;	/* Alois fix 11-dec-1999 */
  if( y < 0.0 ) y += 360.0;
  return(y);
}

/* Reduce x modulo TWOPI degrees
 */
double FAR PASCAL_CONV swe_radnorm(double x)
{
  double y;
  y = fmod(x, TWOPI);
  if (fabs(y) < 1e-13) y = 0;	/* Alois fix 11-dec-1999 */
  if( y < 0.0 ) y += TWOPI;
  return(y);
}

double FAR PASCAL_CONV swe_deg_midp(double x1, double x0)
{
  double d, y;
  d = swe_difdeg2n(x1, x0);	/* arc from x0 to x1 */
  y = swe_degnorm(x0 + d / 2);
  return(y);
}

double FAR PASCAL_CONV swe_rad_midp(double x1, double x0)
{
  return DEGTORAD * swe_deg_midp(x1 * RADTODEG, x0 * RADTODEG);
}

/* Reduce x modulo 2*PI
 */
double swi_mod2PI(double x)
{
  double y;
  y = fmod(x, TWOPI);
  if( y < 0.0 ) y += TWOPI;
  return(y);
}


double swi_angnorm(double x)
{
  if (x < 0.0 )
    return x + TWOPI;
  else if (x >= TWOPI)
    return x - TWOPI;
  else
    return x;
}

void swi_cross_prod(double *a, double *b, double *x)
{
  x[0] = a[1]*b[2] - a[2]*b[1];
  x[1] = a[2]*b[0] - a[0]*b[2];
  x[2] = a[0]*b[1] - a[1]*b[0];
}

/*  Evaluates a given chebyshev series coef[0..ncf-1] 
 *  with ncf terms at x in [-1,1]. Communications of the ACM, algorithm 446,
 *  April 1973 (vol. 16 no.4) by Dr. Roger Broucke. 
 */
double swi_echeb(double x, double *coef, int ncf)
{
  int j;
  double x2, br, brp2, brpp;
  x2 = x * 2.;
  br = 0.;
  brp2 = 0.;	/* dummy assign to silence gcc warning */
  brpp = 0.;
  for (j = ncf - 1; j >= 0; j--) {
    brp2 = brpp;
    brpp = br;
    br = x2 * brpp - brp2 + coef[j];
  }
  return (br - brp2) * .5;
}

/*
 * evaluates derivative of chebyshev series, see echeb
 */
double swi_edcheb(double x, double *coef, int ncf)
{
  double bjpl, xjpl;
  int j;
  double x2, bf, bj, dj, xj, bjp2, xjp2;
  x2 = x * 2.;
  bf = 0.;	/* dummy assign to silence gcc warning */
  bj = 0.;	/* dummy assign to silence gcc warning */
  xjp2 = 0.;
  xjpl = 0.;
  bjp2 = 0.;
  bjpl = 0.;
  for (j = ncf - 1; j >= 1; j--) {
    dj = (double) (j + j);
    xj = coef[j] * dj + xjp2;
    bj = x2 * bjpl - bjp2 + xj;
    bf = bjp2;
    bjp2 = bjpl;
    bjpl = bj;
    xjp2 = xjpl;
    xjpl = xj;
  }
  return (bj - bf) * .5;
}

/*
 * conversion between ecliptical and equatorial polar coordinates.
 * for users of SWISSEPH, not used by our routines.
 * for ecl. to equ.  eps must be negative.
 * for equ. to ecl.  eps must be positive.
 * xpo, xpn are arrays of 3 doubles containing position.
 * attention: input must be in degrees!
 */
void FAR PASCAL_CONV swe_cotrans(double *xpo, double *xpn, double eps)
{
  int i;
  double x[6], e = eps * DEGTORAD;
  for(i = 0; i <= 1; i++)
    x[i] = xpo[i];
  x[0] *= DEGTORAD;
  x[1] *= DEGTORAD;
  x[2] = 1;
  for(i = 3; i <= 5; i++)
    x[i] = 0;
  swi_polcart(x, x);
  swi_coortrf(x, x, e);
  swi_cartpol(x, x);
  xpn[0] = x[0] * RADTODEG;
  xpn[1] = x[1] * RADTODEG;
  xpn[2] = xpo[2];
}

/*
 * conversion between ecliptical and equatorial polar coordinates
 * with speed.
 * for users of SWISSEPH, not used by our routines.
 * for ecl. to equ.  eps must be negative.
 * for equ. to ecl.  eps must be positive.
 * xpo, xpn are arrays of 6 doubles containing position and speed.
 * attention: input must be in degrees!
 */
void FAR PASCAL_CONV swe_cotrans_sp(double *xpo, double *xpn, double eps)
{
  int i;
  double x[6], e = eps * DEGTORAD;
  for (i = 0; i <= 5; i++)
    x[i] = xpo[i];
  x[0] *= DEGTORAD;
  x[1] *= DEGTORAD;
  x[2] = 1;	/* avoids problems with polcart(), if x[2] = 0 */
  x[3] *= DEGTORAD;
  x[4] *= DEGTORAD;
  swi_polcart_sp(x, x);
  swi_coortrf(x, x, e);
  swi_coortrf(x+3, x+3, e);
  swi_cartpol_sp(x, xpn);
  xpn[0] *= RADTODEG;
  xpn[1] *= RADTODEG;
  xpn[2] = xpo[2];
  xpn[3] *= RADTODEG;
  xpn[4] *= RADTODEG;
  xpn[5] = xpo[5];
}

/*
 * conversion between ecliptical and equatorial cartesian coordinates
 * for ecl. to equ.  eps must be negative
 * for equ. to ecl.  eps must be positive
 */
void swi_coortrf(double *xpo, double *xpn, double eps) 
{
  double sineps, coseps;
  double x[3];
  sineps = sin(eps);
  coseps = cos(eps);
  x[0] = xpo[0];
  x[1] = xpo[1] * coseps + xpo[2] * sineps;
  x[2] = -xpo[1] * sineps + xpo[2] * coseps;
  xpn[0] = x[0];
  xpn[1] = x[1]; 
  xpn[2] = x[2];
}

/*
 * conversion between ecliptical and equatorial cartesian coordinates
 * sineps            sin(eps)
 * coseps            cos(eps)
 * for ecl. to equ.  sineps must be -sin(eps)
 */
void swi_coortrf2(double *xpo, double *xpn, double sineps, double coseps) 
{
  double x[3];
  x[0] = xpo[0];
  x[1] = xpo[1] * coseps + xpo[2] * sineps;
  x[2] = -xpo[1] * sineps + xpo[2] * coseps;
  xpn[0] = x[0];
  xpn[1] = x[1]; 
  xpn[2] = x[2];
}

/* conversion of cartesian (x[3]) to polar coordinates (l[3]).
 * x = l is allowed.
 * if |x| = 0, then lon, lat and rad := 0.
 */
void swi_cartpol(double *x, double *l) 
{
  double rxy;
  double ll[3];
  if (x[0] == 0 && x[1] == 0 && x[2] == 0) {
    l[0] = l[1] = l[2] = 0;
    return;
  }
  rxy = x[0]*x[0] + x[1]*x[1];
  ll[2] = sqrt(rxy + x[2]*x[2]);
  rxy = sqrt(rxy);
  ll[0] = atan2(x[1], x[0]);
  if (ll[0] < 0.0) ll[0] += TWOPI;
  ll[1] = atan(x[2] / rxy);
  l[0] = ll[0];
  l[1] = ll[1];
  l[2] = ll[2];
}

/* conversion from polar (l[3]) to cartesian coordinates (x[3]).
 * x = l is allowed.
 */
void swi_polcart(double *l, double *x) 
{
  double xx[3];
  double cosl1;
  cosl1 = cos(l[1]);
  xx[0] = l[2] * cosl1 * cos(l[0]);
  xx[1] = l[2] * cosl1 * sin(l[0]);
  xx[2] = l[2] * sin(l[1]);
  x[0] = xx[0];
  x[1] = xx[1];
  x[2] = xx[2];
}

/* conversion of position and speed. 
 * from cartesian (x[6]) to polar coordinates (l[6]). 
 * x = l is allowed.
 * if position is 0, function returns direction of 
 * motion.
 */
void swi_cartpol_sp(double *x, double *l)
{
  double xx[6], ll[6];
  double rxy, coslon, sinlon, coslat, sinlat;
  /* zero position */
  if (x[0] == 0 && x[1] == 0 && x[2] == 0) {
    l[0] = l[1] = l[3] = l[4] = 0;
    l[5] = sqrt(square_sum((x+3)));
    swi_cartpol(x+3, l);
    l[2] = 0;
    return;
  }
  /* zero speed */
  if (x[3] == 0 && x[4] == 0 && x[5] == 0) {
    l[3] = l[4] = l[5] = 0;
    swi_cartpol(x, l);
    return;
  }
  /* position */
  rxy = x[0]*x[0] + x[1]*x[1];
  ll[2] = sqrt(rxy + x[2]*x[2]);
  rxy = sqrt(rxy);
  ll[0] = atan2(x[1], x[0]);
  if (ll[0] < 0.0) ll[0] += TWOPI;
  ll[1] = atan(x[2] / rxy);
  /* speed: 
   * 1. rotate coordinate system by longitude of position about z-axis, 
   *    so that new x-axis = position radius projected onto x-y-plane.
   *    in the new coordinate system 
   *    vy'/r = dlong/dt, where r = sqrt(x^2 +y^2).
   * 2. rotate coordinate system by latitude about new y-axis.
   *    vz"/r = dlat/dt, where r = position radius.
   *    vx" = dr/dt
   */
  coslon = x[0] / rxy; 		/* cos(l[0]); */
  sinlon = x[1] / rxy; 		/* sin(l[0]); */
  coslat = rxy / ll[2];  	/* cos(l[1]); */
  sinlat = x[2] / ll[2];	/* sin(ll[1]); */
  xx[3] = x[3] * coslon + x[4] * sinlon;
  xx[4] = -x[3] * sinlon + x[4] * coslon;
  l[3] = xx[4] / rxy;  		/* speed in longitude */
  xx[4] = -sinlat * xx[3] + coslat * x[5];
  xx[5] =  coslat * xx[3] + sinlat * x[5];
  l[4] = xx[4] / ll[2];  	/* speed in latitude */
  l[5] = xx[5];  		/* speed in radius */
  l[0] = ll[0];			/* return position */
  l[1] = ll[1];
  l[2] = ll[2];
}

/* conversion of position and speed 
 * from polar (l[6]) to cartesian coordinates (x[6]) 
 * x = l is allowed
 * explanation s. swi_cartpol_sp()
 */
void swi_polcart_sp(double *l, double *x)
{
  double sinlon, coslon, sinlat, coslat;
  double xx[6], rxy, rxyz;
  /* zero speed */
  if (l[3] == 0 && l[4] == 0 && l[5] == 0) {
    x[3] = x[4] = x[5] = 0;
    swi_polcart(l, x);
    return;
  }
  /* position */
  coslon = cos(l[0]);
  sinlon = sin(l[0]);
  coslat = cos(l[1]);
  sinlat = sin(l[1]);
  xx[0] = l[2] * coslat * coslon;
  xx[1] = l[2] * coslat * sinlon;
  xx[2] = l[2] * sinlat;
  /* speed; explanation s. swi_cartpol_sp(), same method the other way round*/
  rxyz = l[2];
  rxy = sqrt(xx[0] * xx[0] + xx[1] * xx[1]);
  xx[5] = l[5];
  xx[4] = l[4] * rxyz;
  x[5] = sinlat * xx[5] + coslat * xx[4];	/* speed z */
  xx[3] = coslat * xx[5] - sinlat * xx[4];
  xx[4] = l[3] * rxy;
  x[3] = coslon * xx[3] - sinlon * xx[4];	/* speed x */
  x[4] = sinlon * xx[3] + coslon * xx[4];	/* speed y */
  x[0] = xx[0];					/* return position */
  x[1] = xx[1];
  x[2] = xx[2];
}

double swi_dot_prod_unit(double *x, double *y)
{
  double dop = x[0]*y[0]+x[1]*y[1]+x[2]*y[2];
  double e1 = sqrt(x[0]*x[0]+x[1]*x[1]+x[2]*x[2]);
  double e2 = sqrt(y[0]*y[0]+y[1]*y[1]+y[2]*y[2]);
  dop /= e1;
  dop /= e2;
  if (dop > 1)
    dop = 1;
  if (dop < -1)
    dop = -1;
  return dop;
}

/* Obliquity of the ecliptic at Julian date J
 *
 * IAU Coefficients are from:
 * J. H. Lieske, T. Lederle, W. Fricke, and B. Morando,
 * "Expressions for the Precession Quantities Based upon the IAU
 * (1976) System of Astronomical Constants,"  Astronomy and Astrophysics
 * 58, 1-16 (1977).
 *
 * Before or after 200 years from J2000, the formula used is from:
 * J. Laskar, "Secular terms of classical planetary theories
 * using the results of general theory," Astronomy and Astrophysics
 * 157, 59070 (1986).
 *
 *  See precess and page B18 of the Astronomical Almanac.
 */
double swi_epsiln(double J) 
{
  double T, eps;
  T = (J - 2451545.0)/36525.0;
  /* This expansion is from the AA.
   * Note the official 1976 IAU number is 23d 26' 21.448", but
   * the JPL numerical integration found 21.4119".
   */
  if( fabs(T) < 2.0 )
	  eps = (((1.813e-3*T-5.9e-4)*T-46.8150)*T+84381.448)*DEGTORAD/3600;
  else {
    /* This expansion is from Laskar, cited above.
     * Bretagnon and Simon say, in Planetary Programs and Tables, that it
     * is accurate to 0.1" over a span of 6000 years. Laskar estimates the
     * precision to be 0.01" after 1000 years and a few seconds of arc
     * after 10000 years.
     */
    T /= 10.0;
    eps = ((((((((( 2.45e-10*T + 5.79e-9)*T + 2.787e-7)*T
    + 7.12e-7)*T - 3.905e-5)*T - 2.4967e-3)*T
    - 5.138e-3)*T + 1.99925)*T - 0.0155)*T - 468.093)*T
    + 84381.448;
    eps *= DEGTORAD/3600;
  }
  return(eps);
}

/* Precession of the equinox and ecliptic
 * from epoch Julian date J to or from J2000.0
 *
 * Program by Steve Moshier.
 * Changes in program structure by Dieter Koch.
 *
 * #define PREC_WILLIAMS_1994 1
 * James G. Williams, "Contributions to the Earth's obliquity rate,
 * precession, and nutation,"  Astron. J. 108, 711-724 (1994).
 *
 * #define PREC_SIMON_1994 0
 * J. L. Simon, P. Bretagnon, J. Chapront, M. Chapront-Touze', G. Francou,
 * and J. Laskar, "Numerical Expressions for precession formulae and
 * mean elements for the Moon and the planets," Astronomy and Astrophysics
 * 282, 663-683 (1994).  
 *
 * #define PREC_IAU_1976 0
 * IAU Coefficients are from:
 * J. H. Lieske, T. Lederle, W. Fricke, and B. Morando,
 * "Expressions for the Precession Quantities Based upon the IAU
 * (1976) System of Astronomical Constants,"  Astronomy and
 * Astrophysics 58, 1-16 (1977).
 *
 * #define PREC_LASKAR_1986 0
 * Newer formulas that cover a much longer time span are from:
 * J. Laskar, "Secular terms of classical planetary theories
 * using the results of general theory," Astronomy and Astrophysics
 * 157, 59070 (1986).
 *
 * See also:
 * P. Bretagnon and G. Francou, "Planetary theories in rectangular
 * and spherical variables. VSOP87 solutions," Astronomy and
 * Astrophysics 202, 309-315 (1988).
 *
 * Laskar's expansions are said by Bretagnon and Francou
 * to have "a precision of about 1" over 10000 years before
 * and after J2000.0 in so far as the precession constants p^0_A
 * and epsilon^0_A are perfectly known."
 *
 * Bretagnon and Francou's expansions for the node and inclination
 * of the ecliptic were derived from Laskar's data but were truncated
 * after the term in T**6. I have recomputed these expansions from
 * Laskar's data, retaining powers up to T**10 in the result.
 *
 * The following table indicates the differences between the result
 * of the IAU formula and Laskar's formula using four different test
 * vectors, checking at J2000 plus and minus the indicated number
 * of years.
 *
 *   Years       Arc
 * from J2000  Seconds
 * ----------  -------
 *        0	  0
 *      100	.006	
 *      200     .006
 *      500     .015
 *     1000     .28
 *     2000    6.4
 *     3000   38.
 *    10000 9400.
 */
/* In WILLIAMS and SIMON, Laskar's terms of order higher than t^4
   have been retained, because Simon et al mention that the solution
   is the same except for the lower order terms.  */

#if PREC_WILLIAMS_1994
static double pAcof[] = {
 -8.66e-10, -4.759e-8, 2.424e-7, 1.3095e-5, 1.7451e-4, -1.8055e-3,
 -0.235316, 0.076, 110.5407, 50287.70000 };
static double nodecof[] = {
  6.6402e-16, -2.69151e-15, -1.547021e-12, 7.521313e-12, 1.9e-10, 
  -3.54e-9, -1.8103e-7,  1.26e-7,  7.436169e-5,
  -0.04207794833,  3.052115282424};
static double inclcof[] = {
  1.2147e-16, 7.3759e-17, -8.26287e-14, 2.503410e-13, 2.4650839e-11, 
  -5.4000441e-11, 1.32115526e-9, -6.012e-7, -1.62442e-5,
  0.00227850649, 0.0 };
#endif

#if PREC_SIMON_1994
/* Precession coefficients from Simon et al: */
static double pAcof[] = {
  -8.66e-10, -4.759e-8, 2.424e-7, 1.3095e-5, 1.7451e-4, -1.8055e-3,
  -0.235316, 0.07732, 111.2022, 50288.200 };
static double nodecof[] = {
  6.6402e-16, -2.69151e-15, -1.547021e-12, 7.521313e-12, 1.9e-10, 
  -3.54e-9, -1.8103e-7, 2.579e-8, 7.4379679e-5,
  -0.0420782900, 3.0521126906};
static double inclcof[] = {
  1.2147e-16, 7.3759e-17, -8.26287e-14, 2.503410e-13, 2.4650839e-11, 
  -5.4000441e-11, 1.32115526e-9, -5.99908e-7, -1.624383e-5,
  0.002278492868, 0.0 };
#endif

#if PREC_LASKAR_1986
/* Precession coefficients taken from Laskar's paper: */
static double pAcof[] = {
  -8.66e-10, -4.759e-8, 2.424e-7, 1.3095e-5, 1.7451e-4, -1.8055e-3,
  -0.235316, 0.07732, 111.1971, 50290.966 };
/* Node and inclination of the earth's orbit computed from
 * Laskar's data as done in Bretagnon and Francou's paper.
 * Units are radians.
 */
static double nodecof[] = {
  6.6402e-16, -2.69151e-15, -1.547021e-12, 7.521313e-12, 6.3190131e-10, 
  -3.48388152e-9, -1.813065896e-7, 2.75036225e-8, 7.4394531426e-5,
  -0.042078604317, 3.052112654975 };
static double inclcof[] = {
  1.2147e-16, 7.3759e-17, -8.26287e-14, 2.503410e-13, 2.4650839e-11, 
  -5.4000441e-11, 1.32115526e-9, -5.998737027e-7, -1.6242797091e-5,
  0.002278495537, 0.0 };
#endif

/* Subroutine arguments:
 *
 * R = rectangular equatorial coordinate vector to be precessed.
 *     The result is written back into the input vector.
 * J = Julian date
 * direction =
 *      Precess from J to J2000: direction = 1
 *      Precess from J2000 to J: direction = -1
 * Note that if you want to precess from J1 to J2, you would
 * first go from J1 to J2000, then call the program again
 * to go from J2000 to J2.
 */
int swi_precess(double *R, double J, int direction )
{
  double sinth, costh, sinZ, cosZ, sinz, cosz;
  double eps, sineps, coseps;
  double A, B, T, Z, z, TH, pA, W;
  double x[3];
  double *p;
  int i;
  if( J == J2000 ) 
    return(0);
  /* Each precession angle is specified by a polynomial in
   * T = Julian centuries from J2000.0.  See AA page B18.
   */
  T = (J - J2000)/36525.0;
#if PREC_IAU_1976
  /* Use IAU formula only for a few centuries, if at all.  */
  if( fabs(T) > PREC_IAU_CTIES ) 
    goto laskar;
  Z =  (( 0.017998*T + 0.30188)*T + 2306.2181)*T*DEGTORAD/3600;
  z =  (( 0.018203*T + 1.09468)*T + 2306.2181)*T*DEGTORAD/3600;
  TH = ((-0.041833*T - 0.42665)*T + 2004.3109)*T*DEGTORAD/3600;
  sinth = sin(TH);
  costh = cos(TH);
  sinZ = sin(Z);
  cosZ = cos(Z);
  sinz = sin(z);
  cosz = cos(z);
  A = cosZ*costh;
  B = sinZ*costh;
  if( direction < 0 ) { /* From J2000.0 to J */
    x[0] =    (A*cosz - sinZ*sinz)*R[0]
	    - (B*cosz + cosZ*sinz)*R[1]
		      - sinth*cosz*R[2];
    x[1] =    (A*sinz + sinZ*cosz)*R[0]
	    - (B*sinz - cosZ*cosz)*R[1]
		      - sinth*sinz*R[2];
    x[2] =              cosZ*sinth*R[0]
		      - sinZ*sinth*R[1]
		      + costh*R[2];
  }
  else { /* From J to J2000.0 */
    x[0] =    (A*cosz - sinZ*sinz)*R[0]
	    + (A*sinz + sinZ*cosz)*R[1]
		      + cosZ*sinth*R[2];
    x[1] =  - (B*cosz + cosZ*sinz)*R[0]
	    - (B*sinz - cosZ*cosz)*R[1]
		      - sinZ*sinth*R[2];
    x[2] =            - sinth*cosz*R[0]
		      - sinth*sinz*R[1]
                      + costh*R[2];
  }	
  goto done;
  laskar:
#endif /* IAU */
  /* Implementation by elementary rotations using Laskar's expansions.
   * First rotate about the x axis from the initial equator
   * to the ecliptic. (The input is equatorial.)
   */
  if( direction == 1 ) 
    eps = swi_epsiln(J); /* To J2000 */
  else 
    eps = swi_epsiln(J2000); /* From J2000 */
  sineps = sin(eps);
  coseps = cos(eps);
  x[0] = R[0];
  z = coseps*R[1] + sineps*R[2];
  x[2] = -sineps*R[1] + coseps*R[2];
  x[1] = z;
  /* Precession in longitude */
  T /= 10.0; /* thousands of years */
  p = pAcof;
  pA = *p++;
  for( i=0; i<9; i++ ) 
    pA = pA * T + *p++;
  pA *= DEGTORAD/3600 * T;
  /* Node of the moving ecliptic on the J2000 ecliptic.
   */
  p = nodecof;
  W = *p++;
  for( i=0; i<10; i++ ) 
    W = W * T + *p++;
  /* Rotate about z axis to the node.
   */
  if( direction == 1 ) 
    z = W + pA;
  else 
    z = W;
  B = cos(z);
  A = sin(z);
  z = B * x[0] + A * x[1];
  x[1] = -A * x[0] + B * x[1];
  x[0] = z;
  /* Rotate about new x axis by the inclination of the moving
   * ecliptic on the J2000 ecliptic.
   */
  p = inclcof;
  z = *p++;
  for( i=0; i<10; i++ ) 
    z = z * T + *p++;
  if( direction == 1 ) 
    z = -z;
  B = cos(z);
  A = sin(z);
  z = B * x[1] + A * x[2];
  x[2] = -A * x[1] + B * x[2];
  x[1] = z;
  /* Rotate about new z axis back from the node.
   */
  if( direction == 1 ) 
    z = -W;
  else 
    z = -W - pA;
  B = cos(z);
  A = sin(z);
  z = B * x[0] + A * x[1];
  x[1] = -A * x[0] + B * x[1];
  x[0] = z;
  /* Rotate about x axis to final equator.
   */
  if( direction == 1 ) 
    eps = swi_epsiln(J2000);
  else 
    eps = swi_epsiln(J);
  sineps = sin(eps);
  coseps = cos(eps);
  z = coseps * x[1] - sineps * x[2];
  x[2] = sineps * x[1] + coseps * x[2];
  x[1] = z;
#if PREC_IAU_1976
  done:
#endif
  for( i=0; i<3; i++ ) 
    R[i] = x[i];
  return(0);
}

/* Nutation in longitude and obliquity
 * computed at Julian date J.
 *
 * References:
 * "Summary of 1980 IAU Theory of Nutation (Final Report of the
 * IAU Working Group on Nutation)", P. K. Seidelmann et al., in
 * Transactions of the IAU Vol. XVIII A, Reports on Astronomy,
 * P. A. Wayman, ed.; D. Reidel Pub. Co., 1982.
 *
 * "Nutation and the Earth's Rotation",
 * I.A.U. Symposium No. 78, May, 1977, page 256.
 * I.A.U., 1980.
 *
 * Woolard, E.W., "A redevelopment of the theory of nutation",
 * The Astronomical Journal, 58, 1-3 (1953).
 *
 * This program implements all of the 1980 IAU nutation series.
 * Results checked at 100 points against the 1986 AA; all agreed.
 *
 *
 * - S. L. Moshier, November 1987
 *   October, 1992 - typo fixed in nutation matrix
 *
 * - D. Koch, November 1995: small changes in structure,
 *   Corrections to IAU 1980 Series added from Expl. Suppl. p. 116
 *
 * Each term in the expansion has a trigonometric
 * argument given by
 *   W = i*MM + j*MS + k*FF + l*DD + m*OM
 * where the variables are defined below.
 * The nutation in longitude is a sum of terms of the
 * form (a + bT) * sin(W). The terms for nutation in obliquity
 * are of the form (c + dT) * cos(W).  The coefficients
 * are arranged in the tabulation as follows:
 * 
 * Coefficient:
 * i  j  k  l  m      a      b      c     d
 * 0, 0, 0, 0, 1, -171996, -1742, 92025, 89,
 * The first line of the table, above, is done separately
 * since two of the values do not fit into 16 bit integers.
 * The values a and c are arc seconds times 10000.  b and d
 * are arc seconds per Julian century times 100000.  i through m
 * are integers.  See the program for interpretation of MM, MS,
 * etc., which are mean orbital elements of the Sun and Moon.
 *
 * If terms with coefficient less than X are omitted, the peak
 * errors will be:
 *
 *   omit	error,		  omit	error,
 *   a <	longitude	  c <	obliquity
 * .0005"	.0100"		.0008"	.0094"
 * .0046	.0492		.0095	.0481
 * .0123	.0880		.0224	.0905
 * .0386	.1808		.0895	.1129
 */
static short FAR nt[] = {  
/* LS and OC are units of 0.0001"
 *LS2 and OC2 are units of 0.00001"
 *MM,MS,FF,DD,OM, LS, LS2,OC, OC2 */
 0, 0, 0, 0, 2, 2062, 2,-895, 5,
-2, 0, 2, 0, 1, 46, 0,-24, 0,
 2, 0,-2, 0, 0, 11, 0, 0, 0,
-2, 0, 2, 0, 2,-3, 0, 1, 0,
 1,-1, 0,-1, 0,-3, 0, 0, 0,
 0,-2, 2,-2, 1,-2, 0, 1, 0,
 2, 0,-2, 0, 1, 1, 0, 0, 0,
 0, 0, 2,-2, 2,-13187,-16, 5736,-31,
 0, 1, 0, 0, 0, 1426,-34, 54,-1,
 0, 1, 2,-2, 2,-517, 12, 224,-6,
 0,-1, 2,-2, 2, 217,-5,-95, 3,
 0, 0, 2,-2, 1, 129, 1,-70, 0,
 2, 0, 0,-2, 0, 48, 0, 1, 0,
 0, 0, 2,-2, 0,-22, 0, 0, 0,
 0, 2, 0, 0, 0, 17,-1, 0, 0,
 0, 1, 0, 0, 1,-15, 0, 9, 0,
 0, 2, 2,-2, 2,-16, 1, 7, 0,
 0,-1, 0, 0, 1,-12, 0, 6, 0,
-2, 0, 0, 2, 1,-6, 0, 3, 0,
 0,-1, 2,-2, 1,-5, 0, 3, 0,
 2, 0, 0,-2, 1, 4, 0,-2, 0,
 0, 1, 2,-2, 1, 4, 0,-2, 0,
 1, 0, 0,-1, 0,-4, 0, 0, 0,
 2, 1, 0,-2, 0, 1, 0, 0, 0,
 0, 0,-2, 2, 1, 1, 0, 0, 0,
 0, 1,-2, 2, 0,-1, 0, 0, 0,
 0, 1, 0, 0, 2, 1, 0, 0, 0,
-1, 0, 0, 1, 1, 1, 0, 0, 0,
 0, 1, 2,-2, 0,-1, 0, 0, 0,
 0, 0, 2, 0, 2,-2274,-2, 977,-5,
 1, 0, 0, 0, 0, 712, 1,-7, 0,
 0, 0, 2, 0, 1,-386,-4, 200, 0,
 1, 0, 2, 0, 2,-301, 0, 129,-1,
 1, 0, 0,-2, 0,-158, 0,-1, 0,
-1, 0, 2, 0, 2, 123, 0,-53, 0,
 0, 0, 0, 2, 0, 63, 0,-2, 0,
 1, 0, 0, 0, 1, 63, 1,-33, 0,
-1, 0, 0, 0, 1,-58,-1, 32, 0,
-1, 0, 2, 2, 2,-59, 0, 26, 0,
 1, 0, 2, 0, 1,-51, 0, 27, 0,
 0, 0, 2, 2, 2,-38, 0, 16, 0,
 2, 0, 0, 0, 0, 29, 0,-1, 0,
 1, 0, 2,-2, 2, 29, 0,-12, 0,
 2, 0, 2, 0, 2,-31, 0, 13, 0,
 0, 0, 2, 0, 0, 26, 0,-1, 0,
-1, 0, 2, 0, 1, 21, 0,-10, 0,
-1, 0, 0, 2, 1, 16, 0,-8, 0,
 1, 0, 0,-2, 1,-13, 0, 7, 0,
-1, 0, 2, 2, 1,-10, 0, 5, 0,
 1, 1, 0,-2, 0,-7, 0, 0, 0,
 0, 1, 2, 0, 2, 7, 0,-3, 0,
 0,-1, 2, 0, 2,-7, 0, 3, 0,
 1, 0, 2, 2, 2,-8, 0, 3, 0,
 1, 0, 0, 2, 0, 6, 0, 0, 0,
 2, 0, 2,-2, 2, 6, 0,-3, 0,
 0, 0, 0, 2, 1,-6, 0, 3, 0,
 0, 0, 2, 2, 1,-7, 0, 3, 0,
 1, 0, 2,-2, 1, 6, 0,-3, 0,
 0, 0, 0,-2, 1,-5, 0, 3, 0,
 1,-1, 0, 0, 0, 5, 0, 0, 0,
 2, 0, 2, 0, 1,-5, 0, 3, 0, 
 0, 1, 0,-2, 0,-4, 0, 0, 0,
 1, 0,-2, 0, 0, 4, 0, 0, 0,
 0, 0, 0, 1, 0,-4, 0, 0, 0,
 1, 1, 0, 0, 0,-3, 0, 0, 0,
 1, 0, 2, 0, 0, 3, 0, 0, 0,
 1,-1, 2, 0, 2,-3, 0, 1, 0,
-1,-1, 2, 2, 2,-3, 0, 1, 0,
-2, 0, 0, 0, 1,-2, 0, 1, 0,
 3, 0, 2, 0, 2,-3, 0, 1, 0,
 0,-1, 2, 2, 2,-3, 0, 1, 0,
 1, 1, 2, 0, 2, 2, 0,-1, 0,
-1, 0, 2,-2, 1,-2, 0, 1, 0,
 2, 0, 0, 0, 1, 2, 0,-1, 0,
 1, 0, 0, 0, 2,-2, 0, 1, 0,
 3, 0, 0, 0, 0, 2, 0, 0, 0,
 0, 0, 2, 1, 2, 2, 0,-1, 0,
-1, 0, 0, 0, 2, 1, 0,-1, 0,
 1, 0, 0,-4, 0,-1, 0, 0, 0,
-2, 0, 2, 2, 2, 1, 0,-1, 0,
-1, 0, 2, 4, 2,-2, 0, 1, 0,
 2, 0, 0,-4, 0,-1, 0, 0, 0,
 1, 1, 2,-2, 2, 1, 0,-1, 0,
 1, 0, 2, 2, 1,-1, 0, 1, 0,
-2, 0, 2, 4, 2,-1, 0, 1, 0,
-1, 0, 4, 0, 2, 1, 0, 0, 0,
 1,-1, 0,-2, 0, 1, 0, 0, 0,
 2, 0, 2,-2, 1, 1, 0,-1, 0,
 2, 0, 2, 2, 2,-1, 0, 0, 0,
 1, 0, 0, 2, 1,-1, 0, 0, 0,
 0, 0, 4,-2, 2, 1, 0, 0, 0,
 3, 0, 2,-2, 2, 1, 0, 0, 0,
 1, 0, 2,-2, 0,-1, 0, 0, 0,
 0, 1, 2, 0, 1, 1, 0, 0, 0,
-1,-1, 0, 2, 1, 1, 0, 0, 0,
 0, 0,-2, 0, 1,-1, 0, 0, 0,
 0, 0, 2,-1, 2,-1, 0, 0, 0,
 0, 1, 0, 2, 0,-1, 0, 0, 0,
 1, 0,-2,-2, 0,-1, 0, 0, 0,
 0,-1, 2, 0, 1,-1, 0, 0, 0,
 1, 1, 0,-2, 1,-1, 0, 0, 0,
 1, 0,-2, 2, 0,-1, 0, 0, 0,
 2, 0, 0, 2, 0, 1, 0, 0, 0,
 0, 0, 2, 4, 2,-1, 0, 0, 0,
 0, 1, 0, 1, 0, 1, 0, 0, 0,
#if NUT_CORR_1987 
/* corrections to IAU 1980 nutation series by Herring 1987
 *             in 0.00001" !!!
 *              LS      OC      */
 101, 0, 0, 0, 1,-725, 0, 213, 0,
 101, 1, 0, 0, 0, 523, 0, 208, 0,
 101, 0, 2,-2, 2, 102, 0, -41, 0,
 101, 0, 2, 0, 2, -81, 0,  32, 0,
/*              LC      OS !!!  */
 102, 0, 0, 0, 1, 417, 0, 224, 0,
 102, 1, 0, 0, 0,  61, 0, -24, 0,
 102, 0, 2,-2, 2,-118, 0, -47, 0,
#endif
 ENDMARK,
};

int swi_nutation(double J, double *nutlo)
{
  /* arrays to hold sines and cosines of multiple angles */
  double ss[5][8];
  double cc[5][8];
  double arg;
  double args[5];
  double f, g, T, T2;
  double MM, MS, FF, DD, OM;
  double cu, su, cv, sv, sw, s;
  double C, D;
  int i, j, k, k1, m, n;
  int ns[5];
  short *p;
  /* Julian centuries from 2000 January 1.5,
   * barycentric dynamical time
   */
  T = (J - 2451545.0) / 36525.0;
  T2 = T * T;
  /* Fundamental arguments in the FK5 reference system.
   * The coefficients, originally given to 0.001",
   * are converted here to degrees.
   */
  /* longitude of the mean ascending node of the lunar orbit
   * on the ecliptic, measured from the mean equinox of date
   */
  OM = -6962890.539 * T + 450160.280 + (0.008 * T + 7.455) * T2;
  OM = swe_degnorm(OM/3600) * DEGTORAD;
  /* mean longitude of the Sun minus the
   * mean longitude of the Sun's perigee
   */
  MS = 129596581.224 * T + 1287099.804 - (0.012 * T + 0.577) * T2;
  MS = swe_degnorm(MS/3600) * DEGTORAD;
  /* mean longitude of the Moon minus the
   * mean longitude of the Moon's perigee
   */
  MM = 1717915922.633 * T + 485866.733 + (0.064 * T + 31.310) * T2;
  MM = swe_degnorm(MM/3600) * DEGTORAD;
  /* mean longitude of the Moon minus the
   * mean longitude of the Moon's node
   */
  FF = 1739527263.137 * T + 335778.877 + (0.011 * T - 13.257) * T2;
  FF = swe_degnorm(FF/3600) * DEGTORAD;
  /* mean elongation of the Moon from the Sun.
   */
  DD = 1602961601.328 * T + 1072261.307 + (0.019 * T - 6.891) * T2;
  DD = swe_degnorm(DD/3600) * DEGTORAD;
  args[0] = MM;
  ns[0] = 3;
  args[1] = MS;
  ns[1] = 2;
  args[2] = FF;
  ns[2] = 4;
  args[3] = DD;
  ns[3] = 4;
  args[4] = OM;
  ns[4] = 2;
  /* Calculate sin( i*MM ), etc. for needed multiple angles
   */
  for (k = 0; k <= 4; k++) {
    arg = args[k];
    n = ns[k];
    su = sin(arg);
    cu = cos(arg);
    ss[k][0] = su;			/* sin(L) */
    cc[k][0] = cu;			/* cos(L) */
    sv = 2.0*su*cu;
    cv = cu*cu - su*su;
    ss[k][1] = sv;			/* sin(2L) */
    cc[k][1] = cv;
    for( i=2; i<n; i++ ) {
      s =  su*cv + cu*sv;
      cv = cu*cv - su*sv;
      sv = s;
      ss[k][i] = sv;		/* sin( i+1 L ) */
      cc[k][i] = cv;
    }
  }
  /* first terms, not in table: */
  C = (-0.01742*T - 17.1996)*ss[4][0];	/* sin(OM) */
  D = ( 0.00089*T +  9.2025)*cc[4][0];	/* cos(OM) */
  for(p = &nt[0]; *p != ENDMARK; p += 9) {
    /* argument of sine and cosine */
    k1 = 0;
    cv = 0.0;
    sv = 0.0;
    for( m=0; m<5; m++ ) {
      j = p[m];
      if (j > 100) 
	j = 0; /* p[0] is a flag */
      if( j ) {
	k = j;
	if( j < 0 ) 
	  k = -k;
	su = ss[m][k-1]; /* sin(k*angle) */
	if( j < 0 ) 
	  su = -su;
	cu = cc[m][k-1];
	if( k1 == 0 ) { /* set first angle */
	  sv = su;
	  cv = cu;
	  k1 = 1;
	}
	else {		/* combine angles */
	  sw = su*cv + cu*sv;
	  cv = cu*cv - su*sv;
	  sv = sw;
	}
      }
    }
    /* longitude coefficient, in 0.0001" */
    f  = p[5] * 0.0001;
    if( p[6] != 0 ) 
      f += 0.00001 * T * p[6];
    /* obliquity coefficient, in 0.0001" */
    g = p[7] * 0.0001;
    if( p[8] != 0 ) 
      g += 0.00001 * T * p[8];
    if (*p >= 100) { 	/* coefficients in 0.00001" */
      f *= 0.1;
      g *= 0.1;
    }
    /* accumulate the terms */
    if (*p != 102) {
      C += f * sv;
      D += g * cv;
    }
    else { 		/* cos for nutl and sin for nuto */
      C += f * cv;
      D += g * sv;
    }
    /*
    if (i >= 105) {
      printf("%4.10f, %4.10f\n",f*sv,g*cv);
    }
    */
  }
  /*
      printf("%4.10f, %4.10f, %4.10f, %4.10f\n",MS*RADTODEG,FF*RADTODEG,DD*RADTODEG,OM*RADTODEG);
  printf( "nutation: in longitude %.9f\", in obliquity %.9f\"\n", C, D );
  */
  /* Save answers, expressed in radians */
  nutlo[0] = DEGTORAD * C / 3600.0;
  nutlo[1] = DEGTORAD * D / 3600.0;
  return(0);
}

/* DeltaT = Ephemeris Time - Universal Time, in days.
 * 
 * 1620 - today + a couple of years:
 * ---------------------------------
 * The tabulated values of deltaT, in hundredths of a second,
 * were taken from The Astronomical Almanac 1997, page K8.  The program
 * adjusts for a value of secular tidal acceleration ndot = -25.7376.
 * arcsec per century squared, the value used in JPL's DE403 ephemeris.
 * ELP2000 (and DE200) used the value -23.8946.
 * To change ndot, one can
 * either redefine SE_TIDAL_DEFAULT in swephexp.h
 * or use the routine swe_set_tid_acc() before calling Swiss 
 * Ephemeris.
 * Bessel's interpolation formula is implemented to obtain fourth 
 * order interpolated values at intermediate times.
 *
 * -500 - 1620:
 * ---------------------------------
 * For dates between -500 and 1600, the table given by 
 * Stephenson (1997; p. 515) is used, with linear interpolation.
 * This table is based on an assumed value of ndot = -26.
 * The program adjusts for ndot = -25.7376.
 * For 1600 - 1620, a linear interpolation between the last value
 * of the latter and the first value of the former table is made.
 *
 * before -500:
 * ---------------------------------
 * For times before -600, a formula of Stephenson & Morrison (1995) 
 * (S. Stephenson 1997; p. 508) is used: 
 * dt = 35 * t * t - 20 sec, where t is centuries from 1735 AD.
 * For -600 to -500, a transition from this formula to the Stephenson
 * table has been implemented in order to avoid a jump.
 *
 * future:
 * ---------------------------------
 * For the time after the last tabulated value, we use the formula
 * of Stephenson (1997; p. 507), with a modification that avoids a jump
 * at the end of the tabulated period. A linear term is added that
 * makes a slow transition from the table to the formula over a period
 * of 100 years. (Need not be updated, when table will be enlarged.)
 *
 * References:
 *
 * Stephenson, F. R., and L. V. Morrison, "Long-term changes
 * in the rotation of the Earth: 700 B.C. to A.D. 1980,"
 * Philosophical Transactions of the Royal Society of London
 * Series A 313, 47-70 (1984)
 *
 * Borkowski, K. M., "ELP2000-85 and the Dynamical Time
 * - Universal Time relation," Astronomy and Astrophysics
 * 205, L8-L10 (1988)
 * Borkowski's formula is derived from partly doubtful eclipses 
 * going back to 2137 BC and uses lunar position based on tidal 
 * coefficient of -23.9 arcsec/cy^2.
 *
 * Chapront-Touze, Michelle, and Jean Chapront, _Lunar Tables
 * and Programs from 4000 B.C. to A.D. 8000_, Willmann-Bell 1991
 * Their table agrees with the one here, but the entries are
 * rounded to the nearest whole second.
 *
 * Stephenson, F. R., and M. A. Houlden, _Atlas of Historical
 * Eclipse Maps_, Cambridge U. Press (1986)
 *
 * Stephenson, F.R. & Morrison, L.V., "Long-Term Fluctuations in 
 * the Earth's Rotation: 700 BC to AD 1990", Philosophical 
 * Transactions of the Royal Society of London, 
 * Ser. A, 351 (1995), 165-202. 
 *
 * Stephenson, F. Richard, _Historical Eclipses and Earth's 
 * Rotation_, Cambridge U. Press (1997)
 * 
 * Table from AA for 1620 through today
 * Note, Stephenson and Morrison's table starts at the year 1630.
 * The Chapronts' table does not agree with the Almanac prior to 1630.
 * The actual accuracy decreases rapidly prior to 1780.
 *
 * Jean Meeus, Astronomical Algorithms, 2nd edition, 1998.
 * 
 * For a comprehensive collection of publications and formulae, see:
 * http://www.phys.uu.nl/~vgent/astro/deltatime.htm
 * 
 * For future values of delta t, the following data from the 
 * Earth Orientation Department of the US Naval Observatory can be used:
 * (TAI-UTC) from: ftp://maia.usno.navy.mil/ser7/tai-utc.dat
 * (UT1-UTC) from: ftp://maia.usno.navy.mil/ser7/finals.all
 * file description in: ftp://maia.usno.navy.mil/ser7/readme.finals
 * Delta T = TAI-UT1 + 32.184 sec = (TAI-UTC) - (UT1-UTC) + 32.184 sec
 *
 * Last update of table dt[]: Dieter Koch, 31 March 2005.
 * ATTENTION: Whenever updating this table, do not forget to adjust
 * the macros TABEND and TABSIZ !
 */
#define TABSTART 	1620
#define TABEND 		2014
#define TABSIZ 		(TABEND-TABSTART+1) 
/* we make the table greater for additional values read from external file */
#define TABSIZ_SPACE 	(TABSIZ+50)
static short FAR dt[TABSIZ_SPACE] = {
/* 1620.0 thru 1659.0 */
12400, 11900, 11500, 11000, 10600, 10200, 9800, 9500, 9100, 8800,
8500, 8200, 7900, 7700, 7400, 7200, 7000, 6700, 6500, 6300,
6200, 6000, 5800, 5700, 5500, 5400, 5300, 5100, 5000, 4900,
4800, 4700, 4600, 4500, 4400, 4300, 4200, 4100, 4000, 3800,
/* 1660.0 thru 1699.0 */
3700, 3600, 3500, 3400, 3300, 3200, 3100, 3000, 2800, 2700,
2600, 2500, 2400, 2300, 2200, 2100, 2000, 1900, 1800, 1700,
1600, 1500, 1400, 1400, 1300, 1200, 1200, 1100, 1100, 1000,
1000, 1000, 900, 900, 900, 900, 900, 900, 900, 900,
/* 1700.0 thru 1739.0 */
900, 900, 900, 900, 900, 900, 900, 900, 1000, 1000,
1000, 1000, 1000, 1000, 1000, 1000, 1000, 1100, 1100, 1100,
1100, 1100, 1100, 1100, 1100, 1100, 1100, 1100, 1100, 1100,
1100, 1100, 1100, 1100, 1200, 1200, 1200, 1200, 1200, 1200,
/* 1740.0 thru 1779.0 */
1200, 1200, 1200, 1200, 1300, 1300, 1300, 1300, 1300, 1300,
1300, 1400, 1400, 1400, 1400, 1400, 1400, 1400, 1500, 1500,
1500, 1500, 1500, 1500, 1500, 1600, 1600, 1600, 1600, 1600,
1600, 1600, 1600, 1600, 1600, 1700, 1700, 1700, 1700, 1700,
/* 1780.0 thru 1799.0 */
1700, 1700, 1700, 1700, 1700, 1700, 1700, 1700, 1700, 1700,
1700, 1700, 1600, 1600, 1600, 1600, 1500, 1500, 1400, 1400,
/* 1800.0 thru 1819.0 */
1370, 1340, 1310, 1290, 1270, 1260, 1250, 1250, 1250, 1250,
1250, 1250, 1250, 1250, 1250, 1250, 1250, 1240, 1230, 1220,
/* 1820.0 thru 1859.0 */
1200, 1170, 1140, 1110, 1060, 1020, 960, 910, 860, 800,
750, 700, 660, 630, 600, 580, 570, 560, 560, 560,
570, 580, 590, 610, 620, 630, 650, 660, 680, 690,
710, 720, 730, 740, 750, 760, 770, 770, 780, 780,
/* 1860.0 thru 1899.0 */
788, 782, 754, 697, 640, 602, 541, 410, 292, 182,
161, 10, -102, -128, -269, -324, -364, -454, -471, -511,
-540, -542, -520, -546, -546, -579, -563, -564, -580, -566,
-587, -601, -619, -664, -644, -647, -609, -576, -466, -374,
/* 1900.0 thru 1939.0 */
-272, -154, -2, 124, 264, 386, 537, 614, 775, 913,
1046, 1153, 1336, 1465, 1601, 1720, 1824, 1906, 2025, 2095,
2116, 2225, 2241, 2303, 2349, 2362, 2386, 2449, 2434, 2408,
2402, 2400, 2387, 2395, 2386, 2393, 2373, 2392, 2396, 2402,
/* 1940.0 thru 1979.0 */
 2433, 2483, 2530, 2570, 2624, 2677, 2728, 2778, 2825, 2871,
 2915, 2957, 2997, 3036, 3072, 3107, 3135, 3168, 3218, 3268,
 3315, 3359, 3400, 3447, 3503, 3573, 3654, 3743, 3829, 3920,
 4018, 4117, 4223, 4337, 4449, 4548, 4646, 4752, 4853, 4959,
/* 1980.0 thru 1999.0 */
 5054, 5138, 5217, 5296, 5379, 5434, 5487, 5532, 5582, 5630,
 5686, 5757, 5831, 5912, 5998, 6078, 6163, 6230, 6297, 6347,
/* 2000.0 thru 2005.0 */
 6383, 6409, 6430, 6447, 6457, 6469,       
/* Extrapolated values, 2006 - 2014 */
                                     6481, 6500, 6520, 6550,
 6600, 6650, 6700, 6750, 6800,
};
/* Table for -500 through 1600, from Stephenson & Morrison (1995).
 *
 * The first value for -550 has been added from Borkowski
 * in order to make this table fit with the Borkowski formula
 * for times before -550.
 */
#define TAB2_SIZ	43
#define TAB2_START	(-500)
#define TAB2_END	1600
static short FAR dt2[TAB2_SIZ] = {
/* -500  -450  -400  -350  -300  -250  -200  -150  -100   -50*/
  16800,16000,15300,14600,14000,13400,12800,12200,11600,11100,
/*    0    50   100   150   200   250   300   350   400   450*/
  10600,10100, 9600, 9100, 8600, 8200, 7700, 7200, 6700, 6200,
/*  500   550   600   650   700   750   800   850   900   950*/
   5700, 5200, 4700, 4300, 3800, 3400, 3000, 2600, 2200, 1900,
/* 1000  1050  1100  1150  1200  1250  1300  1350  1400  1450*/
   1600, 1350, 1100,  900,  750,  600,  470,  380,  300,  230,
/* 1500  1550  1600 */
    180,  140,  110,
};
/* returns DeltaT (ET - UT) in days
 * double tjd 	= 	julian day in UT
 */
#define DEMO 0
double FAR PASCAL_CONV swe_deltat(double tjd)
{
  double ans = 0, ans2, ans3;
  double p, B, B2, Y, dd;
  int d[6];
  int i, iy, k;
  /* read additional values from swedelta.txt */
  int tabsiz = init_dt();
  int tabend = TABSTART + tabsiz - 1;
  Y = 2000.0 + (tjd - J2000)/365.25;
  /* before -500:
   * formula by Stephenson (1997; p. 508) but adjusted to fit the starting
   * point of table dt2 (Stephenson 1997). */
  if( Y < TAB2_START ) {
    B = (Y - 1735) * 0.01;
    ans = -20 + 35 * B * B;
    ans = adjust_for_tidacc(ans, Y);
    /* transition from formula to table over 100 years */
    if (Y >= TAB2_START - 100) {
      /* starting value of table dt2: */
      ans2 = adjust_for_tidacc(dt2[0], TAB2_START);
      /* value of formula at epoch TAB2_START */
      B = (TAB2_START - 1735) * 0.01;
      ans3 = -20 + 35 * B * B;
      ans3 = adjust_for_tidacc(ans3, Y);
      dd = ans3 - ans2;
      B = (Y - (TAB2_START - 100)) * 0.01;
      /* fit to starting point of table dt2. */
      ans = ans - dd * B;
    }
  }
  /* between -500 and 1600: 
   * linear interpolation between values of table dt2 (Stephenson 1997) */
  if (Y >= TAB2_START && Y < TAB2_END) { 
    p = floor(Y);
    iy = (int) ((p - TAB2_START) / 50.0);
    dd = (Y - (TAB2_START + 50 * iy)) / 50.0;
    ans = dt2[iy] + (dt2[iy+1] - dt2[iy]) * dd;
    /* correction for tidal acceleration used by our ephemeris */
    ans = adjust_for_tidacc(ans, Y);
  }
  /* between 1600 and 1620:
   * linear interpolation between 
   * end of table dt2 and start of table dt */
  if (Y >= TAB2_END && Y < TABSTART) { 
    B = TABSTART - TAB2_END;
    iy = (TAB2_END - TAB2_START) / 50;
    dd = (Y - TAB2_END) / B;
    ans = dt2[iy] + dd * (dt[0] / 100.0 - dt2[iy]);
    ans = adjust_for_tidacc(ans, Y);
  }
  /* 1620 - today + a few years (tabend):
   * Besselian interpolation from tabulated values in table dt.
   * See AA page K11.
   */
  if (Y >= TABSTART && Y <= tabend) {
    /* Index into the table.
     */
    p = floor(Y);
    iy = (int) (p - TABSTART);
    /* Zeroth order estimate is value at start of year
     */
    ans = dt[iy];
    k = iy + 1;
    if( k >= tabsiz )
      goto done; /* No data, can't go on. */
    /* The fraction of tabulation interval
     */
    p = Y - p;
    /* First order interpolated value
     */
    ans += p*(dt[k] - dt[iy]);
    if( (iy-1 < 0) || (iy+2 >= tabsiz) )
      goto done; /* can't do second differences */
    /* Make table of first differences
     */
    k = iy - 2;
    for( i=0; i<5; i++ ) {
      if( (k < 0) || (k+1 >= tabsiz) ) 
        d[i] = 0;
      else
        d[i] = dt[k+1] - dt[k];
      k += 1;
    }
    /* Compute second differences
     */
    for( i=0; i<4; i++ )
      d[i] = d[i+1] - d[i];
    B = 0.25*p*(p-1.0);
    ans += B*(d[1] + d[2]);
#if DEMO
    printf( "B %.4lf, ans %.4lf\n", B, ans );
#endif
    if( iy+2 >= tabsiz )
      goto done;
    /* Compute third differences
     */
    for( i=0; i<3; i++ )
      d[i] = d[i+1] - d[i];
    B = 2.0*B/3.0;
    ans += (p-0.5)*B*d[1];
#if DEMO
    printf( "B %.4lf, ans %.4lf\n", B*(p-0.5), ans );
#endif
    if( (iy-2 < 0) || (iy+3 > tabsiz) )
      goto done;
    /* Compute fourth differences
     */
    for( i=0; i<2; i++ )
      d[i] = d[i+1] - d[i];
    B = 0.125*B*(p+1.0)*(p-2.0);
    ans += B*(d[0] + d[1]);
#if DEMO
    printf( "B %.4lf, ans %.4lf\n", B, ans );
#endif
    done:
    ans *= 0.01;
    ans = adjust_for_tidacc(ans, Y);
  }
  /* today - : 
   * Formula Stephenson (1997; p. 507),
   * with modification to avoid jump at end of AA table,
   * similar to what Meeus 1998 had suggested.
   * Slow transition within 100 years.
   */
  if (Y > tabend) {
    B = 0.01 * (Y - 1820);
    ans = -20 + 31 * B * B;
    /* slow transition from tabulated values to Stephenson formula: */
    if (Y <= tabend+100) {
      B2 = 0.01 * (tabend - 1820);
      ans2 = -20 + 31 * B2 * B2;
      ans3 = dt[tabsiz-1] * 0.01;
      dd = (ans2 - ans3);
      ans += dd * (Y - (tabend + 100)) * 0.01;
    }
  }
#ifdef TRACE
  swi_open_trace(NULL);
  if (swi_trace_count < TRACE_COUNT_MAX) {
    if (swi_fp_trace_c != NULL) {
      fputs("\n/*SWE_DELTAT*/\n", swi_fp_trace_c);
      fprintf(swi_fp_trace_c, "  tjd = %.9f;", tjd);
      fprintf(swi_fp_trace_c, " t = swe_deltat(tjd);\n");
      fputs("  printf(\"swe_deltat: %f\\t%f\\t\\n\", ", swi_fp_trace_c);
      fputs("tjd, t);\n", swi_fp_trace_c);
      fflush(swi_fp_trace_c);
    }
    if (swi_fp_trace_out != NULL) {
      fprintf(swi_fp_trace_out, "swe_deltat: %f\t%f\t\n", tjd, ans);
      fflush(swi_fp_trace_out);
    }
  }
#endif
  return ans / 86400.0;
}

/* Read delta t values from external file.
 * record structure: year(whitespace)delta_t in 0.01 sec.
 */
static int init_dt(void)
{
  FILE *fp;
  int year;
  int tab_index;
  int tabsiz;
  int i;
  char s[AS_MAXCH];
  char *sp;
  if (!init_dt_done) {
    init_dt_done = TRUE;
    /* no error message if file is missing */
    if ((fp = swi_fopen(-1, "sedeltat.txt", swed.ephepath, NULL)) == NULL)
      return TABSIZ; 
    while(fgets(s, AS_MAXCH, fp) != NULL) {
      sp = s;
      while (strchr(" \t", *sp) != NULL && *sp != '\0') 
        sp++;	/* was *sp++  fixed by Alois 2-jul-2003 */
      if (*sp == '#' || *sp == '\n')
        continue;
      year = atoi(s);
      tab_index = year - TABSTART;
      /* table space is limited. no error msg, if exceeded */
      if (tab_index >= TABSIZ_SPACE)
        continue; 
      sp += 4;
      while (strchr(" \t", *sp) != NULL && *sp != '\0')
        sp++;	/* was *sp++  fixed by Alois 2-jul-2003 */
      dt[tab_index] = (short) (atof(sp) * 100 + 0.5);
    }
    fclose(fp);
  }
  /* find table size */
  tabsiz = 2001 - TABSTART + 1;
  for (i = tabsiz - 1; i < TABSIZ_SPACE; i++) {
    if (dt[i] == 0) 
      break;
    else
      tabsiz++;
  }
  tabsiz--;
  return tabsiz;
}

/* Astronomical Almanac table is corrected by adding the expression
 *     -0.000091 (ndot + 26)(year-1955)^2  seconds
 * to entries prior to 1955 (AA page K8), where ndot is the secular
 * tidal term in the mean motion of the Moon.
 *
 * Entries after 1955 are referred to atomic time standards and
 * are not affected by errors in Lunar or planetary theory.
 */
static double adjust_for_tidacc(double ans, double Y)
{
  double B;
  if( Y < 1955.0 ) {
    B = (Y - 1955.0);
    ans += -0.000091 * (tid_acc + 26.0) * B * B;
  }
  return ans;
}

/* returns tidal acceleration used in swe_deltat() */
double FAR PASCAL_CONV swe_get_tid_acc()
{
#if 0
  if (tid_acc == TID_ACC_DE403)
    return 403;
  if (tid_acc == TID_ACC_DE402)
    return 200;
#endif
  return tid_acc;
}

void FAR PASCAL_CONV swe_set_tid_acc(double t_acc)
{
  tid_acc = t_acc;
#if TRACE
  swi_open_trace(NULL);
  if (swi_trace_count < TRACE_COUNT_MAX) {
    if (swi_fp_trace_c != NULL) {
      fputs("\n/*SWE_SET_TID_ACC*/\n", swi_fp_trace_c);
      fprintf(swi_fp_trace_c, "  t = %.9f;\n", t_acc);
      fprintf(swi_fp_trace_c, "  swe_set_tid_acc(t);\n");
      fputs("  printf(\"swe_set_tid_acc: %f\\t\\n\", ", swi_fp_trace_c);
      fputs("t);\n", swi_fp_trace_c);
      fflush(swi_fp_trace_c);
    }
    if (swi_fp_trace_out != NULL) {
      fprintf(swi_fp_trace_out, "swe_set_tid_acc: %f\t\n", t_acc);
      fflush(swi_fp_trace_out);
    }
  }
#endif
}

/* Apparent Sidereal Time at Greenwich with equation of the equinoxes
 * AA page B6
 *
 * returns sidereal time in hours.
 *
 * Caution. At epoch J2000.0, the 16 decimal precision
 * of IEEE double precision numbers
 * limits time resolution measured by Julian date
 * to approximately 24 microseconds.
 * 
 * program returns sidereal hours since sidereal midnight 
 * tjd 		julian day UT
 * eps 		obliquity of ecliptic, degrees 
 * nut 		nutation, degrees 
 */
double FAR PASCAL_CONV swe_sidtime0( double tjd, double eps, double nut )
{
  double jd0;    	/* Julian day at midnight Universal Time */
  double secs;   	/* Time of day, UT seconds since UT midnight */
  double eqeq, jd, T0, msday;
  double gmst;
  /* Julian day at given UT */
  jd = tjd;
  jd0 = floor(jd);
  secs = tjd - jd0;
  if( secs < 0.5 ) {
    jd0 -= 0.5;
    secs += 0.5;
  } else {
    jd0 += 0.5;
    secs -= 0.5;
  }
  secs *= 86400.0;
  /* Same but at 0h Universal Time of date */
  T0 = (jd0 - J2000)/36525.0;
  eqeq = 240.0 * nut * cos(eps * DEGTORAD);
    /* Greenwich Mean Sidereal Time at 0h UT of date */
  gmst = (( -6.2e-6*T0 + 9.3104e-2)*T0 + 8640184.812866)*T0 + 24110.54841;
  /* mean solar days per sidereal day at date T0, = 1.00273790934 in 1986 */
  msday = 1.0 + ((-1.86e-5*T0 + 0.186208)*T0 + 8640184.812866)/(86400.*36525.);
  /* Local apparent sidereal time at given UT at Greenwich */
  gmst = gmst + msday*secs + eqeq  /* + 240.0*tlong */;
  /* Sidereal seconds modulo 1 sidereal day */
  gmst = gmst - 86400.0 * floor( gmst/86400.0 );
  /* return in hours */
  gmst /= 3600;
#ifdef TRACE
  swi_open_trace(NULL);
  if (swi_trace_count < TRACE_COUNT_MAX) {
    if (swi_fp_trace_c != NULL) {
      fputs("\n/*SWE_SIDTIME0*/\n", swi_fp_trace_c);
      fprintf(swi_fp_trace_c, "  tjd = %.9f;", tjd);
      fprintf(swi_fp_trace_c, "  eps = %.9f;", eps);
      fprintf(swi_fp_trace_c, "  nut = %.9f;\n", nut);
      fprintf(swi_fp_trace_c, "  t = swe_sidtime0(tjd, eps, nut);\n");
      fputs("  printf(\"swe_sidtime0: %f\\tsidt = %f\\teps = %f\\tnut = %f\\t\\n\", ", swi_fp_trace_c);
      fputs("tjd, t, eps, nut);\n", swi_fp_trace_c);
      fflush(swi_fp_trace_c);
    }
    if (swi_fp_trace_out != NULL) {
      fprintf(swi_fp_trace_out, "swe_sidtime0: %f\tsidt = %f\teps = %f\tnut = %f\t\n", tjd, gmst, eps, nut);
      fflush(swi_fp_trace_out);
    }
  }
#endif
  return gmst;
}

/* sidereal time, without eps and nut as parameters.
 * tjd must be UT !!!
 * for more informsation, see comment with swe_sidtime0()
 */
double FAR PASCAL_CONV swe_sidtime(double tjd_ut)
{
  int i;
  double eps, nutlo[2], tsid;
  double tjde = tjd_ut + swe_deltat(tjd_ut);
  eps = swi_epsiln(tjde) * RADTODEG;
  swi_nutation(tjde, nutlo);
  for (i = 0; i < 2; i++)
    nutlo[i] *= RADTODEG;
  tsid = swe_sidtime0(tjd_ut, eps + nutlo[1], nutlo[0]);
#ifdef TRACE
  swi_open_trace(NULL);
  if (swi_trace_count < TRACE_COUNT_MAX) {
    if (swi_fp_trace_c != NULL) {
      fputs("\n/*SWE_SIDTIME*/\n", swi_fp_trace_c);
      fprintf(swi_fp_trace_c, "  tjd = %.9f;\n", tjd_ut);
      fprintf(swi_fp_trace_c, "  t = swe_sidtime(tjd);\n");
      fputs("  printf(\"swe_sidtime: %f\\t%f\\t\\n\", ", swi_fp_trace_c);
      fputs("tjd, t);\n", swi_fp_trace_c);
      fflush(swi_fp_trace_c);
    }
    if (swi_fp_trace_out != NULL) {
      fprintf(swi_fp_trace_out, "swe_sidtime: %f\t%f\t\n", tjd_ut, tsid);
      fflush(swi_fp_trace_out);
    }
  }
#endif
  return tsid;
}

/* SWISSEPH
 * generates name of ephemeris file
 * file name looks as follows:
 * swephpl.m30, where
 *
 * "sweph"              	"swiss ephemeris"
 * "pl","mo","as"               planet, moon, or asteroid 
 * "m"  or "_"                  BC or AD
 *
 * "30"                         start century
 * tjd        	= ephemeris file for which julian day
 * ipli       	= number of planet
 * fname      	= ephemeris file name
 */
void swi_gen_filename(double tjd, int ipli, char *fname) 
{  
  int icty;
  int ncties = (int) NCTIES;
  short gregflag;
  int jmon, jday, jyear, sgn;
  double jut;
  char *sform;
  switch(ipli) {
    case SEI_MOON:
      strcpy(fname, "semo");
      break;
    case SEI_EMB:
    case SEI_MERCURY:
    case SEI_VENUS:
    case SEI_MARS:
    case SEI_JUPITER:
    case SEI_SATURN:
    case SEI_URANUS:
    case SEI_NEPTUNE:
    case SEI_PLUTO:
    case SEI_SUNBARY:
      strcpy(fname, "sepl");
      break;
    case SEI_CERES:
    case SEI_PALLAS:
    case SEI_JUNO:
    case SEI_VESTA:
    case SEI_CHIRON:
    case SEI_PHOLUS:
      strcpy(fname, "seas");
      break;
    default: 	/* asteroid */
      sform = "ast%d%sse%05d.%s";
      if (ipli - SE_AST_OFFSET > 99999) 
	sform = "ast%d%ss%06d.%s";
      sprintf(fname, sform,
	(ipli - SE_AST_OFFSET) / 1000, DIR_GLUE, ipli - SE_AST_OFFSET, 
	SE_FILE_SUFFIX);
      return;	/* asteroids: only one file 3000 bc - 3000 ad */
      /* break; */
  }
  /* century of tjd */
  /* if tjd > 1600 then gregorian calendar */
  if (tjd >= 2305447.5) {
    gregflag = TRUE;
    swe_revjul(tjd, gregflag, &jyear, &jmon, &jday, &jut);
  /* else julian calendar */
  } else {
    gregflag = FALSE;
    swe_revjul(tjd, gregflag, &jyear, &jmon, &jday, &jut);
  }
  /* start century of file containing tjd */
  if (jyear < 0)
    sgn = -1;
  else
    sgn = 1;
  icty = jyear / 100;
  if (sgn < 0 && jyear % 100 != 0)
    icty -=1;
  while(icty % ncties != 0)
    icty--;
#if 0
  if (icty < BEG_YEAR / 100)
    icty = BEG_YEAR / 100;
  if (icty >= END_YEAR / 100)
    icty = END_YEAR / 100 - ncties;
#endif
  /* B.C. or A.D. */
  if (icty < 0) 
    strcat(fname, "m");
  else 
    strcat(fname, "_");
  icty = abs(icty);
  sprintf(fname + strlen(fname), "%02d.%s", icty, SE_FILE_SUFFIX);
#if 0
  printf("fname  %s\n", fname); 
  fflush(stdout);
#endif
}

/**************************************************************
cut the string s at any char in cutlist; put pointers to partial strings
into cpos[0..n-1], return number of partial strings;
if less than nmax fields are found, the first empty pointer is
set to NULL.
More than one character of cutlist in direct sequence count as one
separator only! cut_str_any("word,,,word2",","..) cuts only two parts,
cpos[0] = "word" and cpos[1] = "word2".
If more than nmax fields are found, nmax is returned and the
last field nmax-1 rmains un-cut.
**************************************************************/
int swi_cutstr(char *s, char *cutlist, char *cpos[], int nmax)
{
  int n = 1;
  cpos [0] = s;
  while (*s != '\0') {
    if ((strchr(cutlist, (int) *s) != NULL) && n < nmax) {
      *s = '\0';
      while (*(s + 1) != '\0' && strchr (cutlist, (int) *(s + 1)) != NULL) s++;
      cpos[n++] = s + 1;
    }
    if (*s == '\n' || *s == '\r') {	/* treat nl or cr like end of string */
      *s = '\0';
      break;
    }
    s++;
  }
  if (n < nmax) cpos[n] = NULL;
  return (n);
}	/* cutstr */

char *swi_right_trim(char *s)
{
  char *sp = s + strlen(s) - 1;
  while (isspace((int)(unsigned char) *sp) && sp >= s)
    *sp-- = '\0';
  return s;
}

/*
 * The following C code (by Rob Warnock rpw3@sgi.com) does CRC-32 in
 * BigEndian/BigEndian byte/bit order. That is, the data is sent most
 * significant byte first, and each of the bits within a byte is sent most
 * significant bit first, as in FDDI. You will need to twiddle with it to do
 * Ethernet CRC, i.e., BigEndian/LittleEndian byte/bit order.
 * 
 * The CRCs this code generates agree with the vendor-supplied Verilog models
 * of several of the popular FDDI "MAC" chips.
 */
static uint32 crc32_table[256];
/* Initialized first time "crc32()" is called. If you prefer, you can
 * statically initialize it at compile time. [Another exercise.]
 */

uint32 swi_crc32(unsigned char *buf, int len)
{
  unsigned char *p;
  uint32  crc;
  if (!crc32_table[1])    /* if not already done, */
    init_crc32();   /* build table */
  crc = 0xffffffff;       /* preload shift register, per CRC-32 spec */
  for (p = buf; len > 0; ++p, --len)
    crc = (crc << 8) ^ crc32_table[(crc >> 24) ^ *p];
  return ~crc;            /* transmit complement, per CRC-32 spec */
}

/*
 * Build auxiliary table for parallel byte-at-a-time CRC-32.
 */
#define CRC32_POLY 0x04c11db7     /* AUTODIN II, Ethernet, & FDDI */

static void init_crc32(void)
{
  int32 i, j;
  uint32 c;
  for (i = 0; i < 256; ++i) {
    for (c = i << 24, j = 8; j > 0; --j)
      c = c & 0x80000000 ? (c << 1) ^ CRC32_POLY : (c << 1);
    crc32_table[i] = c;
  }
}

/*******************************************************
 * other functions from swephlib.c;
 * they are not needed for Swiss Ephemeris,
 * but may be useful to former Placalc users.
 ********************************************************/

/************************************
normalize argument into interval [0..DEG360]
*************************************/
centisec FAR PASCAL_CONV swe_csnorm(centisec p)
{
  if (p < 0) 
    do { p += DEG360; } while (p < 0);
  else if (p >= DEG360)
    do { p -= DEG360; } while (p >= DEG360);
  return (p);
}

/************************************
distance in centisecs p1 - p2
normalized to [0..360[
**************************************/
centisec FAR PASCAL_CONV swe_difcsn (centisec p1, centisec p2)
{ 
  return (swe_csnorm(p1 - p2));
}

double FAR PASCAL_CONV swe_difdegn (double p1, double p2)
{ 
  return (swe_degnorm(p1 - p2));
}

/************************************
distance in centisecs p1 - p2
normalized to [-180..180[
**************************************/
centisec FAR PASCAL_CONV swe_difcs2n(centisec p1, centisec p2)
{ centisec dif;
  dif = swe_csnorm(p1 - p2);
  if (dif  >= DEG180) return (dif - DEG360);
  return (dif);
}

double FAR PASCAL_CONV swe_difdeg2n(double p1, double p2)
{ double dif;
  dif = swe_degnorm(p1 - p2);
  if (dif  >= 180.0) return (dif - 360.0);
  return (dif);
}

double FAR PASCAL_CONV swe_difrad2n(double p1, double p2)
{ double dif;
  dif = swe_radnorm(p1 - p2);
  if (dif  >= TWOPI / 2) return (dif - TWOPI);
  return (dif);
}

/*************************************
round second, but at 29.5959 always down
*************************************/ 
centisec FAR PASCAL_CONV swe_csroundsec(centisec x)	
{
  centisec t;
  t = (x + 50) / 100 *100L;	/* round to seconds */
  if (t > x && t % DEG30 == 0)  /* was rounded up to next sign */
    t = x / 100 * 100L;		/* round last second of sign downwards */
  return (t);
}

/*************************************
double to int32 with rounding, no overflow check
*************************************/ 
int32 FAR PASCAL_CONV swe_d2l(double x)		
{
  if (x >=0)
    return ((int32) (x + 0.5));
  else
    return (- (int32) (0.5 - x));
}

/*
 * monday = 0, ... sunday = 6
 */
int FAR PASCAL_CONV swe_day_of_week(double jd)
{
  return (((int) floor (jd - 2433282 - 1.5) %7) + 7) % 7;
}

char *FAR PASCAL_CONV swe_cs2timestr(CSEC t, int sep, AS_BOOL suppressZero, char *a)
/* does not suppress zeros in hours or minutes */
{
  /* static char a[9];*/
  centisec h,m,s;
  strcpy (a, "        ");
  a[2] = a [5] = sep;
  t = ((t + 50) / 100) % (24L *3600L); /* round to seconds */
  s = t % 60L;
  m = (t / 60) % 60L;
  h = t / 3600 % 100L;
  if (s == 0 && suppressZero)  
    a[5] = '\0';
  else {
    a [6] = (char) (s / 10 + '0');
    a [7] = (char) (s % 10 + '0');
  };
  a [0] = (char) (h / 10 + '0');
  a [1] = (char) (h % 10 + '0');
  a [3] = (char) (m / 10 + '0');
  a [4] = (char) (m % 10 + '0');
  return (a);
  } /* swe_cs2timestr() */

char *FAR PASCAL_CONV swe_cs2lonlatstr(CSEC t, char pchar, char mchar, char *sp)
{
  char a[10];	/* must be initialized at each call */
  char *aa;
  centisec h,m,s;
  strcpy (a, "      '  ");
  /* mask     dddEmm'ss" */
  if (t < 0 ) pchar = mchar;
  t = (ABS4 (t) + 50) / 100; /* round to seconds */
  s = t % 60L;
  m = t / 60 % 60L;
  h = t / 3600 % 1000L;
  if (s == 0)  
    a[6] = '\0';   /* cut off seconds */
  else {
    a [7] = (char) (s / 10 + '0');
    a [8] = (char) (s % 10 + '0');
  }
  a [3] = pchar;
  if (h > 99)  a [0] = (char) (h / 100 + '0');
  if (h > 9)  a [1] = (char) (h % 100 / 10 + '0');
  a [2] = (char) (h % 10 + '0');
  a [4] = (char) (m / 10 + '0');
  a [5] = (char) (m % 10 + '0');
  aa = a;
  while (*aa == ' ') aa++;
  strcpy(sp, aa);
  return (sp);
} /* swe_cs2lonlatstr() */

char *FAR PASCAL_CONV swe_cs2degstr(CSEC t, char *a)
  /* does  suppress leading zeros in degrees */
{
  /* char a[9];	 must be initialized at each call */
  centisec h,m,s;
  strcpy (a, "     '  ");
  a[2] = (unsigned char) ODEGREE_CHAR;
  t = t  / 100 % (30L*3600L); /* truncate to seconds */
  s = t % 60L;
  m = t / 60 % 60L;
  h = t / 3600 % 100L;	/* only 0..99 degrees */ 
  if (h > 9)  a [0] = (char) (h / 10 + '0');
  a [1] = (char) (h % 10 + '0');
  a [3] = (char) (m / 10 + '0');
  a [4] = (char) (m % 10 + '0');
  a [6] = (char) (s / 10 + '0');
  a [7] = (char) (s % 10 + '0');
  return (a);
} /* swe_cs2degstr() */

/*********************************************************
 *  function for splitting centiseconds into             *
 *  ideg 	degrees, 
 *  imin 	minutes, 
 *  isec 	seconds, 
 *  dsecfr	fraction of seconds 
 *  isgn	zodiac sign number; 
 *              or +/- sign
 *  
 *********************************************************/
void FAR PASCAL_CONV swe_split_deg(double ddeg, int32 roundflag, int32 *ideg, int32 *imin, int32 *isec, double *dsecfr, int32 *isgn)
{
  double dadd = 0;
  *isgn = 1;
  if (ddeg < 0) {
    *isgn = -1;
    ddeg = -ddeg;
  }
  if (roundflag & SE_SPLIT_DEG_ROUND_DEG) {
    dadd = 0.5;
  } else if (roundflag & SE_SPLIT_DEG_ROUND_MIN) {
    dadd = 0.5 / 60;
  } else if (roundflag & SE_SPLIT_DEG_ROUND_SEC) {
    dadd = 0.5 / 3600;
  }
  if (roundflag & SE_SPLIT_DEG_KEEP_DEG) {
    if ((int32) (ddeg + dadd) - (int32) ddeg > 0)
      dadd = 0;
  } else if (roundflag & SE_SPLIT_DEG_KEEP_SIGN) {
    if (fmod(ddeg, 30) + dadd >= 30)
      dadd = 0;
  }
  ddeg += dadd;
  if (roundflag & SE_SPLIT_DEG_ZODIACAL) {
    *isgn = (int32) (ddeg / 30);
    ddeg = fmod(ddeg, 30);
  }
  *ideg = (int32) ddeg;
  ddeg -= *ideg;
  *imin = (int32) (ddeg * 60);
  ddeg -= *imin / 60.0;
  *isec = (int32) (ddeg * 3600);
  if (!(roundflag & (SE_SPLIT_DEG_ROUND_DEG | SE_SPLIT_DEG_ROUND_MIN | SE_SPLIT_DEG_ROUND_SEC))) {
    *dsecfr = ddeg * 3600 - *isec;
  }
}  /* end split_deg */

double swi_kepler(double E, double M, double ecce)
{
  double dE = 1, E0;
  double x;
  /* simple formula for small eccentricities */
  if (ecce < 0.4) {
    while(dE > 1e-12) {
      E0 = E;
      E = M + ecce * sin(E0);
      dE = fabs(E - E0);
    }
  /* complicated formula for high eccentricities */
  } else {
    while(dE > 1e-12) {
      E0 = E;
      /*
       * Alois 21-jul-2000: workaround an optimizer problem in gcc 
       * swi_mod2PI sees very small negative argument e-322 and returns +2PI;
       * we avoid swi_mod2PI for small x.
       */
      x = (M + ecce * sin(E0) - E0) / (1 - ecce * cos(E0));
      dE = fabs(x);
      if (dE < 1e-2) {
	E = E0 + x;
      } else {
	E = swi_mod2PI(E0 + x);
	dE = fabs(E - E0);
      }
    }
  }
  return E;
}

void swi_FK4_FK5(double *xp, double tjd)
{
  if (xp[0] == 0 && xp[1] == 0 && xp[2] == 0)
    return;
  swi_cartpol(xp, xp);
  /* according to Expl.Suppl., p. 167f. */
  xp[0] += (0.035 + 0.085 * (tjd - B1950) / 36524.2198782) / 3600 * 15 * DEGTORAD;
  xp[3] += (0.085 / 36524.2198782) / 3600 * 15 * DEGTORAD;
  swi_polcart(xp, xp);
}

void swi_FK5_FK4(double *xp, double tjd)
{
  if (xp[0] == 0 && xp[1] == 0 && xp[2] == 0)
    return;
  swi_cartpol(xp, xp);
  /* according to Expl.Suppl., p. 167f. */
  xp[0] -= (0.035 + 0.085 * (tjd - B1950) / 36524.2198782) / 3600 * 15 * DEGTORAD;
  xp[3] -= (0.085 / 36524.2198782) / 3600 * 15 * DEGTORAD;
  swi_polcart(xp, xp);
}

#ifdef TRACE
void swi_open_trace(char *serr)
{
  swi_trace_count++;
  if (swi_trace_count >= TRACE_COUNT_MAX) {
    if (swi_trace_count == TRACE_COUNT_MAX) { 
      if (serr != NULL)
	sprintf(serr, "trace stopped, %d calls exceeded.", TRACE_COUNT_MAX);
      if (swi_fp_trace_out != NULL)
	fprintf(swi_fp_trace_out, "trace stopped, %d calls exceeded.\n", TRACE_COUNT_MAX);
      if (swi_fp_trace_c != NULL)
	fprintf(swi_fp_trace_c, "/* trace stopped, %d calls exceeded. */\n", TRACE_COUNT_MAX);
    }
    return;
  }
  if (swi_fp_trace_c == NULL) {
    char *sp, *sp1, fname[AS_MAXCH];
    int ipid;
    /* remove(fname_trace_c); */
    strcpy(fname, fname_trace_c);
#if TRACE == 2
    sp = strchr(fname_trace_c, '.');
    sp1 = strchr(fname, '.');
# if MSDOS
    ipid = _getpid();
# else
    ipid = getpid();
# endif
    sprintf(sp1, "_%d%s", ipid, sp);
#endif
    if ((swi_fp_trace_c = fopen(fname, FILE_A_ACCESS)) == NULL) {
      if (serr != NULL)
	sprintf(serr, "could not open trace output file '%s'", fname);
    } else {
      fputs("#include \"sweodef.h\"\n", swi_fp_trace_c);   
      fputs("#include \"swephexp.h\"\n\n", swi_fp_trace_c);   
      fputs("void main()\n{\n", swi_fp_trace_c);   
      fputs("  double tjd, t, nut, eps; int i, ipl, retc; int32 iflag;\n", swi_fp_trace_c);
      fputs("  double armc, geolat, cusp[12], ascmc[10]; int hsys;\n", swi_fp_trace_c);
      fputs("  double xx[6]; int32 iflgret;\n", swi_fp_trace_c);
      fputs("  char s[AS_MAXCH], star[AS_MAXCH], serr[AS_MAXCH];\n", swi_fp_trace_c);
      fflush(swi_fp_trace_c);
    }
  }
  if (swi_fp_trace_out == NULL) {
    char *sp, *sp1, fname[AS_MAXCH];
    int ipid;
    /* remove(fname_trace_out); */
    strcpy(fname, fname_trace_out);
#if TRACE == 2
    sp = strchr(fname_trace_out, '.');
    sp1 = strchr(fname, '.');
# if MSDOS
    ipid = _getpid();
# else
    ipid = getpid();
# endif
    sprintf(sp1, "_%d%s", ipid, sp);
#endif
    if ((swi_fp_trace_out = fopen(fname, FILE_A_ACCESS)) == NULL) {
      if (serr != NULL)
	sprintf(serr, "could not open trace output file '%s'", fname);
    }
  }
}
#endif
