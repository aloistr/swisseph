/* SWISSEPH
 * 
 * IMPORTANT NOTICE: swevents.c is not a supported part of Swiss Ephemeris.
 * If you find bugs and short comings, please fix the source code and submit the fixes
 * on the Swiss Ephemeris mailing list.

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

static char *info1 = "\n\
  Swevents computes planetary phenomena\n\
  for a given start date and a time range.\n\n\
  IMPORTANT NOTICE: swevents.c is not a supported part of Swiss Ephemeris.\n\
  If you find bugs and short comings, please fix the source code and submit the fixes\n\
  on the Swiss Ephemeris mailing list.\n\n\
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
	swevents -p3 -bj2436723.5 -n1000 -s1 -ejpl \n\
	   for -p2 (mercury), use -s0.3 \n\
	\n\
	-cl	do not print command line at bottom of 1st page\n\
	-p    	planet to be computed.\n\
		See the letter coding below.\n\
	-nN	search events for N consecutive days; if no -n option\n\
		is given, the default is 366 (one year).\n\
	-sN	timestep N days, default 1. This option is only meaningful\n\
		when combined with option -n.\n\
	-edirPATH change the directory of the ephemeris files \n\n\
	-doingr	report sign ingresses\n\
	-doconj	report inferior and superior conjunctions with Sun\n\
	-dobrill	report moments of greatest brilliance\n\
	-dorise	report morning set or evening rise\n\
	-doelong	report maximum elongation from Sun, for planets 1,2,3 only\n\
	-doretro	report stations\n\
	-doaps	report minimal and maximal distance from Earth\n\
	-donode	report when on ascending or descending node\n\n\
	-doall  equivalent to all -do.. options above combined\n\
	        If used, output starts only at next cycle begin.\n\n\
	-doing45	crossings over 15 tau, Leo, Sco, Aqu\n\
	-dolphase	report lunar phases (use with -p1)\n\
	-doasp	report aspects between planets (-p option is ignored)\n\
	-dovoc	report Moon void of course periods (-p option is ignored)\n\
	-noingr  no ingresses\n\
	-motab  special format Moon ingres table\n\
	-mojap  special format Moon phases\n\
	-bDATE	use this begin date instead of asking; use -b1.1.1992 if\n\
		the begin date string contains blanks; use format -bj2400000.5\n\
		to express the date as absolute Julian day number.\n\
		Note: the date format is day month year (European style).\n\
	 -eswe  swiss ephemeris\n\
	 -ejpl  jpl ephemeris (DE431), or with ephemeris file name\n\
	 	-ejplde200.eph\n\
	 -emos  moshier ephemeris\n\
	 -true	true positions\n\
	 -noaberr	no aberration\n\
	 -nodefl	no gravitational light deflection\n\
	 -noprec	no precession (i.e. J2000 positions)\n\
	 -nonut		no nutation \n\
	 -dgap		use gap within date\n\
	 -zlong		use long sign names\n\
	 -znam3		use 3-letter sign names\n\
	 -monnum	use month numbers instead of names\n\
	 -gmtoff X	use X hours gmt offset (+ for east)\n\
	 -tzoneTIMEZONE output date and time in timezone (hours east)\n\
	 -transitstderr lists transits of Venus or Mercury as c style data to stderr \n\
	 -jd	show also jd in output \n\
	 -ep		  use extended precision in output\n\n\
	Options only for use by Astrodienst:\n\
	-mscreen output on screen\n\
	-mps postscript [default: HP Laserjet]\n\
	-cycol.. number cycles per column\n\
\n\
	-?	display this info\n\
	-h	display this info\n\
\n";
static char *info2 = "Planet selection (only one possible):\n\
	0 Sun (character zero)\n\
	1 Moon (character 1)\n\
	2 Mercury\n\
	3 Venus\n\
	4 Mars\n\
	5 Jupiter\n\
	6 Saturn\n\
	7 Uranus\n\
	8 Neptune\n\
	9 Pluto\n\
	10 mean lunar node\n\
	11 true lunar node\n\
	12 mean lunar apogee\n\
	13 true lunar apogee\n\
	14 Earth\n\
	15 Chiron\n\
	16 Pholus\n\
	17 Ceres\n\
	18 Pallas\n\
	19 Juno\n\
	20 Vesta\n\
	21 interpolated lunar apogee\n\
	22 interpolated lunar perigee\n\
	h00 .. h18 fictitious factors, see swephexp.h\n\
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

#define PRINTMOD 0

#if PRINTMOD
#  include "ourdef.h"
#  include "astrolib.h"
#  include "printmod.h"
#else
#  define PMODEL_SCREEN 10
#endif
#include "swephexp.h"
#include "swephlib.h"
#include "sweph.h"
#include "swevents.h"
#include <time.h>

#define MAX_LINES       67
#define MAX_COLS        2
double xcol[4] = {20.0, 110.0};
double xcol4[4] = {12.0, 60, 108.0, 156.0};
double xdate = 25;      /* position of date */
double xpos = 52;       /* position of date */
double ytop = 24;
double line_space = 4;
double gmtoff = 0;
int pmodel;
int ncycol = 0;
int do_flag = 0;
int prev_yout = -999999;
int max_cols = MAX_COLS;

#define J2000           2451545.0  /* 2000 January 1.5 */
#define square_sum(x)   (x[0]*x[0]+x[1]*x[1]+x[2]*x[2])
#define SEFLG_EPHMASK   (SEFLG_JPLEPH|SEFLG_SWIEPH|SEFLG_MOSEPH)
#define SUN_RADIUS      (959.63 / 3600 * DEGTORAD)  /*  Meeus germ. p 391 */
#define VENUS_RADIUS	(8.34 / 3600 * DEGTORAD) /* AA96 E43 */
#define MERCURY_RADIUS	(3.36 / 3600 * DEGTORAD) /* AA96 E43 */
#define MARS_RADIUS	(4.68 / 3600 * DEGTORAD) /* AA96 E43 */
#define UNUSED(expr) do { (void)(expr); } while (0)

#define BIT_ROUND_NO	0
#define BIT_ROUND_SEC	1
#define BIT_ROUND_MIN	2
#define BIT_ZODIAC	4
#define BIT_LZEROES     8

#define STRLEN_OUT_TEXT 20

static char *zod_nam[] = {"AR", "TA", "GE", "CN", "LE", "VI",
			  "LI", "SC", "SA", "CP", "AQ", "PI"};
static char *zod_nam3[] = {"ARI", "TAU", "GEM", "CAN", "LEO", "VIR",
			  "LIB", "SCO", "SAG", "CAP", "AQU", "PIS"};
static char *zod_nam_long[] = {"aries", "taurus", "gemini",
			  "cancer", "leo", "virgo",
			  "libra", "scorpio", "sagittarius",
			  "capricorn", "aquarius", "pisces"};
static char *month_nam[] = {"", "Jan", "Feb", "Mar", "Apr", "May", "Jun",
			    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};


static char motab[13][31][10];	/* table for moon ingresses */

char *planet_name = "-";
char spnam[256];
AS_BOOL ephemeris_time = FALSE;
AS_BOOL do_round_min = FALSE;
AS_BOOL do_motab = FALSE;
AS_BOOL do_mojap = FALSE;
AS_BOOL date_gap = FALSE;
AS_BOOL show_jd = FALSE;
AS_BOOL print_cl = TRUE;
AS_BOOL output_extra_prec = FALSE;
AS_BOOL get_data_of_day = FALSE;
AS_BOOL transits_to_stderr = FALSE;
double	phase_mod = 90;
char **znam = zod_nam;
char *gap = " ";
char sdatefrom[AS_MAXCH], sdateto[AS_MAXCH], syear0[80];
char sdate[AS_MAXCH];
AS_BOOL gregflag = TRUE;
int ipl;
double tzone = 0;
int whicheph = SEFLG_JPLEPH; 
char cmdline[4 * AS_MAXCH];
char *stit = "Planetary Phenomena";

#if MSDOS
# define   EPHEPATH "C:\\C600\\SOURCE\\SWEPH\\"
#else
# define   EPHEPATH "/home/ephe"
#endif

static char *dms(double x, int iflag);
static int find_zero(double y00, double y11, double y2, double dx, 
			double *dxret, double *dxret2);
static int find_maximum(double y00, double y11, double y2, double dx, 
			double *dxret, double *yret);
static void print_item(char *s, double t, double x, double elo, double mag);
static int print_motab(void);
static char *set_planet_name(int ipl);

// from old swevents.c
static char *hms(double x, int32 iflag);
static int32 get_next_voc(double tet0, int32 iflag, int32 vocmethod, VOC *pvoc, char *serr);
static int32 calc_all_crossings(
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
static int32 extract_data_of_day(int32 do_flag, double te, double dtol, char *splan, char *sasp, EVENT *pev, char *serr);
static int letter_to_ipl(int letter);




#define DO_CONJ		1
#define DO_RISE		2
#define DO_ELONG	4
#define DO_RETRO	8
#define DO_BRILL	16
#define DO_APS		32
#define DO_NODE		64
#define DO_LAT		128	// obsolete
#define DO_INGR		256
#define DO_LPHASE	512
#define DO_INGR45	1024	// add ingresses over 45,135,215,305° (mid of fixed signs)
#define DO_ASPECTS    	2048
#define DO_VOC    	4096

#define DO_ALL (DO_CONJ|DO_RISE|DO_ELONG|DO_RETRO|DO_BRILL|DO_APS|DO_NODE|DO_INGR)

AS_BOOL sign_change(double x0, double x1) 
{
  if (x0 < 0 && x1 >= 0) return TRUE;
  if (x0 >= 0 && x1 < 0) return TRUE;
  return FALSE;
}


int main(int argc, char *argv[])
{
  AS_BOOL is_opposition;
  char serr[256];
  char sout[AS_MAXCH], s[AS_MAXCH], saves[AS_MAXCH]; 
  char *sp, *spsave;
  char *spno;
  int i, j, k, n, izod;
  int jmon, jday, jyear;
  double elong_vis, magme[3];
  double jut = 0.0;
  int iplfrom = SE_VENUS;
  int nstep = 0, istep;
  double dx;
  double x[6], xs[6], x0[6], x1[6], x2[6];
  double xp[6], xp0[6], xp1[6] = {0}, xp2[6], xs0[6], xs1[6] = {0}, xs2[6];
  double xel0[6], xel1[6] = {0}, xel2[6], xang0[6], xang1[6] = {0}, xang2[6], xma0[6], xma1[6] = {0}, xma2[6];
  double xh0[6], xh1[6] = {0}, xh2[6];
  double attr[20];
  double xel, xma, sunrad;
  double dt, dt1, dt2, elong, rphel;
  char ephepath[AS_MAXCH] = EPHEPATH;
  char fname[80] = "de431.eph";
  char *begindate = NULL;
  int iflag = SEFLG_RADIANS | SEFLG_SPEED;   /* external flag: helio, geo... */
  int nzer, iflgret;
  /*double tjd = 2436723.589269907;*/
  double tjd = 2436723.588888889;
  double t, te, tend, t2, t3, tstep;
  double delt;
  struct tm *tim;
  time_t tloc;
  EVENT *pev0;
  tstep = 1;
  nstep = 366;
  pmodel = PMODEL_SCREEN;
  strcpy(ephepath, EPHEPATH);
  time(&tloc);
  tim = localtime (&tloc);
  sprintf(sdate, "%d/%02d/%02d", 1900+tim->tm_year, tim->tm_mon + 1, tim->tm_mday);
  for (i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-doall") == 0) {
      do_flag = DO_ALL;
    } else if (strcmp(argv[i], "-noingr") == 0) {
      do_flag &= ~DO_INGR;
    } else if (strcmp(argv[i], "-doconj") == 0) {
      do_flag |= DO_CONJ;
    } else if (strcmp(argv[i], "-dobrill") == 0) {
      do_flag |= DO_BRILL;
    } else if (strcmp(argv[i], "-dorise") == 0) {
      do_flag |= DO_RISE;
    } else if (strcmp(argv[i], "-doelong") == 0) {
      do_flag |= DO_ELONG;
    } else if (strcmp(argv[i], "-doretro") == 0) {
      do_flag |= DO_RETRO;
    } else if (strcmp(argv[i], "-doaps") == 0) {
      do_flag |= DO_APS;
    } else if (strcmp(argv[i], "-dolphase") == 0) {
      do_flag |= DO_LPHASE;
    } else if (strcmp(argv[i], "-donode") == 0) {
      do_flag |= DO_NODE;
    } else if (strcmp(argv[i], "-doingr") == 0) {
      do_flag |= DO_INGR;
    } else if (strcmp(argv[i], "-doing45") == 0) {
      do_flag |= DO_INGR45;
    } else if (strcmp(argv[i], "-doasp") == 0) {
      do_flag |= DO_ASPECTS;
      iplfrom = -99;
    } else if (strcmp(argv[i], "-dovoc") == 0) {
      do_flag |= DO_VOC;
      iplfrom = SE_MOON;
    } else if (strcmp(argv[i], "-getday") == 0) {
      get_data_of_day = TRUE;
    } else if (strcmp(argv[i], "-et") == 0) {
      ephemeris_time = TRUE;
    } else if (strcmp(argv[i], "-jd") == 0) {
      show_jd = TRUE;
    } else if (strncmp(argv[i], "-cycol", 6) == 0) {
      ncycol = atoi(argv[i] + 6);
    } else if (strcmp(argv[i], "-ep") == 0) {
      output_extra_prec = TRUE;
    } else if (strncmp(argv[i], "-ejpl", 5) == 0) {
      whicheph = SEFLG_JPLEPH;
      if (*(argv[i]+5) != '\0')
	strcpy(fname, argv[i]+5);
    } else if (strcmp(argv[i], "-eswe") == 0) {
      whicheph = SEFLG_SWIEPH;
    } else if (strcmp(argv[i], "-emos") == 0) {
      whicheph = SEFLG_MOSEPH;
    } else if (strcmp(argv[i], "-zlong") == 0) {
      znam = zod_nam_long;
    } else if (strcmp(argv[i], "-znam3") == 0) {
      znam = zod_nam3;
    } else if (strcmp(argv[i], "-monnum") == 0) {
      for (n = 1; n <= 12; n++) {
        month_nam[n] = malloc(4);
        sprintf(month_nam[n], "%3d", n);
      }
    } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-?") == 0) {
      printf("%s%s", info1, info2);
      return OK;
#if PRINTMOD
    } else if (strcmp(argv[i], "-mpdf") == 0) {
      pmodel = PMODEL_PDF;
    } else if (strcmp(argv[i], "-mps") == 0) {
      pmodel = PMODEL_PS;
    } else if (strcmp(argv[i], "-mscreen") == 0) {
      pmodel = PMODEL_SCREEN;
#endif
    } else if (strcmp(argv[i], "-j2000") == 0) {
      iflag |= SEFLG_J2000;
    } else if (strcmp(argv[i], "-icrs") == 0) {
      iflag |= SEFLG_ICRS;
    } else if (strcmp(argv[i], "-hel") == 0) {
      iflag |= SEFLG_HELCTR;
    } else if (strcmp(argv[i], "-bary") == 0) {
      iflag |= SEFLG_BARYCTR;
    } else if (strcmp(argv[i], "-true") == 0) {
      iflag |= SEFLG_TRUEPOS;
    } else if (strcmp(argv[i], "-noaberr") == 0) {
      iflag |= SEFLG_NOABERR;
    } else if (strcmp(argv[i], "-nodefl") == 0) {
      iflag |= SEFLG_NOGDEFL;
    } else if (strcmp(argv[i], "-nonut") == 0) {
      iflag |= SEFLG_NONUT;
    } else if (strcmp(argv[i], "-noprec") == 0) {
      iflag |= SEFLG_J2000;
    } else if (strcmp(argv[i], "-roundmin") == 0) {
      do_round_min = TRUE;
    } else if (strcmp(argv[i], "-dgap") == 0) {
      date_gap = TRUE;
    } else if (strcmp(argv[i], "-motab") == 0) {
      do_motab = TRUE;
      do_flag = DO_INGR;
      stit = "Sign Ingresses";
    } else if (strcmp(argv[i], "-mojap") == 0) {
      do_mojap = TRUE;
      do_flag = DO_LPHASE;
      phase_mod = 90;
      xpos -= xdate;
      xdate = 0;
      max_cols = 4;
      memcpy(xcol, xcol4, sizeof(xcol));
      stit = "Phases";
      if (atoi(argv[i+1]) > 0) {
        phase_mod = atoi(argv[i+1]);
	i++;
      }
    } else if (strcmp(argv[i], "-gmtoff") == 0) {
      i++;
      gmtoff = atof(argv[i]);
    } else if (strncmp(argv[i], "-p", 2) == 0) {
      spno = argv[i]+2;
      /* single factor */
      if (*spno >= '0' && *spno <= '9' && atol(spno) < SE_NPLANETS) {
	iplfrom = atol(spno);
      /* ecliptic, nutation */
      } else if ((ipl = atol(spno)) == -1) {
	iplfrom = ipl;
      } else if (*spno == 'h') {
	iplfrom = atol(spno + 1) + SE_FICT_OFFSET;
	if (iplfrom > SE_WALDEMATH) {
	  printf("illegal planet number %s\n", argv[i]);
	  exit(1);
	}
      } else {
	printf("illegal planet number %s\n", argv[i]);
	exit(1);
      }
    } else if (strncmp(argv[i], "-n", 2) == 0) {
      n = atoi(argv[i]+2);
      if (n > 0) nstep = n;
    } else if (strncmp(argv[i], "-s", 2) == 0) {
      tstep = atof(argv[i]+2);
    } else if (strncmp(argv[i], "-b", 2) == 0) {
      begindate = argv[i] + 2;
    } else if (strcmp(argv[i], "-cl") == 0) {
      print_cl = FALSE;
    } else if (strncmp(argv[i], "-g", 2) == 0) {
      gap = argv[i] + 2;
      if (*gap == '\0') gap = "\t";
    } else if (strncmp(argv[i], "-tzone", 6) == 0) {
      tzone = atof(argv[i]+6);
    } else if (strncmp(argv[i], "-transitstderr", 6) == 0) {
      transits_to_stderr = TRUE;
    } else {
      printf("illegal option %s\n", argv[i]);
      exit(1);
    }
  }
  set_planet_name(iplfrom);
  strcpy(cmdline, "Command: ");
  for (i = 0; i < argc; i++) {
    if (strlen(cmdline) + strlen(argv[i]) < sizeof(cmdline) - 2)
      sprintf(cmdline + strlen(cmdline), "%s ", argv[i]);
  }
  swe_version(sout);
#if PRINTMOD
  if (pmodel != PMODEL_SCREEN) {
    printmod_set_printer(pmodel, 0);
  } else {
    printf("%s\n\n", cmdline);
    printf("Date: %s\tSwissEph version %s\n%s\n\n", sdate, sout, cmdline);
  }
#else
  printf("%s\n", "Please note: swevents is not a supported part of Swiss Ephemeris. In case of errors,\nplease debug and submit code fixes to the Swiss Ephemeris mailing list.");
  printf("Date: %s\tSwissEph version %s\n%s\nplanet %s\n\n", sdate, sout, cmdline, planet_name);
#endif
  swe_set_ephe_path(ephepath);
  swe_set_jpl_file(fname);
  iflag |= whicheph;
  if (begindate == NULL) {
    printf ("datum ?");
    sp = fgets(s, AS_MAXCH, stdin);
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
    swe_revjul(tjd, gregflag, &jyear, &jmon, &jday, &jut);
  } else if (*sp == '+') {
    n = atoi(sp);
    if (n == 0) n = 1;
    tjd += n;
    swe_revjul(tjd, gregflag, &jyear, &jmon, &jday, &jut);
  } else if (*sp == '-') {
    n = atoi(sp);
    if (n == 0) n = -1;
    tjd += n;
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
    tjd = swe_julday(jyear,jmon,jday,jut,gregflag);        /* PLACALC :
							get jd for the
							date you want */
  }
  sprintf(syear0, "%02d", jyear);
  swe_revjul(tjd + tstep * nstep, gregflag, &jyear, &jmon, &jday, &jut);
  sprintf(sdateto, "%02d", jyear);
  for (t = tjd, istep = 0; istep <= nstep; t += tstep, istep++) {
    if (t < 2299160.5)
      gregflag = FALSE;
    else
      gregflag = TRUE;
    if (strstr(spsave, "jul") != NULL)
      gregflag = FALSE;
    else if (strstr(spsave, "greg") != NULL)
      gregflag = TRUE;
    swe_revjul(t, gregflag, &jyear, &jmon, &jday, &jut);
    if (!ephemeris_time) {
      //delt = swe_deltat(t);
      delt = swe_deltat_ex(t, whicheph, serr);
      te = t + delt;
    } else {
      te = t;
    }
    ipl = iplfrom;
    // handle the three old swevents.c modes and exit
    if (get_data_of_day) {
      if ((pev0 = (EVENT *) calloc((size_t) NEVENTMAX, sizeof(EVENT))) == NULL) {
	fprintf(stderr, "could not allocate structure for event data\n");
	return ERR;
      }
      if (extract_data_of_day(do_flag, te, 1.6, SPLAN_ASPECTS, SASP_ASPECTS, pev0, serr) == ERR) {
	fprintf(stderr, "%s\n", serr);
	return ERR;
      } 
      return OK;
    }
    /* moon void of course */
    if (do_flag & DO_VOC) {
      tend = te + nstep;
      if (calc_all_voc(iflag, te, tend, serr) == ERR) {
	fprintf(stderr, "%s\n", serr);
	return ERR;
      }
      return OK;
    }
    /* mundane aspects */
    if (do_flag & DO_ASPECTS) {
      tend = te + nstep;
      if ((pev0 = (EVENT *) calloc((size_t) NEVENTMAX, sizeof(EVENT))) == NULL) {
	fprintf(stderr, "could not allocate structure for event data\n");
	return ERR;
      }
      if (calc_all_crossings(iflag, CTYP_MASPECTS, te, tend, tstep, SPLAN_ASPECTS, SASP_ASPECTS, 0, NULL, pev0, serr) == ERR) {
	fprintf(stderr, "%s\n", serr);
	return ERR;
      }
      return OK;
    }
    /* we have always 
     * - three positions and speeds for venus
     * - three positions and speeds for sun
     * - three elongations of venus (with elongation speed)
     * - three magnitudes
     */
    memcpy(xp0, xp1, 6 * sizeof(double));	/* planet */
    memcpy(xp1, xp2, 6 * sizeof(double));
    memcpy(xh0, xh1, 6 * sizeof(double));	/* heliocentric */
    memcpy(xh1, xh2, 6 * sizeof(double));	/* heliocentric */
    memcpy(xs0, xs1, 6 * sizeof(double));	/* sun */
    memcpy(xs1, xs2, 6 * sizeof(double));
    memcpy(xel0, xel1, 6 * sizeof(double));	/* elongation in longitude */
    memcpy(xel1, xel2, 6 * sizeof(double));
    memcpy(xang0, xang1, 6 * sizeof(double));/* ang. dist. from sun */
    memcpy(xang1, xang2, 6 * sizeof(double));
    memcpy(xma0, xma1, 6 * sizeof(double));	/* magnitude */
    memcpy(xma1, xma2, 6 * sizeof(double));
    iflgret = swe_calc(te, (int) ipl, iflag, xp2, serr);
    if (iflgret < 0) {
      fprintf(stderr, "return code %d, mesg: %s\n", iflgret, serr);
    }
    iflgret = swe_calc(te, SE_SUN, iflag, xs2, serr);
    if (iflgret < 0) {
      fprintf(stderr, "return code %d, mesg: %s\n", iflgret, serr);
    }
    iflgret = swe_calc(te, (int) ipl, iflag/*|SEFLG_HELCTR*/, xh2, serr);
    if (iflgret < 0) {
      fprintf(stderr, "return code %d, mesg: %s\n", iflgret, serr);
    }
    /* true heliocentric distance */
    /* elongation of planet measured on ecliptic */
    for (i = 0; i <= 5; i++)
      xel2[i] = xp2[i] - xs2[i];
    xel2[0] = swe_radnorm(xel2[0]);
    if (xel2[0] > M_PI)
      xel2[0] -= 2 * M_PI;
    /* other values */
    // cartesian coordinates of planet and sun
    swi_polcart(xp2, x1);
    swi_polcart(xs2, x2);
    for (i = 0; i <= 2; i++)
      x[i] = -x2[i] + x1[i];
    /* 'apparent' hel. distance of planet*/
    rphel = sqrt(square_sum(x));
    // elongation of planet = angular distance of planet from sun 
    elong = acos((xs2[2] * xs2[2] + xp2[2] * xp2[2] - rphel * rphel) / 2.0 / xs2[2] / xp2[2]);
    // this is equivalent to:
    // elong = acos(swi_dot_prod_unit(x1, x2));
    xang2[0] = elong;
    if (rphel != 0 && ipl <= 4) {
      if ((i = swe_pheno(te, ipl, iflag, attr, serr)) < 0) {
	fprintf(stderr, "return code %d, mesg: %s\n", i, serr);
	xma2[0] = 1;
      } else {
	xma2[0] = attr[4];
      }
      //fprintf(stderr, "mag1=%.7f\n", xma2[0]);
    } else {
      xma2[0] = 1;
    }
    if (istep >= 2) { /* now all of the arrays xp* and xs* are filled */
      /* conjunctions with sun, in longitude */
      if (do_flag & DO_LPHASE) 
	goto l_phase;
      if (!(do_flag & DO_CONJ))
	goto l_noconj;
      if (sign_change(xel1[0] ,xel2[0])) {
	double el0 = xel0[0], el1 = xel1[0], el2 = xel2[0];
	if (el0 > M_PI / 2) el0 -= M_PI;
	if (el0 < -M_PI / 2) el0 += M_PI;
	if (el1 > M_PI / 2) el1 -= M_PI;
	if (el1 < -M_PI / 2) el1 += M_PI;
	if (el2 > M_PI / 2) el2 -= M_PI;
	if (el2 < -M_PI / 2) el2 += M_PI;
	nzer = find_zero(el0, el1, el2, tstep, &dt, &dt2);
	if (nzer > 0) {
	  t2 = te + dt;
	  iflgret = swe_calc(t2, (int) ipl, iflag, x, serr);
	  iflgret = swe_calc(t2, SE_SUN, iflag, xs, serr);
	  is_opposition = 0;
	  if (ipl != SE_VENUS && ipl != SE_MERCURY) {
	    if (fabs(xel1[0]) > M_PI / 2) {
	      is_opposition = 1;
	      print_item("opposition", t2, x[0] * RADTODEG, x[1] * RADTODEG, HUGE);
	    } else
	      print_item("conjunction", t2, x[0] * RADTODEG, x[1] * RADTODEG, HUGE);
	  } else {
	    if (x[3] > 0)
	      print_item("superior conj", t2, x[0] * RADTODEG, x[1] * RADTODEG, HUGE);
	    else
	      print_item("inferior conj", t2, x[0] * RADTODEG, x[1] * RADTODEG, HUGE);
	  }
	  /* planet is behind solar disk or is transiting it */
	  for (j = 0, dt1 = tstep; j <= 5; j++, dt1 /= 3) {
	    for (k = 0; k <= 2; k++) {
	      switch(k) {
		case 0:
		  t3 = t2 - dt1;
		  break;
		case 1:
		  t3 = t2;
		  break;
		case 2:
		  t3 = t2 + dt1;
		  break;
	      }
	      iflgret = swe_calc(t3, (int) ipl, iflag, xp, serr);
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
	  iflgret = swe_calc(t2, (int) ipl, iflag, x, serr);
	  iflgret = swe_calc(t2, SE_SUN, iflag, xs, serr);
	  /* minimum elongation of planet */
	  if (is_opposition) {
	    /*print_item("  maximum elong", t2, x[0] * RADTODEG, xel*RADTODEG, HUGE)*/;
	  } else {
	    print_item("  minimum elong", t2, x[0] * RADTODEG, xel*RADTODEG, HUGE);
	  }
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
	    if (x[3] > 0 || ipl > SE_VENUS) {
	      strcpy(sout, "  behind sun begin");
	    } else {
	      strcpy(sout, "  transit begin");
	      print_item("  transit middle", t2, x[0] * RADTODEG, xel*RADTODEG, HUGE);
	    }
	    dt = sqrt(sunrad * sunrad - xel * xel);
	    /*dt = acos(cos(sunrad) / cos(xel)); is not better */
	    dt /= sqrt((x[3]-xs[3]) * (x[3]-xs[3]) + (x[4]-xs[4]) * (x[4]-xs[4]));
	    iflgret = swe_calc(t2-dt, (int) ipl, iflag, x, serr);
	    print_item(sout, t2 - dt, x[0] * RADTODEG, HUGE, HUGE);
	    if (x[3] > 0 || ipl > SE_VENUS)
	      strcpy(sout, "  behind sun end");
	    else
	      strcpy(sout, "  transit end");
	    iflgret = swe_calc(t2+dt, (int) ipl, iflag, x, serr);
	    print_item(sout, t2 + dt, x[0] * RADTODEG, HUGE, HUGE);
	  }
	}
      }
      l_noconj:;
      if (!(do_flag & DO_BRILL))
	goto l_nobrill;
      /* greatest brillancy */
      if (ipl <= SE_MARS
	  && xma0[0] > xma1[0] 
	  && xma2[0] > xma1[0] 
	  && xang1[0] > 10*DEGTORAD) {
	find_maximum(xma0[0], xma1[0], xma2[0], tstep, &dt, &xma);
	t2 = te + dt;
	for (j = 0, dt1 = tstep; j <= 5; j++, dt1 /= 3) {
	  for (k = 0; k <= 2; k++) {
	    switch(k) {
	      case 0:
		t3 = t2 - dt1;
		break;
	      case 1:
		t3 = t2;
		break;
	      case 2:
		t3 = t2 + dt1;
		break;
	    }
	    //iflgret = swe_calc(t3, (int) ipl, iflag, xp, serr);
	    //iflgret = swe_calc(t3, SE_SUN, iflag, xs, serr);
	    //swi_polcart(xp, x1);
	    //swi_polcart(xs, x2);
	    //for (i = 0; i <= 2; i++)
	    //  x[i] = -x2[i] + x1[i];
	    //rphel = sqrt(square_sum(x));
	    if ((i = swe_pheno(t3, ipl, iflag, attr, serr)) < 0) {
	      fprintf(stderr, "return code %d, mesg: %s\n", i, serr);
	      x0[k] = 1;
	      exit(0);
	    } else {
	      x0[k] = attr[4];
	    }
	  }
	  find_maximum(x0[0], x0[1], x0[2], dt1, &dt, &xma);
	  t2 = t2 + dt1 + dt;
	}
	iflgret = swe_calc(t2, (int) ipl, iflag, x, serr);
	print_item("greatest brilliancy", t2, x[0] * RADTODEG, HUGE, xma);
      }
      l_nobrill:;
      if (!(do_flag& DO_RISE) && !(do_flag & DO_ELONG))
	goto l_noelong;
      /* rise and set of morning and evening star 
       * This calculation is very simplistic. Exact dates are not possible 
       * because they depend on the geographic position of the observer.
       */
      *sout = '\0';
      magme[0] = magme[1] = magme[2] = 0;
      switch(ipl) {
	case SE_MERCURY:
	  magme[0] = xma0[0];
	  magme[1] = xma1[0];
	  magme[2] = xma2[0];
	case SE_VENUS:
	case SE_JUPITER:
	  elong_vis = 10;
	  break;
	case SE_MOON:
	case SE_MARS:
	case SE_SATURN:
	  elong_vis = 15;
	  break;
	default:
	  elong_vis = 15;
	  break;
      }
      if (xang1[0] * RADTODEG > elong_vis + magme[1] 
	    && xang2[0] * RADTODEG < elong_vis + magme[2]) {
	if (xel1[0] > 0) 
	  strcpy(sout, "evening set");
	else
	  strcpy(sout, "morning set");
      }
      if (xang1[0] * RADTODEG < elong_vis + magme[1] 
	    && xang2[0] * RADTODEG > elong_vis + magme[2]) {
	if (xel1[0] > 0)
	  strcpy(sout, "evening rise");
	else
	  strcpy(sout, "morning rise");
      }
      if (*sout != '\0') {
	x[0] = xang0[0] - elong_vis * DEGTORAD;
	x[1] = xang1[0] - elong_vis * DEGTORAD;
	x[2] = xang2[0] - elong_vis * DEGTORAD;
	// with Mercury, brightness is taken into account 
	// (this method from Expl. Suppl. of AA 1984, however 
	// magnitudes used are from Swisseph >= 2.07)
	if (ipl == SE_MERCURY) {
	  x[0] -= magme[0] * DEGTORAD;
	  x[1] -= magme[1] * DEGTORAD;
	  x[2] -= magme[2] * DEGTORAD;
	}
      }
      if (!(do_flag & DO_RISE))
	goto l_norise;
      if (*sout != '\0') {
	double cos_elong, deg_elong, derr;
	if ((nzer = find_zero(x[0], x[1], x[2], tstep, &dt, &dt2)) > 0) {
	  t2 = te + dt;
	  /* some test code to verify actual elongation at found date */
	  iflgret = swe_calc(t2, (int) ipl, iflag, x2, serr);
	  iflgret = swe_calc(t2, SE_SUN, iflag, x1, serr);
	  cos_elong = cos(x2[0]-x1[0]) * cos(x2[1]-x1[1]);
	  deg_elong = acos(cos_elong) * RADTODEG;
	  derr = fabs(deg_elong) - elong_vis;
	  if (ipl == SE_MERCURY) {
	    //double xx1[6], xx2[6], rh;
	    double ma;
	    //swi_polcart(x2, xx2);
	    //swi_polcart(x1, xx1);
	    //for (i = 0; i <= 2; i++)
	    //  x[i] = -xx1[i] + xx2[i];
	    /* 'apparent' hel. distance of planet*/
	    //rh = sqrt(square_sum(x));
	    if ((i = swe_pheno(t2, ipl, iflag, attr, serr)) < 0) {
	      fprintf(stderr, "return code %d, mesg: %s\n", i, serr);
	      exit(0);
	    }
	    ma = attr[4];
	    derr -= ma;
	  }
	  if (fabs(derr) > 0.01)
	    fprintf(stderr, "warning elongation planet %d test deviation %.3f arcsec at t=%f\n", ipl, derr * 3600, t2);
      /* end test code */
	  print_item(sout, t2, x2[0] * RADTODEG, HUGE, HUGE);
	}
      }
      l_norise:;
      if (!(do_flag & DO_ELONG) || ipl >= SE_MARS)
	goto l_noelong;
      /* maximum elongation */
      if (fabs(xang0[0]) < fabs(xang1[0]) && fabs(xang2[0]) < fabs(xang1[0])) {
	find_maximum(xang0[0], xang1[0], xang2[0], tstep, &dt, &xel);
	t2 = te + dt;
	for (j = 0, dt1 = tstep; j <= 5; j++, dt1 /= 3) {
	  for (k = 0; k <= 2; k++) {
	    switch(k) {
	      case 0:
		t3 = t2 - dt1;
		break;
	      case 1:
		t3 = t2;
		break;
	      case 2:
		t3 = t2 + dt1;
		break;
	    }
	    iflgret = swe_calc(t3, (int) ipl, iflag, xp, serr);
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
	iflgret = swe_calc(t2, (int) ipl, iflag, x, serr);
	if (ipl > SE_VENUS)
	  strcpy(sout, "maximum elong.");
	else if (xel1[0] > 0)
	  strcpy(sout, "evening max el");
	else
	  strcpy(sout, "morning max el");
	print_item(sout, t2, x[0] * RADTODEG, xel*RADTODEG, HUGE);
      }
      l_noelong:;
      if (!(do_flag & DO_RETRO))
	goto l_noretro;
      /* retrograde or direct, maximum position */
      if ((xp1[3] < 0 && xp2[3] >= 0) || (xp1[3] > 0 && xp2[3] <= 0)) {
	t2 = te - xp2[3] / ((xp2[3] - xp1[3]) / tstep);
	for (j = 0, dt1 = tstep; j <= 5; j++, dt1 /= 3) {
	  for (k = 0; k <= 1; k++) {
	    switch(k) {
	      case 0:
		t3 = t2;
		break;
	      case 1:
		t3 = t2 + dt1;
		break;
	    }
	    iflgret = swe_calc(t3, (int) ipl, iflag, xp, serr);
	    x0[k] = xp[3];
	  }
	  t2 = t3 - x0[1] / ((x0[1] - x0[0]) / dt1);
	  //fprintf(stderr, "tt=%.8f\n", t2);
	}
	iflgret = swe_calc(t2, (int) ipl, iflag, x, serr);
	if (xp2[3] < 0)
	  strcpy(sout, "retrograde");
	else
	  strcpy(sout, "direct");
	print_item(sout, t2, x[0] * RADTODEG, HUGE, HUGE);
      }
      l_noretro:;
      if (!(do_flag & DO_APS))
	goto l_noaps;
      /* apsides */
      if ((xh2[2] < xh1[2] && xh0[2] < xh1[2]) || (xh2[2] > xh1[2] && xh0[2] > xh1[2])) {
	x0[0] = xh0[2];
	x0[1] = xh1[2];
	x0[2] = xh2[2];
	find_maximum(x0[0], x0[1], x0[2], tstep, &dt, &xel);
	t2 = te + dt;
	for (j = 0, dt1 = tstep; j <= 4; j++, dt1 /= 3) {
	  for (k = 0; k <= 2; k++) {
	    switch(k) {
	      case 0:
		t3 = t2 - dt1;
		break;
	      case 1:
		t3 = t2;
		break;
	      case 2:
		t3 = t2 + dt1;
		break;
	    }
	    iflgret = swe_calc(t3, (int) ipl, iflag/*|SEFLG_HELCTR*/, xp, serr);
	    x0[k] = xp[2];
	  }
	  find_maximum(x0[0], x0[1], x0[2], dt1, &dt, &xel);
	  t2 = t2 + dt1 + dt;
	}
	iflgret = swe_calc(t2, (int) ipl, iflag/*|SEFLG_HELCTR*/, x, serr);
	if (xh2[2] < xh1[2]) {
	  if (iflag & SEFLG_HELCTR)
	    strcpy(sout, "aphelion");
	  else
	    strcpy(sout, "max. Earth dist.");
	} else {
	  if (iflag & SEFLG_HELCTR)
	    strcpy(sout, "perihelion");
	  else
	    strcpy(sout, "min. Earth dist.");
	}
	print_item(sout, t2, x[0] * RADTODEG, HUGE, x[2]);
      }
      l_noaps:;
      if (!(do_flag & DO_NODE))
	goto l_nonode;
      if (sign_change(xh1[1] ,xh2[1])) {	// latitude sign change?
	if ((nzer = find_zero(xh0[1], xh1[1], xh2[1], tstep, &dt, &dt2)) > 0) {
	  t2 = te + dt;
	  iflgret = swe_calc(t2, (int) ipl, iflag/*|SEFLG_HELCTR*/, x, serr);
	  if (xh2[1] >= 0)
	    strcpy(sout, "asc. node");
	  else
	    strcpy(sout, "desc. node");
	  print_item(sout, t2, x[0] * RADTODEG, HUGE, HUGE);
	}
      }
      l_nonode:;
      /* sign ingresses */
      if (do_flag & DO_INGR) {
	double x0, x1, x2, xcross, d12, d01, x[6], tx, dx;
	int j;
	x2 = xp2[0] * RADTODEG;	// at t
	x1 = xp1[0] * RADTODEG;	// at t - tstep
	x0 = xp0[0] * RADTODEG;	// at t - 2 * tstep
	// x2 is in [0..360[ 
	// normalize x1 and x0 so that there are no jumps
	d12 = swe_difdeg2n(x2, x1);
	x1 = x2 - d12;
	d01 = swe_difdeg2n(x1, x0);
	x0 = x1 - d01;
	for (i = 0; i <= 12; i++) {	// we consider 13 sign cusps, 0..360
				      // because x2 can be 359, x1 = 361, x0 = 363
	  xcross = i * 30.0;
	  // is xcross between x1 and x2?
	  // or x2 critically near xcross
	  if (sign_change((x1 - xcross), (x2 - xcross))
	  || (fabs(x0-x1) + fabs(x1-x2) > fabs(x2 - xcross))) {
	    nzer = find_zero(x0 - xcross, x1 - xcross, x2 - xcross, tstep, &dt1, &dt2);
	    if (nzer == 1) {
	      tx = te + dt1;
	      for (j = 0; j < 3; j++) {
		iflgret = swe_calc(tx, (int) ipl, iflag, x, serr);
		x[0] *= RADTODEG; x[3] *= RADTODEG;
		dx = swe_degnorm(x[0] - xcross);
		if (dx > 180) dx -= 360;
		tx -= dx / x[3];
	      }
	      izod = i % 12;
	      if (x2 <= xcross) {
		strcpy(sout, "ingress retro. ");
		izod = (izod + 11) % 12;
	      } else {
		strcpy(sout, "ingress ");
	      }
	      print_item(sout, tx, izod, HUGE, HUGE);
	    }
	    if (nzer == 2) {	// double crossed in 1 timestep
	      fprintf(stderr, "warning double crossing in ingress, reduced accuracy: planet=%d, tjd1=%.8f, tjd2=%.8f\n", ipl, te + dt1, te + dt2);
	      if (x2 > xcross) {  // retro, then direct
		strcpy(sout, "ingress retro. ");
		izod = (i + 11) % 12;
		print_item(sout, te + dt1, izod, HUGE, HUGE);
		strcpy(sout, "ingress ");
		izod = i % 12;
		print_item(sout, te + dt2, izod, HUGE, HUGE);
	      } else {	    // direct, then retro
		strcpy(sout, "ingress ");
		izod = i % 12;
		print_item(sout, te + dt1, izod, HUGE, HUGE);
		strcpy(sout, "ingress retro. ");
		izod = (i + 11) % 12;
		print_item(sout, te + dt2, izod, HUGE, HUGE);
	      }
	    }
	  }
	}
      }
      /* 45° ingresses */
      if (do_flag & DO_INGR45) {
	double x0, x1, x2, xcross, d12, d01, tx, x[6];
	int j;
	x2 = xp2[0] * RADTODEG;	// at t
	x1 = xp1[0] * RADTODEG;	// at t - tstep
	x0 = xp0[0] * RADTODEG;	// at t - 2 * tstep
	// x2 is in [0..360[ 
	// normalize x1 and x0 so that there are no jumps
	d12 = swe_difdeg2n(x2, x1);
	x1 = x2 - d12;
	d01 = swe_difdeg2n(x1, x0);
	x0 = x1 - d01;
	for (i = 0; i < 4; i++) {	
	  xcross = 45 + i * 90.0;
	  // is xcross between x1 and x2?
	  // or x2 critically near xcross
	  if (sign_change((x1 - xcross), (x2 - xcross))
	  || (fabs(x0-x1) + fabs(x1-x2) > fabs(x2 - xcross))) {
	    nzer = find_zero(x0 - xcross, x1 - xcross, x2 - xcross, tstep, &dt1, &dt2);
	    izod = xcross / 30;
	    if (nzer == 1) {
	      tx = te + dt1;
	      for (j = 0; j < 3; j++) {
		iflgret = swe_calc(tx, (int) ipl, iflag, x, serr);
		x[0] *= RADTODEG; x[3] *= RADTODEG;
		dx = swe_degnorm(x[0] - xcross);
		if (dx > 180) dx -= 360;
		tx -= dx / x[3];
	      }
	      if (x2 <= xcross) {
		strcpy(sout, "ingr45 retro. ");
	      } else {
		strcpy(sout, "ingr45 ");
	      }
	      print_item(sout, tx, izod, HUGE, HUGE);
	    }
	    if (nzer == 2) {	// double crossed in 1 timestep
	      if (x2 > xcross) {  // retro, then direct
		strcpy(sout, "ingr45 retro. ");
		print_item(sout, te + dt1, izod, HUGE, HUGE);
		strcpy(sout, "ingr45 ");
		print_item(sout, te + dt2, izod, HUGE, HUGE);
	      } else {	    // direct, then retro
		strcpy(sout, "ingr45 ");
		print_item(sout, te + dt1, izod, HUGE, HUGE);
		strcpy(sout, "ingr45 retro. ");
		print_item(sout, te + dt2, izod, HUGE, HUGE);
	      }
	    }
	  }
	}
      }
      /* lunar phases */
l_phase:
      if (do_flag & DO_LPHASE) {
	int new_phase;
	int old_phase;
	int nphases = swe_d2l(360 / phase_mod);
	int j;
	double d, dv, xm[6], xs[6];
	x2[0] = swe_degnorm((xp2[0] - xs2[0]) * RADTODEG);
	x1[0] = swe_degnorm((xp1[0] - xs1[0]) * RADTODEG);
	x0[0] = swe_degnorm((xp0[0] - xs0[0]) * RADTODEG);
	if (x0[0] > x1[0]) {
	  x1[0] += 360;
	  x2[0] += 360;
	}
	if (x1[0] > x2[0]) {
	  x2[0] += 360;
	}
	/* is there a phase change within last tstep ? */
	new_phase = floor(x2[0] / phase_mod) + 1;
	old_phase = floor(x1[0] / phase_mod) + 1;
	if (old_phase != new_phase) {
	  double lphase = HUGE;
	  x0[0] = x0[0] / phase_mod - old_phase;
	  x1[0] = x1[0] / phase_mod - old_phase;
	  x2[0] = x2[0] / phase_mod - old_phase;
	  if ((nzer = find_zero(x0[0], x1[0], x2[0], tstep, &dt1, &dt2)) > 0) {
	    if (fabs(dt2) < fabs(dt1))
	      t2 = te + dt2;
	    else
	      t2 = te + dt1;
	    for (j = 0; j < 2; j++) {
	      iflgret = swe_calc(t2, (int) SE_MOON, iflag, xm, serr);
	      iflgret = swe_calc(t2, (int) SE_SUN, iflag, xs, serr);
	      d = swe_radnorm(xm[0] - xs[0]) * RADTODEG;
	      dx = swe_degnorm(d - (new_phase - 1) * 90);
	      if (dx > 180) dx -= 360;
	      dv = (xm[3] - xs[3]) * RADTODEG;
	      t2 -= dx / dv;
	    }
	    while (new_phase > nphases) {
	      new_phase -= nphases;
	    }
	    strcpy(sout, "phase ");
	    if (pmodel == PMODEL_SCREEN) {
	      iflgret = swe_calc(t2, (int) SE_MOON, iflag, xm, serr);
	      lphase = xm[0] * RADTODEG;
	    }
	    print_item(sout, t2, new_phase, lphase, HUGE);
	  }
	}
      }
    }
    if (0 && (iflgret & whicheph) == 0) {
      sprintf(sout, "ephemeris %d", iflgret & SEFLG_EPHMASK);
      print_item(sout, HUGE, -1, HUGE, HUGE);
    }
  }
  /* close open files and free allocated space */
  if (do_motab && prev_yout != -999999)
    print_motab();
#if PRINTMOD
  printmod_close_page();
  printmod_close_printer();
#endif
  swe_close();
  return OK;
}

static char *set_planet_name(int ipl)
{
  switch(ipl) {
    case -99:
      planet_name = "-";
      break;
    case SE_SUN:
      planet_name = "Sun";
      break;
    case SE_VENUS:
      planet_name = "Venus";
      break;
    case SE_MERCURY:
      planet_name = "Mercury";
      break;
    case SE_MARS:
      planet_name = "Mars";
      break;
    case SE_JUPITER:
      planet_name = "Jupiter";
      break;
    case SE_SATURN:
      planet_name = "Saturn";
      break;
    case SE_URANUS:
      planet_name = "Uranus";
      break;
    case SE_NEPTUNE:
      planet_name = "Neptune";
      break;
    case SE_PLUTO:
      planet_name = "Pluto";
      break;
    default:
      swe_get_planet_name(ipl, spnam);
      planet_name = spnam;
      break;
  }
  return planet_name;
}

static int print_motab(void)
{
  int i, j;
  printf("%d\n", prev_yout);
  for (i = 1; i <= 12; i++)
    printf("\t%s", month_nam[i]);
  printf("\n");
  for (i = 0; i < 31; i++) {
    printf("%02d", i+1);
    for (j = 0; j < 12; j++)
      printf("\t%s", motab[j][i]);
    printf("\n");
  }
  printf("\n\n");
  memset(motab, 0, sizeof(motab));
  prev_yout = -999999;
  return OK;
}

/*
 * a planetary phenomenon ist printed;
 * date time is alway printed (in UT)
 * ingresses: s is "ingress" or "ingress retro" , only  date and 0/30 sign
 * ingr45: s is "ingr45" or "ingr45 retro" , only  date and 15 [r] sign
 * phases: s is "phase" , only  date and phase number
 * if delon or dmag are HUGE, they are not printed.
 */
static void print_item(char *s, double teph, double dpos, double delon, double dmag)
{
  static char smag[AS_MAXCH], sout[AS_MAXCH], serr[AS_MAXCH];
  int mout, dout, yout, hour, min, sec, izod;
  int ing_deg = 0;
  double hout;
  double secfr;
  double jut;
  AS_BOOL is_ingress;
  AS_BOOL is_ingr45;
  AS_BOOL is_phase;
  AS_BOOL is_retro = FALSE;
  char sign_deg[30];
  static AS_BOOL cycle_has_started = FALSE;
  char *jul = "";
  // test for nan cases. NaN is not equal to itself.
  if (teph != teph || delon != delon) return;
  jut = ing_deg * is_retro; // dummy to silence compiler;
//  static double teph_save = 0;
  teph += tzone/24;
  if (do_flag != DO_ALL) {
    cycle_has_started = TRUE;
    swe_revjul(teph, gregflag, &yout, &mout, &dout, &jut);
    sprintf(sdatefrom, "%d", yout);
    if (gregflag && yout < 1700)
      strcat(sdatefrom, "greg");
  }
  if (strncmp(s, "superior", 8) == 0
      || (ipl > SE_VENUS && strncmp(s, "conj", 4) == 0)) {
    if (!cycle_has_started) {
      swe_revjul(teph, gregflag, &yout, &mout, &dout, &jut);
      sprintf(sdatefrom, "%d", yout);
      if (gregflag && yout < 1700)
	strcat(sdatefrom, "greg");
    }
    cycle_has_started = TRUE;
  } else if (!cycle_has_started)
    return;
  is_ingress =  (strncmp(s, "ingr", 4) == 0);
  is_ingr45 =  (strncmp(s, "ingr45", 6) == 0);
  is_phase =  (strncmp(s, "phas", 4) == 0);
  if (gregflag == FALSE) {
    jul = "j";
  } 
  if (strstr(s, "ret") != NULL) {
    is_retro = TRUE;
    ing_deg = 30;
    strcpy(sign_deg, "30");
    if (is_ingr45) {
      ing_deg = 15;
      strcpy(sign_deg, "15 r");
    }
  } else {
    ing_deg = 0;
    strcpy(sign_deg, " 0");
    if (is_ingr45) {
      ing_deg = 15;
      strcpy(sign_deg, "15");
    }
  }
  if (transits_to_stderr && strstr(s, "transit") != NULL) {
    swe_revjul(teph, gregflag, &yout, &mout, &dout, &hout);
    if (strstr(s, "middle") != NULL) {
      fprintf(stderr, "/* %d %s %d %f */\n", yout, month_nam[mout], dout, hout);
    }
    fprintf(stderr, "%f,", teph);
    if (strstr(s, "end") != NULL) {
      fprintf(stderr, "\n");
    }
  }
  teph += gmtoff / 24;
  if (! ephemeris_time)  {
    teph = teph - swe_deltat_ex(teph, whicheph, serr);
  }
  // compute UT and add 0.5 minutes for later rounding to minutes 
  if (do_round_min) {
    teph += 0.5 / 1440.0 ;
  }
  swe_revjul(teph, gregflag, &yout, &mout, &dout, &hout);
  swe_split_deg(hout, SE_SPLIT_DEG_ROUND_SEC, &hour, &min, &sec, &secfr, &izod);
  if (dmag != HUGE) {
    if (strstr(s, "brill") != NULL) {
      sprintf(smag, "    %.1fm", dmag);
    } else {
      if (output_extra_prec)
	sprintf(smag, "   %8.13f AU", dmag);
      else
	sprintf(smag, "   %8.5f AU", dmag);
    }
  } else {
    *smag = '\0';
  }
  if (do_motab) {
    if (yout != prev_yout && prev_yout != -999999)
      print_motab();
    izod = (int) (dpos + 0.1);
    if (do_round_min)
      sprintf(sout, "%02d:%02d %s", hour, min,  znam[izod]);
    else 
      sprintf(sout, "%02d:%02d:%02d %s", hour, min, sec, znam[izod]);
    strcpy(motab[mout-1][dout-1], sout);
    prev_yout = yout;
    return;
  }
  if (is_ingress || is_phase)
    strcpy(s, " ");
  if (pmodel == PMODEL_SCREEN) {
    if (strncmp(s, "superior", 8) == 0)
      putchar('\n');
    else if (ipl > SE_VENUS && strncmp(s, "conj", 4) == 0)
      putchar('\n');
    if (!(is_ingress || is_phase))
      printf("%-20s%s", s, gap);
    if (date_gap) {
      if (do_round_min)
	printf("%02d%s%s%s%2d%s%s%02d:%02d",
	     yout, gap, month_nam[mout], gap, dout, jul, gap, hour, min);
      else
	printf("%02d%s%s%s%2d%s%s%02d:%02d:%02d",
	     yout, gap, month_nam[mout], gap, dout, jul, gap, hour, min, sec);
    } else {
      if (do_round_min)
	printf("%02d %s %2d %s %02d:%02d ",
	       yout, month_nam[mout], dout, jul, hour, min);
      else
	printf("%02d %s %2d %s %02d:%02d:%02d ",
	       yout, month_nam[mout], dout, jul, hour, min, sec);
    } 
    if (show_jd)
      printf("%sjd=%.8lf", gap, teph);
    printf("%s", gap);
    if (is_ingr45) {
      printf("%s", sign_deg);
      izod = (int) (dpos + 0.1);
      if (date_gap)
	printf("%s%s", gap, znam[izod]);
      else
	printf(" %s", znam[izod]);
    } else if (is_ingress) {
      printf("%s", sign_deg);
      izod = (int) (dpos + 0.1);
      if (date_gap)
	printf("%s%s", gap, znam[izod]);
      else
	printf(" %s", znam[izod]);
    } else if (is_phase) {
      izod = (int) (dpos + 0.1);
      switch(izod) {
      case 1:
	strcpy(sout, " New"); break;
      case 2:
	strcpy(sout, " h/wax"); break;
      case 3:
	strcpy(sout, " Full"); break;
      case 4:
	strcpy(sout, " h/wane"); break;
      }
      printf("%s%s%d%s", sout, gap, izod, gap);
      if (delon != HUGE) {
	printf("%s", dms(delon, BIT_ZODIAC|BIT_ROUND_SEC));
	delon = HUGE;
      }
    } else {
      printf("%s", dms(dpos, BIT_ZODIAC|BIT_ROUND_SEC));
    }
    if (delon != HUGE) {
      printf("%s%s", gap, dms(delon, BIT_ROUND_SEC));
    }
    if (*smag)
      printf("%s%s", gap, smag);  /**/
    printf("\n");
  } 
#if PRINTMOD
  else {
    CSEC cspos = dpos * DEG2CSEC;
    char cdtsgn, s2[AS_MAXCH];
    int dtmin, dthr, ncpt;
    int max_lines = MAX_LINES;
    static int line_count, page_count, col_count;
    static int icycol = 0;
    if (page_count == 0 && print_cl)
      max_lines -= 2;	/* leave space for command line output */
    if (line_count > 0 && (strncmp(s, "superior", 8) == 0 
       || (ipl > SE_VENUS && strncmp(s, "conj", 4) == 0))) {
      line_count++;
      if (ncycol > 0) {
	icycol++;
	if (icycol >= ncycol) {
	  line_count = max_lines;
	  icycol = 0;
	}
      }
    }
    if (line_count >= max_lines) {
      col_count++;
      line_count = 0;
      if (col_count == max_cols) {
        if (page_count == 0 && print_cl) {
	  SetFont(FT_TIM_8);
	  MoveToXYmm(xcol[0], ytop + (max_lines + 1) * line_space);
	  Print(cmdline);
	}
	page_count++;
	printmod_close_page();
	col_count = 0;
      }
    }
    if (col_count == 0 && line_count == 0) {
      printmod_open_page(NULL);
      print_date_time(s2);
      if (ephemeris_time) 
	sprintf(sout, "%s of %s  from %s through %s (ET), Astrodienst AG %s,       page %d", stit, planet_name, syear0, sdateto, s2, page_count + 1);
      else
	sprintf(sout, "%s of %s  from %s through %s (UT), Astrodienst AG %s,       page %d", stit, planet_name, syear0, sdateto, s2, page_count + 1);
      if (ephemeris_time) 
	MoveToXYmm(xcol[col_count], ytop - 3 * line_space);
      else 
	MoveToXYmm(xcol[col_count], ytop - 2 * line_space);
      SetFont(FT_TIM_10);
      Print(sout);
      if (ephemeris_time) {
	double delt;
	SetFont(FT_TIM_8);
	MoveToXYmm(xcol[col_count], ytop - 2 * line_space);
	//delt = swe_deltat(teph) * 86400; 
	delt = swe_deltat_ex(teph, whicheph, serr) * 86400; 
	dtmin = (fabs(delt) + 30) / 60;	/* delta t in rounded minutes */
	dthr = dtmin / 60;
	if (delt < 0) 
	  cdtsgn = '+';
	else 
	  cdtsgn = '-';
	dtmin %= 60;
	sprintf(sout, "(UT = ET %c %dh%2dm)", cdtsgn, dthr, dtmin);
	Print(sout);
      }
      if (*sdatefrom == '-') {
	int yb = atoi(sdatefrom);
	SetFont(FT_TIM_8);
	MoveToXYmm(xcol[col_count], ytop - 1 * line_space);
	sprintf(sout, "Attention, astronomical year style is used: The year %d in astronomical counting style is the year %d BCE in historical counting style.", yb, abs(yb) + 1);
	Print(sout);
      }
    }
    MoveToXYmm(xcol[col_count], ytop + line_count * line_space);
    SetFont(FT_TIM_8);
    if (! is_ingress)
      Print(s);
    MoveToXmm(xcol[col_count] + xdate);
    sprintf(sout, "%02d ", yout);
    Print(sout);
    Print(month_nam[mout]);
    ncpt = TextWidth("May") - TextWidth(month_nam[mout]);
    RmoveXcp(ncpt);
    sprintf(sout, " %02d %s %02d:%02d", dout, jul, min / 60, min % 60);
    Print(sout);
    MoveToXmm(xcol[col_count] + xpos);
    if (is_ingr45) {
      if (ing_deg < 10)
	RmoveXcp(TextWidth("0"));
      sprintf(sout, "%d%s", ing_deg, ODEGREE_STRING);
      Print(sout);
      if (is_retro) {
	SetFont(FT_SYMBS);
	Print(symb_special(SYMB_RETRO));
      }
      SetFont(FT_SYMBS);
      Print(symb_sign((int) dpos));
      SetFont(FT_TIM_8);
    } else if (is_ingress) {
      if (ing_deg < 10)
	RmoveXcp(TextWidth("0"));
      sprintf(sout, "%d%s", ing_deg, ODEGREE_STRING);
      Print(sout);
      if (is_retro) {
	SetFont(FT_SYMBS);
	Print(symb_special(SYMB_RETRO));
      }
      SetFont(FT_SYMBS);
      Print(symb_sign((int) dpos));
      SetFont(FT_TIM_8);
    } else if (is_phase) {
      int iphase = dpos;
      if (phase_mod == 180) {
        if (iphase == 1) 
	  strcpy(sout, "new");
	else
	  strcpy(sout, "full");
      } else if (phase_mod == 90) {
        switch(iphase) {
	case 1:
	  strcpy(sout, " New"); break;
	case 2:
	  strcpy(sout, ""); break;
	case 3:
	  strcpy(sout, " Full"); break;
	case 4:
	  strcpy(sout, ""); break;
	}
	SetFont(FT_SYMBN);
	Print(symb_special(SYMB_NEW_MOON + iphase - 1));
	SetFont(FT_TIM_8);
      } else {
        if (iphase == 1) 
	  strcpy(sout, "1 = new");
	else
	  sprintf(sout, "%d", iphase);
      }
      Print(sout);
    } else {
      printmod_pds(cspos, FT_SYMBS, 0);
    }
    if (delon != HUGE) {
      Print("    ");
      printmod_pds(delon * DEG2CSEC, FT_SYMBS, PDS_NO_ZOD_SIGN);
    }
    Print(smag);
    PrintLn();
    line_count++;
  }
# endif
}

static char *dms(double x, int iflag)
{
  int izod = 0;
  int k, kdeg, kmin, ksec;
  char *c = ODEGREE_STRING;
  char *sp;
  static char s[AS_MAXCH], s2[80];
  int sgn;
  *s = *s2 = '\0';
  again_dms:
  if (iflag & SEFLG_EQUATORIAL)
    c = "h";
  if (x < 0) {
    x = -x;
    sgn = -1;
  } else {
    sgn = 1;
  }
  if (iflag & BIT_ZODIAC) {
    izod = (int) floor(x / 30);
    x = fmod(x, 30);
    kdeg = (long) x;
    sprintf(s, "%2d %s ", kdeg, znam[izod]);
  } else {
    kdeg = (long) x;
    sprintf(s, " %3d%s", kdeg, c);
  }
  x -= kdeg;
  x *= 60;
  if (iflag & BIT_ROUND_MIN)
    x += 0.5;
  kmin = (long) x;
  if (kmin == 60) {
    x = (kdeg + 1) * sgn;
    if (iflag & BIT_ZODIAC)
      x += izod * 30;
    goto again_dms;
  }
  strcpy(s2, s);
  if ((iflag & BIT_ZODIAC) && (iflag & BIT_ROUND_MIN))
    sprintf(s, "%s%2d", s2, kmin);
  else
    sprintf(s, "%s%2d'", s2, kmin);
  if (iflag & BIT_ROUND_MIN)
    goto return_dms;
  x -= kmin;
  x *= 60;
  if (iflag & BIT_ROUND_SEC)
    x += 0.5;
  ksec = (long) x;
  if (ksec == 60) {
    x = (kdeg + (kmin + 1) / 60.0 + 0.1 / 3600.0) * sgn;
    if (iflag & BIT_ZODIAC)
      x += izod * 30;
    goto again_dms;
  }
  s[79] = '\0';
  strcpy(s2, s);
  sprintf(s, "%s%2d\"", s2, ksec);
  if (iflag & BIT_ROUND_SEC)
    goto return_dms;
  x -= ksec;
  k = (long) x * 10000;
  sprintf(s + strlen(s), ".%04d", k);
return_dms:
  if (sgn < 0) {
    sp = strpbrk(s, "0123456789"); 
    *(sp-1) = '-';
  }
  if (iflag & BIT_LZEROES) {
    while ((sp = strchr(s+2, ' ')) != NULL) *sp = '0';
  }
  return(s);
}

/* y00, y11, y2 are values for -2*dx, -dx, 0.
 * find zero points of parabola.
 * return: 0 if none
 * 	   1 if one zero in [-dx.. 0[
 * 	   1 if both zeros in [-dx.. 0[
 */
static int find_zero(double y00, double y11, double y2, double dx, 
			double *dxret, double *dxret2)
{
  double a, b, c, x1, x2;
  c = y11;
  b = (y2 - y00) / 2.0;
  a = (y2 + y00) / 2.0 - c;
  if (b * b - 4 * a * c < 0) 
    return 0;
  if (fabs(a) < 1e-100) return 0;
  x1 = (-b + sqrt(b * b - 4 * a * c)) / 2 / a;
  x2 = (-b - sqrt(b * b - 4 * a * c)) / 2 / a;
  // up to here the calcuation was made as if the x-values were -1, 0, 1.
  // This is why below they are shifted by -1
  if (x1 == x2) {
    *dxret = (x1 - 1) * dx;
    *dxret2 = (x1 - 1) * dx;
    return 1;
  }
  if (x1 >=0 && x1 < 1 && x2 >= 0 && x2 < 1) {
    if (x1 > x2) {	// two zeroes, order return values
      *dxret = (x2 - 1) * dx;
      *dxret2 = (x1 - 1) * dx;
    } else {
      *dxret = (x1 - 1) * dx;
      *dxret2 = (x2 - 1) * dx;
    }
    return 2;
  }
  if (x1 >=0 && x1 < 1) {
    *dxret = (x1 - 1) * dx;
    *dxret2 = (x2 - 1) * dx;	// set this value just in case, should not be used.
    return 1;
  } 
  if (x2 >=0 && x2 < 1) {
    *dxret = (x2 - 1) * dx;
    *dxret2 = (x1 - 1) * dx;
    return 1;
  }
  return 0;	// should not happen!
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

// stuff from old swevents.c
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
  double d12, d1, tt1, xa[6], xb[6];
  /*swe_revjul(tt0, 1, &jyear, &jmon, &jday, &jut);
  printf("%d%02d%02d %.2f: %c - %c %d\n", jyear, jmon, jday, jut, *spa, *spb, (int) dang);*/
  d1 = swe_degnorm(xta1 - xtb1 - dang);
  if (d1 > 180) d1 -= 360;
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
  printf("%s", s);
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
#define PATH_FOUTNAM   "."

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
    fprintf(stderr, "%s", s);
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
  UNUSED(xa1);
  UNUSED(xa1d);
  sprintf(foutnam, "%s/%s", PATH_FOUTNAM, FOUTNAM);
  if ((fpout = fopen(foutnam, BFILE_W_CREATE)) == NULL) {
    sprintf(serr, "could not open file %s", foutnam);
    return ERR;
  }
  fprintf(fpout, "%s, mundane aspects\ncreation date: %s\n%s\n", FOUTNAM, sdate, cmdline);
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

#define SWEASP_DAT_RECLEN  (5*sizeof(double) + 3 *sizeof(int32))
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
  UNUSED(tjd2);
  /* open aspects file */
  sprintf(foutnam, "%s/%s", PATH_FOUTNAM, FOUTNAM);
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
  fpos1 = fpos0;  // silence compiler
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
  if (retc == ERR) {
    fprintf(stderr, "%s", serr);
  }
  return retc;
}

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
    if (strlen(ODEGREE_STRING) > 1) {
      strcpy(s2, sp + strlen(ODEGREE_STRING));
    }
    strcpy(sp + 1, s2);
    *(sp + 3) = ':';
    *(sp + 8) = '\0';
  }
  return s;
}

