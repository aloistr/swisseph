
/*******************************************************
module swehouse.c
house and (simple) aspect calculation 

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

//#include "sweodef.h"
#include "swephexp.h"
#include "sweph.h"
#include "swephlib.h"
#include "swehouse.h"
#include <string.h>

#define MILLIARCSEC 	(1.0 / 3600000.0)
#define SOLAR_YEAR   365.24219893
#define ARMCS ((SOLAR_YEAR+1) / SOLAR_YEAR * 360)

static double Asc1(double, double, double, double);
static double AscDash(double, double, double, double);
static double Asc2(double, double, double, double);
/* VERY_SMALL is an angular tolerance in degrees, and it is the wrong thing to
 * compare a denominator against. Sums of squares, products of cosines and the
 * like reach 1e-10 at perfectly ordinary configurations without the cusp being
 * degenerate, and refusing there reports a moving cusp as standing still. A
 * denominator is only unusable when dividing by it would overflow, so the
 * derivative code below tests against this instead. Guards that protect the
 * domain of asin() or sqrt() keep VERY_SMALL, where a real tolerance belongs. */
#define VERY_SMALL_DENOM 1e-200

/* Units used by the cusp speed code below.
 * Every cusp_speed[] and *_speed field is degrees of ecliptic longitude per
 * day, and ARMCS, the rate of armc, is the only place a per-day factor enters.
 * The helpers keep to one of two shapes:
 *   - AscDash(), apc_sector_speed() and plac_speed() already carry the ARMCS
 *     factor and return degrees per day directly. Note that AscDashPole(),
 *     despite the name, belongs to the other group: it is the partial of
 *     Asc1() in the pole height and carries no ARMCS;
 *   - pullen_sr_slopes(), makransky_slope_w(), AscDashPole() and
 *     plac_pole_slope() return a dimensionless degrees per degree, which is
 *     multiplied by ARMCS, or by an existing per-day speed, exactly once at
 *     the point of use. Case 'U' belongs to neither group: swe_cotrans_sp()
 *     is handed rates that already carry ARMCS, so it returns degrees per day
 *     throughout and no factor is applied at the end.
 * Angles are in degrees throughout except inside apc_sector_speed(), which
 * follows apc_sector() in taking radians; its result is a ratio of radians to
 * radians, hence dimensionless, and is scaled by ARMCS like the rest. */

/* Where each house system's cusp speeds come from.
 *
 *   A E D V     every cusp moves with the Ascendant or with the MC
 *   C R T H     AscDash(), closed form, unchanged by this patch
 *   P G         plac_speed(): AscDash() corrected by the implicit function
 *               theorem, because the pole height follows the cusp
 *   K           AscDash() times the rate of its own first argument; ad3 is
 *               differentiated through the MC
 *   O L S       chain rule from ac_speed and mc_speed through the quadrant
 *               size acmc
 *   Q           the same, but through the sector ratio r, which is
 *               differentiated implicitly from the quadrant identity
 *   X M         closed form through the ecliptic/equator conversion; armc is
 *               the only thing that moves. The two use different relations and
 *               so have different formulae, see the comments there
 *   F           ac_speed scaled by the ratio of the two conversion factors,
 *               the cos(eps) cancelling
 *   B           AscDash() times the rate of its first argument; the semi-
 *               diurnal arc is differentiated through the Ascendant
 *   U           the rotations are carried by swe_cotrans_sp(), which moves a
 *               velocity with its position; the conversions are closed form
 *   Y           apc_sector() differentiated in closed form; cusps 1 4 7 10 are
 *               not apc_sector()'s and take the Ascendant's and MC's speeds.
 *               Opposite cusps are not 180 degrees apart in this system, so
 *               all twelve are computed and the mirroring is skipped
 *   I i         treindl: AscDash() is exact, the pole height does not move.
 *               makransky: closed form, with cos(m) and sin(m) taken from
 *               sqrt(cos^2 w + tan^2 pole) rather than from m.
 *               As with Y, opposite cusps are not 180 degrees apart
 *   W N         zero. The cusps stand still between sign ingresses and jump
 *               when the Ascendant changes sign
 *
 * All of these are d(cusp)/d(armc) * ARMCS with the obliquity, and for
 * Sunshine the solar declination, held fixed. That is the same convention the
 * interpolation used: it handed the centre eps and sundec to both of its
 * offset samples. Callers wanting a total d/dt would have to add the terms in
 * d(eps)/dt and d(sundec)/dt themselves. The eps term is small next to
 * armc's rate. The sundec term is not always small for Sunshine: near the
 * polar circle the total d/dt was found to differ from these speeds by up to
 * 18 degrees per day, at a cusp moving about 6000 degrees per day (0.3 per
 * cent; test of 2026-11-13).
 * In sidereal mode the motion of the sidereal frame is included: the rate
 * of the ayanamsa in the traditional mode (sidereal_houses_trad()), the rates
 * of the auxiliary frame in the ECL_T0 and SSY_PLANE modes
 * (sidereal_frame_speeds()).
 *
 * Seven places below hold a formula and its derivative as separate code, with
 * nothing to make them agree. Change one and the other has to change too:
 *
 *   Asc1()                     AscDash(), in its first argument
 *   Asc1()                     AscDashPole(), in the pole height
 *   plac_pole()                plac_pole_slope()
 *   apc_sector()               apc_sector_speed()
 *   makransky_cusp_from_w()    makransky_slope_w()
 *   ad3 in case 'K'            the dad3 block just below it
 *   sda in case 'B'            the dsda block just below it
 *
 * and in case 'Q' the mapping from sector widths to cusps is written out once
 * for the positions and again for the speeds.
 *
 * Sources for the definitions the derivatives below rest on. Each is also
 * named at the case that uses it.
 *
 *   B   Alcabitius   diurnal arc of the Ascendant in six, meridian circles
 *   F   Carter       poli-equatorial, 12 meridians 30 deg apart from the Asc
 *   L Q Pullen       W. D. Pullen, "Sinusoidal House Systems",
 *                    https://www.astrolog.org/astrolog/astsine.htm
 *   M   Morinus      equator in twelve, projected from the ecliptic pole
 *   X   Meridian     equator in twelve, projected from the equatorial pole
 *   Y   APC          diurnal and nocturnal arcs of the Ascendant in six
 *   I i Sunshine     B. Makransky, "Primary Directions: A Primer of
 *                    Calculation", 1988: diurnal and nocturnal arcs of the
 *                    Sun in six, projected with house circles
 *   B F M X Y and the two Sunshine variants are described by Astrodienst,
 *   "Tables of Houses for Astrology",
 *   https://www.astro.com/swisseph/sweph_ht_e.htm
 *   P G K O S W N are classical and are described in any of the above.
 *
 * Two of those definitions are checkable against the library itself rather
 * than against a difference quotient, and both agree:
 *   - Meridian cusp 1 is the equatorial ascendant. Its speed here matches
 *     ascmc_speed[SE_EQUASC], computed by a different route, to 2e-13.
 *   - Krusinski cusp 10 falls out of its chain of rotations, yet equals the
 *     MC, and its speed equals mc_speed to 4e-14 relative.
 */
static double apc_sector_speed(int n, double ph, double e, double az);
static void pullen_sr_sectors(double q, double *x, double *xr, double *xr3, double *xr4, double *r);
static void pullen_sr_slopes(double q, double r, double x, double xr, double xr3,
			     double *dx, double *dxr, double *dxr3, double *dxr4);
static double plac_pole(double cusp, double tanfi, double fac, double sine, int *ok);
static double plac_pole_slope(double cusp, double tanfi, double fac, double sine, int *ok);
static double AscDashPole(double x, double f, double sine, double cose);
static double plac_speed(double rectasc, double cusp, double f, double tanfi, double fac, double sine, double cose);
static int CalcH(double th, double fi, double ekl, char hsy, struct houses *hsp);
static int sidereal_houses_ecl_t0(double tjde, 
			   int32 iflag,
                           double armc, 
                           double eps, 
                           double *nutlo, 
                           double lat, 
			   int hsys, 
                           double *cusp, 
                           double *ascmc,
			   double *cusp_speed,
			   double *ascmc_speed,
			   char *serr);
static void ecl_t0_frame(double tjde, double eps, double *nutlo, double *dvpx, double *epsx, double *dvpxe);
static void ssypl_frame(double tjde, double eps, double *nutlo, double *dvpx, double *epsx, double *dvpxe);
static void sidereal_frame_speeds(int ssypl, double tjde, int32 iflag, int hsys, int ito, double *cusp_speed, double *ascmc_speed);
static int sidereal_houses_trad(double tjde, 
			   int32 iflag,
                           double armc, 
                           double eps, 
                           double nutl, 
                           double lat, 
			   int hsys, 
                           double *cusp, 
                           double *ascmc,
			   double *cusp_speed,
			   double *ascmc_speed,
			   char *serr);
static int sidereal_houses_ssypl(double tjde, 
			   int32 iflag,
                           double armc, 
                           double eps, 
                           double *nutlo, 
                           double lat,
			   int hsys, 
                           double *cusp, 
                           double *ascmc,
			   double *cusp_speed,
			   double *ascmc_speed,
			   char *serr);
static int sunshine_solution_makransky(double ramc, double lat, double ecl, struct houses *hsp);
static double makransky_cusp_from_w(double w, double pole, double ecl, int ih, double lat);
static double makransky_slope_w(double w, double pole, double ecl, int ih, double lat);
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
    if (result < 0) {
      // in case of failure, Porphyry houses
      result = swe_houses_armc_ex2(armc, geolat, eps + nutlo[1], 'O', cusp, ascmc, NULL, NULL, NULL);
      return ERR;
    }
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
  retc = swe_houses_armc_ex2(armc, geolat, eps + nutlo[1], hsys, cusp, ascmc, NULL, NULL, NULL);
  return retc;
}

// For explanation see function swe_houses_ex2() below.
int CALL_CONV swe_houses_ex(double tjd_ut,
                                int32 iflag, 
				double geolat,
				double geolon,
				int hsys,
				double *cusp,
				double *ascmc)
{
  return swe_houses_ex2(tjd_ut, iflag, geolat, geolon, hsys, cusp, ascmc, NULL, NULL, NULL);
}

/* 
 * Function returns OK or ERR.
 * cusps are returned in double cusp[13],
 *                           or cusp[37] with house system 'G'.
 * cusp[1...12]	  houses 1 - 12
 * ascmc[0...10]  additional points:
 *                ascmc[0] = ascendant
 *                ascmc[1] = mc
 *                ascmc[2] = armc
 *                ascmc[3] = vertex
 *                ascmc[4] = equasc		* "equatorial ascendant" *
 *                ascmc[5] = coasc1		* "co-ascendant" (W. Koch) *
 *                ascmc[6] = coasc2		* "co-ascendant" (M. Munkasey) *
 *                ascmc[7] = polasc		* "polar ascendant" (M. Munkasey) *
 * cusp_speed[1...12]  speeds (daily motions) of the cusps.
 * ascmc_speed[0...10] speeds (daily motions) of the additional points.
 * serr           error message or warning
 */
int CALL_CONV swe_houses_ex2(double tjd_ut,
                                int32 iflag, 
				double geolat,
				double geolon,
				int hsys,
				double *cusp,
				double *ascmc,
			        double *cusp_speed,
				double *ascmc_speed,
				char *serr)
{
  int i, retc = 0;
  double armc, eps_mean, nutlo[2];
  double tjde = tjd_ut + swe_deltat_ex(tjd_ut, iflag, NULL);
  struct sid_data *sip = &swed.sidd;
  double xp[6];
  int retc_makr = 0;
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
  if (iflag & SEFLG_NONUT) {
    for (i = 0; i < 2; i++)
      nutlo[i] = 0;
  }
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
//fprintf(stderr, "armc=%f, iflag=%d\n", armc, iflag);
  if (toupper(hsys) ==  'I') {	// compute sun declination for sunshine houses
    int flags = SEFLG_SPEED| SEFLG_EQUATORIAL;
    retc_makr = swe_calc_ut(tjd_ut, SE_SUN, flags, xp, NULL);
    if (retc_makr < 0) {
      // in case of failure, provide Porphyry houses
      hsys = (int) 'O';
    }
    ascmc[9] = xp[1];	// declination in ascmc[9];
  }
  if (iflag & SEFLG_SIDEREAL) { 
    if (sip->sid_mode & SE_SIDBIT_ECL_T0)
      retc = sidereal_houses_ecl_t0(tjde, iflag, armc, eps_mean + nutlo[1], nutlo, geolat, hsys, cusp, ascmc, cusp_speed, ascmc_speed, serr);
    else if (sip->sid_mode & SE_SIDBIT_SSY_PLANE)
      retc = sidereal_houses_ssypl(tjde, iflag, armc, eps_mean + nutlo[1], nutlo, geolat, hsys, cusp, ascmc, cusp_speed, ascmc_speed, serr);
    else
      retc = sidereal_houses_trad(tjde, iflag, armc, eps_mean + nutlo[1], nutlo[0], geolat, hsys, cusp, ascmc, cusp_speed, ascmc_speed, serr);
  } else {
    retc = swe_houses_armc_ex2(armc, geolat, eps_mean + nutlo[1], hsys, cusp, ascmc, cusp_speed, ascmc_speed, serr);
    if (toupper(hsys) ==  'I') 	
      ascmc[9] = xp[1];	// declination in ascmc[9];
  }
  if (iflag & SEFLG_RADIANS) {
    for (i = 1; i <= ito; i++)
      cusp[i] *= DEGTORAD;
    for (i = 0; i < SE_NASCMC; i++)
      ascmc[i] *= DEGTORAD;
    /* The speeds are angles per day and belong in the same unit as the angles
     * they accompany. Without this a SEFLG_RADIANS caller was handed radian
     * positions beside degree per day speeds, so anything that added a speed
     * to a position, or compared the two, was out by a factor of 57.3. The
     * omission predates this patch and mattered less while nine house systems
     * returned interpolated speeds nobody could rely on; now that every system
     * returns a real derivative, callers will use it. */
    if (cusp_speed != NULL)
      for (i = 1; i <= ito; i++)
        cusp_speed[i] *= DEGTORAD;
    if (ascmc_speed != NULL)
      for (i = 0; i < SE_NASCMC; i++)
        ascmc_speed[i] *= DEGTORAD;
  }
  if (retc_makr < 0)
    return retc_makr;
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
			   int32 iflag,
                           double armc, 
                           double eps, 
                           double *nutlo, 
                           double lat, 
			   int hsys, 
                           double *cusp, 
                           double *ascmc,
			   double *cusp_speed,
			   double *ascmc_speed,
			   char *serr)
{
  int i, retc = OK;
  double epsx, dvpx, dvpxe;
  double armcx;
  struct sid_data *sip = &swed.sidd;
  int ito;
  if (toupper(hsys) == 'G')
    ito = 36;
  else
    ito = 12;
  /* steps 1, 2 and 5: auxiliary obliquity, distance dvpx of the auxiliary
   * vernal point from the vernal point at t, distance dvpxe of the auxiliary
   * vernal point from the vernal point of t0 */
  ecl_t0_frame(tjde, eps, nutlo, &dvpx, &epsx, &dvpxe);
  /* auxiliary armc */
  armcx = swe_degnorm(armc - dvpx);        /* 3 */
  /* compute axes and houses: */
  retc = swe_houses_armc_ex2(armcx, lat, epsx, hsys, cusp, ascmc, cusp_speed, ascmc_speed, serr);  /* 4 */
  /* On failure only the twelve Porphyry cusps exist, and swe_houses_armc_ex2()
   * has cleared 13 to 36. The loops below would otherwise subtract the ayanamsa
   * from those cleared elements and hand the caller values that look like
   * ordinary cusps rather than the "not computed" they are. */
  if (retc < 0)
    ito = 12;
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
  if (cusp_speed != NULL || ascmc_speed != NULL)
    sidereal_frame_speeds(0, tjde, iflag, hsys, ito, cusp_speed, ascmc_speed);
  return retc;
}

/* Frame of the ECL_T0 mode at tjde (steps 1, 2 and 5 of the procedure
 * above): the distance dvpx of the auxiliary vernal point from the vernal
 * point at tjde, on the equator; the auxiliary obliquity epsx; the distance
 * dvpxe of the auxiliary vernal point from the vernal point of t0, on the
 * ecliptic of t0. eps is the true obliquity and nutlo the nutation of tjde,
 * in degrees. */
static void ecl_t0_frame(double tjde, double eps, double *nutlo, double *dvpx, double *epsx, double *dvpxe)
{
  int j;
  double x[6], xvpx[6], x2[6], epst0, xnorm[6];
  double rxy, rxyz, c2, sgn, fac;
  struct sid_data *sip = &swed.sidd;
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
  *epsx = asin(rxy / rxyz) * RADTODEG;           /* 1a */
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
  *dvpx = x2[0] * RADTODEG;                      /* 2 */
  /* distance between auxiliary vernal point and
   * vernal point of t0 (a section on the sidereal plane) */
  *dvpxe = acos(swi_dot_prod_unit(x, xvpx)) * RADTODEG;  /* 5 */
  if (tjde < sip->t0)
    *dvpxe = -*dvpxe;
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
			   int32 iflag,
                           double armc, 
                           double eps, 
                           double *nutlo, 
                           double lat, 
			   int hsys, 
                           double *cusp, 
                           double *ascmc,
			   double *cusp_speed,
			   double *ascmc_speed,
			   char *serr)
{
  int i, retc = OK;
  double x0[6];
  double epsx, eps2000, dvpx, dvpxe, x00;
  double armcx;
  struct sid_data *sip = &swed.sidd;
  int ito;
  if (toupper(hsys) == 'G')
    ito = 36;
  else
    ito = 12;
  eps2000 = swi_epsiln(J2000, 0);
  /* steps 1, 2 and 5: auxiliary obliquity, distance dvpx of the auxiliary
   * vernal point from the mean vernal point at t, distance dvpxe of the
   * auxiliary vernal point from the sidereal zero point of 2000 */
  ssypl_frame(tjde, eps, nutlo, &dvpx, &epsx, &dvpxe);
  /* auxiliary armc */
  armcx = swe_degnorm(armc - dvpx);        /* 3 */
  /* compute axes and houses: */
  retc = swe_houses_armc_ex2(armcx, lat, epsx, hsys, cusp, ascmc, cusp_speed, ascmc_speed, serr);  /* 4 */
  /* On failure only the twelve Porphyry cusps exist, and swe_houses_armc_ex2()
   * has cleared 13 to 36. The loops below would otherwise subtract the ayanamsa
   * from those cleared elements and hand the caller values that look like
   * ordinary cusps rather than the "not computed" they are. */
  if (retc < 0)
    ito = 12;
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
  if (cusp_speed != NULL || ascmc_speed != NULL)
    sidereal_frame_speeds(1, tjde, iflag, hsys, ito, cusp_speed, ascmc_speed);
  return retc;
}

/* Frame of the SSY_PLANE mode at tjde (steps 1, 2 and 5 of the procedure
 * above): the distance dvpx of the auxiliary vernal point from the mean
 * vernal point at tjde, on the equator; the auxiliary obliquity epsx; the
 * distance dvpxe of the auxiliary vernal point from the sidereal zero point
 * of 2000, on the solar system plane. eps is the true obliquity and nutlo
 * the nutation of tjde, in degrees. */
static void ssypl_frame(double tjde, double eps, double *nutlo, double *dvpx, double *epsx, double *dvpxe)
{
  int j;
  double x[6], xvpx[6], x2[6], xnorm[6];
  double rxy, rxyz, c2, eps2000, sgn, fac;
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
  *epsx = asin(rxy / rxyz) * RADTODEG;           /* 1a */
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
  *dvpx = x2[0] * RADTODEG;                      /* 2 */
  /* distance between the auxiliary vernal point at t and
   * the sidereal zero point of 2000 at t
   * (a section on the sidereal plane).
   */ 
  *dvpxe = acos(swi_dot_prod_unit(x, xvpx)) * RADTODEG;  /* 5 */
                /* (always positive for dates after 5400 bc) */
  *dvpxe -= SSY_PLANE_NODE * RADTODEG;
}

/* Speeds in the ECL_T0 (ssypl == 0) and SSY_PLANE (ssypl == 1) modes.
 * There the cusps are f(armc - dvpx, epsx) - dvpxe - const, and the speeds
 * that swe_houses_armc_ex2() returns are df/d(armcx) * ARMCS. As everywhere
 * the obliquity, here epsx, is held fixed; what is added is the motion of the
 * frame: armcx moves at ARMCS - d(dvpx)/dt, and the frame turns at
 * d(dvpxe)/dt on the sidereal plane. This takes the place of the ayanamsa
 * rate that sidereal_houses_trad() subtracts; copying that rate would be
 * wrong here, because these modes compute the houses on another plane and
 * their cusps are not the tropical ones shifted by the ayanamsa.
 * The rates are central differences over 0.2 day, with obliquity and
 * nutation of the neighbouring dates computed as in swe_houses_ex2(). armc
 * (ascmc[2]) is armcx in these modes and gets ARMCS - d(dvpx)/dt; 0 Aries
 * cusps ('N') stand still. Before 2026-11-13 these modes returned the
 * speeds without the frame terms, about 4e-5 degrees per day too large. */
static void sidereal_frame_speeds(int ssypl, double tjde, int32 iflag, int hsys, int ito, double *cusp_speed, double *ascmc_speed)
{
  int i, k;
  double t, epsm, nut[2], dvpx[2], epsx, dvpxe[2], ddvpx, ddvpxe, fac;
  for (k = 0; k < 2; k++) {
    t = tjde + ((k == 0) ? -0.1 : 0.1);
    epsm = swi_epsiln(t, 0) * RADTODEG;
    swi_nutation(t, 0, nut);
    for (i = 0; i < 2; i++)
      nut[i] *= RADTODEG;
    if (iflag & SEFLG_NONUT)
      nut[0] = nut[1] = 0;
    if (ssypl)
      ssypl_frame(t, epsm + nut[1], nut, &dvpx[k], &epsx, &dvpxe[k]);
    else
      ecl_t0_frame(t, epsm + nut[1], nut, &dvpx[k], &epsx, &dvpxe[k]);
  }
  ddvpx = swe_difdeg2n(dvpx[1], dvpx[0]) / 0.2;	/* degrees per day */
  ddvpxe = swe_difdeg2n(dvpxe[1], dvpxe[0]) / 0.2;
  fac = 1 - ddvpx / ARMCS;
  if (cusp_speed != NULL && hsys != 'N')
    for (i = 1; i <= ito; i++)
      cusp_speed[i] = cusp_speed[i] * fac - ddvpxe;
  if (ascmc_speed != NULL)
    for (i = 0; i < SE_NASCMC; i++)
      ascmc_speed[i] = ascmc_speed[i] * fac - ((i == 2) ? 0 : ddvpxe);
}

/* common simplified procedure */
static int sidereal_houses_trad(double tjde,
			   int32 iflag,
                           double armc, 
                           double eps, 
                           double nutl, 
                           double lat, 
			   int hsys, 
                           double *cusp, 
                           double *ascmc,
			   double *cusp_speed,
			   double *ascmc_speed,
			   char *serr)
{
  int i, retc = OK;
  double ay, dayan = 0;
  int ito;
  int ihs = toupper(hsys);
  int ihs2 = ihs;
// ay = swe_get_ayanamsa(tjde);
//fprintf(stderr, "ay=%f\n", ay);
  retc = swe_get_ayanamsa_ex(tjde, iflag, &ay, NULL);
  /* and its rate, for the speeds. Two extra evaluations of a function
   * that is smooth over a day; no house computation is involved. */
  if (cusp_speed != NULL || ascmc_speed != NULL) {
    double ay1, ay2;
    /* The Sunshine declination check inside swe_houses_armc_ex2(), reachable
     * from here with SEFLG_MOSEPH at dates where the obliquity passes 24
     * degrees, returns ERR with all four arrays set to zero. The zeros here
     * are a second safeguard, so that the rate loops at the bottom of this
     * function never read what the caller had in the speed arrays; every
     * other path overwrites them before they are read. */
    if (cusp_speed != NULL)
      for (i = 0; i <= ((ihs == 'G') ? 36 : 12); i++)
	cusp_speed[i] = 0;
    if (ascmc_speed != NULL)
      for (i = 0; i < SE_NASCMC; i++)
	ascmc_speed[i] = 0;
    if (swe_get_ayanamsa_ex(tjde - 0.1, iflag, &ay1, NULL) >= 0
     && swe_get_ayanamsa_ex(tjde + 0.1, iflag, &ay2, NULL) >= 0)
      dayan = swe_difdeg2n(ay2, ay1) / 0.2;	/* degrees per day */
  }
//fprintf(stderr, "ay=%f\n", ay);
//fprintf(stderr, "nutl=%f\n", nutl);
  if (ihs == 'G')
    ito = 36;
  else
    ito = 12;
  if (ihs == 'W')  /* whole sign houses: treat as 'E' and fix later */
    ihs2 = 'E';
//fprintf(stderr, "armc=%f\n", armc);
//if (hsys == 'P') fprintf(stderr, "ay=%f, t=%f %c", ay, tjde, (char) hsys);
  retc = swe_houses_armc_ex2(armc, lat, eps, ihs2, cusp, ascmc, cusp_speed, ascmc_speed, serr);
  /* On failure only the twelve Porphyry cusps exist, and swe_houses_armc_ex2()
   * has cleared 13 to 36. The loops below would otherwise subtract the ayanamsa
   * from those cleared elements and hand the caller values that look like
   * ordinary cusps rather than the "not computed" they are. */
  if (retc < 0)
    ito = 12;
//if (hsys == 'P') fprintf(stderr, "  h1=%f", cusp[1]);
  for (i = 1; i <= ito; i++) {
    //cusp[i] = swe_degnorm(cusp[i] - ay - nutl);
    cusp[i] = swe_degnorm(cusp[i] - ay);
    if (ihs == 'W') /* whole sign houses */
      cusp[i] -= fmod(cusp[i], 30);
  }
  if (ihs == 'N') { /* 1 = 0° Aries */
    for (i = 1; i <= ito; i++) {
      cusp[i] = (i - 1) * 30;
    }
  }
  /* Whole sign and 0 Aries cusps sit on sign boundaries and stand still
   * between ingresses. Case 'W' in CalcH() zeroes their speeds, but it is
   * never reached from here: 'W' is computed as equal houses through ihs2
   * above and the sign boundaries are imposed afterwards, so without this the
   * equal house speeds would survive on cusps the truncation has just pinned,
   * which is the defect this patch removes on the tropical path. 'N' is
   * already zero and is included so the two cannot drift apart. */
  if ((ihs == 'W' || ihs == 'N') && cusp_speed != NULL) {
    for (i = 1; i <= ito; i++)
      cusp_speed[i] = 0;
  }
  for (i = 0; i < SE_NASCMC; i++) {
    if (i == 2)	/* armc */
      continue;
    //ascmc[i] = swe_degnorm(ascmc[i] - ay - nutl);
    ascmc[i] = swe_degnorm(ascmc[i] - ay);
  }
//if (hsys == 'P') fprintf(stderr, " => %f\n", cusp[1]);
  /* Every longitude above has had the ayanamsa subtracted from it and no
   * speed has had its rate subtracted, so a sidereal call returned exactly
   * the tropical speeds beside sidereal positions. The term is the rate of
   * the ayanamsa, about 0.145 arc seconds per day for Fagan/Bradley, which
   * is 4.0e-5 degrees and shows in the last digits swetest prints. armc is
   * not shifted and keeps its tropical rate. Whole sign
   * and 0 Aries cusps are excluded: they are pinned to sidereal sign
   * boundaries, which do not move in the sidereal frame at all. */
  /* Not conditioned on retc. Inside the polar circle several systems fall
   * back to Porphyry and return ERR having written every element, and the
   * loop above has already taken the ayanamsa off those positions; skipping
   * the speeds there would leave exactly the inconsistency this patch exists
   * to remove, on every date at such latitudes. On the Sunshine declination
   * check swe_houses_armc_ex2() returns ERR with all arrays set to zero, and
   * the speed arrays are also zeroed before the call above, so these loops
   * read defined values on that path too. */
  if (dayan != 0) {
    /* The whole sign and 0 Aries carve-out is about cusps: those sit on
     * sidereal sign boundaries and do not move in the sidereal frame at all.
     * ascmc is not a house system quantity, the loop above shifts it by the
     * ayanamsa for every letter including W and N, and its speeds need the
     * rate for every letter too. */
    if (cusp_speed != NULL && ihs != 'W' && ihs != 'N')
      for (i = 1; i <= ito; i++)
	cusp_speed[i] -= dayan;
    if (ascmc_speed != NULL)
      for (i = 0; i < SE_NASCMC; i++)
	if (i != 2)	/* armc */
	  ascmc_speed[i] -= dayan;
  }
  return retc;
}

// For explanation see function swe_houses_armc_ex2() below.
int CALL_CONV swe_houses_armc(
				double armc,
				double geolat,
				double eps,
				int hsys,
				double *cusp,
				double *ascmc)
{
  return swe_houses_armc_ex2(armc, geolat, eps, hsys, cusp, ascmc, NULL, NULL, NULL);
}

/* 
 * Function returns OK or ERR.
 * this function is required for very special computations
 * where no date is given for house calculation,
 * e.g. for composite charts or progressive charts.
 * cusps are returned in double cusp[13],
 *                           or cusp[37] with house system 'G'.
 * cusp[1...12]	  houses 1 - 12
 * ascmc[0...10]  additional points:
 *                ascmc[0] = ascendant
 *                ascmc[1] = mc
 *                ascmc[2] = armc
 *                ascmc[3] = vertex
 *                ascmc[4] = equasc		* "equatorial ascendant" *
 *                ascmc[5] = coasc1		* "co-ascendant" (W. Koch) *
 *                ascmc[6] = coasc2		* "co-ascendant" (M. Munkasey) *
 *                ascmc[7] = polasc		* "polar ascendant" (M. Munkasey) *
 * cusp_speed[1...12]  speeds (daily motions) of the cusps.
 * ascmc_speed[0...10] speeds (daily motions) of the additional points.
 * serr           error message or warning
 */
int CALL_CONV swe_houses_armc_ex2(
				double armc,
				double geolat,
				double eps,
				int hsys,
				double *cusp,
				double *ascmc,
				double *cusp_speed,
				double *ascmc_speed,
				char *serr)
{
  struct houses h, hm1, hp1;
  int i, retc = 0, rm1, rp1;
  int ito;
  static double saved_sundec = 99;
  if (toupper(hsys) == 'G')
    ito = 36;
  else
    ito = 12;
  armc = swe_degnorm(armc);
  h.do_speed = FALSE;
  h.do_hspeed = FALSE;
  if (ascmc_speed != NULL || cusp_speed != NULL)
    h.do_speed = TRUE;	// is needed if cusp_speed wanted
  if (cusp_speed != NULL)
    h.do_hspeed = TRUE;
  if (toupper(hsys) ==  'I') {	// declination for sunshine houses
    if (ascmc[9] == 99) {
      h.sundec = 0;
      if (saved_sundec != 99) h.sundec = saved_sundec;
    } else {
      h.sundec = ascmc[9];
      saved_sundec = h.sundec;
    }
    if (h.sundec < -24 || h.sundec > 24) {
      if (serr != NULL)
        sprintf(serr, "House system I (Sunshine) needs valid Sun declination in ascmc[9]");
      /* Every other return from this function writes the caller's arrays;
       * this one returned with all four untouched, and the sidereal wrappers
       * then subtract the ayanamsa from whatever the caller had there and
       * hand it back through swe_degnorm() looking like a longitude. Zeros
       * instead. ascmc[9] is the caller's own input and is left as passed. */
      cusp[0] = 0;
      for (i = 1; i <= ito; i++)
        cusp[i] = 0;
      if (cusp_speed != NULL) {
        cusp_speed[0] = 0;
        for (i = 1; i <= ito; i++)
          cusp_speed[i] = 0;
      }
      for (i = 0; i < 9; i++)
        ascmc[i] = 0;
      if (ascmc_speed != NULL) {
        for (i = 0; i < 10; i++)
          ascmc_speed[i] = 0;
      }
      return ERR;
    }
  }
  retc = CalcH(armc, geolat, eps, (char)hsys, &h);
  cusp[0] = 0;
  if (h.do_hspeed) cusp_speed[0] = 0;
  // on failure, we only have 12 Porphyry cusps
  if (retc < 0) {
    /* The caller sized the array for the house system it asked for, and
     * for Gauquelin sectors that is cusp[37]. Copying only the twelve
     * Porphyry cusps leaves 13 to 36 holding whatever the caller had
     * there, which for a fresh automatic array is uninitialised stack.
     * They are cleared before ito is clamped, so that a failure returns
     * a defined value in every element it is documented to fill. */
    for (i = 13; i <= ito; i++) {
      cusp[i] = 0;
      if (h.do_hspeed) cusp_speed[i] = 0;
    }
    ito = 12;
    if (serr != NULL) strcpy(serr, h.serr);
  }
  for (i = 1; i <= ito; i++) {
    cusp[i] = h.cusp[i];
    if (h.do_hspeed) cusp_speed[i] = h.cusp_speed[i];   
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
  if (h.do_speed && ascmc_speed != NULL) {
    ascmc_speed[0] = h.ac_speed;        /* Asc */    
    ascmc_speed[1] = h.mc_speed;        /* Mid */    
    ascmc_speed[2] = h.armc_speed;   
    ascmc_speed[3] = h.vertex_speed;
    ascmc_speed[4] = h.equasc_speed;
    ascmc_speed[5] = h.coasc1_speed;	/* "co-ascendant" (W. Koch) */
    ascmc_speed[6] = h.coasc2_speed;	/* "co-ascendant" (M. Munkasey) */
    ascmc_speed[7] = h.polasc_speed;	/* "polar ascendant" (M. Munkasey) */
    for (i = SE_NASCMC; i < 10; i++)
      ascmc_speed[i] = 0;
  }
  /* The house systems now compute cusp_speed in their own case blocks, so
   * do_interpol is normally clear. It is still honoured, and is also raised
   * here if any analytic speed came back non-finite, so that the generic
   * interpolation below remains available as a fallback rather than letting
   * a NaN or an infinity reach the caller. It exists so that a future house
   * system, or an input outside what has been examined, degrades to the old
   * behaviour instead of returning garbage.
   * The cusp itself is tested as well, because a finite speed beside a
   * non-finite position is not a usable answer either, and the analytic code
   * does produce one: AscDash() decides on d > VERY_SMALL, which is false for
   * a NaN, and so returns 0 rather than propagating it. Sunshine house system
   * 'I' leaves a NaN in some cusps at latitudes beyond about 70 degrees, and
   * reporting those cusps as standing still would hide it. Interpolating a
   * NaN position gives a NaN speed, which is what the caller saw before this
   * patch. */
  if (h.do_hspeed && ! h.do_interpol) {
    for (i = 1; i <= ito; i++) {
      if (! (cusp_speed[i] > -1e300 && cusp_speed[i] < 1e300)
       || ! (cusp[i] > -1e300 && cusp[i] < 1e300)) {
	h.do_interpol = TRUE;
	break;
      }
    }
  }
  if (h.do_interpol) {	// must compute cusp_speed via interpolation
    double dt = 1.0 / 86400;
    double darmc = dt * ARMCS;
    hm1.do_speed = FALSE;
    hm1.do_hspeed = FALSE;
    hp1.do_speed = FALSE;
    hp1.do_hspeed = FALSE;
    if (toupper(hsys) ==  'I') {
      hm1.sundec = h.sundec;
      hp1.sundec = h.sundec;
    }
    rm1 = CalcH(armc - darmc, geolat, eps, (char)hsys, &hm1);
    rp1 = CalcH(armc + darmc, geolat, eps, (char)hsys, &hp1);
    if (rp1 >= 0 && rm1 >=0) {
      if (fabs(swe_difdeg2n(hp1.ac, h.ac)) > 90) {
	hp1 = h;	// use only upper interval
	dt = dt / 2;
      } else if (fabs(swe_difdeg2n(hm1.ac, h.ac)) > 90) {
	hm1 = h;	// use only lower interval
	dt = dt / 2;
      }
      /* ito, not 12. With Gauquelin sectors there are 36 cusps and the guard
       * above scans all 36, so filling only 12 here would leave the bad speed
       * in place in exactly the case that raised do_interpol. ito is 36 only
       * when CalcH returned OK, and the rp1/rm1 test above covers the two
       * offset samples, so all 36 are known good whenever this runs. */
      for (i = 1; i <= ito; i++) {
	double dx = swe_difdeg2n(hp1.cusp[i], hm1.cusp[i]);
	cusp_speed[i] = dx / 2 / dt ;
      }
    }
  }
#ifdef TRACE
  swi_open_trace(NULL);
  if (swi_trace_count <= TRACE_COUNT_MAX) {
    if (swi_fp_trace_c != NULL) {
      fputs("\n/*SWE_HOUSES_ARMC_EX2*/\n", swi_fp_trace_c);
      fprintf(swi_fp_trace_c, "  armc = %.9f;", armc);
      fprintf(swi_fp_trace_c, " geolat = %.9f;", geolat);
      fprintf(swi_fp_trace_c, " eps = %.9f;", eps);
      fprintf(swi_fp_trace_c, " hsys = %d;\n", hsys);
      fprintf(swi_fp_trace_c, "  retc = swe_houses_armc_ex2(armc, geolat, eps, hsys, cusp, ascmc, cusp_speed, ascmc_speed, serr);\n");
      fputs("  printf(\"swe_houses_armc_ex2: %f\\t%f\\t%f\\t%c\\t\\n\", ", swi_fp_trace_c);
      fputs("  armc, geolat, eps, hsys);\n", swi_fp_trace_c);
      fputs("  printf(\"retc = %d\\n\", retc);\n", swi_fp_trace_c);
      fputs("  printf(\"cusp:\\n\");\n", swi_fp_trace_c);
      fputs("  for (i = 1; i <= 12; i++)\n", swi_fp_trace_c);
      fputs("    printf(\"  %d\\t%f\\n\", i, cusp[i]);\n", swi_fp_trace_c);
      fputs("  printf(\"ascmc:\\n\");\n", swi_fp_trace_c);
      fputs("  for (i = 0; i < 10; i++)\n", swi_fp_trace_c);
      fputs("    printf(\"  %d\\t%f\\n\", i, ascmc[i]);\n", swi_fp_trace_c);
      fputs("  printf(\"cusp_speed:\\n\");\n", swi_fp_trace_c);
      fputs("  for (i = 1; i <= 12; i++)\n", swi_fp_trace_c);
      fputs("    printf(\"  %d\\t%f\\n\", i, cusp_speed[i]);\n", swi_fp_trace_c);
      fputs("  printf(\"ascmc_speed:\\n\");\n", swi_fp_trace_c);
      fputs("  for (i = 0; i < 10; i++)\n", swi_fp_trace_c);
      fputs("    printf(\"  %d\\t%f\\n\", i, ascmc_speed[i]);\n", swi_fp_trace_c);
      fflush(swi_fp_trace_c);
    }
    if (swi_fp_trace_out != NULL) {
      fprintf(swi_fp_trace_out, "swe_houses_armc_ex2: %f\t%f\t%f\t%c\t\n", armc, geolat, eps, hsys);
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
/* Derivative of apc_sector() with respect to az, scaled by ARMCS to give
 * degrees per day. Mirrors apc_sector() line for line; if that changes, this
 * has to change with it.
 * APC, introduced by L. Knegt and used by the Dutch Werkgemeenschap van
 * Astrologen, divides the diurnal and nocturnal arcs of the Ascendant into
 * equal segments and carries great circles from the north point through them
 * (astro.com/swisseph/sweph_ht_e.htm), so
 * cusp 1 is the Ascendant and cusp 10 the MC exactly, and those two take
 * ac_speed and mc_speed rather than this function. Cusps 12-6, 11-5, 2-8 and
 * 3-9 are not exactly opposite in this system, which is why case 'Y' is
 * excluded from the mirroring below and each of the twelve is computed here. Differentiating
 *   kv = atan(T cos az / (1 + T sin az)),  T = tan(ph) tan(e)
 * gives the compact
 *   dkv/daz = -T (sin az + T) / (1 + T^2 + 2 T sin az),
 * and the rest follows by the chain rule through sin(kv), a and the final
 * atan2.
 * Being analytic, this stays correct where the Ascendant turns arbitrarily
 * fast, which is exactly where differencing the position fails. */
static double apc_sector_speed(int n, double ph, double e, double az)
{
   int k, is_below_hor = 0;
   double kv, dkv = 0, T, den, a, da, skv, dskv, P, N, D, dN, dD;
   /* CalcH() clamps fi to exactly 90 - VERY_SMALL before the switch, so for a
    * valid latitude this test is never true; it catches latitudes outside
    * +-90. Kept to match apc_sector(), which guards the same way. */
   if (fabs(ph * RADTODEG) > 90 - VERY_SMALL) {
     kv = 0;
   } else {
     T = tan(ph) * tan(e);
     kv = atan(T * cos(az) / (1 + T * sin(az)));
     /* A sum of squares rather than 1 + T*T + 2*T*sin(az). The two are the
     * same number, but the second is a difference of near-equal quantities
     * when |T| approaches 1 with sin(az) near -1, and it can round to exactly
     * zero there. The guard below then fires and reports twelve moving cusps
     * as stationary, which is worse than the interpolation it replaces. This
     * form cannot cancel and cannot come out negative. */
    den = (1 + T * sin(az)) * (1 + T * sin(az)) + (T * cos(az)) * (T * cos(az));
     if (fabs(den) < VERY_SMALL_DENOM)
       return 0;
     dkv = -T * (sin(az) + T) / den;
   }
   /* apc_sector() uses the declination of the Ascendant only through the
    * product tan(dasc) * tan(ph), which the ascensional difference relation
    * makes equal to sin(kv). Differentiating that form rather than the round
    * trip through dasc keeps this function defined on the equator, where the
    * round trip is 0/0. Upstream apc_sector() breaks that tie with a constant
    * and returns wrong cusps within 1e-10 degrees of the equator; this
    * derivative follows the corrected form, so in that band it will not agree
    * with an unpatched apc_sector(). Everywhere else the two forms agree to a
    * few units in the last place and it makes no difference. */
   skv = sin(kv);
   dskv = cos(kv) * dkv;
   if (n < 8) {
     is_below_hor = 1;
     k = n - 1;
   } else {
     k = n - 13;
   }
   if (is_below_hor) {
     a = kv + az + PI/2 + k * (PI/2 - kv) / 3;
     da = dkv * (1 - k / 3.0) + 1;
   } else {
     a = kv + az + PI/2 + k * (PI/2 + kv) / 3;
     da = dkv * (1 + k / 3.0) + 1;
   }
   a = swe_radnorm(a);
   P = tan(ph);
   N = skv * sin(az) + sin(a);
   D = cos(e) * (skv * cos(az) + cos(a)) + sin(e) * P * sin(az - a);
   dN = dskv * sin(az) + skv * cos(az) + cos(a) * da;
   dD = cos(e) * (dskv * cos(az) - skv * sin(az) - sin(a) * da)
      + sin(e) * P * cos(az - a) * (1 - da);
   if (N * N + D * D < VERY_SMALL_DENOM)	/* atan2() has no direction */
     return 0;
   return (dN * D - N * dD) / (N * N + D * D) * ARMCS;
}

static double apc_sector(int n, double ph, double e, double az)
{
   int k, is_below_hor = 0;
   double kv, a, skv, dret;
   /* kv: ascensional difference of the ascendant */
   if (fabs(ph * RADTODEG) > 90 - VERY_SMALL) {
     kv = 0;
   } else {
     kv   = atan(tan(ph) * tan(e) * cos(az)/(1 + tan(ph) * tan(e) * sin(az)));
   }
   /* The declination of the ascendant, dasc, only ever entered this function
    * as the product tan(dasc) * tan(ph), and the ascensional difference
    * relation sin(kv) = tan(ph) * tan(dasc) says that product is sin(kv).
    * Forming dasc = atan(sin(kv) / tan(ph)) and multiplying by tan(ph) again
    * is a round trip that is 0/0 on the equator, where both sin(kv) and
    * tan(ph) vanish. The old code broke the tie there with a constant dasc,
    * which made the product some 2e12 times too large and returned cusps that
    * were not the ascendant and descendant at all. Taking sin(kv) directly
    * removes the round trip and the special case together, and agrees with
    * the old form to a few units in the last place wherever it was defined. */
   skv = sin(kv);
   /* note, at polar circles, when the mc sinks below the horizon,
    * kv, and with it skv, changes sign in the above formula.
    * this is what we need, because the ascendand jumps by 180 deg */
   /* printf("%f, %f\n", kv*RADTODEG, skv); */
   if (n < 8) {
     is_below_hor = 1;  /* 1 and 7 are included here */
     k = n - 1;
   } else {
     k = n - 13;
   }
   /* az + PI/2 + kv = armc + 90 + asc. diff. = right ascension of ascendant
    * PI/2 +- kv = semi-diurnal or seminocturnal arc of ascendant 
    * a = right ascension of house cusp on apc circle (ascendant-parallel
    * circle), whose declination is the ascendant's */
   if (is_below_hor) {
     a = kv + az + PI/2 + k * (PI/2 - kv) / 3;
   } else {
     a = kv + az + PI/2 + k * (PI/2 + kv) / 3;
   }
   a = swe_radnorm(a);
   dret = atan2(skv * sin(az) + sin(a),
      cos(e) * (skv * cos(az) + cos(a)) + sin(e) * tan(ph) * sin(az - a));
   dret = swe_degnorm(dret * RADTODEG);
   return dret;
}

const char *CALL_CONV swe_house_name(int hsys)
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
  case 'J': return "Savard-A";
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
// We follow the proposal by Dieter Koch, who wants to keep it in analogy with
// Regiomontanus, where we keep the MC above the horizon, by switching it to the north.
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

//#define DEBUG_PLAC_ITER 1
#define VERY_SMALL_PLAC_ITER (1.0 / 360000.0 )
static int CalcH(
	double th, double fi, double ekl, char hsy, struct houses *hsp)
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
 *                   J  Savard-A
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
 * *********************************************************
 *  Koch and Placidus don't work in the polar circle.
 *  We swap MC/IC so that MC is always before AC in the zodiac
 *  We then divide the quadrants into 3 equal parts, ie apply Porphyry.
 * *********************************************************
 *  All angles are expressed in degrees.
 *  Special trigonometric functions sind, cosd etc. are
 *  implemented for arguments in degrees.
 ***********************************************************/
{
  double tane, tanfi, cosfi, sinfi, tant, sina, cosa, th2, mc0;
  double a, c, f, fh1, fh2, xh1, xh2, xs1, xs2, rectasc, ad3, acmc, vemc;
  int 	i, ih, ih2, retc = OK;
  int 	tant_small;		/* the |tant| < VERY_SMALL branch of a Placidus or
				 * Gauquelin iteration fired and has already set
				 * the speed, so plac_speed() must not overwrite it
				 * with a pole height from an earlier iterate */
  double sine, cose;
  double x[3], krHorizonLon; /* BK 14.02.2006 */
  int niter_max = 100; // maximum iterations allowed with Placidus
  double cuspsv;
  *hsp->serr = '\0';
  hsp->do_interpol = 0;
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
  if (hsp->do_speed) hsp->mc_speed = AscDash(th, 0, sine, cose); 
  // ascendant
  // In case of ascendant, the great circle is the horizon, which has pole height latitude.
  // intersection equator horizon is at th + 90, reactasce 90° east of meridian.
  hsp->ac = Asc1(th + 90, fi, sine, cose);
  if (hsp->do_speed) 
    hsp->ac_speed = AscDash(th + 90, fi, sine, cose);
  if (hsp->do_hspeed) {
    for (i = 0; i <= 12; i++)
      hsp->cusp_speed[i] = 0;
  }
  hsp->armc_speed = ARMCS;
  // these cusp[1] and cusp[10] values may be changed further down for some house systems
  hsp->cusp[1] = hsp->ac;
  hsp->cusp[10] = hsp->mc;
  if (hsp->do_hspeed) {	
    hsp->cusp_speed[1] = hsp->ac_speed;
    hsp->cusp_speed[10] = hsp->mc_speed;
  }
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
    for (i = 2; i <=12; i++) {
      hsp->cusp[i] = swe_degnorm(hsp->cusp[1] + (i-1) * 30);
    }
    if (hsp->do_hspeed) {
      for (i = 1; i <=12; i++) {
	hsp->cusp_speed[i] = hsp->ac_speed;
      }
    }
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
    if (hsp->do_hspeed) {
      for (i = 1; i <=12; i++) {
	hsp->cusp_speed[i] = hsp->mc_speed;
      }
    }
    break;
  case 'C': // Campanus houses:
    // Prime vertical is divided into 3 parts of 30° each, great circles from
    // north point to south point go through these points and intersect ecliptic.
    // pole height = shortest distance of plane from North pole,
    // measured along declination circle.
    // NP = North Pole, N = north point, P11 = house 11 point in prime vertical
    // triangle NP - N - P11 , angle 30 at N, side fi between NP and N
    // sin fh1 = sin fi * sin 30°,
    fh1 = asind(sind (fi) / 2);
    // triangle NP - N - P12 , angle 60 at N, side fi between NP and N
    // sin fh2 = sin fi * sin 60°,
    fh2 = asind(sqrt (3.0) / 2 * sind(fi)); 
    cosfi = cosd(fi);
    if (fabs(cosfi) == 0) {	/* '==' should be save! */ 
      if (fi > 0)
	xh1 = xh2 = 90; /* cosfi = VERY_SMALL; */
      else
	xh1 = xh2 = 270; /* cosfi = -VERY_SMALL; */
    } else {
      // triangle formed by equator, prime vertical, great circle 
      // through P11, S and N
      // with right angle between prime vertical and great circle
      // side length xh1 on equ, 60 on prime vertical, angle fi between
      // tan xh1 = tan 60 / cos fi = √3 / cos fi
      xh1 = atand(sqrt (3.0) / cosfi);	
      // side length xh2 on equ, 30° on prime vertical, angle fi between
      // tan xh2 = tan 30 / cos fi = 1/√3 / cos fi
      xh2 = atand(1 / sqrt (3.0) / cosfi);
    }
    hsp->cusp[11] = Asc1(th + 90 - xh1, fh1, sine, cose);
    hsp->cusp[12] = Asc1(th + 90 - xh2, fh2, sine, cose);
    hsp->cusp[2] = Asc1(th + 90 + xh2, fh2, sine, cose);
    hsp->cusp[3] = Asc1(th + 90 + xh1, fh1, sine, cose);
    if (hsp->do_hspeed) {
      hsp->cusp_speed[11] = AscDash(th + 90 - xh1, fh1, sine, cose);
      hsp->cusp_speed[12] = AscDash(th + 90 - xh2, fh2, sine, cose);
      hsp->cusp_speed[2] = AscDash(th + 90 + xh2, fh2, sine, cose);
      hsp->cusp_speed[3] = AscDash(th + 90 + xh1, fh1, sine, cose);
    }
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
  case 'H': /* Horizon or Azimut system, similar to Campanus calulation */
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
    fh1 = asind(sind (fi) / 2);
    fh2 = asind(sqrt (3.0) / 2 * sind(fi)); 
    cosfi = cosd(fi);
    if (fabs(cosfi) == 0) {	/* '==' should be save! */ 
      if (fi > 0)
	xh1 = xh2 = 90; /* cosfi = VERY_SMALL; */
      else
	xh1 = xh2 = 270; /* cosfi = -VERY_SMALL; */
    } else {
      // triangle formed by equator, prime vertical, declination circle,
      // with right angle between equator and declination circle:
      // side length xh1 on equ, 60 on prime vertical, angle fi between
      // tan xh1 = tan 60 / cos fi = √3 / cos fi
      xh1 = atand(sqrt (3.0) / cosfi);	
      // side length xh2 on equ, 30° on prime vertical, angle fi between
      // tan xh2 = tan 30 / cos fi = 1/√3 / cos fi
      xh2 = atand(1 / sqrt (3.0) / cosfi);
    }
    hsp->cusp[11] = Asc1(th + 90 - xh1, fh1, sine, cose);
    hsp->cusp[12] = Asc1(th + 90 - xh2, fh2, sine, cose);
    hsp->cusp[1] = Asc1(th + 90, fi, sine, cose);
    hsp->cusp[2] = Asc1(th + 90 + xh2, fh2, sine, cose);
    hsp->cusp[3] = Asc1(th + 90 + xh1, fh1, sine, cose);
    if (hsp->do_hspeed) {
      hsp->cusp_speed[11] = AscDash(th + 90 - xh1, fh1, sine, cose);
      hsp->cusp_speed[12] = AscDash(th + 90 - xh2, fh2, sine, cose);
      hsp->cusp_speed[1] = AscDash(th + 90, fi, sine, cose);
      hsp->cusp_speed[2] = AscDash(th + 90 + xh2, fh2, sine, cose);
      hsp->cusp_speed[3] = AscDash(th + 90 + xh1, fh1, sine, cose);
    }
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
    break;
  /* Sunshine houses, invented by Bob Makransky and published in 1988 in
   * "Primary Directions: A Primer of Calculation". Two solutions of the same
   * definition: Treindl for 'I', Makransky for 'i'. */
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
    /* Cusps 1 and 10 are the Ascendant and the MC and already have their
     * speeds from before the switch; 4 and 7 are their opposites and share
     * them. The sunshine solution below fills in the other eight, skipping
     * exactly these four. */
    if (hsp->do_hspeed) {
      hsp->cusp_speed[4] = hsp->cusp_speed[10];
      hsp->cusp_speed[7] = hsp->cusp_speed[1];
    }
    if (hsy == 'I') {
      /* Sunshine, invented by Bob Makransky and published in 1988 in
       * "Primary Directions: A Primer of Calculation", divides the Sun's
       * diurnal and nocturnal arcs into six each and projects the division
       * points with house circles. Cusp 1 is therefore the Ascendant and
       * cusp 10 the MC exactly, which is why they are assigned above rather
       * than solved for, while cusps 11 12 2 3 are not opposite 5 6 8 9 and
       * the mirroring below is skipped.
       * The cusps depend on the Sun's declination as well as on armc, but
       * this function receives that declination as a bare number in
       * ascmc[9] with no rate attached, so the speeds below hold it fixed,
       * exactly as the interpolation they replace did. The omitted term
       * d(cusp)/d(sundec) * d(sundec)/dt reaches about 0.14 deg/day at
       * latitude 35, 0.6 at latitude 52, and under half a percent of the
       * reported speed as far as latitude 70. A caller wanting the total
       * derivative has to add it. */
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
  case 'J': /* Savard's supposed Albategnius houses */
    // house 11: latitude circle at 2/3 fi intersects prime meridian at p11.
    // house 12: latitude circle at fi / 3 intersects prime meridian at p12.
    // triangle X-p12-E formed by equator, prime vertical, declination circle,
    // side length xs1 or xs2 on prime vertical, 2/3 fi or fi/3  on declination circle,
    // angle fi between prime vertical and equator at E, right angle between equator
    // and declination circle.
    // sin b = sin B sin c, with b = 1/3 or 2/3 fi, B = fi
    sinfi = sind(fi);
    cosfi = cosd(fi);
    if (fabs(fi) < VERY_SMALL) {	
      xs2 = 1 / 3.0;
      xs1 = 2 / 3.0;
    } else {
      xs2 = sind(fi / 3) / sinfi;	
      xs1 = sind(2 * fi / 3) / sinfi;
    }
    xs2 = asind(xs2);
    xs1 = asind(xs1);
    // now consider triangle great circle through h11, equ, prime vertical, with
    // right angle between prime vertical and great circle
    // side length xh1 on equ, xs1 on prime vertical, angle fi between
    // tan xh1 = tan xs1 / cos fi 
    if (cosfi == 0) {
      if (fi > 0)
	xh1 = xh2 = 90; 
      else
	xh1 = xh2 = 270;
    } else {
      xh1 = atand(tand(xs1) / cosfi);	
      xh2 = atand(tand(xs2) / cosfi);	
    }
    // Pole height:
    // great circle S - p11 - N has angle 90 - xs1 on North or South point,
    // north point to south point go through these points and intersect ecliptic.
    // pole height = shortest distance of plane from North pole,
    // measured along declination circle.
    // NP = North Pole, N = north point, H11 = house 11 point in prime vertical
    // triangle NP - N - h11 , angle xs1 at N, side fi between NP and N
    // sin fh1 = sin fi * sin (90 - xs1),
    fh1 = asind(sind (fi) * sind(90 - xs1));
    fh2 = asind(sind (fi) * sind(90 - xs2));
    hsp->cusp[12] = Asc1(th + 90 - xh2, fh2, sine, cose);
    hsp->cusp[11] = Asc1(th + 90 - xh1, fh1, sine, cose);
    hsp->cusp[2] = Asc1(th + 90 + xh2, fh2, sine, cose);
    hsp->cusp[3] = Asc1(th + 90 + xh1, fh1, sine, cose);
    if (hsp->do_hspeed) {
      hsp->cusp_speed[11] = AscDash(th + 90 - xh1, fh1, sine, cose);
      hsp->cusp_speed[12] = AscDash(th + 90 - xh2, fh2, sine, cose);
      hsp->cusp_speed[3] = AscDash(th + 90 + xh1, fh1, sine, cose);
      hsp->cusp_speed[2] = AscDash(th + 90 + xh2, fh2, sine, cose);
    }
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
  case 'K': /* Koch houses */
    if (fabs(fi) >= 90 - ekl) {  /* within polar circle */
      retc = ERR;
      strcpy(hsp->serr, "within polar circle, switched to Porphyry"); 
      goto porphyry;
    }
    mc0 = hsp->mc;
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
    if (hsp->do_hspeed) {
      /* Derivative of the four Asc1() calls just above. If the ad3 or sina
       * expressions above change, this block has to change with them.
       * Koch trisects the semi-arc of the MC rather than of each cusp, which
       * is why one ascensional difference ad3 serves all four and why the
       * derivative is a single chain through the MC. Like Placidus it is
       * undefined beyond the polar circles, and the case bails to Porphyry
       * there before reaching this block.
       * ad3 is a third of the ascensional difference of the MC, so it moves
       * as the MC does and the arguments of Asc1() below advance at a rate
       * different from armc's. AscDash() supplies the derivative with respect
       * to its first argument, already scaled by ARMCS, so it needs only the
       * factor by which that argument actually moves.
       * Differentiating ad3 = asind(sind(c) * sina) / 3 through
       * c = atand(tanfi / cosa) and sina = sind(mc) * sine / cosd(fi):  */
      double dad3 = 0, kk;
      /* Neither test can fail as the code stands: the polar circle bailout
       * above keeps cos(fi) above sin(ekl), so sina stays strictly inside
       * +-1, cosa strictly above 0, and uu = sind(c) * sina with it. They
       * guard the asind() and the division, and cost one comparison. */
      if (cosa > VERY_SMALL) {
	double uu = sind(c) * sina;
	if (fabs(uu) < 1) {
	  double dsina_dmc = cosd(mc0) * DEGTORAD * sine / cosd(fi);
	  double dcosa_dsina = -sina / cosa;
	  double dc_dcosa = (1 / DEGTORAD) * (-tanfi / (cosa * cosa))
			  / (1 + (tanfi / cosa) * (tanfi / cosa));
	  double du_dmc = (cosd(c) * DEGTORAD * sina * dc_dcosa * dcosa_dsina
			   + sind(c)) * dsina_dmc;
	  dad3 = du_dmc / (3 * DEGTORAD * sqrt(1 - uu * uu)) * hsp->mc_speed;
	}
      }
      kk = dad3 / ARMCS;
      hsp->cusp_speed[11] = AscDash(th + 30 - 2 * ad3, fi, sine, cose) * (1 - 2 * kk);
      hsp->cusp_speed[12] = AscDash(th + 60 - ad3, fi, sine, cose) * (1 - kk);
      hsp->cusp_speed[2] = AscDash(th + 120 + ad3, fi, sine, cose) * (1 + kk);
      hsp->cusp_speed[3] = AscDash(th + 150 + 2 * ad3, fi, sine, cose) * (1 + 2 * kk);
    }
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
      /* Every cusp above is the Ascendant or the MC plus a term that depends
       * on the quadrant size acmc only, so the speeds follow by the chain rule
       * from ac_speed and mc_speed. dq is the rate at which the 4th quadrant
       * grows; the 1st quadrant q1 = 180 - acmc grows at -dq.
       * The zero-width branches have their own derivative and are handled
       * separately, as their cusps depend on acmc with a different factor.
       * The switch at 30 degrees is part of the definition, not a numerical
       * guard: Pullen (astrolog.org/astrolog/astsine.htm) states that below a
       * 30 degree quadrant the middle house
       * is pinned at zero width and the quadrant is simply bisected. The
       * position is continuous there but its slope is not, so the two branches
       * here must track the two branches above exactly. */
      if (hsp->do_hspeed) {
	double dq = hsp->ac_speed - hsp->mc_speed;
	if (acmc <= 30) {
	  hsp->cusp_speed[11] = hsp->cusp_speed[12] = hsp->mc_speed + dq / 2;
	} else {
	  hsp->cusp_speed[11] = hsp->mc_speed + dq / 4;
	  hsp->cusp_speed[12] = hsp->mc_speed + dq * 3 / 4;
	}
	if (q1 <= 30) {
	  hsp->cusp_speed[2] = hsp->cusp_speed[3] = hsp->ac_speed - dq / 2;
	} else {
	  hsp->cusp_speed[2] = hsp->ac_speed - dq / 4;
	  hsp->cusp_speed[3] = hsp->ac_speed - dq * 3 / 4;
	}
      }
    }
    break;
  /* Whole signs from 0 Aries: the cusps are the twelve sign boundaries and do
   * not depend on armc at all, so every speed is zero. The code before this
   * patch left ac_speed on cusp 1 and mc_speed on cusp 10, and the mirroring
   * carried them to 7 and 4, describing a motion these cusps do not have. */
  case 'N':	/* whole signs, begin at 0° Aries */
    acmc = swe_difdeg2n(hsp->ac, hsp->mc);
    if (acmc < 0) {
      /* within polar circle we swap AC/DC if AC is on wrong side */
      hsp->ac = swe_degnorm(hsp->ac + 180);
    }
    for (i = 1; i <= 12; i++) 
      hsp->cusp[i] = (i - 1) * 30.0;
    /* These cusps are constants, they do not move at all. Without this the
     * Ascendant and MC speeds set before the switch would be reported for
     * cusps 1 and 10 (and, through the mirroring below, for 4 and 7). */
    if (hsp->do_hspeed)
      for (i = 1; i <= 12; i++)
	hsp->cusp_speed[i] = 0;
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
    hsp->cusp[1] = hsp->ac;  // may have been destroyed if defaulting from Gauquelin
    hsp->cusp[10] = hsp->mc; // dito
    hsp->cusp[2] = swe_degnorm(hsp->ac + (180 - acmc) / 3);
    hsp->cusp[3] = swe_degnorm(hsp->ac + (180 - acmc) / 3 * 2);
    hsp->cusp[11] = swe_degnorm(hsp->mc + acmc / 3);
    hsp->cusp[12] = swe_degnorm(hsp->mc + acmc / 3 * 2);
    /* Porphyry trisects each ecliptic quadrant equally, so every cusp is the
     * Ascendant or the MC plus a fixed fraction of the quadrant acmc and the
     * speeds follow directly. Cusps 11 and 12 are offsets from the MC and
     * grow from mc_speed; before this patch they grew from ac_speed, which
     * was wrong. */
    if (hsp->do_hspeed) {
      double q1_speed = hsp->ac_speed - hsp->mc_speed;	// rate of growth of quadrant 1
      // double q4_speed = hsp->mc_speed - hsp->ac_speed;	// rate of growth of quadrant 4
      hsp->cusp_speed[1] = hsp->ac_speed;  // may have been destroyed if defaulting from Gauquelin
      hsp->cusp_speed[10] = hsp->mc_speed; // dito
      hsp->cusp_speed[2] = hsp->ac_speed  - q1_speed / 3;
      hsp->cusp_speed[3] = hsp->ac_speed  - q1_speed / 3 * 2;
      /* cusps 11 and 12 are offsets from the MC, not from the Ascendant,
       * so they grow from mc_speed. q1_speed is d(acmc)/dt. */
      hsp->cusp_speed[11] = hsp->mc_speed  + q1_speed / 3;
      hsp->cusp_speed[12] = hsp->mc_speed  + q1_speed / 3 * 2;
    }
    break;
  case 'Q':	/* Pullen sinusoidal ratio */
    {
      double q, x, xr, xr3, xr4, rr = 0;
      acmc = swe_difdeg2n(hsp->ac, hsp->mc);
      if (acmc < 0) {
      /* within polar circle we swap AC/DC if AC is on wrong side */
       hsp->ac = swe_degnorm(hsp->ac + 180);
       hsp->cusp[1] = hsp->ac;
       acmc = swe_difdeg2n(hsp->ac, hsp->mc);
      }
      q = acmc;
      if (q > 90) q = 180 - q;
      pullen_sr_sectors(q, &x, &xr, &xr3, &xr4, &rr);
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
      /* The two branches below repeat the width-to-cusp mapping of the two
       * branches just above. Keep the pairs in step.
       * The sector widths depend on the quadrant size q alone, and q on the
       * Ascendant and MC, so the speeds follow by the chain rule. The widths
       * are a root of a quartic in q and unpleasant to differentiate as the
       * radicals above are written, so pullen_sr_slopes() differentiates the
       * two defining equations implicitly instead. That is exact, and it is
       * defined at q = 90, where a difference taken in q would have to turn
       * one-sided. Nothing here differences the cusp positions with respect to
       * armc, which is what the interpolation this patch replaces did,
       * straight through the direction in which they can jump. */
      if (hsp->do_hspeed) {
	double dq = hsp->ac_speed - hsp->mc_speed;	// rate of growth of acmc
	double dxdq, dxrdq, dxr3dq, dxr4dq;
	if (q < 1e-4) {
	  /* The quadrant is collapsing, as it does at the geographic pole where
	   * the Ascendant and the MC coincide. r runs away there, so the term is
	   * dropped and the cusps move with the Ascendant and the MC alone. */
	  dxdq = dxrdq = dxr3dq = dxr4dq = 0;
	} else {
	  pullen_sr_slopes(q, rr, x, xr, xr3, &dxdq, &dxrdq, &dxr3dq, &dxr4dq);
	}
	/* q = acmc below 90 degrees, q = 180 - acmc above it */
	if (acmc > 90) dq = -dq;
	if (acmc > 90) {
	  hsp->cusp_speed[11] = hsp->mc_speed + dxr3dq * dq;
	  hsp->cusp_speed[12] = hsp->cusp_speed[11] + dxr4dq * dq;
	  hsp->cusp_speed[2] = hsp->ac_speed + dxrdq * dq;
	  hsp->cusp_speed[3] = hsp->cusp_speed[2] + dxdq * dq;
	} else {
	  hsp->cusp_speed[11] = hsp->mc_speed + dxrdq * dq;
	  hsp->cusp_speed[12] = hsp->cusp_speed[11] + dxdq * dq;
	  hsp->cusp_speed[2] = hsp->ac_speed + dxr3dq * dq;
	  hsp->cusp_speed[3] = hsp->cusp_speed[2] + dxr4dq * dq;
	}
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
    if (hsp->do_hspeed) {
      hsp->cusp_speed[11] = AscDash(30 + th, fh1, sine, cose); 
      hsp->cusp_speed[12] = AscDash(60 + th, fh2, sine, cose); 
      hsp->cusp_speed[2] = AscDash(120 + th, fh2, sine, cose);
      hsp->cusp_speed[3] = AscDash(150 + th, fh1, sine, cose); 
    }
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
      /* As with Porphyry above, the cusps are the Ascendant or the MC plus a
       * multiple of a sector width, and the sector widths depend only on the
       * quadrant size acmc, which grows at dq = ac_speed - mc_speed.
       * So s4 grows at dq/3 and s1 at -dq/3, and the speeds follow directly.
       * Sripati shifts the Porphyry cusps back by half a sector, so cusp 1
       * is not the Ascendant for this house system, it is
       * half a sector before it, so it does not have ac_speed. */
      if (hsp->do_hspeed) {
	double dq = hsp->ac_speed - hsp->mc_speed;
	hsp->cusp_speed[1] = hsp->ac_speed - dq / 6;
	hsp->cusp_speed[2] = hsp->ac_speed - dq / 6;
	hsp->cusp_speed[3] = hsp->ac_speed - dq / 2;
	hsp->cusp_speed[10] = hsp->mc_speed + dq / 6;
	hsp->cusp_speed[11] = hsp->mc_speed + dq / 6;
	hsp->cusp_speed[12] = hsp->mc_speed + dq / 2;
      }
    }
    break;
  case 'T':	/* 'topocentric' houses */
    fh1 = atand (tanfi / 3.0);
    fh2 = atand (tanfi * 2.0 / 3.0);
    hsp->cusp[11] =  Asc1(30 + th, fh1, sine, cose); 
    hsp->cusp[12] =  Asc1(60 + th, fh2, sine, cose);
    hsp->cusp[2] =  Asc1(120 + th, fh2, sine, cose); 
    hsp->cusp[3] =  Asc1(150 + th, fh1, sine, cose);
    if (hsp->do_hspeed) {
      hsp->cusp_speed[11] =  AscDash(30 + th, fh1, sine, cose); 
      hsp->cusp_speed[12] =  AscDash(60 + th, fh2, sine, cose);
      hsp->cusp_speed[2] =  AscDash(120 + th, fh2, sine, cose); 
      hsp->cusp_speed[3] =  AscDash(150 + th, fh1, sine, cose);
    }
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
    }
    hsp->cusp[1] = swe_degnorm(hsp->ac - 15);
    for (i = 2; i <=12; i++)
      hsp->cusp[i] = swe_degnorm(hsp->cusp[1] + (i-1) * 30);
    if (hsp->do_hspeed) {
      for (i = 1; i <=12; i++) {
	hsp->cusp_speed[i] = hsp->ac_speed;
      }
    }
    break;
  case 'W':	/* equal, whole-sign houses */
    acmc = swe_difdeg2n(hsp->ac, hsp->mc);
    if (acmc < 0) {
      /* within polar circle we swap AC/DC if AC is on wrong side */
      hsp->ac = swe_degnorm(hsp->ac + 180);
      hsp->cusp[1] = hsp->ac;
    }
    /* Whole sign houses: the cusps sit on sign boundaries and do not move
     * between ingresses, so every speed is zero. The derivative does not
     * exist at an ingress and is zero everywhere else; reporting ac_speed
     * here, as the code did before, described a motion the cusps do not
     * have. */
    hsp->cusp[1] = hsp->ac - fmod(hsp->ac, 30);
    for (i = 2; i <=12; i++)
      hsp->cusp[i] = swe_degnorm(hsp->cusp[1] + (i-1) * 30);
    /* The cusps are the start of the Ascendant's sign and fixed multiples of
     * 30 degrees from it. They stand still while the Ascendant crosses a sign
     * and jump when it enters the next one, so the derivative is zero
     * everywhere it exists. */
    if (hsp->do_hspeed)
      for (i = 1; i <= 12; i++)
	hsp->cusp_speed[i] = 0;
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
    /* Each cusp is the ecliptic point whose right ascension is armc plus a
     * fixed multiple of 30 degrees, so that right ascension advances at
     * exactly the rate of armc. Differentiating tan(lon) = tan(a) / cos(eps)
     * gives d(lon)/da = (cos^2 lon + cos^2 eps sin^2 lon) / cos(eps), which is
     * never zero or singular.
     * Nothing here depends on geographic latitude, and neither do the cusps,
     * as the definition requires (astro.com/swisseph/sweph_ht_e.htm). Cusp 10 is the MC and takes mc_speed to the
     * last bit, but cusp 1 is the equatorial ascendant, not the Ascendant, so
     * it does not take ac_speed; the loop overwrites the value set before the
     * switch. */
    if (hsp->do_hspeed) {
      for (i = 1; i <= 12; i++) {
	double cl = cosd(hsp->cusp[i]), sl = sind(hsp->cusp[i]);
	hsp->cusp_speed[i] = ARMCS * (cl * cl + cose * cose * sl * sl) / cose;
      }
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
    /* As with the meridian system above, each cusp comes from a point of the
     * equator whose right ascension advances with armc, so only the conversion
     * to the ecliptic has to be differentiated. Here swe_cotrans() carries the
     * equatorial point into the ecliptic the other way round, giving
     * tan(lon) = cos(eps) tan(a) and hence
     * d(lon)/da = (cos^2 eps cos^2 lon + sin^2 lon) / cos(eps).
     * Morinus projects from the pole of the ecliptic rather than the pole of
     * the equator (astro.com/swisseph/sweph_ht_e.htm), so neither the cusps
     * nor these speeds depend on geographic
     * latitude. Cusp 1 is not the Ascendant and cusp 10 is not quite the MC,
     * so all twelve are taken from the formula and neither ac_speed nor
     * mc_speed survives from before the switch. */
    if (hsp->do_hspeed) {
      for (i = 1; i <= 12; i++) {
	double cl = cosd(hsp->cusp[i]), sl = sind(hsp->cusp[i]);
	hsp->cusp_speed[i] = ARMCS * (cose * cose * cl * cl + sl * sl) / cose;
      }
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
    /* Every cusp is the ecliptic longitude whose right ascension is the
     * Ascendant's plus a fixed multiple of 30 degrees, so all of them move
     * because the Ascendant does. With
     *   dRA/dlon = cos(eps) / (cos^2 lon + cos^2 eps * sin^2 lon)
     * the chain rule through RA gives
     *   speed[i] = ac_speed * w(cusp[i]) / w(ac),  w(L) = cos^2 L + cos^2 eps * sin^2 L,
     * the cos(eps) factors cancelling. w is never zero, so this is safe
     * everywhere, including where the cusp formula above takes its
     * ra = 90 or 270 branch.
     * The loop starts at 2 because cusp 1 is the Ascendant in this system and
     * already carries ac_speed. Cusp 10 is not the MC
     * (astro.com/swisseph/sweph_ht_e.htm), and is covered by the loop like
     * the rest. */
    if (hsp->do_hspeed) {
      double wac = cosd(hsp->ac) * cosd(hsp->ac)
                 + cose * cose * sind(hsp->ac) * sind(hsp->ac);
      for (i = 2; i <= 12; i++) {
	if (i <= 3 || i >= 10) {
	  double wc = cosd(hsp->cusp[i]) * cosd(hsp->cusp[i])
	            + cose * cose * sind(hsp->cusp[i]) * sind(hsp->cusp[i]);
	  hsp->cusp_speed[i] = hsp->ac_speed * wc / wac;
	}
      }
    }
    break; }
  case 'B': {	/* Alcabitius */
      // created by Alois 17-sep-2000, followed example in Matrix
      // electrical library. The code reproduces the example!
      // This corresponds to Munkasey 'The Alcibitius Semiarc House System'
      // as described in his Astrological House Formulae'
      double dek, r, sna, sda, sn3, sd3;
      acmc = swe_difdeg2n(hsp->ac, hsp->mc);
      if (acmc < 0) {
	hsp->ac = swe_degnorm(hsp->ac + 180);
	hsp->cusp[1] = hsp->ac;
	acmc = swe_difdeg2n(hsp->ac, hsp->mc);
      }
      dek = asind(sind(hsp->ac) * sine);	/* declination of Ascendant */
      // triangle horizon - decl circle - equator, right angle between equ and 
      // decl circle, angle 90 - fi between horizon and equator A, decl = a
      // tan a = sin b tan A, sin b = tan decl * cot (90-fi) = tan decl * tan fi
      // We want semidiurnal circle  90 + b; cos (90 + b) = - sin b = r = -tan decl * tan fi
      // sda = arccos r
      // case fi == 90 or -90 is dealt with at entry into function
      r = -tanfi * tand(dek);
      // must treat the case of abs(r) > 1; happens very rarely
      // because dek becomes smaller when fi is large, as ac is close to
      // zero Aries/Libra in that case.
      if (r > 1) r = 1;
      if (r < -1) r = -1;
      sda = acosd(r);	// semidiurnal arc, measured on equator 
      sna = 180 - sda;	// complement, seminocturnal arc
      sd3 = sda / 3;
      sn3 = sna / 3;
      rectasc = swe_degnorm(th + sd3);	/* cusp 11 */
      // project rectasc onto eclipitic with pole height 0, i.e. along the
      // declination circle 
      hsp->cusp[11] = Asc1(rectasc, 0, sine, cose);
      rectasc = swe_degnorm(th + 2 * sd3);	/* cusp 12 */
      hsp->cusp[12] = Asc1(rectasc, 0, sine, cose);
      rectasc = swe_degnorm(th + 180 - 2 * sn3);	/* cusp 2 */
      hsp->cusp[2] = Asc1(rectasc, 0, sine, cose);
      rectasc = swe_degnorm(th + 180 -  sn3);	/* cusp 3 */
      hsp->cusp[3] = Asc1(rectasc, 0, sine, cose);
      /* Derivative of the four Asc1() calls just above. If the sda, sd3 or
       * sn3 expressions above change, this block has to change with them.
       * Alcabitius trisects the diurnal arc of the Ascendant per quadrant and
       * draws meridian circles through the division points
       * (astro.com/swisseph/sweph_ht_e.htm), so sda below is
       * the Ascendant's semidiurnal arc and moves with the Ascendant alone.
       * The opposite cusps are exactly 180 degrees away, so the mirroring
       * below applies and their speeds are equal.
       * Each cusp is Asc1() of an equatorial point that moves with th and, in
       * addition, with the semidiurnal arc sda, which depends on the
       * declination of the Ascendant and so on the Ascendant itself.
       * Differentiating sda = acos(-tan(fi) * tan(dek)) with
       * dek = asin(sin(ac) * sin(eps)) gives
       *   d(sda)/dt = tan(fi)*cos(ac)*sin(eps) / (sqrt(1-r^2)*cos(dek)^3) * ac_speed,
       * the degree conversions cancelling. AscDash() supplies the derivative
       * of Asc1() with respect to its first argument, already scaled by ARMCS,
       * so all it needs is the factor by which that argument moves faster than
       * th itself.
       * If r was clamped, the arc is at an extreme and no longer responds to
       * the Ascendant, so the sda term is dropped. */
      if (hsp->do_hspeed) {
	double dsda = 0, k;
	if (fabs(r) < 1) {
	  double cdek = cosd(dek);
	  dsda = tanfi * cosd(hsp->ac) * sine
	       / (sqrt(1 - r * r) * cdek * cdek * cdek) * hsp->ac_speed;
	}
	k = dsda / (3 * ARMCS);
	hsp->cusp_speed[11] = AscDash(swe_degnorm(th + sd3), 0, sine, cose) * (1 + k);
	hsp->cusp_speed[12] = AscDash(swe_degnorm(th + 2 * sd3), 0, sine, cose) * (1 + 2 * k);
	hsp->cusp_speed[2] = AscDash(swe_degnorm(th + 180 - 2 * sn3), 0, sine, cose) * (1 + 2 * k);
	hsp->cusp_speed[3] = AscDash(swe_degnorm(th + 180 - sn3), 0, sine, cose) * (1 + k);
      }
    }
    break;
  case 'G': 	/* 36 Gauquelin sectors */
    for (i = 1; i <= 36; i++) {
      hsp->cusp[i] = 0;
      hsp->cusp_speed[i] = 0;
    }
    if (fabs(fi) >= 90 - ekl) {  /* within polar circle */
      retc = ERR;
      strcpy(hsp->serr, "within polar circle, switched to Porphyry"); 
      hsy = (int) 'O';
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
	if (hsp->do_hspeed) hsp->cusp_speed[ih] = AscDash(rectasc, atand(ih2 / 9.0 * tanfi), sine, cose);
      } else {
	/* pole height */
	f = atand(sind(asind(tanfi * tant) * ih2 / 9)  /tant);
        hsp->cusp[ih] = Asc1(rectasc, f, sine, cose);
	cuspsv = 0;
        tant_small = 0;
        for (i = 1; i <= niter_max; i++) {
	  tant = tand(asind(sine * sind(hsp->cusp[ih])));
	  if (fabs(tant) < VERY_SMALL) {
	    hsp->cusp[ih] = rectasc;
	    if (hsp->do_hspeed) hsp->cusp_speed[ih] = AscDash(rectasc, atand(ih2 / 9.0 * tanfi), sine, cose);
	    tant_small = 1;
	    break;
	  }
	  /* pole height */
	  f = atand(sind(asind(tanfi * tant) * ih2 / 9) / tant);
  	  hsp->cusp[ih] = Asc1(rectasc, f, sine, cose);
	  if (i > 1 && fabs(swe_difdeg2n(hsp->cusp[ih], cuspsv)) < VERY_SMALL_PLAC_ITER)
	    break;
	  cuspsv = hsp->cusp[ih];
        }
#ifdef DEBUG_PLAC_ITER
  fprintf(stderr, "h=%d, niter=%d\n", ih, i);
#endif
	if (i >= niter_max) {
	  retc = ERR;
	  hsy = (int) 'O';
	  strcpy(hsp->serr, "very close to polar circle, switched to Porphyry"); 
	  goto porphyry;
	}
	if (hsp->do_hspeed && ! tant_small) hsp->cusp_speed[ih] = plac_speed(rectasc, hsp->cusp[ih], f, tanfi, ih2 / 9.0, sine, cose);
      }
      hsp->cusp[ih+18] = swe_degnorm(hsp->cusp[ih] + 180);
      if (hsp->do_hspeed) hsp->cusp_speed[ih + 18] = hsp->cusp_speed[ih];
    }
    /*************** first/third quarter ***************/
    for (ih = 29; ih <= 36; ih++) {
      ih2 = ih - 28;
      fh1 = atand(sind(a * ih2 / 9) / tane);
      rectasc = swe_degnorm(180 - ih2 * 90 / 9 + th);
      tant = tand(asind(sine * sind(Asc1(rectasc, fh1, sine, cose))));
      if (fabs(tant) < VERY_SMALL) {
        hsp->cusp[ih] = rectasc;
	if (hsp->do_hspeed) hsp->cusp_speed[ih] = AscDash(rectasc, atand(ih2 / 9.0 * tanfi), sine, cose);
      } else {
        f = atand(sind(asind(tanfi * tant) * ih2 / 9) / tant);
        /*  pole height */
        hsp->cusp[ih] = Asc1(rectasc, f, sine, cose);
	cuspsv = 0;
        tant_small = 0;
        for (i = 1; i <= niter_max; i++) {
	  tant = tand(asind(sine * sind(hsp->cusp[ih])));
	  if (fabs(tant) < VERY_SMALL) {
	    hsp->cusp[ih] = rectasc;
	    if (hsp->do_hspeed) hsp->cusp_speed[ih] = AscDash(rectasc, atand(ih2 / 9.0 * tanfi), sine, cose);
	    tant_small = 1;
	    break;
	  }
	  f = atand(sind(asind(tanfi * tant) * ih2 / 9) / tant);
	  /*  pole height */
  	  hsp->cusp[ih] = Asc1(rectasc, f, sine, cose);
	  if (i > 1 && fabs(swe_difdeg2n(hsp->cusp[ih], cuspsv)) < VERY_SMALL_PLAC_ITER)
	    break;
	  cuspsv = hsp->cusp[ih];
	}
#ifdef DEBUG_PLAC_ITER
  fprintf(stderr, "h=%d, niter=%d\n", ih, i);
#endif
	if (i >= niter_max) {
	  retc = ERR;
	  hsy = (int) 'O';
	  strcpy(hsp->serr, "very close to polar circle, switched to Porphyry"); 
	  goto porphyry;
	}
	if (hsp->do_hspeed && ! tant_small) hsp->cusp_speed[ih] = plac_speed(rectasc, hsp->cusp[ih], f, tanfi, ih2 / 9.0, sine, cose);
      }
      hsp->cusp[ih-18] = swe_degnorm(hsp->cusp[ih] + 180);
      if (hsp->do_hspeed) hsp->cusp_speed[ih - 18] = hsp->cusp_speed[ih];
    }
    hsp->cusp[1] = hsp->ac;
    hsp->cusp[10] = hsp->mc;
    hsp->cusp[19] = swe_degnorm(hsp->ac + 180);
    hsp->cusp[28] = swe_degnorm(hsp->mc + 180);
    if (hsp->do_hspeed) {
      hsp->cusp_speed[1] = hsp->ac_speed;
      hsp->cusp_speed[10] = hsp->mc_speed;
      hsp->cusp_speed[19] = hsp->ac_speed;
      hsp->cusp_speed[28] = hsp->mc_speed;
    }
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
    /* The cusps come out of a chain of rotations rather than a closed
     * formula, but the chain is still differentiable. Only two things move:
     * armc itself, and the Ascendant, which enters through its right
     * ascension and its declination.
     *   u    = RA(ac) - (th - 90)
     *   kr   = longitude of (u, dec(ac)) rotated by -(90 - fi)
     *   RA_i = longitude of (30i, 0) rotated by 90, shifted by kr, rotated
     *          by (90 - fi), plus (th - 90)
     *   cusp = ecliptic longitude having right ascension RA_i
     * The two rotations are carried by swe_cotrans_sp(), which moves a
     * velocity along with its position, so nothing here is differenced; the
     * ecliptic/equator conversions are differentiated in closed form
     * through dRA/dlon = cos(eps) / (cos^2 lon + cos^2 eps sin^2 lon).
     * The singular direction enters only through ac_speed, which is
     * AscDash()'s and is already correct at the horizon tangency. */
    if (hsp->do_hspeed) {
      double de, dde, u, du, kr, dkr, wac;
      double xa[6];
      xa[0] = hsp->ac; xa[1] = 0.0; xa[2] = 1.0;
      swe_cotrans(xa, xa, -ekl);
      de = xa[1];
      wac = cosd(hsp->ac) * cosd(hsp->ac) + cose * cose * sind(hsp->ac) * sind(hsp->ac);
      dde = cosd(hsp->ac) * sine / cosd(de) * hsp->ac_speed;
      u = swe_degnorm(xa[0] - (th - 90));
      du = cose / wac * hsp->ac_speed - ARMCS;
      {
	/* swe_cotrans_sp() rotates a position and its velocity together, so the
	 * rotation needs no differencing: hand it (u, de) moving at (du, dde) and
	 * read the rotated longitude and its rate straight back out. */
	double xb[6];
	xb[0] = u; xb[1] = de; xb[2] = 1.0;
	xb[3] = du; xb[4] = dde; xb[5] = 0.0;
	swe_cotrans_sp(xb, xb, -(90 - fi));
	kr = xb[0];
	dkr = xb[3];
      }
      for (i = 0; i < 6; i++) {
	double xc[6], dRAi, wc;
	xc[0] = 30.0 * i; xc[1] = 0.0; xc[2] = 1.0;
	xc[3] = 0.0; xc[4] = 0.0; xc[5] = 0.0;
	swe_cotrans_sp(xc, xc, 90);
	xc[0] += kr; xc[3] = dkr;
	swe_cotrans_sp(xc, xc, 90 - fi);
	dRAi = xc[3] + ARMCS;
	wc = cosd(hsp->cusp[i+1]) * cosd(hsp->cusp[i+1])
	   + cose * cose * sind(hsp->cusp[i+1]) * sind(hsp->cusp[i+1]);
	hsp->cusp_speed[i+1] = dRAi * wc / cose;
	hsp->cusp_speed[i+7] = hsp->cusp_speed[i+1];
      }
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
    if (hsp->do_hspeed) {
      for (i = 1; i <= 12; i++)
	hsp->cusp_speed[i] = apc_sector_speed(i, fi * DEGTORAD, ekl * DEGTORAD, th * DEGTORAD);
      /* cusps 4 and 10 are taken from the generic MC above, not from
       * apc_sector(), so their speeds come from the MC as well; cusps 1 and 7
       * are the Ascendant and Descendant and use AscDash(), which is defined
       * at the horizon tangency where apc_sector() itself is not.
       * In the released code that identity failed within 1e-10 degrees of
       * the equator, where apc_sector() replaced the declination of the
       * Ascendant by the constant 90 - VERY_SMALL. The product
       * tan(dasc) * tan(ph) then swept from 0 to about 10 across that band
       * and the cusps it returned were not the Ascendant and Descendant at
       * all: just inside 1e-10 degrees of latitude cusp 1 was 48 degrees from
       * the Ascendant and cusps 1 and 7 were 95 degrees away from opposite.
       * This file repairs that defect in apc_sector() by using sin(kv) in
       * place of the round trip through dasc, which moves positions inside
       * that band only (e.g. lat 5e-11, armc 242: cusp 1 was 303.139, is now
       * the Ascendant 329.906). apc_sector_speed() differentiates the
       * corrected form. Cusps 1 and 7 now agree with the Ascendant and
       * Descendant to about 1e-11 degrees at every latitude (test of
       * 2026-11-13). The band is about a fiftieth of a millimetre wide on the
       * ground. */
      hsp->cusp_speed[10] = hsp->mc_speed;
      hsp->cusp_speed[4] = hsp->mc_speed;
      hsp->cusp_speed[1] = hsp->ac_speed;
      hsp->cusp_speed[7] = hsp->ac_speed;
    }
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
    ih = 11;
    if (fabs(tant) < VERY_SMALL) {
      hsp->cusp[ih] = rectasc;
      if (hsp->do_hspeed) hsp->cusp_speed[ih] = AscDash(rectasc, atand(1.0 / 3 * tanfi), sine, cose);
    } else {
      /* pole height */
      f = atand(sind(asind(tanfi * tant) / 3)  /tant);  
      hsp->cusp[ih] = Asc1(rectasc, f, sine, cose);
      cuspsv = 0;
      tant_small = 0;
      for (i = 1; i <= niter_max; i++) {
	tant = tand(asind(sine * sind(hsp->cusp[ih])));
	if (fabs(tant) < VERY_SMALL) {
	  hsp->cusp[ih] = rectasc;
	  if (hsp->do_hspeed) hsp->cusp_speed[ih] = AscDash(rectasc, atand(1.0 / 3 * tanfi), sine, cose);
	  tant_small = 1;
	  break;
	}
	/* pole height */
	f = atand(sind(asind(tanfi * tant) / 3) / tant);
	hsp->cusp[ih] = Asc1(rectasc, f, sine, cose);
	if (i > 1 && fabs(swe_difdeg2n(hsp->cusp[ih], cuspsv)) < VERY_SMALL_PLAC_ITER)
	  break;
	cuspsv = hsp->cusp[ih];
      }
      if (i >= niter_max) {
	retc = ERR;
	strcpy(hsp->serr, "very close to polar circle, switched to Porphyry"); 
	goto porphyry;
      }
      if (hsp->do_hspeed && ! tant_small) hsp->cusp_speed[ih] = plac_speed(rectasc, hsp->cusp[ih], f, tanfi, 1.0 / 3, sine, cose);
#ifdef DEBUG_PLAC_ITER
  fprintf(stderr, "h=%d, niter=%d\n", ih, i);
#endif
    }
    /* ************  house 12 ******************** */
    rectasc = swe_degnorm(60 + th);
    tant = tand(asind(sine*sind(Asc1(rectasc,  fh2, sine, cose))));
    ih = 12;
    if (fabs(tant) < VERY_SMALL) {
      hsp->cusp[ih] = rectasc;
      if (hsp->do_hspeed) hsp->cusp_speed[ih] = AscDash(rectasc, atand(2.0 / 3 * tanfi), sine, cose);
    } else {
      f = atand(sind(asind(tanfi * tant) / 1.5) / tant);  
      /*  pole height */
      hsp->cusp[ih] = Asc1(rectasc, f, sine, cose);
      cuspsv = 0;
      tant_small = 0;
      for (i = 1; i <= niter_max; i++) {
	tant = tand(asind(sine * sind(hsp->cusp[ih])));
	if (fabs(tant) < VERY_SMALL) {
	  hsp->cusp[ih] = rectasc;
	  if (hsp->do_hspeed) hsp->cusp_speed[ih] = AscDash(rectasc, atand(2.0 / 3 * tanfi), sine, cose);
	  tant_small = 1;
	  break;
	}
	f = atand(sind(asind(tanfi * tant) / 1.5) / tant);  
	/*  pole height */
	hsp->cusp[ih] = Asc1(rectasc, f, sine, cose);
	if (i > 1 && fabs(swe_difdeg2n(hsp->cusp[ih], cuspsv)) < VERY_SMALL_PLAC_ITER)
	  break;
	cuspsv = hsp->cusp[ih];
      }
      if (i >= niter_max) {
	retc = ERR;
	strcpy(hsp->serr, "very close to polar circle, switched to Porphyry"); 
	goto porphyry;
      }
      if (hsp->do_hspeed && ! tant_small) hsp->cusp_speed[ih] = plac_speed(rectasc, hsp->cusp[ih], f, tanfi, 2.0 / 3, sine, cose);
#ifdef DEBUG_PLAC_ITER
  fprintf(stderr, "h=%d, niter=%d\n", ih, i);
#endif
    }
    /* ************  house  2 ******************** */
    rectasc = swe_degnorm(120 + th);
    tant = tand(asind(sine * sind(Asc1(rectasc, fh2, sine, cose))));
    ih = 2;
    if (fabs(tant) < VERY_SMALL) {
      hsp->cusp[ih] = rectasc;
      if (hsp->do_hspeed) hsp->cusp_speed[ih] = AscDash(rectasc, atand(2.0 / 3 * tanfi), sine, cose);
    } else {
      f = atand(sind(asind(tanfi * tant) / 1.5) / tant);
      /*  pole height */
      hsp->cusp[ih] = Asc1(rectasc, f, sine, cose);
      cuspsv = 0;
      tant_small = 0;
      for (i = 1; i <= niter_max; i++) {
	tant = tand(asind(sine * sind(hsp->cusp[ih])));
	if (fabs(tant) < VERY_SMALL) {
	  hsp->cusp[ih] = rectasc;
	  if (hsp->do_hspeed) hsp->cusp_speed[ih] = AscDash(rectasc, atand(2.0 / 3 * tanfi), sine, cose);
	  tant_small = 1;
	  break;
	}
	f = atand(sind(asind(tanfi * tant) / 1.5) / tant);
	/*  pole height */
	hsp->cusp[ih] = Asc1(rectasc, f, sine, cose);
	if (i > 1 && fabs(swe_difdeg2n(hsp->cusp[ih], cuspsv)) < VERY_SMALL_PLAC_ITER)
	  break;
	cuspsv = hsp->cusp[ih];
      }
      if (i >= niter_max) {
	retc = ERR;
	strcpy(hsp->serr, "very close to polar circle, switched to Porphyry"); 
	goto porphyry;
      }
      if (hsp->do_hspeed && ! tant_small) hsp->cusp_speed[ih] = plac_speed(rectasc, hsp->cusp[ih], f, tanfi, 2.0 / 3, sine, cose);
#ifdef DEBUG_PLAC_ITER
  fprintf(stderr, "h=%d, niter=%d\n", ih, i);
#endif
    }
    /* ************  house  3 ******************** */
    rectasc = swe_degnorm(150 + th);
    tant = tand(asind(sine * sind(Asc1(rectasc, fh1, sine, cose))));
    ih = 3;
    if (fabs(tant) < VERY_SMALL) {
      hsp->cusp[ih] = rectasc;
      if (hsp->do_hspeed) hsp->cusp_speed[ih] = AscDash(rectasc, atand(1.0 / 3 * tanfi), sine, cose);
    } else {
      f = atand(sind(asind(tanfi * tant) / 3) / tant);  
      /*  pole height */
      hsp->cusp[ih] = Asc1(rectasc, f, sine, cose);
      cuspsv = 0;
      tant_small = 0;
      for (i = 1; i <= niter_max; i++) {
	tant = tand(asind(sine * sind(hsp->cusp[ih])));
	if (fabs(tant) < VERY_SMALL) {
	  hsp->cusp[ih] = rectasc;
	  if (hsp->do_hspeed) hsp->cusp_speed[ih] = AscDash(rectasc, atand(1.0 / 3 * tanfi), sine, cose);
	  tant_small = 1;
	  break;
	}
	f = atand(sind(asind(tanfi * tant) / 3) / tant);
	/*  pole height */
	hsp->cusp[ih] = Asc1(rectasc, f, sine, cose);
	if (i > 1 && fabs(swe_difdeg2n(hsp->cusp[ih], cuspsv)) < VERY_SMALL_PLAC_ITER)
	  break;
	cuspsv = hsp->cusp[ih];
      }
      if (i >= niter_max) {
	retc = ERR;
	strcpy(hsp->serr, "very close to polar circle, switched to Porphyry"); 
	goto porphyry;
      }
      if (hsp->do_hspeed && ! tant_small) hsp->cusp_speed[ih] = plac_speed(rectasc, hsp->cusp[ih], f, tanfi, 1.0 / 3, sine, cose);
#ifdef DEBUG_PLAC_ITER
  fprintf(stderr, "h=%d, niter=%d\n", ih, i);
#endif
    }
    break;
  } /* end switch */
  if (hsy != 'G' && hsy != 'Y' && toupper(hsy) != 'I' ) {
    hsp->cusp[4] = swe_degnorm(hsp->cusp[10] + 180);
    hsp->cusp[5] = swe_degnorm(hsp->cusp[11] + 180);
    hsp->cusp[6] = swe_degnorm(hsp->cusp[12] + 180);
    hsp->cusp[7] = swe_degnorm(hsp->cusp[1] + 180);
    hsp->cusp[8] = swe_degnorm(hsp->cusp[2] + 180);
    hsp->cusp[9] = swe_degnorm(hsp->cusp[3] + 180);
    if (hsp->do_hspeed && ! hsp->do_interpol) {
      hsp->cusp_speed[4] = hsp->cusp_speed[10];
      hsp->cusp_speed[5] = hsp->cusp_speed[11];
      hsp->cusp_speed[6] = hsp->cusp_speed[12];
      hsp->cusp_speed[7] = hsp->cusp_speed[1];
      hsp->cusp_speed[8] = hsp->cusp_speed[2];
      hsp->cusp_speed[9] = hsp->cusp_speed[3];
    }
  }
  /* vertex */
  if (fi >= 0)
    f = 90 - fi;
  else
    f = -90 - fi;
  hsp->vertex = Asc1(th - 90, f, sine, cose);
  if (hsp->do_speed) hsp->vertex_speed = AscDash(th - 90, f, sine, cose);
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
  if (hsp->do_speed) hsp->equasc_speed = AscDash(th + 90, 0, sine, cose); 
  /* "co-ascendant" W. Koch */
  hsp->coasc1 = swe_degnorm(Asc1(th - 90, fi, sine, cose) + 180);
  if (hsp->do_speed) hsp->coasc1_speed = AscDash(th - 90, fi, sine, cose);
  /* "co-ascendant" M. Munkasey */
  if (fi >= 0) {
    hsp->coasc2 = Asc1(th + 90, 90 - fi, sine, cose);
    if (hsp->do_speed) hsp->coasc2_speed = AscDash(th + 90, 90 - fi, sine, cose);
  } else { /* southern hemisphere */
    hsp->coasc2 = Asc1(th + 90, -90 - fi, sine, cose);
    if (hsp->do_speed) hsp->coasc2_speed = AscDash(th + 90, -90 - fi, sine, cose);
  }
  /* "polar ascendant" M. Munkasey */
  hsp->polasc = Asc1(th - 90, fi, sine, cose);
  if (hsp->do_speed) hsp->polasc_speed = AscDash(th - 90, fi, sine, cose);
  return retc;
} /* procedure houses */

/*****
 * oblique triangle formed by: great circle with pole height f, ecliptic and equator,
 * x = intersection equator - great circle, measured along equator.
 * return crossing of ecliptic with great circle.
 * Prepare quadrants before doing the work in Asc2.
 */
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


/*
 * x in range 0..90
 * f in range -90 .. +90
 * sine, cose around e=23°
 * oblique triangle formed by: great circle with pole height f, ecliptic and equator,
 * In case of ascendant, the great circle is the horizon, which has pole height latitude.
 * x = intersection equator - great circle, measured along equator.
 * return crossing of ecliptic with great circle.
 */
static double Asc2(double x, double f, double sine, double cose) 
{
  double ass, sinx;
  // from https://en.wikipedia.org/wiki/Spherical_trigonometry CT5
  // cot c sin a = cot C sin B + cos a cos B, with B = ecl, a = x, C = 90 +f
  // cot 90 + f = - tan f
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
    // resolve ass = sin x cot c; cot c = ass / sini x; tan c = sin x / ass
    ass = atand(sinx / ass);
  }
  if (ass < 0)
    ass = 180 + ass;
  return (ass);
} /* Asc2 */

/* Pole height used by Placidus and by the Gauquelin sectors: the cusp's own
 * declination sets it, so it moves as the cusp moves. fac is 1/3 or 2/3 for
 * Placidus and ih2/9 for the Gauquelin sectors. */
/* The cusp sitting on an equinox is the degenerate case: the declination and
 * with it tant go to zero, and the pole height becomes 0/0. Its limit is
 * finite and independent of the cusp, atand(fac * tanfi), which is what the
 * callers use for the speed in that branch; here it is reported through *ok
 * so that plac_speed() skips the correction rather than differencing across it.
 * *ok is a separate flag rather than a magic return value because 0 is also a
 * perfectly good pole height: on the equator tanfi is 0 and every cusp has
 * pole 0, which is not a degeneracy but the correct answer. */
static double plac_pole(double cusp, double tanfi, double fac, double sine, int *ok)
{
  double tant = tand(asind(sine * sind(cusp)));
  double v;
  *ok = 0;
  if (fabs(tant) < VERY_SMALL)
    return 0;
  v = tanfi * tant;
  if (v > 1 || v < -1)
    return 0;
  *ok = 1;
  return atand(sind(asind(v) * fac) / tant);
}

/* Partial derivative of Asc1() with respect to the pole height, the companion
 * to AscDash(), which differentiates the first argument instead. Asc1() is
 *   Asc = atan(sin x / c),   c = cos(e) cos x - tan f sin(e),
 * mapped into the right quadrant, and the mapping leaves this derivative
 * unchanged in all four of them, so one expression covers the circle.
 * Both variables are in degrees and the conversion factors cancel. */
static double AscDashPole(double x, double f, double sine, double cose)
{
  double sinx = sind(x), cosf = cosd(f);
  double c = cose * cosd(x) - tand(f) * sine;
  double d = sinx * sinx + c * c;
  /* The two tests are not equally reachable. cos(f) cannot get small: f comes
   * from atand() and would need |tant| below any usable tolerance, which
   * plac_pole() already refuses. d can, in principle: it vanishes when x is
   * near 0 and
   * tan(f) approaches cot(eps), that is a pole height near 90 - eps, which is
   * a real Placidus configuration near the polar circle. It did not occur in
   * 1.4e7 sampled calls, but it is not impossible the way the other one is. */
  if (d < VERY_SMALL_DENOM || fabs(cosf) < VERY_SMALL_DENOM)
    return 0;
  return sinx * sine / (cosf * cosf * d);
}

/* Derivative of plac_pole() with respect to the cusp, following the same chain
 *   dek  = asin(sin(e) sin(cusp)),   T = tan(dek),
 *   pole = atan(sin(fac * asin(tanfi T)) / T).
 * *ok clears on the degeneracies plac_pole() itself reports, and additionally
 * in a thin band where |tanfi T| approaches 1 and this derivative runs away
 * although plac_pole() itself is still fine. The caller has already refused
 * |tanfi T| > 1 and switched to Porphyry, so the band is only reachable within
 * about 1e-5 of the polar circle; plac_speed() falls back to AscDash() there,
 * as it did before. */
static double plac_pole_slope(double cusp, double tanfi, double fac, double sine, int *ok)
{
  double s = sine * sind(cusp), rs = 1 - s * s;
  double dek, cdek, tt, ddek, dtt, v, rv, dv, a, da, n, dn, q;
  *ok = 0;
  if (rs < VERY_SMALL_DENOM)
    return 0;
  dek = asind(s);
  cdek = cosd(dek);
  tt = tand(dek);
  if (fabs(tt) < VERY_SMALL_DENOM)
    return 0;
  ddek = sine * cosd(cusp) / sqrt(rs);
  dtt = DEGTORAD * ddek / (cdek * cdek);
  v = tanfi * tt;
  rv = 1 - v * v;
  if (rv < VERY_SMALL_DENOM)
    return 0;
  dv = tanfi * dtt;
  a = asind(v) * fac;
  da = fac * RADTODEG / sqrt(rv) * dv;
  n = sind(a);
  dn = DEGTORAD * cosd(a) * da;
  q = n / tt;
  *ok = 1;
  return RADTODEG / (1 + q * q) * (dn * tt - n * dtt) / (tt * tt);
}

/* True derivative of a cusp that is only defined implicitly, by
 *   cusp = Asc1(rectasc, f(cusp)),
 * as in Placidus and the Gauquelin sectors, where the iteration above has
 * already converged on the cusp. Placidus trisects the diurnal and nocturnal
 * semi-arcs of each ecliptic degree, so every cusp carries its own pole
 * height, and that pole height is what makes the derivative implicit. Beyond
 * the polar circles the trisection has nothing to divide and the system is
 * undefined; falling back to Porphyry there is what the caller already does
 * and is standard practice across programs, so no speed is produced for those
 * latitudes by this function. AscDash() is the partial derivative with
 * respect to rectasc alone, but the pole height follows the cusp, so
 *   dcusp/dth = A_x / (1 - A_f * df/dcusp).
 * Both partials are closed form, AscDashPole() and plac_pole_slope(). If
 * either leaves its domain the correction is skipped rather than guessed. */
static double plac_speed(double rectasc, double cusp, double f, double tanfi,
			 double fac, double sine, double cose)
{
  double dA_df, df_dc, denom, ad, fc;
  int k, ok;
  /* The caller's cusp comes out of a loop that stops at VERY_SMALL_PLAC_ITER,
   * a hundredth of an arcsecond, and its f belongs to the iterate before that.
   * Both are far more accurate than the cusp needs to be, but they set a floor
   * on the derivative, which depends on the pole height through the correction
   * below, so the fixed point is refined here before differentiating.
   *
   * The loop runs to a tolerance rather than a fixed number of steps, and that
   * matters. A version that took exactly one step was tried, because at
   * ordinary latitudes one is enough: below 60 degrees it agreed with a fully
   * converged reference to 4.7e-11 relative and ran Placidus at 1.33 times the
   * unpatched cost instead of 1.60. It is wrong near the polar circle. The
   * map's contraction factor approaches 1 as |tanfi tant| approaches 1, so one
   * step barely moves the iterate and the implicit function correction is then
   * evaluated at a pair that is not a fixed point. At latitude 66.5, armc
   * 268.7, Placidus cusp 2 came out 743.729633 against a true 743.729661, and
   * Gauquelin sector 2 at 66.5606 was out by 3.3 arcseconds a day. The onset is
   * around latitude 65, which is inhabited. Fixed step counts are therefore
   * not safe here whatever the count.
   *
   * 1e-9 degrees rather than the 1e-12 tried first: at 1e-9 the returned speed
   * already agrees with a reference built at VERY_SMALL_PLAC_ITER = 1e-14 and
   * niter_max = 2000 to 7.5e-11 relative at the worst point near the polar
   * circle, and tightening to 1e-11 moves it by 3e-8 degrees per day, which
   * nothing downstream can use. The composed map cannot settle below about
   * 1e-13 degrees on cusps near 100 degrees anyway, so a target below that only
   * makes the loop exit by exhaustion instead of by convergence. */
  for (k = 0; k < 20; k++) {
    fc = plac_pole(cusp, tanfi, fac, sine, &ok);
    if (! ok)
      break;
    f = fc;
    fc = Asc1(rectasc, f, sine, cose);
    if (fabs(swe_difdeg2n(fc, cusp)) < 1e-9)
      break;
    cusp = fc;
  }
  ad = AscDash(rectasc, f, sine, cose);
  /* f comes out of atand(), so it is always strictly inside +-90 and this
   * never fires. It is kept because the expression below divides by cos(f)
   * through Asc1(), and a future change to how f is formed should not be able
   * to reach that division silently. */
  if (fabs(f) > 90 - VERY_SMALL)
    return ad;
  dA_df = AscDashPole(rectasc, f, sine, cose);
  df_dc = plac_pole_slope(cusp, tanfi, fac, sine, &ok);
  if (! ok)			/* degenerate, see plac_pole() */
    return ad;
  denom = 1 - dA_df * df_dc;
  if (fabs(denom) < VERY_SMALL_DENOM)
    return ad;
  return ad / denom;
}

/* Pullen SR sector widths for a quadrant of size q, factored out of case 'Q'
 * so that the same code can be evaluated at neighbouring q when the cusp
 * speeds are wanted. */
static void pullen_sr_sectors(double q, double *x, double *xr, double *xr3, double *xr4, double *rp)
{
  double c, csq, ccr, cqx, r, r1, r2;
  double third = 1.0 / 3.0;
  double two23 = pow(2.0 * 2.0, third);        // 2^(2/3)
  if (q < 1e-30) {    // degenerate case of quadrant = zer0
    *x = *xr = *xr3 = 0;
    *xr4 = 180;
    if (rp != NULL) *rp = 0;
    return;
  }
  c = (180 - q) / q;
  csq = c * c;
  ccr = pow(csq - c, third);          // cuberoot(c^2 -c)
  cqx = sqrt(two23 * ccr + 1.0);      // sqrt{2^(2/3)*cuberoot(c^2-c) + 1}
  r1 = 0.5 * cqx;
  r2 = 0.5 * sqrt(-2*(1-2*c) / cqx - two23 * ccr + 2);
  r = r1 + r2 - 0.5;
  *x = q / (2 * r + 1);
  *xr = r * *x;
  *xr3 = *xr * r * r;
  *xr4 = *xr3 * r;
  if (rp != NULL) *rp = r;
}

/* Sector widths differentiated with respect to the quadrant size q.
 * r is a root of a quartic and the radicals above are unpleasant to
 * differentiate, but r never has to be differentiated in that form. Pullen
 * (https://www.astrolog.org/astrolog/astsine.htm) defines the system by two
 * equations, rx + x + rx = q and
 * xr^3 + xr^4 + xr^3 = 180 - q, so the two quadrants add up:
 *   q = x (1 + 2r),   180 - q = x r^3 (2 + r),
 * and eliminating x leaves
 *   F(r, q) = q r^3 (2 + r) - (180 - q)(1 + 2r) = 0.
 * Implicit differentiation of F gives dr/dq, and the widths follow by the
 * product rule. This is exact, and it is defined at q = 90, where a difference
 * taken in q would have to turn one-sided. r running away as q goes to zero is
 * the intended behaviour, not a numerical accident: Pullen notes that in the
 * degenerate case Asc = MC, x goes to 0 and r goes to infinity, which is what
 * the q < 1e-4 guard in case 'Q' stands in for. */
static void pullen_sr_slopes(double q, double r, double x, double xr, double xr3,
			     double *dx, double *dxr, double *dxr3, double *dxr4)
{
  double dFdr = q * (4 * r * r * r + 6 * r * r) - 2 * (180 - q);
  double dFdq = r * r * r * (2 + r) + (1 + 2 * r);
  double dr, den = 2 * r + 1;
  /* dFdr cannot vanish on the constraint surface. Setting it to zero gives
   * q r^2 (3 + 2r) = 180 - q, and substituting the two defining equations
   * leaves (1 + 2r)(3 + 2r) = r(2 + r), that is (r + 1)^2 = 0. r is positive
   * throughout, so this guard is defensive only. */
  if (fabs(dFdr) < VERY_SMALL_DENOM) {
    *dx = *dxr = *dxr3 = *dxr4 = 0;
    return;
  }
  dr = -dFdq / dFdr;
  *dx = 1 / den - q * 2 * dr / (den * den);
  *dxr = dr * x + r * *dx;
  *dxr3 = *dxr * r * r + xr * 2 * r * dr;
  *dxr4 = *dxr3 * r + xr3 * dr;
}

// derivative of Asc1, computes speed
// code contributed by Graham Dawson
static double AscDash(double x, double f, double sine, double cose)
{
  double cosx = cosd(x);
  double sinx = sind(x);
  double sinx2 = sinx * sinx;
  double c = cose * cosx - tand(f) * sine;
  double d = sinx2 + c * c;
  double dudt;
  if (d > VERY_SMALL) {
      dudt = (cosx * c + cose * sinx2) / d;
  } else {
      dudt = 0.0; //  When we are on axis of ecliptic
  }
  return dudt * ARMCS;	// 360.985647366;
}

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
  double fh, ra0, tanfi, sinfi, fac, dfac, tanx;
  double x[3], xasc[3], xs1, xs2, raep, raaz, oblaz, xtemp; /* BK 21.02.2006 */
  double hcusp[37], ascmc[10];
  double sine = sind(eps);
  double cose = cosd(eps);
  double c1, c2, d, hsize;
  int i, j, nloop;
  double dsun = 0, darmc, harmc, y, sinpsi, sa;
  AS_BOOL is_western_half = FALSE;
  hsys = toupper(hsys);
  if (1) {
    /* input is a house cusp: no calculation is required */
    ascmc[9] = 99;// dirty hack. Sunshine house system needs sun declination
		  // which we do not know. If it sees ascmc[9] == 99, it uses
		  // the one is saved from last call. can lead to bugs, but can 
		  // also solve many problems.
    if (swe_houses_armc_ex2(armc, geolat, eps, hsys, hcusp, ascmc, NULL, NULL, serr) == ERR) {
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
      admc = tand(eps) * tand(geolat) * sind(armc);
      /* midheaven is circumpolar */
      if (fabs(admc) > 1) {
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
      // we measure on equator from east point towards IC.
      // transformation to prime vertical, with these coordinate references
      // EP = 0, nadir = 90, WP = 180, Zenith = 270
      swe_cotrans(xeq, xp, -geolat);
      /* to make sure that a call with a house cusp position returns
       * a value within the house, 0.001" is added */
      xp[0] = swe_degnorm(xp[0] + MILLIARCSEC);
      hpos = xp[0] / 30.0 + 1;
      break;
    case 'J': // Savard-A
      sinfi = sind(geolat);
      if (fabs(geolat) < VERY_SMALL) {	
	xs2 = 1 / 3.0;
	xs1 = 2 / 3.0;
      } else {
	xs2 = sind(geolat / 3) / sinfi;	
	xs1 = sind(2 * geolat / 3) / sinfi;
      }
      xs2 = asind(xs2);
      xs1 = asind(xs1);
      // xs1 and xs2 always in >= 0 < 90
      // house borders on prime vertical are, measured from EP downwards
      // h1 = 0, h4 = 90, h7 = 180, h10 = 270
      // h2 = xs2, h3 = xs1, h12 = 360 - xs2, h11 = 360 - xs1
      // h5 = h11 - 180, h6 = h12 - 180, h8 = h2 + 180, h9 = h3 + 180
      hcusp[1] = 0;
      hcusp[2] = xs2;
      hcusp[3] = xs1;
      hcusp[4] = 90;
      hcusp[5] = 180 - xs1;
      hcusp[6] = 180 - xs2;
      hcusp[7] = 180;
      hcusp[8] = 180 + xs2;
      hcusp[9] = 180 + xs1;
      hcusp[10] = 270;
      hcusp[11] = 360 - xs1;
      hcusp[12] = 360 - xs2;
      xeq[0] = swe_degnorm(mdd - 90);
      swe_cotrans(xeq, xp, -geolat);
      a = xp[0];
      if (swe_difdeg2n(hcusp[6], hcusp[1]) > 0) {
	d = swe_degnorm(a - hcusp[1]);
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
	d = swe_degnorm(hcusp[1] - a);
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
    if (swe_houses_armc_ex2(armc, geolat, eps, hsys, hcusp, ascmc, NULL, NULL, serr) == ERR) {
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

static int sunshine_init(double lat, double dec, double xh[])
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

/* The last stage of the Makransky sunshine solution: the cusp as a function of
 * w, the house circle's position measured from the meridian, with the pole
 * height held fixed. Factored out of sunshine_solution_makransky(), so that it
 * can also be evaluated at neighbouring w when the speeds are wanted. Against
 * the released code it has three fixes, each commented below: southern charts
 * at exactly w == 90 or 270, w == 180 exactly (w >= 180 instead of w > 180),
 * and the result normalised to 0 .. 360 (no cusp of exactly 360). */
static double makransky_cusp_from_w(double w, double pole, double ecl, int ih, double lat)
{
  double r = 0, cu, z = 0, sinecl = sind(ecl);
  if (w == 90 || w == 270) {
    /* Which meridian side w names depends on the hemisphere. Southern charts
     * have already had rah turned by 180, so the side this frame calls w == 90
     * is the one a northern chart calls w == 270. The general branch below gets
     * that right by way of its own 180 shift; these two had their own formulae
     * and no shift, so a southern chart landing exactly on 90 or 270 came back
     * on the wrong side of the meridian. */
    int at90 = (w == 90);
    if (lat < 0) at90 = !at90;
    r = atand(sinecl * tand(pole));
    if (at90) {
      if (ih <= 3 || ih >= 11) cu = 90 + r; else cu = 90 - r;
    } else {
      if (ih <= 3 || ih >= 11) cu = 270 - r; else cu = 270 + r;
    }
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
      else if (w >= 180 && w < 270)
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
      else if (w >= 180 && w < 270)
	cu = 360 - r;
      else
	cu = 180 + r;
    }
    if (lat < 0)	// Makransky deals with southern latitude this way
      cu = swe_degnorm(cu + 180);
  }
  /* the z > 90 chain returns 360 - r, and r is 0 at w == 180, so without this
   * a northern chart can be handed a cusp of exactly 360 */
  return swe_degnorm(cu);
}

/* Derivative of makransky_cusp_from_w() with respect to w, in degrees per
 * degree. The cusp is a chain of atand() calls,
 *   m = atan|tan(pole)/cos(w)|,  z = m -+ ecl,
 *   r = atan|cos(m) tan(w)/cos(z)|,  cusp = -+r plus a multiple of 180,
 * and the quadrant of w and the side of z = 90 fix only the sign in front of
 * r, so one expression with a sign covers all of them.
 *
 * Two quantities in that chain must not be evaluated the way the position
 * code writes them. Near w = 90 and 270, m comes within a fraction of a
 * degree of 90, and cos(m) formed from m loses almost all of its digits:
 * m carries about 1e-14 degrees of absolute error, which is a relative error
 * of 1e-10 in 90 - m. The product cos(m) tan(w) is then a small number times
 * a large one. Both have exact closed forms. With
 *   S = sqrt(cos^2 w + tan^2 pole)
 * the definition of m gives cos(m) = |cos w| / S and sin(m) = |tan pole| / S
 * directly, so that
 *   cos(m) tan(w) = sgn(cos w) sin(w) / S,
 * with no cancellation and no 0 x infinity, and cos(z) follows from cos(m)
 * and sin(m) by the addition formula without forming m at all. A version
 * written in terms of m instead was tried first and was wrong by 3e-5 at
 * w within 2e-4 of 90; this one is right there and needs no step size, so
 * the differences and the one-sided handling near the bounds are both gone. */
static double makransky_slope_w(double w, double pole, double ecl, int ih, double lat)
{
  double cw = cosd(w), sw = sind(w), tp = tand(pole);
  double ce = cosd(ecl), se = sind(ecl);
  double S, cm, sm, cz, sz, dS, sdcz, den, sig, sgn;
  double sgncw, sgnsw, sgncz;
  /* lat enters makransky_cusp_from_w() only through the constant 180 degree
   * shift it applies at southern latitudes, which the derivative does not
   * see. The parameter is kept so that the two functions take the same
   * arguments and stay easy to compare. */
  (void) lat;
  S = sqrt(cw * cw + tp * tp);
  if (S == 0)		/* w on 90 or 270 and the pole on the equator */
    return 0;
  sgncw = (cw < 0) ? -1 : 1;
  sgnsw = (w > 180) ? -1 : 1;	/* not from sw; see the note at sgn below */
  cm = fabs(cw) / S;	/* cos(m) and sin(m) from the definition of m, */
  sm = fabs(tp) / S;	/* never from m itself */
  if (ih <= 3 || ih >= 11)
    sig = (w > 90 && w < 270) ? -1 : 1;
  else
    sig = (w > 90 && w < 270) ?  1 : -1;
  cz = cm * ce - sig * sm * se;		/* cos(z), sin(z), z = m + sig * ecl */
  sz = sm * ce + sig * cm * se;
  sgncz = (cz < 0) ? -1 : 1;
  dS   = -sgncw * cm * sw * DEGTORAD;
  sdcz = -sgncw * sw * DEGTORAD * sm * sz;	/* S times d(cos z)/dw */
  /* Writing D = S |cos z|, the slope is
   *   [ sgn(sin w) cos(w) D - |sin w| (dS |cos z| + sgn(cos z) S dcz) ]
   *     divided by  D^2 + sin^2 w,
   * which never needs S, cos(z) or sin(w) in a denominator of its own. Each
   * of those does reach zero at ordinary configurations, cos(z) whenever m
   * reaches 90 - ecl and S when the pole meets the equator with w on 90, and
   * the slope stays finite at all of them; guarding them separately reported
   * a moving cusp as standing still. Only the sum below can vanish, and only
   * where the cusp itself is undefined. */
  den = S * S * cz * cz + sw * sw;
  if (den < VERY_SMALL_DENOM)
    return 0;
  /* The quadrant of w and the side of z = 90 fix only the sign in front of r;
   * sgn * sgn(cos z) is the same either side of z = 90, so this is continuous
   * across it.
   *
   * Every bound here is <=, so that w landing exactly on 90, 180 or 270 takes
   * the branch below it rather than the one above. Those three values are
   * reachable and not rare: w is the house circle's position measured from the
   * meridian, and on the equator the ascensional difference vanishes, so the
   * diurnal and nocturnal arcs are exactly 90 degrees and the house circles sit
   * at exact multiples of 30. Every armc that is a multiple of 30 then lands
   * three of the twelve cusps on one of them. The slope is continuous through
   * all three: evaluated a millionth of a degree either side it agrees to the
   * digits printed, so the boundary value is simply the neighbouring one and
   * there is no limit to choose. With a strict < it came out negated instead.
   *
   * sgnsw is taken from the quadrant rather than from sw for the same reason.
   * sin(180 degrees) is not zero in double precision but about 1.2e-16, and
   * positive, so a test on its sign silently uses the convention for w below
   * 180 while sgn uses the one above, and the two disagree by a factor of -1
   * at exactly that point. Reading the sign off w instead settles it without
   * depending on which way a particular libm rounds sin near pi. Everywhere
   * else the two agree, sw being of the sign the quadrant implies. */
  if (cz >= 0)
    sgn = (w <= 90) ? 1 : (w <= 180) ? -1 : (w <= 270) ? 1 : -1;
  else
    sgn = (w <= 90) ? -1 : (w <= 180) ? 1 : (w <= 270) ? -1 : 1;
  return sgn * RADTODEG
       * (sgnsw * cw * DEGTORAD * S * fabs(cz)
          - fabs(sw) * (dS * fabs(cz) + sgncz * sdcz)) / den;
}


static int sunshine_solution_makransky(double ramc, double lat, double ecl, struct houses *hsp)
{
  double xh[13];
  double md;
  double zd;	// zenith distance of house circle, along prime vertical
  double pole, q, w, a, b, c, f, cu, rah;
  double sinlat, coslat, tanlat, tandec;
  double dec = hsp->sundec;
  sinlat = sind(lat);
  coslat = cosd(lat);
  tanlat = tand(lat);
  tandec = tand(dec);
  int ih;
  // if (90 - fabs(lat) <= ecl) {
  //   strcpy(hsp->serr, "Sunshine in polar circle not allowed");
  //   return ERR;
  // }
  if (sunshine_init(lat, dec, xh) == ERR)
    return ERR;
  for (ih = 1; ih <= 12; ih++) {
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
    cu = makransky_cusp_from_w(w, pole, ecl, ih, lat);
    hsp->cusp[ih] = cu;
    /* Of everything above, only rah and therefore w depend on ramc, and they
     * follow it one for one: xh[], md, zd, pole and q are set by the latitude
     * and the Sun's declination alone. So the cusp is a function of w only,
     * d(cusp)/d(ramc) = d(cusp)/dw, and makransky_slope_w() gives that in
     * closed form. The Sun's declination is held fixed; see case 'I'. */
    if (hsp->do_hspeed)
      hsp->cusp_speed[ih] = makransky_slope_w(w, pole, ecl, ih, lat) * ARMCS;
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
    /* pole and rax depend on the latitude and the solar declination only, so
     * as armc advances, a advances with it at the same rate and the pole
     * height stays put. AscDash() is therefore the exact derivative.
     * The declination is held fixed here, which is the same convention the
     * interpolation this replaces already used: it passed the centre value of
     * sundec to both of its offset samples. */
    if (hsp->do_hspeed)
      hsp->cusp_speed[ih] = AscDash(a, pole, sinecl, cosecl);
  }
  if (mc_under_horizon && ! SUNSHINE_KEEP_MC_SOUTH) {
    for (ih = 2; ih <= 12; ih++) {
      if ((ih - 1) % 3 == 0) continue;	// skip 1,4,7,10
      hsp->cusp[ih] = swe_degnorm(hsp->cusp[ih] + 180);
    }
  }
  return retval;
}
