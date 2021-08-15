
/************************************************************

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

#define PREC_IAU_1976_CTIES          2.0 	/* J2000 +/- two centuries */
#define PREC_IAU_2000_CTIES          2.0 	/* J2000 +/- two centuries */
/* we use P03 for whole ephemeris */
#define PREC_IAU_2006_CTIES          75.0 	/* J2000 +/- 75 centuries */

/* For reproducing JPL Horizons to 2 mas (SEFLG_JPLHOR): 
 * The user has to keep the following files up to date which contain
 * the earth orientation parameters related to the IAU 1980 nutation
 * theory. 
 * Download the file 
 * datacenter.iers.org/eop/-/somos/5Rgv/document/tx13iers.u24/eopc04_08.62-now
 * and rename it as eop_1962_today.txt. For current data and estimations for
 * the near future, also download maia.usno.navy.mil/ser7/finals.all and 
 * rename it as eop_finals.txt */
#define DPSI_DEPS_IAU1980_FILE_EOPC04   "eop_1962_today.txt"
#define DPSI_DEPS_IAU1980_FILE_FINALS   "eop_finals.txt"
#define DPSI_DEPS_IAU1980_TJD0_HORIZONS  2437684.5 
#define HORIZONS_TJD0_DPSI_DEPS_IAU1980  2437684.5 
#define DPSI_IAU1980_TJD0	(64.284 / 1000.0)  // arcsec
#define DEPS_IAU1980_TJD0	(6.151 / 1000.0)   // arcsec

/* The above files must be available in order to reproduce JPL Horizons 
 * in agreement with IERS Conventions 1996 (1992), p. 22. 
 * Call swe_calc_ut() with iflag|SEFLG_JPLHOR.  
 * This options works only, if the files DPSI_DEPS_IAU1980_FILE_EOPC04 
 * and DPSI_DEPS_IAU1980_FILE_FINALS are in the ephemeris path.
 *
 * If the software does not find the earth orientation files 
 * in the ephemeris path, then SEFLG_JPLHOR will run as 
 * SEFLG_JPLHOR_APPROX.
 */

/* coordinate transformation */
extern void swi_coortrf(double *xpo, double *xpn, double eps);

/* coordinate transformation */
extern void swi_coortrf2(double *xpo, double *xpn, double sineps, double coseps);

/* cartesian to polar coordinates */
extern void swi_cartpol(double *x, double *l);
 
/* cartesian to polar coordinates with velocity */
extern void swi_cartpol_sp(double *x, double *l);
extern void swi_polcart_sp(double *l, double *x);
 
/* polar to cartesian coordinates */
extern void swi_polcart(double *l, double *x);

/* GCRS to J2000 */
extern void swi_bias(double *x, double tjd, int32 iflag, AS_BOOL backward);
extern void swi_get_eop_time_range(void);
/* GCRS to FK5 */
extern void swi_icrs2fk5(double *x, int32 iflag, AS_BOOL backward);

/* precession */
extern int swi_precess(double *R, double J, int32 iflag, int direction );
extern void swi_precess_speed(double *xx, double t, int32 iflag, int direction);

extern int32 swi_guess_ephe_flag(void);

/* from sweph.c, light deflection, aberration, etc. */
extern void swi_deflect_light(double *xx, double dt, int32 iflag);
extern void swi_aberr_light(double *xx, double *xe, int32 iflag);
extern int swi_plan_for_osc_elem(int32 iflag, double tjd, double *xx);
extern int swi_trop_ra2sid_lon(double *xin, double *xout, double *xoutr, int32 iflag);
extern int swi_trop_ra2sid_lon_sosy(double *xin, double *xout, int32 iflag);
extern int swi_get_observer(double tjd, int32 iflag, 
	AS_BOOL do_save, double *xobs, char *serr);
extern void swi_force_app_pos_etc(void);

/* obliquity of ecliptic */
extern void swi_check_ecliptic(double tjd, int32 iflag);
extern double swi_epsiln(double J, int32 iflag);
extern void swi_ldp_peps(double J, double *dpre, double *deps);

/* nutation */
extern void swi_check_nutation(double tjd, int32 iflag);
extern int swi_nutation(double J, int32 iflag, double *nutlo);
extern void swi_nutate(double *xx, int32 iflag, AS_BOOL backward);

extern void swi_mean_lunar_elements(double tjd, 
							 double *node, double *dnode, 
							 double *peri, double *dperi);
/* */
extern double swi_mod2PI(double x);

/* evaluation of chebyshew series and derivative */
extern double swi_echeb(double x, double *coef, int ncf);
extern double swi_edcheb(double x, double *coef, int ncf);

/* cross product of vectors */
extern void swi_cross_prod(double *a, double *b, double *x);
/* dot product of vecotrs */
extern double swi_dot_prod_unit(double *x, double *y);

extern double swi_angnorm(double x);

/* generation of SWISSEPH file names */
extern void swi_gen_filename(double tjd, int ipli, char *fname);

/* cyclic redundancy checksum (CRC), 32 bit */
extern uint32 swi_crc32(unsigned char *buf, int len);

extern int swi_cutstr(char *s, char *cutlist, char *cpos[], int nmax);
extern char *swi_right_trim(char *s);

extern double swi_kepler(double E, double M, double ecce);

extern char *swi_get_fict_name(int32 ipl, char *s);

extern void swi_FK4_FK5(double *xp, double tjd);

extern char *swi_strcpy(char *to, char *from);
extern char *swi_strncpy(char *to, char *from, size_t n);

extern double swi_deltat_ephe(double tjd_ut, int32 epheflag);

#ifdef TRACE
#  define TRACE_COUNT_MAX         10000
  extern TLS FILE *swi_fp_trace_c;
  extern TLS FILE *swi_fp_trace_out;
  extern TLS int32 swi_trace_count;
  extern void swi_open_trace(char *serr);
  static const char *fname_trace_c = "swetrace.c";
  static const char *fname_trace_out = "swetrace.txt";
#ifdef FORCE_IFLAG
  static const char *fname_force_flg = "force.flg";
#endif
#endif /* TRACE */
