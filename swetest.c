/* 
  $Header: /users/dieter/sweph/RCS/swetest.c,v 1.66 2004/09/07 18:53:47 dieter Exp $
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
  Version: $Header: /users/dieter/sweph/RCS/swetest.c,v 1.66 2004/09/07 18:53:47 dieter Exp $\n\
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
        -bary   compute barycentric positions (bar. earth instead of node) \n\
	-sid..    sidereal, with number of method; 'sid0' for Fagan/Bradley\n\
	-sidt0..  sidereal, projection on ecliptic of t0 \n\
	-sidsp..  sidereal, projection on solar system plane \n\
        -topo[long,lat,elev]	\n\
		topocentric positions. The longitude, latitude (degrees with\n\
		DECIMAL fraction) and elevation (meters) can be given, with\n\
		commas separated, + for east and north. If none are given,\n\
		Zürich is used: 8.55,47.38,400\n\
        -house[long,lat,hsys]	\n\
		include house cusps. The longitude, latitude (degrees with\n\
		DECIMAL fraction) and house system letter can be given, with\n\
		commas separated, + for east and north. If none are given,\n\
		Greenwich UK and Placidus is used: 0.00,51.50,p.\n\
		The output lists 12 house cusps, Asc, MC, ARMC and Vertex.\n\
		Houses can only be computed if option -ut is given.\n";
static char *infocmd2 = "\
        -hsy[hsys]	\n\
		house system to be used (for house positions of planets)\n\
		for long, lat, hsys, see -house\n\
        -geopos[long,lat,elev]	\n\
		Geographic position. Can be used for azimuth and altitude\n\
                or topocentric or house cups calculations.\n\
                The longitude, latitude (degrees with DECIMAL fraction)\n\
		and elevation (meters) can be given, with\n\
		commas separated, + for east and north. If none are given,\n\
		Zürich is used: 8.55,47.38,400\n\
        -head   don\'t print the header before the planet data. This option\n\
                is useful when you want to paste the output into a\n\
                spreadsheet for displaying graphical ephemeris.\n\
        -bDATE  begin date; e.g. -b1.1.1992 if\n\
                Note: the date format is day month year (European style).\n\
        -bj...  begin date as an absolute Julian day number; e.g. -bj2415020.5\n\
        -j...   same as -bj\n\
        -fSEQ   use SEQ as format sequence for the output columns;\n\
                default is PLBRS.\n\
        -tHH.MMSS  input time (ephemeris time)\n\
        -ut     input date is universal time\n\
	-utHH:MM:SS input time\n\
	-utHH.MMSS input time\n";
static char *infocmd3 = "\
        -eswe   swiss ephemeris\n\
        -ejpl   jpl ephemeris (DE406), or with ephemeris file name\n\
                -ejplde200.eph \n\
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
static char *infocmd4 = "\
        -solecl solar eclipse\n\
                output 1st line:\n\
                  eclipse date,\n\
                  time of maximum,\n\
                  core shadow width (negative with total eclipses),\n\
                  fraction of solar diameter that is eclipsed\n\
                output 2nd line:\n\
                  start and end times for partial and total phase\n\
                output 3rd line:\n\
                  geographical longitude and latitude of maximum eclipse,\n\
                  totality duration at that geographical position,\n\
                output with -local, see below.\n\
        -occult occultation of planet or star by the moon. Use -p to \n\
                specify planet (-pf -xfAldebaran for stars) \n\
                output format same as with -solecl\n\
        -lunecl lunar eclipse\n\
                output 1st line:\n\
                  eclipse date,\n\
                  time of maximum,\n\
                output 2nd line:\n\
                  6 contacts for start and end of penumbral, partial, and\n\
                  total phase\n\
        -local  only with -solecl or -occult, if the next event of this\n\
                kind is wanted for a given geogr. position.\n\
                Use -geopos[long,lat,elev] to specify that position.\n\
                If -local is not set, the program \n\
                searches for the next event anywhere on earth.\n\
                output 1st line:\n\
                  eclipse date,\n\
                  time of maximum,\n\
                  fraction of solar diameter that is eclipsed\n\
                output 2nd line:\n\
                  local eclipse duration,\n\
                  local four contacts,\n";
static char *infocmd5 = "\
        -total  total eclipse (only with -solecl, -lunecl)\n\
        -partial partial eclipse (only with -solecl, -lunecl)\n\
        -annular annular eclipse (only with -solecl)\n\
        -anntot annular-total eclipse (only with -solecl)\n\
        -penumbral penumbral lunar eclipse (only with -lunecl)\n\
        -central central eclipse (only with -solecl, nonlocal)\n\
        -noncentral non-central eclipse (only with -solecl, nonlocal)\n\
        -rise   rising and setting of a planet or star.\n\
                Use -geopos[long,lat,elev] to specify geographical position.\n\
        -metr   southern and northern meridian transit of a planet of star\n\
                Use -geopos[long,lat,elev] to specify geographical position.\n\
        -bwd    search backward\n";
/* characters still available:
  bcegijklruvxy
 */
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
        n nutation\n\
        o obliquity of ecliptic\n\
	q delta t\n\
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
        X Pickering's Pluto\n\
        Y Vulcan\n\
        Z White Moon\n\
	w Waldemath's dark Moon\n\
        f fixed star, with name or number given in -xf option\n\
        z hypothetical body, with number given in -xz\n\
        s minor planet, with MPC number given in -xs\n\
        e print a line of labels\n\
          \n";
/* characters still available 
   cegjmov
*/
static char *infoform = "\n\
  Output format SEQ letters:\n\
  In the standard setting five columns of coordinates are printed with\n\
  the default format PLBRS. You can change the default by providing an\n\
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
        I Azimuth degree\n\
        i Azimuth decimal\n\
        H Height degree\n\
        h Height decimal\n\
        K Height (with refraction) degree\n\
        k Height (with refraction) decimal\n\
        G house position in degrees\n\
        g house position in degrees decimal\n\
        j house number 1.0 - 12.99999\n\
        X x-, y-, and z-coordinates ecliptical\n\
        x x-, y-, and z-coordinates equatorial\n\
        U unit vector ecliptical\n\
        u unit vector equatorial\n\
        Q l, b, r, dl, db, dr, a, d, da, dd\n\
	n mean values: ascending, descending node (Me - Ne) decimal degree\n\
	N osculating values: ascending, descending node\n\
	f mean values for longitude: perihel, aphel, second focal point\n\
	F oscul. values for longitude: perihel, aphel, second focal point\n\
	+ phase angle\n\
	- phase\n\
	* elongation\n\
	/ apparent diameter of disc (without refraction)\n\
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
    swetest -pf -xfAldebaran -b1.1.2000\n\
	position of fixed star Aldebaran \n\
\n\
    swetest -p1 -d0 -b1.12.1900 -n10 -fPTl -head\n\
	angular distance of moon (-p1) from sun (-d0) for 10\n\
	consecutive days (-n10).\n\
\n\
    swetest -p6 -DD -b1.12.1900 -n100 -s5 -fPTZ -head -roundmin\n\
      Midpoints between Saturn (-p6) and Chiron (-DD) for 100\n\
      consecutive steps (-n100) with 5-day steps (-s5) with\n\
      longitude in degree-sign format (-f..Z) rounded to minutes (-roundmin)\n\
\n\
    swetest -b5.1.2002 -p -house12.05,49.50,k -ut12:30\n\
	Koch houses for a location in Germany at a given date and time\n";
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
#define PLSEL_H  "JKLMNOPQRSTUVWXYZw"
#define PLSEL_A  "0123456789mtABCDEFGHIJKLMNOPQRSTUVWXYZw"

#define DIFF_DIFF	'd'
#define DIFF_MIDP	'D'
#define MODE_HOUSE	1
#define MODE_LABEL	2

static char se_pname[AS_MAXCH];
static char *zod_nam[] = {"ar", "ta", "ge", "cn", "le", "vi", 
                          "li", "sc", "sa", "cp", "aq", "pi"};

static char star[AS_MAXCH] = "algol", star2[AS_MAXCH];
static char sastno[AS_MAXCH] = "433";
static char shyp[AS_MAXCH] = "1";
static char *dms(double x, int32 iflag);
static int make_ephemeris_path(int32 iflag, char *argv0);
static int letter_to_ipl(int letter);
static int print_line(int mode);
static int do_eclipse_etc();
static char *hms_from_tjd(double x);
static void do_printf(char *info);
static char *hms(double x, int32 iflag);
#if MSDOS
static int cut_str_any(char *s, char *cutlist, char *cpos[], int nmax);
#endif

/* globals shared between main() and print_line() */
static char *fmt = "PLBRS";
static char *gap = " ";
static double t, te, tut, jut = 0;
static int jmon, jday, jyear;
static int ipl = SE_SUN, ipldiff = SE_SUN, nhouses = 12;
static char spnam[AS_MAXCH], spnam2[AS_MAXCH], serr[AS_MAXCH];
static char serr_save[AS_MAXCH], serr_warn[AS_MAXCH];
static int gregflag = SE_GREG_CAL;
static int diff_mode = 0;
static AS_BOOL universal_time = FALSE;
static int32 round_flag = 0;
static double x[6], x2[6], xequ[6], xcart[6], xcartq[6], xobl[6], xaz[6], xt[6], geopos[3], hpos, hpos2, hposj, armc, xsv[6];
static int hpos_meth = 0;
static double attr[20], tret[20];
static int32 iflag = 0, iflag2;              /* external flag: helio, geo... */
static char *hs_nam[] = {"undef",
	"Ascendant", "MC", "ARMC", "Vertex"};
static int smod = 0;
static int direction = 1;
static AS_BOOL direction_flag = FALSE;
static double tjd = 2415020.5;
static int32 nstep = 1, istep;
static int32 search_flag = 0;
static char sout[AS_MAXCH];
static int32 whicheph = SEFLG_SWIEPH;   
static char *psp;

# define SMOD_LUNAR     1
# define SMOD_HOW       2       /* an option for Lunar */
# define SMOD_SOLAR     4
# define SMOD_LOCAL     8       /* an option for Solar */
# define SMOD_TOTAL     16
# define SMOD_ANNULAR   32      /* count as penumbral for Lunar */
# define SMOD_ANNTOT    64
# define SMOD_PARTIAL   128
# define SMOD_PENUMBRAL 256
# define SMOD_ALL       (SMOD_TOTAL| SMOD_ANNULAR|SMOD_PARTIAL|SMOD_ANNTOT)
# define SMOD_OCCULT    512
# define SMOD_RISE      1024
# define SMOD_METR      2048
# define SMOD_HOCAL     4096

# define ECL_LUN_PENUMBRAL       1       /* eclipse types for hocal list */
# define ECL_LUN_PARTIAL        2
# define ECL_LUN_TOTAL          3
# define ECL_SOL_PARTIAL        4
# define ECL_SOL_ANNULAR        5
# define ECL_SOL_TOTAL          6

int main(int argc, char *argv[])
{
  char saves[AS_MAXCH]; 
  char s1[AS_MAXCH], s2[AS_MAXCH];
  char *sp, *spsave, *sp2;
  char *spno;
  char *plsel = PLSEL_D;
#if HPUNIX
  char hostname[80];
#endif
  int i, j, n, iflag_f = -1, iflgt;
  int line_count, line_limit = 32000;
  double daya;
  double top_long = 8.55;	/* Zurich */
  double top_lat = 47.38;
  double top_elev = 400;
  double hous_long = 0;	/* Greenwich, UK */
  double hous_lat = 51.5;
  int ihsy = 'p';
  AS_BOOL do_houses = FALSE;
  char ephepath[AS_MAXCH];
  char fname[AS_MAXCH];
  char sdate[AS_MAXCH];
  char *begindate = NULL;
  int32 iflgret;
  AS_BOOL with_header = TRUE;
  AS_BOOL with_header_always = FALSE;
  AS_BOOL do_ayanamsa = FALSE;
  int32 sid_mode = SE_SIDM_FAGAN_BRADLEY;
  double t2, tstep = 1, thour = 0;
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
    } else if (strncmp(argv[i], "-house", 6) == 0) {
      *sout = '\0';
      sscanf(argv[i] + 6, "%lf,%lf,%s", &hous_long, &hous_lat, sout);
      top_long = hous_long;
      top_lat = hous_lat;
      if (*sout) ihsy = sout[0];
      do_houses = TRUE;
    } else if (strncmp(argv[i], "-hsy", 4) == 0) {
      ihsy = *(argv[i] + 4);
      if (ihsy == '\0') ihsy = 'p';
      if (strlen(argv[i]) > 5)
	hpos_meth = atoi(argv[i] + 5);
    } else if (strncmp(argv[i], "-topo", 5) == 0) {
      iflag |= SEFLG_TOPOCTR;
      sscanf(argv[i] + 5, "%lf,%lf,%lf", &top_long, &top_lat, &top_elev);
    } else if (strncmp(argv[i], "-geopos", 7) == 0) {
      sscanf(argv[i] + 7, "%lf,%lf,%lf", &top_long, &top_lat, &top_elev);
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
    } else if (strcmp(argv[i], "-lunecl") == 0) {
      smod |= SMOD_LUNAR;
      smod &= ~SMOD_SOLAR;
    } else if (strcmp(argv[i], "-solecl") == 0) {
      smod |= SMOD_SOLAR;
      search_flag |= SE_ECL_CENTRAL;
      search_flag |= SE_ECL_NONCENTRAL;
    } else if (strcmp(argv[i], "-occult") == 0) {
      smod |= SMOD_OCCULT;
      smod &= ~SMOD_SOLAR;
      smod &= ~SMOD_LUNAR;
      search_flag |= SE_ECL_CENTRAL;
      search_flag |= SE_ECL_NONCENTRAL;
    } else if (strcmp(argv[i], "-hocal") == 0) {
      /* used to create a listing for inclusion in hocal.c source code */
      smod |= SMOD_HOCAL;
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
    } else if (strcmp(argv[i], "-penumbral") == 0) {
      smod |= SMOD_PENUMBRAL;
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
    } else if (strcmp(argv[i], "-bwd") == 0) {
      direction = -1;
      direction_flag = TRUE;
    } else if (strncmp(argv[i], "-p", 2) == 0) {
      spno = argv[i]+2;
      switch (*spno) {
      case 'd':
      /*
      case '\0':
      case ' ':  
      */
	plsel = PLSEL_D; break;
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
    } else if (strncmp(argv[i], "-xz", 3) == 0) {
      /* number of hypothetical body */
      strcpy(shyp, argv[i] + 3);
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
        t =  fmod(thour, 1) * 100 + 1e-10;
        j = (int) t;
        t = fmod(t, 1) * 100 + 1e-10;
        thour = (int) thour + j / 60.0 + t / 3600.0;
      }
    } else if (strncmp(argv[i], "-h", 2) == 0
      || strncmp(argv[i], "-?", 2) == 0) {
      sp = argv[i]+2;
      if (*sp == 'c' || *sp == '\0') {
        printf(infocmd0);
        printf(infocmd1);
        printf(infocmd2);
        printf(infocmd3);
        printf(infocmd4);
        printf(infocmd5);
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
  if (smod & (SMOD_OCCULT | SMOD_RISE | SMOD_METR)) {
    ipl = letter_to_ipl(*plsel);
    if (*plsel == 'f') 
      ipl = SE_FIXSTAR;
    else
      *star = '\0';
    if ((smod & SMOD_OCCULT) && ipl == 1)
      ipl = 2; /* no occultation of moon by moon */
  }
  geopos[0] = top_long;
  geopos[1] = top_lat;
  geopos[2] = top_elev;
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
      fgets(sdate, AS_MAXCH, stdin);
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
      if ((int32) jyear * 10000L + (int32) jmon * 100L + (int32) jday < 15821015L) 
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
    if (smod > 0) {
      do_eclipse_etc();
      swe_close();
      return OK;
    }
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
	t2 += 0.5 / 86400; /* round to second */
        t2 = (t2 - (int32) t2) * 24;
        sprintf(sout, "  % 2d:", (int) t2); 
        printf(sout);
        t2 = (t2 - (int32) t2) * 60;
        sprintf(sout, "%02d:", (int) t2); 
        printf(sout);
        t2 = (t2 - (int32) t2) * 60;
        sprintf(sout, "%02d", (int) t2); 
        printf(sout);
        if (universal_time)
          printf(" UT");
        else
          printf(" ET");
      }
      delt = swe_deltat(t);
      if (universal_time) {
        if (with_header) {
          sprintf(sout, "\nUT: %f", t);
          printf(sout);
        }
        if (with_header) {
          sprintf(sout, "     delta t: %f sec", delt * 86400.0);
          printf(sout);
        }
        te = t + delt;
        tut = t;
      } else {
        te = t;
        tut = t - delt;
      }
      iflgret = swe_calc(te, SE_ECL_NUT, iflag, xobl, serr);
      if (with_header) {
        sprintf(sout, "\nET: %f", te);
        printf(sout);
	if (iflag & SEFLG_SIDEREAL) {
	  daya = swe_get_ayanamsa(te);
	  sprintf(sout, "   ayanamsa = %s", dms(daya, round_flag));
	  printf(sout);
	}
	if (iflag_f >=0)
	  iflag = iflag_f;
	if (strchr(plsel, 'o') == NULL) {
	  sprintf(sout, "\n%-15s %s", "Epsilon (true)", dms(xobl[0],round_flag)); 
	  printf(sout);
	}
	if (strchr(plsel, 'n') == NULL) {
	  strcpy(s1, dms(xobl[2], round_flag)); 
	  strcpy(s2, dms(xobl[3], round_flag));
	  sprintf(sout, "\n%-15s %s%s%s", "Nutation", s1, gap, s2);
	  printf(sout);
	}
        printf("\n");
	if (do_houses) {
	  if (!universal_time) {
	    do_houses = FALSE;
	    printf("option -house requires option -ut for Universal Time\n");
	  } else {
	  strcpy(s1, dms(hous_long, round_flag)); 
	  strcpy(s2, dms(hous_lat, round_flag));
	  sprintf(sout, "Houses system %c for long=%s, lat=%s\n", ihsy, s1, s2);
	  printf(sout);
	  }
	}
      }
      if (with_header && !with_header_always)
        with_header = FALSE;
      if (do_ayanamsa) {
	daya = swe_get_ayanamsa(te);
	printf("Ayanamsa%s%s\n", gap, dms(daya, round_flag));
	continue;
      }
      if (strchr(plsel, 'e'))
        print_line(MODE_LABEL);
      for (psp = plsel; *psp != '\0'; psp++) {
        if (*psp == 'e') continue;
        ipl = letter_to_ipl((int) *psp);
	if (ipl == -2) {
	  printf("illegal parameter -p%s\n", plsel);
	  exit(1);
	}
        if (*psp == 'f')
          ipl = SE_FIXSTAR;
        else if (*psp == 's')
          ipl = atoi(sastno) + 10000;
        else if (*psp == 'z')
          ipl = atoi(shyp) + SE_FICT_OFFSET_1;
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
	if (*psp == 'q') {/* delta t */
	  x[0] = swe_deltat(te) * 86400;
	  x[1] = x[2] = x[3] = 0;
	  strcpy(se_pname, "Delta T");
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
	  if (strstr(serr, "'seorbel.txt' not found") == NULL)
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
        /* azimuth and height */
        if (strpbrk(fmt, "IiHhKk") != NULL) {
          /* first, get topocentric equatorial positions */
          iflgt = whicheph | SEFLG_EQUATORIAL | SEFLG_TOPOCTR;
          if (ipl == SE_FIXSTAR)
            iflgret = swe_fixstar(star, te, iflgt, xt, serr);
          else
            iflgret = swe_calc(te, ipl, iflgt, xt, serr);
          /* to azimuth/height */
          swe_azalt(tut, SE_EQU2HOR, geopos, 1013.25, 10, xt, xaz);
          if (diff_mode) {
            iflgret = swe_calc(te, ipldiff, iflgt, xt, serr);
            swe_azalt(tut, SE_EQU2HOR, geopos, 1013.25, 10, xt, x2);
	    if (diff_mode == DIFF_DIFF) {
	      for (i = 1; i < 3; i++) 
		xaz[i] -= x2[i];
	      if ((iflag & SEFLG_RADIANS) == 0) 
		xaz[0] = swe_difdeg2n(xaz[0], x2[0]);
	      else
		xaz[0] = swe_difrad2n(xaz[0], x2[0]);
	    } else {	/* DIFF_MIDP */
	      for (i = 1; i < 3; i++) 
		xaz[i] = (xaz[i] + x2[i]) / 2;
	      if ((iflag & SEFLG_RADIANS) == 0) 
		xaz[0] = swe_deg_midp(xaz[0], x2[0]);
	      else
		xaz[0] = swe_rad_midp(xaz[0], x2[0]);
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
        /* house position */
        if (strpbrk(fmt, "gGj") != NULL) {
	  armc = swe_degnorm(swe_sidtime(tut) * 15 + geopos[0]);
	  for (i = 0; i < 6; i++)
	    xsv[i] = x[i];
	  if (hpos_meth == 1)
	    xsv[1] = 0;
	    if (ipl == SE_FIXSTAR) 
	      strcpy(star2, star);
	    else 
	      *star2 = '\0';
	  if (hpos_meth >= 2 && tolower(ihsy) == 'g') {
	    swe_gauquelin_sector(tut, ipl, star2, iflag, hpos_meth, geopos, 0, 0, &hposj, serr);
	  } else {
	    hposj = swe_house_pos(armc, geopos[1], xobl[0], ihsy, xsv, serr);
	  }
	  if (tolower(ihsy) == 'g')
	    hpos = (hposj - 1) * 10;
	  else
	    hpos = (hposj - 1) * 30;
          if (diff_mode) {
	    for (i = 0; i < 6; i++)
	      xsv[i] = x2[i];
	    if (hpos_meth == 1)
	      xsv[1] = 0;
	    hpos2 = swe_house_pos(armc, geopos[1], xobl[0], ihsy, xsv, serr);
	    if (tolower(ihsy) == 'g')
	      hpos2 = (hpos2 - 1) * 10;
            else
	      hpos2 = (hpos2 - 1) * 30;
	    if (diff_mode == DIFF_DIFF) {
	      if ((iflag & SEFLG_RADIANS) == 0) 
		hpos = swe_difdeg2n(hpos, hpos2);
	      else
		hpos = swe_difrad2n(hpos, hpos2);
	    } else {	/* DIFF_MIDP */
	      if ((iflag & SEFLG_RADIANS) == 0) 
		hpos = swe_deg_midp(hpos, hpos2);
	      else
		hpos = swe_rad_midp(hpos, hpos2);
	    }
          }
        }
        strcpy(spnam, se_pname);
	print_line(0);
        line_count++;
        if (line_count >= line_limit) {
          sprintf(sout, "****** line count %d was exceeded\n", line_limit);
          printf(sout);
          break;
        }
      }         /* for psp */
      if (do_houses) {
	double cusp[100];
	int iofs; 
	if (tolower(ihsy) == 'g')
	  nhouses = 36;
	iofs = nhouses + 1;
	iflgret = swe_houses_ex(t,iflag, hous_lat, hous_long, ihsy, cusp, cusp+iofs);
        if (iflgret < 0) { 
          if (strcmp(serr, serr_save) != 0 ) {
            sprintf(sout, "error: %s\n", serr);
            printf(sout);
          }
          strcpy(serr_save, serr);
	} else {
	  for (ipl = 1; ipl < iofs+4; ipl++) {
	    x[0] = cusp[ipl];
	    x[1] = 0;	/* latitude */
	    x[2] = 1.0;	/* pseudo radius vector */
	    if (strpbrk(fmt, "aADdQ") != NULL) {
	      swe_cotrans(x, xequ, -xobl[0]);
	    }
	    print_line(MODE_HOUSE);
	    line_count++;
	  }
	}
      }
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

/*
 * The string fmt contains a sequence of format specifiers;
 * each character in fmt creates a column, the columns are
 * sparated by the gap string.
 */
static int print_line(int mode)
{
  char *sp, *sp2, sout[AS_MAXCH];
  double t2, ju2 = 0;
  double y_frac;
  double ar, sinp;
  AS_BOOL is_house = ((mode & MODE_HOUSE) != 0);
  AS_BOOL is_label = ((mode & MODE_LABEL) != 0);
  int32 iflgret;
  for (sp = fmt; *sp != '\0'; sp++) {
    if (is_house && strchr("bBsSrRxXuUQnNfF+-*/=", *sp) != NULL) continue;
    if (sp != fmt) 
      printf(gap);
    switch(*sp) {
    case 'y':
        if (is_label) { printf("year"); break; }
	sprintf(sout, "%d", jyear);
	printf(sout);
	break;
    case 'Y':
        if (is_label) { printf("year"); break; }
	t2 = swe_julday(jyear,1,1,ju2,gregflag);
	y_frac = (t - t2) / 365.0;
	sprintf(sout, "%.2f", jyear + y_frac);
	printf(sout);
	break;
    case 'p':
        if (is_label) { printf("obj.nr"); break; }
	if (! is_house && diff_mode == DIFF_DIFF)
	  sprintf(sout, "%d-%d", ipl, ipldiff);
	else if (! is_house && diff_mode == DIFF_MIDP)
	  sprintf(sout, "%d/%d", ipl, ipldiff);
	else
	  sprintf(sout, "%d", ipl);
	printf(sout);
	break;
    case 'P':
        if (is_label) { printf("%-15s", "name"); break; }
	if (is_house) {
	  if (ipl <= nhouses)
	    sprintf(sout, "house %2d       ", ipl);
	  else
	    sprintf(sout, "%-15s", hs_nam[ipl - nhouses]);
	} else if (diff_mode == DIFF_DIFF)
	  sprintf(sout, "%.3s-%.3s", spnam, spnam2);
	else if (diff_mode == DIFF_MIDP)
	  sprintf(sout, "%.3s/%.3s", spnam, spnam2);
	else
	  sprintf(sout, "%-15s", spnam);
	printf(sout);
	break;
    case 'J':
        if (is_label) { printf("julday"); break; }
	y_frac = (t - floor(t)) * 100;
	if (floor(y_frac) != y_frac)
	  sprintf(sout, "%.5f", t);
	else
	  sprintf(sout, "%.2f", t);
	printf(sout);
	break;
    case 'T':
        if (is_label) { printf("date"); break; }
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
        if (is_label) { printf("date"); break; }
	sprintf(sout, "%02d%02d%02d", jyear % 100, jmon, jday);
	printf(sout);
	break;
    case 'L':
        if (is_label) { printf("long."); break; }
	if (*psp == 'q') /* delta t */
	  goto ldec;
	printf(dms(x[0], round_flag));
	break;
    case 'l':
        if (is_label) { printf("long"); break; }
	ldec:
	sprintf(sout, "%# 11.7f", x[0]);
	printf(sout);
	break;
    case 'G':
        if (is_label) { printf("housPos"); break; }
	printf(dms(hpos, round_flag));
	break;
    case 'g':
        if (is_label) { printf("housPos"); break; }
	sprintf(sout, "%# 11.7f", hpos);
	printf(sout);
	break;
    case 'j':
        if (is_label) { printf("houseNr"); break; }
	sprintf(sout, "%# 11.7f", hposj);
	printf(sout);
	break;
    case 'Z':
        if (is_label) { printf("long"); break; }
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
		if (is_label) { printf("lon/day"); break; }
		printf(dms(x[3], round_flag));
		break;
	      case 'l':   /* speed! */
		if (is_label) { printf("lon/day"); break; }
		sprintf(sout, "%11.7f", x[3]);
		printf(sout);
		break;
	      case 'B':   /* speed! */
		if (is_label) { printf("lat/day"); break; }
		printf(dms(x[4], round_flag));
		break;
	      case 'b':   /* speed! */
		if (is_label) { printf("lat/day"); break; }
		sprintf(sout, "%11.7f", x[4]);
		printf(sout);
		break;
	      case 'A':   /* speed! */
		if (is_label) { printf("RA/day"); break; }
		printf(dms(xequ[3]/15, round_flag|SEFLG_EQUATORIAL));
		break;
	      case 'a':   /* speed! */
		if (is_label) { printf("RA/day"); break; }
		sprintf(sout, "%11.7f", xequ[3]);
		printf(sout);
		break;
	      case 'D':   /* speed! */
		if (is_label) { printf("dcl/day"); break; }
		printf(dms(xequ[4], round_flag));
		break;
	      case 'd':   /* speed! */
		if (is_label) { printf("dcl/day"); break; }
		sprintf(sout, "%11.7f", xequ[4]);
		printf(sout);
		break;
	      case 'R':   /* speed! */
	      case 'r':   /* speed! */
		if (is_label) { printf("AU/day"); break; }
		sprintf(sout, "%# 14.9f", x[5]);
		printf(sout);
		break;
	      case 'U':   /* speed! */
	      case 'X':   /* speed! */
		if (is_label) { printf("speed_0%sspeed_1%sspeed_2",gap,gap); break; }
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
		if (is_label) { printf("speed_0%sspeed_1%sspeed_2",gap,gap); break; }
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
	  if (is_label) { printf("deg/day"); break; }
	  printf(dms(x[3], round_flag));
	} else {
	  if (is_label) { printf("deg/day"); break; }
	  printf("%# 11.7f", x[3]);
	}
	break;
    case 'B':
	if (is_label) { printf("lat"); break; }
	printf(dms(x[1], round_flag));
	break;
    case 'b':
	if (is_label) { printf("lat"); break; }
	sprintf(sout, "%# 11.7f", x[1]);
	printf(sout);
	break;
    case 'A':     /* rectascensio */
	if (is_label) { printf("RA"); break; }
	printf(dms(xequ[0]/15, round_flag|SEFLG_EQUATORIAL));
	break;
    case 'a':     /* rectascensio */
	if (is_label) { printf("RA"); break; }
	sprintf(sout, "%# 11.7f", xequ[0]);
	printf(sout);
	break;
    case 'D':     /* declination */
	if (is_label) { printf("decl"); break; }
	printf(dms(xequ[1], round_flag));
	break;
    case 'd':     /* declination */
	if (is_label) { printf("decl"); break; }
	sprintf(sout, "%# 11.7f", xequ[1]);
	printf(sout);
	break;
    case 'I':     /* azimuth */
	if (is_label) { printf("azimuth"); break; }
	printf(dms(xaz[0], round_flag));
	break;
    case 'i':     /* azimuth */
	if (is_label) { printf("azimuth"); break; }
	sprintf(sout, "%# 11.7f", xaz[0]);
	printf(sout);
	break;
    case 'H':     /* height */
	if (is_label) { printf("height"); break; }
	printf(dms(xaz[1], round_flag));
	break;
    case 'h':     /* height */
	if (is_label) { printf("height"); break; }
	sprintf(sout, "%# 11.7f", xaz[1]);
	printf(sout);
	break;
    case 'K':     /* height (apparent) */
	if (is_label) { printf("hgtApp"); break; }
	printf(dms(xaz[2], round_flag));
	break;
    case 'k':     /* height (apparent) */
	if (is_label) { printf("hgtApp"); break; }
	sprintf(sout, "%# 11.7f", xaz[2]);
	printf(sout);
	break;
    case 'R':
	if (is_label) { printf("distAU"); break; }
	sprintf(sout, "%# 14.9f", x[2]);
	printf(sout);
	break;
    case 'r':
	if (is_label) { printf("dist"); break; }
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
	if (is_label) { printf("x0%sx1%sx2",gap,gap); break; }
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
	if (is_label) { printf("Q"); break; }
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
	double xasc[6], xdsc[6];
	int imeth = (*sp == tolower(*sp))?SE_NODBIT_MEAN:SE_NODBIT_OSCU;
	iflgret = swe_nod_aps(te, ipl, iflag, imeth, xasc, xdsc, NULL, NULL, serr);
	if (iflgret >= 0 && (ipl <= SE_NEPTUNE || *sp == 'N') ) {
	  if (is_label) { printf("nodAsc%snodDesc", gap); break; }
	  printf("%# 11.7f", xasc[0]);
	  printf(gap);
	  printf("%# 11.7f", xdsc[0]);
	}
      };
      break;
    case 'F': 
    case 'f':
      if (! is_house) {
	double xfoc[6], xaph[6], xper[6];
	int imeth = (*sp == tolower(*sp))?SE_NODBIT_MEAN:SE_NODBIT_OSCU;
	iflgret = swe_nod_aps(te, ipl, iflag, imeth, NULL, NULL, xper, xaph, serr);
	if (iflgret >= 0 && (ipl <= SE_NEPTUNE || *sp == 'F') ) {
	  if (is_label) { printf("peri%sapo", gap); break; }
	  printf("%# 11.7f", xper[0]);
	  printf(gap);
	  printf("%# 11.7f", xaph[0]);
	}
	imeth |= SE_NODBIT_FOPOINT;
	iflgret = swe_nod_aps(te, ipl, iflag, imeth, NULL, NULL, xper, xfoc, serr);
	if (iflgret >= 0 && (ipl <= SE_NEPTUNE || *sp == 'F') ) {
	  if (is_label) { printf("%sfocus", gap); break; }
	  printf(gap);
	  printf("%# 11.7f", xfoc[0]);
	}
      };
      break;
    case '+':
	if (is_house) break;
        if (is_label) { printf("phase"); break; }
	printf(dms(attr[0], round_flag));
	break;
    case '-':
        if (is_label) { printf("phase"); break; }
	if (is_house) break;
	printf("  %# 14.9f", attr[1]);
	break;
    case '*':
        if (is_label) { printf("elong"); break; }
	if (is_house) break;
	printf(dms(attr[2], round_flag));
	break;
    case '/':
        if (is_label) { printf("diamet"); break; }
	if (is_house) break;
	printf(dms(attr[3], round_flag));
	break;
    case '=':
        if (is_label) { printf("magn"); break; }
	if (is_house) break;
	printf("  %# 6.1fm", attr[4]);
	break;
    }     /* switch */
  }       /* for sp */
  printf("\n");
  return OK;
}

static char *dms(double xv, int32 iflg)
{
  int izod;
  int32 k, kdeg, kmin, ksec;
  char c = (unsigned char) ODEGREE_CHAR;
  char *sp, s1[50];
  static char s[50];
  int sgn;
  *s = '\0';
  if (iflg & SEFLG_EQUATORIAL)
    c = 'h';
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
    sprintf(s, " %3d%c", kdeg, c);
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
  return(s);
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

static int do_eclipse_etc() 
{
  int ii, i;
  AS_BOOL has_found = FALSE;
  double t_ut, dt;
  int ecl_type = 0, eclflag;
  char s1[AS_MAXCH], s2[AS_MAXCH];
  if (smod & (SMOD_LUNAR | SMOD_SOLAR | SMOD_OCCULT))  {
    if ((smod & SMOD_ALL) == 0) /* no selective eclipse type set, set all */
      smod |= SMOD_ALL;
    if (smod & SMOD_TOTAL) search_flag |= SE_ECL_TOTAL;
    if (smod & SMOD_ANNULAR) search_flag |= SE_ECL_ANNULAR | SE_ECL_PENUMBRAL;
    if (smod & SMOD_PARTIAL) search_flag |= SE_ECL_PARTIAL;
    if (smod & SMOD_ANNTOT) search_flag |= SE_ECL_ANNULAR_TOTAL;
  }
  /* 
   * for local eclipses: set geographic position of observer 
   */
  if (smod & (SMOD_LOCAL|SMOD_RISE|SMOD_METR))
    swe_set_topo(geopos[0], geopos[1], geopos[2]); 
  t_ut = tjd;
  do_printf("\n");
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
	sprintf(sout, "\"%04d %02d %02d %02d.%02d %d\",\n", jyear, jmon, jday, ihou, imin, ecl_type);
      } 
      do_printf(sout);
    }
    if ((smod & SMOD_SOLAR) && (smod & SMOD_LOCAL)) {
      if ((eclflag = swe_sol_eclipse_when_loc(t_ut, whicheph, geopos, tret,
attr, direction_flag, serr)) == ERR) {
        do_printf(serr);
        exit(0);
      } else { 
	has_found = FALSE;
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
	  i = swe_calc(t_ut + swe_deltat(t_ut), SE_ECL_NUT, 0, x, serr);
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
    if ((smod & SMOD_OCCULT) && (smod & SMOD_LOCAL)) {
      if ((eclflag = swe_lun_occult_when_loc(t_ut, ipl, star, whicheph, geopos, tret, attr, direction_flag, serr)) == ERR) {
        do_printf(serr);
        exit(0);
      } else { 
	has_found = FALSE;
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
	  i = swe_calc(t_ut + swe_deltat(t_ut), SE_ECL_NUT, 0, x, serr);
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
	sprintf(sout, "\"%04d %02d %02d %02d.%02d %d\",\n", jyear, jmon, jday, ihou, imin, ecl_type);
      } 
      do_printf(sout);
    }
    if ((smod & SMOD_OCCULT) && ! (smod & SMOD_LOCAL)) {
    /* * global search for eclipses */
      if ((eclflag = swe_lun_occult_when_glob(t_ut, ipl, star, whicheph, search_flag, tret, direction_flag, serr)) == ERR) {
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
      swe_lun_occult_where(t_ut, ipl, star, whicheph, geopos, attr, serr);
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
        if ((eclflag = swe_lun_occult_when_loc(t_ut - 10, ipl, star, whicheph, geopos, tret, attr, 0, serr)) == ERR) {
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
	sprintf(sout, "\"%04d %02d %02d %02d.%02d %d\",\n", jyear, jmon, jday, ihou, imin, ecl_type);
      } 
      do_printf(sout);
    }
    if (smod & SMOD_RISE) {
      if (swe_rise_trans(t_ut, ipl, star, whicheph, SE_CALC_RISE, geopos, 1013.25, 10, &(tret[0]), serr) != OK) {
	do_printf(serr);
	exit(0);
      } 
      if (swe_rise_trans(t_ut, ipl, star, whicheph, SE_CALC_SET, geopos, 1013.25, 10, &(tret[1]), serr) != OK) {
	do_printf(serr);
	exit(0);
      } 
      strcpy(sout, "rise     ");
      if (tret[0] == 0 || tret[0] > tret[1]) strcat(sout, "         -                     ");
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
      if (swe_rise_trans(t_ut, ipl, star, whicheph, SE_CALC_MTRANSIT, geopos, 1013.25, 10, &(tret[0]), serr) != OK) {
	do_printf(serr);
	exit(0);
      } 
      if (swe_rise_trans(t_ut, ipl, star, whicheph, SE_CALC_ITRANSIT, geopos, 1013.25, 10, &(tret[1]), serr) != OK) {
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
  return OK;
}

static char *hms_from_tjd(double x)
{
  static char s[AS_MAXCH];
  sprintf(s, "%s ", hms(fmod(x + 1000000.5, 1) * 24, 0));
  return s;
}

static char *hms(double x, int32 iflag)
{
  static char s[AS_MAXCH], *sp;
  char c = (unsigned char) ODEGREE_CHAR;
  x += 0.5 / 36000.0; /* round to 0.1 sec */
  strcpy(s, dms(x, iflag));
  sp = strchr(s, c);
  if (sp != NULL) {
    *sp = ':';
    *(sp + 3) = ':';
    *(sp + 8) = '\0';
  }
  return s;
}

static void do_printf(char *info)
{
#ifdef _WINDOWS
  fprintf(fp, info);
#else
  printf(info);
#endif
}

/* make_ephemeris_path().
 * ephemeris path includes
 *   current working directory
 *   + program directory
 *   + default path from swephexp.h on current drive
 *   +                              on program drive
 *   +                              on drive C:
 */
static int make_ephemeris_path(int32 iflg, char *argv0)
{
  char path[AS_MAXCH], s[AS_MAXCH];
  char *sp;
  char *dirglue = DIR_GLUE;
  size_t pathlen = 0; 
  /* moshier needs no ephemeris path */
  if (iflg & SEFLG_MOSEPH)
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

