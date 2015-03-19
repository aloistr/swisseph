/* SWISSEPH 
 $Header: /home/dieter/sweph/RCS/swehel.c,v 1.1 2009/04/21 06:05:59 dieter Exp dieter $

  Heliacal risings and related calculations
  
  Author: Victor Reijs
  This program code is a translation of part of:
  Victor Reijs' software ARCHAEOCOSMO (archaeoastronomy and
  geodesy functions), 
  http://www.iol.ie/~geniet/eng/archaeocosmoprocedures.htm

  Translation from VB into C by Dieter Koch

  Problem reports can be sent to victor.reijs@gmail.com or dieter@astro.ch
  
  Copyright (c) Victor Reijs, 2008

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

  The authors of Swiss Ephemeris have no control or influence over any of
  the derived works, i.e. over software or services created by other
  programmers which use Swiss Ephemeris functions.

  The names of the authors or of the copyright holder must not
  be used for promoting any software, product or service which uses or contains
  the Swiss Ephemeris. This copyright notice is the ONLY place where the
  names of the authors can legally appear, except in cases where they have
  given special permission in writing.

  The trademarks 'Swiss Ephemeris' and 'Swiss Ephemeris inside' may be used
  for promoting such software, products or services.
*/

#include "swephexp.h"
#include "sweph.h"
#include "swephlib.h"
#include <sys/stat.h>

#define PLSV   0 /*if Planet, Lunar and Stellar Visibility formula is needed PLSV=1*/
#define criticalangle   0.0 /*[deg]*/
#define BNIGHT   	1479.0 /*[nL]*/
#define BNIGHT_FACTOR   1.0
#define PI		M_PI
#define Min2Deg   (1.0 / 60.0)
#define DEBUG  0
#define DONE  1
#define MaxTryHours   4 
#define TimeStepDefault	1
#define LocalMinStep	8

/* time constants */
#define Y2D   365.25 /*[Day]*/
#define D2Y   (1 / Y2D) /*[Year]*/
#define D2H   24.0 /*[Hour]*/
#define H2S   3600.0 /*[sec]*/
#define D2S   (D2H * H2S) /*[sec]*/
#define S2H   (1.0 / H2S) /*[Hour]*/
#define JC2D   36525.0 /*[Day]*/
#define M2S   60.0 /*[sec]*/

/* Determines which algorimths are used*/
#define USE_DELTA_T_VR 	 0
#define REFR_SINCLAIR    0
#define REFR_BENNETTH    1
#define FormAstroRefrac   REFR_SINCLAIR /*for Astronomical refraction can be "bennetth" or "sinclair"*/
#define GravitySource   2 /*0=RGO, 1=Wikipedia,2=Exp. Suppl. 1992,3=van der Werf*/
#define REarthSource   1 /*0=RGO (constant), 1=WGS84 method*/

#define StartYear   1820 /*[year]*/
#define Average   1.80546834626888 /*[msec/cy]*/
#define Periodicy   1443.67123144531 /*[year]*/
#define Amplitude   3.75606495492684 /*[msec]*/
#define phase   0 /*[deg]*/
#define MAX_COUNT_SYNPER           5  /* search within 10 synodic periods */
#define MAX_COUNT_SYNPER_MAX 1000000  /* high, so there is not max count */
#define AvgRadiusMoon  (15.541 / 60) /* '[Deg] at 2007 CE or BCE*/

/* WGS84 ellipsoid constants
 * http://w3sli.wcape.gov.za/Surveys/Mapping/wgs84.htm*/
#define Ra   6378136.6   /*'[m]*/
#define Rb   6356752.314 /*'[m]*/

/* choices in Schaefer's model */
#define nL2erg 	(1.02E-15)
#define erg2nL 	(1 / nL2erg) /*erg2nL to nLambert*/
#define MoonDistance 	384410.4978 /*[km]*/
#define scaleHwater 	3000.0 /*[m] Ricchiazzi [1997] 8200 Schaefer [2000]*/
#define scaleHrayleigh 	8515.0 /*[m] Su [2003] 8200 Schaefer [2000]*/
#define scaleHaerosol 	3745.0 /*m Su [2003] 1500 Schaefer [2000]*/
#define scaleHozone 	20000.0 /*[m] Schaefer [2000]*/
#define astr2tau 	0.921034037197618  /*LN(10 ^ 0.4)*/
#define tau2astr 	1 / astr2tau

/* meteorological constants*/
#define C2K   273.15 /*[K]*/
#define DELTA   18.36
#define TempNulDiff   0.000001
#define PressRef   1000 /*[mbar]*/
#define MD   28.964 /*[kg] Mol weight of dry air van der Werf*/
#define MW   18.016 /*[kg] Mol weight of water vapor*/
#define GCR   8314.472 /*[L/kmol/K] van der Werf*/
#define LapseSA   0.0065 /*[K/m] standard atmosphere*/
#define LapseDA   0.0098 /*[K/m] dry adiabatic*/

/* lowest apparent altitude to provide*/
#define LowestAppAlt   -3.5 /*[Deg]*/

/*optimization delta*/
#define epsilon   0.001
/* for Airmass usage*/
#define staticAirmass   0 /* use staticAirmass=1 instead depending on difference k's*/

/* optic stuff */
#define GOpticMag   1 /*telescope magnification*/
#define GOpticTrans   0.8 /*telescope transmission*/
#define GBinocular   1 /*1-binocular 0=monocular*/
#define GOpticDia   50 /*telescope diameter [mm]*/

static double mymin(double a, double b)
{
  if (a <= b)
    return a;
  return b;
}

static double mymax(double a, double b)
{
  if (a >= b)
    return a;
  return b;
}

/*###################################################################*/
static double Tanh(double x)
{
  return (exp(x) - exp(-x)) / (exp(x) + exp(-x));
}

/*
' B [nL]
' SN [-]
' CVA [deg]
*/
static double CVA(double B, double SN)
{
  /*Schaefer, Astronomy and the limits of vision, Archaeoastronomy, 1993*/
  if (B > BNIGHT)
    return (40.0 / SN) * pow(10, (8.28 * pow(B, (-0.29)))) / 60.0 / 60.0;
  else
    return mymin(900, 380 / SN * pow(10, (0.3 * pow(B, (-0.29))))) / 60.0 / 60.0;
}

/*
' age [year]
' B [nL]
' PupilDia [mm]
*/
static double PupilDia(double Age, double B)
{
  /* age dependancy from Garstang [2000]*/
  return (0.534 - 0.00211 * Age - (0.236 - 0.00127 * Age) * Tanh(0.4 * log(B) / log(10) - 2.2)) * 10;
}

/*
'Input
' Bback [nL]
' kX [-]
' Binocular [-]
' OpticMag [-]
' OpticDia [mm]
' OpticTrans [-]
' JDNDaysUT [JDN]
' Age [Year]
' SN [-]
' ObjectName
' TypeFactor [0=itensity factor 1=background factor]
'Output
' OpticFactor [-]
*/
static double OpticFactor(double Bback, double kX, double *dobs, double JDNDaysUT, char *ObjectName, int TypeFactor, int helflag)
{
  double Pst, CIb, CIi, ObjectSize, Fb, Fe, Fsc, Fci, Fcb, Ft, Fp, Fa, Fr, Fm;
  double Age = dobs[0];
  double SN = dobs[1], SNi;
  double Binocular = dobs[2];
  double OpticMag = dobs[3];
  double OpticDia = dobs[4];
  double OpticTrans = dobs[5];
  SNi = SN;
  if (SNi <= 0.00000001) SNi = 0.00000001;
  /* 23 jaar as standard from Garstang*/
  Pst = PupilDia(23, Bback);
  if (OpticMag == 1) { /*OpticMagn=1 means using eye*/
     OpticTrans = 1;
     OpticDia = Pst;
  }
#if 0 /*is done in default_heliacal_parameters()*/
  if (OpticMag == 0) { /*OpticMagn=0 (undefined) using eye*/
    OpticTrans = 1;
    OpticDia = Pst;
    Binocular = 1;
    OpticMag = 1;
  }
#endif
  /* Schaefer, Astronomy and the limits of vision, Archaeoastronomy, 1993*/
  CIb = 0.7; /* color of background (from Ben Sugerman)*/
  CIi = 0.5; /* Color index for white (from Ben Sugerman), should be function of ObjectName*/
  ObjectSize = 0;
  if (strcmp(ObjectName, "moon") == 0) {
    /*ObjectSize and CI needs to be determined (depending on JDNDaysUT)*/
    ;
  }
  Fb = 1;
  if (Binocular == 0) Fb = 1.41;
  if (Bback < BNIGHT && !(helflag & SE_HELFLAG_VISLIM_PHOTOPIC)) {
    Fe = pow(10, (0.48 * kX));
    Fsc = mymin(1, (1 - pow(Pst / 124.4, 4)) / (1 - pow((OpticDia / OpticMag / 124.4), 4)));
    Fci = pow(10, (-0.4 * (1 - CIi / 2.0)));
    Fcb = pow(10, (-0.4 * (1 - CIb / 2.0)));
  } else {
     Fe = pow(10, (0.4 * kX));
     Fsc = mymin(1, pow((OpticDia / OpticMag / Pst), 2) * (1 - exp(-pow((Pst / 6.2), 2))) / (1 - exp(-pow((OpticDia / OpticMag / 6.2), 2))));
     Fci = 1;
     Fcb = 1;
  }
  Ft = 1 / OpticTrans;
  Fp = mymax(1, pow((Pst / (OpticMag * PupilDia(Age, Bback))), 2));
  Fa = pow((Pst / OpticDia), 2);
  Fr = (1 + 0.03 * pow((OpticMag * ObjectSize / CVA(Bback, SNi)), 2)) / pow(SNi, 2);
  Fm = pow(OpticMag, 2);
#if DEBUG
  fprintf(stderr, "Pst=%f\n", Pst);
  fprintf(stderr, "Fb =%f\n", Fb);
  fprintf(stderr, "Fe =%f\n", Fe);
  fprintf(stderr, "Ft =%f\n", Ft);
  fprintf(stderr, "Fp =%f\n", Fp);
  fprintf(stderr, "Fa =%f\n", Fa);
  fprintf(stderr, "Fm =%f\n", Fm);
  fprintf(stderr, "Fsc=%f\n", Fsc);
  fprintf(stderr, "Fci=%f\n", Fci);
  fprintf(stderr, "Fcb=%f\n", Fcb);
  fprintf(stderr, "Fr =%f\n", Fr );
#endif
  if (TypeFactor == 0) 
    return Fb * Fe * Ft * Fp * Fa * Fr * Fsc * Fci;
  else
    return Fb * Ft * Fp * Fa * Fm * Fsc * Fcb;
}

/*###################################################################
*/
static int32 DeterObject(char *ObjectName)
{
  char s[AS_MAXCH];
  char *sp;
  int32 ipl;
  strcpy(s, ObjectName);
  for (sp = s; *sp != '\0'; sp++)
    *sp = tolower(*sp);
  if (strncmp(s, "sun", 3) == 0)
    return SE_SUN;
  if (strncmp(s, "venus", 5) == 0)
    return SE_VENUS;
  if (strncmp(s, "mars", 4) == 0)
    return SE_MARS;
  if (strncmp(s, "mercur", 6) == 0)
    return SE_MERCURY;
  if (strncmp(s, "jupiter", 7) == 0)
    return SE_JUPITER;
  if (strncmp(s, "saturn", 6) == 0)
    return SE_SATURN;
  if (strncmp(s, "uranus", 6) == 0)
    return SE_URANUS;
  if (strncmp(s, "neptun", 6) == 0)
    return SE_NEPTUNE;
  if (strncmp(s, "moon", 4) == 0)
    return SE_MOON;
  if ((ipl = atoi(s)) > 0) {
    ipl += SE_AST_OFFSET;
    return ipl;
  }
  return -1;
}

#if 0
int32 call_swe_calc(double tjd, int32 ipl, int32 iflag, double *x, char *serr) 
{
  int32 retval = OK, ipli, i;
  double dtjd;
  static double tjdsv[3];
  static double xsv[3][6];
  static int32 iflagsv[3];
  ipli = ipl;
  if (ipli > SE_MOON) 
    ipli = 2;
  dtjd = tjd - tjdsv[ipli];
  if (tjdsv[ipli] != 0 && iflag == iflagsv[ipli] && fabs(dtjd) < 5.0 / 1440.0) {
    for (i = 0; i < 3; i++) 
      x[i] = xsv[ipli][i] + dtjd * xsv[ipli][i+3];
    for (i = 3; i < 6; i++) 
      x[i] = xsv[ipli][i];
  } else {
    retval = swe_calc(tjd, ipl, iflag, x, serr);
    tjdsv[ipli] = tjd;
    iflagsv[ipli] = iflag;
    for (i = 0; i < 6; i++) 
      xsv[ipli][i] = x[i];
  }
  return retval;
}
#endif

/* avoids problems with star name string that may be overwritten by 
   swe_fixstar() */
int32 call_swe_fixstar(char *star, double tjd, int32 iflag, double *xx, char *serr)
{
  int32 retval;
  char star2[AS_MAXCH];
  strcpy(star2, star);
  retval =  swe_fixstar(star2, tjd, iflag, xx, serr);
  return retval;
}

/* avoids problems with star name string that may be overwritten by 
   swe_fixstar_mag() */
int32 call_swe_fixstar_mag(char *star, double *mag, char *serr)
{
  int32 retval;
  char star2[AS_MAXCH];
  static double dmag;
  static char star_save[AS_MAXCH];
  if (strcmp(star, star_save) == 0) {
    *mag = dmag;
    return OK;
  }
  strcpy(star_save, star);
  strcpy(star2, star);
  retval = swe_fixstar_mag(star2, &dmag, serr);
  *mag = dmag;
  return retval;
}

/* avoids problems with star name string that may be overwritten by 
   swe_fixstar() */
int32 call_swe_rise_trans(double tjd, int32 ipl, char *star, int32 helflag, int32 eventtype, double *dgeo, double atpress, double attemp, double *tret, char *serr)
{
  int32 retval;
  int32 iflag = helflag & (SEFLG_JPLEPH|SEFLG_SWIEPH|SEFLG_MOSEPH);
  char star2[AS_MAXCH];
  strcpy(star2, star);
  retval = swe_rise_trans(tjd, ipl, star2, iflag, eventtype, dgeo, atpress, attemp, tret, serr);
  return retval;
}

/* 
 * Written by Dieter Koch:
 * Fast function for risings and settings of planets, can be used instead of 
 * swe_rise_trans(), which is much slower.
 * For circumpolar and near-circumpolar planets use swe_rise_trans(), or 
 * generally use it for geographical latitudes higher than 58N/S.
 * For fixed stars, swe_rise_trans() is fast enough.
 */
static int32 calc_rise_and_set(double tjd_start, int32 ipl, double *dgeo, double *datm, int32 eventflag, int32 helflag, double *trise, char *serr)
{
  int retc = OK, i;
  double sda, xs[6], xx[6], xaz[6], xaz2[6], dfac = 1/365.25;
  double rdi, rh;
  double tjd0 = tjd_start, tjdrise;
  double tjdnoon = (int) tjd0 - dgeo[0] / 15.0 / 24.0;
  int32 iflag = helflag & (SEFLG_JPLEPH|SEFLG_SWIEPH|SEFLG_MOSEPH);
  int32 epheflag = iflag;
  iflag |= SEFLG_EQUATORIAL;
  if (!(helflag & SE_HELFLAG_HIGH_PRECISION)) 
    iflag |= SEFLG_NONUT|SEFLG_TRUEPOS;
  if (swe_calc_ut(tjd0, SE_SUN, iflag, xs, serr) == 0) {
    if (serr != NULL)
      strcpy(serr, "error in calc_rise_and_set(): calc(sun) failed ");
    return ERR;
  }
  if (swe_calc_ut(tjd0, ipl, iflag, xx, serr) == 0) {
    if (serr != NULL)
      strcpy(serr, "error in calc_rise_and_set(): calc(sun) failed ");
    return ERR;
  }
  tjdnoon -= swe_degnorm(xs[0] - xx[0])/360.0 + 0;
  /* is planet above horizon or below? */
  swe_azalt(tjd0, SE_EQU2HOR, dgeo, datm[0], datm[1], xx, xaz);
if (eventflag & SE_CALC_RISE) {
  if (xaz[2] > 0) {
    while (tjdnoon - tjd0 < 0.5) {/*printf("e");*/tjdnoon += 1;}
    while (tjdnoon - tjd0 > 1.5) {/*printf("f");*/tjdnoon -= 1;}
  } else {
    while (tjdnoon - tjd0 < 0.0) {/*printf("g");*/tjdnoon += 1;}
    while (tjdnoon - tjd0 > 1.0) {/*printf("h");*/tjdnoon -= 1;}
  }
} else {
  if (xaz[2] > 0) {
    while (tjd0 - tjdnoon > 0.5) {/*printf("a");*/ tjdnoon += 1;}
    while (tjd0 - tjdnoon < -0.5) {/*printf("b");*/ tjdnoon -= 1;}
  } else {
    while (tjd0 - tjdnoon > 0.0) {/*printf("c");*/ tjdnoon += 1;}
    while (tjd0 - tjdnoon < -1.0) {/*printf("d");*/ tjdnoon -= 1;}
  }
}
  /* position of planet */
  if (swe_calc_ut(tjdnoon, ipl, iflag, xx, serr) == ERR) {
    if (serr != NULL)
      strcpy(serr, "error in calc_rise_and_set(): calc(sun) failed ");
    return ERR;
  }
  /* apparent radius of solar disk (ignoring refraction) */
  rdi = asin(696000000.0 / 1.49597870691e+11 / xx[2]) / DEGTORAD;
  if (eventflag & SE_BIT_DISC_CENTER)
    rdi = 0;
  /* true altitude of sun, when it appears at the horizon */
  /* refraction for a body visible at the horizon at 0m above sea,
   * atmospheric temperature 10 deg C, atmospheric pressure 1013.25 is 34.5 arcmin*/
  rh = -(34.5 / 60.0 + rdi);
  /* semidiurnal arc of sun */
  sda = acos(-tan(dgeo[1] * DEGTORAD) * tan(xx[1] * DEGTORAD)) * RADTODEG;
  /* rough rising and setting times */
if (eventflag & SE_CALC_RISE)
  tjdrise = tjdnoon - sda / 360.0;
else
  tjdrise = tjdnoon + sda / 360.0;
  /*ph->tset = tjd_start + sda / 360.0;*/
  /* now calculate more accurate rising and setting times.
   * use vertical speed in order to determine crossing of the horizon  
   * refraction of 34' and solar disk diameter of 16' = 50' = 0.84 deg */
  iflag = epheflag|SEFLG_SPEED|SEFLG_EQUATORIAL;
  if (ipl == SE_MOON)
    iflag |= SEFLG_TOPOCTR;
  if (!(helflag & SE_HELFLAG_HIGH_PRECISION)) 
    iflag |= SEFLG_NONUT|SEFLG_TRUEPOS;
  for (i = 0; i < 2; i++) {
    if (swe_calc_ut(tjdrise, ipl, iflag, xx, serr) == ERR) {
      /*fprintf(stderr, "hev4 tjd=%f, ipl=%d, iflag=%d\n", tjdrise, ipl, iflag);*/
      return ERR;
    }
    swe_azalt(tjdrise, SE_EQU2HOR, dgeo, datm[0], datm[1], xx, xaz);
    xx[0] -= xx[3] * dfac; 
    xx[1] -= xx[4] * dfac;
    swe_azalt(tjdrise - dfac, SE_EQU2HOR, dgeo, datm[0], datm[1], xx, xaz2);
    tjdrise -= (xaz[1] - rh) / (xaz[1] - xaz2[1]) *  dfac;
    /*fprintf(stderr, "%f\n", ph->trise);*/
  }
  *trise = tjdrise;
  return retc;
}

static int32 my_rise_trans(double tjd, int32 ipl, char* starname, int32 eventtype, int32 helflag, double *dgeo, double *datm, double *tret, char *serr)
{
  int retc = OK;
  if (starname != NULL && *starname != '\0')
    ipl = DeterObject(starname);
  /* for non-circumpolar planets we can use a faster algorithm */
  /*if (!(helflag & SE_HELFLAG_HIGH_PRECISION) && ipl != -1 && fabs(dgeo[1]) < 58) {*/
  if (ipl != -1 && fabs(dgeo[1]) < 63) {
    retc = calc_rise_and_set(tjd, ipl, dgeo, datm, eventtype, helflag, tret, serr);
  /* for stars and circumpolar planets we use a rigorous algorithm */
  } else {
    retc = call_swe_rise_trans(tjd, ipl, starname, helflag, eventtype, dgeo, datm[0], datm[1], tret, serr);
  }
/*  printf("%f, %f\n", tjd, *tret);*/
  return retc;
}

/*###################################################################
' JDNDaysUT [Days]
' dgeo [array: longitude, latitude, eye height above sea m]
' TempE [C]
' PresE [mbar]
' ObjectName (string)
' RSEvent (1=rise, 2=set,3=up transit,4=down transit)
' Rim [0=center,1=top]
' RiseSet [Day]
*/
static int32 RiseSet(double JDNDaysUT, double *dgeo, double *datm, char *ObjectName, int32 RSEvent, int32 helflag, int32 Rim, double *tret, char *serr)
{
  int32 eventtype = RSEvent, Planet, retval;
  if (Rim == 0)
    eventtype |= SE_BIT_DISC_CENTER;
  Planet = DeterObject(ObjectName);
  if (Planet != -1)
    retval = my_rise_trans(JDNDaysUT, Planet, "", eventtype, helflag, dgeo, datm, tret, serr);
  else
    retval = my_rise_trans(JDNDaysUT, -1, ObjectName, eventtype, helflag, dgeo, datm, tret, serr);
  return retval;
}

/*###################################################################
' JDNDaysUT [Days]
' actual [0= approximation, 1=actual]
' SunRA [deg]
*/
static double SunRA(double JDNDaysUT, int32 helflag, char *serr)
{
  int imon, iday, iyar, calflag = SE_GREG_CAL;
  double dut;
  static double tjdlast;
  static double ralast;
  if (JDNDaysUT == tjdlast)
    return ralast;
#ifndef SIMULATE_VICTORVB
  if (1) { /*helflag & SE_HELFLAG_HIGH_PRECISION) {*/
    double tjd_tt;
    double x[6];
    int32 epheflag = helflag & (SEFLG_JPLEPH|SEFLG_SWIEPH|SEFLG_MOSEPH);
    int32 iflag = epheflag | SEFLG_EQUATORIAL;
    iflag |= SEFLG_NONUT | SEFLG_TRUEPOS;
    tjd_tt = JDNDaysUT + swe_deltat(JDNDaysUT);
    if (swe_calc(tjd_tt, SE_SUN, iflag, x, serr) != ERR) {
      ralast = x[0];
      tjdlast = JDNDaysUT;
      return ralast;
    }
  }
#endif
  swe_revjul(JDNDaysUT, calflag, &iyar, &imon, &iday, &dut); /* this seems to be much faster than calling swe_revjul() ! Note: only because SunRA is called 1000s of times */
  tjdlast = JDNDaysUT;
  ralast = swe_degnorm((imon + (iday - 1) / 30.4 - 3.69) * 30);
  /*ralast = (DatefromJDut(JDNDaysUT, 2) + (DatefromJDut(JDNDaysUT, 3) - 1) / 30.4 - 3.69) * 30;*/
  return ralast;
}

/*###################################################################
' Temp [C]
' Kelvin [K]
*/
static double Kelvin(double Temp)
{
  /*' http://en.wikipedia.org/wiki/Kelvin*/
  return Temp + C2K;
}

/*###################################################################
' AppAlt [deg]
' TempE [C]
' PresE [mbar]
' TopoAltitudefromAppAlt [deg]
*/
static double TopoAltfromAppAlt(double AppAlt, double TempE, double PresE)
{
  double R = 0;
  double retalt = 0;
  if (AppAlt >= LowestAppAlt) {
    if (AppAlt > 17.904104638432)
      R = 0.97 / tan(AppAlt * DEGTORAD);
    else
      R = (34.46 + 4.23 * AppAlt + 0.004 * AppAlt * AppAlt) / (1 + 0.505 * AppAlt + 0.0845 * AppAlt * AppAlt);
    R = (PresE - 80) / 930 / (1 + 0.00008 * (R + 39) * (TempE - 10)) * R;
    retalt = AppAlt - R * Min2Deg;
  } else {
    retalt = AppAlt;
  }
  return retalt;
}

/*###################################################################
' TopoAlt [deg]
' TempE [C]
' PresE [mbar]
' AppAltfromTopoAlt [deg]
' call this instead of swe_azalt(), because it is faster (lower precision
' is required)
*/
static double AppAltfromTopoAlt(double TopoAlt, double TempE, double PresE, int32 helflag)
{
  /* using methodology of Newtown derivatives (analogue to what Swiss Emphemeris uses)*/
  int i, nloop = 2;
  double newAppAlt = TopoAlt;
  double newTopoAlt = 0.0;
  double oudAppAlt = newAppAlt;
  double oudTopoAlt = newTopoAlt;
  double verschil, retalt;
  if (helflag & SE_HELFLAG_HIGH_PRECISION)
    nloop = 5;
  for (i = 0; i <= nloop; i++) {
    newTopoAlt = newAppAlt - TopoAltfromAppAlt(newAppAlt, TempE, PresE);
    /*newTopoAlt = newAppAlt - swe_refrac(newAppAlt, PresE, TempE, SE_CALC_APP_TO_TRUE);*/
    verschil = newAppAlt - oudAppAlt;
    oudAppAlt = newTopoAlt - oudTopoAlt - verschil;
    if ((verschil != 0) && (oudAppAlt != 0))
      verschil = newAppAlt - verschil * (TopoAlt + newTopoAlt - newAppAlt) / oudAppAlt;
    else
      verschil = TopoAlt + newTopoAlt;
    oudAppAlt = newAppAlt;
    oudTopoAlt = newTopoAlt;
    newAppAlt = verschil;
  }
  retalt = TopoAlt + newTopoAlt;
  if (retalt < LowestAppAlt)
    retalt = TopoAlt;
  return retalt;
}

/*###################################################################
' TopoAlt [deg]
' TopoDecl [deg]
' Lat [deg]
' HourAngle [hour]
*/
static double HourAngle(double TopoAlt, double TopoDecl, double Lat)
{
  double Alti = TopoAlt * DEGTORAD;
  double decli = TopoDecl * DEGTORAD;
  double Lati = Lat * DEGTORAD;
  double ha = (sin(Alti) - sin(Lati) * sin(decli)) / cos(Lati) / cos(decli);
  if (ha < -1) ha = -1; 
  if (ha > 1) ha = 1;
  /* from http://star-www.st-and.ac.uk/~fv/webnotes/chapt12.htm*/
  return acos(ha) / DEGTORAD / 15.0;
}

/*###################################################################
' JDNDays [Days]
' COD [msec/cy]
' DeltaTSE [Sec]
*/
static double DeltaTSE(double JDNDays, int COD)
{
  double OffSetYear;
  int gregflag = SE_GREG_CAL;
  if (StartYear < 1583)
    gregflag = SE_JUL_CAL;
  /* from Swiss Emphemeris */
  if (COD != 0) {
    /* Determined by V. Reijs*/
    OffSetYear = (swe_julday((int) StartYear, 1, 1, 0, gregflag) - JDNDays) / 365.25;
    return (OffSetYear * OffSetYear / 100.0 / 2.0 * COD * Y2D) / 1000.0;
  }
  return swe_deltat(JDNDays) * D2S;
}

/*###################################################################
' JDNDays [Day]
' COD [msec/cy]
' DeltaTVR [Sec]
*/
static double DeltaTVR(double JDNDays, int COD)
{
  /* Determined by V. Reijs */
  double DeltaTVR;
  int gregflag = SE_GREG_CAL;
  double OffSetYear;
  if (StartYear < 1583)
    gregflag = SE_JUL_CAL;
  OffSetYear = (swe_julday((int) StartYear, 1, 1, 0, gregflag) - JDNDays) / 365.25;
  if (COD == 0) {
    DeltaTVR = (OffSetYear * OffSetYear / 100.0 / 2.0 * Average + Periodicy / 2.0 / PI * Amplitude * (cos((2 * PI * OffSetYear / Periodicy)) - 1)) * Y2D;
  } else {
    DeltaTVR = OffSetYear * OffSetYear / 100.0 / 2.0 * COD * Y2D;
  }
  return DeltaTVR / 1000.0;
}

/*###################################################################
' JDNDays [Days]
' COD [msec/cy]
' DeltaT [Sec]
*/
static double DeltaT(double JDNDays, int COD)
{
  if (USE_DELTA_T_VR)
    return DeltaTVR(JDNDays, COD);
  return DeltaTSE(JDNDays, COD);
}

/*###################################################################
' JDNDaysUT [Days]
' dgeo [array: longitude, latitude, eye height above sea m]
' TempE [C]
' PresE [mbar]
' ObjectName [-]
' Angle (0 = TopoAlt, 1 = Azi, 2=Topo Declination, 3=Topo Rectascension, 4=AppAlt,5=Geo Declination, 6=Geo Rectascension)
' ObjectLoc [deg]
 */
static int32 ObjectLoc(double JDNDaysUT, double *dgeo, double *datm, char *ObjectName, int32 Angle, int32 helflag, double *dret, char *serr)
{
  double x[6], xin[3], xaz[3], tjd_tt;
  int32 Planet;
  int32 epheflag;
  int32 iflag = SEFLG_EQUATORIAL;
  epheflag = helflag & (SEFLG_JPLEPH|SEFLG_SWIEPH|SEFLG_MOSEPH);
  iflag |= epheflag;
  if (!(helflag & SE_HELFLAG_HIGH_PRECISION))
    iflag |= SEFLG_NONUT | SEFLG_TRUEPOS;
  if (Angle < 5) iflag = iflag | SEFLG_TOPOCTR;
  if (Angle == 7) Angle = 0;
  tjd_tt = JDNDaysUT + DeltaT(JDNDaysUT, 0) / D2S;
  Planet = DeterObject(ObjectName);
  if (Planet != -1) {
    if (swe_calc(tjd_tt, Planet, iflag, x, serr) == ERR)
      return ERR;
  } else {
    if (call_swe_fixstar(ObjectName, tjd_tt, iflag, x, serr) == ERR)
      return ERR;
  }
  if (Angle == 2 ||  Angle == 5) {
    *dret = x[1];
  } else {
    if (Angle == 3 || Angle == 6) {
      *dret = x[0];
    } else {
      xin[0] = x[0];
      xin[1] = x[1];
      swe_azalt(JDNDaysUT, SE_EQU2HOR, dgeo, datm[0], datm[1], xin, xaz);
      if (Angle == 0)
	*dret = xaz[1];
      if (Angle == 4)
	*dret = AppAltfromTopoAlt(xaz[1], datm[0], datm[1], helflag);
      if (Angle == 1) {
	xaz[0] += 180;
	if (xaz[0] >= 360)
	  xaz[0] -= 360;
        *dret = xaz[0];
      }
    }
  }
  return OK;
}

/*###################################################################
' JDNDaysUT [Days]
' dgeo [array: longitude, latitude, eye height above sea m]
' TempE [C]
' PresE [mbar]
' ObjectName [-]
' Angle (0 = TopoAlt, 1 = Azi, 2=Topo Declination, 3=Topo Rectascension, 4=AppAlt,5=Geo Declination, 6=Geo Rectascension)
' ObjectLoc [deg]
 */
static int32 azalt_cart(double JDNDaysUT, double *dgeo, double *datm, char *ObjectName, int32 helflag, double *dret, char *serr)
{
  double x[6], xin[3], xaz[3], tjd_tt;
  int32 Planet;
  int32 epheflag;
  int32 iflag = SEFLG_EQUATORIAL;
  epheflag = helflag & (SEFLG_JPLEPH|SEFLG_SWIEPH|SEFLG_MOSEPH);
  iflag |= epheflag;
  if (!(helflag & SE_HELFLAG_HIGH_PRECISION))
    iflag |= SEFLG_NONUT | SEFLG_TRUEPOS;
  iflag = iflag | SEFLG_TOPOCTR;
  tjd_tt = JDNDaysUT + DeltaT(JDNDaysUT, 0) / D2S;
  Planet = DeterObject(ObjectName);
  if (Planet != -1) {
    if (swe_calc(tjd_tt, Planet, iflag, x, serr) == ERR)
      return ERR;
  } else {
    if (call_swe_fixstar(ObjectName, tjd_tt, iflag, x, serr) == ERR)
      return ERR;
  }
  xin[0] = x[0];
  xin[1] = x[1];
  swe_azalt(JDNDaysUT, SE_EQU2HOR, dgeo, datm[0], datm[1], xin, xaz);
  dret[0] = xaz[0];
  dret[1] = xaz[1]; /* true altitude */
  dret[2] = xaz[2]; /* apparent altitude */
  /* also return cartesian coordinates, for apparent altitude */
  xaz[1] = xaz[2];
  xaz[2] = 1;
  swi_polcart(xaz, xaz);
  dret[3] = xaz[0];
  dret[4] = xaz[1];
  dret[5] = xaz[2];
  return OK;
}

/*###################################################################
' LatA [rad]
' LongA [rad]
' LatB [rad]
' LongB [rad]
' DistanceAngle [rad]
*/
static double DistanceAngle(double LatA, double LongA, double LatB, double LongB)
{
  double dlon = LongB - LongA;
  double dlat = LatB - LatA;
  /* Haversine formula
   * http://www.movable-type.co.uk/scripts/GIS-FAQ-5.1.html
   * R.W. Sinnott, Virtues of the Haversine, Sky and Telescope, vol. 68, no. 2, 1984, p. 159
   */
  double sindlat2 = sin(dlat / 2);
  double sindlon2 = sin(dlon / 2);
  double corde = sindlat2 * sindlat2 + cos(LatA) * cos(LatB) * sindlon2 *sindlon2;
  if (corde > 1) corde = 1;
  return 2 * asin(sqrt(corde));
}

/*###################################################################
' heighteye [m]
' TempS [C]
' RH [%]
' kW [-]
*/
static double kW(double HeightEye, double TempS, double RH)
{
  /* From Schaefer , Archaeoastronomy, XV, 2000, page 128*/
  double WT = 0.031;
  WT *= 0.94 * (RH / 100.0) * exp(TempS / 15) * exp(-1 * HeightEye / scaleHwater);
  return WT;
}

/*###################################################################
' JDNDaysUT [-]
' AltS [deg]
' lat [deg]
' kOZ [-]
*/
static double kOZ(double AltS, double sunra, double Lat)
{
  double CHANGEKO, OZ, LT, kOZret;
  static double koz_last, alts_last, sunra_last;
  if (AltS == alts_last && sunra == sunra_last)
    return koz_last;
  alts_last = AltS; sunra_last = sunra;
  OZ = 0.031;
  LT = Lat * DEGTORAD;
  /* From Schaefer , Archaeoastronomy, XV, 2000, page 128*/
  kOZret = OZ * (3.0 + 0.4 * (LT * cos(sunra * DEGTORAD) - cos(3 * LT))) / 3.0;
  /* depending on day/night vision (altitude of sun < start astronomical twilight), KO changes from 100% to 30%
   * see extinction section of Vistas in Astronomy page 343*/
  CHANGEKO = (100 - 11.6 * mymin(6, mymax(-AltS - 12, 0))) / 100;
  koz_last = kOZret * CHANGEKO;
  return koz_last;
}

/*###################################################################
' AltS [deg]
' heighteye [m]
' kR [-]
*/
static double kR(double AltS, double HeightEye)
{
  /* depending on day/night vision (altitude of sun < start astronomical twilight),
   * lambda eye sensibility changes
   * see extinction section of Vistas in Astronomy page 343*/
  double CHANGEK, LAMBDA;
  double val = -AltS - 12;
  if (val < 0) val = 0;
  if (val > 6) val = 6;
  /*CHANGEK = (1 - 0.166667 * Min(6, Max(-AltS - 12, 0)));*/
  CHANGEK = (1 - 0.166667 * val );
  LAMBDA = 0.55 + (CHANGEK - 1) * 0.04;
  /* From Schaefer , Archaeoastronomy, XV, 2000, page 128 */
  return 0.1066 * exp(-1 * HeightEye / scaleHrayleigh) * pow(LAMBDA / 0.55 , -4);
}

static int Sgn(double x)
{
  if (x < 0) 
    return -1;
  return 1;
}

/*###################################################################
' JDNDaysUT [-]
' AltS [deg]
' lat [deg]
' heighteye [m]
' TempS [C]
' RH [%]
' VR [km]
' ka [-]
*/
static double ka(double AltS, double sunra, double Lat, double HeightEye, double TempS, double RH, double VR, char *serr)
{
  double CHANGEKA, LAMBDA, BetaVr, Betaa, kaact;
  double SL = Sgn(Lat);
  /* depending on day/night vision (altitude of sun < start astronomical twilight),
   * lambda eye sensibility changes
   * see extinction section of Vistas in Astronomy page 343 */
  static double alts_last, sunra_last, ka_last;
  if (AltS == alts_last && sunra == sunra_last)
    return ka_last;
  alts_last = AltS; sunra_last = sunra;
  CHANGEKA = (1 - 0.166667 * mymin(6, mymax(-AltS - 12, 0)));
  LAMBDA = 0.55 + (CHANGEKA - 1) * 0.04;
  if (VR != 0) {
    if (VR >= 1) {
      /* Visbility range from http://www1.cs.columbia.edu/CAVE/publications/pdfs/Narasimhan_CVPR03.pdf
       * http://www.icao.int/anb/SG/AMOSSG/meetings/amossg3/wp/SN11Rev.pdf where MOR=2.995/ke
       * factor 1.3 is the relation between "prevailing visibility" and 
       * meteorological range was derived by Koshmeider in the 1920's */
      BetaVr = 3.912 / VR;
      Betaa = BetaVr - (kW(HeightEye, TempS, RH) / scaleHwater + kR(AltS, HeightEye) / scaleHrayleigh) * 1000 * astr2tau;
      kaact = Betaa * scaleHaerosol / 1000 * tau2astr;
      if (kaact < 0) {
	if (serr != NULL)
	  strcpy(serr, "The provided Meteorological range is too long, when taking into acount other atmospheric parameters"); /* is a warning */
        /* return 0; * return "#HIGHVR"; */
      }
    } else {
      kaact = VR - kW(HeightEye, TempS, RH) - kR(AltS, HeightEye) - kOZ(AltS, sunra, Lat);
      if (kaact < 0) {
	if (serr != NULL)
	  strcpy(serr, "The provided atmosphic coeefficent (ktot) is too low, when taking into acount other atmospheric parameters"); /* is a warning */
        /* return 0; * "#LOWktot"; */
      }
    }
  } else {
    /* From Schaefer , Archaeoastronomy, XV, 2000, page 128 */
#ifdef SIMULATE_VICTORVB
    if (RH <= 0.00000001) RH = 0.00000001;
    if (RH >= 99.99999999) RH = 99.99999999;
#endif
    kaact = 0.1 * exp(-1 * HeightEye / scaleHaerosol) * pow(1 - 0.32 / log(RH / 100.0), 1.33) * (1 + 0.33 * SL * sin(sunra * DEGTORAD));
    kaact = kaact * pow(LAMBDA / 0.55, -1.3);
  }
  ka_last = kaact;
  return kaact;
}

/*###################################################################
' JDNDaysUT [-]
' AltS [deg]
' lat [deg]
' heighteye [m]
' TempS [C]
' RH [%]
' VR [km]
' ExtType [0=ka,1=kW,2=kR,3=kOZ,4=ktot]
' kt [-]
*/
static double kt(double AltS, double sunra, double Lat, double HeightEye, double TempS, double RH, double VR, int32 ExtType, char *serr)
{
  double kRact = 0;
  double kWact = 0;
  double kOZact = 0;
  double kaact = 0;
  if (ExtType == 2 || ExtType == 4)
    kRact = kR(AltS, HeightEye);
  if (ExtType == 1 || ExtType == 4)
    kWact = kW(HeightEye, TempS, RH);
  if (ExtType == 3 || ExtType == 4)
    kOZact = kOZ(AltS, sunra, Lat);
  if (ExtType == 0 || ExtType == 4)
    kaact = ka(AltS, sunra, Lat, HeightEye, TempS, RH, VR, serr);
  if (kaact < 0)
    kaact = 0;
  return kWact + kRact + kOZact + kaact;
}

/*###################################################################
' AppAlt0 [deg]
' PresS [mbar]
' Airmass [??]
*/
static double Airmass(double AppAltO, double Press)
{
  double airm, zend;
  zend = (90 - AppAltO) * DEGTORAD;
  if (zend > PI / 2)
    zend = PI / 2;
  airm = 1 / (cos(zend) + 0.025 * exp(-11 * cos(zend)));
  return Press / 1013 * airm;
}

/*###################################################################
' scaleH '[m]
' zend [rad]
' PresS [mbar]
' Xext [-]
*/
static double Xext(double scaleH, double zend, double Press)
{
  return Press / 1013.0 / (cos(zend) + 0.01 * sqrt(scaleH / 1000.0) * exp(-30.0 / sqrt(scaleH / 1000.0) * cos(zend)));
}

/*###################################################################
' scaleH '[m]
' zend [rad]
' PresS [mbar]
' Xlay [-]
*/
static double Xlay(double scaleH, double zend, double Press)
{
  /*return Press / 1013.0 / sqrt(1.0 - pow(sin(zend) / (1.0 + (scaleH / Ra)), 2));*/
  double a = sin(zend) / (1.0 + (scaleH / Ra));
  return Press / 1013.0 / sqrt(1.0 - a * a);
}

/*###################################################################
' Meteorological formula
'###################################################################
' TempS [C]
' HeightEye [m]
' TempEfromTempS [C]
*/
static double TempEfromTempS(double TempS, double HeightEye, double Lapse)
{
  return TempS - Lapse * HeightEye;
}

/*###################################################################
' TempS [C]
' PresS [mbar]
' HeightEye [m]
' PresEfromPresS [mbar]
*/
static double PresEfromPresS(double TempS, double Press, double HeightEye)
{
  return Press * exp(-9.80665 * 0.0289644 / (Kelvin(TempS) + 3.25 * HeightEye / 1000) / 8.31441 * HeightEye);
}

/*###################################################################
' AltO [deg]
' JDNDaysUT [-]
' AltS [deg]
' lat [deg]
' heighteye [m]
' TempS [C]
' PresS [mbar]
' RH [%]
' VR [km]
' Deltam [-]
*/
static double Deltam(double AltO, double AltS, double sunra, double Lat, double HeightEye, double *datm, int32 helflag, char *serr)
{
  double zend, xR, XW, Xa, XOZ;
  double PresE = PresEfromPresS(datm[1], datm[0], HeightEye);
  double TempE = TempEfromTempS(datm[1], HeightEye, LapseSA);
  double AppAltO = AppAltfromTopoAlt(AltO, TempE, PresE, helflag);
  double deltam;
  static double alts_last, alto_last, sunra_last, deltam_last;
  if (AltS == alts_last && AltO == alto_last && sunra == sunra_last)
    return deltam_last;
  alts_last = AltS; alto_last = AltO; sunra_last = sunra;
  if (staticAirmass == 0) {
    zend = (90 - AppAltO) * DEGTORAD;
    if (zend > PI / 2)
      zend = PI / 2;
    /* From Schaefer , Archaeoastronomy, XV, 2000, page 128*/
    xR = Xext(scaleHrayleigh, zend, datm[0]);
    XW = Xext(scaleHwater, zend, datm[0]);
    Xa = Xext(scaleHaerosol, zend, datm[0]);
    XOZ = Xlay(scaleHozone, zend, datm[0]);
    deltam = kR(AltS, HeightEye) * xR + kt(AltS, sunra, Lat, HeightEye, datm[1], datm[2], datm[3], 0, serr) * Xa + kOZ(AltS, sunra, Lat) * XOZ + kW(HeightEye, datm[1], datm[2]) * XW;
  } else {
    deltam = kt(AltS, sunra, Lat, HeightEye, datm[1], datm[2], datm[3], 4, serr) * Airmass(AppAltO, datm[0]);
  }
  deltam_last = deltam;
  return deltam;
}

/*###################################################################
' AltO [deg]
' JDNDaysUT [-]
' AltS [deg]
' lat [deg]
' heighteye [m]
' TempS [C]
' PresS [mbar]
' RH [%]
' VR [km]
' Bn [nL]
*/
static double Bn(double AltO, double JDNDayUT, double AltS, double sunra, double Lat, double HeightEye, double *datm, int32 helflag, char *serr)
{
  double PresE = PresEfromPresS(datm[1], datm[0], HeightEye);
  double TempE = TempEfromTempS(datm[1], HeightEye, LapseSA);
  double AppAltO = AppAltfromTopoAlt(AltO, TempE, PresE, helflag);
  double zend, YearB, MonthB, DayB, Bna, kX, Bnb;
  double B0 = 0.0000000000001, dut;
  int iyar, imon, iday;
  /* Below altitude of 10 degrees, the Bn stays the same (see page 343 Vistas in Astronomy) */
  if (AppAltO < 10)
    AppAltO = 10;
  zend = (90 - AppAltO) * DEGTORAD;
  /* From Schaefer , Archaeoastronomy, XV, 2000, page 128 and adjusted for sunspot period*/
  /*YearB = DatefromJDut(JDNDayUT, 1);
    MonthB = DatefromJDut(JDNDayUT, 2);
    DayB = DatefromJDut(JDNDayUT, 3);*/
  swe_revjul(JDNDayUT, SE_GREG_CAL, &iyar, &imon, &iday, &dut); 
  YearB = iyar; MonthB = imon; DayB = iday;
  Bna = B0 * (1 + 0.3 * cos(6.283 * (YearB + ((DayB - 1) / 30.4 + MonthB - 1) / 12 - 1990.33) / 11.1));
  kX = Deltam(AltO, AltS, sunra, Lat, HeightEye, datm, helflag, serr);
  /* From Schaefer , Archaeoastronomy, XV, 2000, page 129 */
  Bnb = Bna * (0.4 + 0.6 / sqrt(1 - 0.96 * pow(sin(zend), 2))) * pow(10, -0.4 * kX);
  return mymax(Bnb, 0) * erg2nL;
}

/*###################################################################
' JDNDaysUT [-]
' dgeo [array: longitude, latitude, eye height above sea m]
' TempE [C]
' PresE [mbar]
' ObjectName [-]
' Magnitude [-]
*/
static int32 Magnitude(double JDNDaysUT, double *dgeo, char *ObjectName, int32 helflag, double *dmag, char *serr)
{
  double x[20];
  int32 Planet, iflag, epheflag;
  epheflag = helflag & (SEFLG_JPLEPH|SEFLG_SWIEPH|SEFLG_MOSEPH);
  *dmag = -99.0;
  Planet = DeterObject(ObjectName);
  iflag = SEFLG_TOPOCTR | SEFLG_EQUATORIAL | epheflag;
  if (!(helflag & SE_HELFLAG_HIGH_PRECISION))
    iflag |= SEFLG_NONUT|SEFLG_TRUEPOS;
  if (Planet != -1) {
    /**dmag = Phenomena(JDNDaysUT, Lat, Longitude, HeightEye, TempE, PresE, ObjectName, 4);*/
    swe_set_topo(dgeo[0], dgeo[1], dgeo[2]);
    if (swe_pheno_ut(JDNDaysUT, Planet, iflag, x, serr) == ERR)
      return ERR;
    *dmag = x[4];
  } else {
    if (call_swe_fixstar_mag(ObjectName, dmag, serr) == ERR)
      return ERR;
  }
  return OK;
}

#if 0
static int32 fast_magnitude(double tjd, double *dgeo, char *ObjectName, int32 helflag, double *dmag, char *serr)
{
  int32 retval = OK, ipl, ipli;
  double dtjd;
  static double tjdsv[3];
  static double dmagsv[3];
  static int32 helflagsv[3];
  ipl = DeterObject(ObjectName);
  ipli = ipl;
  if (ipli > SE_MOON) 
    ipli = 2;
  dtjd = tjd - tjdsv[ipli];
  if (tjdsv[ipli] != 0 && helflag == helflagsv[ipli] && fabs(dtjd) < 5.0 / 1440.0) {
    *dmag = dmagsv[ipli];
  } else {
    retval = Magnitude(tjd, dgeo, ObjectName, helflag, dmag, serr);
    tjdsv[ipli] = tjd;
    helflagsv[ipli] = helflag;
    dmagsv[ipli] = *dmag;
  }
  return retval;
}
#endif

/*###################################################################
' dist [km]
' phasemoon [-]
' MoonsBrightness [-]
*/
static double MoonsBrightness(double dist, double phasemoon)
{
  double log10 = 2.302585092994;
  /*Moon's brightness changes with distance: http://hem.passagen.se/pausch/comp/ppcomp.html#15 */
  return -21.62 + 5 * log(dist / (Ra / 1000)) / log10 + 0.026 * fabs(phasemoon) + 0.000000004 * pow(phasemoon, 4);
}

/*###################################################################
' AltM [deg]
' AziM [deg]
' AziS [deg]
' MoonPhase [deg]
*/
static double MoonPhase(double AltM, double AziM, double AziS)
{
  double AltMi = AltM * DEGTORAD;
  double AziMi = AziM * DEGTORAD;
  double AziSi = AziS * DEGTORAD;
  return 180 - acos(cos(AziSi - AziMi) * cos(AltMi + 0.95 * DEGTORAD)) / DEGTORAD;
}

/*###################################################################
' Pressure [mbar]
*/
static double Bm(double AltO, double AziO, double AltM, double AziM, double AltS, double AziS, double sunra, double Lat, double HeightEye, double *datm, int32 helflag, char *serr)
{
  double M0 = -11.05;
  double Bm = 0;
  double RM, kXM, kX, C3, FM, phasemoon, MM;
  if (AltM > -0.26) {
  /* moon only adds light when (partly) above horizon
   * From Schaefer , Archaeoastronomy, XV, 2000, page 129*/
    RM = DistanceAngle(AltO * DEGTORAD, AziO * DEGTORAD, AltM * DEGTORAD, AziM * DEGTORAD) / DEGTORAD;
    kXM = Deltam(AltM, AltS, sunra, Lat, HeightEye, datm, helflag, serr);
    kX = Deltam(AltO, AltS, sunra, Lat, HeightEye, datm, helflag, serr);
    C3 = pow(10, -0.4 * kXM);
    FM = (62000000.0) / RM / RM + pow(10, 6.15 - RM / 40) + pow(10, 5.36) * (1.06 + pow(cos(RM * DEGTORAD), 2));
    Bm = FM * C3 + 440000 * (1 - C3);
    phasemoon = MoonPhase(AltM, AziM, AziS);
    MM = MoonsBrightness(MoonDistance, phasemoon);
    Bm = Bm * pow(10, -0.4 * (MM - M0 + 43.27));
    Bm = Bm * (1 - pow(10, -0.4 * kX));
  }
  Bm = mymax(Bm, 0) * erg2nL;
  return Bm;
}

/*###################################################################
' Pressure [mbar]
*/
static double Btwi(double AltO, double AziO, double AltS, double AziS, double sunra, double Lat, double HeightEye, double *datm, int32 helflag, char *serr)
{
double M0 = -11.05;
double MS = -26.74;
double PresE = PresEfromPresS(datm[1], datm[0], HeightEye);
double TempE = TempEfromTempS(datm[1], HeightEye, LapseSA);
double AppAltO = AppAltfromTopoAlt(AltO, TempE, PresE, helflag);
double ZendO = 90 - AppAltO;
double RS = DistanceAngle(AltO * DEGTORAD, AziO * DEGTORAD, AltS * DEGTORAD, AziS * DEGTORAD) / DEGTORAD;
double kX = Deltam(AltO, AltS, sunra, Lat, HeightEye, datm, helflag, serr);
double k = kt(AltS, sunra, Lat, HeightEye, datm[1], datm[2], datm[3], 4, serr);
/* From Schaefer , Archaeoastronomy, XV, 2000, page 129*/
double Btwi = pow(10, -0.4 * (MS - M0 + 32.5 - AltS - (ZendO / (360 * k))));
Btwi = Btwi * (100 / RS) * (1 - pow(10, -0.4 * kX));
Btwi = mymax(Btwi, 0) * erg2nL;
return Btwi;
}

/*###################################################################
' Pressure [mbar]
*/
static double Bday(double AltO, double AziO, double AltS, double AziS, double sunra, double Lat, double HeightEye, double *datm, int32 helflag, char *serr)
{
  double M0 = -11.05;
  double MS = -26.74;
  double RS = DistanceAngle(AltO * DEGTORAD, AziO * DEGTORAD, AltS * DEGTORAD, AziS * DEGTORAD) / DEGTORAD;
  double kXS = Deltam(AltS, AltS, sunra, Lat, HeightEye, datm, helflag, serr);
  double kX = Deltam(AltO, AltS, sunra, Lat, HeightEye, datm, helflag, serr);
  /* From Schaefer , Archaeoastronomy, XV, 2000, page 129*/
  double C4 = pow(10, -0.4 * kXS);
  double FS = (62000000.0) / RS / RS + pow(10, (6.15 - RS / 40)) + pow(10, 5.36) * (1.06 + pow(cos(RS * DEGTORAD), 2));
  double Bday = FS * C4 + 440000.0 * (1 - C4);
  Bday = Bday * pow(10, (-0.4 * (MS - M0 + 43.27)));
  Bday = Bday * (1 - pow(10, -0.4 * kX));
  Bday = mymax(Bday, 0) * erg2nL;
  return Bday;
}

/*###################################################################
' Value [nL]
' PresS [mbar]
' Bcity [nL]
*/
static double Bcity(double Value, double Press)
{
  double Bcity = Value;
  Bcity = mymax(Bcity, 0);
  return Bcity;
}

/*###################################################################
' Pressure [mbar]
*/
static double Bsky(double AltO, double AziO, double AltM, double AziM, double JDNDaysUT, double AltS, double AziS, double sunra, double Lat, double HeightEye, double *datm, int32 helflag, char *serr)
{
  double Bsky = 0;
  if (AltS < -3) {
    Bsky += Btwi(AltO, AziO, AltS, AziS, sunra, Lat, HeightEye, datm, helflag, serr);
  } else {
    if (AltS > 4) {
      Bsky += Bday(AltO, AziO, AltS, AziS, sunra, Lat, HeightEye, datm, helflag, serr);
    } else {
      Bsky += mymin(Bday(AltO, AziO, AltS, AziS, sunra, Lat, HeightEye, datm, helflag, serr), Btwi(AltO, AziO, AltS, AziS, sunra, Lat, HeightEye, datm, helflag, serr));
    }
  }
  /* if max. Bm [1E7] <5% of Bsky don't add Bm*/
  if (Bsky < 200000000.0) 
    Bsky += Bm(AltO, AziO, AltM, AziM, AltS, AziS, sunra, Lat, HeightEye, datm, helflag, serr);
  if (AltS <= 0)
    Bsky += Bcity(0, datm[0]);
  /* if max. Bn [250] <5% of Bsky don't add Bn*/
  if (Bsky < 5000)
    Bsky = Bsky + Bn(AltO, JDNDaysUT, AltS, sunra, Lat, HeightEye, datm, helflag, serr);
  /* if max. Bm [1E7] <5% of Bsky don't add Bm*/
  return Bsky;
}

/* default handling:
 * 1. datm (atmospheric conditions):
 * datm consists of 
 *     [0]  atmospheric pressure
 *     [1]  temperature
 *     [2]  relative humidity
 *     [3]  extinction coefficient
 * In order to get default values for [0..2], set datm[0] = 0.
 * Default values for [1-2] are only provided if [0] == 0.
 * [3] defaults outside this function, depending on [0-2].
 * 
 * 2. dobs (observer definition):
 *     [0]  age (default 36)
 *     [1]  Snellen ratio or visual acuity of observer (default 1)
 */
static void default_heliacal_parameters(double *datm, double *dgeo, double *dobs, int helflag)
{
  int i;
  if (datm[0] <= 0) {
    /* estimate atmospheric pressure, according to the
     * International Standard Atmosphere (ISA) */
    datm[0] = 1013.25 * pow(1 - 0.0065 * dgeo[2] / 288, 5.255);
    /* temperature */
    if (datm[1] == 0)
      datm[1] = 15 - 0.0065 * dgeo[2];
    /* relative humidity, independent of atmospheric pressure and altitude */
    if (datm[2] == 0)
      datm[2] = 40;
    /* note: datm[3] / VR defaults outside this function */
  } else {
#ifndef SIMULATE_VICTORVB
    if (datm[2] <= 0.00000001) datm[2] = 0.00000001;
    if (datm[2] >= 99.99999999) datm[2] = 99.99999999;
#endif
  }
  /* age of observer */
  if (dobs[0] == 0)
    dobs[0] = 36;
  /* SN Snellen factor of the visual acuity of the observer */
  if (dobs[1] == 0)
    dobs[1] = 1;
  if (!(helflag & SE_HELFLAG_OPTICAL_PARAMS)) {
    for (i = 2; i <= 5; i++)
      dobs[i] = 0;
  }
  /* OpticMagn undefined -> use eye */
  if (dobs[3] == 0) {
    dobs[2] = 1; /* Binocular = 1 */
    dobs[3] = 1; /* OpticMagn = 1: use eye */
    /* dobs[4] and dobs[5] (OpticDia and OpticTrans) will be defaulted in 
     * OpticFactor() */
  }
}

/*###################################################################
' age [Year]
' SN [-]
' AltO [deg]
' AziO [deg]
' AltM [deg]
' AziM [deg]
' MoonDistance [km]
' JDNDaysUT [-]
' AltS [deg]
' AziS [deg]
' lat [deg]
' heighteye [m]
' TempS [C]
' PresS [mbar]
' RH [%]
' VR [km]
' VisLimMagn [-]
*/
static double VisLimMagn(double *dobs, double AltO, double AziO, double AltM, double AziM, double JDNDaysUT, double AltS, double AziS, double sunra, double Lat, double HeightEye, double *datm, int32 helflag, int32 *scotopic_flag, char *serr)
{
  double C1, C2, Th, kX, Bsk, CorrFactor1, CorrFactor2;
  double log10 = 2.302585092994;
  /*double Age = dobs[0];*/
  /*double SN = dobs[1];*/
  Bsk = Bsky(AltO, AziO, AltM, AziM, JDNDaysUT, AltS, AziS, sunra, Lat, HeightEye, datm, helflag, serr);
  /* Schaefer, Astronomy and the limits of vision, Archaeoastronomy, 1993 Verder:*/
  kX = Deltam(AltO, AltS, sunra, Lat, HeightEye, datm, helflag, serr);
  /* influence of age*/
  /*Fa = mymax(1, pow(p(23, Bsk) / p(Age, Bsk), 2)); */
  CorrFactor1 = OpticFactor(Bsk, kX, dobs, JDNDaysUT, "", 1, helflag);
  CorrFactor2 = OpticFactor(Bsk, kX, dobs, JDNDaysUT, "", 0, helflag);
  /* From Schaefer , Archaeoastronomy, XV, 2000, page 129*/
  if (Bsk < BNIGHT && !(helflag & SE_HELFLAG_VISLIM_PHOTOPIC)) {
    C1 = 1.5848931924611e-10; /*pow(10, -9.8);*/ /* C1 = 10 ^ (-9.8);*/
    C2 = 0.012589254117942; /*pow(10, -1.9);*/ /* C2 = 10 ^ (-1.9);*/
    if (scotopic_flag != NULL) 
      *scotopic_flag = 1;
  } else {
    C1 = 4.4668359215096e-9; /*pow(10, -8.35);*/ /* C1 = 10 ^ (-8.35);*/
    C2 = 1.2589254117942e-6; /*pow(10, -5.9);*/ /* C2 = 10 ^ (-5.9);*/
    if (scotopic_flag != NULL) 
      *scotopic_flag = 0;
  }
  if (scotopic_flag != NULL) {
    if (BNIGHT * BNIGHT_FACTOR > Bsk && BNIGHT / BNIGHT_FACTOR < Bsk)
      *scotopic_flag |= 2;
  }
  /*Th = C1 * pow(1 + sqrt(C2 * Bsk), 2) * Fa;*/
  Bsk = Bsk / CorrFactor1;
  Th = C1 * pow(1 + sqrt(C2 * Bsk), 2) * CorrFactor2;
#if DEBUG
  fprintf(stderr, "Bsk=%f\n", Bsk);
  fprintf(stderr, "kX =%f\n", kX);
  fprintf(stderr, "Th =%f\n", Th);
  fprintf(stderr, "CorrFactor1=%f\n", CorrFactor1);
  fprintf(stderr, "CorrFactor2=%f\n", CorrFactor2);
#endif
  /* Visual limiting magnitude of point source*/
#if 0
  if (SN <= 0.00000001)
    SN = 0.00000001;
  return -16.57 - 2.5 * (log(Th) / log10) - kX + 5.0 * (log(SN) / log10);*/
#endif
  return -16.57 - 2.5 * (log(Th) / log10);
}

/* Limiting magnitude in dark skies 
 * function returns:
 * -1   Error
 * -2   Object is below horizon
 *  0   OK, photopic vision
 *  |1  OK, scotopic vision
 *  |2  OK, near limit photopic/scotopic
*/
int32 FAR PASCAL_CONV swe_vis_limit_mag(double tjdut, double *dgeo, double *datm, double *dobs, char *ObjectName, int32 helflag, double *dret, char *serr)
{
  int32 retval = OK, i, scotopic_flag = 0;
  double AltO, AziO, AltM, AziM, AltS, AziS;
  double sunra;
  swi_set_tid_acc(tjdut, helflag, 0);
  sunra = SunRA(tjdut, helflag, serr);
  default_heliacal_parameters(datm, dgeo, dobs, helflag);
  swe_set_topo(dgeo[0], dgeo[1], dgeo[2]);
  for (i = 0; i < 7; i++)
    dret[i] = 0;
  if (ObjectLoc(tjdut, dgeo, datm, ObjectName, 0, helflag, &AltO, serr) == ERR)
    return ERR;
  if (AltO < 0 && serr != NULL) {
    strcpy(serr, "object is below local horizon");
    *dret = -100;
    return -2;
  }
  if (ObjectLoc(tjdut, dgeo, datm, ObjectName, 1, helflag, &AziO, serr) == ERR)
    return ERR;
  if (helflag & SE_HELFLAG_VISLIM_DARK) {
    AltS = -90;
    AziS = 0;
  } else {
    if (ObjectLoc(tjdut, dgeo, datm, "sun", 0, helflag, &AltS, serr) == ERR)
      return ERR;
    if (ObjectLoc(tjdut, dgeo, datm, "sun", 1, helflag, &AziS, serr) == ERR)
      return ERR;
  }
  if (strncmp(ObjectName, "moon", 4) == 0 ||
      (helflag & SE_HELFLAG_VISLIM_DARK) ||
      (helflag & SE_HELFLAG_VISLIM_NOMOON)
     ) {
    AltM = -90; AziM = 0;
  } else {
    if (ObjectLoc(tjdut, dgeo, datm, "moon", 0, helflag, &AltM, serr) == ERR)
      return ERR;
    if (ObjectLoc(tjdut, dgeo, datm, "moon", 1, helflag, &AziM, serr) == ERR)
      return ERR;
  }
#if DEBUG
{
  int i;
  for (i = 0; i < 6;i++)
    printf("dobs[%d] = %f\n", i, dobs[i]);
  printf("AltO = %.10f, AziO = %.10f\n", AltO, AziO);
  printf("AltM = %.10f, AziM = %.10f\n", AltM, AziM);
  printf("AltS = %.10f, AziS = %.10f\n", AltS, AziS);
  printf("JD = %.10f\n", tjdut);
  printf("lat = %f, eyeh = %f\n", dgeo[1], dgeo[2]);
  for (i = 0; i < 4;i++)
    printf("datm[%d] = %f\n", i, datm[i]);
  printf("helflag = %d\n", helflag);
}
#endif
  dret[0] = VisLimMagn(dobs, AltO, AziO, AltM, AziM, tjdut, AltS, AziS, sunra, dgeo[1], dgeo[2], datm, helflag, &scotopic_flag, serr);
  dret[1] = AltO;
  dret[2] = AziO;
  dret[3] = AltS;
  dret[4] = AziS;
  dret[5] = AltM;
  dret[6] = AziM;
  if (Magnitude(tjdut, dgeo, ObjectName, helflag, &(dret[7]), serr) == ERR)
    return ERR;
  retval = scotopic_flag;
  /*dret[8] = (double) is_scotopic;*/
  /*if (*serr != '\0') * in VisLimMagn(), serr is only a warning *
    retval = ERR; */
  return retval;
}

/*###################################################################
' Magn [-]
' age [Year]
' SN [-]
' AltO [deg]
' AziO [deg]
' AltM [deg]
' AziM [deg]
' MoonDistance [km]
' JDNDaysUT [-]
' AziS [deg]
' lat [deg]
' heighteye [m]
' Temperature [C]
' Pressure [mbar]
' RH [%]
' VR [km]
' TopoArcVisionis [deg]
*/
static int32 TopoArcVisionis(double Magn, double *dobs, double AltO, double AziO, double AltM, double AziM, double JDNDaysUT, double AziS, double sunra, double Lat, double HeightEye, double *datm, int32 helflag, double *dret, char *serr)
{
  double Xm, Ym, AltSi, AziSi;
  double xR = 0;
  double Xl = 45;
  double Yl, Yr;
  Yl = Magn - VisLimMagn(dobs, AltO, AziO, AltM, AziM, JDNDaysUT, AltO - Xl, AziS, sunra, Lat, HeightEye, datm, helflag, NULL, serr);
  /* if (*serr != '\0') return ERR; * serr is only a warning */
  Yr = Magn - VisLimMagn(dobs, AltO, AziO, AltM, AziM, JDNDaysUT, AltO - xR, AziS, sunra, Lat, HeightEye, datm, helflag, NULL, serr);
  /* if (*serr != '\0') return ERR; * serr is only a warning */
  /* http://en.wikipedia.org/wiki/Bisection_method*/
  if ((Yl * Yr) <= 0) {
    while(fabs(xR - Xl) > epsilon) {
      /*Calculate midpoint of domain*/
      Xm = (xR + Xl) / 2.0;
      AltSi = AltO - Xm;
      AziSi = AziS;
      Ym = Magn - VisLimMagn(dobs, AltO, AziO, AltM, AziM, JDNDaysUT, AltSi, AziSi, sunra, Lat, HeightEye, datm, helflag, NULL, serr);
      /* if (*serr != '\0') return ERR; * serr is only a warning */
      if ((Yl * Ym) > 0) {
	/* Throw away left half*/
	Xl = Xm;
	Yl = Ym;
      } else {
	/* Throw away right half */
	xR = Xm;
	Yr = Ym;
      }
    }
    Xm = (xR + Xl) / 2.0;
  } else {
    Xm = 99;
  }
  if (Xm < AltO) 
    Xm = AltO;
  *dret = Xm;
  return OK;
}

int32 FAR PASCAL_CONV swe_topo_arcus_visionis(double tjdut, double *dgeo, double *datm, double *dobs, int32 helflag, double mag, double azi_obj, double alt_obj, double azi_sun, double azi_moon, double alt_moon, double *dret, char *serr)
{
  double sunra;
  swi_set_tid_acc(tjdut, helflag, 0);
  sunra = SunRA(tjdut, helflag, serr);
  if (serr != NULL && *serr != '\0')
    return ERR;
  return TopoArcVisionis(mag, dobs, alt_obj, azi_obj, alt_moon, azi_moon, tjdut, azi_sun, sunra, dgeo[1], dgeo[2], datm, helflag, dret, serr);
}

/*###################################################################*/
/*' Magn [-]
' age [Year]
' SN Snellen factor of the visual aquity of the observer
  see: http://www.i-see.org/eyecharts.html#make-your-own
' AziO [deg]
' AltM [deg]
' AziM [deg]
' MoonDistance [km]
' JDNDaysUT [-]
' AziS [deg]
' Lat [deg]
' HeightEye [m]
' Temperature [C]
' Pressure [mbar]
' RH [%]   relative humidity
' VR [km]  Meteorological Range, 
  see http://www.iol.ie/~geniet/eng/atmoastroextinction.htm
' TypeAngle 
'   [0=Object's altitude, 
'    1=Arcus Visonis (Object's altitude - Sun's altitude), 
'    2=Sun's altitude]
' HeliacalAngle [deg]
*/
static int32 HeliacalAngle(double Magn, double *dobs, double AziO, double AltM, double AziM, double JDNDaysUT, double AziS, double *dgeo, double *datm, int32 helflag, double *dangret, char *serr)
{
  double x, minx, maxx, xmin, ymin, Xl, xR, Yr, Yl, Xm, Ym, xmd, ymd;
  double Arc, DELTAx;
  double sunra = SunRA(JDNDaysUT, helflag, serr);
  double Lat = dgeo[1];
  double HeightEye = dgeo[2];
  if (PLSV == 1) {
    dangret[0] = criticalangle;
    dangret[1] = criticalangle + Magn * 2.492 + 13.447;
    dangret[2] = -(Magn * 2.492 + 13.447); /* Magn * 1.1 + 8.9;*/
    return OK;
  }
  minx = 2;
  maxx = 20;
  xmin = 0;
  ymin = 10000;
  for (x = minx; x <= maxx; x++) {
    if (TopoArcVisionis(Magn, dobs, x, AziO, AltM, AziM, JDNDaysUT, AziS, sunra, Lat, HeightEye, datm, helflag, &Arc, serr) == ERR)
      return ERR;
    if (Arc < ymin) {
      ymin = Arc;
      xmin = x;
    }
  }
  Xl = xmin - 1;
  xR = xmin + 1;
  if (TopoArcVisionis(Magn, dobs, xR, AziO, AltM, AziM, JDNDaysUT, AziS, sunra, Lat, HeightEye, datm, helflag, &Yr, serr) == ERR)
    return ERR;
  if (TopoArcVisionis(Magn, dobs, Xl, AziO, AltM, AziM, JDNDaysUT, AziS, sunra, Lat, HeightEye, datm, helflag, &Yl, serr) == ERR)
    return ERR;
  /* http://en.wikipedia.org/wiki/Bisection_method*/
  while(fabs(xR - Xl) > 0.1) {
    /* Calculate midpoint of domain */
    Xm = (xR + Xl) / 2.0;
    DELTAx = 0.025;
    xmd = Xm + DELTAx;
    if (TopoArcVisionis(Magn, dobs, Xm, AziO, AltM, AziM, JDNDaysUT, AziS, sunra, Lat, HeightEye, datm, helflag, &Ym, serr) == ERR)
      return ERR;
    if (TopoArcVisionis(Magn, dobs, xmd, AziO, AltM, AziM, JDNDaysUT, AziS, sunra, Lat, HeightEye, datm, helflag, &ymd, serr) == ERR)
      return ERR;
    if (Ym >= ymd) {
      /* Throw away left half */
      Xl = Xm;
      Yl = Ym;
    } else {
      /*Throw away right half */
      xR = Xm;
      Yr = Ym;
    }
  }
  Xm = (xR + Xl) / 2.0;
  Ym = (Yr + Yl) / 2.0;
  dangret[1] = Ym;
  dangret[2] = Xm - Ym;
  dangret[0] = Xm;
  return OK;
}

int32 FAR PASCAL_CONV swe_heliacal_angle(double tjdut, double *dgeo, double *datm, double *dobs, int32 helflag, double mag, double azi_obj, double azi_sun, double azi_moon, double alt_moon, double *dret, char *serr)
{
  if (dgeo[2] < SEI_ECL_GEOALT_MIN || dgeo[2] > SEI_ECL_GEOALT_MAX) {
    if (serr != NULL)
      sprintf(serr, "location for heliacal events must be between %.0f and %.0f m above sea", SEI_ECL_GEOALT_MIN, SEI_ECL_GEOALT_MAX);
    return ERR;
  }
  swi_set_tid_acc(tjdut, helflag, 0);
  return HeliacalAngle(mag, dobs, azi_obj, alt_moon, azi_moon, tjdut, azi_sun, dgeo, datm, helflag, dret, serr);
}

/*###################################################################
' AltO [deg]
' AziO [deg]
' AltS [deg]
' AziS [deg]
' parallax [deg]
' WidthMoon [deg]
*/
static double WidthMoon(double AltO, double AziO, double AltS, double AziS, double parallax)
{
  /* Yallop 1998, page 3*/
  double GeoAltO = AltO + parallax;
  return 0.27245 * parallax * (1 + sin(GeoAltO * DEGTORAD) * sin(parallax * DEGTORAD)) * (1 - cos((AltS - GeoAltO) * DEGTORAD) * cos((AziS - AziO) * DEGTORAD));
}

/*###################################################################
' W [deg]
' LengthMoon [deg]
*/
static double LengthMoon(double W, double Diamoon)
{
  double Wi, D;
  if (Diamoon == 0) Diamoon = AvgRadiusMoon * 2;
  Wi = W * 60;
  D = Diamoon * 60;
  /* Crescent length according: http://calendar.ut.ac.ir/Fa/Crescent/Data/Sultan2005.pdf*/
  return (D - 0.3 * (D + Wi) / 2.0 / Wi) / 60.0;
}

/*###################################################################
' W [deg]
' GeoARCVact [deg]
' q [-]
*/
static double qYallop(double W, double GeoARCVact)
{
  double Wi = W * 60;
  return (GeoARCVact - (11.8371 - 6.3226 * Wi + 0.7319 * Wi * Wi - 0.1018 * Wi * Wi * Wi)) / 10;
}

/*###################################################################
'A (0,p)
'B (1,q)
'C (0,r)
'D (1,s)
*/
static double crossing(double A, double B, double C, double D)
{
return (C - A) / ((B - A) - (D - C));
}

/*###################################################################*/
static int32 DeterTAV(double *dobs, double JDNDaysUT, double *dgeo, double *datm, char *ObjectName, int32 helflag, double *dret, char *serr)
{
  double Magn, AltO, AziS, AziO, AziM, AltM;
  double sunra = SunRA(JDNDaysUT, helflag, serr);
  if (Magnitude(JDNDaysUT, dgeo, ObjectName, helflag, &Magn, serr) == ERR)
    return ERR;
  if (ObjectLoc(JDNDaysUT, dgeo, datm, ObjectName, 0, helflag, &AltO, serr) == ERR)
    return ERR;
  if (ObjectLoc(JDNDaysUT, dgeo, datm, ObjectName, 1, helflag, &AziO, serr) == ERR)
    return ERR;
  if (strncmp(ObjectName, "moon", 4) == 0) {
    AltM = -90; 
    AziM = 0;
  } else {
    if (ObjectLoc(JDNDaysUT, dgeo, datm, "moon", 0, helflag, &AltM, serr) == ERR)
      return ERR;
    if (ObjectLoc(JDNDaysUT, dgeo, datm, "moon", 1, helflag, &AziM, serr) == ERR)
      return ERR;
  }
  if (ObjectLoc(JDNDaysUT, dgeo, datm, "sun", 1, helflag, &AziS, serr) == ERR)
    return ERR;
  if (TopoArcVisionis(Magn, dobs, AltO, AziO, AltM, AziM, JDNDaysUT, AziS, sunra, dgeo[1], dgeo[2], datm, helflag, dret, serr) == ERR)
    return ERR;
  return OK;
}

/*###################################################################
' A y-value at x=1
' B y-value at x=0
' C y-value at x=-1
' x2min minimum for the quadratic function
*/
static double x2min(double A, double B, double C)
{
  double term = A + C - 2 * B;
  if (term == 0)
    return 0;
  return -(A - C) / 2.0 / term;
}


/*###################################################################
' A y-value at x=1
' B y-value at x=0
' C y-value at x=-1
' x
' y is y-value of quadratic function
*/
static double funct2(double A, double B, double C, double x)
{
  return (A + C - 2 * B) / 2.0 * x * x + (A - C) / 2.0 * x + B;
}

static void strcpy_VBsafe(char *sout, char *sin)
{ 
  char *sp, *sp2; 
  int iw = 0;
  sp = sin; 
  sp2 = sout;
  while((isalnum(*sp) || *sp == ' ' || *sp == '-') && iw < 30) {
    *sp2 = *sp;
    sp++; sp2++; iw++;
  }
  *sp2 = '\0';
}

/*###################################################################
' JDNDaysUT [JDN]
' HPheno
'0=AltO [deg]		topocentric altitude of object (unrefracted)
'1=AppAltO [deg]        apparent altitude of object (refracted)
'2=GeoAltO [deg]        geocentric altitude of object
'3=AziO [deg]           azimuth of object
'4=AltS [deg]           topocentric altitude of Sun
'5=AziS [deg]           azimuth of Sun
'6=TAVact [deg]         actual topocentric arcus visionis
'7=ARCVact [deg]        actual (geocentric) arcus visionis
'8=DAZact [deg]         actual difference between object's and sun's azimuth
'9=ARCLact [deg]        actual longitude difference between object and sun
'10=kact [-]            extinction coefficient
'11=minTAV [deg]        smallest topocentric arcus visionis
'12=TfistVR [JDN]       first time object is visible, according to VR
'13=TbVR [JDN]          optimum time the object is visible, according to VR
'14=TlastVR [JDN]       last time object is visible, according to VR
'15=TbYallop[JDN]       best time the object is visible, according to Yallop
'16=WMoon [deg]         cresent width of moon
'17=qYal [-]            q-test value of Yallop 
'18=qCrit [-]           q-test criterion of Yallop
'19=ParO [deg]          parallax of object
'20 Magn [-]            magnitude of object
'21=RiseO [JDN]         rise/set time of object
'22=RiseS [JDN]         rise/set time of sun
'23=Lag [JDN]           rise/set time of object minus rise/set time of sun
'24=TvisVR [JDN]        visibility duration
'25=LMoon [deg]         cresent length of moon
'26=CVAact [deg]
'27=Illum [%] 'new
'28=CVAact [deg] 'new
'29=MSk [-]
*/
int32 FAR PASCAL_CONV swe_heliacal_pheno_ut(double JDNDaysUT, double *dgeo, double *datm, double *dobs, char *ObjectNameIn, int32 TypeEvent, int32 helflag, double *darr, char *serr)
{
  double AziS, AltS, AltS2, AziO, AltO, AltO2, GeoAltO, AppAltO, DAZact, TAVact, ParO, MagnO;
  double ARCVact, ARCLact, kact, WMoon, LMoon = 0, qYal, qCrit;
  double RiseSetO, RiseSetS, Lag, TbYallop, TfirstVR, TlastVR, TbVR;
  double MinTAV = 0, MinTAVact, Ta, Tc, TimeStep, TimePointer, MinTAVoud = 0, DeltaAltoud = 0, DeltaAlt, TvisVR, crosspoint;
  double OldestMinTAV, extrax, illum;
  double elong, attr[30];
  double TimeCheck, LocalminCheck;
  int32 retval = OK, RS, Planet;
  AS_BOOL noriseO = FALSE;
  char ObjectName[AS_MAXCH];
  double sunra;
  int32 iflag = helflag & (SEFLG_JPLEPH|SEFLG_SWIEPH|SEFLG_MOSEPH);
  if (dgeo[2] < SEI_ECL_GEOALT_MIN || dgeo[2] > SEI_ECL_GEOALT_MAX) {
    if (serr != NULL)
      sprintf(serr, "location for heliacal events must be between %.0f and %.0f m above sea", SEI_ECL_GEOALT_MIN, SEI_ECL_GEOALT_MAX);
    return ERR;
  }
  swi_set_tid_acc(JDNDaysUT, helflag, 0);
  sunra = SunRA(JDNDaysUT, helflag, serr);
  /* note, the fixed stars functions rewrite the star name. The input string 
     may be too short, so we have to make sure we have enough space */
  strcpy_VBsafe(ObjectName, ObjectNameIn);
  default_heliacal_parameters(datm, dgeo, dobs, helflag);
  swe_set_topo(dgeo[0], dgeo[1], dgeo[2]);
  retval = ObjectLoc(JDNDaysUT, dgeo, datm, "sun", 1, helflag, &AziS, serr);
  if (retval == OK)
    retval = ObjectLoc(JDNDaysUT, dgeo, datm, "sun", 0, helflag, &AltS, serr);
  if (retval == OK)
    retval = ObjectLoc(JDNDaysUT, dgeo, datm, ObjectName, 1, helflag, &AziO, serr);
  if (retval == OK)
    retval = ObjectLoc(JDNDaysUT, dgeo, datm, ObjectName, 0, helflag, &AltO, serr);
  if (retval == OK)
    retval = ObjectLoc(JDNDaysUT, dgeo, datm, ObjectName, 7, helflag, &GeoAltO, serr);
  if (retval == ERR)
    return ERR;
  AppAltO = AppAltfromTopoAlt(AltO, datm[1], datm[0], helflag);
  DAZact = AziS - AziO;
  TAVact = AltO - AltS;
  /*this parallax seems to be somewhat smaller then in Yallop and SkyMap! Needs to be studied*/
  ParO = GeoAltO - AltO;
  if (Magnitude(JDNDaysUT, dgeo, ObjectName, helflag, &MagnO, serr) == ERR)
    return ERR;
  ARCVact = TAVact + ParO;
  ARCLact = acos(cos(ARCVact * DEGTORAD) * cos(DAZact * DEGTORAD)) / DEGTORAD;
  Planet = DeterObject(ObjectName);
  if (Planet == -1) {
    elong = ARCLact;
    illum = 100;
  } else {
    retval = swe_pheno_ut(JDNDaysUT, Planet, iflag|(SEFLG_TOPOCTR|SEFLG_EQUATORIAL), attr, serr);
    if (retval == ERR) return ERR;
    elong = attr[2];
    illum = attr[1] * 100;
  }
  kact = kt(AltS, sunra, dgeo[1], dgeo[2], datm[1], datm[2], datm[3], 4, serr);
  if (0) {
darr[26] = kR(AltS, dgeo[2]);
darr[27] = kW(dgeo[2], datm[1], datm[2]);
darr[28] = kOZ(AltS, sunra, dgeo[1]);
darr[29] = ka(AltS, sunra, dgeo[1], dgeo[2], datm[1], datm[2], datm[3], serr);
darr[30] = darr[26] + darr[27] + darr[28] + darr[29];
  }
  WMoon = 0;
  qYal = 0;
  qCrit = 0;
  LMoon = 0;
  if (Planet == SE_MOON) {
    WMoon = WidthMoon(AltO, AziO, AltS, AziS, ParO);
    LMoon = LengthMoon(WMoon, 0);
    qYal = qYallop(WMoon, ARCVact);
    if (qYal > 0.216) qCrit = 1; /* A */
    if (qYal < 0.216 && qYal > -0.014) qCrit = 2; /* B */
    if (qYal < -0.014 && qYal > -0.16) qCrit = 3; /* C */
    if (qYal < -0.16 && qYal > -0.232) qCrit = 4; /* D */
    if (qYal < -0.232 && qYal > -0.293) qCrit = 5; /* E */
    if (qYal < -0.293) qCrit = 6; /* F */
  }
  /*determine if rise or set event*/
  RS = 2;
  if (TypeEvent == 1 || TypeEvent == 4) RS = 1;
  retval = RiseSet(JDNDaysUT - 4.0 / 24.0, dgeo, datm, "sun", RS, helflag, 0, &RiseSetS, serr);
  if (retval == ERR)
    return ERR;
  retval = RiseSet(JDNDaysUT - 4.0 / 24.0, dgeo, datm, ObjectName, RS, helflag, 0, &RiseSetO, serr);
  if (retval == ERR)
    return ERR;
  TbYallop = TJD_INVALID;
  if (retval == -2) { /* object does not rise or set */
    Lag = 0;
    noriseO = TRUE;
  } else {
    Lag = RiseSetO - RiseSetS;
    if (Planet == SE_MOON)
      TbYallop = (RiseSetO * 4 + RiseSetS * 5) / 9.0;
  }
  if ((TypeEvent == 3 || TypeEvent == 4) && (Planet == -1 || Planet >= SE_MARS)) {
    TfirstVR = TJD_INVALID;
    TbVR = TJD_INVALID;
    TlastVR = TJD_INVALID;
    TvisVR = 0;
    MinTAV = 0;
    goto output_heliacal_pheno;
  }
  /* If HPheno >= 11 And HPheno <= 14 Or HPheno = 24 Then*/
  /*te bepalen m.b.v. walkthrough*/
    MinTAVact = 199;
    DeltaAlt = 0; 
    OldestMinTAV = 0;
    Ta = 0;
    Tc = 0;
    TbVR = 0;
    TimeStep = -TimeStepDefault / 24.0 / 60.0;
    if (RS == 2) TimeStep = -TimeStep;
    TimePointer = RiseSetS - TimeStep;
    do {
      TimePointer = TimePointer + TimeStep;
      OldestMinTAV = MinTAVoud;
      MinTAVoud = MinTAVact;
      DeltaAltoud = DeltaAlt;
      retval = ObjectLoc(TimePointer, dgeo, datm, "sun", 0, helflag, &AltS2, serr);
      if (retval == OK)
	retval = ObjectLoc(TimePointer, dgeo, datm, ObjectName, 0, helflag, &AltO2, serr);
      if (retval != OK)
        return ERR;
      DeltaAlt = AltO2 - AltS2;
      if (DeterTAV(dobs, TimePointer, dgeo, datm, ObjectName, helflag, &MinTAVact, serr) == ERR)
        return ERR;
      if (MinTAVoud < MinTAVact && TbVR == 0) {
        /* determine if this is a local minimum with object still above horizon*/
	TimeCheck = TimePointer + Sgn(TimeStep) * LocalMinStep / 24.0 / 60.0;
	if (RiseSetO != 0) {
	  if (TimeStep > 0)
	    TimeCheck = mymin(TimeCheck, RiseSetO);
	  else
	    TimeCheck = mymax(TimeCheck, RiseSetO);
	}
        if (DeterTAV(dobs, TimeCheck, dgeo, datm, ObjectName, helflag, &LocalminCheck, serr) == ERR)
	  return ERR;
	if (LocalminCheck > MinTAVact) {
	  extrax = x2min(MinTAVact, MinTAVoud, OldestMinTAV);
	  TbVR = TimePointer - (1 - extrax) * TimeStep;
	  MinTAV = funct2(MinTAVact, MinTAVoud, OldestMinTAV, extrax);
	}
      }
      if (DeltaAlt > MinTAVact && Tc == 0 && TbVR == 0) {
	crosspoint = crossing(DeltaAltoud, DeltaAlt, MinTAVoud, MinTAVact);
	Tc = TimePointer - TimeStep * (1 - crosspoint);
      }
      if (DeltaAlt < MinTAVact && Ta == 0 && Tc != 0) {
	crosspoint = crossing(DeltaAltoud, DeltaAlt, MinTAVoud, MinTAVact);
	Ta = TimePointer - TimeStep * (1 - crosspoint);
      }
    } while (fabs(TimePointer - RiseSetS) <= MaxTryHours / 24.0 && Ta == 0 && !((TbVR != 0 && (TypeEvent == 3 || TypeEvent == 4) && (strncmp(ObjectName, "moon", 4) != 0 && strncmp(ObjectName, "venus", 5) != 0 && strncmp(ObjectName, "mercury", 7) != 0))));
    if (RS == 2) {
      TfirstVR = Tc;
      TlastVR = Ta;
    } else {
      TfirstVR = Ta;
      TlastVR = Tc;
    }
    if (TfirstVR == 0 && TlastVR == 0) {
      if (RS == 1)
        TfirstVR = TbVR - 0.000001;
      else
        TlastVR = TbVR + 0.000001;
    }
    if (!noriseO) {
      if (RS == 1)
        TfirstVR = mymax(TfirstVR, RiseSetO);
      else
        TlastVR = mymin(TlastVR, RiseSetO);
    }
    TvisVR = TJD_INVALID; /*"#NA!" */
    if (TlastVR != 0 && TfirstVR != 0)
      TvisVR = TlastVR - TfirstVR;
    if (TlastVR == 0) TlastVR = TJD_INVALID; /*"#NA!" */
    if (TbVR == 0) TbVR = TJD_INVALID; /*"#NA!" */
    if (TfirstVR == 0) TfirstVR = TJD_INVALID; /*"#NA!" */
output_heliacal_pheno:
  /*End If*/
  darr[0] = AltO;
  darr[1] = AppAltO;
  darr[2] = GeoAltO;
  darr[3] = AziO;
  darr[4] = AltS;
  darr[5] = AziS;
  darr[6] = TAVact;
  darr[7] = ARCVact;
  darr[8] = DAZact;
  darr[9] = ARCLact;
  darr[10] = kact;
  darr[11] = MinTAV;
  darr[12] = TfirstVR;
  darr[13] = TbVR;
  darr[14] = TlastVR;
  darr[15] = TbYallop;
  darr[16] = WMoon;
  darr[17] = qYal;
  darr[18] = qCrit;
  darr[19] = ParO;
  darr[20] = MagnO;
  darr[21] = RiseSetO;
  darr[22] = RiseSetS;
  darr[23] = Lag;
  darr[24] = TvisVR;
  darr[25] = LMoon;
  darr[26] = elong;
  darr[27] = illum;
  return OK;
}

#if 0
int32 FAR PASCAL_CONV HeliacalJDut(double JDNDaysUTStart, double Age, double SN, double Lat, double Longitude, double HeightEye, double Temperature, double Pressure, double RH, double VR, char *ObjectName, int TypeEvent, char *AVkind, double *dret, char *serr)
{
  double dgeo[3], datm[4], dobs[6];
  int32 helflag = SE_HELFLAG_HIGH_PRECISION;
  helflag |= SE_HELFLAG_AVKIND_VR;
  dgeo[0] = Longitude;
  dgeo[1] = Lat;
  dgeo[2] = HeightEye;
  datm[0] = Pressure;
  datm[1] = Temperature;
  datm[2] = RH;
  datm[3] = VR;
  dobs[0] = Age;
  dobs[1] = SN;
  return swe_heliacal_ut(JDNDaysUTStart, dgeo, datm, dobs, ObjectName, TypeEvent, helflag, dret, serr);
}
#endif

static double get_synodic_period(int Planet) 
{
  /* synodic periods from:
   * Kelley/Milone/Aveni, "Exploring ancient Skies", p. 43. */
  switch(Planet) {
    case SE_MOON: return 29.530588853;
    case SE_MERCURY: return 115.8775;
    case SE_VENUS: return 583.9214;
    case SE_MARS: return 779.9361;
    case SE_JUPITER: return 398.8840;
    case SE_SATURN: return 378.0919;
    case SE_URANUS: return 369.6560;
    case SE_NEPTUNE: return 367.4867;
    case SE_PLUTO: return 366.7207;
  }
  return 366; /* for stars and default for far away planets */
}

/*###################################################################*/
static int32 moon_event_arc_vis(double JDNDaysUTStart, double *dgeo, double *datm, double *dobs, int32 TypeEvent, int32 helflag, double *dret, char *serr)
{
  double x[20], MinTAV, MinTAVoud, OldestMinTAV;
  double phase1, phase2, JDNDaysUT, JDNDaysUTi;
  double tjd_moonevent, tjd_moonevent_start;
  double DeltaAltoud, TimeCheck, LocalminCheck;
  double AltS, AltO, DeltaAlt = 90;
  char ObjectName[30];
  int32 iflag, Daystep, goingup, Planet, retval;
  int32 avkind = helflag & SE_HELFLAG_AVKIND;
  int32 epheflag = helflag & (SEFLG_JPLEPH|SEFLG_SWIEPH|SEFLG_MOSEPH);
  dret[0] = JDNDaysUTStart; /* will be returned in error case */
  if (avkind == 0)
    avkind = SE_HELFLAG_AVKIND_VR;
  if (avkind != SE_HELFLAG_AVKIND_VR) {
    if (serr != NULL)
      strcpy(serr, "error: in valid AV kind for the moon");
    return ERR;
  }
  if (TypeEvent == 1 || TypeEvent == 2) {
    if (serr != NULL)
      strcpy(serr, "error: the moon has no morning first or evening last");
    return ERR;
  }
  strcpy(ObjectName, "moon");
  Planet = SE_MOON;
  iflag = SEFLG_TOPOCTR | SEFLG_EQUATORIAL | epheflag;
  if (!(helflag & SE_HELFLAG_HIGH_PRECISION))
    iflag |= SEFLG_NONUT|SEFLG_TRUEPOS;
  Daystep = 1;
  if (TypeEvent == 3) {
    /*morning last */
    TypeEvent = 2;
  } else {
    /*evening first*/
    TypeEvent = 1;
    Daystep = -Daystep;
  }
  /* check Synodic/phase Period */
  JDNDaysUT = JDNDaysUTStart;
  /* start 30 days later if TypeEvent=4 (1) */
  if (TypeEvent == 1) JDNDaysUT = JDNDaysUT + 30;
  /* determination of new moon date */
  swe_pheno_ut(JDNDaysUT, Planet, iflag, x, serr);
  phase2 = x[0];
  goingup = 0;
  do {
    JDNDaysUT = JDNDaysUT + Daystep;
    phase1 = phase2;
    swe_pheno_ut(JDNDaysUT, Planet, iflag, x, serr);
    phase2 = x[0];
    if (phase2 > phase1) 
      goingup = 1;
  } while (goingup == 0 || (goingup == 1 && (phase2 > phase1)));
  /* fix the date to get the day with the smallest phase (nwest moon) */
  JDNDaysUT = JDNDaysUT - Daystep;
  /* initialize the date to look for set */
  JDNDaysUTi = JDNDaysUT;
  JDNDaysUT = JDNDaysUT - Daystep;
  MinTAVoud = 199;
  do {
    JDNDaysUT = JDNDaysUT + Daystep;
    if ((retval = RiseSet(JDNDaysUT, dgeo, datm, ObjectName, TypeEvent, helflag, 0, &tjd_moonevent, serr)) != OK)
      return retval;
    tjd_moonevent_start = tjd_moonevent;
    MinTAV = 199;
    OldestMinTAV = MinTAV;
    do {
      OldestMinTAV = MinTAVoud;
      MinTAVoud = MinTAV;
      DeltaAltoud = DeltaAlt;
      tjd_moonevent = tjd_moonevent - 1.0 / 60.0 / 24.0 * Sgn(Daystep);
      if (ObjectLoc(tjd_moonevent, dgeo, datm, "sun", 0, helflag, &AltS, serr) == ERR)
	return ERR;
      if (ObjectLoc(tjd_moonevent, dgeo, datm, ObjectName, 0, helflag, &AltO, serr) == ERR)
	return ERR;
      DeltaAlt = AltO - AltS;
      if (DeterTAV(dobs, tjd_moonevent, dgeo, datm, ObjectName, helflag, &MinTAV, serr) == ERR)
        return ERR;
      TimeCheck = tjd_moonevent - LocalMinStep / 60.0 / 24.0 * Sgn(Daystep);
      if (DeterTAV(dobs, TimeCheck, dgeo, datm, ObjectName, helflag, &LocalminCheck, serr) == ERR)
        return ERR;
/*printf("%f, %f <= %f\n", tjd_moonevent, MinTAV, MinTAVoud);*/
    /* while (MinTAV <= MinTAVoud && fabs(tjd_moonevent - tjd_moonevent_start) < 120.0 / 60.0 / 24.0);*/
    } while ((MinTAV <= MinTAVoud || LocalminCheck < MinTAV) && fabs(tjd_moonevent - tjd_moonevent_start) < 120.0 / 60.0 / 24.0);
  /* while (DeltaAlt < MinTAVoud && fabs(JDNDaysUT - JDNDaysUTi) < 15);*/
  } while (DeltaAltoud < MinTAVoud && fabs(JDNDaysUT - JDNDaysUTi) < 15);
  if (fabs(JDNDaysUT - JDNDaysUTi) < 15) {
    tjd_moonevent += (1 - x2min(MinTAV, MinTAVoud, OldestMinTAV)) * Sgn(Daystep) / 60.0 / 24.0;
  } else {
    strcpy(serr, "no date found for lunar event");
    return ERR;
  }
  dret[0] = tjd_moonevent;
  return OK;
}

static int32 heliacal_ut_arc_vis(double JDNDaysUTStart, double *dgeo, double *datm, double *dobs, char *ObjectName, int32 TypeEventIn, int32 helflag, double *dret, char *serr_ret)
{
  double x[6];
  double xin[2];
  double xaz[2];
  double dang[3];
  double objectmagn = 0, maxlength, DayStep;
  double JDNDaysUT, JDNDaysUTfinal, JDNDaysUTstep, JDNDaysUTstepoud, JDNarcvisUT, tjd_tt, tret, OudeDatum, JDNDaysUTinp = JDNDaysUTStart, JDNDaysUTtijd;
  double ArcusVis, ArcusVisDelta, ArcusVisPto, ArcusVisDeltaoud;
  double Trise, sunsangle, Theliacal, Tdelta, Angle;
  double TimeStep, TimePointer, OldestMinTAV, MinTAVoud, MinTAVact, extrax, TbVR = 0;
  double AziS, AltS, AziO, AltO, DeltaAlt;
  double direct, Pressure, Temperature, d;
  int32 epheflag, retval = OK;
  int32 iflag, Planet, eventtype;
  int32 TypeEvent = TypeEventIn;
  int doneoneday;
  char serr[AS_MAXCH];
  *dret = JDNDaysUTStart;
  *serr = '\0';
  Planet = DeterObject(ObjectName);
  Pressure = datm[0];
  Temperature = datm[1];
  /* determine Magnitude of star*/
  if ((retval = Magnitude(JDNDaysUTStart, dgeo, ObjectName, helflag, &objectmagn, serr)) == ERR)
    goto swe_heliacal_err;
  epheflag = helflag & (SEFLG_JPLEPH|SEFLG_SWIEPH|SEFLG_MOSEPH);
  iflag = SEFLG_TOPOCTR | SEFLG_EQUATORIAL | epheflag;
  if (!(helflag & SE_HELFLAG_HIGH_PRECISION)) 
    iflag |= SEFLG_NONUT | SEFLG_TRUEPOS;
  /* start values for search of heliacal rise
   * maxlength = phase period in days, smaller than minimal synodic period */
  /* days per step (for heliacal rise) in power of two */
  switch(Planet) {
    case SE_MERCURY: 
      DayStep = 1; maxlength = 100; break;
    case SE_VENUS: 
      DayStep = 64; maxlength = 384; break;
    case SE_MARS: 
      DayStep = 128; maxlength = 640; break;
    case SE_JUPITER: 
      DayStep = 64; maxlength = 384; break;
    case SE_SATURN: 
      DayStep = 64; maxlength = 256; break;
    default:
      DayStep = 64; maxlength = 256; break;
  }
  /* heliacal setting */
  eventtype = TypeEvent;
  if (eventtype == 2) DayStep = -DayStep;
  /* acronychal setting */
  if (eventtype == 4) {
      eventtype = 1;
      DayStep = -DayStep;
  }
  /* acronychal rising */
  if (eventtype == 3) eventtype = 2;
  eventtype |= SE_BIT_DISC_CENTER;
  /* normalize the maxlength to the step size */
  {
    /* check each Synodic/phase Period */
    JDNDaysUT = JDNDaysUTStart;
    /* make sure one can find an event on the just after the JDNDaysUTStart */
    JDNDaysUTfinal = JDNDaysUT + maxlength;
    JDNDaysUT = JDNDaysUT - 1;
    if (DayStep < 0) {
      JDNDaysUTtijd = JDNDaysUT;
      JDNDaysUT = JDNDaysUTfinal;
      JDNDaysUTfinal = JDNDaysUTtijd;
    }
    /* prepair the search */
    JDNDaysUTstep = JDNDaysUT - DayStep;
    doneoneday = 0;
    ArcusVisDelta = 199;
    ArcusVisPto = -5.55;
    do { /* this is a do {} while() loop */
      if (fabs(DayStep) == 1) doneoneday = 1;
      do { /* this is a do {} while() loop */
	/* init search for heliacal rise */
	JDNDaysUTstepoud = JDNDaysUTstep;
	ArcusVisDeltaoud = ArcusVisDelta;
	JDNDaysUTstep = JDNDaysUTstep + DayStep;
	/* determine rise/set time */
	if ((retval = my_rise_trans(JDNDaysUTstep, SE_SUN, "", eventtype, helflag, dgeo, datm, &tret, serr)) == ERR)
	  goto swe_heliacal_err;
	/* determine time compensation to get Sun's altitude at heliacal rise */
	tjd_tt = tret + DeltaT(tret, 0) / D2S;
	if ((retval = swe_calc(tjd_tt, SE_SUN, iflag, x, serr)) == ERR)
	  goto swe_heliacal_err;
	xin[0] = x[0];
	xin[1] = x[1];
	swe_azalt(tret, SE_EQU2HOR, dgeo, Pressure, Temperature, xin, xaz);
	Trise = HourAngle(xaz[1], x[1], dgeo[1]);
	sunsangle = ArcusVisPto;
	if (helflag & SE_HELFLAG_AVKIND_MIN7) sunsangle = -7;
	if (helflag & SE_HELFLAG_AVKIND_MIN9) sunsangle = -9;
	Theliacal = HourAngle(sunsangle, x[1], dgeo[1]);
	Tdelta = Theliacal - Trise;
	if (TypeEvent == 2 || TypeEvent== 3) Tdelta = -Tdelta;
	/* determine appr.time when sun is at the wanted Sun's altitude */
	JDNarcvisUT = tret - Tdelta / 24;
	tjd_tt = JDNarcvisUT + DeltaT(JDNarcvisUT, 0) / D2S;
	/* determine Sun's position */
	if ((retval = swe_calc(tjd_tt, SE_SUN, iflag, x, serr)) == ERR)
	  goto swe_heliacal_err;
	xin[0] = x[0];
	xin[1] = x[1];
	swe_azalt(JDNarcvisUT, SE_EQU2HOR, dgeo, Pressure, Temperature, xin, xaz);
	AziS = xaz[0] + 180;
	if (AziS >= 360) AziS = AziS - 360;
	AltS = xaz[1];
	/* determine Moon's position */
#if 0
  double AltM, AziM;
	if ((retval = swe_calc(tjd_tt, SE_MOON, iflag, x, serr)) == ERR)
	  goto swe_heliacal_err;
	xin[0] = x[0];
	xin[1] = x[1];
	swe_azalt(JDNarcvisUT, SE_EQU2HOR, dgeo, Pressure, Temperature, xin, xaz);
	AziM = xaz[0] + 180;
	if (AziM >= 360) AziM = AziM - 360;
	AltM = xaz[1];
#endif
	/* determine object's position */
	if (Planet != -1) {
	  if ((retval = swe_calc(tjd_tt, Planet, iflag, x, serr)) == ERR)
	    goto swe_heliacal_err;
	  /* determine magnitude of Planet */
	  if ((retval = Magnitude(JDNarcvisUT, dgeo, ObjectName, helflag, &objectmagn, serr)) == ERR)
	    goto swe_heliacal_err;
	} else {
	  if ((retval = call_swe_fixstar(ObjectName, tjd_tt, iflag, x, serr)) == ERR)
	    goto swe_heliacal_err;
	}
	xin[0] = x[0];
	xin[1] = x[1];
	swe_azalt(JDNarcvisUT, SE_EQU2HOR, dgeo, Pressure, Temperature, xin, xaz);
	AziO = xaz[0] + 180;
	if (AziO >= 360) AziO = AziO - 360;
	AltO = xaz[1];
	/* determine arcusvisionis */
	DeltaAlt = AltO - AltS;
	/*if ((retval = HeliacalAngle(objectmagn, dobs, AziO, AltM, AziM, JDNarcvisUT, AziS, dgeo, datm, helflag, dang, serr)) == ERR)*/
	if ((retval = HeliacalAngle(objectmagn, dobs, AziO, -1, 0, JDNarcvisUT, AziS, dgeo, datm, helflag, dang, serr)) == ERR)
	  goto swe_heliacal_err;
	ArcusVis = dang[1];
	ArcusVisPto = dang[2];
	ArcusVisDelta = DeltaAlt - ArcusVis;
      /*} while (((ArcusVisDeltaoud > 0 && ArcusVisDelta < 0) || ArcusVisDelta < 0) && (JDNDaysUTfinal - JDNDaysUTstep) * Sgn(DayStep) > 0);*/
      } while ((ArcusVisDeltaoud > 0 || ArcusVisDelta < 0) && (JDNDaysUTfinal - JDNDaysUTstep) * Sgn(DayStep) > 0);
      if (doneoneday == 0 && (JDNDaysUTfinal - JDNDaysUTstep) * Sgn(DayStep) > 0) {
	/* go back to date before heliacal altitude */
	ArcusVisDelta = ArcusVisDeltaoud;
	DayStep = ((int) (fabs(DayStep) / 2.0)) * Sgn(DayStep);
	JDNDaysUTstep = JDNDaysUTstepoud;
      }
    } while (doneoneday == 0 && (JDNDaysUTfinal - JDNDaysUTstep) * Sgn(DayStep) > 0);
  }
  d = (JDNDaysUTfinal - JDNDaysUTstep) * Sgn(DayStep);
  if (d <= 0 || d >= maxlength) {
    dret[0] = JDNDaysUTinp; /* no date found, just return input */
    retval = -2; /* marks "not found" within synodic period */
    sprintf(serr, "heliacal event not found within maxlength %f\n", maxlength);
    goto swe_heliacal_err;
  } 
#if 0
  if (helflag & SE_HELFLAG_AVKIND_VR) {
    double darr[40];
    if (swe_heliacal_pheno_ut(JDNarcvisUT, dgeo, datm, dobs, ObjectName, TypeEvent, helflag, darr, serr) != OK)
      return ERR;
    JDNarcvisUT = darr[13];
    }
  }
#endif
  direct = TimeStepDefault / 24.0 / 60.0;
  if (DayStep < 0) direct = -direct;
  if (helflag & SE_HELFLAG_AVKIND_VR) {
    /*te bepalen m.b.v. walkthrough*/
    TimeStep = direct;
    TbVR = 0;
    TimePointer = JDNarcvisUT;
    if (DeterTAV(dobs, TimePointer, dgeo, datm, ObjectName, helflag, &OldestMinTAV, serr) == ERR)
      return ERR;
    TimePointer = TimePointer + TimeStep;
    if (DeterTAV(dobs, TimePointer, dgeo, datm, ObjectName, helflag, &MinTAVoud, serr) == ERR)
      return ERR;
    if (MinTAVoud > OldestMinTAV) {
      TimePointer = JDNarcvisUT;
      TimeStep = -TimeStep;
      MinTAVact = OldestMinTAV;
    } else {
      MinTAVact = MinTAVoud;
      MinTAVoud = OldestMinTAV;
    }
    /*TimePointer = TimePointer - Timestep*/
    do {
      TimePointer = TimePointer + TimeStep;
      OldestMinTAV = MinTAVoud;
      MinTAVoud = MinTAVact;
      if (DeterTAV(dobs, TimePointer, dgeo, datm, ObjectName, helflag, &MinTAVact, serr) == ERR)
        return ERR;
      if (MinTAVoud < MinTAVact) {
	extrax = x2min(MinTAVact, MinTAVoud, OldestMinTAV);
	TbVR = TimePointer - (1 - extrax) * TimeStep;
      }
    } while (TbVR == 0);
    JDNarcvisUT = TbVR;
  }
  /*if (strncmp(AVkind, "pto", 3) == 0) */
  if (helflag & SE_HELFLAG_AVKIND_PTO) {
    do {
      OudeDatum = JDNarcvisUT;
      JDNarcvisUT = JDNarcvisUT - direct;
      tjd_tt = JDNarcvisUT + DeltaT(JDNarcvisUT, 0) / D2S;
      if (Planet != -1) {
	if ((retval = swe_calc(tjd_tt, Planet, iflag, x, serr)) == ERR)
	  goto swe_heliacal_err;
      } else {
	if ((retval = call_swe_fixstar(ObjectName, tjd_tt, iflag, x, serr)) == ERR)
	  goto swe_heliacal_err;
      }
      xin[0] = x[0];
      xin[1] = x[1];
      swe_azalt(JDNarcvisUT, SE_EQU2HOR, dgeo, Pressure, Temperature, xin, xaz);
      Angle = xaz[1];
    } while (Angle > 0);
    JDNarcvisUT = (JDNarcvisUT + OudeDatum) / 2.0;
  }
  if (JDNarcvisUT < -9999999 || JDNarcvisUT > 9999999) {
    dret[0] = JDNDaysUT; /* no date found, just return input */
    strcpy(serr, "no heliacal date found");
    retval = ERR;
    goto swe_heliacal_err;
  }
  dret[0] = JDNarcvisUT;
swe_heliacal_err:
  if (serr_ret != NULL && *serr != '\0')
    strcpy(serr_ret, serr);
  return retval;
}

static int32 get_asc_obl(double tjd, int32 ipl, char *star, int32 iflag, double
 *dgeo, AS_BOOL desc_obl, double *daop, char *serr)
{
  int32 retval;
  int32 epheflag = iflag & (SEFLG_JPLEPH|SEFLG_SWIEPH|SEFLG_MOSEPH);
  double x[6], adp;
  char s[AS_MAXCH];
  char star2[AS_MAXCH];
  strcpy(star2, star);
  if (ipl == -1) {
    if ((retval = swe_fixstar(star2, tjd, epheflag | SEFLG_EQUATORIAL, x, serr)) == ERR)
      return ERR;
  } else {
    if ((retval = swe_calc(tjd, ipl, epheflag | SEFLG_EQUATORIAL, x, serr)) == ERR)
      return ERR;
  }
  adp = tan(dgeo[1] * DEGTORAD) * tan(x[1] * DEGTORAD);
  if (fabs(adp) > 1) {
    if (star != NULL && *star != '\0')
      strcpy(s, star);
    else
      swe_get_planet_name(ipl, s);
    sprintf(serr, "%s is circumpolar, cannot calculate heliacal event", s);
    return -2;
  }
  adp = asin(adp) / DEGTORAD;
  if (desc_obl)
    *daop = x[0] + adp;
  else
    *daop = x[0] - adp;
  *daop = swe_degnorm(*daop);
  return OK;
}

#if 0
static int32 get_asc_obl_old(double tjd, int32 ipl, char *star, int32 iflag, double *dgeo, AS_BOOL desc_obl, double *daop, char *serr)
{
  int32 retval;
  int32 epheflag = iflag & (SEFLG_JPLEPH|SEFLG_SWIEPH|SEFLG_MOSEPH);
  double x[6], adp;
  char s[AS_MAXCH];
  if (star != NULL && *star != '\0') {
    if ((retval = call_swe_fixstar(star, tjd, epheflag | SEFLG_EQUATORIAL, x, serr)) == ERR)
      return ERR;
  } else {
    if ((retval = swe_calc(tjd, ipl, epheflag | SEFLG_EQUATORIAL, x, serr)) == ERR)
      return ERR;
  }
  adp = tan(dgeo[1] * DEGTORAD) * tan(x[1] * DEGTORAD); 
  if (fabs(adp) > 1) {
    if (star != NULL && *star != '\0') 
      strcpy(s, star);
    else 
      swe_get_planet_name(ipl, s);
    sprintf(serr, "%s is circumpolar, cannot calculate heliacal event", s);
    return -2;
  }
  adp = asin(adp) / DEGTORAD;
  if (desc_obl)
    *daop = x[0] + adp;
  else
    *daop = x[0] - adp;
  *daop = swe_degnorm(*daop);
  return OK;
}
#endif

static int32 get_asc_obl_diff(double tjd, int32 ipl, char *star, int32 iflag, double *dgeo, AS_BOOL desc_obl, AS_BOOL is_acronychal, double *dsunpl, char *serr)
{
  int32 retval = OK;
  double aosun, aopl;
  /* ascensio obliqua of sun */
  retval = get_asc_obl(tjd, SE_SUN, "", iflag, dgeo, desc_obl, &aosun, serr);
  if (retval != OK)
    return retval;
  if (is_acronychal) {
    if (desc_obl == TRUE)
      desc_obl = FALSE;
    else
      desc_obl = TRUE;
  }
  /* ascensio obliqua of body */
  retval = get_asc_obl(tjd, ipl, star, iflag, dgeo, desc_obl, &aopl, serr);
  if (retval != OK)
    return retval;
  *dsunpl = swe_degnorm(aosun - aopl);
  if (is_acronychal)
    *dsunpl = swe_degnorm(*dsunpl - 180);
  if (*dsunpl > 180) *dsunpl -= 360;
  return OK;
}

#if 0
static int32 get_asc_obl_diff_old(double tjd, int32 ipl, char *star, int32 iflag, double *dgeo, AS_BOOL desc_obl, double *dsunpl, char *serr)
{
  int32 retval = OK;
  double aosun, aopl;
  /* ascensio obliqua of sun */
  retval = get_asc_obl(tjd, SE_SUN, "", iflag, dgeo, desc_obl, &aosun, serr);
  if (retval != OK)
    return retval;
  /* ascensio obliqua of body */
  retval = get_asc_obl(tjd, ipl, star, iflag, dgeo, desc_obl, &aopl, serr);
  if (retval != OK)
    return retval;
  *dsunpl = swe_degnorm(aosun - aopl);
  return OK;
}
#endif

/* times of 
 * - superior and inferior conjunction (Mercury and Venus)
 * - conjunction and opposition (ipl >= Mars)
 */
static double tcon[] =
{
  0, 0, 
  2451550, 2451550,  /* Moon */
  2451604, 2451670,  /* Mercury */
  2451980, 2452280,  /* Venus */
  2451727, 2452074,  /* Mars */
  2451673, 2451877,  /* Jupiter */ 
  2451675, 2451868,  /* Saturn */
  2451581, 2451768,  /* Uranus */ 
  2451568, 2451753,  /* Neptune */
};

static int32 find_conjunct_sun(double tjd_start, int32 ipl, int32 helflag, int32 TypeEvent, double *tjd, char *serr)
{
  int32 epheflag = helflag & (SEFLG_JPLEPH|SEFLG_SWIEPH|SEFLG_MOSEPH);
  int i;
  double tjdcon, tjd0, ds, dsynperiod, x[6], xs[6], daspect = 0;
  if (ipl >= SE_MARS && TypeEvent >= 3)
    daspect = 180;
  i = (TypeEvent - 1) / 2 + ipl * 2;
  tjd0 = tcon[i];
  dsynperiod = get_synodic_period(ipl);
  tjdcon = tjd0 + ((floor) ((tjd_start - tjd0) / dsynperiod) + 1) * dsynperiod;
  ds = 100;
  while (ds > 0.5) {
    if (swe_calc(tjdcon, ipl, epheflag|SEFLG_SPEED, x, serr) == ERR)
      return ERR;
    if (swe_calc(tjdcon, SE_SUN, epheflag|SEFLG_SPEED, xs, serr) == ERR)
      return ERR;
    ds = swe_degnorm(x[0] - xs[0] - daspect);
    if (ds > 180) ds -= 360;
    tjdcon -= ds / (x[3] - xs[3]);
  }
  *tjd = tjdcon;
  return OK;
}

static int32 get_asc_obl_with_sun(double tjd_start, int32 ipl, char *star, int32 helflag, int32 evtyp, double dperiod, double *dgeo, double *tjdret, char *serr)
{
  int i, retval;
  int32 is_acronychal = FALSE;
  int32 epheflag = helflag & (SEFLG_JPLEPH|SEFLG_SWIEPH|SEFLG_MOSEPH);
  double dsunpl = 1, dsunpl_save, dsunpl_test, tjd, daystep;
  AS_BOOL desc_obl = FALSE, retro = FALSE;
  if (evtyp == SE_EVENING_LAST || evtyp == SE_EVENING_FIRST)
    desc_obl = TRUE;
  if (evtyp == SE_MORNING_FIRST || evtyp == SE_EVENING_LAST)
    retro = TRUE;
  if (evtyp == SE_ACRONYCHAL_RISING)
    desc_obl = TRUE;
  if (evtyp == SE_ACRONYCHAL_RISING || evtyp ==  SE_ACRONYCHAL_SETTING) {
    is_acronychal = TRUE;
    if (ipl != SE_MOON)
      retro = TRUE;
  }
  //  if (evtyp == 3 || evtyp == 4)
  //    dangsearch = 180;
  /* find date when sun and object have the same ascensio obliqua */
  tjd = tjd_start;
  dsunpl_save = -999999999;
  retval = get_asc_obl_diff(tjd, ipl, star, epheflag, dgeo, desc_obl, is_acronychal, &dsunpl, serr);
  if (retval != OK)  /* retval may be ERR or -2 */
    return retval;
  daystep = 20;
  i = 0;
  while (dsunpl_save == -999999999 ||
      /*fabs(dsunpl - dsunpl_save) > 180 ||*/
      fabs(dsunpl) + fabs(dsunpl_save) > 180 ||
      (retro && !(dsunpl_save < 0 && dsunpl >= 0)) ||
      (!retro && !(dsunpl_save >= 0 && dsunpl < 0))) {
    i++;
    if (i > 5000) {
      sprintf(serr, "loop in get_asc_obl_with_sun() (1)");
      return ERR;
    }
    dsunpl_save = dsunpl;
    tjd += 10.0;
    if (dperiod > 0 && tjd - tjd_start > dperiod)
      return -2;
    retval = get_asc_obl_diff(tjd, ipl, star, epheflag, dgeo, desc_obl, is_acronychal, &dsunpl, serr);
    if (retval != OK)  /* retval may be ERR or -2 */
      return retval;
  }
  tjd_start = tjd - daystep;
  daystep /= 2.0;
  tjd = tjd_start + daystep;
  retval = get_asc_obl_diff(tjd, ipl, star, epheflag, dgeo, desc_obl, is_acronychal, &dsunpl_test, serr);
  if (retval != OK)  /* retval may be ERR or -2 */
    return retval;
  i = 0;
  while (fabs(dsunpl) > 0.00001) {
    i++;
    if (i > 5000) {
      sprintf(serr, "loop in get_asc_obl_with_sun() (2)");
      return ERR;
    }
    if (dsunpl_save * dsunpl_test >= 0) {
      dsunpl_save = dsunpl_test;
      tjd_start = tjd;
    } else {
      dsunpl = dsunpl_test;
    }
    daystep /= 2.0;
    tjd = tjd_start + daystep;
    retval = get_asc_obl_diff(tjd, ipl, star, epheflag, dgeo, desc_obl, is_acronychal, &dsunpl_test, serr);
    if (retval != OK)  /* retval may be ERR or -2 */
      return retval;
  }
  *tjdret = tjd;
  return OK;
}

#if 0
/* works only for fixed stars */
static int32 get_asc_obl_with_sun_old(double tjd_start, int32 ipl, char *star, int32 helflag, int32 TypeEvent, double *dgeo, double *tjdret, char *serr)
{
  int retval;
  int32 epheflag = helflag & (SEFLG_JPLEPH|SEFLG_SWIEPH|SEFLG_MOSEPH);
  double dsunpl = 1, tjd, daystep, dsunpl_save;
  double dsynperiod = 367;
  double dangsearch = 0;
  AS_BOOL desc_obl = FALSE;
  if (TypeEvent == 2 || TypeEvent == 3)
    desc_obl = TRUE;
  if (TypeEvent == 3 || TypeEvent == 4)
    dangsearch = 180;
  /* find date when sun and object have the same ascensio obliqua */
  daystep = dsynperiod;
  tjd = tjd_start;
  retval = get_asc_obl_diff(tjd, ipl, star, epheflag, dgeo, desc_obl, &dsunpl, serr);
  if (retval != OK)  /* retval may be ERR or -2 */
    return retval;
  while (dsunpl < 359.99999) {
    dsunpl_save = dsunpl;
    daystep /= 2.0;
    retval = get_asc_obl_diff(tjd + daystep, ipl, star, epheflag, dgeo, desc_obl, &dsunpl, serr);
    if (retval != OK)  /* retval may be ERR or -2 */
      return retval;
    if (dsunpl > dsunpl_save)
      tjd += daystep;
    else
      dsunpl = dsunpl_save;
  }
  *tjdret = tjd;
  return OK;
}
#endif

#if 0
/* works only for fixed stars */
static int32 get_asc_obl_acronychal(double tjd_start, int32 ipl, char *star, int32 helflag, int32 TypeEvent, double *dgeo, double *tjdret, char *serr)
{
  int retval;
  int32 epheflag = helflag & (SEFLG_JPLEPH|SEFLG_SWIEPH|SEFLG_MOSEPH);
  double dsunpl = 1, tjd, daystep, dsunpl_save;
  double dsynperiod = 367;
  double aosun, aopl;
  AS_BOOL sun_desc = TRUE, obj_desc = FALSE;
  daystep = dsynperiod;
  tjd = tjd_start;
  if (TypeEvent == 4) {
    sun_desc = FALSE; 
    obj_desc = TRUE;
  }
  /* ascensio (descensio) obliqua of sun */
  retval = get_asc_obl(tjd, SE_SUN, "", epheflag, dgeo, sun_desc, &aosun, serr);
  if (retval != OK)  /* retval may be ERR or -2 */
    return retval;
  /* ascensio (descensio) obliqua of body */
  retval = get_asc_obl(tjd, ipl, star, epheflag, dgeo, obj_desc, &aopl, serr);
  if (retval != OK)  /* retval may be ERR or -2 */
    return retval;
  dsunpl = swe_degnorm(aosun - aopl + 180);
  while (dsunpl < 359.99999) {
    dsunpl_save = dsunpl;
    daystep /= 2.0;
    /* ascensio (descensio) obliqua of sun */
    retval = get_asc_obl(tjd+daystep, SE_SUN, "", epheflag, dgeo, sun_desc, &aosun, serr);
    if (retval != OK)  /* retval may be ERR or -2 */
      return retval;
    /* ascensio (descensio) obliqua of body */
    retval = get_asc_obl(tjd+daystep, ipl, star, epheflag, dgeo, obj_desc, &aopl, serr);
    if (retval != OK)  /* retval may be ERR or -2 */
      return retval;
    dsunpl = swe_degnorm(aosun - aopl + 180);
    if (dsunpl > dsunpl_save)
      tjd += daystep;
    else
      dsunpl = dsunpl_save;
  }
  *tjdret = tjd;
  return OK;
}
#endif

static int32 get_heliacal_day(double tjd, double *dgeo, double *datm, double *dobs, char *ObjectName, int32 helflag, int32 TypeEvent, double *thel, char *serr)
{
  int32 is_rise_or_set = 0, ndays, retval, retval_old;
  double direct_day = 0, direct_time = 0, tfac, tend, daystep, tday, vdelta, tret;
  double darr[30], vd, dmag;
  int32 ipl = DeterObject(ObjectName);
  /* 
   * find the day and minute on which the object becomes visible 
   */
  switch (TypeEvent) {
    /* morning first */
    case 1: is_rise_or_set = SE_CALC_RISE; 
      direct_day = 1; direct_time = -1;
      break;
    /* evening last */
    case 2: is_rise_or_set = SE_CALC_SET; 
      direct_day = -1; direct_time = 1;
      break;
    /* evening first */
    case 3: is_rise_or_set = SE_CALC_SET; 
      direct_day = 1; direct_time = 1;
      break;
    /* morning last */
    case 4: is_rise_or_set = SE_CALC_RISE; 
      direct_day = -1; direct_time = -1;
      break;
  }
  tfac = 1;
  switch (ipl) {
    case SE_MOON: 
      ndays = 16; 
      daystep = 1;
      break;
    case SE_MERCURY: 
      ndays = 60; tjd -= 0 * direct_day; 
      daystep = 5;
      tfac = 5;
      break;
    case SE_VENUS: 
      ndays = 300; tjd -= 30 * direct_day; 
      daystep = 5;
      if (TypeEvent >= 3) {
	daystep = 15;
        tfac = 3;
      }
      break;
    case SE_MARS: 
      ndays = 400; 
      daystep = 15;
      tfac = 5;
      break; 
    case SE_SATURN: 
      ndays = 300; 
      daystep = 20;
      tfac = 5;
      break; 
    case -1:
      ndays = 300;
      if (call_swe_fixstar_mag(ObjectName, &dmag, serr) == ERR) {
	return ERR;
      }
      daystep = 15;
      tfac = 10;
      if (dmag > 2) {
        daystep = 15;
      }
      if (dmag < 0) {
	tfac = 3;
      }
      break;
    default:
      ndays = 300; 
      daystep = 15;
      tfac = 3;
      break;
  }
  tend = tjd + ndays * direct_day;
  retval_old = -2;
  for (tday = tjd;
       (direct_day > 0 && tday < tend) || (direct_day < 0 && tday > tend);
       tday += daystep * direct_day) {
    vdelta = -100; 
    if ((retval = my_rise_trans(tday, SE_SUN, "", is_rise_or_set, helflag, dgeo, datm, &tret, serr)) == ERR) {
      return ERR;
    }
    /* sun does not rise: try next day */
    if (retval == -2) {
      retval_old = retval;
      continue;
    }
    retval = swe_vis_limit_mag(tret, dgeo, datm, dobs, ObjectName, helflag, darr, serr);
    if (retval == ERR)
      return ERR;
#if 1
    /*  object has appeared above horizon: reduce daystep */
    if (retval_old == -2 && retval >= 0 && daystep > 1) {
      retval_old = retval;
      tday -= daystep * direct_day;
      daystep = 1;
      /* Note: beyond latitude 55N (?), Mars can have a morning last. 
       * If the period of visibility is less than 5 days, we may miss the
       * event. I don't know if this happens */
      if (ipl >= SE_MARS || ipl == -1)
        daystep = 5;
      continue;
    }
    retval_old = retval;
#endif
    /*  object below horizon: try next day */
    if (retval == -2)
      continue;
    vdelta = darr[0] - darr[7];
    /* find minute of object's becoming visible */
    while (retval != -2 && (vd = darr[0] - darr[7]) < 0) {
      if (vd < -1.0)
	tret += 5.0 / 1440.0 * direct_time * tfac;
      else if (vd < -0.5)
	tret += 2.0 / 1440.0 * direct_time * tfac;
      else if (vd < -0.1)
	tret += 1.0 / 1440.0 * direct_time * tfac;
      else 
	tret += 1.0 / 1440.0 * direct_time;
      retval = swe_vis_limit_mag(tret, dgeo, datm, dobs, ObjectName, helflag, darr, serr);
      if (retval == ERR)
	return ERR;
    }
    vdelta = darr[0] - darr[7];
    /* object is visible, save time of appearance */
    if (vdelta > 0) {
      if ((ipl >= SE_MARS || ipl == -1) && daystep > 1) {
	tday -= daystep * direct_day;
	daystep = 1;
      } else {
	*thel = tret;
	return OK;
      }
    }
  }
  sprintf(serr, "heliacal event does not happen");
  return -2;
}

#if 0
static int32 get_acronychal_day_new(double tjd, double *dgeo, double *datm, double *dobs, char *ObjectName, int32 helflag, int32 TypeEvent, double *thel, char *serr) {
  double tjdc = tjd, tret, x[6], xaz[6], AltO = -10;
  int32 retval, is_rise_or_set, iter_day;
  int32 ipl = DeterObject(ObjectName);
  int32 epheflag = helflag & (SEFLG_JPLEPH|SEFLG_SWIEPH|SEFLG_MOSEPH);
  int32 iflag = epheflag | SEFLG_EQUATORIAL | SEFLG_TOPOCTR;
  if ((retval = my_rise_trans(tret, 0, ObjectName, SE_CALC_RISE, helflag, dgeo, datm, &tret, serr)) == ERR) return ERR;
  trise = tret;
  tret += 0.01
  if ((retval = my_rise_trans(tret, 0, ObjectName, SE_CALC_SET, helflag, dgeo, datm, &tret, serr)) == ERR) return ERR;
  trise = tset;

  *thel = tret;
  return OK;
}
#endif

#if 0
static int32 get_acronychal_day_old(double tjd, double *dgeo, double *datm, double *dobs, char *ObjectName, int32 helflag, int32 TypeEvent, double *thel, char *serr) {
  double tjdc = tjd, tret, x[6], xaz[6], AltO = -10;
  int32 retval, is_rise_or_set, iter_day;
  int32 ipl = DeterObject(ObjectName);
  int32 epheflag = helflag & (SEFLG_JPLEPH|SEFLG_SWIEPH|SEFLG_MOSEPH);
  int32 iflag = epheflag | SEFLG_EQUATORIAL | SEFLG_TOPOCTR;
  if (TypeEvent == 3) {
    is_rise_or_set = SE_CALC_SET; 
    tret = tjdc - 3;
    if (ipl >= SE_MARS)
      tret = tjdc - 3;
    iter_day = 1;
  } else {
    is_rise_or_set = SE_CALC_RISE; 
    tret = tjdc + 3;
    if (ipl >= SE_MARS)
      tret = tjdc + 3;
    iter_day = -1;
  }
  while (AltO < 0) {
    tret += 0.3 * iter_day;
    if (iter_day == -1)
      tret -= 1;
    retval = my_rise_trans(tret, SE_SUN, "", is_rise_or_set, helflag, dgeo, datm, &tret, serr);
    if (retval != OK)
      return retval;
    /* determine object's position */
    if (ipl == -1)
      retval = call_swe_fixstar(ObjectName, tret+swe_deltat(tret), iflag, x, serr);
    else 
      retval = swe_calc(tret+swe_deltat(tret), ipl, iflag, x, serr);
    if (retval == ERR) return ERR;
    swe_azalt(tret, SE_EQU2HOR, dgeo, datm[0], datm[1], x, xaz);
    AltO = xaz[2];
  }
  *thel = tret;
  return OK;
}
#endif

static int32 time_optimum_visibility(double tjd, double *dgeo, double *datm, double *dobs, char *ObjectName, int32 helflag, double *tret, char *serr)
{
  int32 retval, retval_sv, i;
  double d, vl, darr[10], phot_scot_opic, phot_scot_opic_sv;
  *tret = tjd;
  retval = swe_vis_limit_mag(tjd, dgeo, datm, dobs, ObjectName, helflag, darr, serr);
  if (retval == ERR) return ERR;
  retval_sv = retval;
  vl = darr[0] - darr[7];
  phot_scot_opic_sv = retval & SE_SCOTOPIC_FLAG;
  for (i = 0, d = 100.0 / 86400.0; i < 3; i++, d /= 10.0) {
    while((retval = swe_vis_limit_mag(tjd - d, dgeo, datm, dobs, ObjectName, helflag, darr, serr)) >= 0 
	&& darr[0] > darr[7] 
	&& darr[0] - darr[7] > vl) { 
      tjd -= d; vl = darr[0] - darr[7]; 
      retval_sv = retval;
      phot_scot_opic_sv = retval & SE_SCOTOPIC_FLAG;
    /*  printf("1: %f\n", darr[8]);*/
    }
    if (retval == ERR) return ERR;
    while((retval = swe_vis_limit_mag(tjd + d, dgeo, datm, dobs, ObjectName, helflag, darr, serr)) >= 0 
        && darr[0] > darr[7] 
	&& darr[0] - darr[7] > vl) { 
      tjd += d; vl = darr[0] - darr[7]; 
      retval_sv = retval;
      phot_scot_opic_sv = retval & SE_SCOTOPIC_FLAG;
    /*  printf("2: %f\n", darr[8]);*/
    }
    if (retval == ERR) return ERR;
  }
    /*  printf("3: %f <-> %f\n", darr[8], phot_scot_opic_sv);*/
  *tret = tjd;
  if (retval >= 0) {
    /* search for optimum came to an end because change scotopic/photopic: */
    phot_scot_opic = (retval & SE_SCOTOPIC_FLAG);
    if (phot_scot_opic_sv != phot_scot_opic) {
      /* calling function writes warning into serr */
      return -2;
    }
    /* valid result found but it is close to the scotopic/photopic limit */
    if (retval_sv & SE_MIXEDOPIC_FLAG) {
      return -2;
    }
  }
  return OK;
}

static int32 time_limit_invisible(double tjd, double *dgeo, double *datm, double *dobs, char *ObjectName, int32 helflag, int32 direct, double *tret, char *serr)
{
  int32 retval, retval_sv, i, ncnt = 3;
  double d = 0, darr[10], phot_scot_opic, phot_scot_opic_sv;
  double d0 = 100.0 / 86400.0;
  *tret = tjd;
  if (strcmp(ObjectName, "moon") == 0) {
    d0 *= 10;
    ncnt = 4;
  }
  retval = swe_vis_limit_mag(tjd + d * direct, dgeo, datm, dobs, ObjectName, helflag, darr, serr);
  if (retval == ERR) return ERR;
  retval_sv = retval;
  phot_scot_opic_sv = retval & SE_SCOTOPIC_FLAG;
  for (i = 0, d = d0; i < ncnt; i++, d /= 10.0) {
    while((retval = swe_vis_limit_mag(tjd + d * direct, dgeo, datm, dobs, ObjectName, helflag, darr, serr)) >= 0 
        && darr[0] > darr[7]) { 
      tjd += d * direct; 
      retval_sv = retval;
      phot_scot_opic_sv = retval & SE_SCOTOPIC_FLAG;
  /*    printf("%d: %f\n", direct, darr[8]); */
    }
  }
   /*   printf("4: %f, %f/%f %f <-> %f\n", darr[8], darr[0], darr[7], tjd, phot_scot_opic_sv); */
  *tret = tjd;
  /* if object disappears at setting, retval is -2, but we want it OK, and
   * also suppress the warning "object is below local horizon" */
  *serr = '\0'; 
  if (retval >= 0) {
    /* search for limit came to an end because change scotopic/photopic: */
    phot_scot_opic = (retval & SE_SCOTOPIC_FLAG);
    if (phot_scot_opic_sv != phot_scot_opic) {
      /* calling function writes warning into serr */
      return -2;
    }
    /* valid result found but it is close to the scotopic/photopic limit */
    if (retval_sv & SE_MIXEDOPIC_FLAG) {
      return -2;
    }
  }
  return OK;
}

static int32 get_acronychal_day(double tjd, double *dgeo, double *datm, double *dobs, char *ObjectName, int32 helflag, int32 TypeEvent, double *thel, char *serr) {
  double tret, tret_dark, darr[30], dtret;
  /* x[6], xaz[6], alto, azio, alto_dark, azio_dark;*/
  int32 retval, is_rise_or_set, direct;
  int32 ipl = DeterObject(ObjectName);
  helflag |= SE_HELFLAG_VISLIM_PHOTOPIC;
  /*int32 epheflag = helflag & (SEFLG_JPLEPH|SEFLG_SWIEPH|SEFLG_MOSEPH);*/
  /* int32 iflag = epheflag | SEFLG_EQUATORIAL | SEFLG_TOPOCTR;*/
  if (TypeEvent == 3 || TypeEvent == 5) {
    is_rise_or_set = SE_CALC_RISE; 
    /* tret = tjdc - 3;
    if (ipl >= SE_MARS)
      tret = tjdc - 3;*/
    direct = -1;
  } else {
    is_rise_or_set = SE_CALC_SET; 
    /*tret = tjdc + 3;
    if (ipl >= SE_MARS)
      tret = tjdc + 3;*/
    direct = 1;
  }
  dtret = 999;
#if 0
  while (fabs(dtret) > 0.5) {
#else
  while (fabs(dtret) > 0.5 / 1440.0) {
#endif
    tjd += 0.7 * direct;
    if (direct < 0) tjd -= 1;
    retval = my_rise_trans(tjd, ipl, ObjectName, is_rise_or_set, helflag, dgeo, datm, &tjd, serr);
    if (retval == ERR) return ERR;
    retval = swe_vis_limit_mag(tjd, dgeo, datm, dobs, ObjectName, helflag, darr, serr);
    while(darr[0] < darr[7]) {
      tjd += 10.0 / 1440.0 * -direct;
      retval = swe_vis_limit_mag(tjd, dgeo, datm, dobs, ObjectName, helflag, darr, serr);
    }
    retval = time_limit_invisible(tjd, dgeo, datm, dobs, ObjectName, helflag | SE_HELFLAG_VISLIM_DARK, direct, &tret_dark, serr);
    if (retval == ERR) return ERR;
    retval = time_limit_invisible(tjd, dgeo, datm, dobs, ObjectName, helflag | SE_HELFLAG_VISLIM_NOMOON, direct, &tret, serr);
    if (retval == ERR) return ERR;
#if 0
    if (azalt_cart(tret_dark, dgeo, datm, ObjectName, helflag, darr, serr) == ERR)
      return ERR;
    if (azalt_cart(tret, dgeo, datm, ObjectName, helflag, darr+6, serr) == ERR)
      return ERR;
    dtret = acos(swi_dot_prod_unit(darr+3, darr+9)) / DEGTORAD;
#else
    dtret = fabs(tret - tret_dark);
#endif
  }
  if (azalt_cart(tret, dgeo, datm, "sun", helflag, darr, serr) == ERR)
    return ERR;
  *thel = tret;
  if (darr[1] < -12) {
    sprintf(serr, "acronychal rising/setting not available, %f", darr[1]);
    return OK;
  } else {
    sprintf(serr, "solar altitude, %f", darr[1]);
  }
  return OK;
}

static int32 get_heliacal_details(double tday, double *dgeo, double *datm, double *dobs, char *ObjectName, int32 TypeEvent, int32 helflag, double *dret, char *serr)
{
  int32 i, retval, direct;
  AS_BOOL optimum_undefined, limit_1_undefined, limit_2_undefined;
  /* find next optimum visibility */
  optimum_undefined = FALSE;
  retval = time_optimum_visibility(tday, dgeo, datm, dobs, ObjectName, helflag, &(dret[1]), serr);
  if (retval == ERR) return ERR;
  if (retval == -2) {
    retval = OK;
    optimum_undefined = TRUE; /* change photopic <-> scotopic vision */
  }
  /* find moment of becoming visible */
  direct = 1;
  if (TypeEvent == 1 || TypeEvent == 4)
    direct = -1;
  limit_1_undefined = FALSE;
  retval = time_limit_invisible(tday, dgeo, datm, dobs, ObjectName, helflag, direct, &(dret[0]), serr);
  if (retval == ERR) return ERR;
  if (retval == -2) {
    retval = OK;
    limit_1_undefined = TRUE; /* change photopic <-> scotopic vision */
  }
  /* find moment of end of visibility */
  direct *= -1;
  limit_2_undefined = FALSE;
  retval = time_limit_invisible(dret[1], dgeo, datm, dobs, ObjectName, helflag, direct, &(dret[2]), serr);
  if (retval == ERR) return ERR;
  if (retval == -2) {
    retval = OK;
    limit_2_undefined = TRUE; /* change photopic <-> scotopic vision */
  }
  /* correct sequence of times: 
   * with event types 2 and 3 swap dret[0] and dret[2] */
  if (TypeEvent == 2 || TypeEvent == 3) {
    tday = dret[2];
    dret[2] = dret[0];
    dret[0] = tday;
    i = (int) limit_1_undefined;
    limit_1_undefined = limit_2_undefined;
    limit_2_undefined = (AS_BOOL) i;
  }
  /*if (retval == OK && dret[0] == dret[1]) */
  if (optimum_undefined || limit_1_undefined || limit_2_undefined) {
    sprintf(serr, "return values [");
    if (limit_1_undefined)
      strcat(serr, "0,");
    if (optimum_undefined)
      strcat(serr, "1,");
    if (limit_2_undefined)
      strcat(serr, "2,");
    strcat(serr, "] are uncertain due to change between photopic and scotopic vision");
  }
  return OK;
}

static int32 heliacal_ut_vis_lim(double tjd_start, double *dgeo, double *datm, double *dobs, char *ObjectName, int32 TypeEventIn, int32 helflag, double *dret, char *serr_ret)
{
  int i;
  double d, darr[10], direct = 1, tjd, tday;
  int32 epheflag, retval = OK, helflag2;
  int32 iflag, ipl;
  int32 TypeEvent = TypeEventIn;
  char serr[AS_MAXCH];
  for (i = 0; i < 10; i++)
    dret[i] = 0;
  *dret = tjd_start;
  *serr = '\0';
  ipl = DeterObject(ObjectName);
  epheflag = helflag & (SEFLG_JPLEPH|SEFLG_SWIEPH|SEFLG_MOSEPH);
  iflag = SEFLG_TOPOCTR | SEFLG_EQUATORIAL | epheflag;
  if (!(helflag & SE_HELFLAG_HIGH_PRECISION)) 
    iflag |= SEFLG_NONUT | SEFLG_TRUEPOS;
  if (ipl == SE_MERCURY)
    tjd = tjd_start - 30;
  else
    tjd = tjd_start - 50; /* -50 makes sure, that no event is missed, 
                         * but may return an event before start date */
  helflag2 = helflag;
  /*helflag2 &= ~SE_HELFLAG_HIGH_PRECISION;*/
  /* 
   * heliacal event
   */
  if (ipl == SE_MERCURY || ipl == SE_VENUS || TypeEvent <= 2) {
    if (ipl == -1) {
      /* find date when star rises with sun (cosmic rising) */
    retval = get_asc_obl_with_sun(tjd, ipl, ObjectName, helflag, TypeEvent, 0, dgeo, &tjd, serr);
      if (retval != OK)
	goto swe_heliacal_err; /* retval may be -2 or ERR */
    } else {
      /* find date of conjunction of object with sun */
      if ((retval = find_conjunct_sun(tjd, ipl, helflag, TypeEvent, &tjd, serr)) == ERR) {
	goto swe_heliacal_err;
      }
    }
    /* find the day and minute on which the object becomes visible */
    retval = get_heliacal_day(tjd, dgeo, datm, dobs, ObjectName, helflag2, TypeEvent, &tday, serr); 
    if (retval != OK)
      goto swe_heliacal_err;
  /* 
   * acronychal event
   */
  } else {
    if (1 || ipl == -1) {
      /*retval = get_asc_obl_acronychal(tjd, ipl, ObjectName, helflag2, TypeEvent, dgeo, &tjd, serr);*/
      retval = get_asc_obl_with_sun(tjd, ipl, ObjectName, helflag, TypeEvent, 0, dgeo, &tjd, serr);
      if (retval != OK)
	goto swe_heliacal_err;
    } else {
      /* find date of conjunction of object with sun */
      if ((retval = find_conjunct_sun(tjd, ipl, helflag, TypeEvent, &tjd, serr)) == ERR)
	goto swe_heliacal_err;
    }
    tday = tjd;
    retval = get_acronychal_day(tjd, dgeo, datm, dobs, ObjectName, helflag2, TypeEvent, &tday, serr);
    if (retval != OK)
      goto swe_heliacal_err;
  }
  dret[0] = tday;
  if (!(helflag & SE_HELFLAG_NO_DETAILS)) {
    /* more precise event times for 
     * - morning first, evening last
     * - venus and mercury's evening first and morning last
     */
    if (ipl == SE_MERCURY || ipl == SE_VENUS || TypeEvent <= 2) {
      retval = get_heliacal_details(tday, dgeo, datm, dobs, ObjectName, TypeEvent, helflag2, dret, serr);
      if (retval == ERR) goto swe_heliacal_err;
    } else if (0) {
      if (TypeEvent == 4 || TypeEvent == 6) direct = -1;
      for (i = 0, d = 100.0 / 86400.0; i < 3; i++, d /= 10.0) {
	while((retval = swe_vis_limit_mag(*dret + d * direct, dgeo, datm, dobs, ObjectName, helflag, darr, serr)) == -2 || (retval >= 0 && darr[0] < darr[7])) { 
	  *dret += d * direct; 
	}
      }
      /* the last time step must be added */
      if (retval == OK)
	*dret += 1.0 / 86400.0 * direct;
    }
  } /* if (1) */
swe_heliacal_err:
  if (serr_ret != NULL && *serr != '\0')
    strcpy(serr_ret, serr);
  return retval;
}

/*###################################################################*/
static int32 moon_event_vis_lim(double tjdstart, double *dgeo, double *datm, double *dobs, int32 TypeEvent, int32 helflag, double *dret, char *serr_ret)
{
  double tjd, trise;
  char serr[AS_MAXCH];
  char ObjectName[30];
  int32 iflag, ipl, retval, helflag2, direct;
  int32 epheflag = helflag & (SEFLG_JPLEPH|SEFLG_SWIEPH|SEFLG_MOSEPH);
  dret[0] = tjdstart; /* will be returned in error case */
  if (TypeEvent == 1 || TypeEvent == 2) {
    if (serr != NULL)
      strcpy(serr, "error: the moon has no morning first or evening last");
    return ERR;
  }
  strcpy(ObjectName, "moon");
  ipl = SE_MOON;
  iflag = SEFLG_TOPOCTR | SEFLG_EQUATORIAL | epheflag;
  if (!(helflag & SE_HELFLAG_HIGH_PRECISION))
    iflag |= SEFLG_NONUT|SEFLG_TRUEPOS;
  helflag2 = helflag;
  helflag2 &= ~SE_HELFLAG_HIGH_PRECISION;
  /* check Synodic/phase Period */
  tjd = tjdstart - 30; /* -50 makes sure, that no event is missed, 
                         * but may return an event before start date */
  if ((retval = find_conjunct_sun(tjd, ipl, helflag, TypeEvent, &tjd, serr)) == ERR)
    return ERR;
  /* find the day and minute on which the object becomes visible */
  retval = get_heliacal_day(tjd, dgeo, datm, dobs, ObjectName, helflag2, TypeEvent, &tjd, serr); 
  if (retval != OK)
    goto moon_event_err;
  dret[0] = tjd;
  /* find next optimum visibility */
  retval = time_optimum_visibility(tjd, dgeo, datm, dobs, ObjectName, helflag, &tjd, serr);
  if (retval == ERR) goto moon_event_err;
  dret[1] = tjd;
  /* find moment of becoming visible */
  /* Note: The on the day of fist light the moon may become visible 
   * already during day. It also may appear during day, disappear again
   * and then reappear after sunset */
  direct = 1;
  if (TypeEvent == 4)
    direct = -1;
  retval = time_limit_invisible(tjd, dgeo, datm, dobs, ObjectName, helflag, direct, &tjd, serr);
  if (retval == ERR) goto moon_event_err;
  dret[2] = tjd;
  /* find moment of end of visibility */
  direct *= -1;
  retval = time_limit_invisible(dret[1], dgeo, datm, dobs, ObjectName, helflag, direct, &tjd, serr);
  dret[0] = tjd;
  if (retval == ERR) goto moon_event_err;
  /* if the moon is visible before sunset, we return sunset as start time */
  if (TypeEvent == 3) {
    if ((retval = my_rise_trans(tjd, SE_SUN, "", SE_CALC_SET, helflag, dgeo, datm, &trise, serr)) == ERR)
      return ERR;
    if (trise < dret[1]) {
      dret[0] = trise;
      /* do not warn, it happens too often */
      /*strcpy(serr, "start time given is sunset, but moon is observable before that");*/
    }
  /* if the moon is visible after sunrise, we return sunrise as end time */
  } else {
    if ((retval = my_rise_trans(dret[1], SE_SUN, "", SE_CALC_RISE, helflag, dgeo, datm, &trise, serr)) == ERR)
      return ERR;
    if (dret[0] > trise) {
      dret[0] = trise;
      /* do not warn, it happens too often */
      /*strcpy(serr, "end time given is sunrise, but moon is observable after that");*/
    }
  }
  /* correct order of the three times: */
  if (TypeEvent == 4) {
    tjd = dret[0];
    dret[0] = dret[2];
    dret[2] = tjd;
  }
moon_event_err:
  if (serr_ret != NULL && *serr != '\0')
    strcpy(serr_ret, serr);
  return retval;
}

static int32 MoonEventJDut(double JDNDaysUTStart, double *dgeo, double *datm, double *dobs, int32 TypeEvent, int32 helflag, double *dret, char *serr)
{
  int32 avkind = helflag & SE_HELFLAG_AVKIND;
  if (avkind) 
    return moon_event_arc_vis(JDNDaysUTStart, dgeo, datm, dobs, TypeEvent, helflag, dret, serr);
  else
    return moon_event_vis_lim(JDNDaysUTStart, dgeo, datm, dobs, TypeEvent, helflag, dret, serr);
}

static int32 heliacal_ut(double JDNDaysUTStart, double *dgeo, double *datm, double *dobs, char *ObjectName, int32 TypeEventIn, int32 helflag, double *dret, char *serr_ret)
{
  int32 avkind = helflag & SE_HELFLAG_AVKIND;
  if (avkind) 
    return heliacal_ut_arc_vis(JDNDaysUTStart, dgeo, datm, dobs, ObjectName, TypeEventIn, helflag, dret, serr_ret);
  else
    return heliacal_ut_vis_lim(JDNDaysUTStart, dgeo, datm, dobs, ObjectName, TypeEventIn, helflag, dret, serr_ret);
}

/*' Magn [-]
' tjd_ut            start date (JD) for event search
' dgeo[3]           geogr. longitude, latitude, eye height (m above sea level)
' datm[4]           atm. pressure, temperature, RH, and VR
' - pressure        atmospheric pressure (mbar, =hPa) default 1013.25hPa
' - temperature      deg C, default 15 deg C (if at
'                   If both attemp and atpress are 0, a temperature and
'                   atmospheric pressure are estimated from the above-mentioned
'                   default values and the height above sea level.
' - RH              relative humidity in %
' - VR              VR>=1: the Meteorological range: default 40 km
'                   1>VR>0: the ktot (so the total atmospheric coefficient): 
'                   a good default would be 0.25
'                   VR=-1: the ktot is calculated from the other atmospheric 
'                   constants.
' age [Year]        default 36, experienced sky observer in ancient times
'                   optimum age is 23
' SN                Snellen factor of the visual aquity of the observer
'                   default 1
'                   see: http://www.i-see.org/eyecharts.html#make-your-own
' TypeEvent         1 morning first
'                   2 evening last
'                   3 evening first
'                   4 morning last
' dret		    output: time (tjd_ut) of heliacal event
'                   dret[0]: beginning of visibility (Julian day number)
'                   dret[1]: optimum visibility (Julian day number; 0 if SE_HELFLAG_AV)
'                   dret[2]: end of visibility (Julian day number; 0 if SE_HELFLAG_AV)
' see http://www.iol.ie/~geniet/eng/atmoastroextinction.htm
*/
int32 FAR PASCAL_CONV swe_heliacal_ut(double JDNDaysUTStart, double *dgeo, double *datm, double *dobs, char *ObjectNameIn, int32 TypeEvent, int32 helflag, double *dret, char *serr_ret)
{
  int32 retval, Planet, itry;
  char ObjectName[AS_MAXCH], serr[AS_MAXCH], s[AS_MAXCH];
  double tjd0 = JDNDaysUTStart, tjd, dsynperiod, tjdmax, tadd;
  int32 MaxCountSynodicPeriod = MAX_COUNT_SYNPER;
  char *sevent[7] = {"", "morning first", "evening last", "evening first", "morning last", "acronychal rising", "acronychal setting"};
  if (dgeo[2] < SEI_ECL_GEOALT_MIN || dgeo[2] > SEI_ECL_GEOALT_MAX) {
    if (serr_ret != NULL)
      sprintf(serr_ret, "location for heliacal events must be between %.0f and %.0f m above sea", SEI_ECL_GEOALT_MIN, SEI_ECL_GEOALT_MAX);
    return ERR;
  }
  swi_set_tid_acc(JDNDaysUTStart, helflag, 0);
  if (helflag & SE_HELFLAG_LONG_SEARCH)
    MaxCountSynodicPeriod = MAX_COUNT_SYNPER_MAX;
/*  if (helflag & SE_HELFLAG_SEARCH_1_PERIOD)
      MaxCountSynodicPeriod = 1; */
  *serr = '\0';
  if (serr_ret != NULL)
    *serr_ret = '\0';
  /* note, the fixed stars functions rewrite the star name. The input string 
     may be too short, so we have to make sure we have enough space */
  strcpy_VBsafe(ObjectName, ObjectNameIn);
  default_heliacal_parameters(datm, dgeo, dobs, helflag);
  swe_set_topo(dgeo[0], dgeo[1], dgeo[2]);
  Planet = DeterObject(ObjectName);
  /* 
   * Moon events
   */
  if (Planet == SE_MOON) {
    if (TypeEvent == 1 || TypeEvent == 2) {
      if (serr_ret != NULL) {
        sprintf(serr_ret, "%s (event type %d) does not exist for the moon\n", sevent[TypeEvent], TypeEvent);
      }
      return ERR;
    }
    tjd = tjd0;
    retval = MoonEventJDut(tjd, dgeo, datm, dobs, TypeEvent, helflag, dret, serr);
    while (retval != -2 && *dret < tjd0) {
      tjd += 15;
      *serr = '\0';
      retval = MoonEventJDut(tjd, dgeo, datm, dobs, TypeEvent, helflag, dret, serr);
    }
    if (serr_ret != NULL && *serr != '\0')
      strcpy(serr_ret, serr);
    return retval;
  }
  /* 
   * planets and fixed stars 
   */
  if (!(helflag & SE_HELFLAG_AVKIND)) {
    if (Planet == -1 || Planet >= SE_MARS) {
      if (TypeEvent == 3 || TypeEvent == 4) {
	if (serr_ret != NULL) {
	  if (Planet == -1)
	    strcpy(s, ObjectName);
	  else
	    swe_get_planet_name(Planet, s);
	  sprintf(serr_ret, "%s (event type %d) does not exist for %s\n", sevent[TypeEvent], TypeEvent, s);
	}
	return ERR;
      }
    }
  }
  /* arcus visionis method: set the TypeEvent for acronychal events */
  if (helflag & SE_HELFLAG_AVKIND) {
    if (Planet == -1 || Planet >= SE_MARS) {
      if (TypeEvent == SE_ACRONYCHAL_RISING)
	TypeEvent = 3;
      if (TypeEvent == SE_ACRONYCHAL_SETTING)
	TypeEvent = 4;
    }
  /* acronychal rising and setting (cosmic setting) are ill-defined.
   * We do not calculate them with the "visibility limit method" */
  } else if (1) {
    if (TypeEvent == SE_ACRONYCHAL_RISING || TypeEvent == SE_ACRONYCHAL_SETTING) {
      if (serr_ret != NULL) {
	if (Planet == -1)
	  strcpy(s, ObjectName);
	else
	  swe_get_planet_name(Planet, s);
	sprintf(serr_ret, "%s (event type %d) is not provided for %s\n", sevent[TypeEvent], TypeEvent, s);
      }
      return ERR;
    }
  }
  dsynperiod = get_synodic_period(Planet);
  tjdmax = tjd0 + dsynperiod * MaxCountSynodicPeriod;
  tadd = dsynperiod * 0.6;
  if (Planet == SE_MERCURY)
    tadd = 30;
  /* 
   * this is the outer loop over n synodic periods 
   */
  tjd = tjd0;
  retval = -2;  /* indicates that another synodic period has to be done */
  for (itry = 0; 
       tjd < tjdmax && retval == -2; 
       itry++, tjd += tadd) {
    *serr = '\0';
    retval = heliacal_ut(tjd, dgeo, datm, dobs, ObjectName, TypeEvent, helflag, dret, serr);
    /* if resulting event date < start date for search (tjd0): retry starting
     * from half a period later. The event must be found now, unless there
     * is none, as is often the case with Mercury */
    while (retval != -2 && *dret < tjd0) {
      tjd += tadd;
      *serr = '\0';
      retval = heliacal_ut(tjd, dgeo, datm, dobs, ObjectName, TypeEvent, helflag, dret, serr);
    }
  }
  /* 
   * no event was found within MaxCountSynodicPeriod, return error
   */ 
  if ((helflag & SE_HELFLAG_SEARCH_1_PERIOD) && (retval == -2 || dret[0] > tjd0 + dsynperiod * 1.5)) {
    strcpy(serr, "no heliacal date found within this synodic period");
    retval = -2;
  } else if (retval == -2) {
    sprintf(serr, "no heliacal date found within %d synodic periods", MaxCountSynodicPeriod);
    retval = ERR;
  }
  if (serr_ret != NULL && *serr != '\0')
    strcpy(serr_ret, serr);
  return retval;
}
