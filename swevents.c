/* SWISSEPH
   

**************************************************************/
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


static char *info = "\n\
  Computes planetary phenomena\n\
  (Venus and Mercury only so far),\n\
  for a given start date and a time range.\n\
  Input can either be a date or an absolute julian day number.\n\
  0:00 (midnight).\n\
  Precision of this program:\n\
  Conjunctions:     Deviations from Rosicrucian Ephemeris result from\n\
		    the fact that R.E. gives ephemeris time.\n\
  Ingresses:        ditto.\n\
  Stations:         ditto. The stations given by AA are different, they\n\
		    are in rectascension.\n\
  Max. Elongations: AA gives date and hour. This program computes \n\
		    e.g. 20 Aug. 96, 3:28. AA has a rounded value of 4h.\n\
  Transits:	    There is no venus transit in 20th cty.\n\
		    There was a transit of mercury in 1993. The ingress\n\
		    and egress times computed by this program agree\n\
		    exactly with AA93, p. A86.\n\
  Visibility:       elongation > 10 degrees, according to AA.\n\
  gr. brillancy:    Our times of greatest brillancy differ from the\n\
		    ones given by AA94 and AA96, p. A3, by several days.\n\
		    Probably an error of AA. The times on p. A3 are\n\
		    inconsistent with the magnitudes listed on p. A4.\n\
		    The maxima computed by this program are consistent\n\
		    with these magnitude tables.\n\
  \n\
  Command line options:\n\
	\n\
	example:\n\
	swevents -p3 -bj2436723.5 -n1000 -s1 -mscreen -ejpl \n\
	\n\
	-mscreen output on screen\n\
	-p    	planet to be computed.\n\
		See the letter coding below.\n\
	-nN	output data for N consecutive days; if no -n option\n\
		is given, the default is 1. If the option -n without a\n\
		number is given, the default is 20.\n\
	-sN	timestep N days, default 1. This option is only meaningful\n\
		when combined with option -n.\n\
	-edirPATH change the directory of the ephemeris files \n\
	-noingr  no ingresses\n\
	-bDATE	use this begin date instead of asking; use -b1.1.1992 if\n\
		the begin date string contains blanks; use format -bj2400000.5\n\
		to express the date as absolute Julian day number.\n\
		Note: the date format is day month year (European style).\n\
	 -eswe  swiss ephemeris\n\
	 -ejpl  jpl ephemeris (DE404), or with ephemeris file name\n\
	 	-ejplde200.eph\n\
	 -emos  moshier ephemeris\n\
	 -true	true positions\n\
	 -noaberr	no aberration\n\
	 -nodefl	no gravitational light deflection\n\
	 -noprec	no precession (i.e. J2000 positions)\n\
	 -nonut		no nutation \n\
\n\
	-?	display this info\n\
\n\
  Planet selection letters:\n\
        m (default) main factors\n\
	+ main planets, lunar orbital points and main asteroids\n\
	h fictitious factors\n\
	a all factors\n\
	0 Sun (character zero)\n\
	1 Moon (character 1)\n\
	2 Mercury\n\
	....\n\
	9 Pluto\n\
	10 mean lunar node\n\
	11 true lunar node\n\
	12 Chiron\n\
	13 mean lunar apogee (Lilith, Black Moon) \n\
	14 Ceres \n\
	15 Pallas \n\
	16 Juno \n\
	17 Vesta \n\
	18 osculation lunar apogee \n\
	19 Cupido \n\
	20 Hades \n\
	21 Zeus \n\
	22 Kronos \n\
	23 Apollon \n\
	24 Admetos \n\
	25 Vulkanus \n\
	26 Poseidon \n\
	27 Isis (Sevin) \n\
	28 Nibiru (Sitchin) \n\
	29 Harrington \n\
	30 \n\
\n\
  Date entry:\n\
  In the interactive mode, when you are asked for a start date,\n\
  you can enter data in one of the following formats:\n\
\n\
	1.2.1991	three integers separated by a nondigit character for\n\
			day month year. Dates are interpreted as Gregorian\n\
			after 4.10.1582 and as Julian Calender before.\n\
			Time is always set to midnight.\n\
			If the three letters jul are appended to the date,\n\
			the Julian calendar is used even after 1582.\n\
			If the four letters greg are appended to the date,\n\
			the Gregorian calendar is used even before 1582.\n\
\n\
	j2400123.67	the letter j followed by a real number, for\n\
			the absolute Julian daynumber of the start date.\n\
			Fraction .5 indicates midnight, fraction .0\n\
			indicates noon, other times of the day can be\n\
			chosen accordingly.\n\
\n\
	<RETURN>	repeat the last entry\n\
	\n\
	.		stop the program\n\
\n\
	+20		advance the date by 20 days\n\
\n\
	-10		go back in time 10 days\n";
/**************************************************************/

#include "sweodef.h"/**/
#include "swephexp.h"
#include "swephlib.h"/**/
//#include "swepcalc.h"
#include "swevents.h"
#include "astrolib.h"
#include <time.h>

#define J2000           2451545.0  /* 2000 January 1.5 */
#define square_sum(x)   (x[0]*x[0]+x[1]*x[1]+x[2]*x[2])
#define SEFLG_EPHMASK   (SEFLG_JPLEPH|SEFLG_SWIEPH|SEFLG_MOSEPH)
#define SUN_RADIUS      (959.63 / 3600 * DEGTORAD)  /*  Meeus germ. p 391 */
#define VENUS_RADIUS	(8.34 / 3600 * DEGTORAD) /* AA96 E43 */
#define MERCURY_RADIUS	(3.36 / 3600 * DEGTORAD) /* AA96 E43 */
#define MARS_RADIUS	(4.68 / 3600 * DEGTORAD) /* AA96 E43 */

#define BIT_ROUND_NO	0
#define BIT_ROUND_SEC	1
#define BIT_ROUND_MIN	2
#define BIT_ZODIAC	4
#define BIT_LZEROES	8

#define STRLEN_OUT_TEXT 20

static char *zod_nam[] = {"ar", "ta", "ge", "cn", "le", "vi", 
			  "li", "sc", "sa", "cp", "aq", "pi"};
#if 0
static char *zod_nam_long[] = {"aries", "taurus", "gemini", 
			  "cancer", "leo", "virgo", 
			  "libra", "scorpio", "sagittarius", 
			  "capricorn", "aquarius", "pisces"};
static char *month_nam[] = {"", "Jan", "Feb", "Mar", "Apr", "May", "Jun",
			    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
#endif

#if 0
static double mag_elem[SE_NPLANETS][4] = {{0, 0, 0, 0},
		     			{0, 0, 0, 0},
				  	{-0.42, 3.80, -2.73, 2.00},
				        {-4.40, 0.09, 2.39, -0.65},
				    	{- 1.52, 1.60, 0, 0},
					};
#endif

#define MAX_LINES	67
#define MAX_COLS	2
double xcol[2] = {20.0, 110.0};
double xdate = 25;	/* position of date */
double xpos = 52;	/* position of date */
double ytop = 24;
double line_space = 4;
char *planet_name;
AS_BOOL ephemeris_time = FALSE;
AS_BOOL do_not_round = FALSE;
AS_BOOL verbose = FALSE;
char scmd[AS_MAXCH];
char sdate[20];

static char *dms(double x, int iflag);
static char *hms(double x, int32 iflag);

/*static int get_planet_names();*/
#if 0
static long init_next_step(double te, int ipl, long iflag, char *serr)
static long do_conj_sun(double te, int ipl, long iflag, char *serr);
static long do_visibility(double te, int ipl, long iflag, char *serr);
static long do_max_elong(double te, int ipl, long iflag, char *serr);
static long do_station(double te, int ipl, long iflag, char *serr);
static long do_brillancy(double te, int ipl, long iflag, char *serr);
static int get_next_ingress(double t, int32 ipl, int32 iflag, INGRESS *ding, char *serr);
#endif

static int32 get_next_voc(double tet0, int32 iflag, int32 vocmethod, VOC *pvoc, char *serr);
int32 calc_all_crossings(
              int32 iflag,    /* swiss ephemeris flags */
              int32 itype,    /* type of calculation:
                               * 0 = mundane aspects, 
                               * 1 = transits,
                               * 2 = ingresses */
              double tjd0,    /* start time */
              double tjde,    /* end time */
              double tstep,   /* time step width */
              char *splan,    /* moving planets' string */
              char *sasp,     /* aspects string */
              int32 npos,     /* with ityp=1, number of transit positions */
              double *dpos,   /* transit/ingress positions */
              struct event *pev,  /* struct for output */
              char *serr      /* error string */
             );
static int32 calc_all_voc(int32 iflag, double te, double tend, char *serr);
static int32 extract_data_of_day(int32 doflag, double te, double dtol, char *splan, char *sasp, EVENT *pev, char *serr);

static int letter_to_ipl(int letter);

/* globals */
#if 0
static double xp0[6], xp1[6], xp2[6];		/* planet */
static double xs0[6], xs1[6], xs2[6]; 		/* sun */
static double xel0[6], xel1[6], xel2[6];	/* elongation in longitude */
static double xang0[6], xang1[6], xang2[6];	/* angular distance from sun */
static double xma0[6], xma1[6], xma2[6];	/* magnitude */
static char sout[AS_MAXCH];
#endif

#define DOFLAG_ASP    1
#define DOFLAG_VOC    2

int main(int argc, char *argv[])
{
  EVENT *pev, *pev0;
  double dhour = 0, tend;
  char serr[256];
  char s[AS_MAXCH], saves[AS_MAXCH]; 
  char *sp, *spsave;
  char *spno;
  char *fmt = "PLBRS";
  char *gap = " ";
  int i, n;
  int jmon, jday, jyear;
  int32 doflag = 0;
  double jut = 0.0;
  int32 ipl, iplfrom = SE_SUN, iplto = SE_TRUE_NODE;
  long nstep = 0;
/*  double x[6], xs[6], x0[6], x1[6], x2[6], xp[6];*/
  char ephepath[80] = "/users/ephe/";
  char fname[80] = "de431.eph";
  char *begindate = NULL;
  long iflag = SEFLG_SPEED;              /* external flag: helio, geo... */
  int whicheph = SEFLG_SWIEPH;   
  AS_BOOL gregflag = TRUE;
  AS_BOOL get_data_of_day = FALSE;
  double tjd = 2436723.588888889;
  double t, te, tstep = 1;
  double delt;
  struct tm *tim;
  time_t tloc;
  *saves = '\0';
  time(&tloc);
  tim = localtime (&tloc);
  sprintf(sdate, "%d/%02d/%02d", 1900+tim->tm_year, tim->tm_mon, tim->tm_mday);
  *scmd = '\0';
  if ((pev0 = (EVENT *) calloc((size_t) NEVENTMAX, sizeof(EVENT))) == NULL) {
    fprintf(stderr, "could not allocate structure for event data\n");
    return ERR;
  }
  for (i = 1; i < argc; i++) {
    sprintf(scmd + strlen(scmd), "%s ", argv[i]);
    if (strcmp(argv[i], "-doasp") == 0) {
      doflag |= DOFLAG_ASP;
    } else if (strcmp(argv[i], "-dovoc") == 0) {
      doflag |= DOFLAG_VOC;
    } else if (strcmp(argv[i], "-getday") == 0) {
      get_data_of_day = TRUE;
    } else if (strcmp(argv[i], "-et") == 0) {
      ephemeris_time = TRUE;
    } else if (strncmp(argv[i], "-j", 2) == 0) {
      tjd = atof(argv[i]+2);
    } else if (strncmp(argv[i], "-ejpl", 5) == 0) {
      whicheph = SEFLG_JPLEPH;
      strcpy(ephepath, "/users/ephe/");
      if (*(argv[i]+5) != '\0')
	strcpy(fname, argv[i]+5);
    } else if (strcmp(argv[i], "-v") == 0) {
      verbose = TRUE;
    } else if (strcmp(argv[i], "-eswe") == 0) {
      whicheph = SEFLG_SWIEPH;
      strcpy(ephepath, "/users/ephe/");
    } else if (strcmp(argv[i], "-emos") == 0) {
      whicheph = SEFLG_MOSEPH;
    } else if (strncmp(argv[i], "-p", 2) == 0) {
      spno = argv[i]+2;
      /* single factor */
      if (*spno >= '0' && *spno <= '9') {
	iplfrom = iplto = atol(spno);
      /* ecliptic, nutation */
      } else if ((ipl = atol(spno)) < 0 && ipl >= -2) {
	iplfrom = iplto = ipl;
      /* main factors */
      } else if (*spno == 'm') {
	iplfrom = SE_SUN;
	iplto = SE_TRUE_NODE;
      /* with Chiron, Lilith, etc. */
      } else if (*spno == '+') {
	iplfrom = SE_SUN;
	iplto = SE_NPLANETS-1;
      /* fictitious factors */
      } else if (*spno == 'h') {
	iplfrom = SE_FICT_OFFSET;
	iplto = iplfrom + SE_NFICT_ELEM-1;
      /* all factors */
      } else if (*spno == 'a') {
	iplfrom = SE_SUN;
	iplto = SE_FICT_OFFSET + SE_NFICT_ELEM-1;
      } else {
	printf("illegal planet number %s\n", argv[i]);
	exit(1);
      }
    } else if (strncmp(argv[i], "-n", 2) == 0) {
      nstep = atoi(argv[i]+2);
    } else if (strncmp(argv[i], "-s", 2) == 0) {
      tstep = atof(argv[i]+2);
    } else if (strncmp(argv[i], "-t", 2) == 0) {
      dhour = atof(argv[i]+2);
    } else if (strncmp(argv[i], "-b", 2) == 0) {
      begindate = argv[i] + 2;
    } else if (strncmp(argv[i], "-f", 2) == 0) {
      fmt = argv[i] + 2;
    } else if (strncmp(argv[i], "-?", 2) == 0) {
      printf(info);
      return 0;
    } else if (strncmp(argv[i], "-g", 2) == 0) {
      gap = argv[i] + 2;
      if (*gap == '\0') gap = "\t";
    } else {
      printf("illegal option %s\n", argv[i]);
      exit(1);
    }
  }
  swe_set_ephe_path(ephepath);
  swe_set_jpl_file(fname);
  iflag |= whicheph;
  if (begindate == NULL) {
    printf ("datum ?");
    sp = gets(s);
  } else {
    sp = begindate;
    begindate = ".";  /* to exit afterwards */
  }
  spsave = sp;
  if (*sp == '.') {
    exit(1);
  } else if (*sp == '\0') {
    strcpy (s, saves);
  } else {
    strcpy (saves, s);
  }
  if (*sp == 'j') {   /* it's a day number */
    sscanf(sp+1,"%lf", &tjd);
    if (tjd < 2299160.5)
      gregflag = FALSE;
    else
      gregflag = TRUE;
    if (strstr(sp, "jul") != NULL)
      gregflag = FALSE;
    else if (strstr(sp, "greg") != NULL)
      gregflag = TRUE;
    //(void) revjul(tjd, gregflag, &jmon, &jday, &jyear, &jut);
    swe_revjul(tjd, gregflag, &jyear, &jmon, &jday, &jut);
  } else if (*sp == '+') {
    n = atoi(sp);
    if (n == 0) n = 1;
    tjd += n;
    //(void) revjul(tjd, gregflag, &jmon, &jday, &jyear, &jut);
    swe_revjul(tjd, gregflag, &jyear, &jmon, &jday, &jut);
  } else if (*sp == '-') {
    n = atoi(sp);
    if (n == 0) n = -1;
    tjd += n;
    //(void) revjul(tjd, gregflag, &jmon, &jday, &jyear, &jut);
    swe_revjul(tjd, gregflag, &jyear, &jmon, &jday, &jut);
  } else {
    if (sscanf (sp, "%d%*c%d%*c%d", &jday,&jmon,&jyear) < 1) exit(1);
    if (jyear * 10000 + jmon * 100 + jday < 15821015) 
      gregflag = FALSE;
    else
      gregflag = TRUE;
    if (strstr(sp, "jul") != NULL)
      gregflag = FALSE;
    else if (strstr(sp, "greg") != NULL)
      gregflag = TRUE;
    //tjd = julday(jmon,jday,jyear,jut,gregflag);
    tjd = swe_julday(jyear, jmon, jday, jut, gregflag);
    tjd += dhour / 24.0;
  }
#if 0
    /*title */
    do_print("Venus Phenomena 1000 BC - 2100 AD, computed by Astrodienst AG 30-Sep-1996\n");
#endif
  t = tjd;
  if (t < 2299160.5)
    gregflag = FALSE;
  else
    gregflag = TRUE;
  if (strstr(spsave, "jul") != NULL)
    gregflag = FALSE;
  else if (strstr(spsave, "greg") != NULL)
    gregflag = TRUE;
  //(void) revjul(t, gregflag, &jmon, &jday, &jyear, &jut);
  swe_revjul(t, gregflag, &jyear, &jmon, &jday, &jut);
  if (!ephemeris_time) {
    delt = swe_deltat(t);
    te = t + delt;
  } else {
    te = t;
  }
  iplto = iplfrom;
  tend = te + nstep;
  /* hier die Berechnungen */
  pev = pev0;
  /* moon void of course */
  if (doflag & DOFLAG_VOC) {
    if (calc_all_voc(iflag, te, tend, serr) == ERR) {
      fprintf(stderr, "%s\n", serr);
      return ERR;
    }
  }
  if (get_data_of_day) {
    if (extract_data_of_day(doflag, te, 1.6, SPLAN_ASPECTS, SASP_ASPECTS, pev, serr) == ERR) {
      fprintf(stderr, "%s\n", serr);
      return ERR;
    } 
    return OK;
  }
  /* mundane aspects */
  if (doflag & DOFLAG_ASP) {
    if (calc_all_crossings(iflag, CTYP_MASPECTS, te, tend, tstep, SPLAN_ASPECTS, SASP_ASPECTS, 0, NULL, pev, serr) == ERR) {
      fprintf(stderr, "%s\n", serr);
      return ERR;
    }
  }
#if 0
if ((0)) {   /* replace by a call of calc_all_crossings */
  /* ingresses */
  INGRESS ding;
  char *splan = SPLAN_INGRESS;
  splan = "01mA";
  /* for each planet */
  for (sp = splan; *sp != '\0'; sp++) {
    ipl = letter_to_ipl((int) *sp);
    for (t = te; t < tend; t = tnext) {
      retc = get_next_ingress(t, ipl, iflag, &ding, serr);
      if (retc == ERR) {
	printf("%s\n", serr);
	return ERR;
      }
      swe_revjul(ding.tingr, gregflag, &jyear, &jmon, &jday, &jut);
      printf("INGRESS: t=%d.%d.%d %f, ipl=%d, isign=%d, direction=%d, ino=%d\n", jday, jmon, jyear, jut, ipl, ding.isign, ding.direction, ding.ino);
      tnext = ding.tingr + 1;
    }
  }
}
#endif
#if 0
  for (ipl = iplfrom; ipl <= iplto; ipl++) {
      /* 
       * maximum elongation 
       */   
      if ((retc = do_max_elong(te, ipl, iflag, serr)) == ERR) {
	fprintf(stderr, "return code %ld, mesg: %s\n", retc, serr);
	exit(1);
      }
      /* 
       * station
       */
      if ((retc = do_station(te, ipl, iflag, serr)) == ERR) {
	fprintf(stderr, "return code %ld, mesg: %s\n", retc, serr);
	exit(1);
      }
      /* 
       * greatest brillancy 
       */
      if ((retc = do_brillancy(te, ipl, iflag, serr)) == ERR) {
	fprintf(stderr, "return code %ld, mesg: %s\n", retc, serr);
	exit(1);
      }
    }
#endif
  /*}*/

  /* close open files and free allocated space */
  swe_close();
  return OK;
}

/* 
 * aspect codes
 * 1:   0
 * 2: 180
 * 3:  90
 * 4: 120
 * 5:  60
 * 6:  30
 * 7: 150
 * 8:  72
 * 9: 144
 * A:  45
 * B: 135
 * C:   0  parallel
 * D:   0  anti-parallel
 * input: 
 * sasp   aspects string, e.g. "1234567"
 * output:
 * dasp   aspect angles: 0, 180, 90, 270, 120, 240, 60, 300, 30, 330, 150, 210
 * saspi  aspects string that defines the angles: 123344556677
 */
static int32 get_aspect_angles(char *sasp, char *saspi, double *dasp, char *serr)
{
  int nasp = 0;
  char *sp;
  *saspi = '\0';
  for (sp = sasp; *sp != '\0'; sp++) {
    switch (*sp) {
    case '1': 
      dasp[nasp] = 0;  
      strcat(saspi, "1");
      nasp++;
      break;
    case '2': 
      dasp[nasp] = 180;  
      strcat(saspi, "2");
      nasp++;
      break;
    case '3': 
      dasp[nasp] = 90;  
      nasp++;
      dasp[nasp] = 270;  
      nasp++;
      strcat(saspi, "33");
      break;
    case '4': 
      dasp[nasp] = 120;  
      nasp++;
      dasp[nasp] = 240;  
      nasp++;
      strcat(saspi, "44");
      break;
    case '5': 
      dasp[nasp] = 60;  
      nasp++;
      dasp[nasp] = 300;  
      nasp++;
      strcat(saspi, "55");
      break;
    case '6': 
      dasp[nasp] = 30;  
      nasp++;
      dasp[nasp] = 330;  
      nasp++;
      strcat(saspi, "66");
      break;
    case '7': 
      dasp[nasp] = 150;  
      nasp++;
      dasp[nasp] = 210;  
      nasp++;
      strcat(saspi, "77");
      break;
    case '8': 
      dasp[nasp] = 72;  
      nasp++;
      dasp[nasp] = 288;  
      nasp++;
      strcat(saspi, "88");
      break;
    case '9': 
      dasp[nasp] = 144;  
      nasp++;
      dasp[nasp] = 216;  
      nasp++;
      strcat(saspi, "99");
      break;
    case 'A': 
      dasp[nasp] = 45;  
      nasp++;
      dasp[nasp] = 315;  
      nasp++;
      strcat(saspi, "AA");
      break;
    case 'B': 
      dasp[nasp] = 135;  
      nasp++;
      dasp[nasp] = 225;  
      nasp++;
      strcat(saspi, "BB");
      break;
    default:
      sprintf(serr, "aspects string %s is invalid", sasp);
      return ERR;
      break;
    }
  }
  return nasp;
}

static int32 call_swe_calc(double tjd, int32 ipl, int32 iflag, char *star, double *x, char * serr)
{
  if (ipl == -2) 
    fprintf(stderr, "hallo %d %s\n", ipl, star);
  if (ipl == SE_FIXSTAR) {
    return swe_fixstar(star, tjd, iflag, x, serr);
  } else {
    return swe_calc(tjd, ipl, iflag, x, serr);
  }
}

/* 
 * Binary search of minimum orb with an almost-aspect 
 * and of exact aspects that happen twice within step width
 */
static int get_crossing_bin_search(double dt, double tt0, double dang, double xta1,  double xta2,  double xtb1,  double xtb2, double *tret, int32 ipla, int32 iplb, char *stara, char *starb, int32 iflag, AS_BOOL is_transit, char *serr)
{
  double d12, d1, d2, tt1, xa[6], xb[6];
  /*swe_revjul(tt0, 1, &jyear, &jmon, &jday, &jut);
  printf("%d%02d%02d %.2f: %c - %c %d\n", jyear, jmon, jday, jut, *spa, *spb, (int) dang);*/
  d1 = swe_degnorm(xta1 - xtb1 - dang);
  if (d1 > 180) d1 -= 360;
  d2 = swe_degnorm(xta2 - xtb2 - dang);
  if (d2 > 180) d2 -= 360;
#if 0
  /* this is handled by calling function */
  if (ipla == SE_MOON || iplb == SE_MOON) {
    if (fabs(d1) >  20)
      continue;
  } else if (fabs(d1) > 5) {
    continue;
  }
#endif
  while(dt > HUNDTHOFSEC) {
    dt /= 2.0;
    tt1 = tt0 + dt;
    if (call_swe_calc(tt1, ipla, iflag, stara, xa, serr) == ERR)
      return ERR;
    if (is_transit)
      xb[0] = xtb1;
    else
      if (call_swe_calc(tt1, iplb, iflag, starb, xb, serr) == ERR)
	return ERR;
    d12 = swe_degnorm(xa[0] - xb[0] - dang);
    if (d12 > 180) d12 -= 360;
    if (d1 * d12 < 0) {
      xta2 = xa[0];
      xtb2 = xb[0];
    } else {
      xta1 = xa[0];
      xtb1 = xb[0];
      tt0 += dt;
    }
    d1 = swe_degnorm(xta1 - xtb1 - dang);
    if (d1 > 180) d1 -= 360;
    d2 = swe_degnorm(xta2 - xtb2 - dang);
    if (d2 > 180) d2 -= 360;
  }
  *tret = tt0;
  return OK;
}

/* Binary search of exact aspect
 * This function finds aspects if exactness occurs only once within step
 * width */
static int get_near_crossing_bin_search(double dt, double tt0, double dang, double xta1,  double xta2,  double xtb1,  double xtb2, double *tret, double *tret2, double *dorb, int32 ipla, int32 iplb, char *stara, char *starb, int32 iflag, char *serr)
{
  double d12, d1, d2, tt1, xa[6], xb[6];
  *tret = 0;
  *tret2 = 0;
  *dorb = 0;
  /*swe_revjul(tt0, 1, &jyear, &jmon, &jday, &jut);
  printf("%d%02d%02d %.2f: %c - %c %d\n", jyear, jmon, jday, jut, *spa, *spb, (int) dang);*/
  d1 = swe_degnorm(xta1 - xtb1 - dang);
  if (d1 > 180) d1 -= 360;
  d2 = swe_degnorm(xta2 - xtb2 - dang);
  if (d2 > 180) d2 -= 360;
  while(dt > HUNDTHOFSEC) {
    dt /= 2.0;
    tt1 = tt0 + dt;
    if (call_swe_calc(tt1, ipla, iflag, stara, xa, serr) == ERR)
      return ERR;
    if (call_swe_calc(tt1, iplb, iflag, starb, xb, serr) == ERR)
      return ERR;
    d12 = swe_degnorm(xa[0] - xb[0] - dang);
    if (d12 > 180) d12 -= 360;
    if (d1 * d12 < 0 || d12 * d2 < 0) {
      if (get_crossing_bin_search(dt, tt0, dang, xta1, xa[0], xtb1, xb[0], tret, ipla, iplb, stara, starb, iflag, FALSE, serr) == ERR)
	return ERR;
      if (get_crossing_bin_search(dt, tt1, dang, xa[0], xta2, xb[0], xtb2, tret2, ipla, iplb, stara, starb, iflag, FALSE, serr) == ERR)
	return ERR;
      *dorb = 0;
      return OK;
      /*sprintf(serr, "crossing missed near t=%f, ipl1=%d, ipl2=%d, dang=%.0f\n", tt1, ipla, iplb, dang);
      return ERR;*/
    } else if (fabs(d2) > fabs(d1)) {
      xta2 = xa[0];
      xtb2 = xb[0];
    } else {
      xta1 = xa[0];
      xtb1 = xb[0];
      tt0 += dt;
    }
    d1 = swe_degnorm(xta1 - xtb1 - dang);
    if (d1 > 180) d1 -= 360;
    d2 = swe_degnorm(xta2 - xtb2 - dang);
    if (d2 > 180) d2 -= 360;
  }
  *tret = tt0;
  *dorb = d1;
  return OK;
}

static void fill_pev_day(EVENT *pevd, double tjd, int ipla, int iplb, char *stara, char *starb, int iasp, int bpind, double dasp, double dang, double dorb, char *strg) 
{
  int jyear, jmon, jday;
  double jut;
  char s[AS_MAXCH];
  char spl1[30], spl2[30];
  swe_get_planet_name(ipla, spl1);
  swe_get_planet_name(iplb, spl2);
  if (ipla == SE_FIXSTAR) 
    strcpy(spl1, stara);
  if (iplb == SE_FIXSTAR) 
    strcpy(spl2, starb);
  spl1[3] = '\0';
  spl2[3] = '\0';
  pevd->ipla = ipla;
  pevd->iplb = iplb;
  strcpy(pevd->stnama, spl1);
  strcpy(pevd->stnamb, spl2);
  pevd->tjd = tjd;
  pevd->iasp = iasp;
  pevd->bpind = bpind;
  pevd->dasp = dasp;
  pevd->dang = dang;
  pevd->dorb = dorb;
  tjd -= swe_deltat(tjd);  /* now we have UT */
  swe_revjul(tjd, 1, &jyear, &jmon, &jday, &jut);
  sprintf(s, "%d/%02d/%02d %s: %s - %s ang=%d", jyear, jmon, jday, hms(jut, BIT_LZEROES), spl1, spl2, (int) dang);
  if (fabs(dorb) > 0)
    sprintf(s + strlen(s), " orb=%.4f", dorb);
  if (strg != NULL && *strg != '\0')
    sprintf(s + strlen(s), " %s", strg);
  strcat(s, "\n");
 /*  printf(s);*/
}

static void test_print_date(double tjd, int ipla, int iplb, char *stara, char *starb, double dang, double dorb, char *strg) 
{
  int jyear, jmon, jday;
  double jut;
  char s[AS_MAXCH];
  char spl1[30], spl2[30];
  swe_get_planet_name(ipla, spl1);
  swe_get_planet_name(iplb, spl2);
  if (ipla == SE_FIXSTAR) 
    strcpy(spl1, stara);
  if (iplb == SE_FIXSTAR) 
    strcpy(spl2, starb);
  spl1[3] = '\0';
  spl2[3] = '\0';
  tjd -= swe_deltat(tjd);  /* now we have UT */
  swe_revjul(tjd, 1, &jyear, &jmon, &jday, &jut);
  sprintf(s, "%d/%02d/%02d %s: %s - %s ang=%.0f", jyear, jmon, jday, hms(jut, BIT_LZEROES), spl1, spl2, dang);
  if (fabs(dorb) > 0)
    sprintf(s + strlen(s), " orb=%.4f", dorb);
  if (strg != NULL && *strg != '\0')
    sprintf(s + strlen(s), " %s", strg);
  strcat(s, "\n");
  printf(s);
}

static char *forw_splan(char *sp)
{
  char *sp2;
  /*fprintf(stderr, "vor=%s; ", sp);*/
  if (*sp == ',') {
    sp2 = strchr(sp, ']');
    if (sp2 == NULL)
      sp = sp + strlen(sp); /* end of string splan */
    else
      sp = sp2 + 1;
  } else {
    sp++;
  }
  /* fprintf(stderr, "nach=%s;\n", sp);*/
  return sp;
}

/*
 * Function returns the ipl or name (if star) of the next object in 
 * the planets string.
 * The planets string is formed as follows:
 * "0123456789mtAFD,f[Gal],f[Ald],a[136199],a[433]"
 * The characters before the comma are planet codes as we use them
 * in other software.
 * Asteroids are coded as follows ",a[mpc_number]".
 * Fixed stars are coded as follows ",f[star_name]".
 */
static int32 letter_to_ipl_or_star(char *s, char *stnam)
{
  char *sp, *sp2;
  int ipl = letter_to_ipl((int) *s);
  if (*s == ',') {
    sp = s + 1;
    /* fixed star */
    if (*sp == 'f') {
      sp += 2;
      sp2 = strchr(sp, ']');
      if (sp2 == NULL) { /* bracket at end of string missing */
        strcpy(stnam, sp);
      } else {
        strncpy(stnam, sp, sp2 - sp);
	stnam[sp2 - sp] = '\0';
      }
      ipl = SE_FIXSTAR;
    } else if (*sp == 'a') {
      sp += 2;
      ipl = atoi(sp) + SE_AST_OFFSET;
    }
  }
  return ipl;
}

static int pev_compare(const EVENT *a1, const EVENT *a2)
{
  if (a1->tjd > a2->tjd)
    return 1;
  if (a1->tjd < a2->tjd)
    return -1;
  return 0;
}

#define SWEV_ASPORB 1
#define NSTARS_MAX 30
#define NMAXPL 50
#define NEAR_CROSSING_ORB 1
#define FOUTNAM   "sweasp.dat"
#define PATH_FOUTNAM   "/home/dieter/sweph"
//#define PATH_FOUTNAM   "/home/ephe/"

static int read_sweasp_dat(char *foutnam) 
{
  FILE *fpout;
  char serr[AS_MAXCH];
  int32 flen = 0, headerlen = 0;
  int32 i, nrec = 0, ipla, iplb, iasp;
  double tjd, dasp, tjd_pre, tjd_post;
  int jyear, jmon, jday;
  double jut, dorb, dur;
  char s[AS_MAXCH], *sp;
  char spl1[30], spl2[30];
  /* open aspects file */
  if ((fpout = fopen(foutnam, BFILE_R_ACCESS)) == NULL) {
    sprintf(serr, "could not open file %s", foutnam);
    return ERR;
  }
  while (fgets(s, AS_MAXCH, fpout) != NULL && (sp = strchr(s, '\n')) != NULL) {
    headerlen += strlen(s);
    if (strncmp(s, "####", 4) == 0) 
      break;
  }
  if (fseek(fpout, 0L, SEEK_END) != 0) {
    sprintf(serr, "fseek failed (SEEK_END): %s", foutnam);
    return ERR;
  }
  flen = ftell(fpout) - headerlen;
  nrec = flen / 52;
  if (fseek(fpout, headerlen, SEEK_SET) != 0) {
    sprintf(serr, "fseek failed (SEEK_SET): %s", foutnam);
    return ERR;
  }
dur = 0;
  for (i = 0; i < nrec; i++) {
    fread((void *) &tjd, sizeof(double), 1, fpout);
    fread((void *) &ipla, sizeof(int32), 1, fpout);
    fread((void *) &iplb, sizeof(int32), 1, fpout);
    fread((void *) &iasp, sizeof(int32), 1, fpout);
    fread((void *) &dasp, sizeof(double), 1, fpout);
    fread((void *) &dorb, sizeof(double), 1, fpout);
    fread((void *) &tjd_pre, sizeof(double), 1, fpout);
    fread((void *) &tjd_post, sizeof(double), 1, fpout);
if ((0)) { /* test output find longest possible aspect duration */
if (ipla <= 9 && tjd_pre > 0 && tjd_post > 0 && tjd_post - tjd_pre > dur) {
  dur = tjd_post - tjd_pre;
  fprintf(stderr, "dur = %f\n", dur);
} else {
  continue;
}
}
    swe_get_planet_name(ipla, spl1);
    swe_get_planet_name(iplb, spl2);
/*    if (ipla == SE_FIXSTAR) 
      strcpy(spl1, stara);
    if (iplb == SE_FIXSTAR) 
      strcpy(spl2, starb);*/
    spl1[3] = '\0';
    spl2[3] = '\0';
    tjd -= swe_deltat(tjd);  /* now we have UT */
    swe_revjul(tjd, 1, &jyear, &jmon, &jday, &jut);
    sprintf(s, "%d/%02d/%02d %s: %s - %s ang=%.0f, orb=%.4f, %.5f, %.5f, %.5f", jyear, jmon, jday, hms(jut, BIT_LZEROES), spl1, spl2, dasp, dorb, tjd, tjd_pre, tjd_post);
    strcat(s, "\n");
    fprintf(stderr, s);
  }
  fclose(fpout);
  return OK;
}

/* Search for mundane aspects.
 * The algorithm finds
 * 1. exact aspects
 * 2. near aspects with orb < 3 before the planets separate again
 */
int32 calc_mundane_aspects(int32 iflag, double tjd0, double tjde, double tstep, 
  char *splan, char *sasp, EVENT *pev, char *serr)
{
  int32 ipl, ipla, iplb, ipli, iplia, iplib, bpind;
  int ist = -1;
  char *sp, *spa, *spb;
  char stnam[40], stnama[40], stnamb[40];
  double t, tt0, tret, tret2, dang = 0, dorb = 0;
  double x[6], x1[30], x2[30], x1d[30], x2d[30];
  double xa[6], xa1[30], xa2[30], xa1d[30], xa2d[30];
  double d1d, d2d;
  double xta1, xta2, xtb1, xtb2, dt, d1, d2;
  int32 retflag = 0;
  int32 jyear, jmon, jday;
  double jut;
  char saspi[30];
  double dasp[30];
  int iaspi;
  int nasp = get_aspect_angles(sasp, saspi, dasp, serr);
  int nev;
  EVENT events_day[500], *pevd = &(events_day[0]);
  struct aspdat aspdat[NMAXPL * NMAXPL], *pasp;
  char foutnam[AS_MAXCH];
  FILE *fpout = NULL;
  strcpy(foutnam, FOUTNAM);
  if ((fpout = fopen(foutnam, "w+")) == NULL) {
    sprintf(serr, "could not open file %s", foutnam);
    return ERR;
  }
  fprintf(fpout, "sweasp.dat, mundane aspects\ncreation date: %s\ncommand: swevents %s\n", sdate, scmd);
  swe_revjul(tjd0, 1, &jyear, &jmon, &jday, &jut);
  fprintf(fpout, "start date: %d/%02d/%02d, ", jyear, jmon, jday);
  swe_revjul(tjde, 1, &jyear, &jmon, &jday, &jut);
  fprintf(fpout, "end date:   %d/%02d/%02d\n", jyear, jmon, jday);
  fprintf(fpout, "planets:    %s\n", splan);
  fprintf(fpout, "aspects:    %s\n", sasp);
  fprintf(fpout, "orb:        %.2f\n", (double) SWEV_ASPORB);
  fprintf(fpout, "aspects 90 and 270 are distinguished; the coding of aspects is therefore as follows:\n");
  fprintf(fpout, "aspcode:    0123456789...\n");
  fprintf(fpout, "          = %s\n", saspi);
  fprintf(fpout, "data structure:\ndouble time of exactness (TT)\nint32  number of planet a\nint32  number of planet b\nint32  number of aspect\ndouble aspect angle\ndouble precision of aspect; 0 if exact\ndouble time of crossing of pre-orb\ndouble time of crossing of post-orb\n");
  fprintf(fpout, "Aspects between nodes and apsides have no pre-orb and post-orb.\nIf an aspect comes into orb but does not become exact, time of exactness is the moment of closest approach.\nIf an aspect has no preorb, there is another exactness before that. And if it has no postorb, there is another exactness after that.\n");
  fprintf(fpout, "######################\n");
  memset((void *) &(aspdat[0]), 0, NMAXPL * NMAXPL * sizeof(struct aspdat));
  /* *stnam = '\0';*/
  for (t = tjd0; t < tjde; t += tstep) {
    ist = -1;
    nev = 0;
    pevd = &(events_day[0]);
    for (sp = splan, ipli = 0; *sp != '\0'; sp = forw_splan(sp), ipli++) {
      ipl = letter_to_ipl_or_star(sp, stnam);
      if (t == tjd0) {
	if (call_swe_calc(t, ipl, iflag|SEFLG_SPEED, stnam, x, serr) == ERR)
	  return ERR;
	if (call_swe_calc(t, ipl, iflag|SEFLG_SPEED|SEFLG_EQUATORIAL, stnam, xa, serr) == ERR)
	  return ERR;
	x1[ipli] = x[0];
	x1d[ipli] = x[0] + tstep / 10.0 * x[3];
	/* declination */
	xa1[ipli] = xa[1]; 
	xa1d[ipli] = xa[1] + tstep / 10.0 * xa[4];
      } else {
        x1[ipli] = x2[ipli];
	x1d[ipli] = x2d[ipli];
	/* declination */
        xa1[ipli] = xa2[ipli];
	xa1d[ipli] = xa2d[ipli];
      }
      if (call_swe_calc(t + tstep, ipl, iflag|SEFLG_SPEED, stnam, x, serr) == ERR)
	return ERR;
      if (call_swe_calc(t + tstep, ipl, iflag|SEFLG_SPEED|SEFLG_EQUATORIAL, stnam, xa, serr) == ERR)
	return ERR;
      x2[ipli] = x[0];
      x2d[ipli] = x[0] + tstep / 10.0 * x[3];
      /* declination */
      xa2[ipli] = xa[1];
      xa2d[ipli] = xa[1] + tstep / 10.0 * xa[4];
    }
    /* for all planets a */
    for (spa = splan, iplia = 0; *spa != '\0'; spa = forw_splan(spa), iplia++) {
      ipla = letter_to_ipl_or_star(spa, stnama);
      /* fixed stars are not considered to transit over other bodies */
      if (ipla == SE_FIXSTAR) continue; /* ????? */
      /* for all planets b */
      for (spb = forw_splan(spa), iplib = iplia + 1; *spb != '\0'; spb = forw_splan(spb), iplib++) {
	iplb = letter_to_ipl_or_star(spb, stnamb);
	bpind = iplia * NMAXPL + iplib;
	/* for all aspects */
	for (iaspi = 0; iaspi < nasp; iaspi++) {
	  int iorb, norb = 3;
	  int orbfac;
	  double dmaxorb;
	  int iasp = (int) saspi[iaspi] - (int) '0';
	  /* for pre-orb, exact, post-orb: */
	  for (iorb = 0; iorb < norb; iorb++) {
	    /* no pre- and post orbs for aspects between different kinds of
	     * nodes and apsides */
	    if (norb == 3 && iorb != 1) {
	      if (strchr("mtABcg", *spa) != NULL && strchr("mtABcg", *spb) != NULL)
	         continue;
	    }
	    orbfac = (iorb - 1);  /* is -1, 0, 1 */
	    if (norb == 1)
	      orbfac = 0;
	    dmaxorb = SWEV_ASPORB * orbfac;
	    dang = swe_degnorm(dasp[iaspi] + dmaxorb);
	    xta1 = x1[iplia];
	    xta2 = x2[iplia];
	    xtb1 = x1[iplib];
	    xtb2 = x2[iplib];
	    tt0 = t;
	    dt = tstep;
	    d1 = swe_degnorm(xta1 - xtb1 - dang);
	    if (d1 > 180) d1 -= 360;
	    d2 = swe_degnorm(xta2 - xtb2 - dang);
	    if (d2 > 180) d2 -= 360;
	    if (ipla == SE_MOON || iplb == SE_MOON) {
	      if (fabs(d1) >  20)
		continue;
	    } else if (fabs(d1) > NEAR_CROSSING_ORB + 3) {
	      continue;
	    }
	    d1d = swe_degnorm(x1d[iplia] - x1d[iplib] - dang);
	    if (d1d > 180) d1d -= 360;
	    d2d = swe_degnorm(x2d[iplia] - x2d[iplib] - dang);
	    if (d2d > 180) d2d -= 360;
	    /* 
	     * crossing found 
	     * find t of exact aspect
	     * if exactness happens twice within step width, the aspect is
	     * lost
	     */
	    if (d1 * d2 < 0) {
	      /*
	       * step width 1 day:     1min20sec/100 yr (1 or 2 asp lost per cty)
	       * step width 0.1 day:   1min55sec/100 years
	       * step width 0.01 day:  9min32sec/100 years 
	       * (calculations with SEFLG_NONUT)
	       * Still, we use 1-day step width. The lost aspects will
	       * be found in the "else".
	       */
	      if ((retflag = get_crossing_bin_search(dt, tt0, dang, xta1, xta2, xtb1, xtb2, &tret, ipla, iplb, stnama, stnamb, iflag, FALSE, serr)) == ERR)
		return ERR;
	      fill_pev_day(pevd + nev, tret, ipla, iplb, stnama, stnamb, iasp, bpind, dasp[iaspi], dang, 0, NULL);
	      nev++;
	    /* 
	     * - near crossing occurs (t of smallest orb is found)
	     * - or exact aspect occurs twice within step width
	     *   (was lost by "if")
	     */
	    } else if (fabs(d1) < NEAR_CROSSING_ORB || fabs(d2) < NEAR_CROSSING_ORB) {
	      /* printf("d1=%f, d1d=%f, d2=%f, d2d=%f\n", d1, d1d, d2, d2d);*/
	      if (d1 > 0 && d2 > 0) {
		if (d1 > d2 && d2 > d2d) continue;
		if (d1 < d2 && d1 < d1d) continue;
	      } else {
		if (d1 > d2 && d1 > d1d) continue;
		if (d1 < d2 && d2 < d2d) continue;
	      }
	      if ((retflag = get_near_crossing_bin_search(dt, tt0, dang, xta1, xta2, xtb1, xtb2, &tret, &tret2, &dorb, ipla, iplb, stnama, stnamb, iflag, serr)) == ERR)
		return ERR;
	      if (retflag == -2)
		continue;
	      if (fabs(dorb) > 0) {
		if (orbfac == 0) {
		  fill_pev_day(pevd + nev, tret, ipla, iplb, stnama, stnamb, iasp, bpind, dasp[iaspi], dang, dorb, "*");
		  nev++;
		}
	      } else {
		fill_pev_day(pevd + nev, tret, ipla, iplb, stnama, stnamb, iasp, bpind, dasp[iaspi], dang, dorb, "x");
		nev++;
		if (tret2 != 0) {
		  fill_pev_day(pevd + nev, tret2, ipla, iplb, stnama, stnamb, iasp, bpind, dasp[iaspi], dang, dorb, "x");
		  nev++;
		}
	      }
	    }
	  }
	}
      }
    }
    /* sort transits of current day (time step) */
    qsort((void *) &(events_day[0]), (size_t) nev, sizeof(EVENT),
          (int (*)(const void *, const void *))(pev_compare));
    for (pevd = &(events_day[0]); pevd - &(events_day[0]) < nev; pevd++) {
      test_print_date(pevd->tjd, pevd->ipla, pevd->iplb, pevd->stnama, pevd->stnamb, pevd->dang, pevd->dorb, "");
      pasp = &(aspdat[pevd->bpind]);
      /* new aspect between body pair: init structure */
      if (pasp->tjd == 0 && pasp->tjd_pre == 0) 
        pasp->iasp = -1;
      if (pasp->iasp != pevd->iasp) {
        pasp->tjd = 0;
        pasp->tjd_pre = 0;
	pasp->iasp = pevd->iasp;
      }
      /* aspect is exact */
      if (pevd->dasp == pevd->dang) {
	/* if there was another exactness before this one, we delete tjd_pre */
        if (pasp->tjd != 0) 
	  pasp->tjd_pre = 0;
        pasp->tjd = pevd->tjd;
        /* write database */
	if (fseek(fpout, 0, SEEK_END) != 0) {
	  strcpy(serr, "error in fseek (1)");
	  return ERR;
	}
	fwrite((char *) &(pasp->tjd), sizeof(double), 1, fpout);
	fwrite((char *) &(pevd->ipla), sizeof(int32), 1, fpout);
	fwrite((char *) &(pevd->iplb), sizeof(int32), 1, fpout);
	fwrite((char *) &(pevd->iasp), sizeof(int32), 1, fpout);
	fwrite((char *) &(pevd->dasp), sizeof(double), 1, fpout);
	fwrite((char *) &(pevd->dorb), sizeof(double), 1, fpout);
	fwrite((char *) &(pasp->tjd_pre), sizeof(double), 1, fpout);
	pasp->fpos_tjd_post = ftell(fpout);
	fwrite((char *) &(pasp->tjd_post), sizeof(double), 1, fpout);
        /*fprintf(stderr, "%f: %d - %d ang=%.0f tpre=%f\n", pasp->tjd, pevd->ipla, pevd->iplb, pevd->dasp, pasp->tjd_pre);*/
      /* entering orb: save tjd_pre */
      } else if (pasp->tjd == 0) {
        pasp->tjd_pre = pevd->tjd;
      /* leaving orb */
      } else {
        /* write database: tjd_post */
	if (fseek(fpout, pasp->fpos_tjd_post, SEEK_SET) != 0) {
	  strcpy(serr, "error in fseek (1)");
	  return ERR;
	}
	fwrite((char *) &(pevd->tjd), sizeof(double), 1, fpout);
	/* after writing database, init aspdat */
	pasp->iasp = -1;
        pasp->tjd = 0;
        pasp->tjd_pre = 0;
      }
    }
  }
  fclose(fpout);
  read_sweasp_dat(foutnam);
	//if (get_crossings(iflag, t, tstep, ipl1, ipl2, x1, x2, sasp, pev, serr) == ERR)
  return OK;
}

static int do_fread_double(FILE *fpout, char *foutnam, int32 fposbeg, double *tjdbeg, char *serr)
{
  if (fseek(fpout, fposbeg, SEEK_SET) != 0) {
    sprintf(serr, "fseek failed (SEEK_SET): %s", foutnam);
    return ERR;   
  }
  if (fread((void *) tjdbeg, sizeof(double), 1, fpout) <= 0) {
    sprintf(serr, "error while trying to read %s (1)", foutnam);
    return ERR;   
  }
  return OK;
}

static int do_fread_asp(FILE *fpout, int32 fpos1, double *tjd_ex, int *ipla, int *iplb, int *iasp, double *dasp, double *dorb, double *tjd_pre, double *tjd_post, char *serr)
{
  if (fseek(fpout, fpos1, SEEK_SET) != 0) {
    sprintf(serr, "do_fread_asp(): fseek failed (SEEK_SET)");
    return ERR;   
  }
  if (fread((void *) tjd_ex, sizeof(double), 1, fpout) <= 0) {
    sprintf(serr, "do_fread_asp(): error while trying to read");
    return ERR;   
  }
  if (fread((void *) ipla, sizeof(int), 1, fpout) <= 0) {
    sprintf(serr, "do_fread_asp(): error while trying to read");
    return ERR;   
  }
  if (fread((void *) iplb, sizeof(int), 1, fpout) <= 0) {
    sprintf(serr, "do_fread_asp(): error while trying to read");
    return ERR;   
  }
  if (fread((void *) iasp, sizeof(int), 1, fpout) <= 0) {
    sprintf(serr, "do_fread_asp(): error while trying to read");
    return ERR;   
  }
  if (fread((void *) dasp, sizeof(double), 1, fpout) <= 0) {
    sprintf(serr, "do_fread_asp(): error while trying to read");
    return ERR;   
  }
  if (fread((void *) dorb, sizeof(double), 1, fpout) <= 0) {
    sprintf(serr, "do_fread_asp(): error while trying to read");
    return ERR;   
  }
  if (fread((void *) tjd_pre, sizeof(double), 1, fpout) <= 0) {
    sprintf(serr, "do_fread_asp(): error while trying to read");
    return ERR;   
  }
  if (fread((void *) tjd_post, sizeof(double), 1, fpout) <= 0) {
    sprintf(serr, "do_fread_asp(): error while trying to read");
    return ERR;   
  }
  return OK;
}

static int start_and_end_date_sweasp(FILE *fpout, char *foutnam, int32 fposbeg, int32 fposlast, double *tjdbeg, double *tjdend, char *serr)
{
  if (do_fread_double(fpout, foutnam, fposbeg, tjdbeg, serr) == ERR)
    return ERR;
  if (do_fread_double(fpout, foutnam, fposlast, tjdend, serr) == ERR)
    return ERR;
  return OK;   
}

static int is_node_apsis(int ipl) 
{
  if (ipl == SE_MEAN_NODE || ipl == SE_TRUE_NODE 
    || ipl == SE_MEAN_APOG || ipl == SE_OSCU_APOG
    || ipl == SE_INTP_APOG || ipl == SE_INTP_PERG)
    return 1;
  return 0;
}

#define SWEASP_DAT_RECLEN  52
#define PERIOD_PRE_POST  365 /* days */
/* returns all aspects, that are within orb during the time (tjd +- dtol) */
static int32 extract_data_of_day(int32 doflag, double tjd, double dtol, char *splan, char *sasp, EVENT *pev, char *serr)
{
  int32 flen = 0, headerlen = 0, fposfirst = 0, fposlast = 0, nrecdif;
  int32 fpos0, fpos1, fpos2;
  int32 retc, nrec = 0, ipla, iplb, iasp;
  double dasp, dorb, dorb2, tjd_pre, tjd_post, tjd_ex, tjdstart;
  double tjd0, tjd1, tjd2;
  int ipla2, iplb2, iasp2;
  double tjd_pre2, tjd_post2, tjd_ex2;
  double tjdbeg, tjdend;
  char s[AS_MAXCH], *sp;
  FILE *fpout = NULL;
  char foutnam[AS_MAXCH];
  /* open aspects file */
  sprintf(foutnam, "%s%s", PATH_FOUTNAM, FOUTNAM);
#if 0
read_sweasp_dat(foutnam);
return OK;
#endif
  if ((fpout = fopen(foutnam, BFILE_R_ACCESS)) == NULL) {
    sprintf(serr, "could not open file %s", foutnam);
    return ERR;
  }
  /* read file header */
  while (fgets(s, AS_MAXCH, fpout) != NULL && (sp = strchr(s, '\n')) != NULL) {
    headerlen += strlen(s);
    if (strncmp(s, "####", 4) == 0) 
      break;
  }
  /* start position of file */
  fposfirst = headerlen;
  /* end position of file */
  if (fseek(fpout, 0L, SEEK_END) != 0) {
    sprintf(serr, "fseek failed (SEEK_END): %s", foutnam);
    return ERR;
  }
  fposlast = ftell(fpout); /* pointer to end of file */
  flen = fposlast - headerlen;
  /* number of records in file */
  nrec = flen / SWEASP_DAT_RECLEN;
  fposlast = fposfirst + (nrec - 1) * SWEASP_DAT_RECLEN; /* pointer to last record in file */
  /* is search date in file range ? */
  if ((retc = start_and_end_date_sweasp(fpout, foutnam, fposfirst, fposlast, &tjdbeg, &tjdend, serr)) == ERR) 
    goto end_extract;
  if (tjd <= tjdbeg || tjd >= tjdend) {
    sprintf(serr, "date %f is beyond range of file %s (%.0f - %.0f)", tjd, foutnam, tjdbeg, tjdend);
    return ERR;
    goto end_extract;
  }
  /* binary search for date in file */
  tjdstart = tjd - PERIOD_PRE_POST; /* we start our search a year before the date we want */
  if (tjdstart < tjdbeg)
    tjdstart = tjdbeg;
  nrecdif = (fposlast - fposfirst) / SWEASP_DAT_RECLEN;
  tjd0 = tjdbeg; tjd2 = tjdend; tjd1 = tjd0;
  fpos0 = fposfirst; fpos2 = fposlast;
  while(fabs(tjdstart - tjd1) > 2) {
    fpos1 = fpos0 + (nrecdif / 2) * SWEASP_DAT_RECLEN;
    if ((retc = do_fread_double(fpout, foutnam, fpos1, &tjd1, serr)) == ERR) {
      goto end_extract;
    }
    if (tjdstart >= tjd1) {
      tjd0 = tjd1; 
      fpos0 = fpos1;
    } else {
      tjd2 = tjd1; 
      fpos2 = fpos1;
    }
    nrecdif = (fpos2 - fpos0) / SWEASP_DAT_RECLEN;
  }
  /*fprintf(stderr, "ts = %.0f, t0 = %.0f, t1 = %.0f, t2 = %.0f, d = %d\n", tjdstart, tjd0, tjd1, tjd2, nrecdif);*/
  /* now, from this date we start searching for aspects whose period covers
   * or overlaps with our date tjd +- dtol */
  if ((retc = do_fread_asp(fpout, fpos1, &tjd_ex, &ipla, &iplb, &iasp, &dasp, &dorb, &tjd_pre, &tjd_post, serr)) == ERR)
    goto end_extract;
  while(tjd_ex < tjd + PERIOD_PRE_POST) {
      /*if (ipla == 8 && iplb == 11) {
        fprintf(stderr, "hallo\n");
      }*/
    if (ipla <= SE_CHIRON && iplb <= SE_CHIRON && ipla >= 0 && iplb >= 0 && iasp < 8) {
      /* complete aspect with tjd_pre and tjd_post */
      if (tjd_pre > 0 && tjd_post > 0) {
	/* within time range */
        if (tjd_post >= tjd - dtol && tjd_pre <= tjd + dtol) {
	  fprintf(stderr, "tjd_ex=%.1f, tjd_pre=%.1f, tjd_post=%.1f, %d, %d, %.0f %d\n", tjd_ex, tjd_pre, tjd_post, ipla, iplb, dasp, iasp);
        }
      /* aspect between nodes and aspides (no tjd_pre and tjd_post) */
      } else if (is_node_apsis(ipla) && is_node_apsis(iplb)) {
	/* exactness is within time range */
        if (tjd_ex >= tjd - dtol && tjd_ex <= tjd + dtol) {
	  fprintf(stderr, "tjd_ex=%.1f, tjd_pre=%.1f, tjd_post=%.1f, %d, %d, %.0f %d\n", tjd_ex, tjd_pre, tjd_post, ipla, iplb, dasp, iasp);
	}
      /* aspect is in process already at the beginning of search period,
       * too far away from the required date range */
      } else if (tjd_pre == 0) {
        ;
      /* aspect begins after end of required date range */
      } else if (tjd + dtol < tjd_pre) {
        ;
      /* aspect with unknown end, possibly covering our required date range.
       * we search for its end */
      } else {
        AS_BOOL do_output = FALSE;
	if (tjd_ex > tjd - dtol) {
	  do_output = TRUE;
	  fprintf(stderr, "xxtjd_ex=%.1f, tjd_pre=%.1f, tjd_post=%.1f, %d, %d, %.0f %d\n", tjd_ex, tjd_pre, tjd_post, ipla, iplb, dasp, iasp);
	}
	fpos2 = fpos1;
	tjd_ex2 = tjd_ex;
	while(tjd_ex2 < tjd + PERIOD_PRE_POST && fpos2 < fposlast) {
	  fpos2 += SWEASP_DAT_RECLEN;
	  if ((retc = do_fread_asp(fpout, fpos2, &tjd_ex2, &ipla2, &iplb2, &iasp2, &dasp, &dorb2, &tjd_pre2, &tjd_post2, serr)) == ERR)
	    goto end_extract;
	  if (ipla == ipla2 && iplb == iplb2 && iasp == iasp2) {
	    fprintf(stderr, "yytjd_ex=%.1f, tjd_pre=%.1f, tjd_post=%.1f, %d, %d, %.0f\n", tjd_ex2, tjd_pre2, tjd_post2, ipla2, iplb2, dasp);
	    fprintf(stderr, "yytjd_ex=%.1f, tjd_pre=%.1f, tjd_post=%.1f, %d, %d, %.0f %d\n", tjd_ex2, tjd_pre2, tjd_post2, ipla2, iplb2, dasp, iasp);
	    /* if previous phases had no exactness, but the current one has
	     * or is exacter, take over its time of exactness */
	    if (fabs(dorb2) < fabs(dorb)) {
	      dorb = dorb2;
	      tjd_ex = tjd_ex2;
	    }
	    /* if there are several exactnesses, select the one closest
	     * to the required date */
	    if (dorb2 == 0 && fabs(tjd_ex2 - tjd) < fabs(tjd_ex - tjd)) {
	      tjd_ex = tjd_ex2;
	      dorb = 0;
	    }
	    if (tjd_post2 > tjd - dtol) {
	      do_output = TRUE;
	      tjd_post = tjd_post2;
	    }
	    if (tjd_post2 > 0) /* there is no future exactness after that */
	      break;
	  }
	}
	if (do_output)
	  fprintf(stderr, "xxtjd_ex=%.1f, tjd_pre=%.1f, tjd_post=%.1f, %d, %d, %.0f %d\n", tjd_ex, tjd_pre, tjd_post, ipla, iplb, dasp, iasp);
      }

    }
    fpos1 += SWEASP_DAT_RECLEN;
    if ((retc = do_fread_asp(fpout, fpos1, &tjd_ex, &ipla, &iplb, &iasp, &dasp, &dorb, &tjd_pre, &tjd_post, serr)) == ERR)
      goto end_extract;
  }
end_extract:
  fclose(fpout);
  if (retc == ERR)
    fprintf(stderr, serr);
  return retc;
}

#if 0
/* Search for ingresses and transits
 */
int32 get_all_transits(int32 iflag, double tjdut_nat, double tjd0, double tjde, double tstep, char *splan, char *splat, double *dpos, char *sasp, EVENT *pev, char *serr)
{
  int32 ipl, ipla, iplb, ipli, iplia, iplib;
  char *sp, *spa, *spb;
  double t, tt0, tret, tret2, dang = 0, dorb = 0, x[6], x1[30], x2[30];
  double x1d[30], x2d[30];
  double d1d, d2d;
  double xta1, xta2, xtb1, xtb2, dt, d1, d2;
  int jday, jmon, jyear; 
  int32 retflag = 0;
  double jut;
  char saspi[30];
  double dasp[30];
  int iasp;
  int nasp = get_aspect_angles(sasp, saspi, dasp, serr);
  for (t = tjd0; t < tjde; t += tstep) {
    for (sp = splan, ipli = 0; *sp != '\0'; sp++, ipli++) {
      ipl = letter_to_ipl((int) *sp);
      if (t == tjd0) {
	if (swe_calc(t, ipl, iflag|SEFLG_SPEED, x, serr) == ERR)
	  return ERR;
	x1[ipli] = x[0];
	x1d[ipli] = x[0] + tstep / 10.0 * x[3];
      } else {
        x1[ipli] = x2[ipli];
	x1d[ipli] = x2d[ipli];
      }
      if (swe_calc(t + tstep, ipl, iflag|SEFLG_SPEED, x, serr) == ERR)
	return ERR;
      x2[ipli] = x[0];
      x2d[ipli] = x[0] + tstep / 10.0 * x[3];
    }
    for (spa = splan, iplia = 0; *spa != '\0'; spa++, iplia++) {
      ipla = letter_to_ipl((int) *spa);
      for (spb = splat, iplib = 0; *spb != '\0'; spb++, iplib++) {
	iplb = letter_to_ipl((int) *spb);
	for (iasp = 0; iasp < nasp; iasp++) {
	  dang = dasp[iasp];
	  xta1 = x1[iplia];
	  xta2 = x2[iplia];
	  xtb = dpos[iplib];
	  tt0 = t;
	  dt = tstep;
	  d1 = swe_degnorm(xta1 - xtb - dang);
	  if (d1 > 180) d1 -= 360;
	  d2 = swe_degnorm(xta2 - xtb - dang);
	  if (d2 > 180) d2 -= 360;
	  if (ipla == SE_MOON) {
	    if (fabs(d1) >  20)
	      continue;
	  } else if (fabs(d1) > NEAR_CROSSING_ORB + 3) {
	    continue;
	  }
	  d1d = swe_degnorm(x1d[iplia] - xtb - dang);
	  if (d1d > 180) d1d -= 360;
	  d2d = swe_degnorm(x2d[iplia] - xtb - dang);
	  if (d2d > 180) d2d -= 360;
	  /* 
	   * crossing found 
	   * find t of exact aspect
	   * if exactness happens twice within step width, the aspect is
	   * lost
	   */
	  if (d1 * d2 < 0) {
	    /*
	     * step width 1 day:     1min20sec/100 yr (1 or 2 asp lost per cty)
	     * step width 0.1 day:   1min55sec/100 years
	     * step width 0.01 day:  9min32sec/100 years 
	     * (calculations with SEFLG_NONUT)
	     * Still, we use 1-day step width. The lost aspects will
	     * be found in the "else".
	     */
	    if ((retflag = get_crossing_bin_search(dt, tt0, dang, xta1, xta2, xtb, xtb, &tret, ipla, iplb, iflag, TRUE, serr)) == ERR)
	      return ERR;
	    swe_revjul(tret, 1, &jyear, &jmon, &jday, &jut);
	    printf("%d%02d%02d %.3f: %c - %c %d\n", jyear, jmon, jday, jut, *spa, *spb, (int) dang);
	  /* 
	   * - near crossing occurs (t of smallest orb is found)
	   * - or exact aspect occurs twice within step width
	   *   (was lost by "if")
	   */
	  } else if (fabs(d1) < NEAR_CROSSING_ORB || fabs(d2) < NEAR_CROSSING_ORB) {
	    /* printf("d1=%f, d1d=%f, d2=%f, d2d=%f\n", d1, d1d, d2, d2d);*/
	    if (d1 > 0 && d2 > 0) {
	      if (d1 > d2 && d2 > d2d) continue;
	      if (d1 < d2 && d1 < d1d) continue;
	    } else {
	      if (d1 > d2 && d1 > d1d) continue;
	      if (d1 < d2 && d2 < d2d) continue;
	    }
	    if ((retflag = get_near_crossing_bin_search(dt, tt0, dang, xta1, xta2, xtb1, xtb2, &tret, &tret2, &dorb, ipla, iplb, iflag, serr)) == ERR)
	      return ERR;
	    if (retflag == -2)
	      continue;
	    swe_revjul(tret, 1, &jyear, &jmon, &jday, &jut);
	    printf("%d%02d%02d %.3f: %c - %c %d orb=%f\n", jyear, jmon, jday, jut, *spa, *spb, (int) dang, dorb);
	    if (tret2 != 0) {
	      swe_revjul(tret2, 1, &jyear, &jmon, &jday, &jut);
	      printf("%d%02d%02d %.3f: %c - %c %d orb=%f\n", jyear, jmon, jday, jut, *spa, *spb, (int) dang, dorb);
	    }
	  }
	}
      }
    }
  }
	//if (get_crossings(iflag, t, tstep, ipl1, ipl2, x1, x2, sasp, pev, serr) == ERR)
  return OK;
}
#endif

int32 calc_all_crossings(
              int32 iflag,    /* swiss ephemeris flags */
              int32 itype,    /* type of calculation:
                               * 0 = mundane aspects, 
                               * 1 = transits,
                               * 2 = ingresses */
              double tjd0,    /* start time */
              double tjde,    /* end time */
              double tstep,   /* time step width */
              char *splan,    /* moving planets' string */
              char *sasp,     /* aspects string */
              int32 npos,     /* with ityp=1, number of transit positions */
              double *dpos,   /* transit/ingress positions */
              EVENT *pev,  /* struct for output */
              char *serr      /* error string */
             ) 
{
  switch (itype) {
  case 0:
    if (tstep == 0)
      tstep = 1;
    if (calc_mundane_aspects(SEFLG_SWIEPH|SEFLG_NONUT, tjd0, tjde, tstep, splan, sasp, pev, serr) == ERR) 
      return ERR;
    break;
  }
  return OK;
}

static char get_casp(double dasp) 
{
  if (dasp == 0) return ((char) 1); 
  if (dasp == 180) return ((char) 2); 
  if (dasp == 90) return ((char) 3); 
  if (dasp == 270) return ((char) 3); 
  if (dasp == 120) return ((char) 4); 
  if (dasp == 240) return ((char) 4); 
  if (dasp == 60) return ((char) 5); 
  if (dasp == 300) return ((char) 5); 
  return ((char) 0);
}

static int32 get_prev_lunasp(double t0, int32 ipl, int32 iflag, double *tret, double *dasp, int32 *isign, char *serr)
{
  double xx[6], xm[6], dang, dx, t, mspeed;
  int nsign;
  if (swe_calc(t0, SE_MOON, iflag, xm, serr) == ERR) 
    return ERR;
  if (swe_calc(t0, ipl, iflag, xx, serr) == ERR) 
    return ERR;
  nsign = 0;
  dx = swe_degnorm(xm[0] - xx[0]);
  nsign = (int) (dx / 30);
  /* ignore semisextiles and inconjuncts */
  if (nsign == 1 || nsign == 5 || nsign == 7 || nsign == 11)
    nsign--;
  dang = nsign * 30;
  dx -= dang;
  /* lunar speed roughly */
  mspeed = xm[3] - xx[3];
  if (iflag & SEFLG_TOPOCTR)
    mspeed = 13 - xx[3];
  t = t0 - dx / mspeed;
  while (fabs(dx) > 1e-5) {
    if (swe_calc(t, SE_MOON, iflag, xm, serr) == ERR) 
      return ERR;
    if (swe_calc(t, ipl, iflag, xx, serr) == ERR) 
      return ERR;
    dx = swe_degnorm(xm[0] - xx[0] - dang);
    if (dx > 180)
      dx -= 360;
    t -= dx / mspeed;
  }
  *dasp = (double) ((int) (dang + 0.5));
  *tret = t;
  *isign = (int) (xm[0] / 30);
  return OK;
}

static int32 get_sign_ingress_direct_body(double tet0, int32 ipl, int32 iflag, int32 backward, double *tret, int32 *isign, char *serr)
{
  double xx[6], xingr, dx, mspeed, t;
  iflag |= SEFLG_SPEED;
  if (swe_calc(tet0, ipl, iflag, xx, serr) == ERR)
    return ERR;
  *isign = (int) (xx[0] / 30); 
/*  if (!backward) {*/
    if (ipl != SE_MEAN_NODE) 
      (*isign) ++;
    if (*isign == 12)
      *isign = 0;
    if (*isign == -1)
      * isign = 11;
/*  }*/
  xingr = *isign * 30;
  dx = swe_difdeg2n(xingr, xx[0]);
  t = tet0;
  /* body's speed roughly */
  mspeed = xx[3];
  if (ipl == SE_MOON && (iflag & SEFLG_TOPOCTR))
    mspeed = 13;
  while(fabs(dx) > 1e-6) {
    t += dx / mspeed;
    if (swe_calc(t, ipl, iflag, xx, serr) == ERR)
      return ERR;
    dx = swe_difdeg2n(xingr, xx[0]);
  }
  *tret = t;
  return OK;
}

#if 0
static int get_next_ingress(double t, int32 ipl, int32 iflag, INGRESS *ding, char *serr)
{
  double tingr;
  int isign;
  int direction = 1;
  int ino = 1;
  int retc;
  switch (ipl) {
  case SE_MOON: case SE_SUN: case SE_MEAN_NODE: case SE_MEAN_APOG:
    if ((retc = get_sign_ingress_direct_body(t, ipl, iflag, 0, &tingr, &isign, serr)) == ERR)
      return retc;
    if (ipl == SE_MEAN_NODE)
      direction = -1;
    break;
  default:
    /* hier weitermachen */
    if ((retc = get_sign_ingress_planet(t, ipl, iflag, 0, &tingr, &isign, serr)) == ERR)
      return retc;
    break;
  }
  ding->ipl = ipl;
  ding->tingr = tingr;
  ding->isign = isign;
  ding->direction = direction;
  ding->ino = ino;
  return OK; 
}
#endif

/*
Lunar void-of-course phases
===========================
 
Definition:
Last lunar aspect (COQTS) to a planet (0-9) before a lunar ingress.
 
Problem:
It happens that a the moon does not form ANY aspects with planets while
crossing a sign. 
The Rosicrucian Ephemeris lists the following case for March 1965:
Last aspect    Lunar ingress
Day    h  m    Day    h  m
 4     1:35     4    18:45
 4     1:35     7     1:50
 
Method 1 (-v1):
A VOC phase can start before the last sign ingress, and it can last longer
than a sign transit. There are less VOC phases than ingresses. The output 
contains a line per VOC phase, but not a line per ingress.
For a given time t:
- calculate the next lunar ingress (ti_next)
- calculate the next lunar ingress after ti_next (ti_nextnext)
- set tvoc_start = 0, tvoc_end = ti_nextnext
- calculate the last aspect before tvoc_end (ta_last)
  as soon as an aspect later than ti_next is found,
  set tvoc_end = ti_next; break the search and repeat it
- set tvoc_start = ta_last
 
Method 2 (-v2): 
A VOC phase can start before the last sign ingress, and it can last longer
than a sign transit. But a line is printed for each ingress, and two 
ingresses can have the same start date for their VOC phases.
(Output is as in Ros. Eph., e.g. for 4-mar-1965) 
- calculate the next lunar ingress (ti_next)
- set tvoc_start = 0, tvoc_end = ti_next
- calculate the last aspect before tvoc_end (ta_last)
- set tvoc_start = ta_last
 
Method 3 (-v3, default): 
A VOC phase cannot start before the last sign ingress, i.e. a VOC phase
can start at a sign ingress and end at the next sign ingress.
the two VOC phases remain separate, a VOC phase may start at ingress
- same procedure as solution 2
but after that:
- calculate previous lunar ingress (ti_prev)
- if ta_last < ti_prev, then ta_last = ti_prev
*/
static int32 get_next_voc(double tet0, int32 iflag, int32 vocmethod, VOC *pvoc, char *serr) 
{
  int32 retc;
  double tet_asp, daspi, tvoc_end, tingr_save;
  double tret[2], dasp;
  int32 ipl, ipllast;
  int32 isign, isigni, isign_save;
  int32 isign_ingr;
  if (vocmethod == 0) vocmethod = 3;
  /* next ingress */
  if ((retc = get_sign_ingress_direct_body(tet0, SE_MOON, iflag, 0, &(tret[1]), &isign, serr)) == ERR)
    return retc;
  isign_save = isign;
  tingr_save = tret[1];
  tvoc_end = tret[1];
  /* overnext ingress */
  if (vocmethod == 1 && (retc = get_sign_ingress_direct_body(tret[1] + 1, SE_MOON, iflag, 0, &tvoc_end, &isign, serr)) == ERR)
    return retc;
  isign_ingr = isign;
  /* find the last aspect with a planet within this time range */
  tret[0] = 0;
repeat_loop:
  for (ipl = SE_SUN; ipl <= SE_PLUTO; ipl++) {
    if (ipl == SE_MOON)
      continue;
    if ((retc = get_prev_lunasp(tvoc_end, ipl, iflag, &tet_asp, &daspi, &isigni, serr)) == ERR)
      return retc;
    if (vocmethod == 1 && tet_asp > tret[1]) {
      tvoc_end = tret[1];
      isign_ingr--;
      if (isign_ingr < 0) isign_ingr = 11;
      goto repeat_loop;
    }
    if (tet_asp > tret[0]) {
      tret[0] = tet_asp;
      dasp = daspi;
      ipllast = ipl;
      isign = isigni;
    }
  }
  tret[1] = tvoc_end;
  if (vocmethod == 3) {
    double tingr_prev;
    /* get previous lunar ingress */
    if ((retc = get_sign_ingress_direct_body(tet0, SE_MOON, iflag, 1, &tingr_prev, &isign, serr)) == ERR)
      return retc;
    if (tret[0] < tingr_prev)
      tret[0] = tingr_prev;
  }
  pvoc->tvoc = tret[0];
  pvoc->tingr = tret[1];
  pvoc->casp = get_casp(dasp);
  pvoc->cpl = (char) ipllast;
  pvoc->isign_voc = isign;
  pvoc->isign_ingr = isign_ingr;
  if (isign_save == isign_ingr) {
    pvoc->tingr0 = 0;
    pvoc->isign_ingr0 = 0;
  } else {
    pvoc->tingr0 = tingr_save;
    pvoc->isign_ingr0 = isign_save;
  }
  return OK;
}

static int32 calc_all_voc(int32 iflag, double te, double tend, char *serr)
{
  double t, tnext;
  int jday, jmon, jyear, gregflag = SE_GREG_CAL;
  double jut;
/*  int32 ctyp = CTYP_VOC; */
  int32 retc;
  VOC dvoc;
  /* moon void of course */
  for (t = te; t < tend; t = tnext) {
    retc = get_next_voc(t, SEFLG_SPEED, 1, &dvoc, serr);
    if (retc == ERR) {
      printf("%s\n", serr);
      return ERR;
    }
    /* content of dvoc:
     * double tvoc;    : time of begin of voc phase 
     * double tingr;   : time of ingress that ends voc phase 
     * double tingr0;  : time of first ingress, if there are two ingresses
     *                   during voc phase 
     * char casp, cpl; : aspect and planet that marks begin of voc phase 
     * int isign_voc;  : sign in which moon begins voc phase 
     * int isign_ingr; : sign of ingress that ends voc phase 
     * int isign_ingr0;: sign of first ingress, if there are two ingresses
     *                   during voc phase 
     */
    swe_revjul(dvoc.tvoc, gregflag, &jyear, &jmon, &jday, &jut);
    printf("VOCBEG: %d.%d.%d, %f, %d -> ", jday, jmon, jyear, jut, dvoc.isign_voc);
    swe_revjul(dvoc.tingr, gregflag, &jyear, &jmon, &jday, &jut);
    printf("VOCEND: %d.%d.%d, %f, %d \n", jday, jmon, jyear, jut, dvoc.isign_ingr);
    if (dvoc.tingr0 != 0) {
      swe_revjul(dvoc.tingr0, gregflag, &jyear, &jmon, &jday, &jut);
      printf("                  -> VOCEND0: %d.%d.%d, %f, %d \n", jday, jmon, jyear, jut, dvoc.isign_ingr0);
    }
    tnext = dvoc.tingr + 0.1;
  }
  return OK;
}

static int letter_to_ipl(int letter)
{
  if (letter >= '0' && letter <= '9')
    return letter - '0' + SE_SUN;
  if (letter >= 'A' && letter <= 'I')
    return letter - 'A' + SE_MEAN_APOG;
  if (letter >= 'J' && letter <= 'Z')
    return letter - 'J' + SE_CUPIDO;
  switch (letter) {
  case 'm': return SE_MEAN_NODE;
  case 'c': return SE_INTP_APOG;
  case 'g': return SE_INTP_PERG;
  case 'n': 
  case 'o': return SE_ECL_NUT;
  case 't': return SE_TRUE_NODE;
  case 'f': return SE_FIXSTAR;
  case 'w': return SE_WALDEMATH;
  case 'e': /* swetest: a line of labels */
  case 'q': /* swetest: delta t */
  case 's': /* swetest: an asteroid, with number given in -xs[number] */
  case 'z': /* swetest: a fictitious body, number given in -xz[number] */
  case 'd': /* swetest: default (main) factors 0123456789mtABC */
  case 'p': /* swetest: main factors ('d') plus main asteroids DEFGHI */
  case 'h': /* swetest: fictitious factors JKLMNOPQRSTUVWXYZw */
  case 'a': /* swetest: all factors, like 'p'+'h' */
    return -1;
  }
  return -2;
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

#if 0
/*
 * a planetary phenomenon ist printed; 
 * date time is alway printed (in UT)
 * ingresses: s is "ingress" or "ingress retro" , only  date and 0/30 sign
 * if delon or dmag are HUGE, they are not printed.
 */
static void print_item(char *s, double teph, CSEC cspos, double delon, double dmag)
{
  char smag[10];
  int mout, dout, yout, min;
  double hout;
  AS_BOOL is_ingress;
  AS_BOOL gregflag = TRUE;
  int ing_deg = 0;
  char *jul = "";
  is_ingress =  (strncmp(s, "ingr", 4) == 0);
  if (teph < 2299160.5) {
    gregflag = FALSE;
    jul = "j";
  } 
  if (strstr(s, "ret") != NULL) 
    ing_deg = 30;
  /* compute UT and add 0.5 minutes for later rounding to minutes */
  if (ephemeris_time) 
    teph = teph + 0.5 / 1440; 
  else
    teph = teph - swe_deltat(teph) + 0.5 / 1440; 
  //revjul(teph, gregflag, &mout, &dout, &yout, &hout);
  swe_revjul(teph, gregflag, &yout, &mout, &dout, &hout);
  min = floor (hout * 60);
  if (dmag != HUGE) {
    sprintf(smag, "    %.1fm", dmag);
  } else {
    *smag = '\0';
  }
}

static int find_zero(double y00, double y11, double y2, double dx, 
			double *dxret, double *dxret2)
{
  double a, b, c, x1, x2;
  c = y11;
  b = (y2 - y00) / 2.0;
  a = (y2 + y00) / 2.0 - c;
  if (b * b - 4 * a * c < 0) 
    return ERR;
  x1 = (-b + sqrt(b * b - 4 * a * c)) / 2 / a;
  x2 = (-b - sqrt(b * b - 4 * a * c)) / 2 / a;
  if (fabs(x1) < 1) {
    *dxret = (x1 - 1) * dx;
    *dxret2 = (x2 - 1) * dx;
  } else {
    *dxret = (x2 - 1) * dx;
    *dxret2 = (x1 - 1) * dx;
  }
  return OK;
}

static int find_maximum(double y00, double y11, double y2, double dx, 
			double *dxret, double *yret)
{
  double a, b, c, x, y;
  c = y11;
  b = (y2 - y00) / 2.0;
  a = (y2 + y00) / 2.0 - c;
  x = -b / 2 / a;
  y = (4 * a * c - b * b) / 4 / a;
  *dxret = (x - 1) * dx;
  *yret = y;
  return OK;
}

static long init_next_step(double te, int ipl, long iflag, char *serr)
{
  int i;
  long iflgret;
  double x[6], x1[6], x2[6];
  double phase, rphel, phdeg;
  /* 
   * we have always 
   * - three positions and speeds for venus
   * - three positions and speeds for sun
   * - three elongations of venus (with elongation speed)
   * - three magnitudes
   */
  memcpy(xp0, xp1, 6 * sizeof(double));	/* planet */
  memcpy(xp1, xp2, 6 * sizeof(double));
  memcpy(xs0, xs1, 6 * sizeof(double));	/* sun */
  memcpy(xs1, xs2, 6 * sizeof(double));
  memcpy(xel0, xel1, 6 * sizeof(double));	/* elongation in longitude */
  memcpy(xel1, xel2, 6 * sizeof(double));
  memcpy(xang0, xang1, 6 * sizeof(double));/* ang. dist. from sun */
  memcpy(xang1, xang2, 6 * sizeof(double));
  memcpy(xma0, xma1, 6 * sizeof(double));	/* magnitude */
  memcpy(xma1, xma2, 6 * sizeof(double));
  iflgret = swe_calc(te, ipl, iflag, xp2, serr);
  if (iflgret < 0) 
    return iflgret;
  iflgret = swe_calc(te, SE_SUN, iflag, xs2, serr);
  if (iflgret < 0) 
    return iflgret;
  /* 
   * elongation in longitude
   */
  for (i = 0; i <= 5; i++)
    xel2[i] = xp2[i] - xs2[i];
  if (xel2[0] > M_PI)
    xel2[0] -= 2 * M_PI;
  if (xel2[0] < -M_PI)
    xel2[0] += 2 * M_PI;
  /* 
   * angular distance from sun 
   */
  swi_polcart(xp2, x1);
  swi_polcart(xs2, x2);
  for (i = 0; i <= 2; i++)
    x[i] = -x2[i] + x1[i];
	  /* 'apparent' hel. distance of planet*/
  rphel = sqrt(square_sum(x));
  xang2[0] = acos((xs2[2] * xs2[2] + xp2[2] * xp2[2] - rphel * rphel) /
				  2.0 / xs2[2] / xp2[2]);
  /* 
   * magnitude 
   */
  phase = acos((rphel * rphel + xp2[2] * xp2[2] - xs2[2] * xs2[2]) /
				  2.0 / rphel / xp2[2]); /* phase angle */
  phdeg = phase * RADTODEG;
  xma2[0] = 5 * log10(rphel * xp2[2])
	    + mag_elem[ipl][1] * phdeg /100.0
	    + mag_elem[ipl][2] * phdeg * phdeg / 10000.0
	    + mag_elem[ipl][3] * phdeg * phdeg * phdeg / 1000000.0
	    + mag_elem[ipl][0];
  return 0;
}

static long do_conj_sun(double te, int ipl, long iflag, char *serr) 
{
  int i, j, k;
  double dt, dt1, dt2, t2, t3;
  double x[6], x0[6], x1[6], x2[6], xp[6], xs[6];
  double rphel, sunrad, xel;
  int iflgret;
  if ((xel1[0] < 0 && xel2[0] >= 0) || (xel1[0] > 0 && xel2[0] <= 0)) {
    find_zero(xel0[0], xel1[0], xel2[0], tstep, &dt, &dt2);
    t2 = te + dt;
    if ((iflgret = swe_calc(t2, ipl, iflag, x, serr)) == ERR)
      return ERR;
    if ((iflgret = swe_calc(t2, SE_SUN, iflag, xs, serr)) == ERR)
      return ERR;
    if (ipl != SE_VENUS && ipl != SE_MERCURY) {
      if (fabs(xel1[0]) > M_PI / 2)
	print_item("opposition", t2, x[0] * RADTOCS, HUGE, HUGE);
      else
	print_item("conjunction", t2, x[0] * RADTOCS, HUGE, HUGE);
    } else {
      if (x[3] > 0)
	print_item("superior conj", t2, x[0] * RADTOCS, HUGE, HUGE);
      else
	print_item("inferior conj", t2, x[0] * RADTOCS, HUGE, HUGE);
    }
    /* occultation or transit */
    for (j = 0, dt1 = tstep; j <= 3; j++, dt1 /= 3) {
      for (k = 0; k <= 2; k++) {
	t3 = t2 + (k-1) * dt1;
	iflgret = swe_calc(t3, ipl, iflag, xp, serr);
	iflgret = swe_calc(t3, SE_SUN, iflag, xs, serr);
	swi_polcart(xp, x1);
	swi_polcart(xs, x2);
	for (i = 0; i <= 2; i++)
	  x[i] = -x2[i] + x1[i];
	rphel = sqrt(square_sum(x));
	x0[k] = acos((xs[2] * xs[2] + xp[2] * xp[2] - rphel * rphel) /
				      2.0 / xs[2] / xp[2]);
	}
      find_maximum(x0[0], x0[1], x0[2], dt1, &dt, &xel);
      t2 = t2 + dt1 + dt;
    }
    if ((iflgret = swe_calc(t2, ipl, iflag, x, serr)) == ERR)
      return ERR;
    if ((iflgret = swe_calc(t2, SE_SUN, iflag, xs, serr)) == ERR)
      return ERR;
#if 0 	/* minimum elongation is not printed, but needed if occultation */
      print_item("  minimum elong", t2, x[0] * RADTOCS, xel, HUGE);
#endif
    switch(ipl) {
      case SE_VENUS: 
	sunrad = SUN_RADIUS / xs[2] + VENUS_RADIUS / x[2];
	break;
      case SE_MERCURY:
	sunrad = SUN_RADIUS / xs[2] + MERCURY_RADIUS / x[2];
	break;
      case SE_MARS:
      default:
	sunrad = SUN_RADIUS / xs[2] + MARS_RADIUS / x[2];
	break;
    }
    if (sunrad > fabs(xel)) {
      if (x[3] > 0 || ipl > SE_VENUS)
	strcpy(sout, "  occultation begin");
      else
	strcpy(sout, "  transit begin");
      dt = sqrt(sunrad * sunrad - xel * xel);
      /*dt = acos(cos(sunrad) / cos(xel)); is not better */
      dt /= sqrt((x[3]-xs[3]) * (x[3]-xs[3]) + (x[4]-xs[4]) * (x[4]-xs[4]));
      if ((iflgret = swe_calc(t2-dt, ipl, iflag, x, serr)) == ERR)
	return ERR;
      print_item(sout, t2 - dt, x[0] * RADTOCS, HUGE, HUGE);
      if (x[3] > 0 || ipl > SE_VENUS)
	strcpy(sout, "  occultation end");
      else
	strcpy(sout, "  transit end");
      if ((iflgret = swe_calc(t2+dt, ipl, iflag, x, serr)) == ERR)
	return ERR;
      print_item(sout, t2 + dt, x[0] * RADTOCS, HUGE, HUGE);
    }
  }
  return OK;
}

static long do_visibility(double te, int ipl, long iflag, char *serr)
{
  double x[6];
  double t2, dt, dt2;
  long iflgret;
  *sout = '\0';
  if (xang1[0] * RADTODEG > 10 && xang2[0] * RADTODEG < 10) {
    if (xel1[0] > 0) 
      strcpy(sout, "evening set");
    else
      strcpy(sout, "morning set");
    x[0] = xang0[0] - 10 * DEGTORAD;
    x[1] = xang1[0] - 10 * DEGTORAD;
    x[2] = xang2[0] - 10 * DEGTORAD;
  }
  if (xang1[0] * RADTODEG < 10 && xang2[0] * RADTODEG > 10) {
    if (xel1[0] > 0) 
      strcpy(sout, "evening rise");
    else
      strcpy(sout, "morning rise");
    x[0] = xang0[0] - 10 * DEGTORAD;
    x[1] = xang1[0] - 10 * DEGTORAD;
    x[2] = xang2[0] - 10 * DEGTORAD;
  }
  if (*sout != '\0') {
    find_zero(x[0], x[1], x[2], tstep, &dt, &dt2);
    t2 = te + dt;
    if ((iflgret = swe_calc(t2, ipl, iflag, x, serr)) == ERR)
      return ERR;
    print_item(sout, t2, x[0] * RADTOCS, HUGE, HUGE);
  }
  return OK;
}

static long do_max_elong(double te, int ipl, long iflag, char *serr)
{
  int i, j, k;
  long iflgret;
  double t2, t3, dt, dt1, xel;
  double xp[6], xs[6], x0[6], x1[6], x2[6], x[6];
  double rphel;
  if ((fabs(xang0[0]) < fabs(xang1[0]) && fabs(xang2[0]) < fabs(xang1[0]))
    || (fabs(xang0[0]) > fabs(xang1[0]) && fabs(xang2[0]) > fabs(xang1[0]))) {
    find_maximum(xang0[0], xang1[0], xang2[0], tstep, &dt, &xel);
    t2 = te + dt;
    for (j = 0, dt1 = tstep; j <= 3; j++, dt1 /= 3) {
      for (k = 0; k <= 2; k++) {
	t3 = t2 + (k-1) * dt1;
	if ((iflgret = swe_calc(t3, ipl, iflag, xp, serr)) == ERR)
	  return ERR;
	if ((iflgret = swe_calc(t3, SE_SUN, iflag, xs, serr)) == ERR)
	  return ERR;
	swi_polcart(xp, x1);
	swi_polcart(xs, x2);
	for (i = 0; i <= 2; i++)
	  x[i] = -x2[i] + x1[i];
	rphel = sqrt(square_sum(x));
	x0[k] = acos((xs[2] * xs[2] + xp[2] * xp[2] - rphel * rphel) /
				      2.0 / xs[2] / xp[2]);
      }
      find_maximum(x0[0], x0[1], x0[2], dt1, &dt, &xel);
      t2 = t2 + dt1 + dt;
    }
    if ((iflgret = swe_calc(t2, ipl, iflag, x, serr)) == ERR)
      return ERR;
    if (ipl > SE_VENUS) {
      if (xel < M_PI / 2)
	strcpy(sout, "minimum elong.");
      else
	strcpy(sout, "maximum elong.");
    } else if (xel1[0] > 12 * DEGTORAD)
      strcpy(sout, "evening max el");
    else if (xel1[0] < -12 * DEGTORAD)
      strcpy(sout, "morning max el");
    else
      strcpy(sout, "minimum elong.");
    print_item(sout, t2, x[0] * RADTOCS, xel, HUGE);
  }
  return OK;
}

static long do_station(double te, int ipl, long iflag, char *serr)
{
  int i, j, k;
  long iflgret;
  double x[6], x0[6], x1[6], xp[6];
  double t2, t3, dt, dt1, dx, xel;
#if 0
	  /* retrograde or direct, zero speed */
	  if (xp1[3] < 0 && xp2[3] >= 0 || xp1[3] > 0 && xp2[3] <= 0 ) {
	    find_zero(xp0[3], xp1[3], xp2[3], tstep, &dt, &dt2);
            t2 = te + dt;
	    if((iflgret = swe_calc(t2, ipl, iflag, x, serr)) == ERR)
	      return ERR;
	    if (xp2[3] < 0)
	      strcpy(sout, "retrograde (sp)");
	    else
	      strcpy(sout, "direct (sp)");
	    print_item(sout, t2, x[0] * RADTOCS, HUGE, HUGE);
          }
#endif
	  /* retrograde or direct, maximum position */
	  if ((xp1[3] < 0 && xp2[3] >= 0) || (xp1[3] > 0 && xp2[3] <= 0)) {
	    x0[0] = xp0[0];
	    x0[1] = xp1[0];
	    x0[2] = xp2[0];
	    for (i = 1; i <= 2; i++) {
	      dx = x0[i] - x0[0];
	      if (dx > M_PI)
		x0[i] -= M_PI;
	      if (dx < -M_PI)
		x0[i] += M_PI;
	    }
	    find_maximum(x0[0], x0[1], x0[2], tstep, &dt, &xel);
            t2 = te + dt;
	    for (j = 0, dt1 = tstep; j <= 3; j++, dt1 /= 3) {
              for (k = 0; k <= 2; k++) {
		t3 = t2 + (k-1) * dt1;
		if((iflgret = swe_calc(t3, ipl, iflag, xp, serr)) == ERR)
		  return ERR;
		swi_polcart(xp, x1);
		x0[k] = xp[0];
	      }
	      for (i = 1; i <= 2; i++) {
		dx = x0[i] - x0[0];
		if (dx > M_PI)
		  x0[i] -= M_PI;
		if (dx < -M_PI)
		  x0[i] += M_PI;
	      }
	      find_maximum(x0[0], x0[1], x0[2], dt1, &dt, &xel);
	      t2 = t2 + dt1 + dt;
            }
	    if((iflgret = swe_calc(t2, ipl, iflag, x, serr)) == ERR)
	      return ERR;
	    if (xp2[3] < 0)
	      strcpy(sout, "retrograde");
	    else
	      strcpy(sout, "direct");
	    print_item(sout, t2, x[0] * RADTOCS, HUGE, HUGE);
          }
  return OK;
}

static long do_brillancy(double te, int ipl, long iflag, char *serr)
{
  int i, j, k;
  long iflgret;
  double t2, t3, dt, dt1, xma, rphel, phdeg, phase;
  double x[6], xp[6], xs[6], x0[6], x1[6], x2[6];
  if (ipl <= SE_MARS 
      && xma0[0] > xma1[0] 
      && xma2[0] > xma1[0] 
      && xang1[0] > 10*DEGTORAD) {
    find_maximum(xma0[0], xma1[0], xma2[0], tstep, &dt, &xma);
    t2 = te + dt;
    for (j = 0, dt1 = tstep; j <= 3; j++, dt1 /= 3) {
      for (k = 0; k <= 2; k++) {
	t3 = t2 + (k-1) * dt1;
	if ((iflgret = swe_calc(t3, ipl, iflag, xp, serr)) == ERR)
	  return ERR;
	if ((iflgret = swe_calc(t3, SE_SUN, iflag, xs, serr)) == ERR)
	  return ERR;
	swi_polcart(xp, x1);
	swi_polcart(xs, x2);
	for (i = 0; i <= 2; i++)
	  x[i] = -x2[i] + x1[i];
	rphel = sqrt(square_sum(x));
	phase = acos((rphel * rphel + xp[2] * xp[2] 
			- xs[2] * xs[2]) / 2.0 / rphel / xp[2]);
	phdeg = phase * RADTODEG;
        x0[k] = 5 * log10(rphel * xp[2])
		+ mag_elem[ipl][1] * phdeg /100.0
		+ mag_elem[ipl][2] * phdeg * phdeg / 10000.0
		+ mag_elem[ipl][3] * phdeg * phdeg * phdeg / 1000000.0
		+ mag_elem[ipl][0];
      }
      find_maximum(x0[0], x0[1], x0[2], dt1, &dt, &xma);
      t2 = t2 + dt1 + dt;
    }
    if ((iflgret = swe_calc(t2, ipl, iflag, x, serr)) == ERR)
      return ERR;
    print_item("greatest brilliancy", t2, x[0] * RADTOCS, HUGE, xma);
  }
  return OK;
}
#endif
