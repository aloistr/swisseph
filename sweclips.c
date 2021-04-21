/* SWISSEPH
   
  Authors: Dieter Koch and Alois Treindl, Astrodienst Zürich

**************************************************************/
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

# define SMOD_LUNAR	1
# define SMOD_HOW	2	/* an option for Lunar */
# define SMOD_SOLAR	4
# define SMOD_LOCAL	8	/* an option for Solar */
# define SMOD_TOTAL	16
# define SMOD_ANNULAR	32	/* count as penumbral for Lunar */
# define SMOD_ANNTOT	64
# define SMOD_PARTIAL	128	
# define SMOD_ALL	(SMOD_TOTAL| SMOD_ANNULAR|SMOD_PARTIAL|SMOD_ANNTOT)
# define SMOD_RISE	256
# define SMOD_METR	512
# define SMOD_HOCAL	1024


/* attention: Microsoft Compiler does not accept strings > 2048 char */
static char *infocmd0 = "\n\
  Sweclips computes solar and lunar eclipses,\n\
  rises, sets, and meridian transits for planets and fixed stars\n\
  Input can either be a date or an absolute julian day number.\n\
\n";

static char *infocmd1 = "\n\
  Command line options:\n\
        -lunar  lunar eclipse\n\
        -solar  solar eclipse\n\
        -hocal  hocallist format\n\
        -local  local solar eclipse\n\
        -annular\n\
        -total\n\
        -partial\n\
        -rise   next rise and set\n\
        -metr   next meridian transits (culminations)\n\
        -lon...   geogr. longitude D.MMSS\n\
        -lat...   geogr. longitude D.MMSS\n\
	-p.       planet (like swetest.c)\n\
        -edirPATH change the directory of the ephemeris files \n\
        -head   don\'t print the header before the planet data. This option\n\
                is useful when you want to paste the output into a\n\
                spreadsheet for displaying graphical ephemeris.\n\
        +head   header before every step (with -s..) \n\
        -bDATE  use this begin date instead of asking; use -b1.1.1992 if\n\
                the begin date string contains blanks; use format -bj2400000.5\n\
                to express the date as absolute Julian day number.\n\
                Note: the date format is day month year (European style).\n\
                \n";

static char *infocmd2 = "\
        -eswe   swiss ephemeris\n\
        -ejpl   jpl ephemeris (DE406), or with ephemeris file name\n\
                -ejplDE200.cdrom \n\
        -emos   moshier ephemeris\n\
        -?, -h  display whole info\n\
        -hcmd   display commands\n\
        -hdate  display input date format\n";

static char *infodate = "\n\
  Date entry:\n\
  In the interactive mode, when you are asked for a start date,\n\
  you can enter data in one of the following formats:\n\
\n\
        1.2.1991        three integers separated by a nondigit character for\n\
                        day month year. Dates are interpreted as Gregorian\n\
                        after 4.10.1582 and as Julian Calender before.\n\
                        Time is always set to midnight.\n\
                        If the three letters jul are appended to the date,\n\
                        the Julian calendar is used even after 1582.\n\
                        If the four letters greg are appended to the date,\n\
                        the Gregorian calendar is used even before 1582.\n\
\n\
        j2400123.67     the letter j followed by a real number, for\n\
                        the absolute Julian daynumber of the start date.\n\
                        Fraction .5 indicates midnight, fraction .0\n\
                        indicates noon, other times of the day can be\n\
                        chosen accordingly.\n\
        \n";
/**************************************************************/

#include "sweodef.h"
#include "swephexp.h"

#if MSDOS
#  include <direct.h>
#  include <dos.h>
#  ifdef _MSC_VER
#    include <sys\types.h>
#  endif
#endif

#ifdef MACOS
#  include <console.h>
#endif


#define J2000           2451545.0  /* 2000 January 1.5 */
#define square_sum(x)   (x[0]*x[0]+x[1]*x[1]+x[2]*x[2])
#define SEFLG_EPHMASK   (SEFLG_JPLEPH|SEFLG_SWIEPH|SEFLG_MOSEPH)

#define BIT_ROUND_SEC   1
#define BIT_ROUND_MIN   2
#define BIT_ZODIAC      4
#define BIT_LZEROES     8

# define ECL_LUN_PENUMBRAL       1       /* eclipse types for hocal list */
# define ECL_LUN_PARTIAL        2
# define ECL_LUN_TOTAL          3
# define ECL_SOL_PARTIAL        4
# define ECL_SOL_ANNULAR        5
# define ECL_SOL_TOTAL          6        

static char *zod_nam[] = {"ar", "ta", "ge", "cn", "le", "vi", 
                          "li", "sc", "sa", "cp", "aq", "pi"};

static char *dms(double xv, int32 iflg);
static char *hms(double x, int32 iflag);
static int do_calc(double tjd, int ipl, long iflag, double *x, char *serr);
static void do_printf(char *info);
static int make_ephemeris_path(long iflag, char *argv0);
static char *hms_from_tjd(double x);
static int cut_str_any(char *s, char *cutlist, char *cpos[], int nmax);
static int letter_to_ipl(int letter);

int main(int argc, char *argv[])
{
  char saves[AS_MAXCH]; 
  char s1[AS_MAXCH], s2[AS_MAXCH];
  char serr[AS_MAXCH], serr_save[AS_MAXCH], serr_warn[AS_MAXCH],
sout[AS_MAXCH];
  char *sp, *sp2;
  char *fmt = "PLBRS";
  int i, ii;
  int smod = 0;
  int ecl_type = 0;
  int jmon, jday, jyear;
  double jut = 0.0;
  long nstep = 1;
  double x[6];
  char ephepath[AS_MAXCH];
  char fname[AS_MAXCH];
  char sdate[AS_MAXCH];
  char *begindate = NULL;
  long iflag = 0;
  long eclflag;
  double geopos[20], attr[20], tret[20];
  double a, b, c;
  int whicheph = SEFLG_SWIEPH;   
  short with_header = TRUE;
  int gregflag = SE_GREG_CAL;
  double tjd = 2415020.5, t_ut;
  double dt;
  int direction = 1;
  AS_BOOL direction_flag = FALSE;
  int32 ipl = 0;
  char starname[AS_MAXCH];
  int32 search_flag = 0;
  char slon[30], slat[30];
  serr[0] = serr_save[0] = serr_warn[0] = saves[0] = '\0';
  starname[0] = '\0';
  strcpy(ephepath, SE_EPHE_PATH);
  strcpy(fname, SE_FNAME_DE406);
  strcpy(slon, "8.33");         /* geographical position of Zurich */
  strcpy(slat, "47.23"); 
  /*
   * command line
   */
  search_flag = SE_ECL_CENTRAL | SE_ECL_NONCENTRAL;
  smod = SMOD_SOLAR;
  for (i = 1; i < argc; i++) {
    if (strncmp(argv[i], "-head", 5) == 0) {
      with_header = FALSE;
    } else if (strcmp(argv[i], "-lunar") == 0) {
      smod |= SMOD_LUNAR;
      smod &= ~SMOD_SOLAR;
    } else if (strcmp(argv[i], "-hocal") == 0) {
      smod |= SMOD_HOCAL;
    } else if (strcmp(argv[i], "-solar") == 0) {
      smod |= SMOD_SOLAR;
    } else if (strcmp(argv[i], "-how") == 0) {
      smod |= SMOD_HOW;
    } else if (strcmp(argv[i], "-total") == 0) {
      smod |= SMOD_TOTAL;
    } else if (strcmp(argv[i], "-annular") == 0) {
      smod |= SMOD_ANNULAR;
    } else if (strcmp(argv[i], "-anntot") == 0) {
      smod |= SMOD_ANNTOT;
    } else if (strcmp(argv[i], "-partial") == 0) {
      smod |= SMOD_PARTIAL;
    } else if (strcmp(argv[i], "-noncentral") == 0) {
      search_flag &= ~SE_ECL_CENTRAL;
      search_flag |= SE_ECL_NONCENTRAL;
    } else if (strcmp(argv[i], "-central") == 0) {
      search_flag &= ~SE_ECL_NONCENTRAL;
      search_flag |= SE_ECL_CENTRAL;
    } else if (strcmp(argv[i], "-local") == 0) {
      smod |= SMOD_LOCAL;
    } else if (strcmp(argv[i], "-rise") == 0) {
      smod |= SMOD_RISE;
      smod &= ~SMOD_SOLAR;
    } else if (strcmp(argv[i], "-metr") == 0) {
      smod |= SMOD_METR;
      smod &= ~SMOD_SOLAR;
    } else if (strncmp(argv[i], "-j", 2) == 0) {
      begindate = argv[i] + 1;
    } else if (strncmp(argv[i], "-lon", 4) == 0) {
      strcpy(slon, argv[i] + 4);
    } else if (strncmp(argv[i], "-lat", 4) == 0) {
      strcpy(slat, argv[i] + 4);
    } else if (strncmp(argv[i], "-ejpl", 5) == 0) {
      whicheph = SEFLG_JPLEPH;
      strcpy(ephepath, SE_EPHE_PATH);
      if (*(argv[i]+5) != '\0')
        strcpy(fname, argv[i]+5);
    } else if (strcmp(argv[i], "-eswe") == 0) {
      whicheph = SEFLG_SWIEPH;
      strcpy(ephepath, SE_EPHE_PATH);
    } else if (strcmp(argv[i], "-emos") == 0) {
      whicheph = SEFLG_MOSEPH;
    } else if (strncmp(argv[i], "-edir", 5) == 0) {
      if (*(argv[i]+5) != '\0')
        strcpy(ephepath, argv[i]+5);
    } else if (strncmp(argv[i], "-n", 2) == 0) {
      nstep = atoi(argv[i]+2);
    } else if (strcmp(argv[i], "-bwd") == 0) {
      direction = -1;
          direction_flag = TRUE;
    } else if (strncmp(argv[i], "-b", 2) == 0) {
      begindate = argv[i] + 2;
    } else if (strncmp(argv[i], "-p", 2) == 0) {
      sp = argv[i] + 2;
      if (strlen(sp) == 1)
	ipl = letter_to_ipl(*sp);
      else
	strcpy(starname, sp);
    } else if (strncmp(argv[i], "-h", 2) == 0
      || strncmp(argv[i], "-?", 2) == 0) {
      sp = argv[i]+2;
      if (*sp == 'c' || *sp == '\0') {
        do_printf(infocmd0);
        do_printf(infocmd1);
        do_printf(infocmd2);
      }
      if (*sp == 'd' || *sp == '\0')
        do_printf(infodate);
      goto end_main;
    } else {
      sprintf(sout, "illegal option %s\n", argv[i]);
      do_printf(sout);
      exit(1);
    }
  }
  if (smod & (SMOD_LUNAR | SMOD_SOLAR))  {
    if ((smod & SMOD_ALL) == 0) /* no selective eclipse type set, set all */
      smod |= SMOD_ALL;
    if (smod & SMOD_TOTAL) search_flag |= SE_ECL_TOTAL;
    if (smod & SMOD_ANNULAR) search_flag |= SE_ECL_ANNULAR | SE_ECL_PENUMBRAL;
    if (smod & SMOD_PARTIAL) search_flag |= SE_ECL_PARTIAL;
    if (smod & SMOD_ANNTOT) search_flag |= SE_ECL_ANNULAR_TOTAL;
  }
  if (with_header) {
    for (i = 0; i < argc; i++) {
      do_printf(argv[i]);
      do_printf(" ");
    }
    do_printf("\n");
  }
  iflag = (iflag & ~SEFLG_EPHMASK) | whicheph;
  if (strpbrk(fmt, "SsQ") != NULL) 
    iflag |= SEFLG_SPEED;
  if (*ephepath != '\0')
    swe_set_ephe_path(ephepath);
  else if (make_ephemeris_path(iflag, argv[0]) == ERR) {
    iflag = (iflag & ~SEFLG_EPHMASK) | SEFLG_MOSEPH;
    whicheph = SEFLG_MOSEPH;
  }
  if (whicheph & SEFLG_JPLEPH)
    swe_set_jpl_file(fname);
  serr[0] = serr_save[0] = serr_warn[0] = '\0';
  if (begindate == NULL) {
    do_printf("\nDate ?");
    sdate[0] = '\0';
    gets(sdate);
  } else {
    strcpy(sdate, begindate);
    begindate = ".";  /* to exit afterwards */
  }
  sp = sdate;
  if (*sp == 'j') {   /* it's a day number */
    if ((sp2 = strchr(sp, ',')) != NULL) 
      *sp2 = '.';
    sscanf(sp+1,"%lf", &tjd);
    if (tjd < 2299160.5)
      gregflag = SE_JUL_CAL;
    else
      gregflag = SE_GREG_CAL;
    if (strstr(sp, "jul") != NULL)
      gregflag = SE_JUL_CAL;
    else if (strstr(sp, "greg") != NULL)
      gregflag = SE_GREG_CAL;
    swe_revjul(tjd, gregflag, &jyear, &jmon, &jday, &jut);
  } else {
    if (sscanf (sp, "%d%*c%d%*c%d", &jday,&jmon,&jyear) < 1) exit(1);
    if ((long) jyear * 10000L + (long) jmon * 100L + (long) jday <
15821015L) 
      gregflag = SE_JUL_CAL;
    else
      gregflag = SE_GREG_CAL;
    if (strstr(sp, "jul") != NULL)
      gregflag = SE_JUL_CAL;
    else if (strstr(sp, "greg") != NULL)
      gregflag = SE_GREG_CAL;
    jut = 0;
    tjd = swe_julday(jyear,jmon,jday,jut,gregflag);        
  }
  if (with_header) {
    sprintf(sout, "begin date (dmy) %d.%d.%d\n", jday, jmon, jyear);
    do_printf(sout);
    
  }
  /* 
   * for local eclipses: set geographic position of observer 
   */
  if (smod & (SMOD_LOCAL|SMOD_RISE|SMOD_METR)) {
    sscanf(slon,"%lf", &a);
    b = fmod(a * 10000, 100);
    c = a - b / 10000;
    c = fmod(c, 1) * 100;
    geopos[0] = a - fmod( a, 1) + c / 60 + b / 3600;
    sscanf(slat,"%lf", &a);
    b = fmod(a * 10000, 100);
    c = a - b / 10000;
    c = fmod(c, 1) * 100;
    geopos[1] = a - fmod( a, 1) + c / 60 + b / 3600;
    swe_set_topo(geopos[0], geopos[1], 0); 
  }
  t_ut = tjd;
  for (ii = 0; ii < nstep; ii++) {
    *sout = '\0';
    if ((smod & SMOD_LUNAR) && (smod & SMOD_HOW)) { 
      if ((eclflag = swe_lun_eclipse_how(t_ut, whicheph, geopos, attr, serr)) ==
ERR) {
        do_printf(serr);
        exit(0);
      } else {
	ecl_type = 0;
        if (eclflag & SE_ECL_TOTAL) {
          sprintf(sout, "total lunar eclipse: %f o/o \n", attr[0]);
	  ecl_type = ECL_LUN_TOTAL;
        } else if (eclflag & SE_ECL_PARTIAL)  {
          sprintf(sout, "partial lunar eclipse: %f o/o \n", attr[0]);
	  ecl_type = ECL_LUN_PARTIAL;
        } else if (eclflag & SE_ECL_PENUMBRAL)  {
          sprintf(sout, "penumbral lunar eclipse: %f o/o \n", attr[0]);
	  ecl_type = ECL_LUN_PENUMBRAL;
        } else {
          sprintf(sout, "no lunar eclipse \n");
	}
        do_printf(sout);
      }
    }
    if ((smod & SMOD_LUNAR) && ! (smod & SMOD_HOW)) { 
      if ((eclflag = swe_lun_eclipse_when(t_ut, whicheph, search_flag, 
                tret, direction_flag, serr)) == ERR) {
        do_printf(serr);
        exit(0);
      } 
      t_ut = tret[0];
      if ((eclflag & SE_ECL_TOTAL)) {
        strcpy(sout, "total   ");
	ecl_type = ECL_LUN_TOTAL;
      }
      if ((eclflag & SE_ECL_PENUMBRAL)) {
        strcpy(sout, "penumb. ");
	ecl_type = ECL_LUN_PENUMBRAL;
      }
      if ((eclflag & SE_ECL_PARTIAL)) {
        strcpy(sout, "partial ");
	ecl_type = ECL_LUN_PARTIAL;
      }
      strcat(sout, "lunar eclipse ");
      swe_revjul(t_ut, gregflag, &jyear, &jmon, &jday, &jut);
      if ((eclflag = swe_lun_eclipse_how(t_ut, whicheph, geopos, attr, serr)) ==
ERR) {
        do_printf(serr);
        exit(0);
      }
      sprintf(sout + strlen(sout), "%2d.%2d.%4d\t%s\t%f o/o\n", 
      /* eclipse times, penumbral, partial, total begin and end */
              jday, jmon, jyear, hms(jut,0), attr[0]);
	sprintf(sout + strlen(sout), "  %s ", hms_from_tjd(tret[6])); 
        if (tret[2] != 0)
	  sprintf(sout + strlen(sout), "%s ", hms_from_tjd(tret[2])); 
        else
          strcat(sout, "   -         ");
        if (tret[4] != 0)
	  sprintf(sout + strlen(sout), "%s ", hms_from_tjd(tret[4])); 
        else
          strcat(sout, "   -         ");
        if (tret[5] != 0)
	  sprintf(sout + strlen(sout), "%s ", hms_from_tjd(tret[5])); 
        else
          strcat(sout, "   -         ");
        if (tret[3] != 0)
	  sprintf(sout + strlen(sout), "%s ", hms_from_tjd(tret[3])); 
        else
          strcat(sout, "   -         ");
	sprintf(sout + strlen(sout), "%s\n", hms_from_tjd(tret[7])); 
      if (smod & SMOD_HOCAL) {
	int ihou, imin, isec, isgn;
	double dfrc;
	swe_split_deg(jut, SE_SPLIT_DEG_ROUND_MIN, &ihou, &imin, &isec, &dfrc, &isgn);
#if 0
	sprintf(sout, "\"%04d %02d %02d %02d.%02d %d\",\n", jyear, jmon, jday, ihou, imin, ecl_type);
#else
	sprintf(sout, "%f, %.0f, /* %04d %02d %02d %02d.%02d %d */,\n", t_ut, (double) ecl_type, jyear, jmon, jday, ihou, imin, ecl_type);
#endif
      } 
      do_printf(sout);
    }
    if ((smod & SMOD_SOLAR) && (smod & SMOD_LOCAL)) { 
      if ((eclflag = swe_sol_eclipse_when_loc(t_ut, whicheph, geopos, tret,
attr, direction_flag, serr)) == ERR) {
        do_printf(serr);
        exit(0);
      } else { 
	AS_BOOL has_found = FALSE;
        t_ut = tret[0];
        if ((smod & SMOD_TOTAL) && (eclflag & SE_ECL_TOTAL)) {
          strcpy(sout, "total   ");
	  has_found = TRUE;
	  ecl_type = ECL_SOL_TOTAL;
	}
        if ((smod & SMOD_ANNULAR) && (eclflag & SE_ECL_ANNULAR)) {
          strcpy(sout, "annular ");
	  has_found = TRUE;
	  ecl_type = ECL_SOL_ANNULAR;
	}
        if ((smod & SMOD_PARTIAL) && (eclflag & SE_ECL_PARTIAL)) {
          strcpy(sout, "partial ");
	  has_found = TRUE;
	  ecl_type = ECL_SOL_PARTIAL;
	}
	if (!has_found) {
	  ii--;
	} else {
	  i = do_calc(t_ut + swe_deltat(t_ut), SE_ECL_NUT, 0, x, serr);
	  swe_revjul(t_ut, gregflag, &jyear, &jmon, &jday, &jut);
	  sprintf(sout + strlen(sout), "%2d.%2d.%4d\t%s\t%fo/o\n", jday, jmon, jyear, hms(jut,0), attr[0]);
	  dt = (tret[3] - tret[2]) * 24 * 60;
	  sprintf(sout + strlen(sout), "\t%d min %4.2f sec\t",
                (int) dt, fmod(dt, 1) * 60); 
	  if (eclflag & SE_ECL_1ST_VISIBLE)
	    sprintf(sout + strlen(sout), "%s ", hms_from_tjd(tret[1])); 
	  else
	    strcat(sout, "   -         ");
	  if (eclflag & SE_ECL_2ND_VISIBLE)
	    sprintf(sout + strlen(sout), "%s ", hms_from_tjd(tret[2])); 
	  else
	    strcat(sout, "   -         ");
	  if (eclflag & SE_ECL_3RD_VISIBLE)
	    sprintf(sout + strlen(sout), "%s ", hms_from_tjd(tret[3])); 
	  else
	    strcat(sout, "   -         ");
	  if (eclflag & SE_ECL_4TH_VISIBLE)
	    sprintf(sout + strlen(sout), "%s ", hms_from_tjd(tret[4])); 
	  else
	    strcat(sout, "   -         ");
#if 0
	  sprintf(sout + strlen(sout), "\t%d min %4.2f sec   %s %s %s %s", 
                (int) dt, fmod(dt, 1) * 60, 
                strcpy(s1, hms(fmod(tret[1] + 0.5, 1) * 24, 0)), 
                strcpy(s3, hms(fmod(tret[2] + 0.5, 1) * 24, 0)), 
                strcpy(s4, hms(fmod(tret[3] + 0.5, 1) * 24, 0)),
                strcpy(s2, hms(fmod(tret[4] + 0.5, 1) * 24, 0)));
#endif
	  strcat(sout, "\n");
	  do_printf(sout);
	}
      }
    }   /* endif search_local */
    if ((smod & SMOD_SOLAR) && ! (smod & SMOD_LOCAL)) { 
    /* * global search for eclipses */
      if ((eclflag = swe_sol_eclipse_when_glob(t_ut, whicheph, search_flag,
                tret, direction_flag, serr)) == ERR) {
        do_printf(serr);
        exit(0);
      } 
      t_ut = tret[0];
      if ((eclflag & SE_ECL_TOTAL)) {
        strcpy(sout, "total   ");
	ecl_type = ECL_SOL_TOTAL;
      }
      if ((eclflag & SE_ECL_ANNULAR)) {
        strcpy(sout, "annular ");
	ecl_type = ECL_SOL_ANNULAR;
      }
      if ((eclflag & SE_ECL_ANNULAR_TOTAL)) {
        strcpy(sout, "ann-tot ");
	ecl_type = ECL_SOL_ANNULAR;	/* by Alois: what is this ? */
      }
      if ((eclflag & SE_ECL_PARTIAL)) {
        strcpy(sout, "partial ");
	ecl_type = ECL_SOL_PARTIAL;
      }
      if ((eclflag & SE_ECL_NONCENTRAL) && !(eclflag & SE_ECL_PARTIAL))
        strcat(sout, "non-central ");
      swe_sol_eclipse_where(t_ut, whicheph, geopos, attr, serr);
      swe_revjul(t_ut, gregflag, &jyear, &jmon, &jday, &jut);
      sprintf(sout + strlen(sout), "%2d.%2d.%4d\t%s\t%f km\t%f o/o\n", 
                jday, jmon, jyear, hms(jut,0), attr[3], attr[0]);
      sprintf(sout + strlen(sout), "\t%s ", hms_from_tjd(tret[2])); 
      if (tret[4] != 0)
	  sprintf(sout + strlen(sout), "%s ", hms_from_tjd(tret[4])); 
      else
          strcat(sout, "   -         ");
      if (tret[5] != 0)
	  sprintf(sout + strlen(sout), "%s ", hms_from_tjd(tret[5])); 
      else
          strcat(sout, "   -         ");
      sprintf(sout + strlen(sout), "%s\n", hms_from_tjd(tret[3])); 
#if 0
      swe_revjul(tret[1], gregflag, &jyear, &jmon, &jday, &jut);
      sprintf(sout + strlen(sout), "%2d.%2d.%4d\t%s\t%f km\n", 
                jday, jmon, jyear, hms(jut,0), attr[3]);
#endif
      sprintf(sout + strlen(sout), "\t%s\t%s", 
                strcpy(s1, dms(geopos[0], BIT_ROUND_MIN)),
                strcpy(s2, dms(geopos[1], BIT_ROUND_MIN)));
      if (!(eclflag & SE_ECL_PARTIAL) && !(eclflag & SE_ECL_NONCENTRAL)) {
        if ((eclflag = swe_sol_eclipse_when_loc(t_ut - 10, whicheph,
geopos, tret, attr, 0, serr)) == ERR) {
          do_printf(serr);
          exit(0);
        }
        if (fabs(tret[0] - t_ut) > 1) 
          do_printf("when_loc returns wrong date\n");
        dt = (tret[3] - tret[2]) * 24 * 60;
        sprintf(sout + strlen(sout), "\t%d min %4.2f sec\t",
                (int) dt, fmod(dt, 1) * 60);
      } 
      strcat(sout, "\n");
      if (smod & SMOD_HOCAL) {
	int ihou, imin, isec, isgn;
	double dfrc;
	swe_split_deg(jut, SE_SPLIT_DEG_ROUND_MIN, &ihou, &imin, &isec, &dfrc, &isgn);
#if 0
	sprintf(sout, "\"%04d %02d %02d %02d.%02d %d\",\n", jyear, jmon, jday, ihou, imin, ecl_type);
#else
	sprintf(sout, "%f, %.0f, /* %04d %02d %02d %02d.%02d %d */\n", t_ut, (double) ecl_type, jyear, jmon, jday, ihou, imin, ecl_type);
#endif
      } 
      do_printf(sout);
    }
    if (smod & SMOD_RISE) {
      if (swe_rise_trans(t_ut, ipl, starname, whicheph, SE_CALC_RISE, geopos, 1013.25, 10, &(tret[0]), serr) != OK) {
	do_printf(serr);
	exit(0);
      } 
      if (swe_rise_trans(t_ut, ipl, starname, whicheph, SE_CALC_SET, geopos, 1013.25, 10, &(tret[1]), serr) != OK) {
	do_printf(serr);
	exit(0);
      } 
      strcpy(sout, "rise     ");
      if (tret[0] == 0) strcat(sout, "         -                     ");
      else {
	swe_revjul(tret[0], gregflag, &jyear, &jmon, &jday, &jut);
	sprintf(sout + strlen(sout), "%2d.%2d.%4d\t%s    ", 
		  jday, jmon, jyear, hms(jut,0));
      }
      strcat(sout, "set      ");
      if (tret[1] == 0) strcat(sout, "         -                     \n");
      else {
	swe_revjul(tret[1], gregflag, &jyear, &jmon, &jday, &jut);
	sprintf(sout + strlen(sout), "%2d.%2d.%4d\t%s\n", 
		  jday, jmon, jyear, hms(jut,0));
      }
      do_printf(sout);
    }
    if (smod & SMOD_METR) {
      if (swe_rise_trans(t_ut, ipl, starname, whicheph, SE_CALC_MTRANSIT, geopos, 1013.25, 10, &(tret[0]), serr) != OK) {
	do_printf(serr);
	exit(0);
      } 
      if (swe_rise_trans(t_ut, ipl, starname, whicheph, SE_CALC_ITRANSIT, geopos, 1013.25, 10, &(tret[1]), serr) != OK) {
	do_printf(serr);
	exit(0);
      } 
      strcpy(sout, "mtransit ");
      if (tret[0] == 0) strcat(sout, "         -                     ");
      else {
	swe_revjul(tret[0], gregflag, &jyear, &jmon, &jday, &jut);
	sprintf(sout + strlen(sout), "%2d.%2d.%4d\t%s    ", 
		  jday, jmon, jyear, hms(jut,0));
      }
      strcat(sout, "itransit ");
      if (tret[1] == 0) strcat(sout, "         -                     \n");
      else {
	swe_revjul(tret[1], gregflag, &jyear, &jmon, &jday, &jut);
	sprintf(sout + strlen(sout), "%2d.%2d.%4d\t%s\n", 
		  jday, jmon, jyear, hms(jut,0));
      }
      do_printf(sout);
    }
    if ((smod & (SMOD_RISE | SMOD_METR)) && tret[1] > 0)
      t_ut = tret[1] + 0.1;
    else
      t_ut += direction;
  }
  if (*serr_warn != '\0') {
    do_printf("\nwarning: ");
    do_printf(serr_warn);
    do_printf("\n");
  }
  /* close open files and free allocated space */
  end_main:
  swe_close();
  return  OK;
}

static char *hms_from_tjd(double x) 
{
  static char s[AS_MAXCH];
  sprintf(s, "%s ", hms(fmod(x + 1000000.5, 1) * 24, 0)); 
  return s;
}

static char *hms(double x, int32 iflag)
{
  static char s[AS_MAXCH], s2[AS_MAXCH], *sp;
  char *c = ODEGREE_STRING;
  x += 0.5 / 36000.0; /* round to 0.1 sec */
  strcpy(s, dms(x, iflag));
  sp = strstr(s, c);
  if (sp != NULL) {
    *sp = ':';
    if (strlen(ODEGREE_STRING) > 1)
      strcpy(s2, sp + strlen(ODEGREE_STRING));
      strcpy(sp + 1, s2);
    *(sp + 3) = ':';
    *(sp + 8) = '\0';
  }
  return s;
}

static char *dms(double xv, int32 iflg)
{
  int izod;
  int32 k, kdeg, kmin, ksec;
  char *c = ODEGREE_STRING;
  char *sp, s1[50];
  static char s[50];
  int sgn;
  *s = '\0';
  if (iflg & SEFLG_EQUATORIAL)
    c = "h";
  if (xv < 0) {
    xv = -xv;
    sgn = -1;
  } else
    sgn = 1;
  if (iflg & BIT_ROUND_MIN)
    xv = swe_degnorm(xv + 0.5/60);
  if (iflg & BIT_ROUND_SEC)
    xv = swe_degnorm(xv + 0.5/3600);
  if (iflg & BIT_ZODIAC) {
    izod = (int) (xv / 30);
    xv = fmod(xv, 30);
    kdeg = (int32) xv;
    sprintf(s, "%2d %s ", kdeg, zod_nam[izod]);
  } else {
    kdeg = (int32) xv;
    sprintf(s, " %3d%s", kdeg, c);
  }
  xv -= kdeg;
  xv *= 60;
  kmin = (int32) xv;
  if ((iflg & BIT_ZODIAC) && (iflg & BIT_ROUND_MIN))
    sprintf(s1, "%2d", kmin);
  else
    sprintf(s1, "%2d'", kmin);
  strcat(s, s1);
  if (iflg & BIT_ROUND_MIN)
    goto return_dms;
  xv -= kmin;
  xv *= 60;
  ksec = (int32) xv;
  if (iflg & BIT_ROUND_SEC)
    sprintf(s1, "%2d\"", ksec);
  else
    sprintf(s1, "%2d", ksec);
  strcat(s, s1);
  if (iflg & BIT_ROUND_SEC)
    goto return_dms;
  xv -= ksec;
  k = (int32) (xv * 10000);
  sprintf(s1, ".%04d", k);
  strcat(s, s1);
return_dms:;
  if (sgn < 0) {
    sp = strpbrk(s, "0123456789");
    *(sp-1) = '-';
  }
  if (iflg & BIT_LZEROES) {
    while ((sp = strchr(s+2, ' ')) != NULL) *sp = '0';
  }
  return(s);
}

/* make_ephemeris_path().
 * ephemeris path includes
 *   current working directory
 *   + program directory
 *   + default path from swephexp.h on current drive
 *   +                              on program drive
 *   +                              on drive C:
 */
static int make_ephemeris_path(long iflag, char *argv0)
{
  char path[AS_MAXCH], s[AS_MAXCH];
  char *sp;
  char *dirglue = DIR_GLUE;
  size_t pathlen = 0; 
  /* moshier needs no ephemeris path */
  if (iflag & SEFLG_MOSEPH)
    return OK;
  /* current working directory */ 
  sprintf(path, ".%c", *PATH_SEPARATOR);
  /* program directory */
  sp = strrchr(argv0, *dirglue);
  if (sp != NULL) {
    pathlen = sp - argv0;
    if (strlen(path) + pathlen < AS_MAXCH-1) {
      strcpy(s, argv0);
      *(s+pathlen) = '\0';
      sprintf(path + strlen(path), "%s%c", s, *PATH_SEPARATOR);
    } 
  }
#if MSDOS
{
  char *cpos[20];
  char s[2 * AS_MAXCH], *s1 = s + AS_MAXCH;
  char *sp[3];
  int i, j, np;
  strcpy(s1, SE_EPHE_PATH);
  np = cut_str_any(s1, PATH_SEPARATOR, cpos, 20);
  /* 
   * default path from swephexp.h
   * - current drive
   * - program drive
   * - drive C
   */
  *s = '\0';
  /* current working drive */
  sp[0] = getcwd(NULL, 0);
  if (sp[0] == NULL) {
    do_printf("error in getcwd()\n");
    exit(1);
  } 
  if (*sp[0] == 'C')
    sp[0] = NULL;
  /* program drive */
  if (*argv0 != 'C' && (sp[0] == NULL || *sp[0] != *argv0))
    sp[1] = argv0;
  else
    sp[1] = NULL;
    /* drive C */
  sp[2] = "C";
  for (i = 0; i < np; i++) {
    strcpy(s, cpos[i]);
    if (*s == '.')	/* current directory */
      continue;
    if (s[1] == ':')  /* drive already there */
      continue;
    for (j = 0; j < 3; j++) {
      if (sp[j] != NULL && strlen(path) + 2 + strlen(s) < AS_MAXCH-1)
	sprintf(path + strlen(path), "%c:%s%c", *sp[j], s, *PATH_SEPARATOR);
    }
  }
}
#else
    if (strlen(path) + pathlen < AS_MAXCH-1)
      strcat(path, SE_EPHE_PATH);
#endif
  return OK;
}

static int do_calc(double tjd, int ipl, long iflag, double *x, char *serr)
{
  return swe_calc(tjd, ipl, iflag, x, serr);
}

static void do_printf(char *info)
{
#ifdef _WINDOWS
  fprintf(fp, info);
#else
  printf(info);
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
static int cut_str_any(char *s, char *cutlist, char *cpos[], int nmax)
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

static int letter_to_ipl(int letter)
{
  if (letter >= '0' && letter <= '9')
    return letter - '0' + SE_SUN;
  if (letter >= 'A' && letter <= 'I')
    return letter - 'A' + SE_MEAN_APOG;
  if (letter >= 'J' && letter <= 'X')
    return letter - 'J' + SE_CUPIDO;
  switch (letter) {
  case 'm': return SE_MEAN_NODE;
  case 'n': 
  case 'o': return SE_ECL_NUT;
  case 't': return SE_TRUE_NODE;
  case 'f': return SE_FIXSTAR;
  }
  return -1;
}
