
/* 
  $Header: swetest.c,v 1.32 2000/07/21 10:02:05 dieter Exp $
  swetest.c	A test program
   
  Authors: Dieter Koch and Alois Treindl, Astrodienst Zürich

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
/* attention: Microsoft Compiler does not accept strings > 2048 char */
static char *infocmd0 = "\n\
  Swetest computes a complete set of geocentric planetary positions,\n\
  for a given date or a sequence of dates.\n\
  Input can either be a date or an absolute julian day number.\n\
  0:00 (midnight).\n\
  With the proper options, swetest can be used to output a printed\n\
  ephemeris and transfer the data into other programs like spreadsheets\n\
  for graphical display.\n\
  Version: $Header: swetest.c,v 1.32 2000/07/21 10:02:05 dieter Exp $\n\
\n";
static char *infocmd1 = "\n\
  Command line options:\n\
	-ay..   ayanamsa, with number of method, e.g. ay0 for Fagan/Bradley\n\
        -pSEQ   planet sequence to be computed.\n\
                See the letter coding below.\n\
        -nN     output data for N consecutive days; if no -n option\n\
                is given, the default is 1. If the option -n without a\n\
                number is given, the default is 20.\n\
        -sN     timestep N days, default 1. This option is only meaningful\n\
                when combined with option -n.\n\
        -edirPATH change the directory of the ephemeris files \n\
        -dX     differential ephemeris: print differential ephemeris between\n\
                body X and each body in list given by -p\n\
                example: -p2 -d0 -fJl -n366 -b1.1.1992 prints the longitude\n\
                distance between SUN (planet 0) and MERCURY (planet 2)\n\
                for a full year starting at 1 Jan 1992.\n\
	-DX	midpoint ephemeris, works the same way as the differential\n\
		mode -d described above, but outputs the midpoint position.\n\
        -hel    compute heliocentric positions\n\
        -bary   compute barycentric positions (bar. sun instead of node) \n\
	-sid..    sidereal, with number of method; 'sid0' for Fagan/Bradley\n\
	-sidt0..  sidereal, projection on ecliptic of t0 \n\
	-sidsp..  sidereal, projection on solar system plane \n\
        -topo[long,lat,elev]	\n\
		topocentric positions. The longitude, latitude (degrees with\n\
		decimal fraction) and elevation (meters) can be given, with\n\
		commas separated, + for east and north. If none are given,\n\
		Zürich is used: 8.55,47.38,400\n\
        -head   don\'t print the header before the planet data. This option\n\
                is useful when you want to paste the output into a\n\
                spreadsheet for displaying graphical ephemeris.\n\
        -bDATE  begin date; e.g. -b1.1.1992 if\n\
                Note: the date format is day month year (European style).\n\
        -bj...  begin date as an absolute Julian day number; e.g. -bj2415020.5\n\
        -j...   same as -bj\n";
static char *infocmd2 = "\
        -fSEQ   use SEQ as format sequence for the output columns;\n\
                default is PLBRuS.\n\
        -ut     input date is universal time\n\
	-uthh:mm:ss input time\n\
	-uthh.mmss input time\n\
        -thh.mmss  input time (ephemeris time)\n\
        -eswe   swiss ephemeris\n\
        -ejpl   jpl ephemeris (DE406), or with ephemeris file name\n\
                -ejplDE200.cdrom \n\
        -emos   moshier ephemeris\n\
        -true   true positions\n\
        -noaberr        no aberration\n\
        -nodefl no gravitational light deflection\n\
        -j2000  no precession (i.e. J2000 positions)\n\
        -nonut  no nutation \n\
        -speed  high precision speed \n\
        -speed3 'low' precision speed from 3 positions \n\
                do not use this option. -speed parameter is faster and preciser \n\
        -testaa96       test example in AA 96, B37,\n\
                        i.e. venus, j2450442.5, DE200.\n\
                        attention: use precession IAU1976\n\
                        and nutation 1980 (s. swephlib.h)\n\
        -testaa95\n\
        -testaa97\n\
        -roundsec       round to seconds\n\
        -roundmin       round to minutes\n\
\n\
        -?, -h  display whole info\n\
        -hcmd   display commands\n\
        -hplan  display planet numbers\n\
        -hform  display format characters\n\
        -hdate  display input date format\n\
        +head   header before every step (with -s..) \n\
        -gPPP   use PPP as gap between output columns; default is a single\n\
                blank.  -g followed by white space sets the\n\
                gap to the TAB character; which is useful for data entry\n\
                into spreadsheets.\n\
	-iXX	force iflag to value XX\n\
        -hexamp  display examples\n";
static char *infoplan = "\n\
  Planet selection letters:\n\
        d (default) main factors 0123456789mtABC\n\
        p main factors as above, plus main asteroids DEFGHI\n\
        h ficticious factors J..X\n\
        a all factors\n\
        (the letters above can only appear as a single letter)\n\n\
        0 Sun (character zero)\n\
        1 Moon (character 1)\n\
        2 Mercury\n\
        ....\n\
        9 Pluto\n\
        m mean lunar node\n\
        t true lunar node\n\
        e obliquity of ecliptic\n\
        n nutation\n\
        A mean lunar apogee (Lilith, Black Moon) \n\
        B osculating lunar apogee \n\
        C Earth \n\
        D Chiron\n\
        E Pholus\n\
        F Ceres \n\
        G Pallas \n\
        H Juno \n\
        I Vesta \n\
        J Cupido \n\
        K Hades \n\
        L Zeus \n\
        M Kronos \n\
        N Apollon \n\
        O Admetos \n\
        P Vulkanus \n\
        Q Poseidon \n\
        R Isis (Sevin) \n\
        S Nibiru (Sitchin) \n\
        T Harrington \n\
        U Leverrier's Neptune\n\
        V Adams' Neptune\n\
        W Lowell's Pluto\n\
        X Pickering's Pluto\n\n\
        f fixed star, with name or number given in -xf option\n\
        s minor planet, with MPC number given in -xs\n\
          \n";
static char *infoform = "\n\
  Output format SEQ letters:\n\
  In the standard setting five columns of coordinates are printed with\n\
  the default format PLBRuS. You can change the default by providing an\n\
  option like -fCCCC where CCCC is your sequence of columns.\n\
  The coding of the sequence is like this:\n\
        y year\n\
        Y year.fraction_of_year\n\
        p planet index\n\
        P planet name\n\
        J absolute juldate\n\
        T date formatted like 23.02.1992 \n\
        t date formatted like 920223 for 1992 february 23\n\
        L longitude in degree ddd°mm'ss\"\n\
        l longitude decimal\n\
        Z longitude ddsignmm'ss\"\n\
        S speed in longitude in degree ddd:mm:ss per day\n\
        SS speed for all values specified in fmt\n\
        s speed longitude decimal (degrees/day)\n\
        ss speed for all values specified in fmt\n\
        B latitude degree\n\
        b latitude decimal\n\
        R distance decimal in AU\n\
        r distance decimal in AU, Moon in seconds parallax\n\
          relative distance (1000=nearest, 0=furthest)\n\
        A Rectascension in hh:mm:ss\n\
        a rectascension hours decimal\n\
        D Declination degree\n\
        d declination decimal\n\
        X x-, y-, and z-coordinates ecliptical\n\
        x x-, y-, and z-coordinates equatorial\n\
        U unit vector ecliptical\n\
        u unit vector equatorial\n\
        Q l, b, r, dl, db, dr, a, d, da, dd\n\
	n mean ascending node (only Mercury - Uranus) decimal degree\n\
	N osculating ascending node for planets and asteroids\n\
	+ phase angle\n\
	- phase\n\
	* elongation\n\
	/ apparent diameter of disc\n\
	= magnitude\n";
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
\n\
        <RETURN>        repeat the last entry\n\
        \n\
        .               stop the program\n\
\n\
        +20             advance the date by 20 days\n\
\n\
        -10             go back in time 10 days\n";
static char *infoexamp = "\n\
\n\
  Examples:\n\
\n\
    swetest -p2 -b1.12.1900 -n15 -s2\n\
	ephemeris of Mercury (-p2) starting on 1 Dec 1900,\n\
	15 positions (-n15) in two-day steps (-s2)\n\
\n\
    swetest -p2 -b1.12.1900 -n15 -s2 -fTZ -roundsec -g, -head\n\
	same, but output format =  date and zodiacal position (-fTZ),\n\
	separated by comma (-g,) and rounded to seconds (-roundsec),\n\
	without header (-head).\n\
\n\
    swetest -ps -xs433 -b1.12.1900\n\
	position of asteroid 433 Eros (-ps -xs433)\n\
\n\
    swetest -p1 -d0 -b1.12.1900 -n10 -fPTl -head\n\
	angular distance of moon (-p1) from sun (-d0) for 10\n\
	consecutive days (-n10).\n";
/**************************************************************/

#include "swephexp.h" 	/* this includes  "sweodef.h" */

/*
 * programmers warning: It looks much worse than it is!
 * Originally swetest.c was a small and simple test program to test
 * the main functions of the Swiss Ephemeris and to demonstrate
 * its precision.
 * It compiles on Unix, on MSDOS and as a non-GUI utility on 16-bit
 * and 32-bit windows.
 * This portability has forced us into some clumsy constructs, which
 * end to hide the actual simplicity of the use of Swiss Ephemeris.
 * For example, the mechanism implemented here in swetest.c to find
 * the binary ephemeris files overrides the much simpler mechanism
 * inside the SwissEph library. This was necessary because we wanted
 * swetest.exe to run directly off the CDROM and search with some
 * intelligence for ephemeris files already installed on a system.
 */

#if MSDOS
#  include <direct.h>
#  include <dos.h>
#  ifdef _MSC_VER
#    include <sys\types.h>
#  endif
#  include <sys\stat.h>
#else
# ifdef MACOS
#  include <console.h>
# else
#  include <sys/stat.h>
# endif
#endif

#define J2000           2451545.0  /* 2000 January 1.5 */
#define square_sum(x)   (x[0]*x[0]+x[1]*x[1]+x[2]*x[2])
#define SEFLG_EPHMASK   (SEFLG_JPLEPH|SEFLG_SWIEPH|SEFLG_MOSEPH)

#define BIT_ROUND_SEC   1
#define BIT_ROUND_MIN   2
#define BIT_ZODIAC      4
#define PLSEL_D  "0123456789mtABC"
#define PLSEL_P  "0123456789mtABCDEFGHI"
#define PLSEL_H  "JKLMNOPQRSTUVWX"
#define PLSEL_A  "0123456789mtABCDEFGHIJKLMNOPQRSTUVWX"

#define DIFF_DIFF	'd'
#define DIFF_MIDP	'D'

static char se_pname[AS_MAXCH];
static char *zod_nam[] = {"ar", "ta", "ge", "cn", "le", "vi", 
                          "li", "sc", "sa", "cp", "aq", "pi"};

static char star[AS_MAXCH] = "algol";
static char sastno[AS_MAXCH] = "433";
static char *dms(double x, long iflag);
static int make_ephemeris_path(long iflag, char *argv0);
static int letter_to_ipl(int letter);
#if MSDOS
static int cut_str_any(char *s, char *cutlist, char *cpos[], int nmax);
#endif

int main(int argc, char *argv[])
{
  char spnam[AS_MAXCH], spnam2[AS_MAXCH], saves[AS_MAXCH]; 
  char s1[AS_MAXCH], s2[AS_MAXCH];
  char serr[AS_MAXCH], serr_save[AS_MAXCH], serr_warn[AS_MAXCH], sout[AS_MAXCH];
  char *sp, *spsave, *sp2, *psp;
  char *spno;
  char *fmt = "PLBRS";
  char *gap = " ";
  char *plsel = PLSEL_D;
#if HPUNIX
  char hostname[80];
#endif
  int i, j, n, iflag_f = -1;
  int jmon, jday, jyear;
  int line_count, line_limit = 32000;
  double jut = 0.0, y_frac, daya;
  double top_long = 8.55;	/* Zurich */
  double top_lat = 47.38;
  double top_elev = 400;
  double attr[20];
  int ipl, ipldiff = SE_SUN;
  long nstep = 1, istep;
  double x[6], x2[6], xequ[6], xcart[6], xcartq[6];
  double ar, sinp;
  char ephepath[AS_MAXCH];
  char fname[AS_MAXCH];
  char sdate[AS_MAXCH];
  char *begindate = NULL;
  long iflag = 0, iflag2;              /* external flag: helio, geo... */
  long iflgret;
  int whicheph = SEFLG_SWIEPH;   
  AS_BOOL universal_time = FALSE;
  AS_BOOL with_header = TRUE;
  AS_BOOL with_header_always = FALSE;
  AS_BOOL do_ayanamsa = FALSE;
  long sid_mode = SE_SIDM_FAGAN_BRADLEY;
  int gregflag = SE_GREG_CAL;
  int diff_mode = 0;
  long round_flag = 0;
  double tjd = 2415020.5;
  double t, te, t2, tstep = 1, thour = 0;
  double delt;
  serr[0] = serr_save[0] = serr_warn[0] = saves[0] = '\0';
# ifdef MACOS
  argc = ccommand(&argv); /* display the arguments window */    
# endif
  strcpy(ephepath, "");
  strcpy(fname, SE_FNAME_DE406);
  for (i = 1; i < argc; i++) {
    if (strncmp(argv[i], "-ut", 3) == 0) {
      universal_time = TRUE;
      if (strlen(argv[i]) > 3) {
	strcpy(s1, argv[i] + 3);
	if ((sp = strchr(s1, ':')) != NULL) {
	  *sp = '.';
	  if ((sp = strchr(s1, ':')) != NULL)
	    strcpy(sp, sp+1);
	}
        thour = atof(s1);
        /* h.mmss -> decimal */
        t =  fmod(thour, 1) * 100 + 1e-10;
        j = (int) t;
        t = fmod(t, 1) * 100 + 1e-10;
        thour = (int) thour + j / 60.0 + t / 3600.0;
      }
    } else if (strncmp(argv[i], "-head", 5) == 0) {
      with_header = FALSE;
    } else if (strncmp(argv[i], "+head", 5) == 0) {
      with_header_always = TRUE;
    } else if (strcmp(argv[i], "-j2000") == 0) {
      iflag |= SEFLG_J2000;
    } else if (strncmp(argv[i], "-ay", 3) == 0) {
      do_ayanamsa = TRUE;
      sid_mode = atol(argv[i]+3);
      swe_set_sid_mode(sid_mode, 0, 0);
    } else if (strncmp(argv[i], "-sidt0", 6) == 0) {
      iflag |= SEFLG_SIDEREAL;
      sid_mode = atol(argv[i]+6);
      if (sid_mode == 0)
	sid_mode = SE_SIDM_FAGAN_BRADLEY;
      sid_mode |= SE_SIDBIT_ECL_T0;
      swe_set_sid_mode(sid_mode, 0, 0);
    } else if (strncmp(argv[i], "-sidsp", 6) == 0) {
      iflag |= SEFLG_SIDEREAL;
      sid_mode = atol(argv[i]+6);
      if (sid_mode == 0)
	sid_mode = SE_SIDM_FAGAN_BRADLEY;
      sid_mode |= SE_SIDBIT_SSY_PLANE;
      swe_set_sid_mode(sid_mode, 0, 0);
    } else if (strncmp(argv[i], "-sid", 4) == 0) {
      iflag |= SEFLG_SIDEREAL;
      sid_mode = atol(argv[i]+4);
      if (sid_mode > 0)
	swe_set_sid_mode(sid_mode, 0, 0);
    } else if (strncmp(argv[i], "-j", 2) == 0) {
      begindate = argv[i] + 1;
    } else if (strncmp(argv[i], "-ejpl", 5) == 0) {
      whicheph = SEFLG_JPLEPH;
      if (*(argv[i]+5) != '\0')
        strcpy(fname, argv[i]+5);
    } else if (strncmp(argv[i], "-edir", 5) == 0) {
      if (*(argv[i]+5) != '\0')
        strcpy(ephepath, argv[i]+5);
    } else if (strcmp(argv[i], "-eswe") == 0) {
      whicheph = SEFLG_SWIEPH;
    } else if (strcmp(argv[i], "-emos") == 0) {
      whicheph = SEFLG_MOSEPH;
    } else if (strcmp(argv[i], "-hel") == 0) {
      iflag |= SEFLG_HELCTR;
    } else if (strcmp(argv[i], "-bary") == 0) {
      iflag |= SEFLG_BARYCTR;
    } else if (strncmp(argv[i], "-topo", 5) == 0) {
      iflag |= SEFLG_TOPOCTR;
      sscanf(argv[i] + 5, "%lf,%lf,%lf", &top_long, &top_lat, &top_elev);
    } else if (strcmp(argv[i], "-true") == 0) {
      iflag |= SEFLG_TRUEPOS;
    } else if (strcmp(argv[i], "-noaberr") == 0) {
      iflag |= SEFLG_NOABERR;
    } else if (strcmp(argv[i], "-nodefl") == 0) {
      iflag |= SEFLG_NOGDEFL;
    } else if (strcmp(argv[i], "-nonut") == 0) {
      iflag |= SEFLG_NONUT;
    } else if (strcmp(argv[i], "-speed3") == 0) {
      iflag |= SEFLG_SPEED3;
    } else if (strcmp(argv[i], "-speed") == 0) {
      iflag |= SEFLG_SPEED;
    } else if (strncmp(argv[i], "-testaa", 7) == 0) {
      whicheph = SEFLG_JPLEPH;
      strcpy(fname, SE_FNAME_DE200);
      if (strcmp(argv[i]+7, "95") == 0)
        begindate = "j2449975.5";
      if (strcmp(argv[i]+7, "96") == 0)
        begindate = "j2450442.5";
      if (strcmp(argv[i]+7, "97") == 0)
        begindate = "j2450482.5";
      fmt = "PADRu";
      universal_time = FALSE;
      plsel="3";
    } else if (strncmp(argv[i], "-p", 2) == 0) {
      spno = argv[i]+2;
      switch (*spno) {
      case 'd':
      case '\0':
      case ' ':  plsel = PLSEL_D; break;
      case 'p':  plsel = PLSEL_P; break;
      case 'h':  plsel = PLSEL_H; break;
      case 'a':  plsel = PLSEL_A; break;
      default:   plsel = spno;
      }
    } else if (strncmp(argv[i], "-xs", 3) == 0) {
      /* number of asteroid */
      strcpy(sastno, argv[i] + 3);
    } else if (strncmp(argv[i], "-xf", 3) == 0) {
      /* name or number of fixed star */
      strcpy(star, argv[i] + 3);
    } else if (strncmp(argv[i], "-x", 2) == 0) {
      /* name or number of fixed star */
      strcpy(star, argv[i] + 2);
    } else if (strncmp(argv[i], "-n", 2) == 0) {
      nstep = atoi(argv[i]+2);
      if (nstep == 0)
	nstep = 20;
    } else if (strncmp(argv[i], "-i", 2) == 0) {
      iflag_f = atoi(argv[i]+2);
      if (iflag_f & SEFLG_XYZ)
	fmt = "PX";
    } else if (strncmp(argv[i], "-s", 2) == 0) {
      tstep = atof(argv[i]+2);
    } else if (strncmp(argv[i], "-b", 2) == 0) {
      begindate = argv[i] + 2;
    } else if (strncmp(argv[i], "-f", 2) == 0) {
      fmt = argv[i] + 2;
    } else if (strncmp(argv[i], "-g", 2) == 0) {
      gap = argv[i] + 2;
      if (*gap == '\0') gap = "\t";
    } else if (strncmp(argv[i], "-d", 2) == 0
    || strncmp(argv[i], "-D", 2) == 0) {
      diff_mode = *(argv[i] + 1);	/* 'd' or 'D' */
      sp = argv[i]+2;
      ipldiff = letter_to_ipl((int) *sp);
      if (ipldiff <0) ipldiff = SE_SUN;
      swe_get_planet_name(ipldiff, spnam2);
    } else if (strcmp(argv[i], "-roundsec") == 0) {
      round_flag |= BIT_ROUND_SEC;
    } else if (strcmp(argv[i], "-roundmin") == 0) {
      round_flag |= BIT_ROUND_MIN;
    } else if (strncmp(argv[i], "-t", 2) == 0) {
      if (strlen(argv[i]) > 2) {
	strcpy(s1, argv[i] + 2);
	if ((sp = strchr(s1, ':')) != NULL) {
	  *sp = '.';
	  if ((sp = strchr(s1, ':')) != NULL)
	    strcpy(sp, sp+1);
	}
        thour = atof(s1);
        /* h.mmss -> decimal */
        t = (thour - (int) thour) * 100;
        j = (int) t;
        t = (t - (int) t) * 100;
        thour = (int) thour + j / 60.0 + t / 3600.0;
      }
    } else if (strncmp(argv[i], "-h", 2) == 0
      || strncmp(argv[i], "-?", 2) == 0) {
      sp = argv[i]+2;
      if (*sp == 'c' || *sp == '\0') {
        printf(infocmd0);
        printf(infocmd1);
        printf(infocmd2);
      }
      if (*sp == 'p' || *sp == '\0')
        printf(infoplan);
      if (*sp == 'f' || *sp == '\0')
        printf(infoform);
      if (*sp == 'd' || *sp == '\0')
        printf(infodate);
      if (*sp == 'e' || *sp == '\0')
        printf(infoexamp);
      goto end_main;
    } else {
      sprintf(sout, "illegal option %s\n", argv[i]);
      printf(sout);
      exit(1);
    }
  }
  swe_set_topo(top_long, top_lat, top_elev);
#if HPUNIX
  gethostname (hostname, 80);
  if (strstr(hostname, "as10") != NULL) 
    line_limit = 1000;
#endif
  if (with_header) {
    for (i = 0; i < argc; i++) {
      printf(argv[i]);
      printf(" ");
    }
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
  while (TRUE) {
    serr[0] = serr_save[0] = serr_warn[0] = '\0';
    if (begindate == NULL) {
      printf("\nDate ?");
      sdate[0] = '\0';
      gets(sdate);
    } else {
      strcpy(sdate, begindate);
      begindate = ".";  /* to exit afterwards */
    }
    if (strcmp(sdate, "-bary") == 0) {
      iflag = iflag & ~SEFLG_HELCTR;
      iflag |= SEFLG_BARYCTR;
      *sdate = '\0';
    } else if (strcmp(sdate, "-hel") == 0) {
      iflag = iflag & ~SEFLG_BARYCTR;
      iflag |= SEFLG_HELCTR;
      *sdate = '\0';
    } else if (strcmp(sdate, "-geo") == 0) {
      iflag = iflag & ~SEFLG_BARYCTR;
      iflag = iflag & ~SEFLG_HELCTR;
      *sdate = '\0';
    } else if (strcmp(sdate, "-ejpl") == 0) {
      iflag &= ~SEFLG_EPHMASK;
      iflag |= SEFLG_JPLEPH;
      *sdate = '\0';
    } else if (strcmp(sdate, "-eswe") == 0) {
      iflag &= ~SEFLG_EPHMASK;
      iflag |= SEFLG_SWIEPH;
      *sdate = '\0';
    } else if (strcmp(sdate, "-emos") == 0) {
      iflag &= ~SEFLG_EPHMASK;
      iflag |= SEFLG_MOSEPH;
      *sdate = '\0';
    } else if (strncmp(sdate, "-xs",3) == 0) {
      /* number of asteroid */
      strcpy(sastno, sdate + 3);
      *sdate = '\0';
    }
    sp = sdate;
    spsave = sp;
    if (*sp == '.') {
      goto end_main;
    } else if (*sp == '\0') {
      strcpy (sdate, saves);
    } else {
      strcpy (saves, sdate);
    }
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
      if ((long) jyear * 10000L + (long) jmon * 100L + (long) jday < 15821015L) 
        gregflag = SE_JUL_CAL;
      else
        gregflag = SE_GREG_CAL;
      if (strstr(sp, "jul") != NULL)
        gregflag = SE_JUL_CAL;
      else if (strstr(sp, "greg") != NULL)
        gregflag = SE_GREG_CAL;
      jut = 0;
      tjd = swe_julday(jyear,jmon,jday,jut,gregflag);        
      tjd += thour / 24;
    }
    line_count = 0;
    for (t = tjd, istep = 1; istep <= nstep; t += tstep, istep++) {
      if (t < 2299160.5)
        gregflag = SE_JUL_CAL;
      else
        gregflag = SE_GREG_CAL;
      if (strstr(spsave, "jul") != NULL)
        gregflag = SE_JUL_CAL;
      else if (strstr(spsave, "greg") != NULL)
        gregflag = SE_GREG_CAL;
      swe_revjul(t, gregflag, &jyear, &jmon, &jday, &jut);
      if (with_header) {
        sprintf(sout, "\ndate (dmy) %d.%d.%d", jday, jmon, jyear);
        printf(sout);
        if (gregflag)
          printf(" greg.");
        else
          printf(" jul.");
        t2 = t + 0.5;
        t2 = (t2 - (long int) t2) * 24;
        sprintf(sout, "  % 2d:", (int) t2); 
        printf(sout);
        t2 = (t2 - (long int) t2) * 60 + 0.5;
        sprintf(sout, "%02d", (int) t2); 
        printf(sout);
        if (universal_time)
          printf(" UT");
        else
          printf(" ET");
      }
      if (universal_time) {
        if (with_header) {
          sprintf(sout, "\nUT: %f", t);
          printf(sout);
        }
        delt = swe_deltat(t);
        if (with_header) {
          sprintf(sout, "     delta t: %f sec", delt * 86400.0);
          printf(sout);
        }
        te = t + delt;
      } else
        te = t;
      if (with_header) {
        sprintf(sout, "\nET: %f", te);
        printf(sout);
	if (iflag & SEFLG_SIDEREAL) {
	  double daya = swe_get_ayanamsa(te);
	  sprintf(sout, "   ayanamsa = %s", dms(daya, round_flag));
	  printf(sout);
	}
	if (iflag_f >=0)
	  iflag = iflag_f;
        iflgret = swe_calc(te, SE_ECL_NUT, iflag, x, serr);
        sprintf(sout, "\n%-15s %s", "Epsilon (true)", dms(x[0], round_flag)); 
        printf(sout);
        strcpy(s1, dms(x[2], round_flag)); 
        strcpy(s2, dms(x[3], round_flag));
        sprintf(sout, "\n%-15s %s%s%s", "Nutation", s1, gap, s2);
        printf(sout);
        printf("\n");
      }
      if (with_header && !with_header_always)
        with_header = FALSE;
      if (do_ayanamsa) {
	daya = swe_get_ayanamsa(te);
	printf("Ayanamsa%s%s\n", gap, dms(daya, round_flag));
	continue;
      }
      for (psp = plsel; *psp != '\0'; psp++) {
        ipl = letter_to_ipl((int) *psp);
        if (*psp == 'f')
          ipl = SE_FIXSTAR;
        else if (*psp == 's')
          ipl = atoi(sastno) + 10000;
	if (ipl == ERR) {
	  printf("illegal parameter -p%s\n", plsel);
	  exit(1);
	}
        if (iflag & SEFLG_HELCTR) {
          if (ipl == SE_SUN 
                || ipl == SE_MEAN_NODE || ipl == SE_TRUE_NODE 
                || ipl == SE_MEAN_APOG || ipl == SE_OSCU_APOG)
            continue;
        } else if (iflag & SEFLG_BARYCTR) {
          if (ipl == SE_MEAN_NODE || ipl == SE_TRUE_NODE 
                || ipl == SE_MEAN_APOG || ipl == SE_OSCU_APOG)
            continue;
        } else          /* geocentric */
          if (ipl == SE_EARTH)
            continue;
        /* ecliptic position */
	if (iflag_f >=0)
	  iflag = iflag_f;
        if (ipl == SE_FIXSTAR) {
          iflgret = swe_fixstar(star, te, iflag, x, serr);
          strcpy(se_pname, star);
        } else {
          iflgret = swe_calc(te, ipl, iflag, x, serr);
	  /* phase, magnitude, etc. */
	  if (iflgret != ERR && strpbrk(fmt, "+-*/=") != NULL)
	    iflgret = swe_pheno(te, ipl, iflag, attr, serr);
	  swe_get_planet_name(ipl, se_pname);
        }
	if (*psp == 'o') {/* ecliptic is wanted, remove nutation */
	  x[2] = x[3] = 0;
	  strcpy(se_pname, "Ecl. Obl.");
	}
	if (*psp == 'n') {/* nutation is wanted, remove ecliptic */
	  x[0] = x[2];
	  x[1] = x[3];
	  x[2] = x[3] = 0;
	  strcpy(se_pname, "Nutation");
        }
        if (iflgret < 0) { 
          if (strcmp(serr, serr_save) != 0 
            && (ipl == SE_SUN || ipl == SE_MOON
                || ipl == SE_MEAN_NODE || ipl == SE_TRUE_NODE
                || ipl == SE_CHIRON || ipl == SE_PHOLUS || ipl == SE_CUPIDO 
                || ipl >= SE_AST_OFFSET || ipl == SE_FIXSTAR)) {
            sprintf(sout, "error: %s\n", serr);
            printf(sout);
          }
          strcpy(serr_save, serr);
        } else if (*serr != '\0' && *serr_warn == '\0') {
          strcpy(serr_warn, serr);
        }
        if (diff_mode) {
          iflgret = swe_calc(te, ipldiff, iflag, x2, serr);
          if (iflgret < 0) { 
            sprintf(sout, "error: %s\n", serr);
            printf(sout);
          }
	  if (diff_mode == DIFF_DIFF) {
	    for (i = 1; i < 6; i++) 
	      x[i] -= x2[i];
	    if ((iflag & SEFLG_RADIANS) == 0) 
	      x[0] = swe_difdeg2n(x[0], x2[0]);
	    else
	      x[0] = swe_difrad2n(x[0], x2[0]);
	  } else {	/* DIFF_MIDP */
	    for (i = 1; i < 6; i++) 
	      x[i] = (x[i] + x2[i]) / 2;
	    if ((iflag & SEFLG_RADIANS) == 0) 
	      x[0] = swe_deg_midp(x[0], x2[0]);
	    else
	      x[0] = swe_rad_midp(x[0], x2[0]);
	  }
        }
        /* equator position */
        if (strpbrk(fmt, "aADdQ") != NULL) {
          iflag2 = iflag | SEFLG_EQUATORIAL;
          if (ipl == SE_FIXSTAR)
            iflgret = swe_fixstar(star, te, iflag2, xequ, serr);
          else
            iflgret = swe_calc(te, ipl, iflag2, xequ, serr);
          if (diff_mode) {
            iflgret = swe_calc(te, ipldiff, iflag2, x2, serr);
	    if (diff_mode == DIFF_DIFF) {
	      for (i = 1; i < 6; i++) 
		xequ[i] -= x2[i];
	      if ((iflag & SEFLG_RADIANS) == 0) 
		xequ[0] = swe_difdeg2n(xequ[0], x2[0]);
	      else
		xequ[0] = swe_difrad2n(xequ[0], x2[0]);
	    } else {	/* DIFF_MIDP */
	      for (i = 1; i < 6; i++) 
		xequ[i] = (xequ[i] + x2[i]) / 2;
	      if ((iflag & SEFLG_RADIANS) == 0) 
		xequ[0] = swe_deg_midp(xequ[0], x2[0]);
	      else
		xequ[0] = swe_rad_midp(xequ[0], x2[0]);
	    }
          }
        }
        /* ecliptic cartesian position */
        if (strpbrk(fmt, "XU") != NULL) {
          iflag2 = iflag | SEFLG_XYZ;
          if (ipl == SE_FIXSTAR)
            iflgret = swe_fixstar(star, te, iflag2, xcart, serr);
          else
            iflgret = swe_calc(te, ipl, iflag2, xcart, serr);
          if (diff_mode) {
            iflgret = swe_calc(te, ipldiff, iflag2, x2, serr);
	    if (diff_mode == DIFF_DIFF) {
	      for (i = 0; i < 6; i++) 
		xcart[i] -= x2[i];
	    } else {
		xcart[i] = (xcart[i] + x2[i]) / 2;
	    }
          }
        }
        /* equator cartesian position */
        if (strpbrk(fmt, "xu") != NULL) {
          iflag2 = iflag | SEFLG_XYZ | SEFLG_EQUATORIAL;
          if (ipl == SE_FIXSTAR)
            iflgret = swe_fixstar(star, te, iflag2, xcartq, serr);
          else
            iflgret = swe_calc(te, ipl, iflag2, xcartq, serr);
          if (diff_mode) {
            iflgret = swe_calc(te, ipldiff, iflag2, x2, serr);
	    if (diff_mode == DIFF_DIFF) {
	      for (i = 0; i < 6; i++) 
		xcartq[i] -= x2[i];
	    } else {
		xcartq[i] = (xcart[i] + x2[i]) / 2;
	    }
          }
        }
        strcpy(spnam, se_pname);
        /*
         * The string fmt contains a sequence of format specifiers;
         * each character in fmt creates a column, the columns are
         * sparated by the gap string.
         */
        for (sp = fmt; *sp != '\0'; sp++) {
          if (sp != fmt) 
            printf(gap);
          switch(*sp) {
          case 'y':
              sprintf(sout, "%d", jyear);
              printf(sout);
              break;
          case 'Y':
              jut = 0;
              t2 = swe_julday(jyear,1,1,jut,gregflag);
              y_frac = (t - t2) / 365.0;
              sprintf(sout, "%.2f", jyear + y_frac);
              printf(sout);
              break;
          case 'p':
              if (diff_mode == DIFF_DIFF)
                sprintf(sout, "%d-%d", ipl, ipldiff);
              else if (diff_mode == DIFF_MIDP)
                sprintf(sout, "%d/%d", ipl, ipldiff);
              else
                sprintf(sout, "%d", ipl);
              printf(sout);
              break;
          case 'P':
              if (diff_mode == DIFF_DIFF)
                sprintf(sout, "%.3s-%.3s", spnam, spnam2);
              else if (diff_mode == DIFF_MIDP)
                sprintf(sout, "%.3s/%.3s", spnam, spnam2);
              else
                sprintf(sout, "%-15s", spnam);
              printf(sout);
              break;
          case 'J':
          case 'j':
              sprintf(sout, "%.2f", t);
              printf(sout);
              break;
          case 'T':
              sprintf(sout, "%02d.%02d.%d", jday, jmon, jyear);
	      if (jut != 0) {
		int h, m, s;
                s = (int) (jut * 3600 + 0.5);
		h = (int) (s / 3600.0);
		m = (int) ((s % 3600) / 60.0);
		s %= 60;
		sprintf(sout + strlen(sout), " %d:%02d:%02d", h, m, s);
		if (universal_time)
                  strcat(sout, " UT");
		else
                  strcat(sout, " ET");
	      }
              printf(sout);
              break;
          case 't':
              sprintf(sout, "%02d%02d%02d", jyear % 100, jmon, jday);
              printf(sout);
              break;
          case 'L':
              printf(dms(x[0], round_flag));
              break;
          case 'l':
              sprintf(sout, "%# 11.7f", x[0]);
              printf(sout);
              break;
          case 'Z':
              printf(dms(x[0], round_flag|BIT_ZODIAC));
              break;
          case 'S':
          case 's':
              if (*(sp+1) == 'S' || *(sp+1) == 's' || strpbrk(fmt, "XUxu") != NULL) {
                for (sp2 = fmt; *sp2 != '\0'; sp2++) {
                  if (sp2 != fmt) 
                    printf(gap);
                  switch(*sp2) {
                    case 'L':   /* speed! */
                    case 'Z':   /* speed! */
                      printf(dms(x[3], round_flag));
                      break;
                    case 'l':   /* speed! */
                      sprintf(sout, "%11.7f", x[3]);
                      printf(sout);
                      break;
                    case 'B':   /* speed! */
                      printf(dms(x[4], round_flag));
                      break;
                    case 'b':   /* speed! */
                      sprintf(sout, "%11.7f", x[4]);
                      printf(sout);
                      break;
                    case 'A':   /* speed! */
                      printf(dms(xequ[3]/15, round_flag|SEFLG_EQUATORIAL));
                      break;
                    case 'a':   /* speed! */
                      sprintf(sout, "%11.7f", xequ[3]);
                      printf(sout);
                      break;
                    case 'D':   /* speed! */
                      printf(dms(xequ[4], round_flag));
                      break;
                    case 'd':   /* speed! */
                      sprintf(sout, "%11.7f", xequ[4]);
                      printf(sout);
                      break;
                    case 'R':   /* speed! */
                    case 'r':   /* speed! */
                      sprintf(sout, "%# 14.9f", x[5]);
                      printf(sout);
                      break;
                    case 'U':   /* speed! */
                    case 'X':   /* speed! */
                      if (*sp =='U') 
                        ar = sqrt(square_sum(xcart));
                      else 
                        ar = 1;
                      sprintf(sout, "%# 14.9f%s", xcart[3]/ar, gap);
                      printf(sout);
                      sprintf(sout, "%# 14.9f%s", xcart[4]/ar, gap);
                      printf(sout);
                      sprintf(sout, "%# 14.9f", xcart[5]/ar);
                      printf(sout);
                      break;
                    case 'u':   /* speed! */
                    case 'x':   /* speed! */
                      if (*sp =='u') 
                        ar = sqrt(square_sum(xcartq));
                      else 
                        ar = 1;
                      sprintf(sout, "%# 14.9f%s", xcartq[3]/ar, gap);
                      printf(sout);
                      sprintf(sout, "%# 14.9f%s", xcartq[4]/ar, gap);
                      printf(sout);
                      sprintf(sout, "%# 14.9f", xcartq[5]/ar);
                      printf(sout);
                      break;
                    default:
                      break;
                  }
                }
                if (*(sp+1) == 'S' || *(sp+1) == 's')
                  sp++;
              } else if (*sp == 'S') {
                printf(dms(x[3], round_flag));
              } else {
		printf("%# 11.7f", x[3]);
	      }
              break;
          case 'B':
              printf(dms(x[1], round_flag));
              break;
          case 'b':
              sprintf(sout, "%# 11.7f", x[1]);
              printf(sout);
              break;
          case 'A':     /* rectascensio */
              printf(dms(xequ[0]/15, round_flag|SEFLG_EQUATORIAL));
              break;
          case 'a':     /* rectascensio */
              sprintf(sout, "%# 11.7f", xequ[0]);
              printf(sout);
              break;
          case 'D':     /* declination */
              printf(dms(xequ[1], round_flag));
              break;
          case 'd':     /* declination */
              sprintf(sout, "%# 11.7f", xequ[1]);
              printf(sout);
              break;
          case 'R':
              sprintf(sout, "%# 14.9f", x[2]);
              printf(sout);
              break;
          case 'r':
              if ( ipl == SE_MOON ) { /* for moon print parallax */
                sinp = 8.794 / x[2];    /* in seconds of arc */
                ar = sinp * (1 + sinp * sinp * 3.917402e-12);
                /* the factor is 1 / (3600^2 * (180/pi)^2 * 6) */
                sprintf(sout, "%# 13.5f\"", ar);        
              } else {
                sprintf(sout, "%# 14.9f", x[2]);
              }
              printf(sout);
              break;
          case 'U':
          case 'X':
              if (*sp =='U') 
                ar = sqrt(square_sum(xcart));
              else 
                ar = 1;
              sprintf(sout, "%# 14.9f%s", xcart[0]/ar, gap);
              printf(sout);
              sprintf(sout, "%# 14.9f%s", xcart[1]/ar, gap);
              printf(sout);
              sprintf(sout, "%# 14.9f", xcart[2]/ar);
              printf(sout);
              break;
          case 'u':
          case 'x':
              if (*sp =='u') 
                ar = sqrt(square_sum(xcartq));
              else 
                ar = 1;
              sprintf(sout, "%# 14.9f%s", xcartq[0]/ar, gap);
              printf(sout);
              sprintf(sout, "%# 14.9f%s", xcartq[1]/ar, gap);
              printf(sout);
              sprintf(sout, "%# 14.9f", xcartq[2]/ar);
              printf(sout);
              break;
          case 'Q':
              sprintf(sout, "%-15s", spnam);
              printf(sout);
              printf(dms(x[0], round_flag));
              printf(dms(x[1], round_flag));
              sprintf(sout, "  %# 14.9f", x[2]);
              printf(sout);
              printf(dms(x[3], round_flag));
              printf(dms(x[4], round_flag));
              sprintf(sout, "  %# 14.9f\n", x[5]);
              printf(sout);
              sprintf(sout, "               %s", dms(xequ[0], round_flag));
              printf(sout);
              printf(dms(xequ[1], round_flag));
              sprintf(sout, "                %s", dms(xequ[3], round_flag));
              printf(sout);
              printf(dms(xequ[4], round_flag));
              break;
	  case 'N': 
	  case 'n': {
	      double xasc[6];
	      int imeth = (*sp == 'n')?SE_NODBIT_MEAN:SE_NODBIT_OSCU;
	      iflgret = swe_nod_aps(te, ipl, iflag, imeth, xasc, NULL, NULL, NULL, serr);
	      if (iflgret >= 0 && (ipl <= SE_NEPTUNE || *sp == 'N') ) 
		printf("%# 11.7f", xasc[0]);
	      };
	      break;
          case '+':
	      printf(dms(attr[0], round_flag));
              break;
          case '-':
              printf("  %# 14.9f", attr[1]);
              break;
          case '*':
	      printf(dms(attr[2], round_flag));
              break;
          case '/':
	      printf(dms(attr[3], round_flag));
              break;
          case '=':
	      printf("  %# 6.1fm", attr[4]);
              break;
          }     /* switch */
        }       /* for sp */
        printf("\n");
        line_count++;
        if (line_count >= line_limit) {
          sprintf(sout, "****** line count %d was exceeded\n", line_limit);
          printf(sout);
          break;
        }
      }         /* for psp */
      if (line_count >= line_limit) 
        break;
    }           /* for tjd */
    if (*serr_warn != '\0') {
      printf("\nwarning: ");
      printf(serr_warn);
      printf("\n");
    }
  }             /* while 1 */
  /* close open files and free allocated space */
  end_main:
  swe_close();
  return  OK;
}

static char *dms(double x, long iflag)
{
  int izod;
  long k, kdeg, kmin, ksec;
  char c = (unsigned char) ODEGREE_CHAR;
  char *sp, s1[50];
  static char s[50];
  int sgn;
  *s = '\0';
  if (iflag & SEFLG_EQUATORIAL)
    c = 'h';
  if (x < 0) {
    x = -x;
    sgn = -1;
  } else 
    sgn = 1;
  if (iflag & BIT_ROUND_MIN)
    x += 0.5/60;
  if (iflag & BIT_ROUND_SEC)
    x += 0.5/3600;
  if (iflag & BIT_ZODIAC) {
    izod = (int) (x / 30); 
    x = fmod(x, 30);
    kdeg = (long) x;
    sprintf(s, "%2ld %s ", kdeg, zod_nam[izod]);
  } else {
    kdeg = (long) x;
    sprintf(s, " %3ld%c", kdeg, c);
  }
  x -= kdeg;
  x *= 60;
  kmin = (long) x;
  if ((iflag & BIT_ZODIAC) && (iflag & BIT_ROUND_MIN))
    sprintf(s1, "%2ld", kmin);
  else
    sprintf(s1, "%2ld'", kmin);
  strcat(s, s1);
  if (iflag & BIT_ROUND_MIN)
    goto return_dms;
  x -= kmin;
  x *= 60;
  ksec = (long) x;
  if (iflag & BIT_ROUND_SEC)
    sprintf(s1, "%2ld\"", ksec);
  else
    sprintf(s1, "%2ld", ksec);
  strcat(s, s1);
  if (iflag & BIT_ROUND_SEC)
    goto return_dms;
  x -= ksec;
  k = (long) (x * 10000);
  sprintf(s1, ".%04ld", k);
  strcat(s, s1);
  return_dms:;
  if (sgn < 0) {
    sp = strpbrk(s, "0123456789"); 
    *(sp-1) = '-';
  }
  return(s);
}

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
  size_t pathlen; 
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
    printf("error in getcwd()\n");
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

#if MSDOS
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
#endif
