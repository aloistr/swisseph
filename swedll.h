/* SWISSEPH
 * $Header: /home/dieter/sweph/RCS/swedll.h,v 1.75 2009/04/08 07:19:08 dieter Exp $
 *
 *  Windows DLL interface imports for the Astrodienst SWISSEPH package
 *

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

/* $Id: swedll.h,v 1.75 2009/04/08 07:19:08 dieter Exp $ */

#ifdef __cplusplus
extern "C" {
#endif
#ifndef _SWEDLL_H
#define _SWEDLL_H

#ifndef _SWEPHEXP_INCLUDED   
#include "swephexp.h"
#endif

#ifdef USE_DLL16 /* 16bit DLL */
#define DllImport extern 
#else
# ifdef __cplusplus
#define DllImport extern "C" __declspec( dllimport )
# else
#define DllImport  __declspec( dllimport )
# endif
#endif

/* DLL defines
  Define UNDECO_DLL for un-decorated dll
  verify compiler option __cdecl for un-decorated and __stdcall for decorated */
/*#define UNDECO_DLL */

#if defined (PASCAL) || defined(__stdcall)
  #if defined UNDECO_DLL
    #define CALL_CONV_IMP __cdecl
  #else
    #define CALL_CONV_IMP __stdcall
  #endif 
#else
  #define CALL_CONV_IMP 
#endif

DllImport int32 CALL_CONV_IMP swe_heliacal_ut(double JDNDaysUTStart, double *geopos, double *datm, double *dobs, char *ObjectName, int32 TypeEvent, int32 iflag, double *dret, char *serr);
DllImport int32 CALL_CONV_IMP swe_heliacal_pheno_ut(double JDNDaysUT, double *geopos, double *datm, double *dobs, char *ObjectName, int32 TypeEvent, int32 helflag, double *darr, char *serr);
DllImport int32 CALL_CONV_IMP swe_vis_limit_mag(double tjdut, double *geopos, double *datm, double *dobs, char *ObjectName, int32 helflag, double *dret, char *serr);
/* the following are secret, for Victor Reijs' */
DllImport int32 CALL_CONV_IMP swe_heliacal_angle(double tjdut, double *dgeo, double *datm, double *dobs, int32 helflag, double mag, double azi_obj, double azi_sun, double azi_moon, double alt_moon, double *dret, char *serr);
DllImport int32 CALL_CONV_IMP swe_topo_arcus_visionis(double tjdut, double *dgeo, double *datm, double *dobs, int32 helflag, double mag, double azi_obj, double alt_obj, double azi_sun, double azi_moon, double alt_moon, double *dret, char *serr);

DllImport double CALL_CONV_IMP swe_degnorm(double deg);

DllImport char * CALL_CONV_IMP swe_version(char *);
DllImport char * CALL_CONV_IMP swe_get_library_path(char *);

DllImport int32 CALL_CONV_IMP swe_calc( 
        double tjd, int ipl, int32 iflag, 
        double *xx,
        char *serr);

DllImport int32 CALL_CONV_IMP swe_calc_ut( 
        double tjd_ut, int32 ipl, int32 iflag, 
        double *xx,
        char *serr);

DllImport int32 CALL_CONV_IMP swe_fixstar(
        char *star, double tjd, int32 iflag, 
        double *xx,
        char *serr);

DllImport int32 CALL_CONV_IMP swe_fixstar_ut(
        char *star, double tjd_ut, int32 iflag, 
        double *xx,
        char *serr);

DllImport int32 CALL_CONV_IMP swe_fixstar_mag(
        char *star, double *xx, char *serr);

DllImport double CALL_CONV_IMP swe_sidtime0(double tjd_ut, double ecl, double nut);
DllImport double CALL_CONV_IMP swe_sidtime(double tjd_ut);

DllImport double CALL_CONV_IMP swe_deltat_ex(double tjd, int32 iflag, char *serr);
DllImport double CALL_CONV_IMP swe_deltat(double tjd);

DllImport int  CALL_CONV_IMP swe_houses(
        double tjd_ut, double geolat, double geolon, int hsys, 
        double *hcusps, double *ascmc);

DllImport int  CALL_CONV_IMP swe_houses_ex(
        double tjd_ut, int32 iflag, double geolat, double geolon, int hsys, 
        double *hcusps, double *ascmc);

DllImport int  CALL_CONV_IMP swe_houses_armc(
        double armc, double geolat, double eps, int hsys, 
        double *hcusps, double *ascmc);

DllImport double  CALL_CONV_IMP swe_house_pos(
        double armc, double geolon, double eps, int hsys, double *xpin, char *serr);

DllImport char * CALL_CONV_IMP swe_house_name(int hsys);

DllImport int32  CALL_CONV_IMP swe_gauquelin_sector(
	double t_ut, int32 ipl, char *starname, int32 iflag, int32 imeth, double *geopos, double atpress, double attemp, double *dgsect, char *serr);

DllImport void  CALL_CONV_IMP swe_set_sid_mode(
        int32 sid_mode, double t0, double ayan_t0);

DllImport int32  CALL_CONV_IMP swe_get_ayanamsa_ex(double tjd_et, int32 iflag, double *daya, char *serr);
DllImport int32  CALL_CONV_IMP swe_get_ayanamsa_ex_ut(double tjd_ut, int32 iflag, double *daya, char *serr);

DllImport double  CALL_CONV_IMP swe_get_ayanamsa(double tjd_et);
DllImport double  CALL_CONV_IMP swe_get_ayanamsa_ut(double tjd_ut);

DllImport char * CALL_CONV_IMP swe_get_ayanamsa_name(int32 isidmode);

DllImport int  CALL_CONV_IMP swe_date_conversion(
        int y , int m , int d ,         /* year, month, day */
        double utime,   /* universal time in hours (decimal) */
        char c,         /* calendar g[regorian]|j[ulian]|a[stro = greg] */
        double *tjd);

DllImport double  CALL_CONV_IMP swe_julday(
        int year, int mon, int mday,
        double hour,
        int gregflag);

DllImport void  CALL_CONV_IMP swe_revjul(
        double jd, int gregflag,
        int *year, int *mon, int *mday,
        double *hour);

DllImport void  CALL_CONV_IMP swe_utc_time_zone(
        int32 iyear, int32 imonth, int32 iday,
	int32 ihour, int32 imin, double dsec,
	double d_timezone,
	int32 *iyear_out, int32 *imonth_out, int32 *iday_out,
	int32 *ihour_out, int32 *imin_out, double *dsec_out);

DllImport int32  CALL_CONV_IMP swe_utc_to_jd(
        int32 iyear, int32 imonth, int32 iday, 
	int32 ihour, int32 imin, double dsec, 
	int32 gregflag, double *dret, char *serr);

DllImport void  CALL_CONV_IMP swe_jdet_to_utc(
        double tjd_et, int32 gregflag, 
	int32 *iyear, int32 *imonth, int32 *iday, 
	int32 *ihour, int32 *imin, double *dsec);

DllImport void  CALL_CONV_IMP swe_jdut1_to_utc(
        double tjd_ut, int32 gregflag, 
	int32 *iyear, int32 *imonth, int32 *iday, 
	int32 *ihour, int32 *imin, double *dsec);

DllImport int  CALL_CONV_IMP swe_time_equ(
        double tjd, double *e, char *serr);
DllImport int  CALL_CONV_IMP swe_lmt_to_lat(double tjd_lmt, double geolon, double *tjd_lat, char *serr);
DllImport int  CALL_CONV_IMP swe_lat_to_lmt(double tjd_lat, double geolon, double *tjd_lmt, char *serr);

DllImport double  CALL_CONV_IMP swe_get_tid_acc(void);
DllImport void  CALL_CONV_IMP swe_set_tid_acc(double tidacc);
DllImport void  CALL_CONV_IMP swe_set_delta_t_userdef(double dt);
DllImport void  CALL_CONV_IMP swe_set_ephe_path(char *path);
DllImport void  CALL_CONV_IMP swe_set_jpl_file(char *fname);
DllImport void  CALL_CONV_IMP swe_close(void);
DllImport char * CALL_CONV_IMP swe_get_planet_name(int ipl, char *spname);
DllImport void  CALL_CONV_IMP swe_cotrans(double *xpo, double *xpn, double eps);
DllImport void  CALL_CONV_IMP swe_cotrans_sp(double *xpo, double *xpn, double eps);

DllImport void  CALL_CONV_IMP swe_set_topo(double geolon, double geolat, double height);

DllImport void CALL_CONV_IMP swe_set_astro_models(char *samod, int32 iflag);
DllImport void CALL_CONV_IMP swe_get_astro_models(char *samod, char *sdet, int32 iflag);

/**************************** 
 * from swecl.c 
 ****************************/

/* computes geographic location and attributes of solar 
 * eclipse at a given tjd */
DllImport int32  CALL_CONV_IMP swe_sol_eclipse_where(double tjd, int32 ifl, double *geopos, double *attr, char *serr);

DllImport int32  CALL_CONV_IMP swe_lun_occult_where(double tjd, int32 ipl, char *starname, int32 ifl, double *geopos, double *attr, char *serr);

/* computes attributes of a solar eclipse for given tjd, geolon, geolat */
DllImport int32  CALL_CONV_IMP swe_sol_eclipse_how(double tjd, int32 ifl, double *geopos, double *attr, char *serr);

/* finds time of next local eclipse */
DllImport int32  CALL_CONV_IMP swe_sol_eclipse_when_loc(double tjd_start, int32 ifl, double *geopos, double *tret, double *attr, int32 backward, char *serr);

DllImport int32  CALL_CONV_IMP swe_lun_occult_when_loc(double tjd_start, int32 ipl, char *starname, int32 ifl, double *geopos, double *tret, double *attr, int32 backward, char *serr);

/* finds time of next eclipse globally */
DllImport int32  CALL_CONV_IMP swe_sol_eclipse_when_glob(double tjd_start, int32 ifl, int32 ifltype, double *tret, int32 backward, char *serr);

/* finds time of next occultation globally */
DllImport int32  CALL_CONV_IMP swe_lun_occult_when_glob(double tjd_start, int32 ipl, char *starname, int32 ifl, int32 ifltype, double *tret, int32 backward, char *serr);

/* computes attributes of a lunar eclipse for given tjd */
DllImport int32  CALL_CONV_IMP swe_lun_eclipse_how(
          double tjd_ut, 
          int32 ifl,
	  double *geopos,
          double *attr, 
          char *serr);
DllImport int32  CALL_CONV_IMP swe_lun_eclipse_when(double tjd_start, int32 ifl, int32 ifltype, double *tret, int32 backward, char *serr);
DllImport int32  CALL_CONV_IMP swe_lun_eclipse_when_loc(double tjd_start, int32 ifl, double *geopos, double *tret, double *attr, int32 backward, char *serr);
/* planetary phenomena */
DllImport int32  CALL_CONV_IMP swe_pheno(double tjd, int32 ipl, int32 iflag, double *attr, char *serr);

DllImport int32  CALL_CONV_IMP swe_pheno_ut(double tjd_ut, int32 ipl, int32 iflag, double *attr, char *serr);

DllImport double  CALL_CONV_IMP swe_refrac(double inalt, double atpress, double attemp, int32 calc_flag);
DllImport double  CALL_CONV_IMP swe_refrac_extended(double inalt, double geoalt, double atpress, double attemp, double lapse_rate, int32 calc_flag, double *dret);
DllImport void  CALL_CONV_IMP swe_set_lapse_rate(double lapse_rate);

DllImport void  CALL_CONV_IMP swe_azalt(
      double tjd_ut,
      int32 calc_flag,
      double *geopos,
      double atpress,
      double attemp,
      double *xin, 
      double *xaz); 

DllImport void  CALL_CONV_IMP swe_azalt_rev(
      double tjd_ut,
      int32 calc_flag,
      double *geopos,
      double *xin, 
      double *xout); 

DllImport int32  CALL_CONV_IMP swe_rise_trans(
               double tjd_ut, int32 ipl, char *starname, 
	       int32 epheflag, int32 rsmi,
               double *geopos, 
	       double atpress, double attemp,
               double *tret,
               char *serr);

DllImport int32  CALL_CONV_IMP swe_rise_trans_true_hor(
               double tjd_ut, int32 ipl, char *starname, 
	       int32 epheflag, int32 rsmi,
               double *geopos, 
	       double atpress, double attemp,
	       double horhgt,
               double *tret,
               char *serr);

DllImport int32  CALL_CONV_IMP swe_nod_aps(double tjd_et, int32 ipl, int32 iflag, 
                      int32  method,
                      double *xnasc, double *xndsc, 
                      double *xperi, double *xaphe, 
                      char *serr);

DllImport int32  CALL_CONV_IMP swe_nod_aps_ut(double tjd_ut, int32 ipl, int32 iflag, 
                      int32  method,
                      double *xnasc, double *xndsc, 
                      double *xperi, double *xaphe, 
                      char *serr);

DllImport int32 CALL_CONV_IMP swe_get_orbital_elements(double tjd_et, int32 ipl, int32 iflag, double *dret, char *serr);

DllImport int32 CALL_CONV_IMP swe_orbit_max_min_true_distance(double tjd_et, int32 ipl, int32 iflag, double *dmax, double *dmin, double *dtrue, char *serr);

/******************************************************* 
 * other functions from swephlib.c;
 * they are not needed for Swiss Ephemeris,
 * but may be useful to former Placalc users.
 ********************************************************/

/* normalize argument into interval [0..DEG360] */
DllImport centisec  CALL_CONV_IMP swe_csnorm(centisec p);

/* distance in centisecs p1 - p2 normalized to [0..360[ */
DllImport centisec  CALL_CONV_IMP swe_difcsn (centisec p1, centisec p2);

DllImport double  CALL_CONV_IMP swe_difdegn (double p1, double p2);

/* distance in centisecs p1 - p2 normalized to [-180..180[ */
DllImport centisec  CALL_CONV_IMP swe_difcs2n(centisec p1, centisec p2);

DllImport double  CALL_CONV_IMP swe_difdeg2n(double p1, double p2);

DllImport double  CALL_CONV_IMP swe_difdeg2n(double p1, double p2);
DllImport double  CALL_CONV_IMP swe_difrad2n(double p1, double p2);
DllImport double  CALL_CONV_IMP swe_rad_midp(double x1, double x0);
DllImport double  CALL_CONV_IMP swe_deg_midp(double x1, double x0);

/* round second, but at 29.5959 always down */
DllImport centisec  CALL_CONV_IMP swe_csroundsec(centisec x);

/* double to int32 with rounding, no overflow check */
DllImport int32  CALL_CONV_IMP swe_d2l(double x);

DllImport void  CALL_CONV_IMP swe_split_deg(double ddeg, int32 roundflag, int32 *ideg, int32 *imin, int32 *isec, double *dsecfr, int32 *isgn);

/* monday = 0, ... sunday = 6 */
DllImport int  CALL_CONV_IMP swe_day_of_week(double jd);

DllImport char * CALL_CONV_IMP swe_cs2timestr(CSEC t, int sep, AS_BOOL suppressZero, char *a);

DllImport char * CALL_CONV_IMP swe_cs2lonlatstr(CSEC t, char pchar, char mchar, char *s);

DllImport char * CALL_CONV_IMP swe_cs2degstr(CSEC t, char *a);


/* additional functions for antiquated GFA basic DLL interface.
 * double -> double *
 * char   -> char *
 * void   -> int
 */

DllImport int32  CALL_CONV_IMP swe_calc_d(
    double *tjd, int ipl, int32 iflag,
    double *x,
    char *serr);

DllImport int32  CALL_CONV_IMP swe_calc_ut_d(
    double *tjd, int16 ipl, int32 iflag,
    double *x,
    char *serr);

DllImport int32  CALL_CONV_IMP swe_fixstar_d(
    char *star, double *tjd, int32 iflag,
    double *x,
    char *serr);

DllImport int32  CALL_CONV_IMP swe_fixstar_ut_d(
    char *star, double *tjd, int32 iflag,
    double *x,
    char *serr);

DllImport int  CALL_CONV_IMP swe_close_d(int ivoid);

DllImport int  CALL_CONV_IMP swe_set_ephe_path_d(char *path);

DllImport int  CALL_CONV_IMP swe_set_jpl_file_d(char *fname);

DllImport char * CALL_CONV_IMP swe_get_planet_name_d(int ipl, char *spname);

DllImport int  CALL_CONV_IMP swe_deltat_d(double *tjd, double *deltat);

DllImport int  CALL_CONV_IMP swe_sidtime0_d(double *tjd_ut, double *eps,
double *nut, double *sidt);
DllImport int  CALL_CONV_IMP swe_sidtime_d(double *tjd_ut, double *sidt);

DllImport int  CALL_CONV_IMP swe_set_sid_mode_d(
        int32 sid_mode, double *t0, double *ayan_t0);

DllImport int  CALL_CONV_IMP swe_get_ayanamsa_d(double *tjd_et, double *ayan);
DllImport int  CALL_CONV_IMP swe_get_ayanamsa_ut_d(double *tjd_et, double *ayan);

DllImport int  CALL_CONV_IMP swe_cotrans_d(double *xpo, double *xpn, double *eps);
DllImport int  CALL_CONV_IMP swe_cotrans_sp_d(double *xpo, double *xpn, double *eps);

DllImport int  CALL_CONV_IMP swe_set_topo_d(double *geolon, double *geolat, double *height);

DllImport int  CALL_CONV_IMP swe_get_tid_acc_d(double *t_acc);
DllImport int  CALL_CONV_IMP swe_set_tid_acc_d(double *t_acc);

DllImport int  CALL_CONV_IMP swe_degnorm_d(double *x);

DllImport int  CALL_CONV_IMP swe_date_conversion_d(
        int y , int m , int d ,         /* year, month, day */
        double *utime,   /* universal time in hours (decimal) */
        char *c,         /* calendar g[regorian]|j[ulian]|a[stro = greg] */
        double *tjd);

DllImport int  CALL_CONV_IMP swe_julday_d(
        int year, int month, int day, double *hour,
        int gregflag, double *tjd);

DllImport int  CALL_CONV_IMP swe_revjul_d(
        double *tjd,
        int gregflag,
        int *jyear, int *jmon, int *jday, double *jut);

DllImport int  CALL_CONV_IMP swe_houses_d(
        double *tjd, double *geolat, double *geolon, int hsys,
        double *hcusps, double *ascmc);

DllImport int  CALL_CONV_IMP swe_houses_ex_d(
        double *tjd_ut, int32 iflag, double *geolat, double *geolon, int hsys, 
        double *hcusps, double *ascmc);

DllImport int  CALL_CONV_IMP swe_houses_armc_d(
        double *armc, double *geolat, double *eps, int hsys,
        double *hcusps, double *ascmc);

DllImport int  CALL_CONV_IMP swe_house_pos_d(
        double *armc, double *geolon, double *eps, int hsys, double *xpin, double *hpos, char *serr);

/* normalize argument into interval [0..DEG360] */
DllImport centisec  CALL_CONV_IMP swe_csnorm_d(centisec p);

/* distance in centisecs p1 - p2 normalized to [0..360[ */
DllImport centisec  CALL_CONV_IMP swe_difcsn_d(centisec p1, centisec p2);

DllImport int  CALL_CONV_IMP swe_difdegn_d(double *p1, double *p2, double *diff);

/* distance in centisecs p1 - p2 normalized to [-180..180[ */
DllImport centisec  CALL_CONV_IMP swe_difcs2n_d(centisec p1, centisec p2);

DllImport int  CALL_CONV_IMP swe_difdeg2n_d(double *p1, double *p2, double *diff);

/* round second, but at 29.5959 always down */
DllImport centisec  CALL_CONV_IMP swe_csroundsec_d(centisec x);

/* double to int32 with rounding, no overflow check */
DllImport int32  CALL_CONV_IMP swe_d2l_d(double *x);

DllImport int  CALL_CONV_IMP swe_split_deg_d(double *ddeg, int32 roundflag, int32 *ideg, int32 *imin, int32 *isec, double *dsecfr, int32 *isgn);

/* monday = 0, ... sunday = 6 */
DllImport int  CALL_CONV_IMP swe_day_of_week_d(double *jd);

DllImport char * CALL_CONV_IMP swe_cs2timestr_d(CSEC t, int sep, AS_BOOL suppressZero, char *a);

DllImport char * CALL_CONV_IMP swe_cs2lonlatstr_d(CSEC t, char *pchar, char *mchar, char *s);

DllImport char * CALL_CONV_IMP swe_cs2degstr_d(CSEC t, char *a);

/**************************** 
 * from swecl.c 
 ****************************/

/* computes geographic location and attributes of solar 
 * eclipse at a given tjd */
DllImport int32  CALL_CONV_IMP swe_sol_eclipse_where_d(double *tjd_ut, int32 ifl, double *geopos, double *attr, char *serr);

/* computes attributes of a solar eclipse for given tjd, geolon, geolat */
DllImport int32  CALL_CONV_IMP swe_sol_eclipse_how_d(double *tjd_ut, int32 ifl, double geolon, double geolat, double geohgt, double *attr, char *serr);

/* finds time of next local eclipse */
DllImport int32  CALL_CONV_IMP swe_sol_eclipse_when_loc_d(double *tjd_start, int32 ifl, double *geopos, double *tret, double *attr, AS_BOOL backward, char *serr);

/* finds time of next eclipse globally */
DllImport int32  CALL_CONV_IMP swe_sol_eclipse_when_glob_d(double *tjd_start, int32 ifl, int32 ifltype,
     double *tret, AS_BOOL backward, char *serr);

/* computes attributes of a lunar eclipse for given tjd */
DllImport int32  CALL_CONV_IMP swe_lun_eclipse_how_d(
          double *tjd_ut, 
          int32 ifl,
          double *attr, 
          char *serr);
DllImport int32  CALL_CONV_IMP swe_lun_eclipse_when_d(double *tjd_start, int32 ifl, int32 ifltype,
     double *tret, AS_BOOL backward, char *serr);

DllImport int32  CALL_CONV_IMP swe_pheno_d(double *tjd, int32 ipl, int32 iflag, 
				double *attr, char *serr);

DllImport int32  CALL_CONV_IMP swe_pheno_ut_d(double *tjd_ut, int32 ipl, int32 iflag, double *attr, char *serr);

DllImport int  CALL_CONV_IMP swe_refrac_d(double *inalt, double *atpress, double *attemp, int32 calc_flag, double *retalt);

DllImport int  CALL_CONV_IMP swe_azalt_d(
      double *tjd_ut,
      int32 calc_flag,
      double *geopos,
      double *atpress,
      double *attemp,
      double *xin, 
      double *xaz); 

DllImport int  CALL_CONV_IMP swe_azalt_rev_d(
      double *tjd_ut,
      int32 calc_flag,
      double *geopos,
      double *xin, 
      double *xout); 

DllImport int32  CALL_CONV_IMP swe_rise_trans_d(
               double *tjd_ut, int32 ipl, char *starname, 
	       int32 epheflag, int32 rsmi,
               double *geopos, 
	       double *atpress, double *attemp,
               double *tret,
               char *serr);

DllImport int32  CALL_CONV_IMP swe_nod_aps_d(double *tjd_et, int32 ipl, int32 iflag, 
                      int32  method,
                      double *xnasc, double *xndsc, 
                      double *xperi, double *xaphe, 
                      char *serr);

DllImport int32  CALL_CONV_IMP swe_nod_aps_ut_d(double *tjd_ut, int32 ipl, int32 iflag, 
                      int32  method,
                      double *xnasc, double *xndsc, 
                      double *xperi, double *xaphe, 
                      char *serr);

#endif /* !_SWEDLL_H */
#ifdef __cplusplus
} /* extern C */
#endif
