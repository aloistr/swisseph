/* SWISSEPH
   $Header: swedllst.c,v 1.30 97/07/08 15:16:43 dieter Exp $
 *
 *  Windows DLL interface code for the Astrodienst SWISSEPH package
 */

#include "swephexp.h"

#ifdef MAKE_DLL16 /* 16bit DLL code for Windows 3.x */

int PASCAL LibMain(HANDLE hInstance, WORD wDataSeg,
        WORD cbHeapSize, LPSTR lpCmdLine)
{
  /* Since the DLL is non-reentrant it might be a good idea
     to return 0 here, if we're not the first instance.
     However, where is our instance counter? */
  return 1;  /* 1=success, 0=failure */
}

int PASCAL WEP(int nParameter) 
{
  /* nParameter is either WEB_SYSTEMEXIT or WEP_FREE_DLL,
     however this doesn't matter for us. */
  /*swe_close();*/
  return 1;  /* 1=success, 0=failure */
}

#else /* 32bit DLL code */

/* Export stubs for the 32bit PLACALC DLL as generated using
  Microsoft Developer Studio and Visual C/C++ 4.2 */
BOOL WINAPI DllMain(HANDLE hInst,
                    unsigned long ul_reason_for_call,
                    LPVOID lpReserved)
{
  switch(ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
      dllhandle = hInst;	// used by swe_version()
      break;
    case DLL_PROCESS_DETACH:
      swe_close();
      break;
    default:
      break;
  }
  /* TRUE (1) indicates success */
  return TRUE;
}
#endif /* 16,32 bit DLL code */

/* The GFABASIC DLL interface does not support 8 and 64 bit data types.
 * The following functions avoid problems with doubles by use of
 * pointers of doubles instead of doubles.
 * VOID is not supported either. We declare VOID functions int
 * and ignore the result.
 */

/* planets, moon, nodes etc. */
extern EXP32 long CALL_CONV EXP16 swe_calc_d(
    double *tjd, int ipl, long iflag,
    double *xx,
    char *serr);

/* planets, moon, nodes etc. */
extern EXP32 long CALL_CONV EXP16 swe_calc_ut_d(
    double *tjd_ut, int ipl, long iflag,
    double *xx,
    char *serr);

/* fixed stars */
extern EXP32 long CALL_CONV EXP16 swe_fixstar_d(
    char *star, double *tjd, long iflag,
    double *xx,
    char *serr);

/* fixed stars */
extern EXP32 long CALL_CONV EXP16 swe_fixstar_ut_d(
    char *star, double *tjd_ut, long iflag,
    double *xx,
    char *serr);

/* close Swiss Ephemeris */
extern EXP32 int CALL_CONV EXP16 swe_close_d(int ivoid);

/* set directory path of ephemeris files */
extern EXP32 int CALL_CONV EXP16 swe_set_ephe_path_d(char *path);

/* set file name of JPL file */
extern EXP32 int CALL_CONV EXP16 swe_set_jpl_file_d(char *fname);

/* get planet name */
extern EXP32 char *CALL_CONV EXP16 swe_get_planet_name_d(int ipl, char *spname);

/* set sidereal mode */
extern EXP32 int CALL_CONV EXP16 swe_set_sid_mode_d(long sid_mode, double *t0, double *ayan_t0);

/* get ayanamsa */
extern EXP32 int CALL_CONV EXP16 swe_get_ayanamsa_d(double *tjd_et, double *ayan);

/* get ayanamsa */
extern EXP32 int CALL_CONV EXP16 swe_get_ayanamsa_ut_d(double *tjd_ut, double *ayan);

/* delta t */
extern EXP32 int CALL_CONV EXP16 swe_deltat_d(double *tjd, double *deltat);

/* sidereal time */
extern EXP32 int CALL_CONV EXP16 swe_sidtime0_d(
        double *tjd_ut, double *eps, double *nut, double *sidt);
extern EXP32 int CALL_CONV EXP16 swe_sidtime_d(
        double *tjd_ut, double *sidt);

/* set geographic location for topocentric planet calculation */
extern EXP32 int CALL_CONV EXP16 swe_set_topo_d(double *geolon, double *geolat, double *height);

/* coordinate transformation polar -> polar */
extern EXP32 int CALL_CONV EXP16 swe_cotrans_d(double *xpo, double *xpn, double *eps);
extern EXP32 int CALL_CONV EXP16 swe_cotrans_sp_d(double *xpo, double *xpn, double *eps);

/* tidal acceleration to be used in swe_deltat() */
extern EXP32 int CALL_CONV EXP16 swe_get_tid_acc_d(double *t_acc);
extern EXP32 int CALL_CONV EXP16 swe_set_tid_acc_d(double *t_acc);

extern EXP32 int CALL_CONV EXP16 swe_degnorm_d(double *x);

extern EXP32 int CALL_CONV EXP16 swe_date_conversion_d(
        int y , int m , int d ,         /* year, month, day */
        double *utime,   /* universal time in hours (decimal) */
        char *c,         /* calendar g[regorian]|j[ulian]|a[stro = greg] */
        double *tjd);

extern EXP32 int CALL_CONV EXP16 swe_julday_d(
        int year, int month, int day, double *hour,
        int gregflag, double *tjd);

extern EXP32 int CALL_CONV EXP16 swe_revjul_d(
        double *tjd,
        int gregflag,
        int *jyear, int *jmon, int *jday, double *jut);

extern EXP32 int CALL_CONV EXP16 swe_time_equ_d(
        double *tjd,
        double *e,
        char *serr);

extern EXP32 int CALL_CONV EXP16 swe_houses_d(
        double *tjd_ut, double *geolat, double *geolon, int hsys, 
        double *cusps, double *ascmc);

extern EXP32 int CALL_CONV EXP16 swe_houses_ex_d(
        double *tjd_ut, int32 iflag, double *geolat, double *geolon, int hsys, 
        double *hcusps, double *ascmc);

extern EXP32 int CALL_CONV EXP16 swe_houses_armc_d(
        double *armc, double *geolat, double *eps, int hsys, 
        double *cusps, double *ascmc);

extern EXP32 int CALL_CONV EXP16 swe_house_pos_d(
        double *armc, double *geolat, double *eps, int hsys, double *xpin,
        double *hpos, char *serr);

/* normalize argument into interval [0..DEG360] */
extern EXP32 centisec CALL_CONV EXP16 swe_csnorm_d(centisec p);

/* distance in centisecs p1 - p2 normalized to [0..360[ */
extern EXP32 centisec CALL_CONV EXP16 swe_difcsn_d(centisec p1, centisec p2);

extern EXP32 int CALL_CONV EXP16 swe_difdegn_d(double *p1, double *p2, double *diff);

/* distance in centisecs p1 - p2 normalized to [-180..180[ */
extern EXP32 centisec CALL_CONV EXP16 swe_difcs2n_d(centisec p1, centisec p2);

extern EXP32 int CALL_CONV EXP16 swe_difdeg2n_d(double *p1, double *p2, double *diff);

/* round second, but at 29.5959 always down */
extern EXP32 centisec CALL_CONV EXP16 swe_csroundsec_d(centisec x);

/* double to long with rounding, no overflow check */
extern EXP32 long CALL_CONV EXP16 swe_d2l_d(double *x);

extern EXP32 int CALL_CONV EXP16 swe_split_deg_d(double *ddeg, int32 roundflag, int32 *ideg, int32 *imin, int32 *isec, double *dsecfr, int32 *isgn);

/* monday = 0, ... sunday = 6 */
extern EXP32 int CALL_CONV EXP16 swe_day_of_week_d(double *jd);

extern EXP32 char *CALL_CONV EXP16 swe_cs2timestr_d(CSEC t, int sep, AS_BOOL suppressZero, char *a);

extern EXP32 char *CALL_CONV EXP16 swe_cs2lonlatstr_d(CSEC t, char *pchar, char *mchar, char *s);

extern EXP32 char *CALL_CONV EXP16 swe_cs2degstr_d(CSEC t, char *a);


/**************************** 
 * from swecl.c 
 ****************************/

/* computes geographic location and attributes of solar 
 * eclipse at a given tjd */
extern EXP32 int32 CALL_CONV EXP16 swe_sol_eclipse_where_d(double *tjd_ut, int32 ifl, double *geopos, double *attr, char *serr);

/* computes attributes of a solar eclipse for given tjd, geolon, geolat */
extern EXP32 int32 CALL_CONV EXP16 swe_sol_eclipse_how_d(double *tjd_ut, int32 ifl, double *geopos, double *attr, char *serr);
/* finds time of next local eclipse */

extern EXP32 int32 CALL_CONV EXP16 swe_sol_eclipse_when_loc_d(double *tjd_start, int32 ifl, double *geopos, double *tret, double *attr, AS_BOOL backward, char *serr);

/* finds time of next eclipse globally */
extern EXP32 int32 CALL_CONV EXP16 swe_sol_eclipse_when_glob_d(double *tjd_start, int32 ifl, int32 ifltype,
     double *tret, AS_BOOL backward, char *serr);

/* computes attributes of a lunar eclipse for given tjd */
extern EXP32 int32 CALL_CONV EXP16 swe_lun_eclipse_how_d(
          double *tjd_ut, 
          int32 ifl,
		  double *geopos,
          double *attr, 
          char *serr);
extern EXP32 int32 CALL_CONV EXP16 swe_lun_eclipse_when_d(double *tjd_start, int32 ifl, int32 ifltype,
     double *tret, AS_BOOL backward, char *serr);

/* planetary phenomena */
extern EXP32 int32 CALL_CONV EXP16 swe_pheno_d(double *tjd, int32 ipl, int32 iflag,
     double *attr, char *serr);

extern EXP32 int32 CALL_CONV EXP16 swe_pheno_ut_d(double *tjd_ut, int32 ipl, int32 iflag, double *attr, char *serr);

extern EXP32 int CALL_CONV EXP16 swe_refrac_d(double *inalt, double *atpress, double *attemp, int32 calc_flag, double *retalt);

extern EXP32 int CALL_CONV EXP16 swe_azalt_d(
      double *tjd_ut,
      int32 calc_flag,
      double *geopos,
      double *atpress,
      double *attemp,
      double *xin, 
      double *xaz); 

extern EXP32 int CALL_CONV EXP16 swe_azalt_rev_d(
      double *tjd_ut,
      int32 calc_flag,
      double *geopos,
      double *xin, 
      double *xout); 

extern EXP32 int32 CALL_CONV EXP16 swe_rise_trans_d(
               double *tjd_ut, int32 ipl, char *starname, 
	       int32 epheflag, int32 rsmi,
               double *geopos, 
	       double *atpress, double *attemp,
               double *tret,
               char *serr);

extern EXP32 int32 CALL_CONV EXP16 swe_nod_aps_d(double *tjd_et, int32 ipl, int32 iflag, 
                      int32  method,
                      double *xnasc, double *xndsc, 
                      double *xperi, double *xaphe, 
                      char *serr);

extern EXP32 int32 CALL_CONV EXP16 swe_nod_aps_ut_d(double *tjd_ut, int32 ipl, int32 iflag, 
                      int32  method,
                      double *xnasc, double *xndsc, 
                      double *xperi, double *xaphe, 
                      char *serr);


/**************************/
/* modified DLL functions */
/**************************/

/* planets, moon, nodes etc. */
long CALL_CONV swe_calc_d(
    double *tjd, int ipl, long iflag,
    double *xx,
    char *serr) 
{
  return swe_calc(*tjd, ipl, iflag, xx, serr);
}

/* planets, moon, nodes etc. */
long CALL_CONV swe_calc_ut_d(
    double *tjd_ut, int ipl, long iflag,
    double *xx,
    char *serr) 
{
  return swe_calc(*tjd_ut, ipl, iflag, xx, serr);
}

/* fixed stars */
long CALL_CONV swe_fixstar_d(
    char *star, double *tjd, long iflag,
    double *xx, char *serr) 
{
  return swe_fixstar(star, *tjd, iflag, xx, serr);
}

/* fixed stars */
long CALL_CONV swe_fixstar_ut_d(
    char *star, double *tjd_ut, long iflag,
    double *xx, char *serr) 
{
  return swe_fixstar(star, *tjd_ut, iflag, xx, serr);
}

/* close Swiss Ephemeris */
int CALL_CONV swe_close_d(int ivoid) 
{
  swe_close();
  return OK;
}

/* set directory path of ephemeris files */
int CALL_CONV swe_set_ephe_path_d(char *path) 
{
  swe_set_ephe_path(path);
  return OK;
}

/* set file name of JPL file */
int CALL_CONV swe_set_jpl_file_d(char *fname) 
{
  swe_set_jpl_file(fname);
  return OK;
}

/* get planet name */
char *CALL_CONV swe_get_planet_name_d(int ipl, char *spname) 
{
  return swe_get_planet_name(ipl, spname);
}

/* get planet name */
int CALL_CONV swe_set_sid_mode_d(long sid_mode, double *t0, double *ayan_t0) 
{
  swe_set_sid_mode(sid_mode, *t0, *ayan_t0);
  return OK;
}

/* get planet name */
int CALL_CONV swe_get_ayanamsa_d(double *tjd_et, double *ayan) 
{
  *ayan = swe_get_ayanamsa(*tjd_et);
  return OK;
}

/* get planet name */
int CALL_CONV swe_get_ayanamsa_ut_d(double *tjd_ut, double *ayan) 
{
  *ayan = swe_get_ayanamsa(*tjd_ut);
  return OK;
}

/* delta t */
int CALL_CONV swe_deltat_d(double *tjd, double *deltat) 
{
  *deltat = swe_deltat(*tjd);
  return OK;
}

/* sidereal time */
int CALL_CONV swe_sidtime0_d(double *tjd, double *eps,
      double *nut, double *sidt) 
{
  *sidt = swe_sidtime0(*tjd, *eps, *nut);
  return OK;
}

/* sidereal time */
int CALL_CONV swe_sidtime_d(double *tjd, double *sidt) 
{
  *sidt = swe_sidtime(*tjd);
  return OK;
}

/* set geographic location for topocentric planet calculation */
int CALL_CONV swe_set_topo_d(double *geolon, double *geolat, double *height)
{
  swe_set_topo(*geolon, *geolat, *height);
  return OK;
}

/* coordinate transformation polar -> polar */
int CALL_CONV swe_cotrans_d(double *xpo, double *xpn, double *eps) 
{
  swe_cotrans(xpo, xpn, *eps);
  return OK;
}

int CALL_CONV swe_cotrans_sp_d(double *xpo, double *xpn, double *eps) 
{
  swe_cotrans_sp(xpo, xpn, *eps);
  return OK;
}

/* tidal acceleration to be used in swe_deltat() */
int CALL_CONV swe_get_tid_acc_d(double *t_acc) 
{
  *t_acc = swe_get_tid_acc();
  return OK;
}

int CALL_CONV swe_set_tid_acc_d(double *t_acc) 
{
  swe_set_tid_acc(*t_acc);
  return OK;
}

int CALL_CONV swe_degnorm_d(double *x) 
{
  *x = swe_degnorm(*x);
  return OK;
}

int CALL_CONV swe_date_conversion_d(
        int y , int m , int d ,         /* year, month, day */
        double *utime,   /* universal time in hours (decimal) */
        char *c,         /* calendar g[regorian]|j[ulian]|a[stro = greg] */
        double *tjd) 
{
  return swe_date_conversion(y, m, d, *utime, *c, tjd);
}

int CALL_CONV swe_julday_d(
        int year, int month, int day, double *hour,
        int gregflag, double *tjd) 
{
  *tjd = swe_julday(year, month, day, *hour, gregflag);
  return OK;
}

int CALL_CONV swe_revjul_d(
        double *tjd,
        int gregflag,
        int *jyear, int *jmon, int *jday, double *jut) 
{
  swe_revjul(*tjd, gregflag, jyear, jmon, jday, jut);
  return OK;
  
}

int CALL_CONV swe_time_equ_d(
        double *tjd,
        double *e,
        char *serr)
{
  return swe_time_equ(*tjd, e, serr);
}  

int CALL_CONV swe_houses_d(
        double *tjd_ut, double *geolat, double *geolon, int hsys, 
        double *cusps, double *ascmc)
{
  return swe_houses(*tjd_ut, *geolat, *geolon, hsys, cusps, ascmc);
}

int CALL_CONV swe_houses_ex_d(
        double *tjd_ut, int32 iflag, double *geolat, double *geolon, int hsys, 
        double *cusps, double *ascmc)
{
  return swe_houses_ex(*tjd_ut, iflag, *geolat, *geolon, hsys, cusps, ascmc);
}

int CALL_CONV swe_houses_armc_d(
        double *armc, double *geolat, double *eps, int hsys, 
        double *cusps, double *ascmc)
{
  return swe_houses_armc(*armc, *geolat, *eps, hsys, cusps, ascmc);
}

extern EXP32 int CALL_CONV EXP16 swe_house_pos_d(
        double *armc, double *geolat, double *eps, int hsys, double *xpin,
        double *hpos, char *serr)
{
  *hpos = swe_house_pos(*armc, *geolat, *eps, hsys, xpin, serr);
  if (*hpos == 0)
    return ERR;
  else
    return OK;
}

/* normalize argument into interval [0..DEG360] */
centisec CALL_CONV swe_csnorm_d(centisec p) 
{
  return swe_csnorm(p);
}

/* distance in centisecs p1 - p2 normalized to [0..360[ */
centisec CALL_CONV swe_difcsn_d(centisec p1, centisec p2)
{
  return swe_difcsn(p1, p2);
}

int CALL_CONV swe_difdegn_d(double *p1, double *p2, double *diff)
{
  *diff = swe_difdegn(*p1, *p2);
  return OK;
}

/* distance in centisecs p1 - p2 normalized to [-180..180[ */
centisec CALL_CONV swe_difcs2n_d(centisec p1, centisec p2)
{
  return swe_difcs2n(p1, p2);
}

int CALL_CONV swe_difdeg2n_d(double *p1, double *p2, double *diff)
{
  *diff = swe_difdeg2n(*p1, *p2);
  return OK;
}

/* round second, but at 29.5959 always down */
centisec CALL_CONV swe_csroundsec_d(centisec x)
{
  return swe_csroundsec(x);
}

/* double to long with rounding, no overflow check */
long CALL_CONV swe_d2l_d(double *x)
{
  return swe_d2l(*x);
}

int CALL_CONV swe_split_deg_d(double *ddeg, int32 roundflag, int32 *ideg, int32 *imin, int32 *isec, double *dsecfr, int32 *isgn)
{
  swe_split_deg(*ddeg, roundflag, ideg, imin, isec, dsecfr, isgn);
  return 0;
}

/* monday = 0, ... sunday = 6 */
int CALL_CONV swe_day_of_week_d(double *jd)
{
  return swe_day_of_week(*jd);
}

char *CALL_CONV swe_cs2timestr_d(CSEC t, int sep, AS_BOOL suppressZero, char *a)
{
  return swe_cs2timestr(t, sep, suppressZero, a);
}

char *CALL_CONV swe_cs2lonlatstr_d(CSEC t, char *pchar, char *mchar, char *s)
{
  return swe_cs2lonlatstr(t, *pchar, *mchar, s);
}

char *CALL_CONV swe_cs2degstr_d(CSEC t, char *a)
{
  return swe_cs2degstr(t, a);
}


/* computes geographic location and attributes of solar 
 * eclipse at a given tjd */
int32 CALL_CONV swe_sol_eclipse_where_d(double *tjd_ut, int32 ifl, double *geopos, double *attr, char *serr)
{
  return swe_sol_eclipse_where(*tjd_ut, ifl, geopos, attr, serr);
}

/* computes attributes of a solar eclipse for given tjd, geolon, geolat */
int32 CALL_CONV swe_sol_eclipse_how_d(double *tjd_ut, int32 ifl, double *geopos, double *attr, char *serr)
{
  return swe_sol_eclipse_how(*tjd_ut, ifl, geopos, attr, serr);
}
/* finds time of next local eclipse */
int32 CALL_CONV swe_sol_eclipse_when_loc_d(double *tjd_start, int32 ifl, double *geopos, double *tret, double *attr, AS_BOOL backward, char *serr)
{
  return swe_sol_eclipse_when_loc(*tjd_start, ifl, geopos, tret, attr, backward, serr);
}

/* finds time of next eclipse globally */
int32 CALL_CONV swe_sol_eclipse_when_glob_d(double *tjd_start, int32 ifl, int32 ifltype,
     double *tret, AS_BOOL backward, char *serr)
{
  return swe_sol_eclipse_when_glob(*tjd_start, ifl, ifltype, tret, backward, serr);
}

/* computes attributes of a lunar eclipse for given tjd */
int32 CALL_CONV swe_lun_eclipse_how_d(
          double *tjd_ut, 
          int32 ifl,
		  double *geopos,
          double *attr, 
          char *serr)
{
  return swe_lun_eclipse_how(*tjd_ut, ifl, geopos, attr, serr);
}

int32 CALL_CONV swe_lun_eclipse_when_d(double *tjd_start, int32 ifl, int32 ifltype,
     double *tret, AS_BOOL backward, char *serr)
{
  return swe_lun_eclipse_when(*tjd_start, ifl, ifltype, tret, backward, serr);
}

int32 CALL_CONV swe_pheno_d(double *tjd, int32 ipl, int32 iflag,
     double *attr, char *serr)
{
  return swe_pheno(*tjd, ipl, iflag, attr, serr);
}

int32 CALL_CONV swe_pheno_ut_d(double *tjd_ut, int32 ipl, int32 iflag, double *attr, char *serr)
{
  return swe_pheno_ut(*tjd_ut, ipl, iflag, attr, serr);
}

int CALL_CONV swe_refrac_d(double *inalt, double *atpress, double *attemp, int32 calc_flag, double *retalt)
{
  *retalt = swe_refrac(*inalt, *atpress, *attemp, calc_flag);
  return OK;
}

int CALL_CONV swe_azalt_d(
      double *tjd_ut,
      int32 calc_flag,
      double *geopos,
      double *atpress,
      double *attemp,
      double *xin, 
      double *xaz)
{
  swe_azalt(*tjd_ut, calc_flag, geopos, *atpress, *attemp, xin, xaz);
  return OK;
}

int CALL_CONV swe_azalt_rev_d(
      double *tjd_ut,
      int32 calc_flag,
      double *geopos,
      double *xin, 
      double *xout)
{
  swe_azalt_rev(*tjd_ut, calc_flag, geopos, xin, xout);
  return OK;
}

int32 CALL_CONV swe_rise_trans_d(
               double *tjd_ut, int32 ipl, char *starname, 
	       int32 epheflag, int32 rsmi,
               double *geopos, 
	       double *atpress, double *attemp,
               double *tret,
               char *serr)
{
  return swe_rise_trans(*tjd_ut, ipl, starname, epheflag, rsmi,
             geopos, *atpress, *attemp, tret, serr);
}

int32 CALL_CONV swe_nod_aps_d(double *tjd_et, int32 ipl, int32 iflag, 
                      int32  method,
                      double *xnasc, double *xndsc, 
                      double *xperi, double *xaphe, 
                      char *serr)
{
  return swe_nod_aps(*tjd_et, ipl, iflag, method, xnasc, xndsc, xperi, xaphe, serr);
}

int32 CALL_CONV swe_nod_aps_ut_d(double *tjd_ut, int32 ipl, int32 iflag, 
                      int32  method,
                      double *xnasc, double *xndsc, 
                      double *xperi, double *xaphe, 
                      char *serr)
{
  return swe_nod_aps(*tjd_ut, ipl, iflag, method, xnasc, xndsc, xperi, xaphe, serr);
}
