/*
** Astrolog (Version 5.41) File: swe_call.c
**
** IMPORTANT NOTICE: The graphics database and chart display routines
** used in this program are Copyright (C) 1991-1996 by Walter D. Pullen
** (Astara@msn.com, http://www.magitech.com/~cruiser1/astrolog.htm).
** Permission is granted to freely use and distribute these routines
** provided one doesn't sell, restrict, or profit from them in any way.
** Modification is allowed provided these notices remain with any
** altered or edited versions of the program.
**
** The main planetary calculation routines used in this program have
** been Copyrighted and the core of this program is basically a
** conversion to C of the routines created by James Neely as listed in
** Michael Erlewine's 'Manual of Computer Programming for Astrologers',
** available from Matrix Software. The copyright gives us permission to
** use the routines for personal use but not to sell them or profit from
** them in any way.
**
** The PostScript code within the core graphics routines are programmed
** and Copyright (C) 1992-1993 by Brian D. Willoughby
** (brianw@sounds.wa.com). Conditions are identical to those above.
**
** The extended accurate ephemeris databases and formulas are from the
** calculation routines in the program "Swiss Ephemeris" and are 
** Copyright (C) 1998 by Astrodienst AG.
** The use of that source code is subject to
** the Swiss Ephemeris Public License, available at 
** http://www.astro.ch/swisseph.
** This copyright notice must not be changed or removed
** by any user of this program.
** by any user of this program.
**
** Initial programming 8/28,30, 9/10,13,16,20,23, 10/3,6,7, 11/7,10,21/1991.
** X Window graphics initially programmed 10/23-29/1991.
** PostScript graphics initially programmed 11/29-30/1992.
** Last code change made 9/22/1996.
*/

#include "astrolog.h"
#include "swephexp.h"

#  define d2l(x)	swe_d2l(x)

/* Given an object index and a Julian Day time, get        */
/* ecliptic longitude and latitude of the object  */
/* and its velocity and distance from the Earth or Sun. This basically    */
/* just calls the Placalc calculation function to actually do it, but as  */
/* this is the one routine called from Astrolog, this is the one routine  */
/* which has knowledge of and uses both Astrolog and Placalc definitions, */
/* and does things such as translation to Placalc indices and formats.    */

bool FPlacalcPlanet(ind, jd, helio, obj, objalt, dir, space)
int ind, helio;
double jd;
real *obj, *objalt, *dir, *space;
{
  static AS_BOOL path_built = FALSE;
  int iobj;
  long flag;
  double jde, xx[6];
  char serr[AS_MAXCH];
  /*
   * if the function is called the first time, we build an ephemeris path
   * in the same way as FileOpen() in io.c searchs for a file.
   * we use serr to build the list of directories in it.
   * If environment variable SE_EPHE_PATH exists, it will override all this.
   */
  if (! path_built) {
#ifdef ENVIRON
  char *env;
#endif
    char name[80];
    /* First look in the current directory. */
    strcpy(serr, ".");
#ifdef ENVIRON
    /* Next add the directory indicated by the version */
    /* specific system environment variable.                            */
    sprintf(name, "%s%s", ENVIRONVER, szVersionCore);
    env = getenv(name);
    if (env && *env) 
      sprintf(serr + strlen(serr), "%s%s", PATH_SEPARATOR, env);
    /* Next look in the directory in the general environment variable. */
    env = getenv(ENVIRONALL);
    if (env && *env) 
      sprintf(serr + strlen(serr), "%s%s", PATH_SEPARATOR, env);
    /* Next add the directory in the version prefix environment variable. */
    env = getenv(ENVIRONVER);
    if (env && *env) 
      sprintf(serr + strlen(serr), "%s%s", PATH_SEPARATOR, env);
#endif
    /* Finally add several directories specified at compile time. */
    sprintf(serr + strlen(serr), "%s%s", PATH_SEPARATOR, DEFAULT_DIR);
    sprintf(serr + strlen(serr), "%s%s", PATH_SEPARATOR, EPHE_DIR);
    sprintf(serr + strlen(serr), "%s%s", PATH_SEPARATOR, CHART_DIR);
    swe_set_ephe_path(serr);
    path_built = TRUE;
  }
  if (ind <= oPlu)      /* Convert Astrolog object index to SwissEph index. */
    iobj = ind-1;
  else if (ind == oChi)
    iobj = SE_CHIRON;
  else if (FBetween(ind, oCer, oVes))
    iobj = ind - oCer + SE_CERES;
  else if (ind == oNod)
    iobj = us.fTrueNode ? SE_TRUE_NODE : SE_MEAN_NODE;
  else if (ind == oLil)
    iobj = SE_MEAN_APOG;
  else
    return fFalse;

  if (ind == oSun && helio)
    iobj = SE_EARTH;
  flag = helio ? SEFLG_SPEED | SEFLG_HELCTR : SEFLG_SPEED;
  jde = jd + swe_deltat(jd);
  if (swe_calc(jde, iobj, flag, xx, serr) >= 0) {
    *obj    = xx[0];
    *objalt = xx[1];
    *space  = xx[2];
    *dir    = xx[3];
    return fTrue;
  }
  return fFalse;
}

double julday(int month, int day, int year, double hour, int gregflag) 
{
  double jd;
  jd = swe_julday(year, month, day, hour, gregflag);
  return jd;
}

void revjul (double jd, int gregflag,
	     int *jmon, int *jday, int *jyear, double *jut)
{
  swe_revjul(jd, gregflag, jyear, jmon, jday, jut);
}
