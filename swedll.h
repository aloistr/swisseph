/* SWISSEPH
 * $Header: swedll.h,v 1.65 2003/06/14 13:09:51 alois Exp $
 *
 *  Windows DLL interface imports for the Astrodienst SWISSEPH package
 *
 */

/* $Id$ */

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


DllImport double FAR PASCAL swe_degnorm(double deg);

DllImport int32 FAR PASCAL swe_calc( 
        double tjd, int ipl, int32 iflag, 
        double *xx,
        char *serr);

DllImport int32 FAR PASCAL swe_calc_ut( 
        double tjd_ut, int32 ipl, int32 iflag, 
        double *xx,
        char *serr);

DllImport int32 FAR PASCAL swe_fixstar(
        char *star, double tjd, int32 iflag, 
        double *xx,
        char *serr);

DllImport int32 FAR PASCAL swe_fixstar_ut(
        char *star, double tjd_ut, int32 iflag, 
        double *xx,
        char *serr);

DllImport double FAR PASCAL swe_sidtime0(double tjd_ut, double ecl, double nut);
DllImport double FAR PASCAL swe_sidtime(double tjd_ut);

DllImport double FAR PASCAL swe_deltat(double tjd);

DllImport int FAR PASCAL swe_houses(
        double tjd_ut, double geolat, double geolon, int hsys, 
        double *hcusps, double *ascmc);

DllImport int FAR PASCAL swe_houses_ex(
        double tjd_ut, int32 iflag, double geolat, double geolon, int hsys, 
        double *hcusps, double *ascmc);

DllImport int FAR PASCAL swe_houses_armc(
        double armc, double geolat, double eps, int hsys, 
        double *hcusps, double *ascmc);

DllImport double FAR PASCAL swe_house_pos(
        double armc, double geolon, double eps, int hsys, double *xpin, char *serr);

DllImport int32 FAR PASCAL swe_gauquelin_sector(
	double t_ut, int32 ipl, char *starname, int32 iflag, int32 imeth, double *geopos, double atpress, double attemp, double *dgsect, char *serr);

DllImport void FAR PASCAL swe_set_sid_mode(
        int32 sid_mode, double t0, double ayan_t0);

DllImport double FAR PASCAL swe_get_ayanamsa(double tjd_et);

DllImport double FAR PASCAL swe_get_ayanamsa_ut(double tjd_ut);

DllImport char *FAR PASCAL swe_get_ayanamsa_name(int32 isidmode);

DllImport int FAR PASCAL swe_date_conversion(
        int y , int m , int d ,         /* year, month, day */
        double utime,   /* universal time in hours (decimal) */
        char c,         /* calendar g[regorian]|j[ulian]|a[stro = greg] */
        double *tjd);

DllImport double FAR PASCAL swe_julday(
        int year, int mon, int mday,
        double hour,
        int gregflag);

DllImport void FAR PASCAL swe_revjul(
        double jd, int gregflag,
        int *year, int *mon, int *mday,
        double *hour);

DllImport int FAR PASCAL swe_time_equ(
        double tjd, double *e, char *serr);

DllImport double FAR PASCAL swe_get_tid_acc(void);
DllImport void FAR PASCAL swe_set_tid_acc(double tidacc);
DllImport void FAR PASCAL swe_set_ephe_path(char *path);
DllImport void FAR PASCAL swe_set_jpl_file(char *fname);
DllImport void FAR PASCAL swe_close(void);
DllImport char *FAR PASCAL swe_get_planet_name(int ipl, char *spname);
DllImport void FAR PASCAL swe_cotrans(double *xpo, double *xpn, double eps);
DllImport void FAR PASCAL swe_cotrans_sp(double *xpo, double *xpn, double eps);

DllImport void FAR PASCAL swe_set_topo(double geolon, double geolat, double height);

/**************************** 
 * from swecl.c 
 ****************************/

/* computes geographic location and attributes of solar 
 * eclipse at a given tjd */
DllImport int32 FAR PASCAL swe_sol_eclipse_where(double tjd, int32 ifl, double *geopos, double *attr, char *serr);

DllImport int32 FAR PASCAL swe_lun_occult_where(double tjd, int32 ipl, char *starname, int32 ifl, double *geopos, double *attr, char *serr);

/* computes attributes of a solar eclipse for given tjd, geolon, geolat */
DllImport int32 FAR PASCAL swe_sol_eclipse_how(double tjd, int32 ifl, double *geopos, double *attr, char *serr);

/* finds time of next local eclipse */
DllImport int32 FAR PASCAL swe_sol_eclipse_when_loc(double tjd_start, int32 ifl, double *geopos, double *tret, double *attr, int32 backward, char *serr);

DllImport int32 FAR PASCAL swe_lun_occult_when_loc(double tjd_start, int32 ipl, char *starname, int32 ifl, double *geopos, double *tret, double *attr, int32 backward, char *serr);

/* finds time of next eclipse globally */
DllImport int32 FAR PASCAL swe_sol_eclipse_when_glob(double tjd_start, int32 ifl, int32 ifltype, double *tret, int32 backward, char *serr);

/* finds time of next occultation globally */
DllImport int32 FAR PASCAL swe_lun_occult_when_glob(double tjd_start, int32 ipl, char *starname, int32 ifl, int32 ifltype, double *tret, int32 backward, char *serr);

/* computes attributes of a lunar eclipse for given tjd */
DllImport int32 FAR PASCAL swe_lun_eclipse_how(
          double tjd_ut, 
          int32 ifl,
	  double *geopos,
          double *attr, 
          char *serr);
DllImport int32 FAR PASCAL swe_lun_eclipse_when(double tjd_start, int32 ifl, int32 ifltype, double *tret, int32 backward, char *serr);

/* planetary phenomena */
DllImport int32 FAR PASCAL swe_pheno(double tjd, int32 ipl, int32 iflag, 
				double *attr, char *serr);

DllImport int32 FAR PASCAL swe_pheno_ut(double tjd_ut, int32 ipl, int32 iflag, double *attr, char *serr);

DllImport double FAR PASCAL swe_refrac(double inalt, double atpress, double attemp, int32 calc_flag);

DllImport void FAR PASCAL swe_azalt(
      double tjd_ut,
      int32 calc_flag,
      double *geopos,
      double atpress,
      double attemp,
      double *xin, 
      double *xaz); 

DllImport void FAR PASCAL swe_azalt_rev(
      double tjd_ut,
      int32 calc_flag,
      double *geopos,
      double *xin, 
      double *xout); 

DllImport int32 FAR PASCAL swe_rise_trans(
               double tjd_ut, int32 ipl, char *starname, 
	       int32 epheflag, int32 rsmi,
               double *geopos, 
	       double atpress, double attemp,
               double *tret,
               char *serr);

DllImport int32 FAR PASCAL swe_nod_aps(double tjd_et, int32 ipl, int32 iflag, 
                      int32  method,
                      double *xnasc, double *xndsc, 
                      double *xperi, double *xaphe, 
                      char *serr);

DllImport int32 FAR PASCAL swe_nod_aps_ut(double tjd_ut, int32 ipl, int32 iflag, 
                      int32  method,
                      double *xnasc, double *xndsc, 
                      double *xperi, double *xaphe, 
                      char *serr);

/******************************************************* 
 * other functions from swephlib.c;
 * they are not needed for Swiss Ephemeris,
 * but may be useful to former Placalc users.
 ********************************************************/

/* normalize argument into interval [0..DEG360] */
DllImport centisec FAR PASCAL swe_csnorm(centisec p);

/* distance in centisecs p1 - p2 normalized to [0..360[ */
DllImport centisec FAR PASCAL swe_difcsn (centisec p1, centisec p2);

DllImport double FAR PASCAL swe_difdegn (double p1, double p2);

/* distance in centisecs p1 - p2 normalized to [-180..180[ */
DllImport centisec FAR PASCAL swe_difcs2n(centisec p1, centisec p2);

DllImport double FAR PASCAL swe_difdeg2n(double p1, double p2);

DllImport double FAR PASCAL swe_difdeg2n(double p1, double p2);
DllImport double FAR PASCAL swe_difrad2n(double p1, double p2);
DllImport double FAR PASCAL swe_rad_midp(double x1, double x0);
DllImport double FAR PASCAL swe_deg_midp(double x1, double x0);

/* round second, but at 29.5959 always down */
DllImport centisec FAR PASCAL swe_csroundsec(centisec x);

/* double to int32 with rounding, no overflow check */
DllImport int32 FAR PASCAL swe_d2l(double x);

DllImport void FAR PASCAL swe_split_deg(double ddeg, int32 roundflag, int32 *ideg, int32 *imin, int32 *isec, double *dsecfr, int32 *isgn);

/* monday = 0, ... sunday = 6 */
DllImport int FAR PASCAL swe_day_of_week(double jd);

DllImport char *FAR PASCAL swe_cs2timestr(CSEC t, int sep, AS_BOOL suppressZero, char *a);

DllImport char *FAR PASCAL swe_cs2lonlatstr(CSEC t, char pchar, char mchar, char *s);

DllImport char *FAR PASCAL swe_cs2degstr(CSEC t, char *a);


/* additional functions for antiquated GFA basic DLL interface.
 * double -> double *
 * char   -> char *
 * void   -> int
 */

DllImport int32 FAR PASCAL swe_calc_d(
    double *tjd, int ipl, int32 iflag,
    double *x,
    char *serr);

DllImport int32 FAR PASCAL swe_calc_ut_d(
    double *tjd, int16 ipl, int32 iflag,
    double *x,
    char *serr);

DllImport int32 FAR PASCAL swe_fixstar_d(
    char *star, double *tjd, int32 iflag,
    double *x,
    char *serr);

DllImport int32 FAR PASCAL swe_fixstar_ut_d(
    char *star, double *tjd, int32 iflag,
    double *x,
    char *serr);

DllImport int FAR PASCAL swe_close_d(int ivoid);

DllImport int FAR PASCAL swe_set_ephe_path_d(char *path);

DllImport int FAR PASCAL swe_set_jpl_file_d(char *fname);

DllImport char *FAR PASCAL swe_get_planet_name_d(int ipl, char *spname);

DllImport int FAR PASCAL swe_deltat_d(double *tjd, double *deltat);

DllImport int FAR PASCAL swe_sidtime0_d(double *tjd_ut, double *eps,
double *nut, double *sidt);
DllImport int FAR PASCAL swe_sidtime_d(double *tjd_ut, double *sidt);

DllImport int FAR PASCAL swe_set_sid_mode_d(
        int32 sid_mode, double *t0, double *ayan_t0);

DllImport int FAR PASCAL swe_get_ayanamsa_d(double *tjd_et, double *ayan);
DllImport int FAR PASCAL swe_get_ayanamsa_ut_d(double *tjd_et, double *ayan);

DllImport int FAR PASCAL swe_cotrans_d(double *xpo, double *xpn, double *eps);
DllImport int FAR PASCAL swe_cotrans_sp_d(double *xpo, double *xpn, double *eps);

DllImport int FAR PASCAL swe_set_topo_d(double *geolon, double *geolat, double *height);

DllImport int FAR PASCAL swe_get_tid_acc_d(double *t_acc);
DllImport int FAR PASCAL swe_set_tid_acc_d(double *t_acc);

DllImport int FAR PASCAL swe_degnorm_d(double *x);

DllImport int FAR PASCAL swe_date_conversion_d(
        int y , int m , int d ,         /* year, month, day */
        double *utime,   /* universal time in hours (decimal) */
        char *c,         /* calendar g[regorian]|j[ulian]|a[stro = greg] */
        double *tjd);

DllImport int FAR PASCAL swe_julday_d(
        int year, int month, int day, double *hour,
        int gregflag, double *tjd);

DllImport int FAR PASCAL swe_revjul_d(
        double *tjd,
        int gregflag,
        int *jyear, int *jmon, int *jday, double *jut);

DllImport int FAR PASCAL swe_houses_d(
        double *tjd, double *geolat, double *geolon, int hsys,
        double *hcusps, double *ascmc);

DllImport int FAR PASCAL swe_houses_ex_d(
        double *tjd_ut, int32 iflag, double *geolat, double *geolon, int hsys, 
        double *hcusps, double *ascmc);

DllImport int FAR PASCAL swe_houses_armc_d(
        double *armc, double *geolat, double *eps, int hsys,
        double *hcusps, double *ascmc);

DllImport int FAR PASCAL swe_house_pos_d(
        double *armc, double *geolon, double *eps, int hsys, double *xpin, double *hpos, char *serr);

/* normalize argument into interval [0..DEG360] */
DllImport centisec FAR PASCAL swe_csnorm_d(centisec p);

/* distance in centisecs p1 - p2 normalized to [0..360[ */
DllImport centisec FAR PASCAL swe_difcsn_d(centisec p1, centisec p2);

DllImport int FAR PASCAL swe_difdegn_d(double *p1, double *p2, double *diff);

/* distance in centisecs p1 - p2 normalized to [-180..180[ */
DllImport centisec FAR PASCAL swe_difcs2n_d(centisec p1, centisec p2);

DllImport int FAR PASCAL swe_difdeg2n_d(double *p1, double *p2, double *diff);

/* round second, but at 29.5959 always down */
DllImport centisec FAR PASCAL swe_csroundsec_d(centisec x);

/* double to int32 with rounding, no overflow check */
DllImport int32 FAR PASCAL swe_d2l_d(double *x);

DllImport int FAR PASCAL swe_split_deg_d(double *ddeg, int32 roundflag, int32 *ideg, int32 *imin, int32 *isec, double *dsecfr, int32 *isgn);

/* monday = 0, ... sunday = 6 */
DllImport int FAR PASCAL swe_day_of_week_d(double *jd);

DllImport char *FAR PASCAL swe_cs2timestr_d(CSEC t, int sep, AS_BOOL suppressZero, char *a);

DllImport char *FAR PASCAL swe_cs2lonlatstr_d(CSEC t, char *pchar, char *mchar, char *s);

DllImport char *FAR PASCAL swe_cs2degstr_d(CSEC t, char *a);

/**************************** 
 * from swecl.c 
 ****************************/

/* computes geographic location and attributes of solar 
 * eclipse at a given tjd */
DllImport int32 FAR PASCAL swe_sol_eclipse_where_d(double *tjd_ut, int32 ifl, double *geopos, double *attr, char *serr);

/* computes attributes of a solar eclipse for given tjd, geolon, geolat */
DllImport int32 FAR PASCAL swe_sol_eclipse_how_d(double *tjd_ut, int32 ifl, double geolon, double geolat, double geohgt, double *attr, char *serr);

/* finds time of next local eclipse */
DllImport int32 FAR PASCAL swe_sol_eclipse_when_loc_d(double *tjd_start, int32 ifl, double *geopos, double *tret, double *attr, AS_BOOL backward, char *serr);

/* finds time of next eclipse globally */
DllImport int32 FAR PASCAL swe_sol_eclipse_when_glob_d(double *tjd_start, int32 ifl, int32 ifltype,
     double *tret, AS_BOOL backward, char *serr);

/* computes attributes of a lunar eclipse for given tjd */
DllImport int32 FAR PASCAL swe_lun_eclipse_how_d(
          double *tjd_ut, 
          int32 ifl,
          double *attr, 
          char *serr);
DllImport int32 FAR PASCAL swe_lun_eclipse_when_d(double *tjd_start, int32 ifl, int32 ifltype,
     double *tret, AS_BOOL backward, char *serr);

DllImport int32 FAR PASCAL swe_pheno_d(double *tjd, int32 ipl, int32 iflag, 
				double *attr, char *serr);

DllImport int32 FAR PASCAL swe_pheno_ut_d(double *tjd_ut, int32 ipl, int32 iflag, double *attr, char *serr);

DllImport int FAR PASCAL swe_refrac_d(double *inalt, double *atpress, double *attemp, int32 calc_flag, double *retalt);

DllImport int FAR PASCAL swe_azalt_d(
      double *tjd_ut,
      int32 calc_flag,
      double *geopos,
      double *atpress,
      double *attemp,
      double *xin, 
      double *xaz); 

DllImport int FAR PASCAL swe_azalt_rev_d(
      double *tjd_ut,
      int32 calc_flag,
      double *geopos,
      double *xin, 
      double *xout); 

DllImport int32 FAR PASCAL swe_rise_trans_d(
               double *tjd_ut, int32 ipl, char *starname, 
	       int32 epheflag, int32 rsmi,
               double *geopos, 
	       double *atpress, double *attemp,
               double *tret,
               char *serr);

DllImport int32 FAR PASCAL swe_nod_aps_d(double *tjd_et, int32 ipl, int32 iflag, 
                      int32  method,
                      double *xnasc, double *xndsc, 
                      double *xperi, double *xaphe, 
                      char *serr);

DllImport int32 FAR PASCAL swe_nod_aps_ut_d(double *tjd_ut, int32 ipl, int32 iflag, 
                      int32  method,
                      double *xnasc, double *xndsc, 
                      double *xperi, double *xaphe, 
                      char *serr);

#endif /* !_SWEDLL_H */
#ifdef __cplusplus
}
#endif
