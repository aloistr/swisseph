/* SWISSEPH
   $Header: swemplan.c,v 1.27 98/12/02 19:18:03 dieter Exp $
   Moshier planet routines

   modified for SWISSEPH by Dieter Koch

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
#include "swephexp.h"
#include "sweph.h"
#include "swephlib.h"
#ifndef NO_MOSHIER
#include "swemptab.c"

#define TIMESCALE 3652500.0

#define mods3600(x) ((x) - 1.296e6 * floor ((x)/1.296e6))

static void embofs_mosh(double J, double *xemb);

static int pnoint2msh[]   = {2, 2, 0, 1, 3, 4, 5, 6, 7, 8, };

/* From Simon et al (1994)  */
static double freqs[] =
{
/* Arc sec per 10000 Julian years.  */
  53810162868.8982,
  21066413643.3548,
  12959774228.3429,
  6890507749.3988,
  1092566037.7991,
  439960985.5372,
  154248119.3933,
  78655032.0744,
  52272245.1795
};

static double phases[] =
{
/* Arc sec.  */
  252.25090552 * 3600.,
  181.97980085 * 3600.,
  100.46645683 * 3600.,
  355.43299958 * 3600.,
  34.35151874 * 3600.,
  50.07744430 * 3600.,
  314.05500511 * 3600.,
  304.34866548 * 3600.,
  860492.1546,
};

static struct plantbl *planets[] =
{
  &mer404,
  &ven404,
  &ear404,
  &mar404,
  &jup404,
  &sat404,
  &ura404,
  &nep404,
  &plu404
};

static double FAR ss[9][24];
static double FAR cc[9][24];

static void sscc (int k, double arg, int n);

int swi_moshplan2 (double J, int iplm, double *pobj)
{
  int i, j, k, m, k1, ip, np, nt;
  signed char FAR *p;
  double FAR *pl, *pb, *pr;
  double su, cu, sv, cv, T;
  double t, sl, sb, sr;
  struct plantbl *plan = planets[iplm];

  T = (J - J2000) / TIMESCALE;
  /* Calculate sin( i*MM ), etc. for needed multiple angles.  */
  for (i = 0; i < 9; i++)
    {
      if ((j = plan->max_harmonic[i]) > 0)
	{
	  sr = (mods3600 (freqs[i] * T) + phases[i]) * STR;
	  sscc (i, sr, j);
	}
    }

  /* Point to start of table of arguments. */
  p = plan->arg_tbl;
  /* Point to tabulated cosine and sine amplitudes.  */
  pl = plan->lon_tbl;
  pb = plan->lat_tbl;
  pr = plan->rad_tbl;
  sl = 0.0;
  sb = 0.0;
  sr = 0.0;

  for (;;)
    {
      /* argument of sine and cosine */
      /* Number of periodic arguments. */
      np = *p++;
      if (np < 0)
	break;
      if (np == 0)
	{			/* It is a polynomial term.  */
	  nt = *p++;
	  /* Longitude polynomial. */
	  cu = *pl++;
	  for (ip = 0; ip < nt; ip++)
	    {
	      cu = cu * T + *pl++;
	    }
	  sl +=  mods3600 (cu);
	  /* Latitude polynomial. */
	  cu = *pb++;
	  for (ip = 0; ip < nt; ip++)
	    {
	      cu = cu * T + *pb++;
	    }
	  sb += cu;
	  /* Radius polynomial. */
	  cu = *pr++;
	  for (ip = 0; ip < nt; ip++)
	    {
	      cu = cu * T + *pr++;
	    }
	  sr += cu;
	  continue;
	}
      k1 = 0;
      cv = 0.0;
      sv = 0.0;
      for (ip = 0; ip < np; ip++)
	{
	  /* What harmonic.  */
	  j = *p++;
	  /* Which planet.  */
	  m = *p++ - 1;
	  if (j)
	    {
	      k = j;
	      if (j < 0)
		k = -k;
	      k -= 1;
	      su = ss[m][k];	/* sin(k*angle) */
	      if (j < 0)
		su = -su;
	      cu = cc[m][k];
	      if (k1 == 0)
		{		/* set first angle */
		  sv = su;
		  cv = cu;
		  k1 = 1;
		}
	      else
		{		/* combine angles */
		  t = su * cv + cu * sv;
		  cv = cu * cv - su * sv;
		  sv = t;
		}
	    }
	}
      /* Highest power of T.  */
      nt = *p++;
      /* Longitude. */
      cu = *pl++;
      su = *pl++;
      for (ip = 0; ip < nt; ip++)
	{
	  cu = cu * T + *pl++;
	  su = su * T + *pl++;
	}
      sl += cu * cv + su * sv;
      /* Latitiude. */
      cu = *pb++;
      su = *pb++;
      for (ip = 0; ip < nt; ip++)
	{
	  cu = cu * T + *pb++;
	  su = su * T + *pb++;
	}
      sb += cu * cv + su * sv;
      /* Radius. */
      cu = *pr++;
      su = *pr++;
      for (ip = 0; ip < nt; ip++)
	{
	  cu = cu * T + *pr++;
	  su = su * T + *pr++;
	}
      sr += cu * cv + su * sv;
    }
  pobj[0] = STR * sl;
  pobj[1] = STR * sb;
  pobj[2] = STR * plan->distance * sr + plan->distance;
  return OK;
}

/* Moshier ephemeris.
 * computes heliocentric cartesian equatorial coordinates of
 * equinox 2000
 * for earth and a planet
 * tjd		julian day
 * ipli		internal SWEPH planet number
 * xp		array of 6 doubles for planet's position and speed
 * xe		                       earth's
 * serr		error string
 */
int swi_moshplan(double tjd, int ipli, AS_BOOL do_save, double *xpret, double *xeret, char *serr) 
{
  int i;
  int do_earth = FALSE;
  double dx[3], x2[3], xxe[6], xxp[6];
  double *xp, *xe;
  double dt; 
  char s[AS_MAXCH];
  int iplm = pnoint2msh[ipli];
  struct plan_data *pdp = &swed.pldat[ipli];
  struct plan_data *pedp = &swed.pldat[SEI_EARTH];
  double seps2000 = swed.oec2000.seps;
  double ceps2000 = swed.oec2000.ceps;
  if (do_save) {
    xp = pdp->x;
    xe = pedp->x;
  } else {
    xp = xxp;
    xe = xxe;
  }
  if (do_save || ipli == SEI_EARTH || xeret != NULL) 
    do_earth = TRUE;
  /* tjd beyond ephemeris limits */
  if (tjd < MOSHPLEPH_START || tjd > MOSHPLEPH_END) {
    if (serr != NULL) {
      sprintf(s, "jd %f beyond Moshier eph. limits %f and %f ",
		    tjd, MOSHPLEPH_START, MOSHPLEPH_END);
      if (strlen(serr) + strlen(s) < AS_MAXCH)
	strcat(serr, s);
    }
    return(ERR);
  }  
  /* earth, for geocentric position */
  if (do_earth) {
    if (tjd == pedp->teval
	  && pedp->iephe == SEFLG_MOSEPH) {
      xe = pedp->x;
    } else {
      /* emb */
      swi_moshplan2(tjd, pnoint2msh[SEI_EMB], xe); /* emb hel. ecl. 2000 polar */ 
      swi_polcart(xe, xe);			  /* to cartesian */
      swi_coortrf2(xe, xe, -seps2000, ceps2000);/* and equator 2000 */
      embofs_mosh(tjd, xe);		  /* emb -> earth */
      if (do_save) {
	pedp->teval = tjd;		  
	pedp->xflgs = -1;
	pedp->iephe = SEFLG_MOSEPH;
      }
      /* one more position for speed. */
      swi_moshplan2(tjd - PLAN_SPEED_INTV, pnoint2msh[SEI_EMB], x2); 
      swi_polcart(x2, x2);
      swi_coortrf2(x2, x2, -seps2000, ceps2000);
      embofs_mosh(tjd - PLAN_SPEED_INTV, x2);/**/
      for (i = 0; i <= 2; i++) 
	dx[i] = (xe[i] - x2[i]) / PLAN_SPEED_INTV;
      /* store speed */
      for (i = 0; i <= 2; i++) {
	xe[i+3] = dx[i];
      }
    }
    if (xeret != NULL)
      for (i = 0; i <= 5; i++) 
	xeret[i] = xe[i];
  }
  /* earth is the planet wanted */
  if (ipli == SEI_EARTH) {
    xp = xe;
  } else {
    /* other planet */
    /* if planet has already been computed, return */
    if (tjd == pdp->teval && pdp->iephe == SEFLG_MOSEPH) {
      xp = pdp->x;
    } else { 
      swi_moshplan2(tjd, iplm, xp); 
      swi_polcart(xp, xp);
      swi_coortrf2(xp, xp, -seps2000, ceps2000);
      if (do_save) {
	pdp->teval = tjd;/**/
	pdp->xflgs = -1;
	pdp->iephe = SEFLG_MOSEPH;
      }
      /* one more position for speed. 
       * the following dt gives good speed for light-time correction
       */
    #if 0
      for (i = 0; i <= 2; i++) 
	dx[i] = xp[i] - pedp->x[i];
      dt = LIGHTTIME_AUNIT * sqrt(square_sum(dx));   
    #endif
      dt = PLAN_SPEED_INTV;
      swi_moshplan2(tjd - dt, iplm, x2); 
      swi_polcart(x2, x2);
      swi_coortrf2(x2, x2, -seps2000, ceps2000);
      for (i = 0; i <= 2; i++) 
	dx[i] = (xp[i] - x2[i]) / dt;
      /* store speed */
      for (i = 0; i <= 2; i++) {
	xp[i+3] = dx[i];
      }
    }
    if (xpret != NULL)
      for (i = 0; i <= 5; i++)
	xpret[i] = xp[i];
  }
  return(OK);
}


/* Prepare lookup table of sin and cos ( i*Lj )
 * for required multiple angles
 */
static void sscc (int k, double arg, int n)
{
  double cu, su, cv, sv, s;
  int i;

  su = sin (arg);
  cu = cos (arg);
  ss[k][0] = su;		/* sin(L) */
  cc[k][0] = cu;		/* cos(L) */
  sv = 2.0 * su * cu;
  cv = cu * cu - su * su;
  ss[k][1] = sv;		/* sin(2L) */
  cc[k][1] = cv;
  for (i = 2; i < n; i++)
    {
      s = su * cv + cu * sv;
      cv = cu * cv - su * sv;
      sv = s;
      ss[k][i] = sv;		/* sin( i+1 L ) */
      cc[k][i] = cv;
    }
}


/* Adjust position from Earth-Moon barycenter to Earth
 *
 * J = Julian day number
 * xemb = rectangular equatorial coordinates of Earth
 */
static void embofs_mosh(double tjd, double *xemb) 
{
  double T, M, a, L, B, p;
  double smp, cmp, s2mp, c2mp, s2d, c2d, sf, cf;
  double s2f, sx, cx, xyz[6];
  double seps = swed.oec.seps;
  double ceps = swed.oec.ceps;
  int i;
  /* Short series for position of the Moon
   */
  T = (tjd-J1900)/36525.0;
  /* Mean anomaly of moon (MP) */
  a = swe_degnorm(((1.44e-5*T + 0.009192)*T + 477198.8491)*T + 296.104608);
  a *= DEGTORAD;
  smp = sin(a);
  cmp = cos(a);
  s2mp = 2.0*smp*cmp;		/* sin(2MP) */
  c2mp = cmp*cmp - smp*smp;	/* cos(2MP) */
  /* Mean elongation of moon (D) */
  a = swe_degnorm(((1.9e-6*T - 0.001436)*T + 445267.1142)*T + 350.737486);
  a  = 2.0 * DEGTORAD * a;
  s2d = sin(a);
  c2d = cos(a);
  /* Mean distance of moon from its ascending node (F) */
  a = swe_degnorm((( -3.e-7*T - 0.003211)*T + 483202.0251)*T + 11.250889);
  a  *= DEGTORAD;
  sf = sin(a);
  cf = cos(a);
  s2f = 2.0*sf*cf;	/* sin(2F) */
  sx = s2d*cmp - c2d*smp;	/* sin(2D - MP) */
  cx = c2d*cmp + s2d*smp;	/* cos(2D - MP) */
  /* Mean longitude of moon (LP) */
  L = ((1.9e-6*T - 0.001133)*T + 481267.8831)*T + 270.434164;
  /* Mean anomaly of sun (M) */
  M = swe_degnorm((( -3.3e-6*T - 1.50e-4)*T + 35999.0498)*T + 358.475833);
  /* Ecliptic longitude of the moon */
  L =	L
	+ 6.288750*smp
	+ 1.274018*sx
	+ 0.658309*s2d
	+ 0.213616*s2mp
	- 0.185596*sin( DEGTORAD * M )
	- 0.114336*s2f;
  /* Ecliptic latitude of the moon */
  a = smp*cf;
  sx = cmp*sf;
  B =	  5.128189*sf
	+ 0.280606*(a+sx)		/* sin(MP+F) */
	+ 0.277693*(a-sx)		/* sin(MP-F) */
	+ 0.173238*(s2d*cf - c2d*sf);	/* sin(2D-F) */
  B *= DEGTORAD;
  /* Parallax of the moon */
  p =	 0.950724
	+0.051818*cmp
	+0.009531*cx
	+0.007843*c2d
	+0.002824*c2mp;
  p *= DEGTORAD;
  /* Elongation of Moon from Sun
   */
  L = swe_degnorm(L);
  L *= DEGTORAD;
  /* Distance in au */
  a = 4.263523e-5/sin(p);
  /* Convert to rectangular ecliptic coordinates */
  xyz[0] = L;
  xyz[1] = B;
  xyz[2] = a;
  swi_polcart(xyz, xyz);
  /* Convert to equatorial */
  swi_coortrf2(xyz, xyz, -seps, ceps);
  /* Precess to equinox of J2000.0 */
  swi_precess(xyz, tjd, J_TO_J2000);/**/
  /* now emb -> earth */
  for (i = 0; i <= 2; i++)
    xemb[i] -= xyz[i] / (EARTH_MOON_MRAT + 1.0);
}
#endif /* NO_MOSHIER */

/* orbital elements of planets that are computed from osculating elements
 *   epoch
 *   equinox
 *   mean anomaly,
 *   semi axis,
 *   eccentricity,
 *   argument of perihelion,
 *   ascending node
 *   inclination
 */
#define SE_NEELY                /* use James Neely's revised elements 
                                 *      of Uranian planets*/
static double plan_oscu_elem[SE_NFICT_ELEM][8] = {
#ifdef SE_NEELY
  {J1900, J1900, 163.7409, 40.99837, 0.00460, 171.4333, 129.8325, 1.0833},	/* Cupido Neely */ 
  {J1900, J1900,  27.6496, 50.66744, 0.00245, 148.1796, 161.3339, 1.0500},	/* Hades Neely */
  {J1900, J1900, 165.1232, 59.21436, 0.00120, 299.0440,   0.0000, 0.0000},	/* Zeus Neely */
  {J1900, J1900, 169.0193, 64.81960, 0.00305, 208.8801,   0.0000, 0.0000},	/* Kronos Neely */ 
  {J1900, J1900, 138.0533, 70.29949, 0.00000,   0.0000,   0.0000, 0.0000},    /* Apollon Neely */
  {J1900, J1900, 351.3350, 73.62765, 0.00000,   0.0000,   0.0000, 0.0000},	/* Admetos Neely */
  {J1900, J1900,  55.8983, 77.25568, 0.00000,   0.0000,   0.0000, 0.0000},	/* Vulcanus Neely */
  {J1900, J1900, 165.5163, 83.66907, 0.00000,   0.0000,   0.0000, 0.0000},	/* Poseidon Neely */
#else
  {J1900, J1900, 104.5959, 40.99837,  0, 0, 0, 0}, /* Cupido   */
  {J1900, J1900, 337.4517, 50.667443, 0, 0, 0, 0}, /* Hades    */
  {J1900, J1900, 104.0904, 59.214362, 0, 0, 0, 0}, /* Zeus     */
  {J1900, J1900,  17.7346, 64.816896, 0, 0, 0, 0}, /* Kronos   */
  {J1900, J1900, 138.0354, 70.361652, 0, 0, 0, 0}, /* Apollon  */
  {J1900, J1900,  -8.678,  73.736476, 0, 0, 0, 0}, /* Admetos  */
  {J1900, J1900,  55.9826, 77.445895, 0, 0, 0, 0}, /* Vulkanus */
  {J1900, J1900, 165.3595, 83.493733, 0, 0, 0, 0}, /* Poseidon */
#endif
  /* Isis-Transpluto; elements from "Die Sterne" 3/1952, p. 70ff. 
   * Strubell does not give an equinox. 1945 is taken to best reproduce 
   * ASTRON ephemeris. (This is a strange choice, though.)
   * The epoch is 1772.76. The year is understood to have 366 days.
   * The fraction is counted from 1 Jan. 1772 */
  {2368547.66, 2431456.5, 0.0, 77.775, 0.3, 0.7, 0, 0},
  /* Nibiru, elements from Christian Woeltge, Hannover */
  {1856113.380954, 1856113.380954, 0.0, 234.8921, 0.981092, 103.966, -44.567, 158.708},
  /* Harrington, elements from Astronomical Journal 96(4), Oct. 1988 */
  {2374696.5, J2000, 0.0, 101.2, 0.411, 208.5, 275.4, 32.4},
  /* Leverrier's Neptune, 
	according to W.G. Hoyt, "Planets X and Pluto", Tucson 1980, p. 63 */
  {2395662.5, 2395662.5, 34.05, 36.15, 0.10761, 284.75, 0, 0}, 
  /* Adam's Neptune */
  {2395662.5, 2395662.5, 24.28, 37.25, 0.12062, 299.11, 0, 0}, 
  /* Lowell's Pluto */
  {2425977.5, 2425977.5, 281, 43.0, 0.202, 204.9, 0, 0}, 
  /* Pickering's Pluto */
  {2425977.5, 2425977.5, 48.95, 55.1, 0.31, 280.1, 100, 15}, /**/
#if 0	/* Ceres JPL 1600, without perturbations from other minor planets,
	 * from following initial elements: 
	 * 2450600.5 2000 0 1 164.7073602 73.0340746 80.5995101 
	 * 10.5840296 0.07652422 0.0 2.770176095 */
  {2305447.5, J2000, 0.5874558977449977e+02, 0.2766536058742327e+01, 
    0.7870946565779195e-01, 0.5809199028919189e+02, 
    0.8650119410725021e+02, 0.1066835622280712e+02}, 
  /* Chiron, Bowell database 18-mar-1997 */
  {2450500.5, J2000, 7.258191, 13.67387471, 0.38174778, 339.558345, 209.379239, 6.933360}, /**/
#endif
};

/* computes a planet from osculating elements *
 * tjd		julian day
 * ipl		body number
 * ipli 	body number in planetary data structure
 * iflag	flags
 */
void swi_osc_el_plan(double tjd, double *xp, int ipl, int ipli)
{
  double pqr[9], x[6];
  double eps, K, fac, rho, cose, sine;
  double alpha, beta, zeta, sigma, M2, Msgn, M_180_or_0;
  double M, E;
  struct plan_data *pedp = &swed.pldat[SEI_EARTH];
  struct plan_data *pdp = &swed.pldat[ipli];
  /* orbital elements */
  double tjd0 = plan_oscu_elem[ipl][0];			/* epoch */
  double tequ = plan_oscu_elem[ipl][1];			/* equinox */
  double mano = plan_oscu_elem[ipl][2] * DEGTORAD;	/* mean anomaly */
  double sema = plan_oscu_elem[ipl][3];			/* semi-axis */
  double ecce = plan_oscu_elem[ipl][4];			/* eccentricity */
  double parg = plan_oscu_elem[ipl][5] * DEGTORAD;	/* arg. of peri. */
  double node = plan_oscu_elem[ipl][6] * DEGTORAD;	/* asc. node */
  double incl = plan_oscu_elem[ipl][7] * DEGTORAD;	/* inclination */
  double dmot = 0.9856076686 * DEGTORAD / sema / sqrt(sema);	/* daily motion */
  double cosnode = cos(node);
  double sinnode = sin(node);
  double cosincl = cos(incl);
  double sinincl = sin(incl); 
  double cosparg = cos(parg);
  double sinparg = sin(parg);
  /* Gaussian vector */
  pqr[0] = cosparg * cosnode - sinparg * cosincl * sinnode;
  pqr[1] = -sinparg * cosnode - cosparg * cosincl * sinnode;
  pqr[2] = sinincl * sinnode;
  pqr[3] = cosparg * sinnode + sinparg * cosincl * cosnode;
  pqr[4] = -sinparg * sinnode + cosparg * cosincl * cosnode;
  pqr[5] = -sinincl * cosnode;
  pqr[6] = sinparg * sinincl;
  pqr[7] = cosparg * sinincl;
  pqr[8] = cosincl;
  /* Kepler problem */
  E = M = swi_mod2PI(mano + (tjd - tjd0) * dmot); /* mean anomaly of date */
  /* better E for very high eccentricity and small M */
  if (ecce > 0.975) {
    M2 = M * RADTODEG;
    if (M2 > 150 && M2 < 210) {
      M2 -= 180;
      M_180_or_0 = 180;
    } else
      M_180_or_0 = 0;
    if (M2 > 330) 
      M2 -= 360;
    if (M2 < 0) {
      M2 = -M2;
      Msgn = -1;
    } else
      Msgn = 1;
    if (M2 < 30) {
      M2 *= DEGTORAD;
      alpha = (1 - ecce) / (4 * ecce + 0.5);
      beta = M2 / (8 * ecce + 1);
      zeta = pow(beta + sqrt(beta * beta + alpha * alpha), 1/3);
      sigma = zeta - alpha / 2;
      sigma = sigma - 0.078 * sigma * sigma * sigma * sigma * sigma / (1 + ecce);
      E = Msgn * (M2 + ecce * (3 * sigma - 4 * sigma * sigma * sigma))
			+ M_180_or_0;
    }
  }
  E = swi_kepler(E, M, ecce);
  /* position and speed, referred to orbital plane */
  K = KGAUSS / sqrt(sema);
  cose = cos(E);
  sine = sin(E);
  fac = sqrt((1 - ecce) * (1 + ecce));
  rho = 1 - ecce * cose;
  x[0] = sema * (cose - ecce);
  x[1] = sema * fac * sine;
  x[3] = -K * sine / rho;
  x[4] = K * fac * cose / rho;
  /* transformation to ecliptic */
  xp[0] = pqr[0] * x[0] + pqr[1] * x[1];
  xp[1] = pqr[3] * x[0] + pqr[4] * x[1];
  xp[2] = pqr[6] * x[0] + pqr[7] * x[1];
  xp[3] = pqr[0] * x[3] + pqr[1] * x[4];
  xp[4] = pqr[3] * x[3] + pqr[4] * x[4];
  xp[5] = pqr[6] * x[3] + pqr[7] * x[4];
  /* transformation to equator */
  eps = swi_epsiln(tequ);
  swi_coortrf(xp, xp, -eps);
  swi_coortrf(xp+3, xp+3, -eps);
  /* FK4 -> FK5; whenever elements are not J2000 */
  if (tequ != J2000) {
    swi_FK4_FK5(xp, tequ);
  }
  /* precess to J2000 */
  if (tequ != J2000) {
    swi_precess(xp, tequ, J_TO_J2000);
    swi_precess(xp+3, tequ, J_TO_J2000);
  }
  if (pdp->x == xp) {
    pdp->teval = tjd;	/* for precession! */
    pdp->iephe = pedp->iephe;
  }
}
