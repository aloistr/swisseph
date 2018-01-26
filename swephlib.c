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
# define strdup _strdup
#endif

#ifdef TRACE
void swi_open_trace(char *serr);
TLS FILE *swi_fp_trace_c = NULL;
TLS FILE *swi_fp_trace_out = NULL;
TLS int32 swi_trace_count = 0;
#endif

static void init_crc32(void);
static int init_dt(void);
static double adjust_for_tidacc(double ans, double Y, double tid_acc, double tid_acc0, AS_BOOL adjust_after_1955);
static double deltat_espenak_meeus_1620(double tjd, double tid_acc);
static double deltat_stephenson_etc_2016(double tjd, double tid_acc);
static double deltat_longterm_morrison_stephenson(double tjd);
static double deltat_stephenson_morrison_2004_1600(double tjd, double tid_acc);
static double deltat_stephenson_morrison_1997_1600(double tjd, double tid_acc);
static double deltat_aa(double tjd, double tid_acc);

#define SEFLG_EPHMASK   (SEFLG_JPLEPH|SEFLG_SWIEPH|SEFLG_MOSEPH)

/* Reduce x modulo 360 degrees
 */
double CALL_CONV swe_degnorm(double x)
{
  double y;
  y = fmod(x, 360.0);
  if (fabs(y) < 1e-13) y = 0;	/* Alois fix 11-dec-1999 */
  if( y < 0.0 ) y += 360.0;
  return(y);
}

/* Reduce x modulo TWOPI degrees
 */
double CALL_CONV swe_radnorm(double x)
{
  double y;
  y = fmod(x, TWOPI);
  if (fabs(y) < 1e-13) y = 0;	/* Alois fix 11-dec-1999 */
  if( y < 0.0 ) y += TWOPI;
  return(y);
}

double CALL_CONV swe_deg_midp(double x1, double x0)
{
  double d, y;
  d = swe_difdeg2n(x1, x0);	/* arc from x0 to x1 */
  y = swe_degnorm(x0 + d / 2);
  return(y);
}

double CALL_CONV swe_rad_midp(double x1, double x0)
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
void CALL_CONV swe_cotrans(double *xpo, double *xpn, double eps)
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
void CALL_CONV swe_cotrans_sp(double *xpo, double *xpn, double eps)
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
  if (rxy == 0) {
    if (x[2] >= 0)
      ll[1] = PI / 2;
    else
      ll[1] = -(PI / 2);
  } else {
    ll[1] = atan(x[2] / rxy);
  }
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

/* functions for precession and ecliptic obliquity according to Vondrák et alii, 2011 */
#define AS2R (DEGTORAD / 3600.0)
#define D2PI TWOPI
#define EPS0 (84381.406 * AS2R)
#define NPOL_PEPS 4
#define NPER_PEPS 10
#define NPOL_PECL 4
#define NPER_PECL 8
#define NPOL_PEQU 4
#define NPER_PEQU 14

/* for pre_peps(): */
/* polynomials */
static const double pepol[NPOL_PEPS][2] = {
  {+8134.017132, +84028.206305},
  {+5043.0520035, +0.3624445},
  {-0.00710733, -0.00004039},
  {+0.000000271, -0.000000110}
};

/* periodics */
static const double peper[5][NPER_PEPS] = {
  {+409.90, +396.15, +537.22, +402.90, +417.15, +288.92, +4043.00, +306.00, +277.00, +203.00},
  {-6908.287473, -3198.706291, +1453.674527, -857.748557, +1173.231614, -156.981465, +371.836550, -216.619040, +193.691479, +11.891524},
  {+753.872780, -247.805823, +379.471484, -53.880558, -90.109153, -353.600190, -63.115353, -28.248187, +17.703387, +38.911307},
  {-2845.175469, +449.844989, -1255.915323, +886.736783, +418.887514, +997.912441, -240.979710, +76.541307, -36.788069, -170.964086},
  {-1704.720302, -862.308358, +447.832178, -889.571909, +190.402846, -56.564991, -296.222622, -75.859952, +67.473503, +3.014055}
};

/* for pre_pecl(): */
/* polynomials */
static const double pqpol[NPOL_PECL][2] = {
  {+5851.607687, -1600.886300},
  {-0.1189000, +1.1689818},
  {-0.00028913, -0.00000020},
  {+0.000000101, -0.000000437}
};

/* periodics */
static const double pqper[5][NPER_PECL] = {
  {708.15, 2309, 1620, 492.2, 1183, 622, 882, 547},
  {-5486.751211, -17.127623, -617.517403, 413.44294, 78.614193, -180.732815, -87.676083, 46.140315},
  // original publication    A&A 534, A22 (2011):
//{-684.66156, 2446.28388, 399.671049, -356.652376, -186.387003, -316.80007, 198.296071, 101.135679}, 
  // typo fixed according to A&A 541, C1 (2012)
  {-684.66156, 2446.28388, 399.671049, -356.652376, -186.387003, -316.80007, 198.296701, 101.135679}, 
  {667.66673, -2354.886252, -428.152441, 376.202861, 184.778874, 335.321713, -185.138669, -120.97283},
  {-5523.863691, -549.74745, -310.998056, 421.535876, -36.776172, -145.278396, -34.74445, 22.885731}
};

/* for pre_pequ(): */
/* polynomials */
static const double xypol[NPOL_PEQU][2] = {
  {+5453.282155, -73750.930350},
  {+0.4252841, -0.7675452},
  {-0.00037173, -0.00018725},
  {-0.000000152, +0.000000231}
};

/* periodics */
static const double xyper[5][NPER_PEQU] = {
  {256.75, 708.15, 274.2, 241.45, 2309, 492.2, 396.1, 288.9, 231.1, 1610, 620, 157.87, 220.3, 1200},
  {-819.940624, -8444.676815, 2600.009459, 2755.17563, -167.659835, 871.855056, 44.769698, -512.313065, -819.415595, -538.071099, -189.793622, -402.922932, 179.516345, -9.814756},
  {75004.344875, 624.033993, 1251.136893, -1102.212834, -2660.66498, 699.291817, 153.16722, -950.865637, 499.754645, -145.18821, 558.116553, -23.923029, -165.405086, 9.344131},
  {81491.287984, 787.163481, 1251.296102, -1257.950837, -2966.79973, 639.744522, 131.600209, -445.040117, 584.522874, -89.756563, 524.42963, -13.549067, -210.157124, -44.919798},
  {1558.515853, 7774.939698, -2219.534038, -2523.969396, 247.850422, -846.485643, -1393.124055, 368.526116, 749.045012, 444.704518, 235.934465, 374.049623, -171.33018, -22.899655}
};

void swi_ldp_peps(double tjd, double *dpre, double *deps)
{
  int i;
  int npol = NPOL_PEPS;
  int nper = NPER_PEPS;
  double t, p, q, w, a, s, c;
  t = (tjd - J2000) / 36525.0;
  p = 0;
  q = 0;
  /* periodic terms */
  for (i = 0; i < nper; i++) {
    w = D2PI * t;
    a = w / peper[0][i];
    s = sin(a);
    c = cos(a);
    p += c * peper[1][i] + s * peper[3][i];
    q += c * peper[2][i] + s * peper[4][i];
  }
  /* polynomial terms */
  w = 1;
  for (i = 0; i < npol; i++) {
    p += pepol[i][0] * w;
    q += pepol[i][1] * w;
    w *= t;
  }
  /* both to radians */
  p *= AS2R;
  q *= AS2R;
  /* return */
  if (dpre != NULL)
    *dpre = p;
  if (deps != NULL)
    *deps = q;
//fprintf(stderr, "%.17f\n", *deps / DEGTORAD);
}

/*
 * Long term high precision precession, 
 * according to Vondrak/Capitaine/Wallace, "New precession expressions, valid
 * for long time intervals", in A&A 534, A22(2011).
 */
/* precession of the ecliptic */
static void pre_pecl(double tjd, double *vec) 
{
  int i;
  int npol = NPOL_PECL;
  int nper = NPER_PECL;
  double t, p, q, w, a, s, c, z;
  t = (tjd - J2000) / 36525.0;
  p = 0;
  q = 0;
  /* periodic terms */
  for (i = 0; i < nper; i++) {
    w = D2PI * t;
    a = w / pqper[0][i];
    s = sin(a);
    c = cos(a);
    p += c * pqper[1][i] + s * pqper[3][i];
    q += c * pqper[2][i] + s * pqper[4][i];
  }
  /* polynomial terms */
  w = 1;
  for (i = 0; i < npol; i++) {
    p += pqpol[i][0] * w;
    q += pqpol[i][1] * w;
    w *= t;
  }
  /* both to radians */
  p *= AS2R;
  q *= AS2R;
  /* ecliptic pole vector */
  z = 1 - p * p - q * q;
  if (z < 0)
    z = 0;
  else 
    z = sqrt(z);
  s = sin(EPS0);
  c = cos(EPS0);
  vec[0] = p;
  vec[1] = - q * c - z * s;
  vec[2] = - q * s + z * c;
}

/* precession of the equator */
static void pre_pequ(double tjd, double *veq) 
{
  int i;
  int npol = NPOL_PEQU;
  int nper = NPER_PEQU;
  double t, x, y, w, a, s, c;
  t = (tjd - J2000) / 36525.0;
  x = 0;
  y = 0;
  for (i = 0; i < nper; i++) {
    w = D2PI * t;
    a = w / xyper[0][i];
    s = sin(a);
    c = cos(a);
    x += c * xyper[1][i] + s * xyper[3][i];
    y += c * xyper[2][i] + s * xyper[4][i];
  }
  /* polynomial terms */
  w = 1;
  for (i = 0; i < npol; i++) {
    x += xypol[i][0] * w;
    y += xypol[i][1] * w;
    w *= t;
  }
  x *= AS2R;
  y *= AS2R;
  /* equator pole vector */
  veq[0] = x;
  veq[1] = y;
  w = x * x + y * y;
  if (w < 1)
    veq[2] = sqrt(1 - w);
  else
    veq[2] = 0;
}

#if 0
static void swi_cross_prod(double *a, double *b, double *x)
{
  x[0] = a[1] * b[2] - a[2] * b[1];
  x[1] = a[2] * b[0] - a[0] * b[2];
  x[2] = a[0] * b[1] - a[1] * b[0];
}
#endif

/* precession matrix */
static void pre_pmat(double tjd, double *rp)
{
  double peqr[3], pecl[3], v[3], w, eqx[3];
//tjd = 1219339.078000;
  /*equator pole */
  pre_pequ(tjd, peqr);
  /* ecliptic pole */
  pre_pecl(tjd, pecl);
//  fprintf(stderr, "%.17f %.17f %.17f\n", peqr[0], peqr[1], peqr[2]);
//  fprintf(stderr, "%.17f %.17f %.17f\n", pecl[0], pecl[1], pecl[2]);
  /* equinox */
  swi_cross_prod(peqr, pecl, v);
  w = sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
  eqx[0] = v[0] / w;
  eqx[1] = v[1] / w;
  eqx[2] = v[2] / w;
  swi_cross_prod(peqr, eqx, v);
  rp[0] = eqx[0];
  rp[1] = eqx[1];
  rp[2] = eqx[2];
  rp[3] = v[0];
  rp[4] = v[1];
  rp[5] = v[2];
  rp[6] = peqr[0];
  rp[7] = peqr[1];
  rp[8] = peqr[2];
//  int i;
//  for (i = 0; i < 3; i++) {
//    fprintf(stderr, "(%.17f   %.17f   %.17f)\n", rp[i*3], rp[i*3+1],rp[i*3+2]);
//  } /**/
}

/* precession according to Owen 1990:
 * Owen, William M., Jr., (JPL) "A Theory of the Earth's Precession
 * Relative to the Invariable Plane of the Solar System", Ph.D.
 * Dissertation, University of Florida, 1990.
 * Implemented for time range -18000 to 14000. 
 */
/* 
 * p. 177: central time Tc = -160, covering time span -200 <= T <= -120
 * i.e. -14000 +- 40 centuries
 * p. 178: central time Tc = -80, covering time span -120 <= T <= -40 
 * i.e. -6000 +- 40 centuries 
 * p. 179: central time Tc = 0, covering time span -40 <= T <= +40 
 * i.e. 2000 +- 40 centuries 
 * p. 180: central time Tc = 80, covering time span 40 <= T <= 120 
 * i.e. 10000 +- 40 centuries 
 * p. 181: central time Tc = 160, covering time span 120 <= T <= 200
 * i.e. 10000 +- 40 centuries 
 */
static const double owen_eps0_coef[5][10] = {
{23.699391439256386, 5.2330816033981775e-1, -5.6259493384864815e-2, -8.2033318431602032e-3, 6.6774163554156385e-4, 2.4931584012812606e-5, -3.1313623302407878e-6, 2.0343814827951515e-7, 2.9182026615852936e-8, -4.1118760893281951e-9,},
{24.124759551704588, -1.2094875596566286e-1, -8.3914869653015218e-2, 3.5357075322387405e-3, 6.4557467824807032e-4, -2.5092064378707704e-5, -1.7631607274450848e-6, 1.3363622791424094e-7, 1.5577817511054047e-8, -2.4613907093017122e-9,},
{23.439103144206208, -4.9386077073143590e-1, -2.3965445283267805e-4, 8.6637485629656489e-3, -5.2828151901367600e-5, -4.3951004595359217e-5, -1.1058785949914705e-6, 6.2431490022621172e-8, 3.4725376218710764e-8, 1.3658853127005757e-9,},
{22.724671295125046, -1.6041813558650337e-1, 7.0646783888132504e-2, 1.4967806745062837e-3, -6.6857270989190734e-4, 5.7578378071604775e-6, 3.3738508454638728e-6, -2.2917813537654764e-7, -2.1019907929218137e-8, 4.3139832091694682e-9,},
{22.914636050333696, 3.2123508304962416e-1, 3.6633220173792710e-2, -5.9228324767696043e-3, -1.882379107379328e-4, 3.2274552870236244e-5, 4.9052463646336507e-7, -5.9064298731578425e-8, -2.0485712675098837e-8, -6.2163304813908160e-10,},
};

static const double owen_psia_coef[5][10] = {
{-218.57864954903122, 51.752257487741612, 1.3304715765661958e-1, 9.2048123521890745e-2, -6.0877528127241278e-3, -7.0013893644531700e-5, -4.9217728385458495e-5, -1.8578234189053723e-6, 7.4396426162029877e-7, -5.9157528981843864e-9,},
{-111.94350527506128, 55.175558131675861, 4.7366115762797613e-1, -4.7701750975398538e-2, -9.2445765329325809e-3, 7.0962838707454917e-4, 1.5140455277814658e-4, -7.7813159018954928e-7, -2.4729402281953378e-6, -1.0898887008726418e-7,},
{-2.041452011529441e-1, 55.969995858494106, -1.9295093699770936e-1, -5.6819574830421158e-3, 1.1073687302518981e-2, -9.0868489896815619e-5, -1.1999773777895820e-4, 9.9748697306154409e-6, 5.7911493603430550e-7, -2.3647526839778175e-7,},
{111.61366860604471, 56.404525305162447, 4.4403302410703782e-1, 7.1490030578883907e-2, -4.9184559079790816e-3, -1.3912698949042046e-3, -6.8490613661884005e-5, 1.2394328562905297e-6, 1.7719847841480384e-6, 2.4889095220628068e-7,},
{228.40683531269390, 60.056143904919826, 2.9583200718478960e-2, -1.5710838319490748e-1, -7.0017356811600801e-3, 3.3009615142224537e-3, 2.0318123852537664e-4, -6.5840216067828310e-5, -5.9077673352976155e-6, 1.3983942185303064e-6,},
};

static const double owen_oma_coef[5][10] = {
{25.541291140949806, 2.377889511272162e-1, -3.7337334723142133e-1, 2.4579295485161534e-2, 4.3840999514263623e-3, -3.1126873333599556e-4, -9.8443045771748915e-6, -7.9403103080496923e-7, 1.0840116743893556e-9, 9.2865105216887919e-9,},
{24.429357654237926, -9.5205745947740161e-1, 8.6738296270534816e-2, 3.0061543426062955e-2, -4.1532480523019988e-3, -3.7920928393860939e-4, 3.5117012399609737e-5, 4.6811877283079217e-6, -8.1836046585546861e-8, -6.1803706664211173e-8,},
{23.450465062489337, -9.7259278279739817e-2, 1.1082286925130981e-2, -3.1469883339372219e-2, -1.0041906996819648e-4, 5.6455168475133958e-4, -8.4403910211030209e-6, -3.8269157371098435e-6, 3.1422585261198437e-7, 9.3481729116773404e-9,},
{22.581778052947806, -8.7069701538602037e-1, -9.8140710050197307e-2, 2.6025931340678079e-2, 4.8165322168786755e-3, -1.906558772193363e-4, -4.6838759635421777e-5, -1.6608525315998471e-6, -3.2347811293516124e-8, 2.8104728109642000e-9,},
{21.518861835737142, 2.0494789509441385e-1, 3.5193604846503161e-1, 1.5305977982348925e-2, -7.5015367726336455e-3, -4.0322553186065610e-4, 1.0655320434844041e-4, 7.1792339586935752e-6, -1.603874697543020e-6, -1.613563462813512e-7,},
};

static const double owen_chia_coef[5][10] = {
{8.2378850337329404e-1, -3.7443109739678667, 4.0143936898854026e-1, 8.1822830214590811e-2, -8.5978790792656293e-3, -2.8350488448426132e-5, -4.2474671728156727e-5, -1.6214840884656678e-6, 7.8560442001953050e-7, -1.032016641696707e-8,},
{-2.1726062070318606, 7.8470515033132925e-1, 4.4044931004195718e-1, -8.0671247169971653e-2, -8.9672662444325007e-3, 9.2248978383109719e-4, 1.5143472266372874e-4, -1.6387009056475679e-6, -2.4405558979328144e-6, -1.0148113464009015e-7,},
{-4.8518673570735556e-1, 1.0016737299946743e-1, -4.7074888613099918e-1, -5.8604054305076092e-3, 1.4300208240553435e-2, -6.7127991650300028e-5, -1.3703764889645475e-4, 9.0505213684444634e-6, 6.0368690647808607e-7, -2.2135404747652171e-7,},
{-2.0950740076326087, -9.4447359463206877e-1, 4.0940512860493755e-1, 1.0261699700263508e-1, -5.3133241571955160e-3, -1.6634631550720911e-3, -5.9477519536647907e-5, 2.9651387319208926e-6, 1.6434499452070584e-6, 2.3720647656961084e-7,},
{6.3315163285678715e-1, 3.5241082918420464, 2.1223076605364606e-1, -1.5648122502767368e-1, -9.1964075390801980e-3, 3.3896161239812411e-3, 2.1485178626085787e-4, -6.6261759864793735e-5, -5.9257969712852667e-6, 1.3918759086160525e-6,},
};

static void get_owen_t0_icof(double tjd, double *t0, int *icof) 
{
  int i, j = 0;
  double t0s[5] = {-3392455.5, -470455.5, 2451544.5, 5373544.5, 8295544.5, };
  *t0 = t0s[0];
  for (i = 1; i < 5; i++) {
    if (tjd < (t0s[i-1] + t0s[i]) / 2) {
      ;
    } else {
      *t0 = t0s[i];
      j++;
    }
  }
  *icof = j;
}

/* precession matrix Owen 1990 */
static void owen_pre_matrix(double tjd, double *rp, int iflag)
{
  int i, icof = 0;
  double eps0 = 0, chia = 0, psia = 0, oma = 0;
  double coseps0, sineps0, coschia, sinchia, cospsia, sinpsia, cosoma, sinoma;
  double k[10], tau[10];
  double t0;
  get_owen_t0_icof(tjd, &t0, &icof);
//  fprintf(stderr, "%d, %.17f\n", icof, t0);
  tau[0] = 0;
  tau[1] = (tjd - t0) / 36525.0 / 40.0;
  for (i = 2; i <= 9; i++) {
    tau[i] = tau[1] * tau[i-1];
  }
  k[0] = 1;
  k[1] = tau[1];
  k[2] = 2 * tau[2] - 1;
  k[3] = 4 * tau[3] - 3 * tau[1];
  k[4] = 8 * tau[4] - 8 * tau[2] + 1;
  k[5] = 16 * tau[5] - 20 * tau[3] + 5 * tau[1];
  k[6] = 32 * tau[6] - 48 * tau[4] + 18 * tau[2] - 1;
  k[7] = 64 * tau[7] - 112 * tau[5] + 56 * tau[3] - 7 * tau[1];
  k[8] = 128 * tau[8] - 256 * tau[6] + 160 * tau[4] - 32 * tau[2] + 1;
  k[9] = 256 * tau[9] - 576 * tau[7] + 432 * tau[5] - 120 * tau[3] + 9 * tau[1];
  for (i = 0; i < 10; i++) {
    //eps += (k[i] * owen_eps0_coef[icof][i]);
    psia += (k[i] * owen_psia_coef[icof][i]);
    oma  += (k[i] * owen_oma_coef[icof][i]);
    chia += (k[i] * owen_chia_coef[icof][i]);
  }
  if (iflag & (SEFLG_JPLHOR || SEFLG_JPLHOR_APPROX)) {
    /* 
     * In comparison with JPL Horizons we have an almost constant offset
     * almost constant offset in ecl. longitude of about -0.000019 deg. 
     * We fix this as follows: */
    psia += -0.000018560;
  }
  eps0 = 84381.448 / 3600.0;
//fprintf(stderr, "e=%.17f, ps=%.17f, om=%.17f, ch=%.17f\n", eps0, psia, oma, chia);
  eps0 *= DEGTORAD;
  psia *= DEGTORAD;
  chia *= DEGTORAD;
  oma *= DEGTORAD;
  coseps0 = cos(eps0);
  sineps0 = sin(eps0);
  coschia = cos(chia);
  sinchia = sin(chia);
  cospsia = cos(psia);
  sinpsia = sin(psia);
  cosoma = cos(oma);
  sinoma = sin(oma);
  rp[0] = coschia * cospsia + sinchia * cosoma * sinpsia;
  rp[1] = (-coschia * sinpsia + sinchia * cosoma * cospsia) * coseps0 + sinchia * sinoma * sineps0;
  rp[2] = (-coschia * sinpsia + sinchia * cosoma * cospsia) * sineps0 - sinchia * sinoma * coseps0;
  rp[3] = -sinchia * cospsia + coschia * cosoma * sinpsia;
  rp[4] = (sinchia * sinpsia + coschia * cosoma * cospsia) * coseps0 + coschia * sinoma * sineps0;
  rp[5] = (sinchia * sinpsia + coschia * cosoma * cospsia) * sineps0 - coschia * sinoma * coseps0;
  rp[6] = sinoma * sinpsia;
  rp[7] = sinoma * cospsia * coseps0 - cosoma * sineps0;
  rp[8] = sinoma * cospsia * sineps0 + cosoma * coseps0;
  /*for (i = 0; i < 3; i++) {
    fprintf(stderr, "(%.17f   %.17f   %.17f)\n", rp[i*3], rp[i*3+1],rp[i*3+2]);
  } */
}
static void epsiln_owen_1986(double tjd, double *eps)
{
  int i, icof = 0;
  double k[10], tau[10];
  double t0;
  get_owen_t0_icof(tjd, &t0, &icof);
  *eps = 0;
  tau[0] = 0;
  tau[1] = (tjd - t0) / 36525.0 / 40.0;
  for (i = 2; i <= 9; i++) {
    tau[i] = tau[1] * tau[i-1];
  }
  k[0] = 1;
  k[1] = tau[1];
  k[2] = 2 * tau[2] - 1;
  k[3] = 4 * tau[3] - 3 * tau[1];
  k[4] = 8 * tau[4] - 8 * tau[2] + 1;
  k[5] = 16 * tau[5] - 20 * tau[3] + 5 * tau[1];
  k[6] = 32 * tau[6] - 48 * tau[4] + 18 * tau[2] - 1;
  k[7] = 64 * tau[7] - 112 * tau[5] + 56 * tau[3] - 7 * tau[1];
  k[8] = 128 * tau[8] - 256 * tau[6] + 160 * tau[4] - 32 * tau[2] + 1;
  k[9] = 256 * tau[9] - 576 * tau[7] + 432 * tau[5] - 120 * tau[3] + 9 * tau[1];
  for (i = 0; i < 10; i++) {
    *eps += (k[i] * owen_eps0_coef[icof][i]);
  }
//fprintf(stderr, "eps=%.17f\n", *eps);
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
#define OFFSET_EPS_JPLHORIZONS (35.95) 
#define DCOR_EPS_JPL_TJD0  2437846.5
#define NDCOR_EPS_JPL  51
double dcor_eps_jpl[] = {
36.726, 36.627, 36.595, 36.578, 36.640, 36.659, 36.731, 36.765,
36.662, 36.555, 36.335, 36.321, 36.354, 36.227, 36.289, 36.348, 36.257, 36.163,
35.979, 35.896, 35.842, 35.825, 35.912, 35.950, 36.093, 36.191, 36.009, 35.943,
35.875, 35.771, 35.788, 35.753, 35.822, 35.866, 35.771, 35.732, 35.543, 35.498,
35.449, 35.409, 35.497, 35.556, 35.672, 35.760, 35.596, 35.565, 35.510, 35.394,
35.385, 35.375, 35.415,
};
double swi_epsiln(double J, int32 iflag) 
{
  double T, eps;
  double tofs, dofs, t0, t1;
  int prec_model = swed.astro_models[SE_MODEL_PREC_LONGTERM];
  int prec_model_short = swed.astro_models[SE_MODEL_PREC_SHORTTERM];
  int jplhora_model = swed.astro_models[SE_MODEL_JPLHORA_MODE];
  AS_BOOL is_jplhor = FALSE;
  if (prec_model == 0) prec_model = SEMOD_PREC_DEFAULT;
  if (prec_model_short == 0) prec_model_short = SEMOD_PREC_DEFAULT_SHORT;
  if (jplhora_model == 0) jplhora_model = SEMOD_JPLHORA_DEFAULT;
  if (iflag & SEFLG_JPLHOR)
    is_jplhor = TRUE;
  if ((iflag & SEFLG_JPLHOR_APPROX) 
      && jplhora_model == SEMOD_JPLHORA_3
      && J <= HORIZONS_TJD0_DPSI_DEPS_IAU1980)
    is_jplhor = TRUE;
  T = (J - 2451545.0)/36525.0;
  if (is_jplhor) {
    if (J > 2378131.5 && J < 2525323.5) { // between 1.1.1799 and 1.1.2202
      eps = (((1.813e-3*T-5.9e-4)*T-46.8150)*T+84381.448)*DEGTORAD/3600;
    } else {
      epsiln_owen_1986(J, &eps);
      eps *= DEGTORAD;
    }
  } else if ((iflag & SEFLG_JPLHOR_APPROX) && jplhora_model == SEMOD_JPLHORA_2) {
    eps = (((1.813e-3*T-5.9e-4)*T-46.8150)*T+84381.448)*DEGTORAD/3600;
  } else if (prec_model_short == SEMOD_PREC_IAU_1976 && fabs(T) <= PREC_IAU_1976_CTIES ) {
    eps = (((1.813e-3*T-5.9e-4)*T-46.8150)*T+84381.448)*DEGTORAD/3600;
  } else if (prec_model == SEMOD_PREC_IAU_1976) {
    eps = (((1.813e-3*T-5.9e-4)*T-46.8150)*T+84381.448)*DEGTORAD/3600;
  } else if (prec_model_short == SEMOD_PREC_IAU_2000 && fabs(T) <= PREC_IAU_2000_CTIES ) {
    eps = (((1.813e-3*T-5.9e-4)*T-46.84024)*T+84381.406)*DEGTORAD/3600;
  } else if (prec_model == SEMOD_PREC_IAU_2000) {
    eps = (((1.813e-3*T-5.9e-4)*T-46.84024)*T+84381.406)*DEGTORAD/3600;
  } else if (prec_model_short == SEMOD_PREC_IAU_2006 && fabs(T) <= PREC_IAU_2006_CTIES) {
    eps =  (((((-4.34e-8 * T -5.76e-7) * T +2.0034e-3) * T -1.831e-4) * T -46.836769) * T + 84381.406) * DEGTORAD / 3600.0; 
  } else if (prec_model == SEMOD_PREC_IAU_2006) {
    eps =  (((((-4.34e-8 * T -5.76e-7) * T +2.0034e-3) * T -1.831e-4) * T -46.836769) * T + 84381.406) * DEGTORAD / 3600.0; 
  } else if (prec_model == SEMOD_PREC_BRETAGNON_2003) {
    eps =  ((((((-3e-11 * T - 2.48e-8) * T -5.23e-7) * T +1.99911e-3) * T -1.667e-4) * T -46.836051) * T + 84381.40880) * DEGTORAD / 3600.0;/* */
  } else if (prec_model == SEMOD_PREC_SIMON_1994) {
    eps =  (((((2.5e-8 * T -5.1e-7) * T +1.9989e-3) * T -1.52e-4) * T -46.80927) * T + 84381.412) * DEGTORAD / 3600.0;/* */
  } else if (prec_model == SEMOD_PREC_WILLIAMS_1994) {
    eps =  ((((-1.0e-6 * T +2.0e-3) * T -1.74e-4) * T -46.833960) * T + 84381.409) * DEGTORAD / 3600.0;/* */
  } else if (prec_model == SEMOD_PREC_LASKAR_1986 || prec_model == SEMOD_PREC_WILL_EPS_LASK) {
    T /= 10.0;
    eps = ((((((((( 2.45e-10*T + 5.79e-9)*T + 2.787e-7)*T
    + 7.12e-7)*T - 3.905e-5)*T - 2.4967e-3)*T
    - 5.138e-3)*T + 1.99925)*T - 0.0155)*T - 468.093)*T
    + 84381.448;
    eps *= DEGTORAD/3600.0;
  } else if (prec_model == SEMOD_PREC_OWEN_1990) {
    epsiln_owen_1986(J, &eps);
    eps *= DEGTORAD;
//fprintf(stderr, "epso=%.17f\n", eps);
  } else { /* SEMOD_PREC_VONDRAK_2011 */
    swi_ldp_peps(J, NULL, &eps);
    if ((iflag & SEFLG_JPLHOR_APPROX) && jplhora_model != SEMOD_JPLHORA_2) {
      tofs = (J - DCOR_EPS_JPL_TJD0) / 365.25;
      dofs = OFFSET_EPS_JPLHORIZONS;
      if (tofs < 0) {
	tofs = 0;
	dofs = dcor_eps_jpl[0];
      } else if (tofs >= NDCOR_EPS_JPL - 1) {
	tofs = NDCOR_EPS_JPL;
	dofs = dcor_eps_jpl[NDCOR_EPS_JPL - 1];
      } else {
	t0 = (int) tofs;
	t1 = t0 + 1;
	dofs = dcor_eps_jpl[(int)t0];
	dofs = (tofs - t0) * (dcor_eps_jpl[(int)t0] - dcor_eps_jpl[(int)t1]) + dcor_eps_jpl[(int)t0];
      }
      dofs /= (1000.0 * 3600.0);
      eps += dofs * DEGTORAD;
    }
//fprintf(stderr, "epsv=%.17f\n", eps);
  }
  return(eps);
}

/* Precession of the equinox and ecliptic
 * from epoch Julian date J to or from J2000.0
 *
 * Original program by Steve Moshier.
 * Changes in program structure and implementation of IAU 2003 (P03) and
 * Vondrak 2011 by Dieter Koch.
 *
 * SEMOD_PREC_VONDRAK_2011 
 * J. Vondrák, N. Capitaine, and P. Wallace, "New precession expressions,
 * valid for long time intervals", A&A 534, A22 (2011)
 *
 * SEMOD_PREC_IAU_2006 
 * N. Capitaine, P.T. Wallace, and J. Chapront, "Expressions for IAU 2000
 * precession quantities", 2003, A&A 412, 567-586 (2003).
 * This is a "short" term model, that can be combined with other models
 *
 * SEMOD_PREC_WILLIAMS_1994 
 * James G. Williams, "Contributions to the Earth's obliquity rate,
 * precession, and nutation,"  Astron. J. 108, 711-724 (1994).
 *
 * SEMOD_PREC_SIMON_1994 
 * J. L. Simon, P. Bretagnon, J. Chapront, M. Chapront-Touze', G. Francou,
 * and J. Laskar, "Numerical Expressions for precession formulae and
 * mean elements for the Moon and the planets," Astronomy and Astrophysics
 * 282, 663-683 (1994).  
 *
 * SEMOD_PREC_IAU_1976 
 * IAU Coefficients are from:
 * J. H. Lieske, T. Lederle, W. Fricke, and B. Morando,
 * "Expressions for the Precession Quantities Based upon the IAU
 * (1976) System of Astronomical Constants,"  Astronomy and
 * Astrophysics 58, 1-16 (1977).
 * This is a "short" term model, that can be combined with other models
 *
 * SEMOD_PREC_LASKAR_1986 
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
 * Bretagnon and Francou's expansions for the node and inclination
 * of the ecliptic were derived from Laskar's data but were truncated
 * after the term in T**6. I have recomputed these expansions from
 * Laskar's data, retaining powers up to T**10 in the result.
 *
 */

static int precess_1(double *R, double J, int direction, int prec_method)
{
  double T, Z = 0, z = 0, TH = 0;
  int i;
  double x[3];
  double sinth, costh, sinZ, cosZ, sinz, cosz, A, B;
  if( J == J2000 ) 
    return(0);
  T = (J - J2000)/36525.0;
  if (prec_method == SEMOD_PREC_IAU_1976) {
    Z =  (( 0.017998*T + 0.30188)*T + 2306.2181)*T*DEGTORAD/3600;
    z =  (( 0.018203*T + 1.09468)*T + 2306.2181)*T*DEGTORAD/3600;
    TH = ((-0.041833*T - 0.42665)*T + 2004.3109)*T*DEGTORAD/3600;
  } else if (prec_method == SEMOD_PREC_IAU_2000) {
    /* AA 2006 B28:*/
    Z =  (((((- 0.0000002*T - 0.0000327)*T + 0.0179663)*T + 0.3019015)*T + 2306.0809506)*T + 2.5976176)*DEGTORAD/3600;
    z =  (((((- 0.0000003*T - 0.000047)*T + 0.0182237)*T + 1.0947790)*T + 2306.0803226)*T - 2.5976176)*DEGTORAD/3600;
    TH = ((((-0.0000001*T - 0.0000601)*T - 0.0418251)*T - 0.4269353)*T + 2004.1917476)*T*DEGTORAD/3600;
  } else if (prec_method == SEMOD_PREC_IAU_2006) {
    T = (J - J2000)/36525.0;
    Z =  (((((- 0.0000003173*T - 0.000005971)*T + 0.01801828)*T + 0.2988499)*T + 2306.083227)*T + 2.650545)*DEGTORAD/3600;
    z =  (((((- 0.0000002904*T - 0.000028596)*T + 0.01826837)*T + 1.0927348)*T + 2306.077181)*T - 2.650545)*DEGTORAD/3600;
    TH = ((((-0.00000011274*T - 0.000007089)*T - 0.04182264)*T - 0.4294934)*T + 2004.191903)*T*DEGTORAD/3600;
  } else if (prec_method == SEMOD_PREC_BRETAGNON_2003) {
    Z =  ((((((-0.00000000013*T - 0.0000003040)*T - 0.000005708)*T + 0.01801752)*T + 0.3023262)*T + 2306.080472)*T + 2.72767)*DEGTORAD/3600;
    z =  ((((((-0.00000000005*T - 0.0000002486)*T - 0.000028276)*T + 0.01826676)*T + 1.0956768)*T + 2306.076070)*T - 2.72767)*DEGTORAD/3600;
    TH = ((((((0.000000000009*T + 0.00000000036)*T -0.0000001127)*T - 0.000007291)*T - 0.04182364)*T - 0.4266980)*T + 2004.190936)*T*DEGTORAD/3600;
  } else {
    return 0;
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
  } else { /* From J to J2000.0 */
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
  for( i=0; i<3; i++ ) 
    R[i] = x[i];
  return(0);
}

/* In WILLIAMS and SIMON, Laskar's terms of order higher than t^4
   have been retained, because Simon et al mention that the solution
   is the same except for the lower order terms.  */

/* SEMOD_PREC_WILLIAMS_1994 */
static const double pAcof_williams[] = {
 -8.66e-10, -4.759e-8, 2.424e-7, 1.3095e-5, 1.7451e-4, -1.8055e-3,
 -0.235316, 0.076, 110.5407, 50287.70000 };
static const double nodecof_williams[] = {
  6.6402e-16, -2.69151e-15, -1.547021e-12, 7.521313e-12, 1.9e-10, 
  -3.54e-9, -1.8103e-7,  1.26e-7,  7.436169e-5,
  -0.04207794833,  3.052115282424};
static const double inclcof_williams[] = {
  1.2147e-16, 7.3759e-17, -8.26287e-14, 2.503410e-13, 2.4650839e-11, 
  -5.4000441e-11, 1.32115526e-9, -6.012e-7, -1.62442e-5,
  0.00227850649, 0.0 };

/* SEMOD_PREC_SIMON_1994 */
/* Precession coefficients from Simon et al: */
static const double pAcof_simon[] = {
  -8.66e-10, -4.759e-8, 2.424e-7, 1.3095e-5, 1.7451e-4, -1.8055e-3,
  -0.235316, 0.07732, 111.2022, 50288.200 };
static const double nodecof_simon[] = {
  6.6402e-16, -2.69151e-15, -1.547021e-12, 7.521313e-12, 1.9e-10, 
  -3.54e-9, -1.8103e-7, 2.579e-8, 7.4379679e-5,
  -0.0420782900, 3.0521126906};
static const double inclcof_simon[] = {
  1.2147e-16, 7.3759e-17, -8.26287e-14, 2.503410e-13, 2.4650839e-11, 
  -5.4000441e-11, 1.32115526e-9, -5.99908e-7, -1.624383e-5,
  0.002278492868, 0.0 };

/* SEMOD_PREC_LASKAR_1986 */
/* Precession coefficients taken from Laskar's paper: */
static const double pAcof_laskar[] = {
  -8.66e-10, -4.759e-8, 2.424e-7, 1.3095e-5, 1.7451e-4, -1.8055e-3,
  -0.235316, 0.07732, 111.1971, 50290.966 };
/* Node and inclination of the earth's orbit computed from
 * Laskar's data as done in Bretagnon and Francou's paper.
 * Units are radians.
 */
static const double nodecof_laskar[] = {
  6.6402e-16, -2.69151e-15, -1.547021e-12, 7.521313e-12, 6.3190131e-10, 
  -3.48388152e-9, -1.813065896e-7, 2.75036225e-8, 7.4394531426e-5,
  -0.042078604317, 3.052112654975 };
static const double inclcof_laskar[] = {
  1.2147e-16, 7.3759e-17, -8.26287e-14, 2.503410e-13, 2.4650839e-11, 
  -5.4000441e-11, 1.32115526e-9, -5.998737027e-7, -1.6242797091e-5,
  0.002278495537, 0.0 };

static int precess_2(double *R, double J, int32 iflag, int direction, int prec_method)
{
  int i;
  double T, z;
  double eps, sineps, coseps;
  double x[3];
  const double *p;
  double A, B, pA, W;
  const double *pAcof, *inclcof, *nodecof;
  if( J == J2000 ) 
    return(0);
  if (prec_method == SEMOD_PREC_LASKAR_1986) {
    pAcof = pAcof_laskar;
    nodecof = nodecof_laskar;
    inclcof = inclcof_laskar;
  } else if (prec_method == SEMOD_PREC_SIMON_1994) {
    pAcof = pAcof_simon;
    nodecof = nodecof_simon;
    inclcof = inclcof_simon;
  } else if (prec_method == SEMOD_PREC_WILLIAMS_1994) {
    pAcof = pAcof_williams;
    nodecof = nodecof_williams;
    inclcof = inclcof_williams;
  } else {	/* default, to satisfy compiler */
    pAcof = pAcof_laskar;
    nodecof = nodecof_laskar;
    inclcof = inclcof_laskar;
  }
  T = (J - J2000)/36525.0;
  /* Implementation by elementary rotations using Laskar's expansions.
   * First rotate about the x axis from the initial equator
   * to the ecliptic. (The input is equatorial.)
   */
  if( direction == 1 ) 
    eps = swi_epsiln(J, iflag); /* To J2000 */
  else 
    eps = swi_epsiln(J2000, iflag); /* From J2000 */
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
  for( i=0; i<9; i++ ) {
    pA = pA * T + *p++;
  }
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
    eps = swi_epsiln(J2000, iflag);
  else 
    eps = swi_epsiln(J, iflag);
  sineps = sin(eps);
  coseps = cos(eps);
  z = coseps * x[1] - sineps * x[2];
  x[2] = sineps * x[1] + coseps * x[2];
  x[1] = z;
  for( i=0; i<3; i++ ) 
    R[i] = x[i];
  return(0);
}

static int precess_3(double *R, double J, int direction, int iflag, int prec_meth)
{
  //double T;
  double x[3], pmat[9];
  int i, j;
  if( J == J2000 ) 
    return(0);
  /* Each precession angle is specified by a polynomial in
   * T = Julian centuries from J2000.0.  See AA page B18.
   */
  //T = (J - J2000)/36525.0;
  if (prec_meth == SEMOD_PREC_OWEN_1990)
    owen_pre_matrix(J, pmat, iflag);
  else
    pre_pmat(J, pmat);
  if (direction == -1) {
    for (i = 0, j = 0; i <= 2; i++, j = i * 3) {
      x[i] = R[0] *  pmat[j + 0] +
	      R[1] * pmat[j + 1] +
	      R[2] * pmat[j + 2];
    }
  } else {
    for (i = 0, j = 0; i <= 2; i++, j = i * 3) {
      x[i] = R[0] * pmat[i + 0] +
	      R[1] * pmat[i + 3] +
	      R[2] * pmat[i + 6];
    }
  }
  for (i = 0; i < 3; i++)
    R[i] = x[i];
  return(0);
}

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
int swi_precess(double *R, double J, int32 iflag, int direction )
{
  double T = (J - J2000)/36525.0;
  int prec_model = swed.astro_models[SE_MODEL_PREC_LONGTERM];
  int prec_model_short = swed.astro_models[SE_MODEL_PREC_SHORTTERM];
  int jplhora_model = swed.astro_models[SE_MODEL_JPLHORA_MODE];
  AS_BOOL is_jplhor = FALSE;
  if (prec_model == 0) prec_model = SEMOD_PREC_DEFAULT;
  if (prec_model_short == 0) prec_model_short = SEMOD_PREC_DEFAULT_SHORT;
  if (jplhora_model == 0) jplhora_model = SEMOD_JPLHORA_DEFAULT;
  if (iflag & SEFLG_JPLHOR)
    is_jplhor = TRUE;
  if ((iflag & SEFLG_JPLHOR_APPROX) 
      && jplhora_model == SEMOD_JPLHORA_3
      && J <= HORIZONS_TJD0_DPSI_DEPS_IAU1980)
    is_jplhor = TRUE;
  /* JPL Horizons uses precession IAU 1976 and nutation IAU 1980 plus
   * some correction to nutation, arriving at extremely high precision */
  if (is_jplhor) {
    if (J > 2378131.5 && J < 2525323.5) { // between 1.1.1799 and 1.1.2202
      return precess_1(R, J, direction, SEMOD_PREC_IAU_1976);
    } else { 
      return precess_3(R, J, direction, iflag, SEMOD_PREC_OWEN_1990);
    }
  /* Use IAU 1976 formula for a few centuries.  */
  } else if (prec_model_short == SEMOD_PREC_IAU_1976 && fabs(T) <= PREC_IAU_1976_CTIES) {
    return precess_1(R, J, direction, SEMOD_PREC_IAU_1976);
  } else if (prec_model == SEMOD_PREC_IAU_1976) {
    return precess_1(R, J, direction, SEMOD_PREC_IAU_1976);
  /* Use IAU 2000 formula for a few centuries.  */
  } else if (prec_model_short == SEMOD_PREC_IAU_2000 && fabs(T) <= PREC_IAU_2000_CTIES) {
    return precess_1(R, J, direction, SEMOD_PREC_IAU_2000);
  } else if (prec_model == SEMOD_PREC_IAU_2000) {
    return precess_1(R, J, direction, SEMOD_PREC_IAU_2000);
  /* Use IAU 2006 formula for a few centuries.  */
  } else if (prec_model_short == SEMOD_PREC_IAU_2006 && fabs(T) <= PREC_IAU_2006_CTIES) {
    return precess_1(R, J, direction, SEMOD_PREC_IAU_2006);
  } else if (prec_model == SEMOD_PREC_IAU_2006) {
    return precess_1(R, J, direction, SEMOD_PREC_IAU_2006);
  } else if (prec_model == SEMOD_PREC_BRETAGNON_2003) {
    return precess_1(R, J, direction, SEMOD_PREC_BRETAGNON_2003);
  } else if (prec_model == SEMOD_PREC_LASKAR_1986) {
    return precess_2(R, J, iflag, direction, SEMOD_PREC_LASKAR_1986);
  } else if (prec_model == SEMOD_PREC_SIMON_1994) {
    return precess_2(R, J, iflag, direction, SEMOD_PREC_SIMON_1994);
  } else if (prec_model == SEMOD_PREC_WILLIAMS_1994 || prec_model == SEMOD_PREC_WILL_EPS_LASK) {
    return precess_2(R, J, iflag, direction, SEMOD_PREC_WILLIAMS_1994);
  } else if (prec_model == SEMOD_PREC_OWEN_1990) { 
    return precess_3(R, J, direction, iflag, SEMOD_PREC_OWEN_1990);
  } else { /* SEMOD_PREC_VONDRAK_2011 */
    //int i;
    //int retval = precess_3(R, J, direction, iflag, SEMOD_PREC_VONDRAK_2011);
    // for (i = 0; i < 3; i++) printf("%.9f, ", R[i]);
    return precess_3(R, J, direction, iflag, SEMOD_PREC_VONDRAK_2011);
  }
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
static const short nt[] = {  
/* LS and OC are units of 0.0001"
 *LS2 and OC2 are units of 0.00001"
 *MM,MS,FF,DD,OM, LS, LS2,OC, OC2 */
 0, 0, 0, 0, 2,  2062,  2, -895,  5,
-2, 0, 2, 0, 1,    46,  0,  -24,  0,
 2, 0,-2, 0, 0,    11,  0,    0,  0,
-2, 0, 2, 0, 2,    -3,  0,    1,  0,
 1,-1, 0,-1, 0,    -3,  0,    0,  0,
 0,-2, 2,-2, 1,    -2,  0,    1,  0,
 2, 0,-2, 0, 1,     1,  0,    0,  0,
 0, 0, 2,-2, 2,-13187,-16, 5736,-31,
 0, 1, 0, 0, 0,  1426,-34,   54, -1,
 0, 1, 2,-2, 2,  -517, 12,  224, -6,
 0,-1, 2,-2, 2,   217, -5,  -95,  3,
 0, 0, 2,-2, 1,   129,  1,  -70,  0,
 2, 0, 0,-2, 0,    48,  0,    1,  0,
 0, 0, 2,-2, 0,   -22,  0,    0,  0,
 0, 2, 0, 0, 0,    17, -1,    0,  0,
 0, 1, 0, 0, 1,   -15,  0,    9,  0,
 0, 2, 2,-2, 2,   -16,  1,    7,  0,
 0,-1, 0, 0, 1,   -12,  0,    6,  0,
-2, 0, 0, 2, 1,    -6,  0,    3,  0,
 0,-1, 2,-2, 1,    -5,  0,    3,  0,
 2, 0, 0,-2, 1,     4,  0,   -2,  0,
 0, 1, 2,-2, 1,     4,  0,   -2,  0,
 1, 0, 0,-1, 0,    -4,  0,    0,  0,
 2, 1, 0,-2, 0,     1,  0,    0,  0,
 0, 0,-2, 2, 1,     1,  0,    0,  0,
 0, 1,-2, 2, 0,    -1,  0,    0,  0,
 0, 1, 0, 0, 2,     1,  0,    0,  0,
-1, 0, 0, 1, 1,     1,  0,    0,  0,
 0, 1, 2,-2, 0,    -1,  0,    0,  0,
 0, 0, 2, 0, 2, -2274, -2,  977, -5,
 1, 0, 0, 0, 0,   712,  1,   -7,  0,
 0, 0, 2, 0, 1,  -386, -4,  200,  0,
 1, 0, 2, 0, 2,  -301,  0,  129, -1,
 1, 0, 0,-2, 0,  -158,  0,   -1,  0,
-1, 0, 2, 0, 2,   123,  0,  -53,  0,
 0, 0, 0, 2, 0,    63,  0,   -2,  0,
 1, 0, 0, 0, 1,    63,  1,  -33,  0,
-1, 0, 0, 0, 1,   -58, -1,   32,  0,
-1, 0, 2, 2, 2,   -59,  0,   26,  0,
 1, 0, 2, 0, 1,   -51,  0,   27,  0,
 0, 0, 2, 2, 2,   -38,  0,   16,  0,
 2, 0, 0, 0, 0,    29,  0,   -1,  0,
 1, 0, 2,-2, 2,    29,  0,  -12,  0,
 2, 0, 2, 0, 2,   -31,  0,   13,  0,
 0, 0, 2, 0, 0,    26,  0,   -1,  0,
-1, 0, 2, 0, 1,    21,  0,  -10,  0,
-1, 0, 0, 2, 1,    16,  0,   -8,  0,
 1, 0, 0,-2, 1,   -13,  0,    7,  0,
-1, 0, 2, 2, 1,   -10,  0,    5,  0,
 1, 1, 0,-2, 0,    -7,  0,    0,  0,
 0, 1, 2, 0, 2,     7,  0,   -3,  0,
 0,-1, 2, 0, 2,    -7,  0,    3,  0,
 1, 0, 2, 2, 2,    -8,  0,    3,  0,
 1, 0, 0, 2, 0,     6,  0,    0,  0,
 2, 0, 2,-2, 2,     6,  0,   -3,  0,
 0, 0, 0, 2, 1,    -6,  0,    3,  0,
 0, 0, 2, 2, 1,    -7,  0,    3,  0,
 1, 0, 2,-2, 1,     6,  0,   -3,  0,
 0, 0, 0,-2, 1,    -5,  0,    3,  0,
 1,-1, 0, 0, 0,     5,  0,    0,  0,
 2, 0, 2, 0, 1,    -5,  0,    3,  0, 
 0, 1, 0,-2, 0,    -4,  0,    0,  0,
 1, 0,-2, 0, 0,     4,  0,    0,  0,
 0, 0, 0, 1, 0,    -4,  0,    0,  0,
 1, 1, 0, 0, 0,    -3,  0,    0,  0,
 1, 0, 2, 0, 0,     3,  0,    0,  0,
 1,-1, 2, 0, 2,    -3,  0,    1,  0,
-1,-1, 2, 2, 2,    -3,  0,    1,  0,
-2, 0, 0, 0, 1,    -2,  0,    1,  0,
 3, 0, 2, 0, 2,    -3,  0,    1,  0,
 0,-1, 2, 2, 2,    -3,  0,    1,  0,
 1, 1, 2, 0, 2,     2,  0,   -1,  0,
-1, 0, 2,-2, 1,    -2,  0,    1,  0,
 2, 0, 0, 0, 1,     2,  0,   -1,  0,
 1, 0, 0, 0, 2,    -2,  0,    1,  0,
 3, 0, 0, 0, 0,     2,  0,    0,  0,
 0, 0, 2, 1, 2,     2,  0,   -1,  0,
-1, 0, 0, 0, 2,     1,  0,   -1,  0,

 1, 0, 0,-4, 0,    -1,  0,    0,  0,
-2, 0, 2, 2, 2,     1,  0,   -1,  0,
-1, 0, 2, 4, 2,    -2,  0,    1,  0,
 2, 0, 0,-4, 0,    -1,  0,    0,  0,
 1, 1, 2,-2, 2,     1,  0,   -1,  0,
 1, 0, 2, 2, 1,    -1,  0,    1,  0,
-2, 0, 2, 4, 2,    -1,  0,    1,  0,
-1, 0, 4, 0, 2,     1,  0,    0,  0,
 1,-1, 0,-2, 0,     1,  0,    0,  0,
 2, 0, 2,-2, 1,     1,  0,   -1,  0,
 2, 0, 2, 2, 2,    -1,  0,    0,  0,
 1, 0, 0, 2, 1,    -1,  0,    0,  0,
 0, 0, 4,-2, 2,     1,  0,    0,  0,
 3, 0, 2,-2, 2,     1,  0,    0,  0,
 1, 0, 2,-2, 0,    -1,  0,    0,  0,
 0, 1, 2, 0, 1,     1,  0,    0,  0,
-1,-1, 0, 2, 1,     1,  0,    0,  0,
 0, 0,-2, 0, 1,    -1,  0,    0,  0,
 0, 0, 2,-1, 2,    -1,  0,    0,  0,
 0, 1, 0, 2, 0,    -1,  0,    0,  0,
 1, 0,-2,-2, 0,    -1,  0,    0,  0,
 0,-1, 2, 0, 1,    -1,  0,    0,  0,
 1, 1, 0,-2, 1,    -1,  0,    0,  0,
 1, 0,-2, 2, 0,    -1,  0,    0,  0,
 2, 0, 0, 2, 0,     1,  0,    0,  0,
 0, 0, 2, 4, 2,    -1,  0,    0,  0,
 0, 1, 0, 1, 0,     1,  0,    0,  0,
/*#if NUT_CORR_1987  switch is handled in function calc_nutation_iau1980() */
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
/*#endif*/
 ENDMARK,
};

static int calc_nutation_iau1980(double J, double *nutlo)
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
  const short *p;
  int nut_model = swed.astro_models[SE_MODEL_NUT];
  if (nut_model == 0) nut_model = SEMOD_NUT_DEFAULT;
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
    if (nut_model != SEMOD_NUT_IAU_CORR_1987 && (p[0] == 101 || p[0] == 102))
      continue;
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
static int calc_nutation_iau2000ab(double J, double *nutlo) 
{
  int i, j, k, inls;
  double M, SM, F, D, OM;
  double AL, ALSU, AF, AD, AOM, APA;
  double ALME, ALVE, ALEA, ALMA, ALJU, ALSA, ALUR, ALNE;
  double darg, sinarg, cosarg;
  double dpsi = 0, deps = 0;
  double T = (J - J2000 ) / 36525.0;
  int nut_model = swed.astro_models[SE_MODEL_NUT];
  if (nut_model == 0) nut_model = SEMOD_NUT_DEFAULT;
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
  if (nut_model == SEMOD_NUT_IAU_2000B)
    inls = NLS_2000B;
  else
    inls = NLS;
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
  if (nut_model == SEMOD_NUT_IAU_2000A) {
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
    /* Planetary longitudes, Mercury through Neptune (Souchay et al. 1999). */
    ALME = swe_radnorm(4.402608842 + 2608.7903141574 * T);
    ALVE = swe_radnorm(3.176146697 + 1021.3285546211 * T);
    ALEA = swe_radnorm(1.753470314 +  628.3075849991 * T);
    ALMA = swe_radnorm(6.203480913 +  334.0612426700 * T);
    ALJU = swe_radnorm(0.599546497 +   52.9690962641 * T);
    ALSA = swe_radnorm(0.874016757 +   21.3299104960 * T);
    ALUR = swe_radnorm(5.481293871 +    7.4781598567 * T);
    ALNE = swe_radnorm(5.321159000 +    3.8127774000 * T);
    /* General accumulated precession in longitude. */
    APA = (0.02438175 + 0.00000538691 * T) * T;
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
     * according to Capitaine et al. A & A 412, 366 (2005) = IAU 2006 */
    dpsi = -8.1 * sin(OM) - 0.6 * sin(2 * F - 2 * D + 2 * OM);
    dpsi += T * (47.8 * sin(OM) + 3.7 * sin(2 * F - 2 * D + 2 * OM) + 0.6 * sin(2 * F + 2 * OM) - 0.6 * sin(2 * OM)); 
    deps = T * (-25.6 * cos(OM) - 1.6 * cos(2 * F - 2 * D + 2 * OM));
    nutlo[0] += dpsi / (3600.0 * 1000000.0);
    nutlo[1] += deps / (3600.0 * 1000000.0);
#endif
  }
  nutlo[0] *= DEGTORAD;
  nutlo[1] *= DEGTORAD;
  return 0;
}

static double bessel(double *v, int n, double t)
{
  int i, iy, k;
  double ans, p, B, d[6];
  if (t <= 0) {
    ans = v[0]; 
    goto done;
  } 
  if (t >= n - 1) {
    ans = v[n - 1]; 
    goto done;
  }
  p = floor(t);
  iy = (int) t;
  /* Zeroth order estimate is value at start of year */
  ans = v[iy];
  k = iy + 1;
  if (k >= n)
    goto done;
  /* The fraction of tabulation interval */
  p = t - p;
  ans += p * (v[k] - v[iy]);
  if( (iy - 1 < 0) || (iy + 2 >= n) )
    goto done; /* can't do second differences */
  /* Make table of first differences */
  k = iy - 2;
  for (i = 0; i < 5; i++) {
    if((k < 0) || (k + 1 >= n)) 
      d[i] = 0;
    else
      d[i] = v[k+1] - v[k];
    k += 1;
  }
  /* Compute second differences */
  for (i = 0; i < 4; i++ )
    d[i] = d[i+1] - d[i];
  B = 0.25 * p * (p - 1.0);
  ans += B * (d[1] + d[2]);
#if DEMO
  printf("B %.4lf, ans %.4lf\n", B, ans);
#endif
  if (iy + 2 >= n)
    goto done;
  /* Compute third differences */
  for (i = 0; i < 3; i++ )
    d[i] = d[i + 1] - d[i];
  B = 2.0 * B / 3.0;
  ans += (p - 0.5) * B * d[1];
#if DEMO
  printf("B %.4lf, ans %.4lf\n", B * (p - 0.5), ans);
#endif
  if ((iy - 2 < 0) || (iy + 3 > n))
    goto done;
  /* Compute fourth differences */
  for (i = 0; i < 2; i++)
    d[i] = d[i + 1] - d[i];
  B = 0.125 * B * (p + 1.0) * (p - 2.0);
  ans += B * (d[0] + d[1]);
#if DEMO
  printf("B %.4lf, ans %.4lf\n", B, ans);
#endif
done:
  return ans;
}

static int calc_nutation(double J, int32 iflag, double *nutlo)
{
  int n;
  double dpsi, deps, J2;
  int nut_model = swed.astro_models[SE_MODEL_NUT];
  int jplhora_model = swed.astro_models[SE_MODEL_JPLHORA_MODE];
  AS_BOOL is_jplhor = FALSE;
  if (nut_model == 0) nut_model = SEMOD_NUT_DEFAULT;
  if (jplhora_model == 0) jplhora_model = SEMOD_JPLHORA_DEFAULT;
  if (iflag & SEFLG_JPLHOR)
    is_jplhor = TRUE;
  if ((iflag & SEFLG_JPLHOR_APPROX) && 
      jplhora_model == SEMOD_JPLHORA_3 
      && J <= HORIZONS_TJD0_DPSI_DEPS_IAU1980)
    is_jplhor = TRUE;
  if (is_jplhor) {
    calc_nutation_iau1980(J, nutlo);
    if (iflag & SEFLG_JPLHOR) {
      n = (int) (swed.eop_tjd_end - swed.eop_tjd_beg + 0.000001);
      J2 = J;
      if (J < swed.eop_tjd_beg_horizons)
	J2 = swed.eop_tjd_beg_horizons;
      dpsi = bessel(swed.dpsi, n + 1, J2 - swed.eop_tjd_beg);
      deps = bessel(swed.deps, n + 1, J2 - swed.eop_tjd_beg);
      nutlo[0] += dpsi / 3600.0 * DEGTORAD;
      nutlo[1] += deps / 3600.0 * DEGTORAD;
#if 0
      printf("tjd=%f, dpsi=%f, deps=%f\n", J, dpsi * 1000, deps * 1000);
#endif
    } else {
      nutlo[0] += DPSI_IAU1980_TJD0 / 3600.0 * DEGTORAD;
      nutlo[1] += DEPS_IAU1980_TJD0 / 3600.0 * DEGTORAD;
    }
  } else if (nut_model == SEMOD_NUT_IAU_1980 || nut_model == SEMOD_NUT_IAU_CORR_1987) {
    calc_nutation_iau1980(J, nutlo);
  } else if (nut_model == SEMOD_NUT_IAU_2000A || nut_model == SEMOD_NUT_IAU_2000B) {
    calc_nutation_iau2000ab(J, nutlo);
    if ((iflag & SEFLG_JPLHOR_APPROX) && jplhora_model == SEMOD_JPLHORA_2) {
      nutlo[0] += -41.7750 / 3600.0 / 1000.0 * DEGTORAD;
      nutlo[1] += -6.8192 / 3600.0 / 1000.0 * DEGTORAD;
    }
  }
  return OK;
}

static double quadratic_intp(double ym, double y0, double yp, double x)
{
  double a, b, c, y;
  c = y0;
  b = (yp - ym) / 2.0;
  a = (yp + ym) / 2.0 - c;
  y = a * x * x + b * x + c;
  return y;
}

int swi_nutation(double tjd, int32 iflag, double *nutlo)
{
  int retc = OK;
  double dnut[2], dx;
  if (!swed.do_interpolate_nut) {
    retc = calc_nutation(tjd, iflag, nutlo);
  // from interpolation, with three data points in 1-day steps;
  // maximum error is about 3 mas
  } else {
    // precalculated data points available
    if (tjd < swed.interpol.tjd_nut2 && tjd > swed.interpol.tjd_nut0) {
      dx = (tjd - swed.interpol.tjd_nut0) - 1.0;
      nutlo[0] = quadratic_intp(swed.interpol.nut_dpsi0, swed.interpol.nut_dpsi1, swed.interpol.nut_dpsi2, dx);
      nutlo[1] = quadratic_intp(swed.interpol.nut_deps0, swed.interpol.nut_deps1, swed.interpol.nut_deps2, dx);
    } else {
      swed.interpol.tjd_nut0 = tjd - 1.0; // one day earlier
      swed.interpol.tjd_nut2 = tjd + 1.0; // one day later
      retc = calc_nutation(swed.interpol.tjd_nut0, iflag, dnut);
      if (retc == ERR) return ERR;
      swed.interpol.nut_dpsi0 = dnut[0];
      swed.interpol.nut_deps0 = dnut[1];
      retc = calc_nutation(swed.interpol.tjd_nut2, iflag, dnut);
      if (retc == ERR) return ERR;
      swed.interpol.nut_dpsi2 = dnut[0];
      swed.interpol.nut_deps2 = dnut[1];
      retc = calc_nutation(tjd, iflag, nutlo);
      if (retc == ERR) return ERR;
      swed.interpol.nut_dpsi1 = nutlo[0];
      swed.interpol.nut_deps1 = nutlo[1];
    }
  }
  return retc;
}

#define OFFSET_JPLHORIZONS (-52.3) 
#define DCOR_RA_JPL_TJD0  2437846.5
#define NDCOR_RA_JPL  51
double dcor_ra_jpl[] = {
-51.257, -51.103, -51.065, -51.503, -51.224, -50.796, -51.161, -51.181,
-50.932, -51.064, -51.182, -51.386, -51.416, -51.428, -51.586, -51.766, -52.038, -52.370,
-52.553, -52.397, -52.340, -52.676, -52.348, -51.964, -52.444, -52.364, -51.988, -52.212,
-52.370, -52.523, -52.541, -52.496, -52.590, -52.629, -52.788, -53.014, -53.053, -52.902,
-52.850, -53.087, -52.635, -52.185, -52.588, -52.292, -51.796, -51.961, -52.055, -52.134,
-52.165, -52.141, -52.255,
};

static void swi_approx_jplhor(double *x, double tjd, int32 iflag, AS_BOOL backward) 
{
  double t0, t1;
  double t = (tjd - DCOR_RA_JPL_TJD0) / 365.25;
  double dofs = OFFSET_JPLHORIZONS;
  int jplhora_model = swed.astro_models[SE_MODEL_JPLHORA_MODE];
  if (jplhora_model == 0) jplhora_model = SEMOD_JPLHORA_DEFAULT;
  if (!(iflag & SEFLG_JPLHOR_APPROX))
    return;
  if (jplhora_model == SEMOD_JPLHORA_2)
    return;
  if (t < 0) {
    t = 0;
    dofs = dcor_ra_jpl[0];
  } else if (t >= NDCOR_RA_JPL - 1) {
    t = NDCOR_RA_JPL;
    dofs = dcor_ra_jpl[NDCOR_RA_JPL - 1];
  } else {
    t0 = (int) t;
    t1 = t0 + 1;
    dofs = dcor_ra_jpl[(int)t0];
    dofs = (t - t0) * (dcor_ra_jpl[(int)t0] - dcor_ra_jpl[(int)t1]) + dcor_ra_jpl[(int)t0];
  }
  dofs /= (1000.0 * 3600.0);
  swi_cartpol(x, x);
  if (backward) 
    x[0] -= dofs * DEGTORAD;
  else
    x[0] += dofs * DEGTORAD;
  swi_polcart(x, x);
}

/* GCRS to J2000 */
void swi_bias(double *x, double tjd, int32 iflag, AS_BOOL backward)
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
  int bias_model = swed.astro_models[SE_MODEL_BIAS];
  int jplhora_model = swed.astro_models[SE_MODEL_JPLHORA_MODE];
  if (bias_model == 0) bias_model = SEMOD_BIAS_DEFAULT;
  if (jplhora_model == 0) jplhora_model = SEMOD_JPLHORA_DEFAULT;
  if (bias_model == SEMOD_BIAS_NONE)
    return;
  if (iflag & SEFLG_JPLHOR_APPROX) {
    if (jplhora_model == SEMOD_JPLHORA_2)
      return;
    if (jplhora_model == SEMOD_JPLHORA_3 && tjd < DPSI_DEPS_IAU1980_TJD0_HORIZONS)
      return;
  }
/* #if FRAME_BIAS_IAU2006 * frame bias 2006 */
  if (bias_model == SEMOD_BIAS_IAU2006) {
    rb[0][0] = +0.99999999999999412;
    rb[1][0] = -0.00000007078368961;
    rb[2][0] = +0.00000008056213978;
    rb[0][1] = +0.00000007078368695;
    rb[1][1] = +0.99999999999999700;
    rb[2][1] = +0.00000003306428553;
    rb[0][2] = -0.00000008056214212;
    rb[1][2] = -0.00000003306427981;
    rb[2][2] = +0.99999999999999634;
/* #else * frame bias 2000, makes no difference in result */
  } else {
    rb[0][0] = +0.9999999999999942;
    rb[1][0] = -0.0000000707827974;
    rb[2][0] = +0.0000000805621715;
    rb[0][1] = +0.0000000707827948;
    rb[1][1] = +0.9999999999999969;
    rb[2][1] = +0.0000000330604145;
    rb[0][2] = -0.0000000805621738;
    rb[1][2] = -0.0000000330604088;
    rb[2][2] = +0.9999999999999962;
  }
/*#endif*/
#if 0
rb[0][0] = +0.9999999999999968;
rb[1][0] = +0.0000000000000000;
rb[2][0] = +0.0000000805621715;
rb[0][1] = -0.0000000000000027;
rb[1][1] = +0.9999999999999994;
rb[2][1] = +0.0000000330604145;
rb[0][2] = -0.0000000805621715;
rb[1][2] = -0.0000000330604145;
rb[2][2] = +0.9999999999999962; 
#endif
  if (backward) {
    swi_approx_jplhor(x, tjd, iflag, TRUE);
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
    swi_approx_jplhor(xx, tjd, iflag, FALSE);
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
 * Before 1955 we use the data developed by 
 * Stephenson, Morrison, and Hohenkerk (2016),
 *
 * 1955 - today + a couple of years:
 * ---------------------------------
 * The tabulated values of deltaT from the Astronomical
 * Alamanc (AA 1997 etc. pp. K8-K9) are used. Some 
 * more recent values have been taken from IERS
 * (http://maia.usno.navy.mil/ser7/deltat.data).
 * Bessel's interpolation formula is implemented to obtain fourth 
 * order interpolated values at intermediate times.
 * The values are adjusted depending on the ephemeris used
 * and its inherent value of secular tidal acceleration ndot.
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
 * Stephenson, F.R., Morrison, L.V., and Hohenkerk, C.Y., "Measurement of the
 * Earth's Rotation: 720 BC to AD 2015", Royal Society Proceedings A 
 * 7 Dec 2016,
 * http://rspa.royalsocietypublishing.org/lookup/doi/10.1098/rspa.2016.0404
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
 * (UT1-UTC) from: ftp://maia.usno.navy.mil/ser7/finals.all (cols. 59-68)
 *             or: ftp://ftp.iers.org/products/eop/rapid/standard/finals.data
 * file description in: ftp://maia.usno.navy.mil/ser7/readme.finals
 * Delta T = TAI-UT1 + 32.184 sec = (TAI-UTC) - (UT1-UTC) + 32.184 sec
 *
 * Also, there is the following file: 
 * http://maia.usno.navy.mil/ser7/deltat.data, but it is about 3 months
 * behind (on 3 feb 2009); and predictions:
 * http://maia.usno.navy.mil/ser7/deltat.preds
 *
 * Last update of table dt[]: Dieter Koch, 18 dec 2013.
 * ATTENTION: Whenever updating this table, do not forget to adjust
 * the macros TABEND and TABSIZ !
 */
#define TABSTART 	1620
#define TABEND 		2026
#define TABSIZ 		(TABEND-TABSTART+1) 
/* we make the table greater for additional values read from external file */
#define TABSIZ_SPACE 	(TABSIZ+100)
static TLS double dt[TABSIZ_SPACE] = {
/* 1620.0 - 1659.0 */
124.00, 119.00, 115.00, 110.00, 106.00, 102.00, 98.00, 95.00, 91.00, 88.00,
85.00, 82.00, 79.00, 77.00, 74.00, 72.00, 70.00, 67.00, 65.00, 63.00,
62.00, 60.00, 58.00, 57.00, 55.00, 54.00, 53.00, 51.00, 50.00, 49.00,
48.00, 47.00, 46.00, 45.00, 44.00, 43.00, 42.00, 41.00, 40.00, 38.00,
/* 1660.0 - 1699.0 */
37.00, 36.00, 35.00, 34.00, 33.00, 32.00, 31.00, 30.00, 28.00, 27.00,
26.00, 25.00, 24.00, 23.00, 22.00, 21.00, 20.00, 19.00, 18.00, 17.00,
16.00, 15.00, 14.00, 14.00, 13.00, 12.00, 12.00, 11.00, 11.00, 10.00,
10.00, 10.00, 9.00, 9.00, 9.00, 9.00, 9.00, 9.00, 9.00, 9.00,
/* 1700.0 - 1739.0 */
9.00, 9.00, 9.00, 9.00, 9.00, 9.00, 9.00, 9.00, 10.00, 10.00,
10.00, 10.00, 10.00, 10.00, 10.00, 10.00, 10.00, 11.00, 11.00, 11.00,
11.00, 11.00, 11.00, 11.00, 11.00, 11.00, 11.00, 11.00, 11.00, 11.00,
11.00, 11.00, 11.00, 11.00, 12.00, 12.00, 12.00, 12.00, 12.00, 12.00,
/* 1740.0 - 1779.0 */
12.00, 12.00, 12.00, 12.00, 13.00, 13.00, 13.00, 13.00, 13.00, 13.00,
13.00, 14.00, 14.00, 14.00, 14.00, 14.00, 14.00, 14.00, 15.00, 15.00,
15.00, 15.00, 15.00, 15.00, 15.00, 16.00, 16.00, 16.00, 16.00, 16.00,
16.00, 16.00, 16.00, 16.00, 16.00, 17.00, 17.00, 17.00, 17.00, 17.00,
/* 1780.0 - 1799.0 */
17.00, 17.00, 17.00, 17.00, 17.00, 17.00, 17.00, 17.00, 17.00, 17.00,
17.00, 17.00, 16.00, 16.00, 16.00, 16.00, 15.00, 15.00, 14.00, 14.00,
/* 1800.0 - 1819.0 */
13.70, 13.40, 13.10, 12.90, 12.70, 12.60, 12.50, 12.50, 12.50, 12.50,
12.50, 12.50, 12.50, 12.50, 12.50, 12.50, 12.50, 12.40, 12.30, 12.20,
/* 1820.0 - 1859.0 */
12.00, 11.70, 11.40, 11.10, 10.60, 10.20, 9.60, 9.10, 8.60, 8.00,
7.50, 7.00, 6.60, 6.30, 6.00, 5.80, 5.70, 5.60, 5.60, 5.60,
5.70, 5.80, 5.90, 6.10, 6.20, 6.30, 6.50, 6.60, 6.80, 6.90,
7.10, 7.20, 7.30, 7.40, 7.50, 7.60, 7.70, 7.70, 7.80, 7.80,
/* 1860.0 - 1899.0 */
7.88, 7.82, 7.54, 6.97, 6.40, 6.02, 5.41, 4.10, 2.92, 1.82,
1.61, .10, -1.02, -1.28, -2.69, -3.24, -3.64, -4.54, -4.71, -5.11,
-5.40, -5.42, -5.20, -5.46, -5.46, -5.79, -5.63, -5.64, -5.80, -5.66,
-5.87, -6.01, -6.19, -6.64, -6.44, -6.47, -6.09, -5.76, -4.66, -3.74,
/* 1900.0 - 1939.0 */
-2.72, -1.54, -.02, 1.24, 2.64, 3.86, 5.37, 6.14, 7.75, 9.13,
10.46, 11.53, 13.36, 14.65, 16.01, 17.20, 18.24, 19.06, 20.25, 20.95,
21.16, 22.25, 22.41, 23.03, 23.49, 23.62, 23.86, 24.49, 24.34, 24.08,
24.02, 24.00, 23.87, 23.95, 23.86, 23.93, 23.73, 23.92, 23.96, 24.02,
/* 1940.0 - 1949.0 */
24.33, 24.83, 25.30, 25.70, 26.24, 26.77, 27.28, 27.78, 28.25, 28.71,
/* 1950.0 - 1959.0 */
29.15, 29.57, 29.97, 30.36, 30.72, 31.07, 31.35, 31.68, 32.18, 32.68,
/* 1960.0 - 1969.0 */
33.15, 33.59, 34.00, 34.47, 35.03, 35.73, 36.54, 37.43, 38.29, 39.20,
/* 1970.0 - 1979.0 */
/* from 1974 on values (with 4-digit precision) were calculated from IERS data */
40.18, 41.17, 42.23, 43.37, 44.4841, 45.4761, 46.4567, 47.5214, 48.5344, 49.5862,
/* 1980.0 - 1989.0 */
50.5387, 51.3808, 52.1668, 52.9565, 53.7882, 54.3427, 54.8713, 55.3222, 55.8197, 56.3000,
/* 1990.0 - 1999.0 */
56.8553, 57.5653, 58.3092, 59.1218, 59.9845, 60.7854, 61.6287, 62.2951, 62.9659, 63.4673,
/* 2000.0 - 2009.0 */
63.8285, 64.0908, 64.2998, 64.4734, 64.5736, 64.6876, 64.8452, 65.1464, 65.4574, 65.7768,
/* 2010.0 - 2017.0 */
66.0699, 66.3246, 66.6030, 66.9069, 67.2810, 67.6439, 68.1024, 68.5927,
/* Extrapolated values, 2018 - 2019 */
                                                                        68.9689, 69.35,
/* Extrapolated values, 2020 - 2026 */
69.80, 70.20, 70.60, 71.00, 71.50, 72.00, 72.50,
};

#define TAB2_SIZ	27
#define TAB2_START	(-1000)
#define TAB2_END	1600
#define TAB2_STEP	100
#define LTERM_EQUATION_YSTART	1820
#define LTERM_EQUATION_COEFF	32
/* Table for -1000 through 1600, from Morrison & Stephenson (2004).  */
static const short dt2[TAB2_SIZ] = {
/*-1000  -900  -800  -700  -600  -500  -400  -300  -200  -100*/
25400,23700,22000,21000,19040,17190,15530,14080,12790,11640,
/*    0   100   200   300   400   500   600   700   800   900*/
10580, 9600, 8640, 7680, 6700, 5710, 4740, 3810, 2960, 2200,
/* 1000  1100  1200  1300  1400  1500  1600,                 */
 1570, 1090,  740,  490,  320,  200,  120,  
};

/* Table for -500 through 1600, from Stephenson & Morrison (1995).
 * 
 * The first value for -550 has been added from Borkowski
 * in order to make this table fit with the Borkowski formula
 * for times before -550.
 */
#define TAB97_SIZ        43
#define TAB97_START      (-500)
#define TAB97_END        (1600)
#define TAB97_STEP       (50)
static const short dt97[TAB97_SIZ] = {
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
 * delta t is adjusted to the tidal acceleration that is compatible 
 * with the ephemeris flag contained in iflag and with the ephemeris
 * files made accessible through swe_set_ephe_path() or swe_set_jplfile().
 * If iflag = -1, then the default tidal acceleration is ussed (i.e.
 * that of DE431).
 */
#define DEMO 0
static int32 calc_deltat(double tjd, int32 iflag, double *deltat, char *serr)
{
  double ans = 0;
  double B, Y, Ygreg, dd;
  int iy;
  int32 retc;
  int deltat_model = swed.astro_models[SE_MODEL_DELTAT];
  double tid_acc;
  int32 denumret;
  int32 epheflag, otherflag;
//fprintf(stderr, "dmod=%f, %.f\n", (double) deltat_model, (double) SEMOD_DELTAT_DEFAULT);
  if (deltat_model == 0) deltat_model = SEMOD_DELTAT_DEFAULT;
  epheflag = iflag & SEFLG_EPHMASK;
  otherflag = iflag & ~SEFLG_EPHMASK;
  /* with iflag == -1, we use default tid_acc */
  if (iflag == -1) {
    retc = swi_get_tid_acc(tjd, 0, 9999, &denumret, &tid_acc, serr); /* for default tid_acc */
  /* otherwise we use tid_acc consistent with epheflag */
  } else {
    if (swi_init_swed_if_start() == 1 && !(epheflag & SEFLG_MOSEPH)) {
      if (serr != NULL) 
	strcpy(serr, "Please call swe_set_ephe_path() or swe_set_jplfile() before calling swe_deltat_ex()");
      retc = swi_set_tid_acc(tjd, epheflag, 0, NULL);  /* _set_ saves tid_acc in swed */
    } else {
      retc = swi_set_tid_acc(tjd, epheflag, 0, serr);  /* _set_ saves tid_acc in swed */
    }
    tid_acc = swed.tid_acc;
  }
  iflag = otherflag | retc;
  Y = 2000.0 + (tjd - J2000)/365.25;
  Ygreg = 2000.0 + (tjd - J2000)/365.2425;
  /* Model for epochs before 1955, currently default in Swiss Ephemeris:
   * Stephenson/Morrison/Hohenkerk 2016 
   * (we switch over to Astronomical Almanac K8-K9 and IERS at 1 Jan. 1955. 
   * To make the curve continuous we apply some linear term over 
   * 1000 days before this date.)
   * Delta T according to Stephenson/Morrison/Hohenkerk 2016 is based on
   * ancient, medieval, and modern observations of eclipses and occultations.
   * Values of Deltat T before 1955 depend on this kind of observations.
   * For more recent data we want to use the data provided by IERS 
   * (or Astronomical Almanac K8-K9).
   */
  if (deltat_model == SEMOD_DELTAT_STEPHENSON_ETC_2016 && tjd < 2435108.5) { // tjd < 2432521.453645833) {
    *deltat = deltat_stephenson_etc_2016(tjd, tid_acc);
    if (tjd >= 2434108.5) {
      *deltat += (1.0 - (2435108.5 - tjd) / 1000.0) * 0.6610218 / 86400.0;
    }
    return iflag;
  }
  /* Model used SE 1.77 - 2.05.01, for epochs before 1633:
   * Polynomials by Espenak & Meeus 2006, 
   * derived from Stephenson & Morrison 2004.
   * deltat_model == SEMOD_DELTAT_ESPENAK_MEEUS_2006: 
   * This method is used only for epochs before 1633. (For more recent
   * epochs, we use the data provided by Astronomical Almanac K8-K9.)
   */
  if (deltat_model == SEMOD_DELTAT_ESPENAK_MEEUS_2006 && tjd < 2317746.13090277789) {
    *deltat = deltat_espenak_meeus_1620(tjd, tid_acc);
    return iflag;
  }
  /* delta t model used in SE 1.72 - 1.76:
   * Stephenson & Morrison 2004;
   * before 1620 */
  if (deltat_model == SEMOD_DELTAT_STEPHENSON_MORRISON_2004 && Y < TABSTART) {
    // before 1600: 
    if (Y < TAB2_END) {
      *deltat = deltat_stephenson_morrison_2004_1600(tjd, tid_acc);
      return iflag;
    } else {
      /* between 1600 and 1620:
       * linear interpolation between 
       * end of table dt2 and start of table dt */
      if (Y >= TAB2_END) { 
	B = TABSTART - TAB2_END;
	iy = (TAB2_END - TAB2_START) / TAB2_STEP;
	dd = (Y - TAB2_END) / B;
	ans = dt2[iy] + dd * (dt[0] - dt2[iy]);
	ans = adjust_for_tidacc(ans, Ygreg, tid_acc, SE_TIDAL_26, FALSE);
	*deltat = ans / 86400.0;
	return iflag;
      }
    }
  }
  /* delta t model used in SE 1.64 - 1.71:
   * Stephenson 1997;
   * before 1620 */
  if (deltat_model == SEMOD_DELTAT_STEPHENSON_1997 && Y < TABSTART) {
    // before 1600: 
    if (Y < TAB97_END) {
      *deltat = deltat_stephenson_morrison_1997_1600(tjd, tid_acc);
      return iflag;
    } else {
      /* between 1600 and 1620:
       * linear interpolation between 
       * end of table dt2 and start of table dt */
      if (Y >= TAB97_END) { 
	B = TABSTART - TAB97_END;
	iy = (TAB97_END - TAB97_START) / TAB97_STEP;
	dd = (Y - TAB97_END) / B;
	ans = dt97[iy] + dd * (dt[0] - dt97[iy]);
	ans = adjust_for_tidacc(ans, Ygreg, tid_acc, SE_TIDAL_26, FALSE);
	*deltat = ans / 86400.0;
	return iflag;
      }
    }
  }
  /* delta t model used before SE 1.64: 
   * Stephenson/Morrison 1984 with Borkowski 1988; 
   * before 1620 */
  if (deltat_model == SEMOD_DELTAT_STEPHENSON_MORRISON_1984 && Y < TABSTART) {
    if( Y >= 948.0 ) {
      /* Stephenson and Morrison, stated domain is 948 to 1600:
       * 25.5(centuries from 1800)^2 - 1.9159(centuries from 1955)^2 */
      B = 0.01 * (Y - 2000.0);
      ans = (23.58 * B + 100.3) * B + 101.6;
    } else {
      /* Borkowski, before 948 and between 1600 and 1620 */
      B = 0.01 * (Y - 2000.0)  +  3.75;
      ans = 35.0 * B * B  +  40.;
    }
    *deltat = ans / 86400.0;
    return iflag;
  }
  /* 1620 - today + a few years (tabend):
   * Tabulated values of deltaT from Astronomical Almanac 
   * (AA 1997etc., pp. K8-K9) and from IERS  
   * (http://maia.usno.navy.mil/ser7/deltat.data).
   */
  if (Y >= TABSTART) {
    *deltat = deltat_aa(tjd, tid_acc);
    return iflag;
  }
#ifdef TRACE
  swi_open_trace(NULL);
  if (swi_trace_count < TRACE_COUNT_MAX) {
    if (swi_fp_trace_c != NULL) {
      fputs("\n/*SWE_DELTAT*/\n", swi_fp_trace_c);
      fprintf(swi_fp_trace_c, "  tjd = %.9f;", tjd);
      fprintf(swi_fp_trace_c, "  iflag = %d;", iflag);
      fprintf(swi_fp_trace_c, " t = swe_deltat_ex(tjd, iflag, NULL);\n");
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
  *deltat = ans / 86400.0;
  return iflag;
}

double CALL_CONV swe_deltat_ex(double tjd, int32 iflag, char *serr)
{
  double deltat;
  if (swed.delta_t_userdef_is_set)
    return swed.delta_t_userdef;
  calc_deltat(tjd, iflag, &deltat, serr);
  return deltat;
}

double CALL_CONV swe_deltat(double tjd)
{
  int32 iflag = swi_guess_ephe_flag();
  return swe_deltat_ex(tjd, iflag, NULL); /* with default tidal acceleration/default ephemeris */
}

/* The tabulated values of deltaT, in hundredths of a second,
 * were taken from The Astronomical Almanac 1997etc., pp. K8-K9.  
 * Some more recent values are taken from IERS
 * http://maia.usno.navy.mil/ser7/deltat.data .
 * Bessel's interpolation formula is implemented to obtain fourth 
 * order interpolated values at intermediate times.
 * The values are adjusted depending on the ephemeris used
 * and its inherent value of secular tidal acceleration ndot.
 * Note by Dieter Jan. 2017:
 * Bessel interpolation assumes equidistant sampling points. However the
 * sampling points are not equidistant, because they are for first January of
 * every year and years can have either 365 or 366 days. The interpolation uses
 * a step width of 365.25 days. As a consequence, in three out of four years
 * the interpolation does not reproduce the exact values of the sampling points
 * on the days they refer to.  */
static double deltat_aa(double tjd, double tid_acc)
{
  double ans = 0, ans2 = 0, ans3;
  double p, B, B2, Y, dd;
  double d[6];
  int i, iy, k;
  /* read additional values from swedelta.txt */
  int tabsiz = init_dt();
  int tabend = TABSTART + tabsiz - 1;
  int deltat_model = swed.astro_models[SE_MODEL_DELTAT];
  if (deltat_model == 0) deltat_model = SEMOD_DELTAT_DEFAULT;
  Y = 2000.0 + (tjd - 2451544.5)/365.25;
  if (Y <= tabend) {
    /* Index into the table.
     */
    p = floor(Y);
    iy = (int) (p - TABSTART);
    /* Zeroth order estimate is value at start of year */
    ans = dt[iy];
    k = iy + 1;
    if( k >= tabsiz )
      goto done; /* No data, can't go on. */
    /* The fraction of tabulation interval */
    p = Y - p;
    /* First order interpolated value */
    ans += p*(dt[k] - dt[iy]);
    if( (iy-1 < 0) || (iy+2 >= tabsiz) )
      goto done; /* can't do second differences */
    /* Make table of first differences */
    k = iy - 2;
    for( i=0; i<5; i++ ) {
      if( (k < 0) || (k+1 >= tabsiz) ) 
	d[i] = 0;
      else
	d[i] = dt[k+1] - dt[k];
      k += 1;
    }
    /* Compute second differences */
    for( i=0; i<4; i++ )
      d[i] = d[i+1] - d[i];
    B = 0.25*p*(p-1.0);
    ans += B*(d[1] + d[2]);
#if DEMO
    printf( "B %.4lf, ans %.4lf\n", B, ans );
#endif
    if( iy+2 >= tabsiz )
      goto done;
    /* Compute third differences */
    for( i=0; i<3; i++ )
      d[i] = d[i+1] - d[i];
    B = 2.0*B/3.0;
    ans += (p-0.5)*B*d[1];
#if DEMO
    printf( "B %.4lf, ans %.4lf\n", B*(p-0.5), ans );
#endif
    if( (iy-2 < 0) || (iy+3 > tabsiz) )
      goto done;
    /* Compute fourth differences */
    for( i=0; i<2; i++ )
      d[i] = d[i+1] - d[i];
    B = 0.125*B*(p+1.0)*(p-2.0);
    ans += B*(d[0] + d[1]);
#if DEMO
    printf( "B %.4lf, ans %.4lf\n", B, ans );
#endif
    done:
    ans = adjust_for_tidacc(ans, Y, tid_acc, SE_TIDAL_26, FALSE);
    return ans / 86400.0;
  }
  /* today - future: 
   * 3rd degree polynomial based on data given by 
   * Stephenson/Morrison/Hohenkerk 2016 here:
   * http://astro.ukho.gov.uk/nao/lvm/
   */
  if (deltat_model == SEMOD_DELTAT_STEPHENSON_ETC_2016) {
    B = (Y - 2000);
    if (Y < 2500) {
      ans = B * B * B * 121.0 / 30000000.0 + B * B / 1250.0 + B * 521.0 / 3000.0 + 64.0;
      /* for slow transition from tablulated data */
      B2 = (tabend - 2000);
      ans2 = B2 * B2 * B2 * 121.0 / 30000000.0 + B2 * B2 / 1250.0 + B2 * 521.0 / 3000.0 + 64.0;
    /* we use a parable after 2500 */
    } else {
      B = 0.01 * (Y - 2000);
      ans = B * B * 32.5 + 42.5;
    }
  /* 
   * Formula Stephenson (1997; p. 507),
   * with modification to avoid jump at end of AA table,
   * similar to what Meeus 1998 had suggested.
   * Slow transition within 100 years.
   */
  } else {
    B = 0.01 * (Y - 1820);
    ans = -20 + 31 * B * B;
    /* for slow transition from tablulated data */
    B2 = 0.01 * (tabend - 1820);
    ans2 = -20 + 31 * B2 * B2;
  }
  /* slow transition from tabulated values to Stephenson formula: */
  if (Y <= tabend+100) {
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

static double deltat_stephenson_morrison_1997_1600(double tjd, double tid_acc)
{
  double ans = 0, ans2, ans3;
  double p, B, Y, dd;
  int iy;
  Y = 2000.0 + (tjd - J2000)/365.25;
  /* before -500:
   * formula by Stephenson (1997; p. 508) but adjusted to fit the starting
   * point of table dt97 (Stephenson 1997). */
  if( Y < TAB97_START ) {
    B = (Y - 1735) * 0.01;
    ans = -20 + 35 * B * B;
    ans = adjust_for_tidacc(ans, Y, tid_acc, SE_TIDAL_26, FALSE);
    /* transition from formula to table over 100 years */
    if (Y >= TAB97_START - 100) {
      /* starting value of table dt97: */
      ans2 = adjust_for_tidacc(dt97[0], TAB97_START, tid_acc, SE_TIDAL_26, FALSE);
      /* value of formula at epoch TAB97_START */
      B = (TAB97_START - 1735) * 0.01;
      ans3 = -20 + 35 * B * B;
      ans3 = adjust_for_tidacc(ans3, Y, tid_acc, SE_TIDAL_26, FALSE);
      dd = ans3 - ans2;
      B = (Y - (TAB97_START - 100)) * 0.01;
      /* fit to starting point of table dt97. */
      ans = ans - dd * B;
    }
  }
  /* between -500 and 1600:
   * linear interpolation between values of table dt97 (Stephenson 1997) */
  if (Y >= TAB97_START && Y < TAB2_END) {
    p = floor(Y);
    iy = (int) ((p - TAB97_START) / 50.0);
    dd = (Y - (TAB97_START + 50 * iy)) / 50.0;
    ans = dt97[iy] + (dt97[iy+1] - dt97[iy]) * dd;
    /* correction for tidal acceleration used by our ephemeris */
    ans = adjust_for_tidacc(ans, Y, tid_acc, SE_TIDAL_26, FALSE);
  }
  ans /= 86400.0;
  return ans;
}

/* Stephenson & Morrison (2004) */
static double deltat_stephenson_morrison_2004_1600(double tjd, double tid_acc)
{
  double ans = 0, ans2, ans3;
  double p, B, dd;
  double tjd0;
  int iy;
  double Y = 2000.0 + (tjd - J2000)/365.2425;
  /* double Y = 2000.0 + (tjd - J2000)/365.25;*/
  /* before -1000:
   * formula by Stephenson & Morrison (2004; p. 335) but adjusted to fit the 
   * starting point of table dt2. */
  if( Y < TAB2_START ) { // before -1000
    ans = deltat_longterm_morrison_stephenson(tjd);
    ans = adjust_for_tidacc(ans, Y, tid_acc, SE_TIDAL_26, FALSE);
    /* transition from formula to table over 100 years */
    if (Y >= TAB2_START - 100) {
      /* starting value of table dt2: */
      ans2 = adjust_for_tidacc(dt2[0], TAB2_START, tid_acc, SE_TIDAL_26, FALSE);
      /* value of formula at epoch TAB2_START */
      /* B = (TAB2_START - LTERM_EQUATION_YSTART) * 0.01;
      ans3 = -20 + LTERM_EQUATION_COEFF * B * B;*/
      tjd0 = (TAB2_START - 2000) * 365.2425 + J2000;
      ans3 = deltat_longterm_morrison_stephenson(tjd0);
      ans3 = adjust_for_tidacc(ans3, Y, tid_acc, SE_TIDAL_26, FALSE);
      dd = ans3 - ans2;
      B = (Y - (TAB2_START - 100)) * 0.01;
      /* fit to starting point of table dt2. */
      ans = ans - dd * B;
    }
  }
  /* between -1000 and 1600: 
   * linear interpolation between values of table dt2 (Stephenson & Morrison 2004) */
  if (Y >= TAB2_START && Y < TAB2_END) { 
    double Yjul = 2000 + (tjd - 2451557.5) / 365.25;
    p = floor(Yjul);
    iy = (int) ((p - TAB2_START) / TAB2_STEP);
    dd = (Yjul - (TAB2_START + TAB2_STEP * iy)) / TAB2_STEP;
    ans = dt2[iy] + (dt2[iy+1] - dt2[iy]) * dd;
    /* correction for tidal acceleration used by our ephemeris */
    ans = adjust_for_tidacc(ans, Y, tid_acc, SE_TIDAL_26, FALSE);
  }
  ans /= 86400.0;
  return ans;
}

/* 
 * These coefficients represent the spline approximation discussed in the
 * paper "Measurement of the Earth's Rotation: 720 BC to AD 2015",
 * Stephenson, F.R., Morrison, L.V., and Hohenkerk, C.Y., published by
 * Royal Society Proceedings A and available from their website at
 * http://rspa.royalsocietypublishing.org/lookup/doi/10.1098/rspa.2016.0404.
 * Year numbers have been replaced by Julian day numbers by D. Koch.
 */
#define NDTCF16 54 
double dtcf16[NDTCF16][6] =
{
/*00*/ {1458085.5, 1867156.5, 20550.593,-21268.478, 11863.418, -4541.129}, /* ybeg=-720, yend= 400 */
/*01*/ {1867156.5, 2086302.5,  6604.404, -5981.266,  -505.093,  1349.609}, /* ybeg= 400, yend=1000 */
/*02*/ {2086302.5, 2268923.5,  1467.654, -2452.187,  2460.927, -1183.759}, /* ybeg=1000, yend=1500 */
/*03*/ {2268923.5, 2305447.5,   292.635,  -216.322,   -43.614,    56.681}, /* ybeg=1500, yend=1600 */
/*04*/ {2305447.5, 2323710.5,    89.380,   -66.754,    31.607,   -10.497}, /* ybeg=1600, yend=1650 */
/*05*/ {2323710.5, 2349276.5,    43.736,   -49.043,     0.227,    15.811}, /* ybeg=1650, yend=1720 */
/*06*/ {2349276.5, 2378496.5,    10.730,    -1.321,    62.250,   -52.946}, /* ybeg=1720, yend=1800 */
/*07*/ {2378496.5, 2382148.5,    18.714,    -4.457,    -1.509,     2.507}, /* ybeg=1800, yend=1810 */
/*08*/ {2382148.5, 2385800.5,    15.255,     0.046,     6.012,    -4.634}, /* ybeg=1810, yend=1820 */
/*09*/ {2385800.5, 2389453.5,    16.679,    -1.831,    -7.889,     3.799}, /* ybeg=1820, yend=1830 */
/*10*/ {2389453.5, 2393105.5,    10.758,    -6.211,     3.509,    -0.388}, /* ybeg=1830, yend=1840 */
/*11*/ {2393105.5, 2396758.5,     7.668,    -0.357,     2.345,    -0.338}, /* ybeg=1840, yend=1850 */
/*12*/ {2396758.5, 2398584.5,     9.317,     1.659,     0.332,    -0.932}, /* ybeg=1850, yend=1855 */
/*13*/ {2398584.5, 2400410.5,    10.376,    -0.472,    -2.463,     1.596}, /* ybeg=1855, yend=1860 */
/*14*/ {2400410.5, 2402237.5,     9.038,    -0.610,     2.325,    -2.497}, /* ybeg=1860, yend=1865 */
/*15*/ {2402237.5, 2404063.5,     8.256,    -3.450,    -5.166,     2.729}, /* ybeg=1865, yend=1870 */
/*16*/ {2404063.5, 2405889.5,     2.369,    -5.596,     3.020,    -0.919}, /* ybeg=1870, yend=1875 */
/*17*/ {2405889.5, 2407715.5,    -1.126,    -2.312,     0.264,    -0.037}, /* ybeg=1875, yend=1880 */
/*18*/ {2407715.5, 2409542.5,    -3.211,    -1.894,     0.154,     0.562}, /* ybeg=1880, yend=1885 */
/*19*/ {2409542.5, 2411368.5,    -4.388,     0.101,     1.841,    -1.438}, /* ybeg=1885, yend=1890 */
/*20*/ {2411368.5, 2413194.5,    -3.884,    -0.531,    -2.473,     1.870}, /* ybeg=1890, yend=1895 */
/*21*/ {2413194.5, 2415020.5,    -5.017,     0.134,     3.138,    -0.232}, /* ybeg=1895, yend=1900 */
/*22*/ {2415020.5, 2416846.5,    -1.977,     5.715,     2.443,    -1.257}, /* ybeg=1900, yend=1905 */
/*23*/ {2416846.5, 2418672.5,     4.923,     6.828,    -1.329,     0.720}, /* ybeg=1905, yend=1910 */
/*24*/ {2418672.5, 2420498.5,    11.142,     6.330,     0.831,    -0.825}, /* ybeg=1910, yend=1915 */
/*25*/ {2420498.5, 2422324.5,    17.479,     5.518,    -1.643,     0.262}, /* ybeg=1915, yend=1920 */
/*26*/ {2422324.5, 2424151.5,    21.617,     3.020,    -0.856,     0.008}, /* ybeg=1920, yend=1925 */
/*27*/ {2424151.5, 2425977.5,    23.789,     1.333,    -0.831,     0.127}, /* ybeg=1925, yend=1930 */
/*28*/ {2425977.5, 2427803.5,    24.418,     0.052,    -0.449,     0.142}, /* ybeg=1930, yend=1935 */
/*29*/ {2427803.5, 2429629.5,    24.164,    -0.419,    -0.022,     0.702}, /* ybeg=1935, yend=1940 */
/*30*/ {2429629.5, 2431456.5,    24.426,     1.645,     2.086,    -1.106}, /* ybeg=1940, yend=1945 */
/*31*/ {2431456.5, 2433282.5,    27.050,     2.499,    -1.232,     0.614}, /* ybeg=1945, yend=1950 */
/*32*/ {2433282.5, 2434378.5,    28.932,     1.127,     0.220,    -0.277}, /* ybeg=1950, yend=1953 */
/*33*/ {2434378.5, 2435473.5,    30.002,     0.737,    -0.610,     0.631}, /* ybeg=1953, yend=1956 */
/*34*/ {2435473.5, 2436569.5,    30.760,     1.409,     1.282,    -0.799}, /* ybeg=1956, yend=1959 */
/*35*/ {2436569.5, 2437665.5,    32.652,     1.577,    -1.115,     0.507}, /* ybeg=1959, yend=1962 */
/*36*/ {2437665.5, 2438761.5,    33.621,     0.868,     0.406,     0.199}, /* ybeg=1962, yend=1965 */
/*37*/ {2438761.5, 2439856.5,    35.093,     2.275,     1.002,    -0.414}, /* ybeg=1965, yend=1968 */
/*38*/ {2439856.5, 2440952.5,    37.956,     3.035,    -0.242,     0.202}, /* ybeg=1968, yend=1971 */
/*39*/ {2440952.5, 2442048.5,    40.951,     3.157,     0.364,    -0.229}, /* ybeg=1971, yend=1974 */
/*40*/ {2442048.5, 2443144.5,    44.244,     3.198,    -0.323,     0.172}, /* ybeg=1974, yend=1977 */
/*41*/ {2443144.5, 2444239.5,    47.291,     3.069,     0.193,    -0.192}, /* ybeg=1977, yend=1980 */
/*42*/ {2444239.5, 2445335.5,    50.361,     2.878,    -0.384,     0.081}, /* ybeg=1980, yend=1983 */
/*43*/ {2445335.5, 2446431.5,    52.936,     2.354,    -0.140,    -0.166}, /* ybeg=1983, yend=1986 */
/*44*/ {2446431.5, 2447527.5,    54.984,     1.577,    -0.637,     0.448}, /* ybeg=1986, yend=1989 */
/*45*/ {2447527.5, 2448622.5,    56.373,     1.649,     0.709,    -0.277}, /* ybeg=1989, yend=1992 */
/*46*/ {2448622.5, 2449718.5,    58.453,     2.235,    -0.122,     0.111}, /* ybeg=1992, yend=1995 */
/*47*/ {2449718.5, 2450814.5,    60.677,     2.324,     0.212,    -0.315}, /* ybeg=1995, yend=1998 */
/*48*/ {2450814.5, 2451910.5,    62.899,     1.804,    -0.732,     0.112}, /* ybeg=1998, yend=2001 */
/*49*/ {2451910.5, 2453005.5,    64.082,     0.675,    -0.396,     0.193}, /* ybeg=2001, yend=2004 */
/*50*/ {2453005.5, 2454101.5,    64.555,     0.463,     0.184,    -0.008}, /* ybeg=2004, yend=2007 */
/*51*/ {2454101.5, 2455197.5,    65.194,     0.809,     0.161,    -0.101}, /* ybeg=2007, yend=2010 */
/*52*/ {2455197.5, 2456293.5,    66.063,     0.828,    -0.142,     0.168}, /* ybeg=2010, yend=2013 */
/*53*/ {2456293.5, 2457388.5,    66.917,     1.046,     0.360,    -0.282}, /* ybeg=2013, yend=2016 */
};
static double deltat_stephenson_etc_2016(double tjd, double tid_acc)
{
  double t, dt, Ygreg;
  int i, irec = -1;
  Ygreg = 2000.0 + (tjd - J2000)/365.2425;
  // after the year -720 get value from spline curve
  for (i = 0; i < NDTCF16; i++) {
    if (tjd < dtcf16[i][0]) break;
    if (tjd < dtcf16[i][1]) {
      irec = i;
      break;
    }
  }
  if (irec >= 0) {
    t = (tjd - dtcf16[irec][0]) / (dtcf16[irec][1] - dtcf16[irec][0]);
    dt = dtcf16[irec][2] + dtcf16[irec][3] * t + dtcf16[irec][4] * t * t + dtcf16[irec][5] * t * t * t;
  // for earlier epochs, use long term parabola
  } else if (Ygreg < -720) {
    t = (Ygreg - 1825) / 100.0;
    dt = -320 + 32.5 * t * t;
    dt -= 179.7337208; // to make curve continous on 1 Jan -720 (D. Koch)
  // future
  } else {
    t = (Ygreg - 1825) / 100.0;
    dt = -320 + 32.5 * t * t;
    dt += 269.4790417; // to make curve continous on 1 Jan 2016 (D. Koch)
  }
  /* The parameter adjust_after_1955 must be TRUE here, because the 
   * Stephenson 2016 curve is based on occultation data alone, 
   * not on IERS data.
   * Note, however, the current function deltat_stephenson_etc_2016()
   * is called only for dates before 1 Jan 1955. */
  dt = adjust_for_tidacc(dt, Ygreg, tid_acc, SE_TIDAL_STEPHENSON_2016, TRUE);
  dt /= 86400.0;
  return dt;
}

static double deltat_espenak_meeus_1620(double tjd, double tid_acc)
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
  ans = adjust_for_tidacc(ans, Ygreg, tid_acc, SE_TIDAL_26, FALSE);
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
if (!swed.init_dt_done) {
  swed.init_dt_done = TRUE;
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
static double adjust_for_tidacc(double ans, double Y, double tid_acc, double tid_acc0, AS_BOOL adjust_after_1955)
{
  double B;
  if( Y < 1955.0 || adjust_after_1955) {
    B = (Y - 1955.0);
    ans += -0.000091 * (tid_acc - tid_acc0) * B * B;
  }
  return ans;
}

/* returns tidal acceleration used in swe_deltat() and swe_deltat_ex() */
double CALL_CONV swe_get_tid_acc()
{
  return swed.tid_acc;
}

/* function sets tidal acceleration of the Moon.
 * t_acc can be either
 * - the value of the tidal acceleration in arcsec/cty^2
 *   of the Moon will be set consistent with that ephemeris.
 * - SE_TIDAL_AUTOMATIC, 
 */
void CALL_CONV swe_set_tid_acc(double t_acc)
{
  if (t_acc == SE_TIDAL_AUTOMATIC) {
    swed.tid_acc = SE_TIDAL_DEFAULT;
    swed.is_tid_acc_manual = FALSE;
    return;
  }
  swed.tid_acc = t_acc;
  swed.is_tid_acc_manual = TRUE;
}

void CALL_CONV swe_set_delta_t_userdef(double dt)
{
  if (dt == SE_DELTAT_AUTOMATIC) {
    swed.delta_t_userdef_is_set = FALSE; 
  } else {
    swed.delta_t_userdef_is_set = TRUE;
    swed.delta_t_userdef = dt;
  }
}

int32 swi_guess_ephe_flag()
{
  int32 iflag = SEFLG_SWIEPH;
  /* if jpl file is open, assume SEFLG_JPLEPH */
  if (swed.jpl_file_is_open) {
    iflag = SEFLG_JPLEPH;
  } else {
    iflag = SEFLG_SWIEPH;
  }
  return iflag;
}

int32 swi_get_tid_acc(double tjd_ut, int32 iflag, int32 denum, int32 *denumret, double *tid_acc, char *serr)
{
  double xx[6], tjd_et;
  iflag &= SEFLG_EPHMASK;
  if (swed.is_tid_acc_manual) {
    *tid_acc = swed.tid_acc;
    return iflag;
  }
  if (denum == 0) {
    if (iflag & SEFLG_MOSEPH) {
      *tid_acc = SE_TIDAL_DE404;
      *denumret = 404;
      return iflag;
    }
    if (iflag & SEFLG_JPLEPH) {
      if (swed.jpl_file_is_open) {
	denum = swed.jpldenum;
      } else {
	tjd_et = tjd_ut; /* + swe_deltat_ex(tjd_ut, 0, NULL); we do not add 
	                    delta t, because it would result in a recursive 
			    call of swi_set_tid_acc() */
	iflag = SEFLG_JPLEPH|SEFLG_J2000|SEFLG_TRUEPOS|SEFLG_ICRS|SEFLG_BARYCTR;
	iflag = swe_calc(tjd_et, SE_JUPITER, iflag, xx, serr);
	if (swed.jpl_file_is_open && (iflag & SEFLG_JPLEPH)) {
	  denum = swed.jpldenum;
	}
      }
    }
    /* SEFLG_SWIEPH wanted or SEFLG_JPLEPH failed: */
    if (denum == 0) {
      tjd_et = tjd_ut; /* + swe_deltat_ex(tjd_ut, 0, NULL); we do not add 
                          delta t, because it would result in a recursive 
			  call of swi_set_tid_acc() */
      if (swed.fidat[SEI_FILE_MOON].fptr == NULL ||
          tjd_et < swed.fidat[SEI_FILE_MOON].tfstart + 1 ||
	  tjd_et > swed.fidat[SEI_FILE_MOON].tfend - 1) {
	iflag = SEFLG_SWIEPH|SEFLG_J2000|SEFLG_TRUEPOS|SEFLG_ICRS;
	iflag = swe_calc(tjd_et, SE_MOON, iflag, xx, serr);
      }
      if (swed.fidat[SEI_FILE_MOON].fptr != NULL) {
	denum = swed.fidat[SEI_FILE_MOON].sweph_denum;
      /* Moon ephemeris file is not available, default to Moshier ephemeris */
      } else {
	denum = 404; /* DE number of Moshier ephemeris */
      }
    }
  }
  switch(denum) {
    case 200: *tid_acc = SE_TIDAL_DE200; break;
    case 403: *tid_acc = SE_TIDAL_DE403; break;
    case 404: *tid_acc = SE_TIDAL_DE404; break;
    case 405: *tid_acc = SE_TIDAL_DE405; break;
    case 406: *tid_acc = SE_TIDAL_DE406; break;
    case 421: *tid_acc = SE_TIDAL_DE421; break;
    case 422: *tid_acc = SE_TIDAL_DE422; break;
    case 430: *tid_acc = SE_TIDAL_DE430; break;
    case 431: *tid_acc = SE_TIDAL_DE431; break;
    default: denum = SE_DE_NUMBER; *tid_acc = SE_TIDAL_DEFAULT; break;
  }
  *denumret = denum;
  iflag &= SEFLG_EPHMASK;
  return iflag;
}

int32 swi_set_tid_acc(double tjd_ut, int32 iflag, int32 denum, char *serr)
{
  int32 retc = iflag;
  int32 denumret;
  /* manual tid_acc overrides automatic tid_acc */
  if (swed.is_tid_acc_manual)
    return retc;
  retc = swi_get_tid_acc(tjd_ut, iflag, denum, &denumret, &(swed.tid_acc), serr);
#if TRACE
  swi_open_trace(NULL);
  if (swi_trace_count < TRACE_COUNT_MAX) {
    if (swi_fp_trace_c != NULL) {
      fputs("\n/*SWE_SET_TID_ACC*/\n", swi_fp_trace_c);
      fprintf(swi_fp_trace_c, "  t = %.9f;\n", swed.tid_acc);
      fprintf(swi_fp_trace_c, "  swe_set_tid_acc(t);\n");
      fputs("  printf(\"swe_set_tid_acc: %f\\t\\n\", ", swi_fp_trace_c);
      fputs("t);\n", swi_fp_trace_c);
      fflush(swi_fp_trace_c);
    }
    if (swi_fp_trace_out != NULL) {
      fprintf(swi_fp_trace_out, "swe_set_tid_acc: %f\t\n", swed.tid_acc);
      fflush(swi_fp_trace_out);
    }
  }
#endif
  return retc;
}

/*
 * The time range of DE431 requires a new calculation of sidereal time that 
 * gives sensible results for the remote past and future.
 * The algorithm is based on the formula of the mean earth by Simon & alii,
 * "Precession formulae and mean elements for the Moon and the Planets",
 * A&A 282 (1994), p. 675/678.
 * The longitude of the mean earth relative to the mean equinox J2000
 * is calculated and then precessed to the equinox of date, using the
 * default precession model of the Swiss Ephmeris. Afte that,
 * sidereal time is derived.
 * The algoritm provides exact agreement for epoch 1 Jan. 2003 with the 
 * definition of sidereal time as given in the IERS Convention 2010.
 */
/*#define SIDT_LTERM   TRUE
#if SIDT_LTERM*/
static double sidtime_long_term(double tjd_ut, double eps, double nut)
{
  double tsid = 0, tjd_et;
  double dlon, xs[6], xobl[6], dhour, nutlo[2];
  double dlt = AUNIT / CLIGHT / 86400.0;
  double t, t2, t3;
  tjd_et = tjd_ut + swe_deltat_ex(tjd_ut, -1, NULL);
  t = (tjd_et - J2000) / 365250.0;
  t2 = t * t; t3 = t * t2;
  /* mean longitude of earth J2000 */
  dlon = 100.46645683 + (1295977422.83429 * t - 2.04411 * t2 - 0.00523 * t3) / 3600.0;
  /* light time sun-earth */
  dlon = swe_degnorm(dlon - dlt * 360.0 / 365.2425);
  xs[0] = dlon * DEGTORAD; xs[1] = 0; xs[2] = 1;
  /* to mean equator J2000, cartesian */
  xobl[0] = 23.45; xobl[1] = 23.45;
  xobl[1] = swi_epsiln(J2000 + swe_deltat_ex(J2000, -1, NULL), 0) * RADTODEG;
  swi_polcart(xs, xs);
  swi_coortrf(xs, xs, -xobl[1] * DEGTORAD);
  /* precess to mean equinox of date */
  swi_precess(xs, tjd_et, 0, -1);
  /* to mean equinox of date */
  xobl[1] = swi_epsiln(tjd_et, 0) * RADTODEG;
  swi_nutation(tjd_et, 0, nutlo);
  xobl[0] = xobl[1] + nutlo[1] * RADTODEG;
  xobl[2] = nutlo[0] * RADTODEG;
  swi_coortrf(xs, xs, xobl[1] * DEGTORAD);
  swi_cartpol(xs, xs);
  xs[0] *= RADTODEG;
  dhour = fmod(tjd_ut - 0.5, 1) * 360;
  /* mean to true (if nut != 0) */ 
  if (eps == 0)
    xs[0] += xobl[2] * cos(xobl[0] * DEGTORAD);
  else
    xs[0] += nut * cos(eps * DEGTORAD);
  /* add hour */
  xs[0] = swe_degnorm(xs[0] + dhour);
  tsid = xs[0] / 15;
  return tsid;
}
/*#endif*/

/* Apparent Sidereal Time at Greenwich with equation of the equinoxes
 *  ERA-based expression for for Greenwich Sidereal Time (GST) based 
 *  on the IAU 2006 precession and IAU 2000A_R06 nutation 
 *  ftp://maia.usno.navy.mil/conv2010/chapter5/tab5.2e.txt
 *
 * returns sidereal time in hours.
 *
 * program returns sidereal hours since sidereal midnight 
 * tjd 		julian day UT
 * eps 		obliquity of ecliptic, degrees 
 * nut 		nutation, degrees 
 */
/*  C'_{s,j})_i     C'_{c,j})_i */
#define SIDTNTERM 33
static const double stcf[SIDTNTERM * 2] = {
2640.96,-0.39,
63.52,-0.02,
11.75,0.01,
11.21,0.01,
-4.55,0.00,
2.02,0.00,
1.98,0.00,
-1.72,0.00,
-1.41,-0.01,
-1.26,-0.01,
-0.63,0.00,
-0.63,0.00,
0.46,0.00,
0.45,0.00,
0.36,0.00,
-0.24,-0.12,
0.32,0.00,
0.28,0.00,
0.27,0.00,
0.26,0.00,
-0.21,0.00,
0.19,0.00,
0.18,0.00,
-0.10,0.05,
0.15,0.00,
-0.14,0.00,
0.14,0.00,
-0.14,0.00,
0.14,0.00,
0.13,0.00,
-0.11,0.00,
0.11,0.00,
0.11,0.00,
};
#define SIDTNARG 14
/* l    l'   F    D   Om   L_Me L_Ve L_E  L_Ma L_J  L_Sa L_U  L_Ne p_A*/
static const int stfarg[SIDTNTERM * SIDTNARG] = {
   0,   0,   0,   0,   1,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   2,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   2,  -2,   3,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   2,  -2,   1,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   2,  -2,   2,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   2,   0,   3,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   2,   0,   1,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   3,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   1,   0,   0,   1,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   1,   0,   0,  -1,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   1,   0,   0,   0,  -1,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   1,   0,   0,   0,   1,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   1,   2,  -2,   3,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   1,   2,  -2,   1,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   4,  -4,   4,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   1,  -1,   1,   0,  -8,  12,   0,   0,   0,   0,   0,   0,
   0,   0,   2,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   2,   0,   2,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   1,   0,   2,   0,   3,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   1,   0,   2,   0,   1,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   2,  -2,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   1,  -2,   2,  -3,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   1,  -2,   2,  -1,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   8, -13,   0,   0,   0,   0,   0,  -1,
   0,   0,   0,   2,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   2,   0,  -2,   0,  -1,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   1,   0,   0,  -2,   1,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   1,   2,  -2,   2,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   1,   0,   0,  -2,  -1,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   4,  -2,   4,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   2,  -2,   4,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   1,   0,  -2,   0,  -3,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   1,   0,  -2,   0,  -1,   0,   0,   0,   0,   0,   0,   0,   0,   0,
};
static double sidtime_non_polynomial_part(double tt)
{
  int i, j;
  double delm[SIDTNARG];
  double dadd, darg;
  /* L Mean anomaly of the Moon.*/
  delm[0] = swe_radnorm(2.35555598 + 8328.6914269554 * tt);
  /* LSU Mean anomaly of the Sun.*/
  delm[1] = swe_radnorm(6.24006013 + 628.301955 * tt);
  /* F Mean argument of the latitude of the Moon. */
  delm[2] = swe_radnorm(1.627905234 + 8433.466158131 * tt);
  /* D Mean elongation of the Moon from the Sun. */
  delm[3] = swe_radnorm(5.198466741 + 7771.3771468121 * tt);
  /* OM Mean longitude of the ascending node of the Moon. */
  delm[4] = swe_radnorm(2.18243920 - 33.757045 * tt);
  /* Planetary longitudes, Mercury through Neptune (Souchay et al. 1999). 
   * LME, LVE, LEA, LMA, LJU, LSA, LUR, LNE */
  delm[5] = swe_radnorm(4.402608842 + 2608.7903141574 * tt);
  delm[6] = swe_radnorm(3.176146697 + 1021.3285546211 * tt);
  delm[7] = swe_radnorm(1.753470314 +  628.3075849991 * tt);
  delm[8] = swe_radnorm(6.203480913 +  334.0612426700 * tt);
  delm[9] = swe_radnorm(0.599546497 +   52.9690962641 * tt);
  delm[10] = swe_radnorm(0.874016757 +   21.3299104960 * tt);
  delm[11] = swe_radnorm(5.481293871 +    7.4781598567 * tt);
  delm[12] = swe_radnorm(5.321159000 +    3.8127774000 * tt);
  /* PA General accumulated precession in longitude. */
  delm[13] = (0.02438175 + 0.00000538691 * tt) * tt;
  dadd = -0.87 * sin(delm[4]) * tt;
  for (i = 0; i < SIDTNTERM; i++) {
    darg = 0;
    for (j = 0; j < SIDTNARG; j++) {
      darg += stfarg[i * SIDTNARG + j] * delm[j];
    }
    dadd += stcf[i * 2] * sin(darg) + stcf[i * 2 + 1] * cos(darg);
  }
  dadd /= (3600.0 * 1000000.0);
  return dadd;
}

/*
 * SEMOD_SIDT_IAU_2006 
 * N. Capitaine, P.T. Wallace, and J. Chapront, "Expressions for IAU 2000
 * precession quantities", 2003, A&A 412, 567-586 (2003), p. 582.
 * This is a "short" term model, that can be combined with other models
 */
/*#define SIDT_IERS_CONV_2010 TRUE*/
/* sidtime_long_term() is not used between the following two dates */
#define SIDT_LTERM_T0  2396758.5  /* 1 Jan 1850  */
#define SIDT_LTERM_T1  2469807.5  /* 1 Jan 2050  */
#define SIDT_LTERM_OFS0   (0.000378172 / 15.0)
#define SIDT_LTERM_OFS1   (0.001385646 / 15.0)
double CALL_CONV swe_sidtime0(double tjd, double eps, double nut)
{
  double jd0;    	/* Julian day at midnight Universal Time */
  double secs;   	/* Time of day, UT seconds since UT midnight */
  double eqeq, jd, tu, tt, msday, jdrel;
  double gmst, dadd;
  int prec_model_short = swed.astro_models[SE_MODEL_PREC_SHORTTERM];
  int sidt_model = swed.astro_models[SE_MODEL_SIDT];
  if (prec_model_short == 0) prec_model_short = SEMOD_PREC_DEFAULT_SHORT;
  if (sidt_model == 0) sidt_model = SEMOD_SIDT_DEFAULT;
  swi_init_swed_if_start();
  if (sidt_model == SEMOD_SIDT_LONGTERM) {
    if (tjd <= SIDT_LTERM_T0 || tjd >= SIDT_LTERM_T1) {
      gmst = sidtime_long_term(tjd, eps, nut);
      if (tjd <= SIDT_LTERM_T0)
	gmst -= SIDT_LTERM_OFS0;
      else if (tjd >= SIDT_LTERM_T1)
	gmst -= SIDT_LTERM_OFS1;
      if (gmst >= 24) gmst -= 24;
      if (gmst < 0) gmst += 24;
      goto sidtime_done;
    }
  }
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
  if (sidt_model == SEMOD_SIDT_IERS_CONV_2010 || sidt_model == SEMOD_SIDT_LONGTERM) {
    /*  ERA-based expression for Greenwich Sidereal Time (GST) based 
     *  on the IAU 2006 precession */
    jdrel = tjd - J2000;
    tt = (tjd + swe_deltat_ex(tjd, -1, NULL) - J2000) / 36525.0;
    gmst = swe_degnorm((0.7790572732640 + 1.00273781191135448 * jdrel) * 360);
    gmst += (0.014506 + tt * (4612.156534 +  tt * (1.3915817 + tt * (-0.00000044 + tt * (-0.000029956 + tt * -0.0000000368))))) / 3600.0;
    dadd = sidtime_non_polynomial_part(tt);
    gmst = swe_degnorm(gmst + dadd);
    /*printf("gmst iers=%f \n", gmst);*/
    gmst = gmst / 15.0 * 3600.0;
  /* sidt_model == SEMOD_SIDT_IAU_2006, older standards according to precession model */
  } else if (sidt_model == SEMOD_SIDT_IAU_2006) {
    tt = (jd0 + swe_deltat_ex(jd0, -1, NULL) - J2000)/36525.0; /* TT in centuries after J2000 */
    gmst = (((-0.000000002454*tt - 0.00000199708)*tt - 0.0000002926)*tt + 0.092772110)*tt*tt + 307.4771013*(tt-tu) + 8640184.79447825*tu + 24110.5493771;
    /* mean solar days per sidereal day at date tu;
     * for the derivative of gmst, we can assume UT1 =~ TT */
    msday = 1 + ((((-0.000000012270*tt - 0.00000798832)*tt - 0.0000008778)*tt + 0.185544220)*tt + 8640184.79447825)/(86400.*36525.);
    gmst += msday * secs;
  /* SEMOD_SIDT_IAU_1976 */
  } else {  /* IAU 1976 formula */
      /* Greenwich Mean Sidereal Time at 0h UT of date */
    gmst = (( -6.2e-6*tu + 9.3104e-2)*tu + 8640184.812866)*tu + 24110.54841;
    /* mean solar days per sidereal day at date tu, = 1.00273790934 in 1986 */
    msday = 1.0 + ((-1.86e-5*tu + 0.186208)*tu + 8640184.812866)/(86400.*36525.);
    gmst += msday * secs;
  }
  /* Local apparent sidereal time at given UT at Greenwich */
  eqeq = 240.0 * nut * cos(eps * DEGTORAD);
  gmst = gmst + eqeq  /* + 240.0*tlong */;
  /* Sidereal seconds modulo 1 sidereal day */
  gmst = gmst - 86400.0 * floor( gmst/86400.0 );
  /* return in hours */
  gmst /= 3600;
  goto sidtime_done;
sidtime_done:
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

void CALL_CONV swe_set_interpolate_nut(AS_BOOL do_interpolate)
{
  if (swed.do_interpolate_nut == do_interpolate)
    return;
  if (do_interpolate) 
    swed.do_interpolate_nut = TRUE;
  else
    swed.do_interpolate_nut = FALSE;
  swed.interpol.tjd_nut0 = 0;
  swed.interpol.tjd_nut2 = 0;
  swed.interpol.nut_dpsi0 = 0;
  swed.interpol.nut_dpsi1 = 0;
  swed.interpol.nut_dpsi2 = 0;
  swed.interpol.nut_deps0 = 0;
  swed.interpol.nut_deps1 = 0;
  swed.interpol.nut_deps2 = 0;
}

/* sidereal time, without eps and nut as parameters.
 * tjd must be UT !!!
 * for more informsation, see comment with swe_sidtime0()
 */
double CALL_CONV swe_sidtime(double tjd_ut)
{
  int i;
  double eps, nutlo[2], tsid;
  double tjde;
  /* delta t adjusted to default tidal acceleration of the moon */
  tjde = tjd_ut + swe_deltat_ex(tjd_ut, -1, NULL); 
  swi_init_swed_if_start();
  eps = swi_epsiln(tjde, 0) * RADTODEG;
  swi_nutation(tjde, 0, nutlo);
  for (i = 0; i < 2; i++)
    nutlo[i] *= RADTODEG;
  tsid = swe_sidtime0(tjd_ut, eps + nutlo[1], nutlo[0]);
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
      sprintf(fname, sform, (ipli - SE_AST_OFFSET) / 1000, DIR_GLUE, ipli - SE_AST_OFFSET, SE_FILE_SUFFIX);
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
static TLS uint32 crc32_table[256];
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
centisec CALL_CONV swe_csnorm(centisec p)
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
centisec CALL_CONV swe_difcsn (centisec p1, centisec p2)
{ 
  return (swe_csnorm(p1 - p2));
}

double CALL_CONV swe_difdegn (double p1, double p2)
{ 
  return (swe_degnorm(p1 - p2));
}

/************************************
distance in centisecs p1 - p2
normalized to [-180..180[
**************************************/
centisec CALL_CONV swe_difcs2n(centisec p1, centisec p2)
{ centisec dif;
  dif = swe_csnorm(p1 - p2);
  if (dif  >= DEG180) return (dif - DEG360);
  return (dif);
}

double CALL_CONV swe_difdeg2n(double p1, double p2)
{ double dif;
  dif = swe_degnorm(p1 - p2);
  if (dif  >= 180.0) return (dif - 360.0);
  return (dif);
}

double CALL_CONV swe_difrad2n(double p1, double p2)
{ double dif;
  dif = swe_radnorm(p1 - p2);
  if (dif  >= TWOPI / 2) return (dif - TWOPI);
  return (dif);
}

/*************************************
round second, but at 29.5959 always down
*************************************/ 
centisec CALL_CONV swe_csroundsec(centisec x)	
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
int32 CALL_CONV swe_d2l(double x)		
{
  if (x >=0)
    return ((int32) (x + 0.5));
  else
    return (- (int32) (0.5 - x));
}

/*
 * monday = 0, ... sunday = 6
 */
int CALL_CONV swe_day_of_week(double jd)
{
  return (((int) floor (jd - 2433282 - 1.5) %7) + 7) % 7;
}

char *CALL_CONV swe_cs2timestr(CSEC t, int sep, AS_BOOL suppressZero, char *a)
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

char *CALL_CONV swe_cs2lonlatstr(CSEC t, char pchar, char mchar, char *sp)
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

char *CALL_CONV swe_cs2degstr(CSEC t, char *a)
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

/******************************************************************
 * decimal degrees in zodiac to nakshatra position, deg, min, sec *
 * for definition of input see function swe_split_deg().
 * output:
 * ideg 	degrees, 
 * imin 	minutes, 
 * isec 	seconds, 
 * dsecfr	fraction of seconds 
 * inak	nakshatra number; 
 ******************************************************************/
static void split_deg_nakshatra(double ddeg, int32 roundflag, int32 *ideg, int32 *imin, int32 *isec, double *dsecfr, int32 *inak)
{
  double dadd = 0;
  double dnakshsize = 13.33333333333333;
  double ddeghelp = fmod(ddeg, dnakshsize);
  *inak = 1;
  if (ddeg < 0) {
    *inak = -1;
    ddeg = 0;
  }
  // Sheoran "Vedic" ayanamsha: 0 Aries = 3°20 Ashvini
  if (swed.sidd.sid_mode & SE_SIDM_TRUE_SHEORAN)
    ddeg = swe_degnorm(ddeg + 3.33333333333333);
  if (roundflag & SE_SPLIT_DEG_ROUND_DEG) {
    dadd = 0.5;
  } else if (roundflag & SE_SPLIT_DEG_ROUND_MIN) {
    dadd = 0.5 / 60;
  } else if (roundflag & SE_SPLIT_DEG_ROUND_SEC) {
    dadd = 0.5 / 3600;
  }
  if (roundflag & SE_SPLIT_DEG_KEEP_DEG) {
    if ((int32) (ddeghelp + dadd) - (int32) ddeghelp > 0)
      dadd = 0;
  } else if (roundflag & SE_SPLIT_DEG_KEEP_SIGN) {
    if (ddeghelp + dadd >= dnakshsize)
      dadd = 0;
  }
  ddeg += dadd;
  *inak = (int32) (ddeg / dnakshsize);
  ddeg = fmod(ddeg, dnakshsize);
  *ideg = (int32) ddeg;
  ddeg -= *ideg;
  *imin = (int32) (ddeg * 60);
  ddeg -= *imin / 60.0;
  *isec = (int32) (ddeg * 3600);
  if (!(roundflag & (SE_SPLIT_DEG_ROUND_DEG | SE_SPLIT_DEG_ROUND_MIN | SE_SPLIT_DEG_ROUND_SEC))) {
    *dsecfr = ddeg * 3600 - *isec;
  }
}  /* end split_deg_nakshtra */

/************************************************************
 * splitting decimal degrees into (zod.sign,) deg, min, sec. *
 * input:
 * ddeg 	decimal degrees, ecliptic longitude
 * roundflag    by default there is no rounding. if rounding is 
 *              required, the following bits can be set:
# define SE_SPLIT_DEG_ROUND_SEC    1
# define SE_SPLIT_DEG_ROUND_MIN    2
# define SE_SPLIT_DEG_ROUND_DEG    4
# define SE_SPLIT_DEG_ZODIACAL     8   * split into zodiac signs
# define SE_SPLIT_DEG_NAKSHATRA 1024   * split into nakshatras *
# define SE_SPLIT_DEG_KEEP_SIGN   16   * don't round to next zodiac sign,
                                       * e.g. 29.9999998 will be rounded
				       * to 29°59'59" (or 29°59' or 29°) * 
				       * or next nakshatra:
                                       * e.g. 13.3333332 will be rounded
				       * to 13°19'59" (or 13°19' or 13°) * 
# define SE_SPLIT_DEG_KEEP_DEG    32   * don't round to next degree
                                       * e.g. 10.9999999 will be rounded
				       * to 10d59'59" (or 10d59' or 10d) * 
 * output:
 *  ideg 	degrees, 
 *  imin 	minutes, 
 *  isec 	seconds, 
 *  dsecfr	fraction of seconds 
 *  isgn	zodiac sign number; 
 *              or +/- sign
 *  
 *********************************************************/
void CALL_CONV swe_split_deg(double ddeg, int32 roundflag, int32 *ideg, int32 *imin, int32 *isec, double *dsecfr, int32 *isgn)
{
  double dadd = 0;
  *isgn = 1;
  if (ddeg < 0) {
    *isgn = -1;
    ddeg = -ddeg;
  } else if (roundflag & SE_SPLIT_DEG_NAKSHATRA) {
    split_deg_nakshatra(ddeg, roundflag, ideg, imin, isec, dsecfr, isgn);
    return;
  }
  if (roundflag & SE_SPLIT_DEG_ROUND_DEG) {
    dadd = 0.5;
  } else if (roundflag & SE_SPLIT_DEG_ROUND_MIN) {
    dadd = 0.5 / 60.0;
  } else if (roundflag & SE_SPLIT_DEG_ROUND_SEC) {
    dadd = 0.5 / 3600.0;
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
  AS_BOOL correct_speed = TRUE;
  if (xp[0] == 0 && xp[1] == 0 && xp[2] == 0)
    return;
  /* with zero speed, we assume that it should be really zero */
  if (xp[3] == 0)
    correct_speed = FALSE;
  swi_cartpol_sp(xp, xp);
  /* according to Expl.Suppl., p. 167f. */
  xp[0] += (0.035 + 0.085 * (tjd - B1950) / 36524.2198782) / 3600 * 15 * DEGTORAD;
  if (correct_speed)
    xp[3] += (0.085 / 36524.2198782) / 3600 * 15 * DEGTORAD;
  swi_polcart_sp(xp, xp);
}

void swi_FK5_FK4(double *xp, double tjd)
{
  if (xp[0] == 0 && xp[1] == 0 && xp[2] == 0)
    return;
  swi_cartpol_sp(xp, xp);
  /* according to Expl.Suppl., p. 167f. */
  xp[0] -= (0.035 + 0.085 * (tjd - B1950) / 36524.2198782) / 3600 * 15 * DEGTORAD;
  xp[3] -= (0.085 / 36524.2198782) / 3600 * 15 * DEGTORAD;
  swi_polcart_sp(xp, xp);
}

/* function for inhouse testing only */
void set_astro_models(char *samod)
{
  int *pmodel = &(swed.astro_models[0]);
  char *sp, *sp2;
  int i = 0;
  swi_init_swed_if_start();
  sp = samod;
  pmodel[0] = atoi(sp);
  i++;
  while((sp2 = strchr(sp, ',')) != NULL && i < NSE_MODELS) {
    sp = sp2 + 1;
    pmodel[i] = atoi(sp);
    i++;
  } 
}


/*
 * Function for inhouse testing of old SE versions.
 *
 * Values of the following macros are defined in swephexp.h 
 * and must be identical to the ones used in the defines below 
D1 SEMOD_DELTAT_STEPHENSON_MORRISON_1984
D2 SEMOD_DELTAT_STEPHENSON_1997
D3 SEMOD_DELTAT_STEPHENSON_MORRISON_2004
D4 SEMOD_DELTAT_ESPENAK_MEEUS_2006
D5 SEMOD_DELTAT_STEPHENSON_ETC_2016

P1 SEMOD_PREC_IAU_1976
P2 SEMOD_PREC_LASKAR_1986
P3 SEMOD_PREC_WILL_EPS_LASK
P4 SEMOD_PREC_WILLIAMS_1994
P5 SEMOD_PREC_SIMON_1994
P6 SEMOD_PREC_IAU_2000
P7 SEMOD_PREC_BRETAGNON_2003
P8 SEMOD_PREC_IAU_2006
P9 SEMOD_PREC_VONDRAK_2011

N1 SEMOD_NUT_IAU_1980
N2 SEMOD_NUT_IAU_CORR_1987
N3 SEMOD_NUT_IAU_2000A
N4 SEMOD_NUT_IAU_2000B

B1 SEMOD_BIAS_NONE
B2 SEMOD_BIAS_IAU2000
B3 SEMOD_BIAS_IAU2006

S1 SEMOD_SIDT_IAU_1976
S2 SEMOD_SIDT_IAU_2006
S3 SEMOD_SIDT_IERS_CONV_2010
S4 SEMOD_SIDT_LONGTERM
 *                           D P P N B J J S 
 */
# define AMODELS_SE_1_00    "1,3,1,1,1,0,0,1"
# define AMODELS_SE_1_64    "2,3,1,1,1,0,0,1"
# define AMODELS_SE_1_70    "2,8,8,4,2,0,0,2"
# define AMODELS_SE_1_72    "3,8,8,4,2,0,0,2"
# define AMODELS_SE_1_77    "4,8,8,4,2,0,0,2"
# define AMODELS_SE_1_78    "4,9,9,4,2,0,0,2"
# define AMODELS_SE_1_80    "4,9,9,4,3,0,0,1"  /* note sid. time (S)! */
# define AMODELS_SE_2_00    "4,9,9,4,3,0,0,4"
# define AMODELS_SE_2_06    "5,9,9,4,3,0,0,4"
void CALL_CONV swe_set_astro_models(char *samod, int32 iflag)
{
  double dversion;
  char s[30], *sp;
  swi_init_swed_if_start();
  if (*samod != '\0' && isdigit(*samod)) {
    set_astro_models(samod);
  } else if (*samod == '\0' || strncmp(samod, "SE", 2) == 0) {
    strncpy(s, samod, 20);
    s[20] = '\0';
    if ((sp = strchr(s + 5, '.')) != NULL)  // remove second '.' in "SE2.05.01"
      swi_strcpy(sp, sp+1);
    if ((sp = strchr(s + 5, 'b')) != NULL)  // remove 'b' in "SE2.05.02b04"
      swi_strcpy(sp, sp+1);
    dversion = atof(s + 2);
    if (dversion == 0)
      dversion = atof(SE_VERSION);
    if (dversion >= 2.06) {
      set_astro_models(AMODELS_SE_2_06);
    } else if (dversion >= 2.01) {
      set_astro_models(AMODELS_SE_2_00);
    } else if (dversion >= 2.00) {
      set_astro_models(AMODELS_SE_2_00);
      if (swi_get_denum(SEI_SUN, iflag) == 431) 
        swe_set_tid_acc(SE_TIDAL_DE406);
    } else if (dversion >= 1.80) {
      set_astro_models(AMODELS_SE_1_80);
      swe_set_tid_acc(SE_TIDAL_DE406);
    } else if (dversion >= 1.78) {
      set_astro_models(AMODELS_SE_1_78);
      swe_set_tid_acc(SE_TIDAL_DE406);
    } else if (dversion >= 1.77) {
      set_astro_models(AMODELS_SE_1_77);
      swe_set_tid_acc(SE_TIDAL_DE406);
    } else if (dversion >= 1.72) {
      set_astro_models(AMODELS_SE_1_72);
      swe_set_tid_acc(-25.7376);
    } else if (dversion >= 1.70) {
      set_astro_models(AMODELS_SE_1_70);
      swe_set_tid_acc(-25.7376);
    } else if (dversion >= 1.64) {
      set_astro_models(AMODELS_SE_1_64);
      swe_set_tid_acc(-25.7376);
    } else {
      set_astro_models(AMODELS_SE_1_00);
      swe_set_tid_acc(-25.7376);
    }
  }
}

/* function for inhouse testing only */
static void get_precession_model(int precmod, int32 iflag, char *s)
{
  if (precmod == 0)
    precmod = SEMOD_PREC_DEFAULT;
  if (iflag & SEFLG_JPLEPH) {
    if (iflag & SEFLG_JPLHOR) {
      strcpy(s, "IAU 1976 (Lieske) / Owen 1990 before 1799");
      return;
    }
    if (iflag & SEFLG_JPLHOR_APPROX) {
      strcpy(s, "Vondrak 2011 / IAU 1976 (Lieske) before 1962 / Owen 1990 before 1799");
      return;
    }
  }
  switch(precmod) {
    case SEMOD_PREC_IAU_1976:
      strcpy(s, "IAU 1976 (Lieske)");
      break;
    case SEMOD_PREC_IAU_2000:
      strcpy(s, "IAU 2000 (Lieske 1976, Mathews 2002)");
      break;
    case SEMOD_PREC_IAU_2006:
      strcpy(s, "IAU 2006 (Capitaine & alii)");
      break;
    case SEMOD_PREC_BRETAGNON_2003:
      strcpy(s, "Bretagnon 2003");
      break;
    case SEMOD_PREC_LASKAR_1986:
      strcpy(s, "Laskar 1986");
      break;
    case SEMOD_PREC_SIMON_1994:
      strcpy(s, "Simon 1994");
      break;
    case SEMOD_PREC_WILLIAMS_1994:
      strcpy(s, "Williams 1994");
      break;
    case SEMOD_PREC_WILL_EPS_LASK:
      strcpy(s, "Williams 1994 / Epsilon Laskar 1986");
      break;
    case SEMOD_PREC_OWEN_1990:
      strcpy(s, "Owen 1990");
      break;
    case SEMOD_PREC_VONDRAK_2011:
      strcpy(s, "Vondrák 2011");
      break;
    default:
      break;
  }
}

/* function for inhouse testing only */
static void get_deltat_model(int dtmod, char *s)
{
  if (dtmod == 0)
    dtmod = SEMOD_DELTAT_DEFAULT;
  switch(dtmod) {
    case SEMOD_DELTAT_ESPENAK_MEEUS_2006:
    strcpy(s, "Espenak/Meeus 2006 (before 1633)");
    break;
    case SEMOD_DELTAT_STEPHENSON_MORRISON_2004:
    strcpy(s, "Stephenson/Morrison 2004 (before 1600)");
    break;
    case SEMOD_DELTAT_STEPHENSON_1997:  
    strcpy(s, "Stephenson 1997 (before 1600)");
    break;
    case SEMOD_DELTAT_STEPHENSON_MORRISON_1984:
    strcpy(s, "Stephenson/Morrison 1984 (before 1600)");
    break;
    case SEMOD_DELTAT_STEPHENSON_ETC_2016:  
    strcpy(s, "Stephenson/Morrison/Hohenkerk 2016 (before 1955)");
    break;
  }
}

/* function for inhouse testing only */
static void get_nutation_model(int nutmod, int32 iflag, char *s)
{
  int jplhormod = swed.astro_models[SE_MODEL_JPLHOR_MODE];
  int jplhoramod = swed.astro_models[SE_MODEL_JPLHORA_MODE];
  if (jplhormod == 0)
    jplhormod = SEMOD_JPLHOR_DEFAULT;
  if (jplhoramod == 0)
    jplhoramod = SEMOD_JPLHORA_DEFAULT;
  if (nutmod == 0)
    nutmod = SEMOD_NUT_DEFAULT;
  switch(nutmod) {
    case SEMOD_NUT_IAU_1980:
    strcpy(s, "IAU 1980 (Wahr)");
    break;
    case SEMOD_NUT_IAU_CORR_1987:
    strcpy(s, "Herring 1986");
    break;
    case SEMOD_NUT_IAU_2000A:
    strcpy(s, "IAU 2000A (Mathews)");
    break;
    case SEMOD_NUT_IAU_2000B:
    strcpy(s, "IAU 2000B (Mathews)");
    break;
  }
  if (iflag & SEFLG_JPLEPH) {
    if (iflag & SEFLG_JPLHOR) 
      strcpy(s, "IAU 1980 (Wahr)");
    if (iflag & SEFLG_JPLHOR) {
      strcat(s, "\n+ daily corrections to dpsi/deps 1962-today");
      if (jplhormod == SEMOD_JPLHOR_LONG_AGREEMENT)
        strcat(s, "\n  good agreement with JPL Horizons between 1800 and today");
      else
        strcat(s, "\n  defaults to SEFLG_JPLEPH_APPROX before 1962");
    } else if (iflag & SEFLG_JPLHOR_APPROX){
      strcat(s, "\n+ some corrections, approximating JPL Horizons");
      if (jplhoramod == SEMOD_JPLHORA_1)
        strcat(s, " (SEMOD_JPLHORA_1)");
      else if (jplhoramod == SEMOD_JPLHORA_2)
        strcat(s, " (SEMOD_JPLHORA_2)");
      else
        strcat(s, " (SEMOD_JPLHORA_3)");
    }
  }
}

/* function for inhouse testing only */
static void get_frame_bias_model(int biasmod, char *s)
{
  if (biasmod == 0)
    biasmod = SEMOD_BIAS_DEFAULT;
  switch(biasmod) {
    case SEMOD_BIAS_IAU2000:
    strcpy(s, "IAU 2000");
    break;
    case SEMOD_BIAS_IAU2006:
    strcpy(s, "IAU 2006");
    break;
    case SEMOD_BIAS_NONE:
    strcpy(s, "none");
    break;
  }
}

/* function for inhouse testing only */
static void get_sidt_model(int sidtmod, char *s)
{
  if (sidtmod == 0)
    sidtmod = SEMOD_SIDT_DEFAULT;
  switch(sidtmod) {
    case SEMOD_SIDT_IAU_1976:
    strcpy(s, "IAU 1976");
    break;
    case SEMOD_SIDT_IAU_2006:
    strcpy(s, "IAU 2006 (Capitaine 2003)");
    break;
    case SEMOD_SIDT_IERS_CONV_2010:
    strcpy(s, "IERS Convention 2010");
    break;
    case SEMOD_SIDT_LONGTERM:
    strcpy(s, "IERS Convention 2010 + long-term extension by Astrodienst");
    break;
  }
}

/* function for inhouse testing only */
void CALL_CONV swe_get_astro_models(char *samod, char *sdet, int32 iflag)
{
  int i, imod;
  int *pmodel = &(swed.astro_models[0]);
  char s[AS_MAXCH], samod0[AS_MAXCH];
  AS_BOOL list_all_models = FALSE;
  if (samod != NULL) {
    if (strchr(samod, '+') != NULL)
      list_all_models = TRUE;
    swe_set_astro_models(samod, iflag);
  }
  *samod0 = '\0';
  for (i = 0; i < NSE_MODELS; i++) {
    imod = pmodel[i];
    switch(i) {
      case SE_MODEL_PREC_LONGTERM:
	if (imod == SEMOD_PREC_DEFAULT) imod = 0; break;
      case SE_MODEL_PREC_SHORTTERM:
	if (imod == SEMOD_PREC_DEFAULT_SHORT) imod = 0; break;
      case SE_MODEL_NUT:
	if (imod == SEMOD_NUT_DEFAULT) imod = 0; break;
      case SE_MODEL_SIDT:
	if (imod == SEMOD_SIDT_DEFAULT) imod = 0; break;
      case SE_MODEL_BIAS:
	if (imod == SEMOD_BIAS_DEFAULT) imod = 0; break;
      case SE_MODEL_JPLHOR_MODE:
	if (imod == SEMOD_JPLHOR_DEFAULT) imod = 0; break;
      case SE_MODEL_JPLHORA_MODE:
	if (imod == SEMOD_JPLHORA_DEFAULT) imod = 0; break;
      case SE_MODEL_DELTAT:
	if (imod == SEMOD_DELTAT_DEFAULT) imod = 0; break;
    }
    sprintf(samod0 + strlen(samod0), "%d,", imod);
  }
  /*if (samod != NULL) 
    strcpy(samod, samod0);*/
  *sdet = '\0';
  if (sdet != NULL) {
    /* JPL ephemeris number and tidal acceleration used with it */
    sprintf(sdet + strlen(sdet), "JPL eph. %d; tidal acc. Moon used by SE: %.4f\n", 
      swi_get_denum(SEI_SUN, iflag), swe_get_tid_acc());
    if (iflag & SEFLG_JPLEPH) {
      if (iflag & SEFLG_JPLHOR) 
	strcat(sdet, "JPL Horizons method:\n");
      if (iflag & SEFLG_JPLHOR_APPROX) 
	strcat(sdet, "JPL Horizons method (approximation):\n");
    } else if (iflag & SEFLG_SWIEPH) {
      strcat(sdet, "Swiss Ephemeris compressed files sepl*/semo*\n");
    } else {
      strcat(sdet, "Moshier semi-analytical approximation\n");
    }
    /* long-term Delta T calculation */
    get_deltat_model(pmodel[SE_MODEL_DELTAT], s);
    sprintf(sdet + strlen(sdet), "Delta T (long-term): %s\n", s);
    /* precession model */
    get_precession_model(pmodel[SE_MODEL_PREC_LONGTERM], iflag, s);
    sprintf(sdet + strlen(sdet), "Precession: %s\n", s);
    if (pmodel[SE_MODEL_PREC_LONGTERM] != pmodel[SE_MODEL_PREC_SHORTTERM] && !(iflag & (SEFLG_JPLHOR | SEFLG_JPLHOR_APPROX))) {
      get_precession_model(pmodel[SE_MODEL_PREC_SHORTTERM], iflag, s);
      sprintf(sdet + strlen(sdet), "+ short-term model: %s\n", s);
    }
    /* nutation */
    get_nutation_model(pmodel[SE_MODEL_NUT], iflag, s);
    sprintf(sdet + strlen(sdet), "Nutation: %s\n", s);
    /* frame bias */
    get_frame_bias_model(pmodel[SE_MODEL_BIAS], s);
    sprintf(sdet + strlen(sdet), "Frame bias: %s\n", s);
    /* sidereal time */
    get_sidt_model(pmodel[SE_MODEL_SIDT], s);
    sprintf(sdet + strlen(sdet), "Sid. time: %s\n", s);
    /* swetest parameters */
    sprintf(sdet + strlen(sdet), "swetest parameters:      D P P N B J J S\n");
    sprintf(sdet + strlen(sdet), "                    -amod%s", samod0);
    sprintf(sdet + strlen(sdet), " -tidacc%f", swe_get_tid_acc());
    strcat(sdet, "\n");
    /* list all available astronomical models */
    if (!list_all_models) {
      sprintf(sdet + strlen(sdet), "For list of all available astronomical models, add a '+' to the version string\n(swetest parameter -amod%s+ or -amod%s+)\n", samod, samod0);
    } else {
      strcat(sdet, "DELTA T MODELS (D)\n");
      for (i = 0; i <= SEMOD_NDELTAT; i++) {
	if (i == SEMOD_DELTAT_DEFAULT) continue;
        sprintf(sdet + strlen(sdet), "  (%d)", i);
	if (i == 0) sprintf(sdet + strlen(sdet), " (=%d)", SEMOD_DELTAT_DEFAULT);
	get_deltat_model(i, s);
	sprintf(sdet + strlen(sdet), ": %s\n", s);
      }
      strcat(sdet, "PRECESSION MODELS (P P) (long-term/short-term)\n");
      for (i = 0; i <= SEMOD_NPREC; i++) {
	if (i == SEMOD_PREC_DEFAULT) continue;
        sprintf(sdet + strlen(sdet), "  (%d)", i);
	if (i == 0) sprintf(sdet + strlen(sdet), " (=%d)", SEMOD_PREC_DEFAULT);
	get_precession_model(i, iflag, s);
	sprintf(sdet + strlen(sdet), ": %s\n", s);
      }
      strcat(sdet, "NUTATION MODELS (N)\n");
      for (i = 0; i <= SEMOD_NNUT; i++) {
	if (i == SEMOD_NUT_DEFAULT) continue;
        sprintf(sdet + strlen(sdet), "  (%d)", i);
	if (i == 0) sprintf(sdet + strlen(sdet), " (=%d)", SEMOD_NUT_DEFAULT);
	get_nutation_model(i, iflag, s);
	sprintf(sdet + strlen(sdet), ": %s\n", s);
      }
      strcat(sdet, "FRAME BIAS MODELS (B)\n");
      for (i = 0; i <= SEMOD_NBIAS; i++) {
	if (i == SEMOD_BIAS_DEFAULT) continue;
        sprintf(sdet + strlen(sdet), "  (%d)", i);
	if (i == 0) sprintf(sdet + strlen(sdet), " (=%d)", SEMOD_BIAS_DEFAULT);
	get_frame_bias_model(i, s);
	sprintf(sdet + strlen(sdet), ": %s\n", s);
      }
      strcat(sdet, "JPL HORIZONS MODELS (J) (with SEFLG_JPLEPH|SEFLG_JPLHOR).\n");
      strcat(sdet, "  IAU 1980 (Wahr) + daily corrections to dpsi/deps 1962-today.\n");
      strcat(sdet, "  (0 (=1): between 1799 and 1962, dpsi/deps of 20-jan-1962 are used.\n");
      strcat(sdet, "           For times beyond the dpsi/deps table, the last tabulated values are used.\n");
      strcat(sdet, "           Beyond 1799 and 2201, precession Owen 1990 is used..\n");
      strcat(sdet, "  Documentation in swephexp.h under 'methods of JPL Horizons'\n");
      strcat(sdet, "JPL HORIZONS APPROXIMATION (J) (with SEFLG_JPLEPH|SEFLG_JPLHORA)\n");
      strcat(sdet, "  Documentation in swephexp.h under 'methods of JPL Horizons'\n");
      strcat(sdet, "SIDEREAL TIME MODELS (S)\n");
      for (i = 0; i <= SEMOD_NSIDT; i++) {
	if (i == SEMOD_SIDT_DEFAULT) continue;
        sprintf(sdet + strlen(sdet), "  (%d)", i);
	if (i == 0) sprintf(sdet + strlen(sdet), " (=%d)", SEMOD_SIDT_DEFAULT);
	get_sidt_model(i, s);
	sprintf(sdet + strlen(sdet), ": %s\n", s);
      }
    }
  }
}

char *swi_strcpy(char *to, char *from)
{
  char *sp, s[AS_MAXCH];
  if (*from == '\0') {
    *to = '\0';
    return to;
  }
  if (strlen(from) < AS_MAXCH) {
    strcpy(s, from);
    strcpy(to, s);
  } else {
    sp = strdup(from);
    if (sp == NULL) {
      strcpy(to, from);
    } else {
      strcpy(to, sp);
      free(sp);
    }
  }
  return to;
}

char *swi_strncpy(char *to, char *from, size_t n)
{ 
  char *sp, s[AS_MAXCH];
  if (*from == '\0') {
    return to;
  }
  if (strlen(from) < AS_MAXCH) {
    strncpy(s, from, n);
    strncpy(to, s, n);
  } else {
    sp = strdup(from);
    if (sp == NULL) {
      strncpy(to, from, n);
    } else {
      strncpy(to, sp, n);
      free(sp);
    }
  }
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
      if (serr != NULL) {
	sprintf(serr, "could not open trace output file '%s'", fname);
      }
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
      if (serr != NULL) {
	sprintf(serr, "could not open trace output file '%s'", fname);
      }
    }
  }
}
#endif
