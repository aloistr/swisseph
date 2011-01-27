
/* SWISSEPH
   $Header: /home/dieter/sweph/RCS/swephlib.c,v 1.75 2009/11/27 11:00:57 dieter Exp $

   SWISSEPH modules that may be useful for other applications
   e.g. chopt.c, venus.c, swetest.c

  Authors: Dieter Koch and Alois Treindl, Astrodienst Zurich

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

**************************************************************/
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
static double deltat_espenak_meeus_1620(double tjd);
static double deltat_longterm_morrison_stephenson(double tjd);
static double deltat_stephenson_morrison_1600(double tjd);
static double deltat_aa(double tjd);

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
 * Bretagnon, P. et al.: 2003, "Expressions for Precession Consistent with 
 * the IAU 2000A Model". A&A 400,785
 *B03  	84381.4088  	-46.836051*t  	-1667*10-7*t2  	+199911*10-8*t3  	-523*10-9*t4  	-248*10-10*t5  	-3*10-11*t6
 *C03   84381.406  	-46.836769*t  	-1831*10-7*t2  	+20034*10-7*t3  	-576*10-9*t4  	-434*10-10*t5
 *
 *  See precess and page B18 of the Astronomical Almanac.
 */
double swi_epsiln(double J) 
{
  double T, eps;
  T = (J - 2451545.0)/36525.0;
  if (PREC_IAU_1976 && fabs(T) <= PREC_IAU_1976_CTIES )
    eps = (((1.813e-3*T-5.9e-4)*T-46.8150)*T+84381.448)*DEGTORAD/3600;
  else if (PREC_IAU_2003 && fabs(T) <= PREC_IAU_2003_CTIES) 
    eps =  (((((-4.34e-8 * T -5.76e-7) * T +2.0034e-3) * T -1.831e-4) * T -46.836769) * T + 84381.406) * DEGTORAD / 3600.0; 
  else if (PREC_BRETAGNON_2003)
    eps =  ((((((-3e-11 * T - 2.48e-8) * T -5.23e-7) * T +1.99911e-3) * T -1.667e-4) * T -46.836051) * T + 84381.40880) * DEGTORAD / 3600.0;/* */
  else if (PREC_SIMON_1994) 
    eps =  (((((2.5e-8 * T -5.1e-7) * T +1.9989e-3) * T -1.52e-4) * T -46.80927) * T + 84381.412) * DEGTORAD / 3600.0;/* */
  else if (PREC_WILLIAMS_1994) 
    eps =  ((((-1.0e-6 * T +2.0e-3) * T -1.74e-4) * T -46.833960) * T + 84381.409) * DEGTORAD / 3600.0;/* */
  else { /* PREC_LASKAR_1986 */
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

#if PREC_BRETAGNON_2003
static double pAcof[] = {};
static double nodecof[] = {};
static double inclcof[] = {};
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
  /* Use IAU formula for a few centuries.  */
  if (PREC_IAU_1976 && fabs(T) <= PREC_IAU_1976_CTIES) {
    Z =  (( 0.017998*T + 0.30188)*T + 2306.2181)*T*DEGTORAD/3600;
    z =  (( 0.018203*T + 1.09468)*T + 2306.2181)*T*DEGTORAD/3600;
    TH = ((-0.041833*T - 0.42665)*T + 2004.3109)*T*DEGTORAD/3600;
  } else if (PREC_IAU_2003 && fabs(T) <= PREC_IAU_2003_CTIES) {
    Z =  (((((- 0.0000003173*T - 0.000005971)*T + 0.01801828)*T + 0.2988499)*T + 2306.083227)*T + 2.650545)*DEGTORAD/3600;
    z =  (((((- 0.0000002904*T - 0.000028596)*T + 0.01826837)*T + 1.0927348)*T + 2306.077181)*T - 2.650545)*DEGTORAD/3600;
    TH = ((((-0.00000011274*T - 0.000007089)*T - 0.04182264)*T - 0.4294934)*T + 2004.191903)*T*DEGTORAD/3600;
    /* AA 2006 B28:
    Z =  (((((- 0.0000002*T - 0.0000327)*T + 0.0179663)*T + 0.3019015)*T + 2306.0809506)*T + 2.5976176)*DEGTORAD/3600;
    z =  (((((- 0.0000003*T - 0.000047)*T + 0.0182237)*T + 1.0947790)*T + 2306.0803226)*T - 2.5976176)*DEGTORAD/3600;
    TH = ((((-0.0000001*T - 0.0000601)*T - 0.0418251)*T - 0.4269353)*T + 2004.1917476)*T*DEGTORAD/3600;
    */
  } else if (PREC_BRETAGNON_2003) {
    Z =  ((((((-0.00000000013*T - 0.0000003040)*T - 0.000005708)*T + 0.01801752)*T + 0.3023262)*T + 2306.080472)*T + 2.72767)*DEGTORAD/3600;
    z =  ((((((-0.00000000005*T - 0.0000002486)*T - 0.000028276)*T + 0.01826676)*T + 1.0956768)*T + 2306.076070)*T - 2.72767)*DEGTORAD/3600;
    TH = ((((((0.000000000009*T + 0.00000000036)*T -0.0000001127)*T - 0.000007291)*T - 0.04182364)*T - 0.4266980)*T + 2004.190936)*T*DEGTORAD/3600;
  } else {
    goto laskar;
  }
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
  done:
  for( i=0; i<3; i++ ) 
    R[i] = x[i];
  return(0);
}

#if NUT_IAU_1980
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
#endif

#if NUT_IAU_1980
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
#endif

#if NUT_IAU_2000A || NUT_IAU_2000B
/* Nutation IAU 2000A model 
 * (MHB2000 luni-solar and planetary nutation, without free core nutation)
 *
 * Function returns nutation in longitude and obliquity in radians with 
 * respect to the equinox of date. For the obliquity of the ecliptic
 * the calculation of Lieske & al. (1977) must be used.
 *
 * The precision in recent years is about 0.001 arc seconds.
 *
 * The calculation includes luni-solar and planetary nutation.
 * Free core nutation, which cannot be predicted, is omitted, 
 * the error being of the order of a few 0.0001 arc seconds.
 *
 * References:
 *
 * Capitaine, N., Wallace, P.T., Chapront, J., A & A 432, 366 (2005).
 *
 * Chapront, J., Chapront-Touze, M. & Francou, G., A & A 387, 700 (2002).
 *
 * Lieske, J.H., Lederle, T., Fricke, W. & Morando, B., "Expressions
 * for the precession quantities based upon the IAU (1976) System of
 * Astronomical Constants", A & A 58, 1-16 (1977).
 *
 * Mathews, P.M., Herring, T.A., Buffet, B.A., "Modeling of nutation
 * and precession   New nutation series for nonrigid Earth and
 * insights into the Earth's interior", J.Geophys.Res., 107, B4,
 * 2002.  
 *
 * Simon, J.-L., Bretagnon, P., Chapront, J., Chapront-Touze, M.,
 * Francou, G., Laskar, J., A & A 282, 663-683 (1994).
 *
 * Souchay, J., Loysel, B., Kinoshita, H., Folgueira, M., A & A Supp.
 * Ser. 135, 111 (1999).
 *
 * Wallace, P.T., "Software for Implementing the IAU 2000
 * Resolutions", in IERS Workshop 5.1 (2002).
 *
 * Nutation IAU 2000A series in: 
 * Kaplan, G.H., United States Naval Observatory Circular No. 179 (Oct. 2005)
 * aa.usno.navy.mil/publications/docs/Circular_179.html
 *
 * MHB2000 code at
 * - ftp://maia.usno.navy.mil/conv2000/chapter5/IAU2000A.
 * - http://www.iau-sofa.rl.ac.uk/2005_0901/Downloads.html
 */

#include "swenut2000a.h"
int swi_nutation(double J, double *nutlo) 
{
  int i, j, k, inls;
  double M, SM, F, D, OM;
#if NUT_IAU_2000A
  double AL, ALSU, AF, AD, AOM, APA;
  double ALME, ALVE, ALEA, ALMA, ALJU, ALSA, ALUR, ALNE;
#endif
  double darg, sinarg, cosarg;
  double dpsi = 0, deps = 0;
  double T = (J - J2000 ) / 36525.0;
  /* luni-solar nutation */
  /* Fundamental arguments, Simon & al. (1994) */
  /* Mean anomaly of the Moon. */
  M  = swe_degnorm(( 485868.249036 +
	      T*( 1717915923.2178 +
	      T*(         31.8792 +
	      T*(          0.051635 +
	      T*(        - 0.00024470 ))))) / 3600.0) * DEGTORAD;
  /* Mean anomaly of the Sun */
  SM = swe_degnorm((1287104.79305 +
	      T*(  129596581.0481 +
	      T*(        - 0.5532 +
	      T*(          0.000136 +
	      T*(        - 0.00001149 ))))) / 3600.0) * DEGTORAD;
  /* Mean argument of the latitude of the Moon. */
  F   = swe_degnorm(( 335779.526232 +
	      T*( 1739527262.8478 +
	      T*(       - 12.7512 +
	      T*(       -  0.001037 +
	      T*(          0.00000417 ))))) / 3600.0) * DEGTORAD;
  /* Mean elongation of the Moon from the Sun. */
  D   = swe_degnorm((1072260.70369 +
	      T*( 1602961601.2090 +
	      T*(        - 6.3706 +
	      T*(          0.006593 +
	      T*(        - 0.00003169 ))))) / 3600.0) * DEGTORAD;
  /* Mean longitude of the ascending node of the Moon. */
  OM  = swe_degnorm(( 450160.398036 +
	      T*(  - 6962890.5431 +
	      T*(          7.4722 +
	      T*(          0.007702 +
	      T*(        - 0.00005939 ))))) / 3600.0) * DEGTORAD;
  /* luni-solar nutation series, in reverse order, starting with small terms */
#if NUT_IAU_2000B
  inls = NLS_2000B;
#else
  inls = NLS;
#endif
  for (i = inls - 1; i >= 0; i--) {
    j = i * 5;
    darg = swe_radnorm((double) nls[j + 0] * M  +
		       (double) nls[j + 1] * SM +
		       (double) nls[j + 2] * F   +
		       (double) nls[j + 3] * D   +
		       (double) nls[j + 4] * OM);
    sinarg = sin(darg);
    cosarg = cos(darg);
    k = i * 6;
    dpsi += (cls[k+0] + cls[k+1] * T) * sinarg + cls[k+2] * cosarg;
    deps += (cls[k+3] + cls[k+4] * T) * cosarg + cls[k+5] * sinarg;
  }
  nutlo[0] = dpsi * O1MAS2DEG;
  nutlo[1] = deps * O1MAS2DEG;
#if NUT_IAU_2000A
  /* planetary nutation 
   * note: The MHB2000 code computes the luni-solar and planetary nutation
   * in different routines, using slightly different Delaunay
   * arguments in the two cases.  This behaviour is faithfully
   * reproduced here.  Use of the Simon et al. expressions for both
   * cases leads to negligible changes, well below 0.1 microarcsecond.*/
  /* Mean anomaly of the Moon.*/
  AL = swe_radnorm(2.35555598 + 8328.6914269554 * T);
  /* Mean anomaly of the Sun.*/
  ALSU = swe_radnorm(6.24006013 + 628.301955 * T);
  /* Mean argument of the latitude of the Moon. */
  AF = swe_radnorm(1.627905234 + 8433.466158131 * T);
  /* Mean elongation of the Moon from the Sun. */
  AD = swe_radnorm(5.198466741 + 7771.3771468121 * T);
  /* Mean longitude of the ascending node of the Moon. */
  AOM = swe_radnorm(2.18243920 - 33.757045 * T);
  /* General accumulated precession in longitude. */
  APA = (0.02438175 + 0.00000538691 * T) * T;
  /* Planetary longitudes, Mercury through Neptune (Souchay et al. 1999). */
  ALME = swe_radnorm(4.402608842 + 2608.7903141574 * T);
  ALVE = swe_radnorm(3.176146697 + 1021.3285546211 * T);
  ALEA = swe_radnorm(1.753470314 +  628.3075849991 * T);
  ALMA = swe_radnorm(6.203480913 +  334.0612426700 * T);
  ALJU = swe_radnorm(0.599546497 +   52.9690962641 * T);
  ALSA = swe_radnorm(0.874016757 +   21.3299104960 * T);
  ALUR = swe_radnorm(5.481293871 +    7.4781598567 * T);
  ALNE = swe_radnorm(5.321159000 +    3.8127774000 * T);
  /* planetary nutation series (in reverse order).*/
  dpsi = 0;
  deps = 0;
  for (i = NPL - 1; i >= 0; i--) {
    j = i * 14;
    darg = swe_radnorm((double) npl[j + 0] * AL   +
	(double) npl[j + 1] * ALSU +
	(double) npl[j + 2] * AF   +
	(double) npl[j + 3] * AD   +
	(double) npl[j + 4] * AOM  +
	(double) npl[j + 5] * ALME +
	(double) npl[j + 6] * ALVE +
	(double) npl[j + 7] * ALEA +
	(double) npl[j + 8] * ALMA +
	(double) npl[j + 9] * ALJU +
	(double) npl[j +10] * ALSA +
	(double) npl[j +11] * ALUR +
	(double) npl[j +12] * ALNE +
	(double) npl[j +13] * APA);
    k = i * 4;
    sinarg = sin(darg);
    cosarg = cos(darg);
    dpsi += (double) icpl[k+0] * sinarg + (double) icpl[k+1] * cosarg;
    deps += (double) icpl[k+2] * sinarg + (double) icpl[k+3] * cosarg;
  }
  nutlo[0] += dpsi * O1MAS2DEG;
  nutlo[1] += deps * O1MAS2DEG;
#if 1
  /* changes required by adoption of P03 precession 
   * according to Capitaine et al. A & A 412, 366 (2005) */
  dpsi = -8.1 * sin(OM) - 0.6 * sin(2 * F - 2 * D + 2 * OM);
  dpsi += T * (47.8 * sin(OM) + 3.7 * sin(2 * F - 2 * D + 2 * OM) + 0.6 * sin(2 * F + 2 * OM) - 0.6 * sin(2 * OM)); 
  deps = T * (-25.6 * cos(OM) - 1.6 * cos(2 * F - 2 * D + 2 * OM));
  nutlo[0] += dpsi / (3600.0 * 1000000.0);
  nutlo[1] += deps / (3600.0 * 1000000.0);
#endif
#endif
  nutlo[0] *= DEGTORAD;
  nutlo[1] *= DEGTORAD;
  return 0;
}
#endif

/* GCRS to J2000 */
void swi_bias(double *x, int32 iflag, AS_BOOL backward)
{
#if 0
  double DAS2R = 1.0 / 3600.0 * DEGTORAD;
  double dpsi_bias = -0.041775 * DAS2R;
  double deps_bias = -0.0068192 * DAS2R;
  double dra0 = -0.0146 * DAS2R;
  double deps2000 = 84381.448 * DAS2R; 
#endif
  double xx[6], rb[3][3];
  int i;
  rb[0][0] = +0.9999999999999942;
  rb[0][1] = +0.0000000707827948;
  rb[0][2] = -0.0000000805621738;
  rb[1][0] = -0.0000000707827974; 
  rb[1][1] = +0.9999999999999969;
  rb[1][2] = -0.0000000330604088;
  rb[2][0] = +0.0000000805621715;
  rb[2][1] = +0.0000000330604145;
  rb[2][2] = +0.9999999999999962;
  if (backward) {
    for (i = 0; i <= 2; i++) {
      xx[i] = x[0] * rb[i][0] +
	      x[1] * rb[i][1] +
	      x[2] * rb[i][2];
      if (iflag & SEFLG_SPEED)
	xx[i+3] = x[3] * rb[i][0] +
	      x[4] * rb[i][1] +
	      x[5] * rb[i][2];
    }
  } else {
    for (i = 0; i <= 2; i++) {
      xx[i] = x[0] * rb[0][i] +
	      x[1] * rb[1][i] +
	      x[2] * rb[2][i];
      if (iflag & SEFLG_SPEED)
	xx[i+3] = x[3] * rb[0][i] +
	      x[4] * rb[1][i] +
	      x[5] * rb[2][i];
    }
  }
  for (i = 0; i <= 2; i++) x[i] = xx[i];
  if (iflag & SEFLG_SPEED)
    for (i = 3; i <= 5; i++) x[i] = xx[i];
}


/* GCRS to FK5 */
void swi_icrs2fk5(double *x, int32 iflag, AS_BOOL backward)
{
#if 0
  double DAS2R = 1.0 / 3600.0 * DEGTORAD;
  double dra0 = -0.0229 * DAS2R;
  double dxi0 =  0.0091 * DAS2R;
  double det0 = -0.0199 * DAS2R;
#endif
  double xx[6], rb[3][3];
  int i;
  rb[0][0] = +0.9999999999999928;
  rb[0][1] = +0.0000001110223287;
  rb[0][2] = +0.0000000441180557;
  rb[1][0] = -0.0000001110223330; 
  rb[1][1] = +0.9999999999999891;
  rb[1][2] = +0.0000000964779176;
  rb[2][0] = -0.0000000441180450;
  rb[2][1] = -0.0000000964779225;
  rb[2][2] = +0.9999999999999943;
  if (backward) {
    for (i = 0; i <= 2; i++) {
      xx[i] = x[0] * rb[i][0] +
	      x[1] * rb[i][1] +
	      x[2] * rb[i][2];
      if (iflag & SEFLG_SPEED)
	xx[i+3] = x[3] * rb[i][0] +
	      x[4] * rb[i][1] +
	      x[5] * rb[i][2];
    }
  } else {
    for (i = 0; i <= 2; i++) {
      xx[i] = x[0] * rb[0][i] +
	      x[1] * rb[1][i] +
	      x[2] * rb[2][i];
      if (iflag & SEFLG_SPEED)
	xx[i+3] = x[3] * rb[0][i] +
	      x[4] * rb[1][i] +
	      x[5] * rb[2][i];
    }
  }
  for (i = 0; i <= 5; i++) x[i] = xx[i];
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
 * -1000 - 1620:
 * ---------------------------------
 * For dates between -500 and 1600, the table given by Morrison &
 * Stephenson (2004; p. 332) is used, with linear interpolation.
 * This table is based on an assumed value of ndot = -26.
 * The program adjusts for ndot = -25.7376.
 * For 1600 - 1620, a linear interpolation between the last value
 * of the latter and the first value of the former table is made.
 *
 * before -1000:
 * ---------------------------------
 * For times before -1100, a formula of Morrison & Stephenson (2004) 
 * (p. 332) is used: 
 * dt = 32 * t * t - 20 sec, where t is centuries from 1820 AD.
 * For -1100 to -1000, a transition from this formula to the Stephenson
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
 * Morrison, L. V., and F.R. Stephenson, "Historical Values of the Earth's 
 * Clock Error DT and the Calculation of Eclipses", JHA xxxv (2004), 
 * pp.327-336
 * 
 * Table from AA for 1620 through today
 * Note, Stephenson and Morrison's table starts at the year 1630.
 * The Chapronts' table does not agree with the Almanac prior to 1630.
 * The actual accuracy decreases rapidly prior to 1780.
 *
 * Jean Meeus, Astronomical Algorithms, 2nd edition, 1998.
 * 
 * For a comprehensive collection of publications and formulae, see:
 * http://www.phys.uu.nl/~vgent/deltat/deltat_modern.htm
 * http://www.phys.uu.nl/~vgent/deltat/deltat_old.htm
 * 
 * For future values of delta t, the following data from the 
 * Earth Orientation Department of the US Naval Observatory can be used:
 * (TAI-UTC) from: ftp://maia.usno.navy.mil/ser7/tai-utc.dat
 * (UT1-UTC) from: ftp://maia.usno.navy.mil/ser7/finals.all
 * file description in: ftp://maia.usno.navy.mil/ser7/readme.finals
 * Delta T = TAI-UT1 + 32.184 sec = (TAI-UTC) - (UT1-UTC) + 32.184 sec
 *
 * Also, there is the following file: 
 * http://maia.usno.navy.mil/ser7/deltat.data, but it is about 3 months
 * behind (on 3 feb 2009)
 *
 * Last update of table dt[]: Dieter Koch, 27 april 2010.
 * ATTENTION: Whenever updating this table, do not forget to adjust
 * the macros TABEND and TABSIZ !
 */
#define TABSTART 	1620
#define TABEND 		2017
#define TABSIZ 		(TABEND-TABSTART+1) 
/* we make the table greater for additional values read from external file */
#define TABSIZ_SPACE 	(TABSIZ+100)
static double FAR dt[TABSIZ_SPACE] = {
/* 1620.0 thru 1659.0 */
124.00, 119.00, 115.00, 110.00, 106.00, 102.00, 98.00, 95.00, 91.00, 88.00,
85.00, 82.00, 79.00, 77.00, 74.00, 72.00, 70.00, 67.00, 65.00, 63.00,
62.00, 60.00, 58.00, 57.00, 55.00, 54.00, 53.00, 51.00, 50.00, 49.00,
48.00, 47.00, 46.00, 45.00, 44.00, 43.00, 42.00, 41.00, 40.00, 38.00,
/* 1660.0 thru 1699.0 */
37.00, 36.00, 35.00, 34.00, 33.00, 32.00, 31.00, 30.00, 28.00, 27.00,
26.00, 25.00, 24.00, 23.00, 22.00, 21.00, 20.00, 19.00, 18.00, 17.00,
16.00, 15.00, 14.00, 14.00, 13.00, 12.00, 12.00, 11.00, 11.00, 10.00,
10.00, 10.00, 9.00, 9.00, 9.00, 9.00, 9.00, 9.00, 9.00, 9.00,
/* 1700.0 thru 1739.0 */
9.00, 9.00, 9.00, 9.00, 9.00, 9.00, 9.00, 9.00, 10.00, 10.00,
10.00, 10.00, 10.00, 10.00, 10.00, 10.00, 10.00, 11.00, 11.00, 11.00,
11.00, 11.00, 11.00, 11.00, 11.00, 11.00, 11.00, 11.00, 11.00, 11.00,
11.00, 11.00, 11.00, 11.00, 12.00, 12.00, 12.00, 12.00, 12.00, 12.00,
/* 1740.0 thru 1779.0 */
12.00, 12.00, 12.00, 12.00, 13.00, 13.00, 13.00, 13.00, 13.00, 13.00,
13.00, 14.00, 14.00, 14.00, 14.00, 14.00, 14.00, 14.00, 15.00, 15.00,
15.00, 15.00, 15.00, 15.00, 15.00, 16.00, 16.00, 16.00, 16.00, 16.00,
16.00, 16.00, 16.00, 16.00, 16.00, 17.00, 17.00, 17.00, 17.00, 17.00,
/* 1780.0 thru 1799.0 */
17.00, 17.00, 17.00, 17.00, 17.00, 17.00, 17.00, 17.00, 17.00, 17.00,
17.00, 17.00, 16.00, 16.00, 16.00, 16.00, 15.00, 15.00, 14.00, 14.00,
/* 1800.0 thru 1819.0 */
13.70, 13.40, 13.10, 12.90, 12.70, 12.60, 12.50, 12.50, 12.50, 12.50,
12.50, 12.50, 12.50, 12.50, 12.50, 12.50, 12.50, 12.40, 12.30, 12.20,
/* 1820.0 thru 1859.0 */
12.00, 11.70, 11.40, 11.10, 10.60, 10.20, 9.60, 9.10, 8.60, 8.00,
7.50, 7.00, 6.60, 6.30, 6.00, 5.80, 5.70, 5.60, 5.60, 5.60,
5.70, 5.80, 5.90, 6.10, 6.20, 6.30, 6.50, 6.60, 6.80, 6.90,
7.10, 7.20, 7.30, 7.40, 7.50, 7.60, 7.70, 7.70, 7.80, 7.80,
/* 1860.0 thru 1899.0 */
7.88, 7.82, 7.54, 6.97, 6.40, 6.02, 5.41, 4.10, 2.92, 1.82,
1.61, .10, -1.02, -1.28, -2.69, -3.24, -3.64, -4.54, -4.71, -5.11,
-5.40, -5.42, -5.20, -5.46, -5.46, -5.79, -5.63, -5.64, -5.80, -5.66,
-5.87, -6.01, -6.19, -6.64, -6.44, -6.47, -6.09, -5.76, -4.66, -3.74,
/* 1900.0 thru 1939.0 */
-2.72, -1.54, -.02, 1.24, 2.64, 3.86, 5.37, 6.14, 7.75, 9.13,
10.46, 11.53, 13.36, 14.65, 16.01, 17.20, 18.24, 19.06, 20.25, 20.95,
21.16, 22.25, 22.41, 23.03, 23.49, 23.62, 23.86, 24.49, 24.34, 24.08,
24.02, 24.00, 23.87, 23.95, 23.86, 23.93, 23.73, 23.92, 23.96, 24.02,
/* 1940.0 thru 1979.0 */
 24.33, 24.83, 25.30, 25.70, 26.24, 26.77, 27.28, 27.78, 28.25, 28.71,
 29.15, 29.57, 29.97, 30.36, 30.72, 31.07, 31.35, 31.68, 32.18, 32.68,
 33.15, 33.59, 34.00, 34.47, 35.03, 35.73, 36.54, 37.43, 38.29, 39.20,
 40.18, 41.17, 42.23, 43.37, 44.49, 45.48, 46.46, 47.52, 48.53, 49.59,
/* 1980.0 thru 1999.0 */
 50.54, 51.38, 52.17, 52.96, 53.79, 54.34, 54.87, 55.32, 55.82, 56.30,
 56.86, 57.57, 58.31, 59.12, 59.98, 60.78, 61.63, 62.30, 62.97, 63.47,
/* 2000.0 thru 2009.0 */
 63.83, 64.09, 64.30, 64.47, 64.57, 64.69, 64.85, 65.15, 65.46, 65.78,      
/* 2010.0 thru 2019.0 */
 66.07, 66.32,
/* Extrapolated values, 2011 - 2014 */
             67.00, 67.50, 68.00, 68.50, 69.00, 69.50,
};
#define ESPENAK_MEEUS_2006 TRUE
#define TAB2_SIZ	27
#define TAB2_START	(-1000)
#define TAB2_END	1600
#define TAB2_STEP	100
#define LTERM_EQUATION_YSTART	1820
#define LTERM_EQUATION_COEFF	32
/* Table for -1000 through 1600, from Morrison & Stephenson (2004).  */
static short FAR dt2[TAB2_SIZ] = {
/*-1000  -900  -800  -700  -600  -500  -400  -300  -200  -100*/
  25400,23700,22000,21000,19040,17190,15530,14080,12790,11640,
/*    0   100   200   300   400   500   600   700   800   900*/
  10580, 9600, 8640, 7680, 6700, 5710, 4740, 3810, 2960, 2200,
/* 1000  1100  1200  1300  1400  1500  1600,                 */
   1570, 1090,  740,  490,  320,  200,  120,  
};
/* returns DeltaT (ET - UT) in days
 * double tjd 	= 	julian day in UT
 */
#define DEMO 0
double FAR PASCAL_CONV swe_deltat(double tjd)
{
  double ans = 0;
  double B, Y, Ygreg, dd;
  int iy;
  /* read additional values from swedelta.txt */
  AS_BOOL use_espenak_meeus = ESPENAK_MEEUS_2006;
  Y = 2000.0 + (tjd - J2000)/365.25;
  Ygreg = 2000.0 + (tjd - J2000)/365.2425;
  /* Before 1633 AD, if the macro ESPENAK_MEEUS_2006 is TRUE: 
   * Polynomials by Espenak & Meeus 2006, derived from Stephenson & Morrison 
   * 2004. 
   * Note, Espenak & Meeus use their formulae only from 2000 BC on.
   * However, they use the long-term formula of Morrison & Stephenson,
   * which can be used even for the remoter past.
   */
  if (use_espenak_meeus && tjd < 2317746.13090277789) {
    return deltat_espenak_meeus_1620(tjd);
  }
  /* If the macro ESPENAK_MEEUS_2006 is FALSE:
   * Before 1620, we follow Stephenson & Morrsion 2004. For the tabulated 
   * values 1000 BC through 1600 AD, we use linear interpolation.
   */
  if (Y < TABSTART) {
    if (Y < TAB2_END) {
      return deltat_stephenson_morrison_1600(tjd);
    } else {
      /* between 1600 and 1620:
       * linear interpolation between 
       * end of table dt2 and start of table dt */
      if (Y >= TAB2_END) { 
	B = TABSTART - TAB2_END;
	iy = (TAB2_END - TAB2_START) / TAB2_STEP;
	dd = (Y - TAB2_END) / B;
	/*ans = dt2[iy] + dd * (dt[0] / 100.0 - dt2[iy]);*/
	ans = dt2[iy] + dd * (dt[0] - dt2[iy]);
	ans = adjust_for_tidacc(ans, Ygreg);
	return ans / 86400.0;
      }
    }
  }
  /* 1620 - today + a few years (tabend):
   * Besselian interpolation from tabulated values in table dt.
   * See AA page K11.
   */
  if (Y >= TABSTART) {
    return deltat_aa(tjd);
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

static double deltat_aa(double tjd)
{
  double ans = 0, ans2, ans3;
  double p, B, B2, Y, dd;
  double d[6];
  int i, iy, k;
  /* read additional values from swedelta.txt */
  int tabsiz = init_dt();
  int tabend = TABSTART + tabsiz - 1;
  /*Y = 2000.0 + (tjd - J2000)/365.25;*/
  Y = 2000.0 + (tjd - J2000)/365.2425;
  if (Y <= tabend) {
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
    ans = adjust_for_tidacc(ans, Y);
    return ans / 86400.0;
  }
  /* today - : 
   * Formula Stephenson (1997; p. 507),
   * with modification to avoid jump at end of AA table,
   * similar to what Meeus 1998 had suggested.
   * Slow transition within 100 years.
   */
  B = 0.01 * (Y - 1820);
  ans = -20 + 31 * B * B;
  /* slow transition from tabulated values to Stephenson formula: */
  if (Y <= tabend+100) {
    B2 = 0.01 * (tabend - 1820);
    ans2 = -20 + 31 * B2 * B2;
    ans3 = dt[tabsiz-1];
    dd = (ans2 - ans3);
    ans += dd * (Y - (tabend + 100)) * 0.01;
  }
  return ans / 86400.0;
}

static double deltat_longterm_morrison_stephenson(double tjd)
{
  double Ygreg =  2000.0 + (tjd - J2000)/365.2425;
  double u = (Ygreg  - 1820) / 100.0;
  return (-20 + 32 * u * u);
}

static double deltat_stephenson_morrison_1600(double tjd)
{
  double ans = 0, ans2, ans3;
  double p, B, dd;
  double tjd0;
  int iy;
  /* read additional values from swedelta.txt */
  double Y = 2000.0 + (tjd - J2000)/365.2425;
  /* double Y = 2000.0 + (tjd - J2000)/365.25;*/
  /* before -1000:
   * formula by Stephenson&Morrison (2004; p. 335) but adjusted to fit the 
   * starting point of table dt2. */
  if( Y < TAB2_START ) {
    /*B = (Y - LTERM_EQUATION_YSTART) * 0.01;
    ans = -20 + LTERM_EQUATION_COEFF * B * B;*/
    ans = deltat_longterm_morrison_stephenson(tjd);
    ans = adjust_for_tidacc(ans, Y);
    /* transition from formula to table over 100 years */
    if (Y >= TAB2_START - 100) {
      /* starting value of table dt2: */
      ans2 = adjust_for_tidacc(dt2[0], TAB2_START);
      /* value of formula at epoch TAB2_START */
      /* B = (TAB2_START - LTERM_EQUATION_YSTART) * 0.01;
      ans3 = -20 + LTERM_EQUATION_COEFF * B * B;*/
      tjd0 = (TAB2_START - 2000) * 365.2425 + J2000;
      ans3 = deltat_longterm_morrison_stephenson(tjd0);
      ans3 = adjust_for_tidacc(ans3, Y);
      dd = ans3 - ans2;
      B = (Y - (TAB2_START - 100)) * 0.01;
      /* fit to starting point of table dt2. */
      ans = ans - dd * B;
    }
  }
  /* between -1000 and 1600: 
   * linear interpolation between values of table dt2 (Stephenson&Morrison 2004) */
  if (Y >= TAB2_START && Y < TAB2_END) { 
    double Yjul = 2000 + (tjd - 2451557.5) / 365.25;
    p = floor(Yjul);
    iy = (int) ((p - TAB2_START) / TAB2_STEP);
    dd = (Yjul - (TAB2_START + TAB2_STEP * iy)) / TAB2_STEP;
    ans = dt2[iy] + (dt2[iy+1] - dt2[iy]) * dd;
    /* correction for tidal acceleration used by our ephemeris */
    ans = adjust_for_tidacc(ans, Y);
  }
  ans /= 86400.0;
  return ans;
}

static double deltat_espenak_meeus_1620(double tjd)
{
  double ans = 0;
  double Ygreg;
  double u;
  /* double Y = 2000.0 + (tjd - J2000)/365.25;*/
  Ygreg = 2000.0 + (tjd - J2000)/365.2425;
  if (Ygreg < -500) {
    ans = deltat_longterm_morrison_stephenson(tjd);
  } else if (Ygreg < 500) {
    u = Ygreg / 100.0;
    ans = (((((0.0090316521 * u + 0.022174192) * u - 0.1798452) * u - 5.952053) * u+ 33.78311) * u - 1014.41) * u + 10583.6;
  } else if (Ygreg < 1600) {
    u = (Ygreg - 1000) / 100.0;
    ans = (((((0.0083572073 * u - 0.005050998) * u - 0.8503463) * u + 0.319781) * u + 71.23472) * u - 556.01) * u + 1574.2;
  } else if (Ygreg < 1700) {
    u = Ygreg - 1600;
    ans = 120 - 0.9808 * u - 0.01532 * u * u + u * u * u / 7129.0;
  } else if (Ygreg < 1800) {
    u = Ygreg - 1700;
    ans = (((-u / 1174000.0 + 0.00013336) * u - 0.0059285) * u + 0.1603) * u + 8.83;
  } else if (Ygreg < 1860) {
    u = Ygreg - 1800;
    ans = ((((((0.000000000875 * u - 0.0000001699) * u + 0.0000121272) * u - 0.00037436) * u + 0.0041116) * u + 0.0068612) * u - 0.332447) * u + 13.72;
  } else if (Ygreg < 1900) {
    u = Ygreg - 1860;
    ans = ((((u / 233174.0 - 0.0004473624) * u + 0.01680668) * u - 0.251754) * u + 0.5737) * u + 7.62;
  } else if (Ygreg < 1920) {
    u = Ygreg - 1900;
    ans = (((-0.000197 * u + 0.0061966) * u - 0.0598939) * u + 1.494119) * u -2.79;
  } else if (Ygreg < 1941) {
    u = Ygreg - 1920;
    ans = 21.20 + 0.84493 * u - 0.076100 * u * u + 0.0020936 * u * u * u;
  } else if (Ygreg < 1961) {
    u = Ygreg - 1950;
    ans = 29.07 + 0.407 * u - u * u / 233.0 + u * u * u / 2547.0;
  } else if (Ygreg < 1986) {
    u = Ygreg - 1975;
    ans = 45.45 + 1.067 * u - u * u / 260.0 - u * u * u / 718.0;
  } else if (Ygreg < 2005) {
    u = Ygreg - 2000;
    ans = ((((0.00002373599 * u + 0.000651814) * u + 0.0017275) * u - 0.060374) * u + 0.3345) * u + 63.86;
  }
  ans = adjust_for_tidacc(ans, Ygreg);
  ans /= 86400.0;
  return ans;
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
    if ((fp = swi_fopen(-1, "swe_deltat.txt", swed.ephepath, NULL)) == NULL
      && (fp = swi_fopen(-1, "sedeltat.txt", swed.ephepath, NULL)) == NULL)
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
      /*dt[tab_index] = (short) (atof(sp) * 100 + 0.5);*/
      dt[tab_index] = atof(sp);
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
  double eqeq, jd, tu, tt, msday;
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
  tu = (jd0 - J2000)/36525.0; /* UT1 in centuries after J2000 */
if (PREC_IAU_2003) {
  tt = (jd0 + swe_deltat(jd0) - J2000)/36525.0; /* TT in centuries after J2000 */
  gmst = (((-0.000000002454*tt - 0.00000199708)*tt - 0.0000002926)*tt + 0.092772110)*tt*tt + 307.4771013*(tt-tu) + 8640184.79447825*tu + 24110.5493771;
  /* mean solar days per sidereal day at date tu;
   * for the derivative of gmst, we can assume UT1 =~ TT */
  msday = 1 + ((((-0.000000012270*tt - 0.00000798832)*tt - 0.0000008778)*tt + 0.185544220)*tt + 8640184.79447825)/(86400.*36525.);
} else {
    /* Greenwich Mean Sidereal Time at 0h UT of date */
  gmst = (( -6.2e-6*tu + 9.3104e-2)*tu + 8640184.812866)*tu + 24110.54841;
  /* mean solar days per sidereal day at date tu, = 1.00273790934 in 1986 */
  msday = 1.0 + ((-1.86e-5*tu + 0.186208)*tu + 8640184.812866)/(86400.*36525.);
}
  /* Local apparent sidereal time at given UT at Greenwich */
  eqeq = 240.0 * nut * cos(eps * DEGTORAD);
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
  t = t  / 100 % (30L*3600L); /* truncate to seconds */
  s = t % 60L;
  m = t / 60 % 60L;
  h = t / 3600 % 100L;	/* only 0..99 degrees */ 
  sprintf(a, "%2d%s%02d'%02d", h, ODEGREE_STRING, m, s);
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

char *swi_strcpy(char *to, char *from)
{
  char *s;
  if (*from == '\0') {
    *to = '\0';
    return to;
  }
  s = strdup(from);
  if (s == NULL) {
    strcpy(to, from);
    return to;
  }
  strcpy(to, s);
  free(s);
  return to;
}

char *swi_strncpy(char *to, char *from, size_t n)
{ 
  char *s;
  if (*from == '\0') {
    return to;
  }
  s = strdup(from);
  if (s == NULL) {
    strncpy(to, from, n);
    return to;
  }
  strncpy(to, s, n);
  free(s);
  return to;
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
    char fname[AS_MAXCH];
#if TRACE == 2
    char *sp, *sp1;
    int ipid;
#endif
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
    char fname[AS_MAXCH];
#if TRACE == 2
    char *sp, *sp1;
    int ipid;
#endif
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
