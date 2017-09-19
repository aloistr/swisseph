
/*******************************************************
$Header: /home/dieter/sweph/RCS/swehouse.c,v 1.76 2016/02/23 09:48:03 dieter Exp $
module swehouse.c
house and (simple) aspect calculation 

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

#include "sweodef.h"
#include "swephexp.h"
#include "sweph.h"
#include "swephlib.h"
#include "swehouse.h"
#include <string.h>

#define MILLIARCSEC 	(1.0 / 3600000.0)

static double Asc1(double, double, double, double);
static double Asc2(double, double, double, double);
static int CalcH(
	double th, double fi, double ekl, char hsy, 
	int iteration_count, struct houses *hsp);
static int sidereal_houses_ecl_t0(double tjde, 
                           double armc, 
                           double eps, 
                           double *nutlo, 
                           double lat, 
			   int hsys, 
                           double *cusp, 
                           double *ascmc);
static int sidereal_houses_trad(double tjde, 
                           double armc, 
                           double eps, 
                           double nutl, 
                           double lat, 
			   int hsys, 
                           double *cusp, 
                           double *ascmc);
static int sidereal_houses_ssypl(double tjde, 
                           double armc, 
                           double eps, 
                           double *nutlo, 
                           double lat,
			   int hsys, 
                           double *cusp, 
                           double *ascmc);
static int sunshine_solution_makransky(double ramc, double lat, double ecl, struct houses *hsp);
static int sunshine_solution_treindl(double ramc, double lat, double ecl, struct houses *hsp);
#if 0
static void test_Asc1();
#endif

/* housasp.c 
 * cusps are returned in double cusp[13],
 *                           or cusp[37] with house system 'G'.
 * cusp[1...12]	houses 1 - 12
 * additional points are returned in ascmc[10].
 * ascmc[0] = ascendant
 * ascmc[1] = mc
 * ascmc[2] = armc
 * ascmc[3] = vertex
 * ascmc[4] = equasc		* "equatorial ascendant" *
 * ascmc[5] = coasc1		* "co-ascendant" (W. Koch) *
 * ascmc[6] = coasc2		* "co-ascendant" (M. Munkasey) *
 * ascmc[7] = polasc		* "polar ascendant" (M. Munkasey) *
 */
int CALL_CONV swe_houses(double tjd_ut,
				double geolat,
				double geolon,
				int hsys,
				double *cusp,
				double *ascmc)
{
  int i, retc = 0;
  double armc, eps, nutlo[2];
  double tjde = tjd_ut + swe_deltat_ex(tjd_ut, -1, NULL);
  eps = swi_epsiln(tjde, 0) * RADTODEG;
  swi_nutation(tjde, 0, nutlo);
  for (i = 0; i < 2; i++)
    nutlo[i] *= RADTODEG;
  armc = swe_degnorm(swe_sidtime0(tjd_ut, eps + nutlo[1], nutlo[0]) * 15 + geolon);
  if (toupper(hsys) ==  'I') {	// compute sun declination for sunshine houses
    int flags = SEFLG_SPEED| SEFLG_EQUATORIAL;
    double xp[6];
    int result = swe_calc_ut(tjd_ut, SE_SUN, flags, xp, NULL);
    if (result < 0) return ERR;
    ascmc[9] = xp[1];	// declination in ascmc[9];
  }
#ifdef TRACE
  swi_open_trace(NULL);
  if (swi_trace_count <= TRACE_COUNT_MAX) {
    if (swi_fp_trace_c != NULL) {
      fputs("\n/*SWE_HOUSES*/\n", swi_fp_trace_c);
      fprintf(swi_fp_trace_c, "#if 0\n");
      fprintf(swi_fp_trace_c, "  tjd = %.9f;", tjd_ut);
      fprintf(swi_fp_trace_c, " geolon = %.9f;", geolon);
      fprintf(swi_fp_trace_c, " geolat = %.9f;", geolat);
      fprintf(swi_fp_trace_c, " hsys = %d;\n", hsys);
      fprintf(swi_fp_trace_c, "  retc = swe_houses(tjd, geolat, geolon, hsys, cusp, ascmc);\n");
      fprintf(swi_fp_trace_c, "  /* swe_houses calls swe_houses_armc as follows: */\n");
      fprintf(swi_fp_trace_c, "#endif\n");
      fflush(swi_fp_trace_c);
    }
  }
#endif
  retc = swe_houses_armc(armc, geolat, eps + nutlo[1], hsys, cusp, ascmc);
  return retc;
}

/* housasp.c 
 * cusps are returned in double cusp[13],
 *                           or cusp[37] with house system 'G'.
 * cusp[1...12]	houses 1 - 12
 * additional points are returned in ascmc[10].
 * ascmc[0] = ascendant
 * ascmc[1] = mc
 * ascmc[2] = armc
 * ascmc[3] = vertex
 * ascmc[4] = equasc		* "equatorial ascendant" *
 * ascmc[5] = coasc1		* "co-ascendant" (W. Koch) *
 * ascmc[6] = coasc2		* "co-ascendant" (M. Munkasey) *
 * ascmc[7] = polasc		* "polar ascendant" (M. Munkasey) *
 */
int CALL_CONV swe_houses_ex(double tjd_ut,
                                int32 iflag, 
				double geolat,
				double geolon,
				int hsys,
				double *cusp,
				double *ascmc)
{
  int i, retc = 0;
  double armc, eps_mean, nutlo[2];
  double tjde = tjd_ut + swe_deltat_ex(tjd_ut, iflag, NULL);
  struct sid_data *sip = &swed.sidd;
  double xp[6];
  int ito;
  if (toupper(hsys) == 'G')
    ito = 36;
  else
    ito = 12;
  if ((iflag & SEFLG_SIDEREAL) && !swed.ayana_is_set)
    swe_set_sid_mode(SE_SIDM_FAGAN_BRADLEY, 0, 0);
  eps_mean = swi_epsiln(tjde, 0) * RADTODEG;
  swi_nutation(tjde, 0, nutlo);
  for (i = 0; i < 2; i++)
    nutlo[i] *= RADTODEG;
#ifdef TRACE
  swi_open_trace(NULL);
  if (swi_trace_count <= TRACE_COUNT_MAX) {
    if (swi_fp_trace_c != NULL) {
      fputs("\n/*SWE_HOUSES_EX*/\n", swi_fp_trace_c);
      fprintf(swi_fp_trace_c, "#if 0\n");
      fprintf(swi_fp_trace_c, "  tjd = %.9f;", tjd_ut);
      fprintf(swi_fp_trace_c, " iflag = %d;\n", iflag);
      fprintf(swi_fp_trace_c, " geolon = %.9f;", geolon);
      fprintf(swi_fp_trace_c, " geolat = %.9f;", geolat);
      fprintf(swi_fp_trace_c, " hsys = %d;\n", hsys);
      fprintf(swi_fp_trace_c, "  retc = swe_houses_ex(tjd, iflag, geolat, geolon, hsys, cusp, ascmc);\n");
      fprintf(swi_fp_trace_c, "  /* swe_houses calls swe_houses_armc as follows: */\n");
      fprintf(swi_fp_trace_c, "#endif\n");
      fflush(swi_fp_trace_c);
    }
  }
#endif
    /*houses_to_sidereal(tjde, geolat, hsys, eps, cusp, ascmc, iflag);*/
  armc = swe_degnorm(swe_sidtime0(tjd_ut, eps_mean + nutlo[1], nutlo[0]) * 15 + geolon);
  if (toupper(hsys) ==  'I') {	// compute sun declination for sunshine houses
    int flags = SEFLG_SPEED| SEFLG_EQUATORIAL;
    int result = swe_calc_ut(tjd_ut, SE_SUN, flags, xp, NULL);
    if (result < 0) return ERR;
    ascmc[9] = xp[1];	// declination in ascmc[9];
  }
  if (iflag & SEFLG_SIDEREAL) { 
    if (sip->sid_mode & SE_SIDBIT_ECL_T0)
      retc = sidereal_houses_ecl_t0(tjde, armc, eps_mean + nutlo[1], nutlo, geolat, hsys, cusp, ascmc);
    else if (sip->sid_mode & SE_SIDBIT_SSY_PLANE)
      retc = sidereal_houses_ssypl(tjde, armc, eps_mean + nutlo[1], nutlo, geolat, hsys, cusp, ascmc);
    else
      retc = sidereal_houses_trad(tjde, armc, eps_mean + nutlo[1], nutlo[0], geolat, hsys, cusp, ascmc);
  } else {
    retc = swe_houses_armc(armc, geolat, eps_mean + nutlo[1], hsys, cusp, ascmc);
    if (toupper(hsys) ==  'I') 	// compute sun declination for sunshine houses
      ascmc[9] = xp[1];	// declination in ascmc[9];
  }
  if (iflag & SEFLG_RADIANS) {
    for (i = 1; i <= ito; i++)
      cusp[i] *= DEGTORAD;
    for (i = 0; i < SE_NASCMC; i++)
      ascmc[i] *= DEGTORAD;
  }
  return retc;
}

/*
 * houses to sidereal
 * ------------------
 * there are two methods: 
 * a) the traditional one
 *    houses are computed tropically, then nutation and the ayanamsa
 *    are subtracted.
 * b) the projection on the ecliptic of t0
 *    The house computation is then as follows:
 *
 * Be t the birth date and t0 the epoch at which ayanamsa = 0.
 * 1. Compute the angle between the mean ecliptic at t0 and 
 *    the true equator at t.
 *    The intersection point of these two circles we call the 
 *    "auxiliary vernal point", and the angle between them the 
 *    "auxiliary obliquity".
 * 2. Compute the distance of the auxiliary vernal point from the 
 *    vernal point at t. (this is a section on the equator)
 * 3. subtract this value from the armc of t = aux. armc.
 * 4. Compute the axes and houses for this aux. armc and aux. obliquity.
 * 5. Compute the distance between the auxiliary vernal point and the
 *    vernal point at t0 (this is the ayanamsa at t, measured on the
 *    ecliptic of t0)
 * 6. subtract this distance from all house cusps.
 * 7. subtract ayanamsa_t0 from all house cusps.
 */
static int sidereal_houses_ecl_t0(double tjde, 
                           double armc, 
                           double eps, 
                           double *nutlo, 
                           double lat, 
			   int hsys, 
                           double *cusp, 
                           double *ascmc)
{
  int i, j, retc = OK;
  double x[6], xvpx[6], x2[6], epst0, xnorm[6];
  double rxy, rxyz, c2, epsx, sgn, fac, dvpx, dvpxe;
  double armcx;
  struct sid_data *sip = &swed.sidd;
  int ito;
  if (toupper(hsys) == 'G')
    ito = 36;
  else
    ito = 12;
  /* epsilon at t0 */
  epst0 = swi_epsiln(sip->t0, 0);
  /* cartesian coordinates of an imaginary moving body on the
   * the mean ecliptic of t0; we take the vernal point: */
  x[0] = x[4] = 1; 
  x[1] = x[2] = x[3] = x[5] = 0;
  /* to equator */
  swi_coortrf(x, x, -epst0);
  swi_coortrf(x+3, x+3, -epst0);
  /* to tjd_et */
  swi_precess(x, sip->t0, 0, J_TO_J2000);
  swi_precess(x, tjde, 0, J2000_TO_J);
  swi_precess(x+3, sip->t0, 0, J_TO_J2000);
  swi_precess(x+3, tjde, 0, J2000_TO_J);
  /* to true equator of tjd_et */
  swi_coortrf(x, x, (eps - nutlo[1]) * DEGTORAD);
  swi_coortrf(x+3, x+3, (eps - nutlo[1]) * DEGTORAD);
  swi_cartpol_sp(x, x);
  x[0] += nutlo[0] * DEGTORAD;
  swi_polcart_sp(x, x);
  swi_coortrf(x, x, -eps * DEGTORAD);
  swi_coortrf(x+3, x+3, -eps * DEGTORAD);
  /* now, we have the moving point precessed to tjd_et.
   * next, we compute the auxiliary epsilon: */
  swi_cross_prod(x, x+3, xnorm);
  rxy =  xnorm[0] * xnorm[0] + xnorm[1] * xnorm[1];
  c2 = (rxy + xnorm[2] * xnorm[2]);
  rxyz = sqrt(c2);
  rxy = sqrt(rxy);
  epsx = asin(rxy / rxyz) * RADTODEG;           /* 1a */
  /* auxiliary vernal point */
  if (fabs(x[5]) < 1e-15)
    x[5] = 1e-15;
  fac = x[2] / x[5];
  sgn = x[5] / fabs(x[5]);
  for (j = 0; j <= 2; j++)
    xvpx[j] = (x[j] - fac * x[j+3]) * sgn;      /* 1b */
  /* distance of the auxiliary vernal point from 
   * the zero point at tjd_et (a section on the equator): */
  swi_cartpol(xvpx, x2);
  dvpx = x2[0] * RADTODEG;                      /* 2 */
  /* auxiliary armc */
  armcx = swe_degnorm(armc - dvpx);        /* 3 */
  /* compute axes and houses: */
  retc = swe_houses_armc(armcx, lat, epsx, hsys, cusp, ascmc);  /* 4 */
  /* distance between auxiliary vernal point and
   * vernal point of t0 (a section on the sidereal plane) */
  dvpxe = acos(swi_dot_prod_unit(x, xvpx)) * RADTODEG;  /* 5 */
  if (tjde < sip->t0)
    dvpxe = -dvpxe;
  for (i = 1; i <= ito; i++)                     /* 6, 7 */
    cusp[i] = swe_degnorm(cusp[i] - dvpxe - sip->ayan_t0);
  for (i = 0; i <= SE_NASCMC; i++) {
    if (i == 2)	/* armc */
      continue;
    ascmc[i] = swe_degnorm(ascmc[i] - dvpxe - sip->ayan_t0);
  }
  if (hsys == 'N') { /* 1 = 0° Aries */
    for (i = 1; i <= ito; i++) {
      cusp[i] = (i - 1) * 30;
    }
  }
  return retc;
}

/*
 * Be t the birth date and t0 the epoch at which ayanamsa = 0.
 * 1. Compute the angle between the solar system rotation plane and 
 *    the true equator at t.
 *    The intersection point of these two circles we call the 
 *    "auxiliary vernal point", and the angle between them the 
 *    "auxiliary obliquity".
 * 2. Compute the distance of the auxiliary vernal point from the 
 *    zero point at t. (this is a section on the equator)
 * 3. subtract this value from the armc of t = aux. armc.
 * 4. Compute the axes and houses for this aux. armc and aux. obliquity.
 * 5. Compute the distance between the auxiliary vernal point at t
 *    and the zero point of the solar system plane J2000
 *    (a section measured on the solar system plane)
 * 6. subtract this distance from all house cusps.
 * 7. compute the ayanamsa of J2000 on the solar system plane, 
 *    referred to t0
 * 8. subtract ayanamsa_t0 from all house cusps.
 * 9. subtract ayanamsa_2000 from all house cusps.
 */
static int sidereal_houses_ssypl(double tjde, 
                           double armc, 
                           double eps, 
                           double *nutlo, 
                           double lat, 
			   int hsys, 
                           double *cusp, 
                           double *ascmc)
{
  int i, j, retc = OK;
  double x[6], x0[6], xvpx[6], x2[6], xnorm[6];
  double rxy, rxyz, c2, epsx, eps2000, sgn, fac, dvpx, dvpxe, x00;
  double armcx;
  struct sid_data *sip = &swed.sidd;
  int ito;
  if (toupper(hsys) == 'G')
    ito = 36;
  else
    ito = 12;
  eps2000 = swi_epsiln(J2000, 0);
  /* cartesian coordinates of the zero point on the
   * the solar system rotation plane */
  x[0] = x[4] = 1; 
  x[1] = x[2] = x[3] = x[5] = 0;
  /* to ecliptic 2000 */
  swi_coortrf(x, x, -SSY_PLANE_INCL);
  swi_coortrf(x+3, x+3, -SSY_PLANE_INCL);
  swi_cartpol_sp(x, x);
  x[0] += SSY_PLANE_NODE_E2000;
  swi_polcart_sp(x, x);
  /* to equator 2000 */
  swi_coortrf(x, x, -eps2000);
  swi_coortrf(x+3, x+3, -eps2000);
  /* to mean equator of t */
  swi_precess(x, tjde, 0, J2000_TO_J);
  swi_precess(x+3, tjde, 0, J2000_TO_J);
  /* to true equator of t */
  swi_coortrf(x, x, (eps - nutlo[1]) * DEGTORAD);
  swi_coortrf(x+3, x+3, (eps - nutlo[1]) * DEGTORAD);
  swi_cartpol_sp(x, x);
  x[0] += nutlo[0] * DEGTORAD;
  swi_polcart_sp(x, x);
  swi_coortrf(x, x, -eps * DEGTORAD);
  swi_coortrf(x+3, x+3, -eps * DEGTORAD);
  /* now, we have the moving point precessed to tjd_et.
   * next, we compute the auxiliary epsilon: */
  swi_cross_prod(x, x+3, xnorm);
  rxy =  xnorm[0] * xnorm[0] + xnorm[1] * xnorm[1];
  c2 = (rxy + xnorm[2] * xnorm[2]);
  rxyz = sqrt(c2);
  rxy = sqrt(rxy);
  epsx = asin(rxy / rxyz) * RADTODEG;           /* 1a */
  /* auxiliary vernal point */
  if (fabs(x[5]) < 1e-15)
    x[5] = 1e-15;
  fac = x[2] / x[5];
  sgn = x[5] / fabs(x[5]);
  for (j = 0; j <= 2; j++)
    xvpx[j] = (x[j] - fac * x[j+3]) * sgn;      /* 1b */
  /* distance of the auxiliary vernal point from 
   * mean vernal point at tjd_et (a section on the equator): */
  swi_cartpol(xvpx, x2);
  dvpx = x2[0] * RADTODEG;                      /* 2 */
  /* auxiliary armc */
  armcx = swe_degnorm(armc - dvpx);        /* 3 */
  /* compute axes and houses: */
  retc = swe_houses_armc(armcx, lat, epsx, hsys, cusp, ascmc);  /* 4 */
  /* distance between the auxiliary vernal point at t and
   * the sidereal zero point of 2000 at t
   * (a section on the sidereal plane).
   */ 
  dvpxe = acos(swi_dot_prod_unit(x, xvpx)) * RADTODEG;  /* 5 */
                /* (always positive for dates after 5400 bc) */
  dvpxe -= SSY_PLANE_NODE * RADTODEG;
  /* ayanamsa between t0 and J2000, measured on solar system plane: */
  /* position of zero point of t0 */
  x0[0] = 1; 
  x0[1] = x0[2] = 0; 
  /* zero point of t0 in J2000 system */
  if (sip->t0 != J2000)
    swi_precess(x0, sip->t0, 0, J_TO_J2000);
  /* zero point to ecliptic 2000 */
  swi_coortrf(x0, x0, eps2000);
  /* to solar system plane */
  swi_cartpol(x0, x0); 
  x0[0] -= SSY_PLANE_NODE_E2000;
  swi_polcart(x0, x0);
  swi_coortrf(x0, x0, SSY_PLANE_INCL);
  swi_cartpol(x0, x0);
  x0[0] += SSY_PLANE_NODE;
  x00 = x0[0] * RADTODEG;                       /* 7 */
  for (i = 1; i <= ito; i++)                     /* 6, 8, 9 */
    cusp[i] = swe_degnorm(cusp[i] - dvpxe - sip->ayan_t0 - x00);
  for (i = 0; i <= SE_NASCMC; i++) {
    if (i == 2)	/* armc */
      continue;
    ascmc[i] = swe_degnorm(ascmc[i] - dvpxe - sip->ayan_t0 - x00);
  }
  if (hsys == 'N') { /* 1 = 0° Aries */
    for (i = 1; i <= ito; i++) {
      cusp[i] = (i - 1) * 30;
    }
  }
  return retc;
}

/* common simplified procedure */
static int sidereal_houses_trad(double tjde,
                           double armc, 
                           double eps, 
                           double nutl, 
                           double lat, 
			   int hsys, 
                           double *cusp, 
                           double *ascmc) 
{
  int i, retc = OK;
  double ay;
  int ito;
  int ihs = toupper(hsys);
  int ihs2 = ihs;
  ay = swe_get_ayanamsa(tjde);
  if (ihs == 'G')
    ito = 36;
  else
    ito = 12;
  if (ihs == 'W')  /* whole sign houses: treat as 'E' and fix later */
    ihs2 = 'E';
//if (hsys == 'P') fprintf(stderr, "ay=%f, t=%f %c", ay, tjde, (char) hsys);
  retc = swe_houses_armc(armc, lat, eps, ihs2, cusp, ascmc);
//if (hsys == 'P') fprintf(stderr, "  h1=%f", cusp[1]);
  for (i = 1; i <= ito; i++) {
    cusp[i] = swe_degnorm(cusp[i] - ay - nutl);
    if (ihs == 'W') /* whole sign houses */
      cusp[i] -= fmod(cusp[i], 30);
  }
  if (ihs == 'N') { /* 1 = 0° Aries */
    for (i = 1; i <= ito; i++) {
      cusp[i] = (i - 1) * 30;
    }
  }
  for (i = 0; i < SE_NASCMC; i++) {
    if (i == 2)	/* armc */
      continue;
    ascmc[i] = swe_degnorm(ascmc[i] - ay - nutl);
  }
//if (hsys == 'P') fprintf(stderr, " => %f\n", cusp[1]);
  return retc;
}

/* 
 * this function is required for very special computations
 * where no date is given for house calculation,
 * e.g. for composite charts or progressive charts.
 * cusps are returned in double cusp[13],
 *                           or cusp[37] with house system 'G'.
 * cusp[1...12]	houses 1 - 12
 * additional points are returned in ascmc[10].
 * ascmc[0] = ascendant
 * ascmc[1] = mc
 * ascmc[2] = armc
 * ascmc[3] = vertex
 * ascmc[4] = equasc		* "equatorial ascendant" *
 * ascmc[5] = coasc1		* "co-ascendant" (W. Koch) *
 * ascmc[6] = coasc2		* "co-ascendant" (M. Munkasey) *
 * ascmc[7] = polasc		* "polar ascendant" (M. Munkasey) *
 */
int CALL_CONV swe_houses_armc(
				double armc,
				double geolat,
				double eps,
				int hsys,
				double *cusp,
				double *ascmc)
{
  struct houses h;
  int i, retc = 0;
  int ito;
  static double saved_sundec = 99;
  if (toupper(hsys) == 'G')
    ito = 36;
  else
    ito = 12;
  armc = swe_degnorm(armc);
  if (toupper(hsys) ==  'I') {	// declination for sunshine houses
    if (ascmc[9] == 99) {
      h.sundec = 0;
      if (saved_sundec != 99) h.sundec = saved_sundec;
    } else {
      h.sundec = ascmc[9];
      saved_sundec = h.sundec;
    }
  }
  retc = CalcH(armc, geolat, eps, (char)hsys, 2, &h);
  cusp[0] = 0;
  for (i = 1; i <= ito; i++) {
    cusp[i] = h.cusp[i];
  }
  ascmc[0] = h.ac;        /* Asc */    
  ascmc[1] = h.mc;        /* Mid */    
  ascmc[2] = armc;   
  ascmc[3] = h.vertex;
  ascmc[4] = h.equasc;
  ascmc[5] = h.coasc1;	/* "co-ascendant" (W. Koch) */
  ascmc[6] = h.coasc2;	/* "co-ascendant" (M. Munkasey) */
  ascmc[7] = h.polasc;	/* "polar ascendant" (M. Munkasey) */
  for (i = SE_NASCMC; i < 10; i++)
    ascmc[i] = 0;
  if (toupper(hsys) ==  'I') 	// declination for sunshine houses
    ascmc[9] = h.sundec ;
#ifdef TRACE
  swi_open_trace(NULL);
  if (swi_trace_count <= TRACE_COUNT_MAX) {
    if (swi_fp_trace_c != NULL) {
      fputs("\n/*SWE_HOUSES_ARMC*/\n", swi_fp_trace_c);
      fprintf(swi_fp_trace_c, "  armc = %.9f;", armc);
      fprintf(swi_fp_trace_c, " geolat = %.9f;", geolat);
      fprintf(swi_fp_trace_c, " eps = %.9f;", eps);
      fprintf(swi_fp_trace_c, " hsys = %d;\n", hsys);
      fprintf(swi_fp_trace_c, "  retc = swe_houses_armc(armc, geolat, eps, hsys, cusp, ascmc);\n");
      fputs("  printf(\"swe_houses_armc: %f\\t%f\\t%f\\t%c\\t\\n\", ", swi_fp_trace_c);
      fputs("  armc, geolat, eps, hsys);\n", swi_fp_trace_c);
      fputs("  printf(\"retc = %d\\n\", retc);\n", swi_fp_trace_c);
      fputs("  printf(\"cusp:\\n\");\n", swi_fp_trace_c);
      fputs("  for (i = 0; i < 12; i++)\n", swi_fp_trace_c);
      fputs("    printf(\"  %d\\t%f\\n\", i, cusp[i]);\n", swi_fp_trace_c);
      fputs("  printf(\"ascmc:\\n\");\n", swi_fp_trace_c);
      fputs("  for (i = 0; i < 10; i++)\n", swi_fp_trace_c);
      fputs("    printf(\"  %d\\t%f\\n\", i, ascmc[i]);\n", swi_fp_trace_c);
      fflush(swi_fp_trace_c);
    }
    if (swi_fp_trace_out != NULL) {
      fprintf(swi_fp_trace_out, "swe_houses_armc: %f\t%f\t%f\t%c\t\n", armc, geolat, eps, hsys);
      fprintf(swi_fp_trace_out, "retc = %d\n", retc);
      fputs("cusp:\n", swi_fp_trace_out);
      for (i = 1; i <= 12; i++)
	fprintf(swi_fp_trace_out, "  %d\t%f\n", i, cusp[i]);
      fputs("ascmc:\n", swi_fp_trace_out);
      for (i = 0; i < 10; i++)
	fprintf(swi_fp_trace_out, "  %d\t%f\n", i, ascmc[i]);
      fflush(swi_fp_trace_out);
    }
  }
#endif
#if 0 
/* for test of swe_house_pos(). 
 * 1st house will be 0, second 30, etc. */
for (i = 1; i <=12; i++) {
  double x[6];
  x[0] = cusp[i]; x[1] = 0; x[2] = 1;
  cusp[i] = (swe_house_pos(armc, geolat, eps, hsys, x, NULL) - 1) * 30;
}
#endif
  return retc;
}

/* for APC houses */
/* n  number of house
 * ph geographic latitude 
 * e  ecliptic obliquity
 * az armc
 */
static double apc_sector(int n, double ph, double e, double az)
{
   int k, is_below_hor = 0;
   double kv, a, dasc, dret;
   /* kv: ascensional difference of the ascendant */
   /* dasc: declination of the ascendant */
   if (fabs(ph * RADTODEG) > 90 - VERY_SMALL) {
     kv = 0;
     dasc = 0;
   } else {
     kv   = atan(tan(ph) * tan(e) * cos(az)/(1 + tan(ph) * tan(e) * sin(az)));
     if (fabs(ph * RADTODEG) < VERY_SMALL) {
       dasc = (90 - VERY_SMALL) * DEGTORAD;
       if (ph < 0)
         dasc = -dasc;
     } else {
       dasc = atan(sin(kv) / tan(ph));
     }
   }
   /* note, at polar circles, when the mc sinks below the horizon,
    * kv and dasc change sign in the above formulae.
    * this is what we need, because the ascendand jumps by 180 deg */
   /* printf("%f, %f\n", kv*RADTODEG, dasc*RADTODEG); */
   if (n < 8) {
     is_below_hor = 1;  /* 1 and 7 are included here */
     k = n - 1;
   } else {
     k = n - 13;
   }
   /* az + PI/2 + kv = armc + 90 + asc. diff. = right ascension of ascendant
    * PI/2 +- kv = semi-diurnal or seminocturnal arc of ascendant 
    * a = right ascension of house cusp on apc circle (ascendant-parallel
    * circle), with declination dasc */
   if (is_below_hor) {
     a = kv + az + PI/2 + k * (PI/2 - kv) / 3;
   } else {
     a = kv + az + PI/2 + k * (PI/2 + kv) / 3;
   }
   a = swe_radnorm(a);
   dret = atan2(tan(dasc) * tan(ph) * sin(az) + sin(a),
      cos(e) * (tan(dasc) * tan(ph) * cos(az) + cos(a)) + sin(e) * tan(ph) * sin(az - a));
   dret = swe_degnorm(dret * RADTODEG);
   return dret;
}

char *CALL_CONV swe_house_name(int hsys)
{
  int h = hsys;
  if (h != 'i') h = toupper(h);
  switch (h) {
  case 'A': return "equal";
  case 'B': return "Alcabitius";
  case 'C': return "Campanus";
  case 'D': return "equal (MC)";
  case 'E': return "equal";
  case 'F': return "Carter poli-equ.";
  case 'G': return "Gauquelin sectors";
  case 'H': return "horizon/azimut";
  case 'I': return "Sunshine";
  case 'i': return "Sunshine/alt.";
  case 'K': return "Koch";
  case 'L': return "Pullen SD";
  case 'M': return "Morinus";
  case 'N': return "equal/1=Aries";
  case 'O': return "Porphyry";
  case 'Q': return "Pullen SR";
  case 'R': return "Regiomontanus";
  case 'S': return "Sripati";
  case 'T': return "Polich/Page";
  case 'U': return "Krusinski-Pisa-Goelzer";
  case 'V': return "equal/Vehlow";
  case 'W': return "equal/ whole sign";
  case 'X': return "axial rotation system/Meridian houses";
  case 'Y': return "APC houses";
  default: return "Placidus";
  }
}

// How to deal with Sunshine houses if the southern crossing point of Equator
// and Ecliptic is under the horizon:
// We follow the proposal by Dieter Koch, who wants to keep it in alalogy with
// Regiomontanus, where we keep the MC above the horozon, by switching it to the noth.
// This results in an clockwise sequence of house cusps in the chart.
//
// One can argue that the MC should be kept south, even when it is under the horizon.
// This would keep the sequence of houses in the chart counterclockwise as usual.
// To achieve it, the offsets on the diurnal arcs must be inverted.
#define SUNSHINE_KEEP_MC_SOUTH	0		// must be 0 or 1

double swi_armc_to_mc(double armc, double eps)
{
  double tant, mc;
  if (fabs(armc - 90) > VERY_SMALL
      && fabs(armc - 270) > VERY_SMALL) {
    tant = tand(armc);
    mc = atand(tant / cosd(eps));
    if (armc > 90 && armc <= 270)
      mc = swe_degnorm(mc + 180);
  } else {
    if (fabs(armc - 90) <= VERY_SMALL)
      mc = 90;
    else
      mc = 270;
  } /*  if */
  return mc;
}

static int CalcH(
	double th, double fi, double ekl, char hsy,
	int iteration_count, struct houses *hsp)
/* *********************************************************
 *  Arguments: th = sidereal time (angle 0..360 degrees
 *             hsy = letter code for house system;
 *                   A  equal
 *                   E  equal
 *                   B  Alcabitius
 *                   C  Campanus
 *                   D  equal (MC)
 *                   F  Carter "Poli-Equatorial"
 *                   G  36 Gauquelin sectors
 *                   H  horizon / azimut
 *                   I  Sunshine solution Treindl
 *                   i  Sunshine solution Makransky
 *                   K  Koch
 *                   L  Pullen SD "sinusoidal delta", ex Neo-Porphyry
 *                   M  Morinus
 *                   N	equal/1=Aries
 *                   O  Porphyry
 *                   P  Placidus
 *                   Q  Pullen SR "sinusoidal ratio"
 *                   R  Regiomontanus
 *                   S	Sripati
 *                   T  Polich/Page ("topocentric")
 *                   U  Krusinski-Pisa-Goelzer
 *                   V  equal Vehlow
 *                   W  equal, whole sign
 *                   X  axial rotation system/ Meridian houses
 *                   Y  APC houses
 *             fi = geographic latitude
 *             ekl = obliquity of the ecliptic
 *             iteration_count = number of iterations in
 *             Placidus calculation; can be 1 or 2.
 * *********************************************************
 *  Koch and Placidus don't work in the polar circle.
 *  We swap MC/IC so that MC is always before AC in the zodiac
 *  We than divide the quadrants into 3 equal parts.
 * *********************************************************
 *  All angles are expressed in degrees.
 *  Special trigonometric functions sind, cosd etc. are
 *  implemented for arguments in degrees.
 ***********************************************************/
{
  double tane, tanfi, cosfi, tant, sina, cosa, th2;
  double a, c, f, fh1, fh2, xh1, xh2, rectasc, ad3, acmc, vemc;
  int 	i, ih, ih2, retc = OK;
  double sine, cose;
  double x[3], krHorizonLon; /* BK 14.02.2006 */
  *hsp->serr = '\0';
  cose  = cosd(ekl);
  sine  = sind(ekl);
  tane  = tand(ekl);
  /* north and south poles */
  if (fabs(fabs(fi) - 90) < VERY_SMALL) {
    if (fi < 0)
      fi = -90 + VERY_SMALL;
    else
      fi = 90 - VERY_SMALL;
  }
  tanfi = tand(fi);
  /* mc */
  if (fabs(th - 90) > VERY_SMALL
      && fabs(th - 270) > VERY_SMALL) {
    tant = tand(th);
    hsp->mc = atand(tant / cose);
    if (th > 90 && th <= 270)
      hsp->mc = swe_degnorm(hsp->mc + 180);
  } else {
    if (fabs(th - 90) <= VERY_SMALL)
      hsp->mc = 90;
    else
      hsp->mc = 270;
  } /*  if */
  hsp->mc = swe_degnorm(hsp->mc);
  /* ascendant */
  hsp->ac = Asc1(th + 90, fi, sine, cose);
  hsp->cusp[1] = hsp->ac;
  hsp->cusp[10] = hsp->mc;
  /* we respect smaller case letter for i, otherwise they are deprecated */
  if (hsy > 95 && hsy != 'i') {
    sprintf(hsp->serr, "use of lower case letters like %c for house systems is deprecated", hsy);
    hsy = (char) (hsy - 32);/* translate into capital letter */
  }
  switch (hsy) {
  case 'A':	/* equal houses */
  case 'E':
    acmc = swe_difdeg2n(hsp->ac, hsp->mc);
    if (acmc < 0) {
      /* within polar circle we swap AC/DC if AC is on wrong side */
      hsp->ac = swe_degnorm(hsp->ac + 180);
      hsp->cusp[1] = hsp->ac;
    }
    for (i = 2; i <=12; i++)
      hsp->cusp[i] = swe_degnorm(hsp->cusp[1] + (i-1) * 30);
    break;
  case 'D':	/* equal, begin  at MC */
    acmc = swe_difdeg2n(hsp->ac, hsp->mc);
    if (acmc < 0) {
      /* within polar circle we swap AC/DC if AC is on wrong side */
      hsp->ac = swe_degnorm(hsp->ac + 180);
    }
    hsp->cusp[10] = hsp->mc;
    for (i = 11; i <= 12; i++) 
      hsp->cusp[i] = swe_degnorm(hsp->cusp[10] + (i-10) * 30);
    for (i = 1; i <= 9; i++) 
      hsp->cusp[i] = swe_degnorm(hsp->cusp[10] + (i + 2) * 30);
    break;
  case 'C': /* Campanus houses and Horizon or Azimut system */
  case 'H':
    if (hsy == 'H') {
      if (fi > 0)
        fi = 90 - fi;
      else
        fi = -90 - fi;
      /* equator */
      if (fabs(fabs(fi) - 90) < VERY_SMALL) {
        if (fi < 0)
          fi = -90 + VERY_SMALL;
        else
	  fi = 90 - VERY_SMALL;
      } 
      th = swe_degnorm(th + 180);
    }
    fh1 = asind(sind (fi) / 2);
    fh2 = asind(sqrt (3.0) / 2 * sind(fi));
    cosfi = cosd(fi);
    if (fabs(cosfi) == 0) {	/* '==' should be save! */ 
      if (fi > 0)
	xh1 = xh2 = 90; /* cosfi = VERY_SMALL; */
      else
	xh1 = xh2 = 270; /* cosfi = -VERY_SMALL; */
    } else {
      xh1 = atand(sqrt (3.0) / cosfi);
      xh2 = atand(1 / sqrt (3.0) / cosfi);
    }
    hsp->cusp[11] = Asc1(th + 90 - xh1, fh1, sine, cose);
    hsp->cusp[12] = Asc1(th + 90 - xh2, fh2, sine, cose);
    if (hsy == 'H') 
      hsp->cusp[1] = Asc1(th + 90, fi, sine, cose);
    hsp->cusp[2] = Asc1(th + 90 + xh2, fh2, sine, cose);
    hsp->cusp[3] = Asc1(th + 90 + xh1, fh1, sine, cose);
    /* within polar circle, when mc sinks below horizon and 
	 * ascendant changes to western hemisphere, all cusps
     * must be added 180 degrees. 
     * houses will be in clockwise direction */
    if (fabs(fi) >= 90 - ekl) {  /* within polar circle */
      acmc = swe_difdeg2n(hsp->ac, hsp->mc);
      if (acmc < 0) {
        hsp->ac = swe_degnorm(hsp->ac + 180);
        hsp->mc = swe_degnorm(hsp->mc + 180);
	for (i = 1; i <= 12; i++) {
	  if (i >= 4 && i < 10) continue;
	  hsp->cusp[i] = swe_degnorm(hsp->cusp[i] + 180);
        }
      }
    }
    if (hsy == 'H') {
      for (i = 1; i <= 3; i++)
        hsp->cusp[i] = swe_degnorm(hsp->cusp[i] + 180);
      for (i = 11; i <= 12; i++)
        hsp->cusp[i] = swe_degnorm(hsp->cusp[i] + 180);
      /* restore fi and th */
      if (fi > 0)
        fi = 90 - fi;
      else
	fi = -90 - fi;
      th = swe_degnorm(th + 180);
      acmc = swe_difdeg2n(hsp->ac, hsp->mc);
      if (acmc < 0) {
        hsp->ac = swe_degnorm(hsp->ac + 180);
      }
    }
    break;
  case 'I': /* Sunshine houses, solution Treindl */
  case 'i': /* Sunshine houses, solution Makranski */
    acmc = swe_difdeg2n(hsp->ac, hsp->mc);
    if (acmc < 0) {
      /* we shift axes */
      hsp->ac = swe_degnorm(hsp->ac + 180);
      hsp->cusp[1] = hsp->ac;
      if (! SUNSHINE_KEEP_MC_SOUTH && hsy == 'I') {
	hsp->mc = swe_degnorm(hsp->mc + 180);
	hsp->cusp[10] = hsp->mc;
      }
    }
    hsp->cusp[4] = swe_degnorm(hsp->cusp[10] + 180);
    hsp->cusp[7] = swe_degnorm(hsp->cusp[1] + 180);
    if (hsy == 'I') {
      retc = sunshine_solution_treindl(th, fi, ekl, hsp);
    } else {
      retc = sunshine_solution_makransky(th, fi, ekl, hsp);
    }
    if (retc == ERR) {	// only Makransky version does this
      strcpy(hsp->serr, "within polar circle, switched to Porphyry"); 
      hsy = 'O';
      goto porphyry;
    }
    break;
  case 'K': /* Koch houses */
    if (fabs(fi) >= 90 - ekl) {  /* within polar circle */
      retc = ERR;
      strcpy(hsp->serr, "within polar circle, switched to Porphyry"); 
      goto porphyry;
    }
    sina = sind(hsp->mc) * sine / cosd(fi);
    if (sina > 1) sina = 1;
    if (sina < -1) sina = -1;
    cosa = sqrt(1 - sina * sina);		/* always >> 0 */
    c = atand(tanfi / cosa);
    ad3 = asind(sind(c) * sina) / 3.0;
    hsp->cusp[11] = Asc1(th + 30 - 2 * ad3, fi, sine, cose);
    hsp->cusp[12] = Asc1(th + 60 - ad3, fi, sine, cose);
    hsp->cusp[2] = Asc1(th + 120 + ad3, fi, sine, cose);
    hsp->cusp[3] = Asc1(th + 150 + 2 * ad3, fi, sine, cose);
    break;
  case 'L':	/* Pullen SD sinusoidal delta, ex Neo-Porphyry */
    {
      double d, q1;
      acmc = swe_difdeg2n(hsp->ac, hsp->mc);
      if (acmc < 0) {
	/* within polar circle we swap AC/DC if AC is on wrong side */
	hsp->ac = swe_degnorm(hsp->ac + 180);
	hsp->cusp[1] = hsp->ac;
	acmc = swe_difdeg2n(hsp->ac, hsp->mc);
      } 
      q1 = 180 - acmc;
      d = (acmc - 90) / 4.0;
      if (acmc <= 30) {	// is quadrant <= 30, house 11 = zero width.
	hsp->cusp[11] = hsp->cusp[12] = swe_degnorm(hsp->mc + acmc / 2);
      } else {
	hsp->cusp[11] = swe_degnorm(hsp->mc + 30 + d);
	hsp->cusp[12] = swe_degnorm(hsp->mc + 60 + 3 * d);
      }
      d = (q1 - 90) / 4.0;
      if (q1 <= 30) {	// is quadrant <= 30, house 2 = zero width.
	hsp->cusp[2] = hsp->cusp[3] = swe_degnorm(hsp->ac + q1 / 2);
      } else {
	hsp->cusp[2] = swe_degnorm(hsp->ac + 30 + d);
	hsp->cusp[3] = swe_degnorm(hsp->ac + 60 + 3 * d);
      }
    }
    break;
  case 'N':	/* whole signs, begin at 0° Aries */
    acmc = swe_difdeg2n(hsp->ac, hsp->mc);
    if (acmc < 0) {
      /* within polar circle we swap AC/DC if AC is on wrong side */
      hsp->ac = swe_degnorm(hsp->ac + 180);
    }
    for (i = 1; i <= 12; i++) 
      hsp->cusp[i] = (i - 1) * 30.0;
    break;
  case 'O':	/* Porphyry houses */
porphyry:
    acmc = swe_difdeg2n(hsp->ac, hsp->mc);
    if (acmc < 0) {
      /* within polar circle we swap AC/DC if AC is on wrong side */
      hsp->ac = swe_degnorm(hsp->ac + 180);
      hsp->cusp[1] = hsp->ac;
      acmc = swe_difdeg2n(hsp->ac, hsp->mc);
    }
    hsp->cusp[2] = swe_degnorm(hsp->ac + (180 - acmc) / 3);
    hsp->cusp[3] = swe_degnorm(hsp->ac + (180 - acmc) / 3 * 2);
    hsp->cusp[11] = swe_degnorm(hsp->mc + acmc / 3);
    hsp->cusp[12] = swe_degnorm(hsp->mc + acmc / 3 * 2);
    break;
  case 'Q':	/* Pullen sinusoidal ratio */
    {
      double q, c, csq, ccr, cqx, two23, third, r, r1, r2, x, xr, xr3, xr4;
      third = 1.0 / 3.0;
      two23 = pow(2.0 * 2.0, third);        // 2^(2/3)
      acmc = swe_difdeg2n(hsp->ac, hsp->mc);
      if (acmc < 0) {
      /* within polar circle we swap AC/DC if AC is on wrong side */
       hsp->ac = swe_degnorm(hsp->ac + 180);
       hsp->cusp[1] = hsp->ac;
       acmc = swe_difdeg2n(hsp->ac, hsp->mc);
      }
      q = acmc;
      if (q > 90) q = 180 - q;
      if (q < 1e-30) {    // degenerate case of quadrant = zer0
	r = INFINITY;
	x = xr = xr3 = 0;
	xr4 = 180;
      } else {
	c = (180 - q) / q;
	csq = c * c;
	ccr = pow(csq - c, third);          // cuberoot(c^2 -c)
	cqx = sqrt(two23 * ccr + 1.0);      // sqrt{2^(2/3)*cuberoot(c^2-c) + 1}
	r1 = 0.5 * cqx;
	r2 = 0.5 * sqrt(-2*(1-2*c) / cqx - two23 * ccr + 2);
	r = r1 + r2 - 0.5;
	x = q / (2 * r + 1);
	xr = r * x;
	xr3 = xr * r * r;
	xr4 = xr3 * r;
      }
      if (acmc > 90) {
	hsp->cusp[11] = swe_degnorm(hsp->mc + xr3);	// house 10 and 12 size xr^3
	hsp->cusp[12] = swe_degnorm(hsp->cusp[11] + xr4);	// house 11 size xr^4
	hsp->cusp[2] = swe_degnorm(hsp->ac + xr);	// house 1 and 3 size xr
	hsp->cusp[3] = swe_degnorm(hsp->cusp[2] + x);	// house 2 size x
      } else {
	hsp->cusp[11] = swe_degnorm(hsp->mc + xr);	// house 10 and 12 size xr
	hsp->cusp[12] = swe_degnorm(hsp->cusp[11] + x);	// house 11 size x
	hsp->cusp[2] = swe_degnorm(hsp->ac + xr3);	// house 1 and 3 size xr^3
	hsp->cusp[3] = swe_degnorm(hsp->cusp[2] + xr4);	// house 2 size xr^4
      }
    }
    break;
  case 'R':	/* Regiomontanus houses */
    fh1 = atand (tanfi * 0.5);
    fh2 = atand (tanfi * cosd(30));
    hsp->cusp[11] = Asc1(30 + th, fh1, sine, cose); 
    hsp->cusp[12] = Asc1(60 + th, fh2, sine, cose); 
    hsp->cusp[2] = Asc1(120 + th, fh2, sine, cose);
    hsp->cusp[3] = Asc1(150 + th, fh1, sine, cose); 
    /* within polar circle, when mc sinks below horizon and 
     * ascendant changes to western hemisphere, all cusps
     * must be added 180 degrees.
     * houses will be in clockwise direction */
    if (fabs(fi) >= 90 - ekl) {  /* within polar circle */
      acmc = swe_difdeg2n(hsp->ac, hsp->mc);
      if (acmc < 0) {
        hsp->ac = swe_degnorm(hsp->ac + 180);
        hsp->mc = swe_degnorm(hsp->mc + 180);
	for (i = 1; i <= 12; i++) {
	  if (i >= 4 && i < 10) continue;
	  hsp->cusp[i] = swe_degnorm(hsp->cusp[i] + 180);
        }
      }
    }
    break;
  case 'S':	/* Sripati houses */
    /* uses Porphyry sectors, but then takes middle of sectors as cusps */
    {
      double s1, s4, q1;
      acmc = swe_difdeg2n(hsp->ac, hsp->mc);	// size of 4th quadrant
      if (acmc < 0) {
	/* within polar circle we swap AC/DC if AC is on wrong side */
	hsp->ac = swe_degnorm(hsp->ac + 180);
	acmc = swe_difdeg2n(hsp->ac, hsp->mc);
      }
      q1 = 180 - acmc;	// size of 1st quadrant
      s1 = q1 / 3.0;
      s4 = acmc / 3.0;
      hsp->cusp[1] = swe_degnorm(hsp->ac - s4 * 0.5);
      hsp->cusp[2] = swe_degnorm(hsp->ac + s1 * 0.5);
      hsp->cusp[3] = swe_degnorm(hsp->ac + s1 * 1.5);
      hsp->cusp[10] = swe_degnorm(hsp->mc - s1 * 0.5);
      hsp->cusp[11] = swe_degnorm(hsp->mc + s4 * 0.5);
      hsp->cusp[12] = swe_degnorm(hsp->mc + s4 * 1.5);
    }
    break;
  case 'T':	/* 'topocentric' houses */
    fh1 = atand (tanfi / 3.0);
    fh2 = atand (tanfi * 2.0 / 3.0);
    hsp->cusp[11] =  Asc1(30 + th, fh1, sine, cose); 
    hsp->cusp[12] =  Asc1(60 + th, fh2, sine, cose);
    hsp->cusp[2] =  Asc1(120 + th, fh2, sine, cose); 
    hsp->cusp[3] =  Asc1(150 + th, fh1, sine, cose);
    /* within polar circle, when mc sinks below horizon and 
     * ascendant changes to western hemisphere, all cusps
     * must be added 180 degrees.
	 * houses will be in clockwise direction */
    if (fabs(fi) >= 90 - ekl) {  /* within polar circle */
      acmc = swe_difdeg2n(hsp->ac, hsp->mc);
      if (acmc < 0) {
        hsp->ac = swe_degnorm(hsp->ac + 180);
	hsp->mc = swe_degnorm(hsp->mc + 180);
	for (i = 1; i <= 12; i++)
	  hsp->cusp[i] = swe_degnorm(hsp->cusp[i] + 180);
      }
    }
    break;
  case 'V':	/* equal houses after Vehlow */
    acmc = swe_difdeg2n(hsp->ac, hsp->mc);
    if (acmc < 0) {
      /* within polar circle we swap AC/DC if AC is on wrong side */
      hsp->ac = swe_degnorm(hsp->ac + 180);
      hsp->cusp[1] = hsp->ac;
    }
    hsp->cusp[1] = swe_degnorm(hsp->ac - 15);
    for (i = 2; i <=12; i++)
      hsp->cusp[i] = swe_degnorm(hsp->cusp[1] + (i-1) * 30);
    break;
  case 'W':	/* equal, whole-sign houses */
    acmc = swe_difdeg2n(hsp->ac, hsp->mc);
    if (acmc < 0) {
      /* within polar circle we swap AC/DC if AC is on wrong side */
      hsp->ac = swe_degnorm(hsp->ac + 180);
      hsp->cusp[1] = hsp->ac;
    }
    hsp->cusp[1] = hsp->ac - fmod(hsp->ac, 30);
    for (i = 2; i <=12; i++)
      hsp->cusp[i] = swe_degnorm(hsp->cusp[1] + (i-1) * 30);
    break;
  case 'X': {
    /*
     * Meridian or axial rotation system:
     * ecliptic points whose rectascensions
     * are armc + n * 30
     */
    int j;
    double a = th;
    for (i = 1; i <= 12; i++) {
      j = i + 10;
      if (j > 12) j -= 12;
      a = swe_degnorm(a + 30);
	  if (fabs(a - 90) > VERY_SMALL
        && fabs(a - 270) > VERY_SMALL) {
        tant = tand(a);
        hsp->cusp[j] = atand(tant / cose);
        if (a > 90 && a <= 270) 
          hsp->cusp[j] = swe_degnorm(hsp->cusp[j] + 180);
      } else {
        if (fabs(a - 90) <= VERY_SMALL)
          hsp->cusp[j] = 90;
        else 
		  hsp->cusp[j] = 270;
      } /*  if */
	  hsp->cusp[j] = swe_degnorm(hsp->cusp[j]);
    }
    acmc = swe_difdeg2n(hsp->ac, hsp->mc);
    if (acmc < 0) {
      hsp->ac = swe_degnorm(hsp->ac + 180);
    }
    break; }
  case 'M': {
    /* 
     * Morinus
     * points of the equator (armc + n * 30) are transformed
     * into the ecliptic coordinate system
     */
    int j;
    double a = th;
    double x[3];
    for (i = 1; i <= 12; i++) {
      j = i + 10;
      if (j > 12) j -= 12;
      a = swe_degnorm(a + 30);
      x[0] = a;
      x[1] = 0;
      swe_cotrans(x, x, ekl);
      hsp->cusp[j] = x[0];
    }
    acmc = swe_difdeg2n(hsp->ac, hsp->mc);
    if (acmc < 0) {
      hsp->ac = swe_degnorm(hsp->ac + 180);
    }
    break; }
  case 'F': {
    /* 
    * Carter poli-equatorial
    * Rectascension a of ascendant is the starting point.
    * house cusps nh on the ecliptic are the points where
    * great circles through points of the equator (a + (nh -1) * 30) 
    * and the poles intersect it.
    */
    double a, ra;
    double x[3];
    acmc = swe_difdeg2n(hsp->ac, hsp->mc);
    if (acmc < 0) {
      /* within polar circle we swap AC/DC if AC is on wrong side */
      hsp->ac = swe_degnorm(hsp->ac + 180);
      hsp->cusp[1] = hsp->ac;
    }
    x[0] = hsp->ac;
    x[1] = 0;
    swe_cotrans(x, x, -ekl);
    a = x[0];   /* rectascension of ascendant */
    for (i = 2; i <= 12; i++) {
      if (i <= 3 || i >= 10) {
        ra = swe_degnorm(a + (i - 1) * 30);
	if (fabs(ra - 90) > VERY_SMALL
	  && fabs(ra - 270) > VERY_SMALL) {
	  tant = tand(ra);
	  hsp->cusp[i] = atand(tant / cose);
	  if (ra > 90 && ra <= 270) 
	    hsp->cusp[i] = swe_degnorm(hsp->cusp[i] + 180);
	} else {
	  if (fabs(ra - 90) <= VERY_SMALL)
	    hsp->cusp[i] = 90;
	  else 
	    hsp->cusp[i] = 270;
	} /*  if */
	hsp->cusp[i] = swe_degnorm(hsp->cusp[i]);
      }
    }
    break; }
  case 'B': {	/* Alcabitius */
    /* created by Alois 17-sep-2000, followed example in Matrix
       electrical library. The code reproduces the example!
       I think the Alcabitius code in Walter Pullen's Astrolog 5.40
       is wrong, because he remains in RA and forgets the transform to
       the ecliptic. */
    double dek, r, sna, sda, sn3, sd3;
    acmc = swe_difdeg2n(hsp->ac, hsp->mc);
    if (acmc < 0) {
      hsp->ac = swe_degnorm(hsp->ac + 180);
      hsp->cusp[1] = hsp->ac;
      acmc = swe_difdeg2n(hsp->ac, hsp->mc);
    }
    dek = asind(sind(hsp->ac) * sine);	/* declination of Ascendant */
    /* must treat the case fi == 90 or -90 */
    r = -tanfi * tand(dek);
    /* must treat the case of abs(r) > 1; probably does not happen
     * because dek becomes smaller when fi is large, as ac is close to
     * zero Aries/Libra in that case.
     */
    sda = acos(r) * RADTODEG;	/* semidiurnal arc, measured on equator */
    sna = 180 - sda;		/* complement, seminocturnal arc */
    sd3 = sda / 3;
    sn3 = sna / 3;
    rectasc = swe_degnorm(th + sd3);	/* cusp 11 */
    /* project rectasc onto eclipitic with pole height 0, i.e. along the
    declination circle */
    hsp->cusp[11] = Asc1(rectasc, 0, sine, cose);
    rectasc = swe_degnorm(th + 2 * sd3);	/* cusp 12 */
    hsp->cusp[12] = Asc1(rectasc, 0, sine, cose);
    rectasc = swe_degnorm(th + 180 - 2 * sn3);	/* cusp 2 */
    hsp->cusp[2] = Asc1(rectasc, 0, sine, cose);
    rectasc = swe_degnorm(th + 180 -  sn3);	/* cusp 3 */
    hsp->cusp[3] = Asc1(rectasc, 0, sine, cose);
    }
    break;
  case 'G': 	/* 36 Gauquelin sectors */
    for (i = 1; i <= 36; i++) {
      hsp->cusp[i] = 0;
    }
    if (fabs(fi) >= 90 - ekl) {  /* within polar circle */
      retc = ERR;
      strcpy(hsp->serr, "within polar circle, switched to Porphyry"); 
      goto porphyry;
    }
    /*************** forth/second quarter ***************/
    /* note: Gauquelin sectors are counted in clockwise direction */
    a = asind(tand(fi) * tane);
    for (ih = 2; ih <= 9; ih++) {
      ih2 = 10 - ih;
      fh1 = atand(sind(a * ih2 / 9) / tane);
      rectasc = swe_degnorm((90 / 9) * ih2 + th);
      tant = tand(asind(sine * sind(Asc1(rectasc, fh1, sine, cose))));
      if (fabs(tant) < VERY_SMALL) {
	hsp->cusp[ih] = rectasc;
      } else {
	/* pole height */
	f = atand(sind(asind(tanfi * tant) * ih2 / 9)  /tant);
        hsp->cusp[ih] = Asc1(rectasc, f, sine, cose);
        for (i = 1; i <= iteration_count; i++) {
	  tant = tand(asind(sine * sind(hsp->cusp[ih])));
	  if (fabs(tant) < VERY_SMALL) {
	    hsp->cusp[ih] = rectasc;
	    break;
	  }
	  /* pole height */
	  f = atand(sind(asind(tanfi * tant) * ih2 / 9) / tant);
  	  hsp->cusp[ih] = Asc1(rectasc, f, sine, cose);
        }
      }
      hsp->cusp[ih+18] = swe_degnorm(hsp->cusp[ih] + 180);
    }
    /*************** first/third quarter ***************/
    for (ih = 29; ih <= 36; ih++) {
      ih2 = ih - 28;
      fh1 = atand(sind(a * ih2 / 9) / tane);
      rectasc = swe_degnorm(180 - ih2 * 90 / 9 + th);
      tant = tand(asind(sine * sind(Asc1(rectasc, fh1, sine, cose))));
      if (fabs(tant) < VERY_SMALL) {
        hsp->cusp[ih] = rectasc;
      } else {
        f = atand(sind(asind(tanfi * tant) * ih2 / 9) / tant);
        /*  pole height */
        hsp->cusp[ih] = Asc1(rectasc, f, sine, cose);
        for (i = 1; i <= iteration_count; i++) {
	  tant = tand(asind(sine * sind(hsp->cusp[ih])));
	  if (fabs(tant) < VERY_SMALL) {
	    hsp->cusp[ih] = rectasc;
	    break;
	  }
	  f = atand(sind(asind(tanfi * tant) * ih2 / 9) / tant);
	  /*  pole height */
  	  hsp->cusp[ih] = Asc1(rectasc, f, sine, cose);
	}
      }
      hsp->cusp[ih-18] = swe_degnorm(hsp->cusp[ih] + 180);
    }
    hsp->cusp[1] = hsp->ac;
    hsp->cusp[10] = hsp->mc;
    hsp->cusp[19] = swe_degnorm(hsp->ac + 180);
    hsp->cusp[28] = swe_degnorm(hsp->mc + 180);
    break;
  case 'U': /* Krusinski-Pisa */
    /*
     * The following code was written by Bogdan Krusinski in 2006.
     * bogdan@astrologia.pl
     *
     * Definition:
     * "Krusinski - house system based on the great circle passing through 
     * ascendant and zenith. This circle is divided into 12 equal parts 
     * (1st cusp is ascendent, 10th cusp is zenith), then the resulting 
     * points are projected onto the ecliptic through meridian circles.
     * The house cusps in space are half-circles perpendicular to the equator
     * and running from the north to the south celestial pole through the
     * resulting cusp points on the house circle. The points where they 
     * cross the ecliptic mark the ecliptic house cusps."
     *
     * Description of the algorithm:
     * Transform into great circle running through Asc and zenit (where arc 
     * between Asc and zenith is always 90 deg), and then return with 
     * house cusps into ecliptic. Eg. solve trigonometrical triangle 
     * with three transformations and two rotations starting from ecliptic. 
     * House cusps in space are meridian circles. 
     *
     * Notes:
     * 1. In this definition we assume MC on ecliptic as point where
     *    half-meridian (from north to south pole) cuts ecliptic,
     *    so MC may be below horizon in arctic regions.
     * 2. Houses could be calculated in all latitudes except the poles 
     *    themselves (-90,90) and points on arctic circle in cases where 
     *    ecliptic is equal to horizon and then ascendant is undefined. 
     *    But ascendant when 'horizon=ecliptic' could be deduced as limes 
     *    from both sides of that point and houses with that provision can 
     *    be computed also there.
     *
     * Starting values for calculations:
     *	   - Asc ecliptic longitude
     *	   - right ascension of MC (RAMC)
     *	   - geographic latitude.
     */
    /*
     * within polar circle we swap AC/DC if AC is on wrong side
     */
    acmc = swe_difdeg2n(hsp->ac, hsp->mc);
    if (acmc < 0) {
      hsp->ac = swe_degnorm(hsp->ac + 180);
    }
    /* A0. Start point - ecliptic coords of ascendant */
    x[0] = hsp->ac; /* Asc longitude   */
    x[1] = 0.0;     /* Asc declination */
    x[2] = 1.0;     /* Radius to test validity of subsequent transformations. */
    swe_cotrans(x, x, -ekl);      /* A1. Transform into equatorial coords */
    x[0] = x[0] - (th-90);        /* A2. Rotate                           */
    swe_cotrans(x, x, -(90-fi));  /* A3. Transform into horizontal coords */
    krHorizonLon = x[0];          /* ...save asc lon on horizon to get back later with house cusp */
    x[0] = x[0] - x[0];           /* A4. Rotate                           */
    swe_cotrans(x, x, -90);       /* A5. Transform into this house system great circle (asc-zenith) */
    /* As it is house circle now, simple add 30 deg increments... */
    for(i = 0; i < 6; i++) {
      /* B0. Set 'n-th' house cusp. 
       *     Note that IC/MC are also calculated here to check 
       *     if really this is the asc-zenith great circle. */
      x[0] = 30.0*i;
      x[1] = 0.0;
      swe_cotrans(x, x, 90);                 /* B1. Transform back into horizontal coords */
      x[0] = x[0] + krHorizonLon;            /* B2. Rotate back.                          */
      swe_cotrans(x, x, 90-fi);              /* B3. Transform back into equatorial coords */
      x[0] = swe_degnorm(x[0] + (th-90));    /* B4. Rotate back -> RA of house cusp as result. */
      /* B5. Where's this house cusp on ecliptic? */
      /* ... so last but not least - get ecliptic longitude of house cusp: */
      hsp->cusp[i+1] = atand(tand(x[0])/cosd(ekl));
      if (x[0] > 90 && x[0] <= 270)
	hsp->cusp[i+1] = swe_degnorm(hsp->cusp[i+1] + 180);
      hsp->cusp[i+1] = swe_degnorm(hsp->cusp[i+1]);
      hsp->cusp[i+7] = swe_degnorm(hsp->cusp[i+1]+180);
    }
    break;
  case 'Y':     /* APC houses */
    for (i = 1; i <= 12; i++) {
      hsp->cusp[i] = apc_sector(i, fi * DEGTORAD, ekl * DEGTORAD, th * DEGTORAD);
    }
    //hsp->ac = hsp->cusp[1];
    //hsp->mc = hsp->cusp[10];
    /* note the MC provided by apc_sector() near latitude 90 is not accurate */
    hsp->cusp[10] = hsp->mc;
    hsp->cusp[4] = swe_degnorm(hsp->mc + 180);
    /* within polar circle, when mc sinks below horizon and 
     * ascendant changes to western hemisphere, all cusps
     * must be added 180 degrees. 
     * houses will be in clockwise direction */
    if (fabs(fi) >= 90 - ekl) {  /* within polar circle */
      acmc = swe_difdeg2n(hsp->ac, hsp->mc);
      if (acmc < 0) {
        hsp->ac = swe_degnorm(hsp->ac + 180);
        hsp->mc = swe_degnorm(hsp->mc + 180);
	for (i = 1; i <= 12; i++)
	  hsp->cusp[i] = swe_degnorm(hsp->cusp[i] + 180);
      }
    }
    break;
  default:	/* Placidus houses */
    if (fabs(fi) >= 90 - ekl) {  /* within polar circle */
      retc = ERR;
      strcpy(hsp->serr, "within polar circle, switched to Porphyry"); 
      goto porphyry;
    } 
    a = asind(tand(fi) * tane);
    fh1 = atand(sind(a / 3) / tane);
    fh2 = atand(sind(a * 2 / 3) / tane);
    /* ************  house 11 ******************** */
    rectasc = swe_degnorm(30 + th);
    tant = tand(asind(sine * sind(Asc1(rectasc, fh1, sine, cose))));
    if (fabs(tant) < VERY_SMALL) {
      hsp->cusp[11] = rectasc;
    } else {
      /* pole height */
      f = atand(sind(asind(tanfi * tant) / 3)  /tant);  
      hsp->cusp[11] = Asc1(rectasc, f, sine, cose);
      for (i = 1; i <= iteration_count; i++) {
	tant = tand(asind(sine * sind(hsp->cusp[11])));
	if (fabs(tant) < VERY_SMALL) {
	  hsp->cusp[11] = rectasc;
	  break;
	}
	/* pole height */
	f = atand(sind(asind(tanfi * tant) / 3) / tant);
	hsp->cusp[11] = Asc1(rectasc, f, sine, cose);
      }
    }
    /* ************  house 12 ******************** */
    rectasc = swe_degnorm(60 + th);
    tant = tand(asind(sine*sind(Asc1(rectasc,  fh2, sine, cose))));
    if (fabs(tant) < VERY_SMALL) {
      hsp->cusp[12] = rectasc;
    } else {
      f = atand(sind(asind(tanfi * tant) / 1.5) / tant);  
      /*  pole height */
      hsp->cusp[12] = Asc1(rectasc, f, sine, cose);
      for (i = 1; i <= iteration_count; i++) {
	tant = tand(asind(sine * sind(hsp->cusp[12])));
	if (fabs(tant) < VERY_SMALL) {
	  hsp->cusp[12] = rectasc;
	  break;
	}
	f = atand(sind(asind(tanfi * tant) / 1.5) / tant);  
	/*  pole height */
	hsp->cusp[12] = Asc1(rectasc, f, sine, cose);
      }
    }
    /* ************  house  2 ******************** */
    rectasc = swe_degnorm(120 + th);
    tant = tand(asind(sine * sind(Asc1(rectasc, fh2, sine, cose))));
	if (fabs(tant) < VERY_SMALL) {
      hsp->cusp[2] = rectasc;
    } else {
      f = atand(sind(asind(tanfi * tant) / 1.5) / tant);
      /*  pole height */
      hsp->cusp[2] = Asc1(rectasc, f, sine, cose);
      for (i = 1; i <= iteration_count; i++) {
	tant = tand(asind(sine * sind(hsp->cusp[2])));
	if (fabs(tant) < VERY_SMALL) {
	  hsp->cusp[2] = rectasc;
	  break;
	}
	f = atand(sind(asind(tanfi * tant) / 1.5) / tant);
	/*  pole height */
	hsp->cusp[2] = Asc1(rectasc, f, sine, cose);
      }
    }
    /* ************  house  3 ******************** */
    rectasc = swe_degnorm(150 + th);
    tant = tand(asind(sine * sind(Asc1(rectasc, fh1, sine, cose))));
    if (fabs(tant) < VERY_SMALL) {
      hsp->cusp[3] = rectasc;
    } else {
      f = atand(sind(asind(tanfi * tant) / 3) / tant);  
      /*  pole height */
      hsp->cusp[3] = Asc1(rectasc, f, sine, cose);
      for (i = 1; i <= iteration_count; i++) {
	tant = tand(asind(sine * sind(hsp->cusp[3])));
	if (fabs(tant) < VERY_SMALL) {
	  hsp->cusp[3] = rectasc;
	  break;
	}
	f = atand(sind(asind(tanfi * tant) / 3) / tant);
	/*  pole height */
	hsp->cusp[3] = Asc1(rectasc, f, sine, cose);
      }
    }
    break;
  } /* end switch */
  if (hsy != 'G' && hsy != 'Y' && toupper(hsy) != 'I') {
    hsp->cusp[4] = swe_degnorm(hsp->cusp[10] + 180);
    hsp->cusp[5] = swe_degnorm(hsp->cusp[11] + 180);
    hsp->cusp[6] = swe_degnorm(hsp->cusp[12] + 180);
    hsp->cusp[7] = swe_degnorm(hsp->cusp[1] + 180);
    hsp->cusp[8] = swe_degnorm(hsp->cusp[2] + 180);
    hsp->cusp[9] = swe_degnorm(hsp->cusp[3] + 180);
  }
  /* vertex */
  if (fi >= 0)
    f = 90 - fi;
  else
    f = -90 - fi;
  hsp->vertex = Asc1(th - 90, f, sine, cose);
  /* with tropical latitudes, the vertex behaves strange, 
   * in a similar way as the ascendant within the polar
   * circle. we keep it always on the western hemisphere.*/
  if (fabs(fi) <= ekl) {
	vemc = swe_difdeg2n(hsp->vertex, hsp->mc);
    if (vemc > 0)
      hsp->vertex = swe_degnorm(hsp->vertex + 180);
  }
  /* 
   * some strange points:
   */
  /* equasc (equatorial ascendant) */
  th2 = swe_degnorm(th + 90);
  if (fabs(th2 - 90) > VERY_SMALL
    && fabs(th2 - 270) > VERY_SMALL) {
    tant = tand(th2);
    hsp->equasc = atand(tant / cose);
    if (th2 > 90 && th2 <= 270)
      hsp->equasc = swe_degnorm(hsp->equasc + 180);
  } else {
    if (fabs(th2 - 90) <= VERY_SMALL)
      hsp->equasc = 90;
    else
      hsp->equasc = 270;
  } /*  if */
  hsp->equasc = swe_degnorm(hsp->equasc);
  /* "co-ascendant" W. Koch */
  hsp->coasc1 = swe_degnorm(Asc1(th - 90, fi, sine, cose) + 180);
  /* "co-ascendant" M. Munkasey */
  if (fi >= 0)
    hsp->coasc2 = Asc1(th + 90, 90 - fi, sine, cose);
  else /* southern hemisphere */
    hsp->coasc2 = Asc1(th + 90, -90 - fi, sine, cose);
  /* "polar ascendant" M. Munkasey */
  hsp->polasc = Asc1(th - 90, fi, sine, cose);
#if 0
  test_Asc1();
#endif
  return retc;
} /* procedure houses */

/******************************/
static double Asc1(double x1, double f, double sine, double cose) 
{ 
  int n;
  double ass;
  x1 = swe_degnorm(x1);
  n  = (int) ((x1 / 90) + 1);	// n is quadrant 1..4
  if (fabs(90 - f) < VERY_SMALL) { // near north pole
    return 180;
  }
  if (fabs(90 + f) < VERY_SMALL) { // near south pole
    return 0;
  }
  if (n == 1)
    ass = ( Asc2(x1, f, sine, cose));
  else if (n == 2) 
    ass = (180 - Asc2(180 - x1, - f, sine, cose));
  else if (n == 3)
    ass = (180 + Asc2(x1 - 180, - f, sine, cose));
  else
    ass = (360 - Asc2(360- x1,  f, sine, cose));
  ass = swe_degnorm(ass);
  if (fabs(ass - 90) < VERY_SMALL)	/* rounding, e.g.: if */
	ass = 90;				/* fi = 0 & st = 0, ac = 89.999... */
  if (fabs(ass - 180) < VERY_SMALL)
    ass = 180;
  if (fabs(ass - 270) < VERY_SMALL)	/* rounding, e.g.: if */
    ass = 270;				/* fi = 0 & st = 0, ac = 89.999... */
  if (fabs(ass - 360) < VERY_SMALL)
    ass = 0;
  return ass;
}  /* Asc1 */

#if 0
/******************************/
static double Asc1_old(double x1, double f, double sine, double cose) 
{ 
  int n;
  double ass;
  if (f == -90) f += VERY_SMALL / 1000;        // avoid exact pole 90, as tan() goes infinite
  if (f == 90) f -= VERY_SMALL / 1000;
  x1 = swe_degnorm(x1);
  n  = (int) ((x1 / 90) + 1);
  if (n == 1)
    ass = ( Asc2(x1, f, sine, cose));
  else if (n == 2) 
    ass = (180 - Asc2(180 - x1, - f, sine, cose));
  else if (n == 3)
    ass = (180 + Asc2(x1 - 180, - f, sine, cose));
  else
    ass = (360 - Asc2(360- x1,  f, sine, cose));
  ass = swe_degnorm(ass);
  if (fabs(ass - 90) < VERY_SMALL)	/* rounding, e.g.: if */
	ass = 90;				/* fi = 0 & st = 0, ac = 89.999... */
  if (fabs(ass - 180) < VERY_SMALL)
    ass = 180;
  if (fabs(ass - 270) < VERY_SMALL)	/* rounding, e.g.: if */
    ass = 270;				/* fi = 0 & st = 0, ac = 89.999... */
  if (fabs(ass - 360) < VERY_SMALL)
    ass = 0;
  return ass;
}  /* Asc1 */

/******************************/
static double Asc1_old_old (double x1, double f, double sine, double cose) 
{ 
  int n;
  double ass;
  x1 = swe_degnorm(x1);
  n  = (int) ((x1 / 90) + 1);
  if (n == 1)
    ass = ( Asc2 (x1, f, sine, cose));
  else if (n == 2) 
    ass = (180 - Asc2 (180 - x1, - f, sine, cose));
  else if (n == 3)
    ass = (180 + Asc2 (x1 - 180, - f, sine, cose));
  else
    ass = (360 - Asc2 (360- x1,  f, sine, cose));
  ass = swe_degnorm(ass);
  if (fabs(ass - 90) < VERY_SMALL)	/* rounding, e.g.: if */
	ass = 90;				/* fi = 0 & st = 0, ac = 89.999... */
  if (fabs(ass - 180) < VERY_SMALL)
    ass = 180;
  if (fabs(ass - 270) < VERY_SMALL)	/* rounding, e.g.: if */
    ass = 270;				/* fi = 0 & st = 0, ac = 89.999... */
  if (fabs(ass - 360) < VERY_SMALL)
    ass = 0;
  return ass;
}  /* Asc1 */

static void test_Asc1()
{
  double armc, dlat, eps = 23.44, asc1, asc1_old, sine, cose;
  sine = sind(eps);
  cose = cosd(eps);
  fprintf(stderr, "Test Asc1() <-> Asc1_old()\n");
  for (dlat = -90; dlat <= 90; dlat++) {
    for (armc = 0; armc <= 360; armc++) {
      asc1 = Asc1(armc, dlat, sine, cose);
      asc1_old = Asc1_old_old(armc, dlat, sine, cose);
      if (asc1 != asc1_old)
        fprintf(stderr, "armc=%f, lat=%f, Asc1: %.16f <-> Asc1_old: %.16f\n", armc, dlat, asc1, asc1_old);
    }
  }

}
#endif

/*
 * x in range 0..90
 * f in range -90 .. +90
 * sine, cose around e=23°
 */
static double Asc2(double x, double f, double sine, double cose) 
{
  double ass, sinx;
  ass = - tand(f) * sine + cose * cosd(x);
  if (fabs(ass) < VERY_SMALL)
    ass = 0;
  sinx = sind(x);
  if (fabs(sinx) < VERY_SMALL)
    sinx = 0;
  if (sinx == 0) {
    if (ass < 0)
      ass = -VERY_SMALL;
    else
      ass = VERY_SMALL;
  } else if (ass == 0) {
    if (sinx < 0)
      ass = -90;
    else
      ass = 90;
  } else {
    ass = atand(sinx / ass);
  }
  if (ass < 0)
    ass = 180 + ass;
  return (ass);
} /* Asc2 */

static double armc_to_mc(double armc, double eps)
{
  double cose = cosd(eps);
  double mc, tant;
  if (fabs(armc - 90) > VERY_SMALL
	  && fabs(armc - 270) > VERY_SMALL) {
    tant = tand(armc);
    mc = swe_degnorm(atand(tant / cose));
    if (armc > 90 && armc <= 270)
    mc = swe_degnorm(mc + 180);
  } else {
    if (fabs(armc - 90) <= VERY_SMALL)
      mc = 90;
    else
      mc = 270;
  }
  return mc;
}

/* if ascendant is on western half of horizon, add 180° */
static double fix_asc_polar(double asc, double armc, double eps, double geolat)
{
  double demc = atand(sind(armc) * tand(eps));
  if (geolat >= 0 && 90 - geolat + demc < 0)
    asc = swe_degnorm(asc + 180);
  if (geolat < 0 && -90 - geolat + demc > 0)
    asc = swe_degnorm(asc + 180);
  return asc;
}

/* Computes the house position of a planet or another point,
 * in degrees: 0 - 30 = 1st house, 30 - 60 = 2nd house, etc.
 * armc 	sidereal time in degrees
 * geolat	geographic latitude
 * eps		true ecliptic obliquity
 * hsys		house system character
 * xpin		array of 6 doubles:
 * 		only the first two of them are used: ecl. long., lat.
 * serr		error message area
 *
 * House position is returned by function.
 * Currently, geometrically correct house positions are provided 
 * for the following house methods:
 * A/E Equal, V Vehlow, W Whole Signs, D Equal/MC, N Equal/Zodiac,
 * O Porphyry, B Alcabitius, X Meridian, F Carter, M Morinus,
 * P Placidus, K Koch, C Campanus, R Regiomontanus, U Krusinski, 
 * T Topocentric, H Horizon, G Gauquelin.
 *
 * A simplified house position (distance_from_cusp / house_size)
 * is currently provided for the following house methods:
 * Y APC houses, L Pullen SD, Q Pullen SR, I Sunshine, S Sripati.
 *
 * For the following house methods only a simplified calcul
 *
 * IMPORTANT: This function should NOT be used for sidereal astrology.
 * If you cannot avoid doing so, please note:
 * - The input longitudes (xpin) MUST always be tropical, even if you 
 *   are a siderealist.
 * - Sidereal and tropical house positions are identical for most house
 *   systems, if a traditional definition of the sidereal zodiac is used 
 *   (sid = trop - ayanamsa).
 * - The function does NOT provide correct positions for Whole Sign houses.
 * - The function does NOT provide correct positions, if you use a 
 *   non-traditional sidereal method (where the sidereal plane is not 
 *   identical to the ecliptic of date) with a house system whose definition 
 *   is dependent on the ecliptic, such as: 
 *   equal, Porphyry, Alcabitius, Koch, Krusinski (all others should work).
 * The Swiss Ephemeris currently does not handle these cases.
 */
double CALL_CONV swe_house_pos(
	double armc, double geolat, double eps, int hsys, double *xpin, char *serr)
{
  double xp[6], xeq[6], ra, de, mdd, mdn, sad, san;
  double hpos, sinad, ad, a, admc, adp, samc, asc, mc, acmc, tant;
  //double demc;
  double fh, ra0, tanfi, fac, dfac, tanx;
  double x[3], xasc[3], raep, raaz, oblaz, xtemp; /* BK 21.02.2006 */
  double hcusp[36], ascmc[10];
  double sine = sind(eps);
  double cose = cosd(eps);
  double c1, c2, d, hsize;
  int i, j, nloop;
  double dsun = 0, darmc, harmc, y, sinpsi, sa;
  AS_BOOL is_western_half = FALSE;
  hsys = toupper(hsys);
if (1) {
  /* input is a house position: no calculation is required */
  ascmc[9] = 99;// dirty hack. Sunshine house system needs sun declination
  		// which we do not know. If it sees ascmc[9] == 99, it uses
		// the one is saved from last call. can lead to bugs, but can 
		// also solve many problems.
  if (swe_houses_armc(armc, geolat, eps, hsys, hcusp, ascmc) == ERR) {
    if (serr != NULL)
      sprintf(serr, "swe_house_pos(): failed for system %c", hsys);
  } else {
    hpos = 0;
    for (i = 1; i <= 12; i++) {
      if (fabs(swe_difdeg2n(xpin[0], hcusp[i])) < MILLIARCSEC && xpin[1] == 0) {
	hpos = (double) i;
      }
    }
    for (i = 1; i <= 12; i += 3) {
      if (fabs(swe_difdeg2n(xpin[0], hcusp[i])) < MILLIARCSEC && xpin[1] == 0) {
	xp[0] = (double) i;
      }
    }
    if (hpos > 0)
      return hpos;
    // for Sunshine houses: declination of Sun
    if (hsys == 'I')
      dsun = ascmc[9];  
    // for APC houses: declination of ascendant into dsun
    if (hsys == 'Y') {
      xeq[0] = ascmc[0];
      xeq[1] = 0;
      xeq[2] = 1;
      swe_cotrans(xeq, xeq, -eps);
      dsun = xeq[1]; 
    }
  }
}
  AS_BOOL is_above_hor = FALSE;
  AS_BOOL is_invalid = FALSE;
  AS_BOOL is_circumpolar = FALSE;
  if (serr != NULL)
    *serr = '\0';
  xeq[0] = xpin[0];
  xeq[1] = xpin[1];
  xeq[2] = 1;
  swe_cotrans(xeq, xeq, -eps);
  ra = xeq[0];
  de = xeq[1];
  mdd = swe_degnorm(ra - armc);
  mdn = swe_degnorm(mdd + 180);
  if (mdd >= 180)
    mdd -= 360;
  if (mdn >= 180)
    mdn -= 360;
  /* xp[0] will contain the house position, a value between 0 and 360 */
  switch(hsys) {
    case 'N': // equal (1=Aries)
      xp[0] = xpin[0];
      hpos = xp[0] / 30.0 + 1;
      break;
    case 'A': // equal
    case 'E': // equal
    case 'D': // equal (MC)
    case 'V': // Vehlow
    case 'W': // whole signs
      asc = Asc1(swe_degnorm(armc + 90), geolat, sine, cose);
      mc = armc_to_mc(armc, eps);
      asc = fix_asc_polar(asc, armc, eps, geolat);
      xp[0] = swe_degnorm(xpin[0] - asc);
      if (hsys == 'V')
	xp[0] = swe_degnorm(xp[0] + 15);
      if (hsys == 'W')
	xp[0] = swe_degnorm(xp[0] + fmod(asc, 30));
      if (hsys == 'D')
	xp[0] = swe_degnorm(xpin[0] - mc - 90);
      /* to make sure that a call with a house cusp position returns
       * a value within the house, 0.001" is added */
      xp[0] = swe_degnorm(xp[0] + MILLIARCSEC);
      hpos = xp[0] / 30.0 + 1;
      break;
    case 'O':  /* Porphyry */
    case 'B':  /* Alcabitius */
    case 'S':  /* Sripati */
      asc = Asc1(swe_degnorm(armc + 90), geolat, sine, cose);
      /* mc */
      mc = armc_to_mc(armc, eps);
      /* while MC is always south,
       * Asc must always be in eastern hemisphere */
      asc = fix_asc_polar(asc, armc, eps, geolat);
      if (hsys ==  'O' || hsys == 'S') {
	xp[0] = swe_degnorm(xpin[0] - asc);
	/* to make sure that a call with a house cusp position returns
	 * a value within the house, 0.001" is added */
	xp[0] = swe_degnorm(xp[0] + MILLIARCSEC);
	if (xp[0] < 180)
	  hpos = 1;
	else {
	  hpos = 7;
	  xp[0] -= 180;
	}
	acmc = swe_difdeg2n(asc, mc);
	if (xp[0] < 180 - acmc)
	  hpos += xp[0] * 3 / (180 - acmc);
	else
	  hpos += 3 + (xp[0] - 180 + acmc) * 3 / acmc;
        if (hsys == 'S') {
	  hpos += 0.5;
	  if (hpos > 12) hpos = 1;
	}
      } else { /* Alcabitius */
	double dek, r, sna, sda;
	dek = asind(sind(asc) * sine);	/* declination of Ascendant */
	/* must treat the case fi == 90 or -90 */
	tanfi = tand(geolat);
	r = -tanfi * tand(dek);
	/* must treat the case of abs(r) > 1; probably does not happen
	 * because dek becomes smaller when fi is large, as ac is close to
	 * zero Aries/Libra in that case.
	 */
	sda = acos(r) * RADTODEG;	/* semidiurnal arc, measured on equator */
	sna = 180 - sda;		/* complement, seminocturnal arc */
	if (mdd > 0) {
	  if (mdd < sda) 
	    hpos = mdd * 90 / sda;
	  else
	    hpos = 90 + (mdd - sda) * 90 / sna;
	} else {
	  if (mdd > -sna) 
	    hpos = 360 + mdd * 90 / sna;
	  else
	    hpos = 270 + (mdd + sna) * 90 / sda;
	}
	hpos = swe_degnorm(hpos - 90) / 30.0 + 1.0;
	if (hpos >= 13.0) hpos -= 12;
      }
      break;
    case 'X': /* Meridian or axial rotation system */
      hpos = swe_degnorm(mdd - 90) / 30.0 + 1.0;
      break;
    case 'F': /* Carter poli-equatorial */
      x[0] = Asc1(swe_degnorm(armc + 90), geolat, sine, cose);
      x[0] = fix_asc_polar(x[0], armc, eps, geolat);
      x[1] = 0;
      swe_cotrans(x, x, -eps);
      hpos = swe_degnorm(ra - x[0]) / 30.0 + 1;
      break;
    case 'M': { /* Morinus */
      double a = xpin[0];
      if (fabs(a - 90) > VERY_SMALL
        && fabs(a - 270) > VERY_SMALL) {
        tant = tand(a);
	hpos = atand(tant / cose);
        if (a > 90 && a <= 270) 
          hpos = swe_degnorm(hpos + 180);
      } else {
	if (fabs(a - 90) <= VERY_SMALL)
          hpos = 90;
        else 
          hpos = 270;
      } /*  if */
      hpos = swe_degnorm(hpos - armc - 90);
      hpos = hpos / 30.0 + 1;
    }
      break;
#if 0
    /* old version of Koch method */
    case 'K':
      demc = atand(sind(armc) * tand(eps));
      /* if body is within circumpolar region, error */
      if (90 - fabs(geolat) <= fabs(de)) {
        if (serr != NULL)
          strcpy(serr, "no Koch house position, because planet is circumpolar.");
        xp[0] = 0;
	hpos = 0;	/* Error */
      } else if (90 - fabs(geolat) <= fabs(demc)) {
	if (serr != NULL)
	  strcpy(serr, "no Koch house position, because mc is circumpolar.");
        xp[0] = 0;
	hpos = 0;	/* Error */
      } else {
        admc = asind(tand(eps) * tand(geolat) * sind(armc));
        adp = asind(tand(geolat) * tand(de));
	samc = 90 + admc;
        if (mdd >= 0) {	/* east */
          xp[0] = swe_degnorm(((mdd - adp + admc) / samc - 1) * 90);
	} else {
	  xp[0] = swe_degnorm(((mdd + 180 + adp + admc) / samc + 1) * 90);
	}
	/* to make sure that a call with a house cusp position returns
	 * a value within the house, 0.001" is added */
	xp[0] = swe_degnorm(xp[0] + MILLIARCSEC);
	hpos = xp[0] / 30.0 + 1;
      }
      break;
#endif
    /* version of Koch method: do calculations within circumpolar circle,
     * if possible; make sure house positions 4 - 9 only appear on western
     * hemisphere */
    case 'K': // Koch
      //demc = atand(sind(armc) * tand(eps));
      is_invalid = FALSE;
      is_circumpolar = FALSE;
      /* object is within a circumpolar circle */
      if (90 - geolat < de || -90 - geolat > de) {
        adp = 90;
	is_circumpolar = TRUE;
      }
      /* object is within a circumpolar circle, southern hemisphere */
      else if (geolat - 90 > de || geolat + 90 < de) {
        adp = -90;
	is_circumpolar = TRUE;
      }
      /* object does rise and set */
      else {
	adp = asind(tand(geolat) * tand(de));
      }
#if 0
      if (fabs(adp) == 90)
        is_invalid = TRUE; /* omit this to use the above values */
#endif
      admc = tand(eps) * tand(geolat) * sind(armc);
      /* midheaven is circumpolar */
      if (fabs(admc) > 1) {
#if 0
        is_invalid = TRUE; /* omit this line to use the below values */
#endif
	if (admc > 1)
	  admc = 1;
	else
	  admc = -1;
	is_circumpolar = TRUE;
      }
      admc = asind(admc);
      samc = 90 + admc;
      if (samc == 0)
        is_invalid = TRUE;
      if (fabs(samc) > 0) {
	if (mdd >= 0) { /* east */
	  dfac = (mdd - adp + admc) / samc;
	  xp[0] = swe_degnorm((dfac - 1) * 90);
	  xp[0] = swe_degnorm(xp[0] + MILLIARCSEC);
	  /* eastern object has longer SA than midheaven */
	  if (dfac > 2 || dfac < 0)
	    is_invalid = TRUE; /* if this is omitted, funny things happen */
	} else {
	  dfac = (mdd + 180 + adp + admc) / samc;
	  xp[0] = swe_degnorm((dfac + 1) * 90);
	  xp[0] = swe_degnorm(xp[0] + MILLIARCSEC);
	  /* western object has longer SA than midheaven */
	  if (dfac > 2 || dfac < 0)
	    is_invalid = TRUE; /* if this is omitted, funny things happen */
	}
      }
      if (is_invalid) {
        xp[0] = 0;
	hpos = 0;
	if (serr != NULL)
          strcpy(serr, "Koch house position failed in circumpolar area");
	break;
      }
      if (is_circumpolar) {
	if (serr != NULL)
          strcpy(serr, "Koch house position, doubtful result in circumpolar area");
      }
      /* to make sure that a call with a house cusp position returns
       * a value within the house, 0.001" is added */
      hpos = xp[0] / 30.0 + 1;
      break;
    case 'C': // Campanus
      xeq[0] = swe_degnorm(mdd - 90);
      swe_cotrans(xeq, xp, -geolat);
      /* to make sure that a call with a house cusp position returns
       * a value within the house, 0.001" is added */
      xp[0] = swe_degnorm(xp[0] + MILLIARCSEC);
      hpos = xp[0] / 30.0 + 1;
      break;
    case 'U': /* Krusinski-Pisa-Goelzer */
      if (fabs(geolat) < VERY_SMALL) {	/* code below does not like geolat 0 */
        geolat = (geolat >= 0) ? VERY_SMALL : -VERY_SMALL;
      }
      /* Purpose: find point where planet's house circle (meridian)
       *   cuts house plane, giving exact planet's house position.
       * Input data: ramc, geolat, asc.
       */
      asc = Asc1(swe_degnorm(armc + 90), geolat, sine, cose);
      /* while MC is always south, 
       * Asc must always be in eastern hemisphere */
      asc = fix_asc_polar(asc, armc, eps, geolat);
      /*
       * Descr: find the house plane 'asc-zenith' - where it intersects 
       * with equator and at what angle, and then simple find arc 
       * from asc on that plane to planet's meridian intersection 
       * with this plane.
       */
      /* I. find plane of 'asc-zenith' great circle relative to equator: 
       *   solve spherical triangle 'EP-asc-intersection of house circle with equator' */
      /* Ia. Find intersection of house plane with equator: */
      x[0] = asc; x[1] = 0.0; x[2] = 1.0;          /* 1. Start with ascendent on ecliptic     */
      swe_cotrans(x, x, -eps);                     /* 2. Transform asc into equatorial coords */
      raep = swe_degnorm(armc + 90);               /* 3. RA of east point                     */
      x[0] = swe_degnorm(raep - x[0]);             /* 4. Rotation - found arc raas-raep      */
      swe_cotrans(x, x, -(90-geolat));             /* 5. Transform into horizontal coords - arc EP-asc on horizon */
      tanx = tand(x[0]);
      if (geolat == 0) {
        xtemp = (tanx >= 0) ? 90 : -90;
      } else {
	xtemp = atand(tanx/cosd((90-geolat))); /* 6. Rotation from horizon on circle perpendicular to equator */
      }
      if (x[0] > 90 && x[0] <= 270)
	xtemp = swe_degnorm(xtemp + 180);
      x[0] = swe_degnorm(xtemp);        
      raaz = swe_degnorm(raep - x[0]); /* result: RA of intersection 'asc-zenith' great circle with equator */
      /* Ib. Find obliquity to equator of 'asc-zenith' house plane: */
      x[0] = raaz; x[1] = 0.0; 
      x[0] = swe_degnorm(raep - x[0]);  /* 1. Rotate start point relative to EP   */
      swe_cotrans(x, x, -(90-geolat));  /* 2. Transform into horizontal coords    */
      x[1] = x[1] + 90;                 /* 3. Add 90 deg do decl - so get the point on house plane most distant from equ. */
      swe_cotrans(x, x, 90-geolat);     /* 4. Rotate back to equator              */
      oblaz = x[1];                     /* 5. Obliquity of house plane to equator */
      /* II. Next find asc and planet position on house plane, 
       *     so to find relative distance of planet from 
       *     coords beginning. */
      /* IIa. Asc on house plane relative to intersection 
       *      of equator with 'asc-zenith' plane. */
      xasc[0] = asc; xasc[1] = 0.0; xasc[2] = 1.0;
      swe_cotrans(xasc, xasc, -eps);
      xasc[0] = swe_degnorm(xasc[0] - raaz);
      xtemp = atand(tand(xasc[0])/cosd(oblaz));
      if (xasc[0] > 90 && xasc[0] <= 270)
          xtemp = swe_degnorm(xtemp + 180);
      xasc[0] = swe_degnorm(xtemp);
      /* IIb. Planet on house plane relative to intersection 
       *      of equator with 'asc-zenith' plane */
      xp[0] = swe_degnorm(xeq[0] - raaz);        /* Rotate on equator  */
      xtemp = atand(tand(xp[0])/cosd(oblaz));    /* Find arc on house plane from equator */
      if (xp[0] > 90 && xp[0] <= 270)
	xtemp = swe_degnorm(xtemp + 180);
      xp[0] = swe_degnorm(xtemp);
      xp[0] = swe_degnorm(xp[0]-xasc[0]); /* find arc between asc and planet, and get planet house position  */
      /* IIc. Distance from planet to house plane on declination circle: */
      x[0] = xeq[0];
      x[1] = xeq[1];
      swe_cotrans(x, x, oblaz);
      xp[1] = xeq[1] - x[1]; /* How many degrees is the point on declination circle from house circle */
      /* to make sure that a call with a house cusp position returns
       * a value within the house, 0.001" is added */
      xp[0] = swe_degnorm(xp[0] + MILLIARCSEC);
      hpos = xp[0] / 30.0 + 1;
      break;
    case 'H': // horizon / azimuth
      xeq[0] = swe_degnorm(mdd - 90);
      swe_cotrans(xeq, xp, 90 - geolat);
      /* to make sure that a call with a house cusp position returns
       * a value within the house, 0.001" is added */
      xp[0] = swe_degnorm(xp[0] + MILLIARCSEC);
      hpos = xp[0] / 30.0 + 1;
      break;
    case 'R': // Regiomontanus
      if (fabs(mdd) < VERY_SMALL)
	xp[0] = 270;
      else if (180 - fabs(mdd) < VERY_SMALL)
        xp[0] = 90; 
      else {
        if (90 - fabs(geolat) < VERY_SMALL) {
          if (geolat > 0)
	    geolat = 90 - VERY_SMALL;
          else
	    geolat = -90 + VERY_SMALL;
        }
        if (90 - fabs(de) < VERY_SMALL) {
          if (de > 0)
            de = 90 - VERY_SMALL;
          else
	    de = -90 + VERY_SMALL;
        }
        a = tand(geolat) * tand(de) + cosd(mdd);
        xp[0] = swe_degnorm(atand(-a / sind(mdd)));
        if (mdd < 0)
          xp[0] += 180;
        xp[0] = swe_degnorm(xp[0]);
	/* to make sure that a call with a house cusp position returns
	 * a value within the house, 0.001" is added */
        xp[0] = swe_degnorm(xp[0] + MILLIARCSEC);
      }  
      hpos = xp[0] / 30.0 + 1;
      break;
    /* with Sunshine and APC houses, the method is the same,
     * except that Sunshine users dsun = (declination of Sun),
     * whereas APC uses dsun = (declination of ascendant). 
     * The Sunshine method has more problems within the polar
     * circles, if the Sun is circumpolar. The ascendant is never
     * circumpolar except if it coincides with the MC or the IC.
     */
    case 'I': case 'i': // sunshine houses (Makransky)
    case 'Y': // APC houses (Knegt)
      if (geolat > 90 - MILLIARCSEC)
        geolat = 90 - MILLIARCSEC;
      if (geolat < -90 + MILLIARCSEC)
        geolat = -90 + MILLIARCSEC;
//fprintf(stdout, "in=%f, mdd=%f\n", xpin[0], mdd);
      if (90 - fabs(de) < VERY_SMALL) {
	if (de > 0)
	  de = 90 - VERY_SMALL;
	else
	  de = -90 + VERY_SMALL;
      }
      a = tand(geolat) * tand(de) + cosd(mdd);
      xp[0] = swe_degnorm(atand(-a / sind(mdd)));
      if (mdd < 0)
	xp[0] += 180;
      xp[0] = swe_degnorm(xp[0]); // house position with hsys = 'R'
      /* is object above horizon? */
      sinad = tand(de) * tand(geolat);
      a = sinad + cosd(mdd);
      if (a >= 0)    
	is_above_hor = TRUE;
      /* height of armc above horizon */
      harmc = 90 - geolat;    
      if (geolat < 0)
	harmc = 90 + geolat;
      /* meridian distance of crossing of house position line with equator */
      darmc = swe_degnorm(xp[0] - 270);
      if (darmc > 180) {
	is_western_half = TRUE;
	darmc = (360 - darmc);
      }
      /* semi-diurnal arc of sun */
      sinad = tand(dsun) * tand(geolat);
      if (sinad >= 1) 
	ad = 90;
	//ad = 90 - VERY_SMALL;
      else if (sinad <= -1)
	ad = -90;
	//ad = -(90 - VERY_SMALL);
      else 
	ad = asind(sinad);
      sad = 90 + ad;
      san = 90 - ad;
      //fprintf(stdout, "in=%f, above=%d, sad=%f, san=%f, sinad=%f\n", xpin[0], (int) is_above_hor, sad, san, sinad);
      /* circumpolar sun has diurnal arc = 0 and object is above the horizon:
       * house position = 10 (270°) */
      if (sad == 0 && is_above_hor) {
	xp[0] = 270;
      /* circumpolar sun has nocturnal arc = 0 and object is below the horizon:
       * house position = 4 (90°) */
      } else if (san == 0 && !is_above_hor) {
	xp[0] = 90;
      /* otherwise we can calculate the house position */
      } else {
	sa = sad;
	if (!is_above_hor) {
	  dsun = -dsun;
	  sa = san;
	  darmc = 180 - darmc;
	  is_western_half = !is_western_half;
	}
	/* length of position line between south point and equator */
	a = acosd(cosd(harmc) * cosd(darmc));
	if (a < VERY_SMALL) 
	  a = VERY_SMALL;
	/* sine of angle between position line and equator */
	sinpsi = sind(harmc) / sind(a);
	if (sinpsi > 1) sinpsi = 1;
	if (sinpsi < -1) sinpsi = -1;
	/* meridian distance of crossing of house position line with solar diurnal arc */
	y = sind(dsun) / sinpsi;
	if (y > 1)
	  y = 90 - VERY_SMALL;
	else if (y < -1)
	  y = - (90 - VERY_SMALL);
	else
	  y = asind(y);
	d = acosd(cosd(y) / cosd(dsun));
	if (dsun < 0) d = -d;
	if (geolat < 0) d = -d;
	darmc += d;
	if (is_western_half) 
	  xp[0] = 270 - (darmc / sa) * 90;
	else
	  xp[0] = 270 + (darmc / sa) * 90;
	if (!is_above_hor)
	  xp[0] = swe_degnorm(xp[0] + 180);
      }
      /* to make sure that a call with a house cusp position returns
       * a value within the house, 0.001" is added */
      xp[0] = swe_degnorm(xp[0] + MILLIARCSEC);
      hpos = xp[0] / 30.0 + 1;
      break;
    case 'T': // Polich-Page ("topocentric")
      fh = geolat;
      if (fh > 89.999)
	fh = 89.999;
      if (fh < -89.999)
	fh = -89.999;
      mdd = swe_degnorm(mdd);
      if (de > 90 - VERY_SMALL)
	de = 90 - VERY_SMALL;
      if (de < -90 + VERY_SMALL)
	de = -90 + VERY_SMALL;
      sinad = tand(de) * tand(fh);
      if (sinad > 1.0) sinad = 1.0;
      if (sinad < -1.0) sinad = -1.0;
      a = sinad + cosd(mdd);
      if (a >= 0)
        is_above_hor = TRUE;
      /* mirror everything below the horizon to the opposite point
       * above the horizon */
      if (!is_above_hor) {
	ra = swe_degnorm(ra + 180);
	de = -de;
	mdd = swe_degnorm(mdd + 180);
      }
      /* mirror everything on western hemisphere to eastern hemisphere */  
      if (mdd > 180) {
	ra = swe_degnorm(armc - mdd);
      }
      /* binary search for "topocentric" position line of body */
      tanfi = tand(fh);
      ra0 = swe_degnorm(armc + 90);
      xp[1] = 1;
      xeq[1] = de;
      fac = 2;
      nloop = 0;
      while (fabs(xp[1]) > 0.000001 && nloop < 1000) {
	if (xp[1] > 0) {
	  fh = atand(tand(fh) - tanfi / fac);
	  ra0 -= 90 / fac;
	} else {
	  fh = atand(tand(fh) + tanfi / fac);
	  ra0 += 90 / fac;
	}
	xeq[0] = swe_degnorm(ra - ra0);
        swe_cotrans(xeq, xp, 90 - fh);
	fac *= 2;
	nloop++;
      }
      hpos = swe_degnorm(ra0 - armc);
      /* mirror back to west */
      if (mdd > 180)
	    hpos = swe_degnorm(-hpos);
      /* mirror back to below horizon */
      if (!is_above_hor)
	hpos = swe_degnorm(hpos + 180);
      hpos = swe_degnorm(hpos - 90) / 30 + 1;
      break;
    case 'P': // Placidus
    case 'G': // Gauquelin
       /* circumpolar region */
      if (90 - fabs(de) <= fabs(geolat)) {
        if (de * geolat < 0)  
          xp[0] = swe_degnorm(90 + mdn / 2);
        else
          xp[0] = swe_degnorm(270 + mdd / 2);
	if (serr != NULL)
          strcpy(serr, "Otto Ludwig procedure within circumpolar regions.");
      } else {
        sinad = tand(de) * tand(geolat);
        ad = asind(sinad);
        a = sinad + cosd(mdd);
        if (a >= 0)
          is_above_hor = TRUE;
        sad = 90 + ad;
        san = 90 - ad;
        if (is_above_hor)
          xp[0] =  (mdd / sad + 3) * 90;
        else
          xp[0] = (mdn / san + 1) * 90;
	/* to make sure that a call with a house cusp position returns
	 * a value within the house, 0.001" is added */
        xp[0] = swe_degnorm(xp[0] + MILLIARCSEC);
      }
      if (hsys == 'G') {
        xp[0] = 360 - xp[0]; /* Gauquelin sectors are in clockwise direction */
        hpos = xp[0] / 10.0 + 1;
      } else {
        hpos = xp[0] / 30.0 + 1;
      }
    break;
  default:
    hpos = 0;
    if (swe_houses_armc(armc, geolat, eps, hsys, hcusp, ascmc) == ERR) {
      if (serr != NULL)
	sprintf(serr, "swe_house_pos(): failed for system %c", hsys);
      break;
    }
    if (swe_difdeg2n(hcusp[6], hcusp[1]) > 0) {
      d = swe_degnorm(xpin[0] - hcusp[1]);
      for (i = 1; i <= 12; i++) {
	j = i + 1;
	if (j > 12) 
	  c2 = 360;
	else 
	  c2 = swe_degnorm(hcusp[j] - hcusp[1]);
	if (d < c2) break;
      }
      c1 = swe_degnorm(hcusp[i] - hcusp[1]);
    } else {  // houses retrograde
      d = swe_degnorm(hcusp[1] - xpin[0]);
      for (i = 1; i <= 12; i++) {
	j = i + 1;
	if (j > 12) 
	  c2 = 360;
	else 
	  c2 = swe_degnorm(hcusp[1] - hcusp[j]);
	if (d < c2) break;
      }
      c1 = swe_degnorm(hcusp[1] - hcusp[i]);
    }
    hsize = c2 - c1;
    if (hsize == 0) {
      hpos = i;
    } else {
      hpos = i + (d - c1) / hsize;
    }
    if (serr != NULL)
      sprintf(serr, "swe_house_pos(): using simplified algorithm for system %c\n", hsys);
    break;
  }
  return hpos;
}

int sunshine_init(double lat, double dec, double xh[])
{
  double ad, nsa, dsa, arg;
  // ascensional difference: sin ad = tan dec tan lat
  // or near +- 90 if Sun circumpolar
  arg = tand(dec) * tand(lat);
  if (arg >= 1) {
    ad = 90 - VERY_SMALL;
  } else if (arg <= -1) {
    ad = -90 + VERY_SMALL;
  } else {         
    ad = asind(arg);
  } 
  nsa = 90 - ad;
  dsa = 90 + ad;
  xh[2] = -2 * nsa / 3;
  xh[3] = -1 * nsa / 3;
  xh[5] = 1 * nsa / 3;
  xh[6] = 2 * nsa / 3;
  xh[8] = -2 * dsa / 3;
  xh[9] = -1 * dsa / 3;
  xh[11] = 1 * dsa / 3;
  xh[12] = 2 * dsa / 3;
  if (fabs(arg) >= 1)
    return ERR;
  return OK;
}

static int sunshine_solution_makransky(double ramc, double lat, double ecl, struct houses *hsp)
{
  double xh[13];
  double md;
  double zd;	// zenith distance of house circle, along prime vertical
  double pole, q, w, a, b, c, f, cu, r, rah;
  double sinlat, coslat, tanlat, tandec, sinecl;
  double dec = hsp->sundec;
  sinlat = sind(lat);
  coslat = cosd(lat);
  tanlat = tand(lat);
  tandec = tand(dec);
  sinecl = sind(ecl);
  int ih;
  // if (90 - fabs(lat) <= ecl) {
  //   strcpy(hsp->serr, "Sunshine in polar circle not allowed");
  //   return ERR;
  // }
  if (sunshine_init(lat, dec, xh) == ERR)
    return ERR;
  for (ih = 1; ih <= 12; ih++) {
    double z = 0;
    if ((ih - 1) % 3 == 0) continue;	// skip 1,4,7,10
    md = fabs(xh[ih]);
    if (ih <= 6)
      rah = swe_degnorm(ramc + 180 + xh[ih]);
    else
      rah = swe_degnorm(ramc + xh[ih]);
    if (lat < 0) {	// Makransky deals with southern latitude this way
      rah = swe_degnorm(180 + rah);
    }
    // HP is the house point on the semidiurnal arc
    // CP = intersection house meridian with prime vertical
    // MP = intersection house meridian with equator
    // XP = intersection house circle with prime meridian
    if (md == 90) {
      // CP = east point (or west point),
      // HP is on meridian east point - north pole
      // use triangle CP - HP - XP with long side dec
      // and angle 90 - lat. 
      // use tan b = cos alph tan c = sin lat tan dec
      zd = 90.0 - atand(sinlat * tandec);
    } else {
      if (md < 90) {
	// triangle 1) CP, Zenith, north pole: side 90-lat, angle md at
	// north pole.
	// tan a = cos lat * tan md
	// a is distance of CP from zenith on prime vertical
	a = atand(coslat * tand(md));
      } else {
	// triangle 1) MP - east point - CP : side b = 90-md, angle lat
	// at east point
	// tan c = tan md / cos lat
	// a is distance of CP from zenith on prime vertical
	a = atand(tand(md - 90) / coslat);	// lat = 90 not allowed
      }
      // triangle 2) CP, MP, east point: side 90 - md, angle lat
      // tan b = tan lat * cos md
      // b is distance of CP from equator
      b = atand(tanlat * cosd(md));
      // c is distance of HP house point from CP, along its meridian.
      if (ih <= 6)
	c = b + dec;
      else
	c = b - dec;
      // triangle 3) HP - CP - XP, side c, angle from triangle 1)
      // tan f = sin lat * sin md * tan c;
      // f is the distance from CP to XP
      f = atand(sinlat * sind(md) * tand(c));
      // a + f give zd, the zenith distance
      // of house circle measured on prime vertical.
      zd = a + f;
    }
    pole = asind(sind(zd) * sinlat);
    q = asind(tandec * tand(pole));
    if (ih <= 3 || ih >= 11)
      w = swe_degnorm(rah - q);
    else
      w = swe_degnorm(rah + q);
    if (w == 90) {
      r = atand(sind(ecl) * tand(pole));
      if (ih <= 3 || ih >= 11)
        cu = 90 + r;
      else
        cu = 90 - r;
    } else if (w == 270) {
      r = atand(sinecl * tand(pole));
      if (ih <= 3 || ih >= 11)
        cu = 270 - r;
      else
        cu = 270 + r;
    } else {
      double m;
      m = atand(fabs(tand(pole) / cosd(w)));
      if (ih <= 3 || ih >= 11) {
        if (w > 90 && w < 270)
	  z = m - ecl;
	else
	  z = m + ecl;
      } else {
        if (w > 90 && w < 270)
	  z = m + ecl;
	else
	  z = m - ecl;
      }
      if (z == 90) {
        if (w < 180)
	  cu = 90;
	else
	  cu = 270;
      } else {
	// r is between 0 and 90
        r = atand(fabs(cosd(m) * tand(w) / cosd(z)));
	if (w < 90)
	  cu = r;
	else if (w > 90 && w < 180)
	  cu = 180 - r;
	else if (w > 180 && w < 270)
	  cu = 180 + r;
	else
	  cu = 360 - r;
      }
      if (z > 90) {
      	// i am not sure if I understood the remark 'value will fall away from cancer..
	// on page 146 correctly.
	if (w < 90)
	  cu = 180 - r;
	else if (w > 90 && w < 180)
	  cu = + r;
	else if (w > 180 && w < 270)
	  cu = 360 - r;
	else
	  cu = 180 + r;
      }
      if (lat < 0)	// Makransky deals with southern latitude this way
        cu = swe_degnorm(cu + 180);
    }
    hsp->cusp[ih] = cu;
  } 
  return OK;
}

static int sunshine_solution_treindl(double ramc, double lat, double ecl, struct houses *hsp)
{
  double xh[13];
  double mcdec, sinlat, coslat, cosdec, tandec, sinecl, cosecl;
  double xhs, pole, a, cosa, alph, alpha2, c, cosc, b, sinzd, zd, rax, hc;
  int ih, retval = OK;
  AS_BOOL mc_under_horizon;
  double dec = hsp->sundec;
  // if (90 - fabs(lat) <= ecl) {
  //   strcpy(hsp->serr, "Sunshine in polar circle not allowed");
  //   return ERR;
  // }
  sinlat = sind(lat);
  coslat = cosd(lat);
  cosdec = cosd(dec);
  tandec = tand(dec);
  sinecl = sind(ecl);
  cosecl = cosd(ecl);
  sunshine_init(lat, dec, xh); 
  // find out if MC under horizon
  mcdec = atand(sind(ramc) * tand(ecl));
  mc_under_horizon = fabs(lat - mcdec) > 90;
  if (mc_under_horizon && SUNSHINE_KEEP_MC_SOUTH) {
    // we have switched ac/mc, invert offsets on diurnal arcs
    for (ih = 2; ih <= 12; ih++) {
      xh[ih] = -xh[ih];
    }
  }
  //if (sunshine_init(lat, dec, xh) == ERR)
  //  return ERR;
  // HP is the house point on the semidiurnal arc
  // CP = intersection house meridian with prime vertical
  // MP = intersection house meridian with equator
  // XP = intersection house circle with prime vertical
  // EP = intersection house circle with equator
  // MP0 = intersection semiarc with meridian
  for (ih = 1; ih <= 12; ih++) {
    if ((ih - 1) % 3 == 0) continue;	// skip 1,4,7,10
    xhs = 2 * asind(cosdec * sind(xh[ih] / 2));	// x'  great-circle length of x
    // compute triangle north pole - mp0 -hp
    // we have two sides 90 - dec, base xhs, ange at pole x
    // derive from cosine rule
    cosa = tandec * tand(xhs / 2); 
    alph = acosd(cosa);
    // compute triangle south point - mp0 - hp
    // we have: side x', side b = 90 - lat + dec, angle alpha2 between the sides.
    // we want: angle zd at south point.
    // we compute first the other side with Seitencosinus-Satz
    // cos c = cos x' cos b + sin x' sin b cos alpha2
    // for nocturnal side: use alpha, side b = 90 - lat - dec;
    // zd will be angle at north point.
    if (ih > 7) {
      // complementary angle
      alpha2 = 180 - alph;
      b = 90 - lat + dec;
    } else {	// nocturnal side
      alpha2 = alph;
      b = 90 - lat - dec;
    }
    // b can be zero, xhs can 90, c can get small.
    cosc = cosd(xhs) * cosd(b) + sind(xhs) * sind(b) * cosd(alpha2);
    c = acosd(cosc);
    // now Sinussatz
    if (c < 1e-6) {
      sprintf(hsp->serr, "Sunshine house %d c=%le very small", ih, c);
      retval = ERR;
    }
    sinzd = sind(xhs) * sind(alpha2) / sind(c);
    zd = asind(sinzd);
    // compute intersection house circle with equator, point rax 
    // day side triangle south point - meridian point - EP
    // night side: triangle north point
    // sides: 90 - lat, angle zd
    rax = atand(coslat * tand(zd));
    // compute pole height (distance of house circle pole from equator
    // with triangle at west point
    pole = asind(sinzd * sinlat);
    if (ih <= 6) {
      pole = -pole;
      a = swe_degnorm(rax + ramc + 180);
    } else {
      a = swe_degnorm(ramc + rax);
    }
    // with pole and a = rectascension of equator intersection, we use Asc1()
    // like with many other house systems, to intersect house circle with eclitpic
    hc = Asc1(a, pole, sinecl, cosecl);
    hsp->cusp[ih] = hc;
  }
  if (mc_under_horizon && ! SUNSHINE_KEEP_MC_SOUTH) {
    for (ih = 2; ih <= 12; ih++) {
      if ((ih - 1) % 3 == 0) continue;	// skip 1,4,7,10
      hsp->cusp[ih] = swe_degnorm(hsp->cusp[ih] + 180);
    }
  }
  return retval;
}
