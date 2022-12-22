
/* SWISSEPH
   
  Authors: Dieter Koch and Alois Treindl, Astrodienst Zurich

swetest -ec3 -p789 -lasp -b1.1.-580 -s30 -n200000 -lsco -ln2
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

/* attention: Microsoft Compiler does not accept strings > 2048 char */
static char *infocmd0 = "\n\
  Swetest computes a complete set of geocentric planetary positions,\n\
  for a given date or a sequence of dates.\n\
  Input can either be a date or an absolute julian day number.\n\
  0:00 (midnight).\n\
  With the proper options, swetest can be used to output a printed\n\
  ephemeris and transfer the data into other programs like spreadsheets\n\
  for graphical display.\n\
\n";
static char *infocmd1 = "\n\
  Command line options:\n\
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
        -hel    compute heliocentric positions\n\
        -bary   compute barycentric positions (bar. sun instead of node) \n\
        -head   don\'t print the header before the planet data. This option\n\
                is useful when you want to paste the output into a\n\
                spreadsheet for displaying graphical ephemeris.\n\
        +head   header before every step (with -s..) \n\
        -bDATE  use this begin date instead of asking; use -b1.1.1992 if\n\
                the begin date string contains blanks; use format -bj2400000.5\n\
                to express the date as absolute Julian day number.\n\
                Note: the date format is day month year (European style).\n\
        -gPPP   use PPP as gap between output columns; default is a single\n\
                blank.  -g followed by white space sets the\n\
                gap to the TAB character; which is useful for data entry\n\
                into spreadsheets.\n\
	-iXX	force iflag to value XX\n\
        -fSEQ   use SEQ as format sequence for the output columns;\n\
                default is PLBRuS.\n";
static char *infocmd2 = "\
        -ut     input date is universal time\n\
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
        -roundsec       round to seconds\n\
        -roundmin       round to minutes\n\
\n\
        -?, -h  display whole info\n\
        -hcmd   display commands\n\
        -hplan  display planet numbers\n\
        -hform  display format characters\n\
        -hdate  display input date format\n";
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
        A lunar apogee mean (Lilith, Black Moon) \n\
        B lunar apogee osculating \n\
        w lunar apogee interpolated\n\
        x lunar perigee interpolated\n\
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
        L longitude in degree ddd mm'ss\"\n\
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
        Q l, b, r, dl, db, dr, a, d, da, dd\n";
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
/**************************************************************/

//#include "sweodef.h"
#include "swephexp.h"
#include "swephlib.h"

#include <string.h>

#if MSDOS
#  include <direct.h>
#  include <dos.h>
#  ifdef _MSC_VER
#    include <sys\types.h>
#  endif
#  include <sys\stat.h>
#else
#  include <sys/stat.h>
#endif

#ifdef _WINDOWS
#include <windows.h>
#endif

//#include "sweph.h"
//#include "swephlib.h"

#define J2000           2451545.0  /* 2000 January 1.5 */
#define square_sum(x)   (x[0]*x[0]+x[1]*x[1]+x[2]*x[2])
#define SEFLG_EPHMASK   (SEFLG_JPLEPH|SEFLG_SWIEPH|SEFLG_MOSEPH)

#define BIT_ROUND_SEC   1
#define BIT_ROUND_MIN   2
#define BIT_ZODIAC      4
#define PLSEL_D  "0123456789mtABC"
#define PLSEL_P  "0123456789mtABCDEFGHI"
#define PLSEL_H  "JKLMNOPQRSTUV"
#define PLSEL_A  "0123456789mtABCDEFGHIJKLMNOPQRSTUV"

#define LIST_CONJ	1
#define LIST_POLYG	2
#define LIST_ASP	4

static int listflag = (LIST_CONJ|LIST_ASP);
static int nlist = 0;
static double maxorb = 4;
static char slist[AS_MAXCH];
static char sretdir[AS_MAXCH];
int iflag = 0;
AS_BOOL do_nod = FALSE, do_nos = FALSE, do_aph = FALSE, do_per = FALSE;
int do_true_node = 0;

#ifdef _WINDOWS
static FILE *fp = NULL;
#endif
static char se_pname[AS_MAXCH];
static char *zod_nam[] = {"ar", "ta", "ge", "cn", "le", "vi", 
                          "li", "sc", "sa", "cp", "aq", "pi"};
extern int swe_plan_pheno(double tjd, int ipl, int iflag, double *attr, char *serr);

static char star[AS_MAXCH] = "algol";
static char sastno[AS_MAXCH] = "433";
static char *dms(double x, long iflag);
static int do_calc(double tjd, int ipl, long iflag, double *x, char *serr);
static long do_fixstar(char *star, double tjd, long iflag, double *x, char *serr);
static void do_printf(char *info);
static double difdeg2n (double p1, double p2);
static int find_ephemeris_path(long iflag, char *argv0);
static int letter_to_ipl(int letter);
static int search_file(char *path, long iflag);
void polcart(double *l, double *x);
static int pos_compare(double *a1, double *a2);
static int find_maximum(double y00, double y11, double y2, double dx, 
			double *dxret, double *yret);
static int find_zero(double y00, double y11, double y2, double dx, 
			double *dxret, double *dxret2);
static double find_max(double tjd, double tstep, char *plsel, 
                       double harm, int recursive,
                       double *xxs);
static char *hms(double x, long iflag);

#define NPLX	30

#ifdef _WINDOWS
int WinMain(HANDLE hInstance, HANDLE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#else
int main(int argc, char *argv[])
#endif
{
#ifdef _WINDOWS
  int argc = 1;
  char *argv[30];
  struct _wopeninfo wo;
  char scmd[AS_MAXCH];
#endif
  char spnam[AS_MAXCH], spnam2[AS_MAXCH], saves[AS_MAXCH]; 
  char serr[AS_MAXCH], serr_save[AS_MAXCH], serr_warn[AS_MAXCH], sout[AS_MAXCH];
  char *sp, *spsave, *sp2, *psp;
  char *spno;
  char *fmt = "PLBRS";
  char *gap = " ";
  char *plsel = PLSEL_D;
#if HPUNIX
  char hostname[80];
#endif
  int i, j, n, iflag_f = -1, isp;
  int listflg2 = 0;
  int jmon, jday, jyear;
  int line_count, line_limit = 32000;
  double jut = 0.0, y_frac;
  int ipl, ipldiff = SE_SUN;
  long nstep = 1, istep;
  double x[6], x2[6], xequ[6], xcart[6], xcartq[6];
  static double xxx[NPLX], xsv[NPLX], xs2[NPLX];
                       /* static to init with 0 */
  double ar, sinp;
  char ephepath[AS_MAXCH];
  char fname[AS_MAXCH];
  char sdate[AS_MAXCH];
  char *begindate = NULL;
  long iflag2;              /* external flag: helio, geo... */
  long iflgret;
  int icomb, ncomb, plslen;
  int whicheph = SEFLG_SWIEPH;   
  short universal_time = FALSE;
  short with_header = TRUE;
  short with_header_always = FALSE;
  int gregflag;
  AS_BOOL plan_phenomena = FALSE;
  AS_BOOL do_eval_const = FALSE;
  AS_BOOL do_print_max = FALSE;
  AS_BOOL do_all_comb = FALSE;
  short diff_mode = FALSE;
  long round_flag = 0;
  double tjd = 2415020.5, t_ut;
  double t, te, t2, tstep = 1, thour = 0;
  double delt;
  serr[0] = serr_save[0] = serr_warn[0] = saves[0] = slist[0] = '\0';
#ifdef _WINDOWS
  wo._version = _QWINVER;
  wo._title = "Dlltest";
  wo._wbufsize = _WINBUFDEF;
  fp = _fwopen(&wo, NULL, "w+");
  do_printf("Enter options: ");
  rewind(fp);
  fgets(scmd, AS_MAXCH, fp);
  rewind(fp);
  argv[0] = "swetest";
  sp = strchr(scmd, '\n');
  if (sp != NULL)
    *sp = '\0';
  sp = scmd;
  /* cut command string */
  while (sp != NULL && *sp != '\0') {
    while (*sp == ' ')
      sp++;
    if (*sp == '\0')
      break;
    if (*sp == '-' || *sp == '+') {
      argv[argc] = sp;
      argc++;
    } else if (*sp == 'j' || (*sp >= '0' && *sp <= '9')) { 
      strcpy(s, "-b");  /* date is allowed without '-b' */
      strcat(s, sp);
      strcpy(sp, s);
      argv[argc] = sp;
      argc++;
    } else
      goto end_main;
    sp = strchr(sp, ' ');
    if (sp != NULL) {
      *sp = '\0';
      sp++;
    }
  }
  iflag = whicheph | SEFLG_SPEED;
#endif
  strcpy(ephepath, SE_EPHE_PATH);
  strcpy(fname, SE_FNAME_DE406);
  for (i = 1; i < argc; i++) {
    if (strlen(argv[i]) >= AS_MAXCH) argv[i][AS_MAXCH-1] = '\0';
    if (strncmp(argv[i], "-ut", 3) == 0) {
      universal_time = TRUE;
      if (strlen(argv[i]) > 3) {
        thour = atof(argv[i] + 3) + 1E-10;
        /* h.mmss -> decimal */
        t = (thour - (int) thour) * 100;
        j = (int) t;
        t = (t - (int) t) * 100;
        thour = (int) thour + j / 60.0 + t / 3600.0;
      }
    } else if (strncmp(argv[i], "-head", 5) == 0) {
      with_header = FALSE;
    } else if (strncmp(argv[i], "+head", 5) == 0) {
      with_header_always = TRUE;
    } else if (strcmp(argv[i], "-j2000") == 0) {
      iflag |= SEFLG_J2000;
    } else if (strncmp(argv[i], "-sidto", 6) == 0) {
      iflag |= SEFLG_SIDEREAL;
      swe_set_sid_mode(atol(argv[i] + 6) + SE_SIDBIT_ECL_T0, 0, 0);
    } else if (strncmp(argv[i], "-sidsy", 6) == 0) {
      iflag |= SEFLG_SIDEREAL;
      swe_set_sid_mode(atol(argv[i] + 6) + SE_SIDBIT_SSY_PLANE, 0, 0);
    } else if (strncmp(argv[i], "-sid", 4) == 0) {
      iflag |= SEFLG_SIDEREAL;
      swe_set_sid_mode(atol(argv[i] + 4), 0, 0);
    } else if (strcmp(argv[i], "-lcon") == 0) {
      listflg2 |= LIST_CONJ;
    } else if (strcmp(argv[i], "-lpol") == 0) {
      listflg2 |= LIST_POLYG;
    } else if (strcmp(argv[i], "-lasp") == 0) {
      listflg2 |= LIST_ASP;
    } else if (strncmp(argv[i], "-ln", 3) == 0) {
      nlist = atoi(argv[i]+3);
    } else if (strncmp(argv[i], "-ls", 3) == 0) {
      strcpy(slist, argv[i]+3);
    } else if (strncmp(argv[i], "-orb", 4) == 0) {
      maxorb = atof(argv[i]+4);
    } else if (strncmp(argv[i], "-ec", 3) == 0) {
      do_eval_const = TRUE;
      if (*(argv[i] + 3) != '\0')
        do_print_max = TRUE;
      if (*(argv[i] + 3) == '3')
        do_all_comb = TRUE;
    } else if (strncmp(argv[i], "-j", 2) == 0) {
      begindate = argv[i] + 1;
    } else if (strncmp(argv[i], "-ejpl", 5) == 0) {
      whicheph = SEFLG_JPLEPH;
      strcpy(ephepath, SE_EPHE_PATH);
      if (*(argv[i]+5) != '\0') {
        strcpy(fname, argv[i]+5);
      }
    } else if (strcmp(argv[i], "-eswe") == 0) {
      whicheph = SEFLG_SWIEPH;
      strcpy(ephepath, SE_EPHE_PATH);
    } else if (strcmp(argv[i], "-emos") == 0) {
      whicheph = SEFLG_MOSEPH;
    } else if (strcmp(argv[i], "-hel") == 0) {
      iflag |= SEFLG_HELCTR;
    } else if (strcmp(argv[i], "-bary") == 0) {
      iflag |= SEFLG_BARYCTR;
    } else if (strcmp(argv[i], "-topo") == 0) {
      iflag |= SEFLG_TOPOCTR;
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
    } else if (strncmp(argv[i], "-phen", 5) == 0) {
      plan_phenomena = TRUE;
    } else if (strncmp(argv[i], "-testaa", 7) == 0) {
      whicheph = SEFLG_JPLEPH;
      strcpy(ephepath, SE_EPHE_PATH);
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
    } else if (strncmp(argv[i], "-nod", 4) == 0) {
	  if (strchr(argv[i], 's') != NULL)
	    do_nos = TRUE;
	  else
	    do_nod = TRUE;
	  if (strchr(argv[i], 'm') != NULL)
		do_true_node = SE_NODBIT_MEAN;
	  if (strchr(argv[i], 't') != NULL)
		do_true_node = SE_NODBIT_OSCU;
	  if (strchr(argv[i], 'b') != NULL)
		do_true_node |= SE_NODBIT_OSCU_BAR;
	  if (strchr(argv[i], 'f') != NULL)
		do_true_node |= SE_NODBIT_FOPOINT;
    } else if (strncmp(argv[i], "-aph", 4) == 0) {
	  do_aph = TRUE;
	  if (strchr(argv[i], 'm') != NULL)
		do_true_node = SE_NODBIT_MEAN;
	  if (strchr(argv[i], 't') != NULL)
		do_true_node = SE_NODBIT_OSCU;
	  if (strchr(argv[i], 'b') != NULL)
		do_true_node |= SE_NODBIT_OSCU_BAR;
	  if (strchr(argv[i], 'f') != NULL)
		do_true_node |= SE_NODBIT_FOPOINT;
    } else if (strncmp(argv[i], "-per", 4) == 0) {
	  do_per = TRUE;
	  if (strchr(argv[i], 'm') != NULL)
		do_true_node = SE_NODBIT_MEAN;
	  if (strchr(argv[i], 't') != NULL)
		do_true_node = SE_NODBIT_OSCU;
	  if (strchr(argv[i], 'b') != NULL)
		do_true_node |= SE_NODBIT_OSCU_BAR;
	  if (strchr(argv[i], 'f') != NULL) 
		do_true_node |= SE_NODBIT_FOPOINT;
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
    } else if (strncmp(argv[i], "-i", 2) == 0) {
      iflag_f = atoi(argv[i]+2);
    } else if (strncmp(argv[i], "-s", 2) == 0) {
      tstep = atof(argv[i]+2);
    } else if (strncmp(argv[i], "-b", 2) == 0) {
      begindate = argv[i] + 2;
    } else if (strncmp(argv[i], "-f", 2) == 0) {
      fmt = argv[i] + 2;
    } else if (strncmp(argv[i], "-g", 2) == 0) {
      gap = argv[i] + 2;
      if (*gap == '\0') gap = "\t";
    } else if (strncmp(argv[i], "-d", 2) == 0) {
      diff_mode = TRUE;
      sp = argv[i]+2;
      ipldiff = letter_to_ipl((int) *sp);
      if (ipldiff <0) ipldiff = SE_SUN;
      swe_get_planet_name(ipldiff, spnam2);
    } else if (strcmp(argv[i], "-roundsec") == 0) {
      round_flag |= BIT_ROUND_SEC;
    } else if (strcmp(argv[i], "-roundmin") == 0) {
      round_flag |= BIT_ROUND_MIN;
    } else if (strncmp(argv[i], "-h", 2) == 0
      || strncmp(argv[i], "-?", 2) == 0) {
      sp = argv[i]+2;
      if (*sp == 'c' || *sp == '\0') {
        do_printf(infocmd0);
        do_printf(infocmd1);
        do_printf(infocmd2);
      }
      if (*sp == 'p' || *sp == '\0')
        do_printf(infoplan);
      if (*sp == 'f' || *sp == '\0')
        do_printf(infoform);
      if (*sp == 'd' || *sp == '\0')
        do_printf(infodate);
      goto end_main;
    } else {
      if (strlen(argv[i]) >= AS_MAXCH - 30) argv[i][AS_MAXCH -30] = '\0';
      sprintf(sout, "illegal option %s\n", argv[i]);
      do_printf(sout);
      exit(1);
    }
  }
  if (listflg2 > 0)
    listflag = listflg2;
  plslen = strlen(plsel);
  if (do_all_comb)
    ncomb = (1 << plslen) - 1;
  swe_set_topo(8.55, 47.35, 400);
#if HPUNIX
  gethostname (hostname, 80);
  if (strstr(hostname, "as10") != NULL) 
    line_limit = 1000;
#endif
  if (with_header) {
    for (i = 0; i < argc; i++) {
      do_printf(argv[i]);
      do_printf(" ");
    }
  }
  iflag = (iflag & ~SEFLG_EPHMASK) | whicheph;
  if (strpbrk(fmt, "SsQ") != NULL) 
    iflag |= SEFLG_SPEED;
#if 1
  swe_set_ephe_path(ephepath);
/*  swe_set_ephe_path("d:\\sweph\\ephe\\");*/
#else
  if (find_ephemeris_path(iflag, argv[0]) == ERR) {
    iflag = (iflag & ~SEFLG_EPHMASK) | SEFLG_MOSEPH;
    whicheph = SEFLG_MOSEPH;
  }
#endif
  if (whicheph & SEFLG_JPLEPH)
    swe_set_jpl_file(fname);
  while (TRUE) {
    serr[0] = serr_save[0] = serr_warn[0] = '\0';
    if (begindate == NULL) {
      do_printf("\nDate ?");
      sdate[0] = '\0';
#ifdef _WINDOWS
      rewind(fp);
      fgets(sdate, AS_MAXCH, fp);
      rewind(fp);
#else
      gets(sdate);
#endif
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
      if (tjd < 2299160.5) {
        gregflag = SE_JUL_CAL;
      } else {
        gregflag = SE_GREG_CAL;
      }
      if (strstr(sp, "jul") != NULL) {
        gregflag = SE_JUL_CAL;
      } else if (strstr(sp, "greg") != NULL) {
        gregflag = SE_GREG_CAL;
      }
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
      if ((long) jyear * 10000L + (long) jmon * 100L + (long) jday < 15821015L)  {
        gregflag = SE_JUL_CAL;
      } else {
        gregflag = SE_GREG_CAL;
      }
      if (strstr(sp, "jul") != NULL) {
        gregflag = SE_JUL_CAL;
      } else if (strstr(sp, "greg") != NULL) {
        gregflag = SE_GREG_CAL;
      }
      jut = 0;
      tjd = swe_julday(jyear,jmon,jday,jut,gregflag);        
      tjd += thour / 24;
    }
    line_count = 0;
    for (t = tjd, istep = 1; istep <= nstep; t += tstep, istep++) {
      if (t < 2299160.5) {
        gregflag = SE_JUL_CAL;
      } else {
        gregflag = SE_GREG_CAL;
      }
      if (strstr(spsave, "jul") != NULL) {
        gregflag = SE_JUL_CAL;
      } else if (strstr(spsave, "greg") != NULL) {
        gregflag = SE_GREG_CAL;
      }
      swe_revjul(t, gregflag, &jyear, &jmon, &jday, &jut);
      if (with_header) {
        sprintf(sout, "\ndate (dmy) %d.%d.%d", jday, jmon, jyear);
        do_printf(sout);
        if (gregflag) {
          do_printf(" greg.");
	} else {
          do_printf(" jul.");
	}
        t2 = t + 0.5;
        t2 = (t2 - (long int) t2) * 24;
        sprintf(sout, "  % 2d:", (int) t2); 
        do_printf(sout);
        t2 = (t2 - (long int) t2) * 60 + 0.5;
        sprintf(sout, "%02d", (int) t2); 
        do_printf(sout);
        if (universal_time) {
          do_printf(" UT");
	} else {
          do_printf(" ET");
	}
      }
      delt = swe_deltat(t);
      if (universal_time) {
        if (with_header) {
          sprintf(sout, "\nUT: %f", t);
          do_printf(sout);
        }
        if (with_header) {
          sprintf(sout, "     delta t: %f sec", delt * 86400.0);
          do_printf(sout);
        }
        te = t + delt;
        t_ut = t;
      } else {
        te = t;
        t_ut = te - delt;
      }
{
double x[6];
x[1] = x[2] = x[0] = 1; 
swi_precess(x, te, iflag, 1);
}
      if (with_header && (iflag & SEFLG_SIDEREAL)) {
        double daya = swe_get_ayanamsa(te);
		char s[AS_MAXCH];
        sprintf(s, "  ayan = %s", dms(daya, 0));
        do_printf(s);
      }
      if (with_header)
        do_printf("\n");
      for (psp = plsel, isp = 0; *psp != '\0'; psp++, isp++) {
        ipl = letter_to_ipl((int) *psp);
        if (with_header && !with_header_always)
          with_header = FALSE;
        if (*psp == 'f') {
          ipl = SE_FIXSTAR;
        } else if (*psp == 's') {
          ipl = atoi(sastno) + 10000;
	}
        if (plan_phenomena) {
          double attr[20];
          if (with_header && psp == plsel)
            do_printf("planet\tphase_ang\tphase\t\telong\t\tdiam\t\tmag\n");
          if (swe_pheno(te, ipl, iflag, attr, serr) == ERR) {
            do_printf(serr);
            exit(0);
          }
		  if (strchr(fmt, 'P') != NULL)
	        swe_get_planet_name(ipl, se_pname);
          se_pname[7] = 0;
          sprintf(sout, "%s\t%f\t%f\t%f\t%f\t%f\n", se_pname, 
                 attr[0], attr[1], attr[2], attr[3], attr[4]);
          do_printf(sout);
          continue;
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
        } else {         /* geocentric */
		  if (do_nod || do_nos || do_per || do_aph) { 
            if (ipl == SE_MEAN_NODE || ipl == SE_TRUE_NODE 
                || ipl == SE_MEAN_APOG || ipl == SE_OSCU_APOG)
			  continue;
          } else if (ipl == SE_EARTH)
            continue;
		}
        /* ecliptic position */
	if (iflag_f >=0)
	  iflag = iflag_f;
        if (ipl == SE_FIXSTAR) {
          iflgret = do_fixstar(star, te, iflag, x, serr);
          strcpy(se_pname, star);
        } else {
          iflgret = do_calc(te, ipl, iflag, x, serr);
	  if (iflgret != ERR)
	    swe_get_planet_name(ipl, se_pname);
        }
        if (1) {
          double xx[6], xnorm[6], m;
          static double xsum[6];
          switch(ipl) {
/*          case SE_SUN:     m =           1;  break;
            case SE_MERCURY: m =     6023600;  break;
            case SE_VENUS: m =       408523.5; break;
            case SE_EARTH: m =       328900.5; break;
            case SE_MARS: m =       3098710.0; break;
            case SE_JUPITER: m =       1047.35;break;
            case SE_SATURN: m =        3498.0; break;
            case SE_URANUS: m =       22960.0; break;
            case SE_NEPTUNE: m =      19314.0; break;
            case SE_PLUTO: m =    130000000.0; break;
            case SE_CERES: m =   2127659574.0; break;
            case SE_PALLAS: m = 10000000000.0; break;
            case SE_VESTA:  m =  7692307692.0; break; */
          case SE_SUN:     m = 2.9591220828559109E-04; break;
          case SE_MERCURY: m = 4.9125474514508119E-11; break;
          case SE_VENUS: m = 7.2434524861627027E-10; break;
          case SE_EARTH: m = 8.9970113467124988E-10; break;
          case SE_MARS: m = 9.5495351057792581E-11; break;
          case SE_JUPITER: m = 2.8253459095242264E-07; break;
          case SE_SATURN: m = 8.4597151856806587E-08; break;
          case SE_URANUS: m = 1.2920249167819694E-08; break;
          case SE_NEPTUNE: m = 1.5243589007842763E-08; break;
          case SE_PLUTO: m = 2.1886997654259697E-12; break;
          //case SE_CERES: m = 1.3907873789422780E-13; break;
          //case SE_PALLAS: m = 2.9591220828559110E-14; break;
          //case SE_VESTA:  m = 3.8468587077126837E-14; break;
		  default: m = 0;
          }
		  if (m > 0) m = 1 / m;
          for (i = 0; i < 6; i++) {
            xx[i] = x[i];
            if (i != 2 && i != 5)
              xx[i] *= DEGTORAD;
          }
          swi_polcart_sp(xx, xx);
          swi_cross_prod(xx, xx+3, xnorm);
          for (i = 0; i < 3; i++) {
            if (ipl == SE_MOON)
              break;
            if (ipl == SE_SUN)
              xsum[i] = 0;
            xsum[i] += xnorm[i] / m;
          }
          if (1 && ipl == SE_EARTH) {
            swi_cartpol(xsum, xsum);
            /*printf("spol:  %f  %f\n", xsum[0] * RADTODEG, xsum[1] * RADTODEG);*/
          }
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
                || ipl == SE_CHIRON || ipl == SE_CUPIDO 
                || ipl >= SE_AST_OFFSET || ipl == SE_FIXSTAR)) {
            sprintf(sout, "error: %s\n", serr);
            do_printf(sout);
          }
          strcpy(serr_save, serr);
        } else if (*serr != '\0' && *serr_warn == '\0') {
          strcpy(serr_warn, serr);
        }
        if (diff_mode) {
          double x3[6], x4[6];
          iflgret = do_calc(te, ipldiff, iflag, x2, serr);
          if (iflgret < 0) { 
            sprintf(sout, "error: %s\n", serr);
            do_printf(sout);
          }
          for (i = 0; i < 6; i++) {
            x3[i] = x[i];
            x4[i] = x2[i];
            x[i] -= x2[i];
          }
          if ((iflag & SEFLG_RADIANS) == 0) 
            x[0] = difdeg2n(x[0], 0.0);
          x3[0] *= DEGTORAD;
          x3[1] *= DEGTORAD;
          x4[0] *= DEGTORAD;
          x4[1] *= DEGTORAD;
          polcart(x3, x3);
          polcart(x4, x4);
          for (i = 0; i < 6; i++) 
            x3[i] -= x4[i];
          x[2] = sqrt(square_sum(x3));
        }
        /* equator position */
        if (strpbrk(fmt, "aADdQ") != NULL) {
          iflag2 = iflag | SEFLG_EQUATORIAL;
          if (ipl == SE_FIXSTAR) {
            iflgret = do_fixstar(star, te, iflag2, xequ, serr);
	  } else {
            iflgret = do_calc(te, ipl, iflag2, xequ, serr);
	  }
          if (diff_mode) {
            iflgret = do_calc(te, ipldiff, iflag2, x2, serr);
            for (i = 0; i < 6; i++) 
              xequ[i] -= x2[i];
            if ((iflag & SEFLG_RADIANS) == 0) 
              xequ[0] = difdeg2n(xequ[0], 0.0);
          }
        }
        /* ecliptic cartesian position */
        if (strpbrk(fmt, "XU") != NULL) {
          iflag2 = iflag | SEFLG_XYZ;
          if (ipl == SE_FIXSTAR) {
            iflgret = do_fixstar(star, te, iflag2, xcart, serr);
	  } else {
            iflgret = do_calc(te, ipl, iflag2, xcart, serr);
	  }
          if (diff_mode) {
            iflgret = do_calc(te, ipldiff, iflag2, x2, serr);
            for (i = 0; i < 6; i++) 
              xcart[i] -= x2[i];
          }
        }
        /* equator cartesian position */
        if (strpbrk(fmt, "xu") != NULL) {
          iflag2 = iflag | SEFLG_XYZ | SEFLG_EQUATORIAL;
          if (ipl == SE_FIXSTAR) {
            iflgret = do_fixstar(star, te, iflag2, xcartq, serr);
	  } else {
            iflgret = do_calc(te, ipl, iflag2, xcartq, serr);
	  }
          if (diff_mode) {
            iflgret = do_calc(te, ipldiff, iflag2, x2, serr);
            for (i = 0; i < 6; i++) 
              xcartq[i] -= x2[i];
          }
        }
        if (do_eval_const) {
          xs2[isp] = xsv[isp];
          xsv[isp] = xxx[isp];
          xxx[isp] = x[0];
          continue;
        }
        strcpy(spnam, se_pname);
        /*
         * The string fmt contains a sequence of format specifiers;
         * each character in fmt creates a column, the columns are
         * sparated by the gap string.
         */
        for (sp = fmt; *sp != '\0'; sp++) {
          if (sp != fmt) 
            do_printf(gap);
          switch(*sp) {
          case 'y':
              sprintf(sout, "%d", jyear);
              do_printf(sout);
              break;
          case 'Y':
              jut = 0;
              t2 = swe_julday(jyear,1,1,jut,gregflag);
              y_frac = (t - t2) / 365.0;
              sprintf(sout, "%.2lf", jyear + y_frac);
              do_printf(sout);
              break;
          case 'p':
              if (diff_mode) {
                sprintf(sout, "%d-%d", ipl, ipldiff);
	      } else {
                sprintf(sout, "%d", ipl);
	      }
              do_printf(sout);
              break;
          case 'P':
              if (diff_mode) {
                sprintf(sout, "%.3s-%.3s", spnam, spnam2);
	      } else {
                sprintf(sout, "%-15s", spnam);
	      }
              do_printf(sout);
              break;
          case 'J':
          case 'j':
              sprintf(sout, "%.2f", t);
              do_printf(sout);
              break;
          case 'T':
              sprintf(sout, "%02d.%02d.%d", jday, jmon, jyear);
              do_printf(sout);
              break;
          case 't':
              sprintf(sout, "%02d%02d%02d", jyear % 100, jmon, jday);
              do_printf(sout);
              break;
          case 'L':
              do_printf(dms(x[0], round_flag));
              break;
          case 'l':
              sprintf(sout, "%# 11.7f", x[0]);
              do_printf(sout);
              break;
          case 'Z':
              do_printf(dms(x[0], round_flag|BIT_ZODIAC));
              break;
          case 'S':
          case 's':
              if (*(sp+1) == 'S' || *(sp+1) == 's' || strpbrk(fmt, "XUxu") != NULL) {
                for (sp2 = fmt; *sp2 != '\0'; sp2++) {
                  if (sp2 != fmt) 
                    do_printf(gap);
                  switch(*sp2) {
                    case 'L':   /* speed! */
                    case 'Z':   /* speed! */
                      do_printf(dms(x[3], round_flag));
                      break;
                    case 'l':   /* speed! */
                      sprintf(sout, "%11.7f", x[3]);
                      do_printf(sout);
                      break;
                    case 'B':   /* speed! */
                      do_printf(dms(x[4], round_flag));
                      break;
                    case 'b':   /* speed! */
                      sprintf(sout, "%11.7f", x[4]);
                      do_printf(sout);
                      break;
                    case 'A':   /* speed! */
                      do_printf(dms(xequ[3]/15, round_flag|SEFLG_EQUATORIAL));
                      break;
                    case 'a':   /* speed! */
                      sprintf(sout, "%11.7f", xequ[3]);
                      do_printf(sout);
                      break;
                    case 'D':   /* speed! */
                      do_printf(dms(xequ[4], round_flag));
                      break;
                    case 'd':   /* speed! */
                      sprintf(sout, "%11.7f", xequ[4]);
                      do_printf(sout);
                      break;
                    case 'R':   /* speed! */
                    case 'r':   /* speed! */
                      sprintf(sout, "%# 14.9f", x[5]);
                      do_printf(sout);
                      break;
                    case 'U':   /* speed! */
                    case 'X':   /* speed! */
                      if (*sp =='U') {
                        ar = sqrt(square_sum(xcart));
		      } else {
                        ar = 1;
		      }
                      sprintf(sout, "%# 14.9f%s", xcart[3]/ar, gap);
                      do_printf(sout);
                      sprintf(sout, "%# 14.9f%s", xcart[4]/ar, gap);
                      do_printf(sout);
                      sprintf(sout, "%# 14.9f", xcart[5]/ar);
                      do_printf(sout);
                      break;
                    case 'u':   /* speed! */
                    case 'x':   /* speed! */
                      if (*sp =='u') {
                        ar = sqrt(square_sum(xcartq));
		      } else {
                        ar = 1;
		      }
                      sprintf(sout, "%# 14.9f%s", xcartq[3]/ar, gap);
                      do_printf(sout);
                      sprintf(sout, "%# 14.9f%s", xcartq[4]/ar, gap);
                      do_printf(sout);
                      sprintf(sout, "%# 14.9f", xcartq[5]/ar);
                      do_printf(sout);
                      break;
                    default:
                      break;
                  }
                }
                if (*(sp+1) == 'S' || *(sp+1) == 's')
                  sp++;
              } else {
                do_printf(dms(x[3], round_flag));
              }
              break;
          case 'B':
              do_printf(dms(x[1], round_flag));
              break;
          case 'b':
              sprintf(sout, "%# 11.7f", x[1]);
              do_printf(sout);
              break;
          case 'A':     /* rectascensio */
              do_printf(dms(xequ[0]/15, round_flag|SEFLG_EQUATORIAL));
              break;
          case 'a':     /* rectascensio */
              sprintf(sout, "%# 11.7f", xequ[0]);
              do_printf(sout);
              break;
          case 'D':     /* declination */
              do_printf(dms(xequ[1], round_flag));
              break;
          case 'd':     /* declination */
              sprintf(sout, "%# 11.7f", xequ[1]);
              do_printf(sout);
              break;
          case 'R':
              sprintf(sout, "%# 14.9f", x[2]);
              do_printf(sout);
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
              do_printf(sout);
              break;
          case 'U':
          case 'X':
              if (*sp =='U') {
                ar = sqrt(square_sum(xcart));
	      } else {
                ar = 1;
	      }
              sprintf(sout, "%# 14.9f%s", xcart[0]/ar, gap);
              do_printf(sout);
              sprintf(sout, "%# 14.9f%s", xcart[1]/ar, gap);
              do_printf(sout);
              sprintf(sout, "%# 14.9f", xcart[2]/ar);
              do_printf(sout);
              break;
          case 'u':
          case 'x':
              if (*sp =='u') {
                ar = sqrt(square_sum(xcartq));
	      } else {
                ar = 1;
	      }
              sprintf(sout, "%# 14.9f%s", xcartq[0]/ar, gap);
              do_printf(sout);
              sprintf(sout, "%# 14.9f%s", xcartq[1]/ar, gap);
              do_printf(sout);
              sprintf(sout, "%# 14.9f", xcartq[2]/ar);
              do_printf(sout);
              break;
          case 'Q':
              sprintf(sout, "%-15s", spnam);
              do_printf(sout);
              do_printf(dms(x[0], round_flag));
              do_printf(dms(x[1], round_flag));
              sprintf(sout, "  %# 14.9f", x[2]);
              do_printf(sout);
              do_printf(dms(x[3], round_flag));
              do_printf(dms(x[4], round_flag));
              sprintf(sout, "  %# 14.9f\n", x[5]);
              do_printf(sout);
              sprintf(sout, "               %s", dms(xequ[0], round_flag));
              do_printf(sout);
              do_printf(dms(xequ[1], round_flag));
              sprintf(sout, "                %s", dms(xequ[3], round_flag));
              do_printf(sout);
              do_printf(dms(xequ[4], round_flag));
              break;
          }     /* switch */
        }       /* for sp */
        do_printf("\n");
        line_count++;
        if (line_count >= line_limit) {
          sprintf(sout, "****** line count %d was exceeded\n", line_limit);
          do_printf(sout);
          break;
        }
      }         /* for psp */
      if (line_count >= line_limit) 
        break;
      if (do_eval_const && t - tjd > tstep * 2.5) {
        int n, n1, n2, pl, i, j, k, ye,mo,da, nc;
        double x, y, xi, jut;
        double xxn[NPLX], xxa[NPLX], xxs[NPLX], xxns[NPLX], xxas[NPLX];
        double val, valn, vala;
        double xxe[NPLX], xxeb[NPLX], lbr[6];
        double d, dang, tout = t;
        double sumx = 0, sumy = 0, sumxn = 0, sumyn = 0;
        double sumxa = 0, sumya = 0;
        double max = 0.5, min = 0.5;
        double y0, y1, y2, dt, dterr;
        char plansel[20];
        char *sasp = "chsqtio";
        AS_BOOL do_output = FALSE;
        AS_BOOL do_balance = TRUE;
        int recursive = 1;
        char styp[AS_MAXCH];
        static double prmin = 0;
        static double valsave = 0.5, valsav2 = 0.5;
        static double valnsave = 0.5, valnsav2 = 0.5;
        static double valasave = 0.5, valasav2 = 0.5;
        double ang, dmax, dmax2;
        char aspstr[30];
    for (icomb = 1; icomb <= ncomb; icomb++) {
        if (plslen == nlist) {
          icomb = ncomb;
          strcpy(plansel, plsel);
        } else {
          for (i = 0, *plansel = 0; i < plslen; i++) {
            if (icomb & (1 << i))
              sprintf(plansel + strlen(plansel), "%c", *(plsel + i));
          } 
          /*printf("%s %x\n", plansel, icomb);*/
        }
        n = strlen(plansel);
        if (n == 1)
          continue;
        ang = 360.0 / n;
#ifdef BALANCE
        if (prmin == 0) {
          dang = n * 3; /* 4 planets -> orbis = 12 dgr */
          for (i = 0, x = y = 0; i < n; i++) {
            xi = i * 360.0 / n;
            if (i == 0)
              xi += dang;
            x += cos(xi * DEGTORAD);
            y += sin(xi * DEGTORAD);
          }
          prmin = sqrt(x * x + y * y) / n;
        }
#endif
        sumx = sumy = sumxn = sumyn = sumxa = sumya = 0;
        for (sp = plansel; *sp != '\0'; sp++) {
          pl = strchr(plsel, *sp) - plsel;
          sumx += cos(xxx[pl] * DEGTORAD);
	  sumy += sin(xxx[pl] * DEGTORAD);
	  xxn[pl] = swe_degnorm(xxx[pl] * n);
          sumxn += cos(xxn[pl] * DEGTORAD);
	  sumyn += sin(xxn[pl] * DEGTORAD);
	  xxa[pl] = swe_degnorm(xxx[pl] * 12);
          sumxa += cos(xxa[pl] * DEGTORAD);
	  sumya += sin(xxa[pl] * DEGTORAD);
        }
	val = sqrt(sumx * sumx + sumy * sumy);
        valn = sqrt(sumxn * sumxn + sumyn * sumyn) / n;
        vala = sqrt(sumxa * sumxa + sumya * sumya) / 12;
        sumx = sumy = sumxn = sumyn = sumxa = sumya = 0;
        for (sp = plansel; *sp != '\0'; sp++) {
          pl = strchr(plsel, *sp) - plsel;
          sumx += cos(xsv[pl] * DEGTORAD);
	  sumy += sin(xsv[pl] * DEGTORAD);
	  xxn[pl] = swe_degnorm(xsv[pl] * n);
          sumxn += cos(xxn[pl] * DEGTORAD);
	  sumyn += sin(xxn[pl] * DEGTORAD);
	  xxa[pl] = swe_degnorm(xsv[pl] * 12);
          sumxa += cos(xxa[pl] * DEGTORAD);
	  sumya += sin(xxa[pl] * DEGTORAD);
        }
	valsave = sqrt(sumx * sumx + sumy * sumy);
        valnsave = sqrt(sumxn * sumxn + sumyn * sumyn) / n;
        valasave = sqrt(sumxa * sumxa + sumya * sumya) / 12;
        sumx = sumy = sumxn = sumyn = sumxa = sumya = 0;
        for (sp = plansel; *sp != '\0'; sp++) {
          pl = strchr(plsel, *sp) - plsel;
          sumx += cos(xs2[pl] * DEGTORAD);
	  sumy += sin(xs2[pl] * DEGTORAD);
	  xxn[pl] = swe_degnorm(xs2[pl] * n);
          sumxn += cos(xxn[pl] * DEGTORAD);
	  sumyn += sin(xxn[pl] * DEGTORAD);
	  xxa[pl] = swe_degnorm(xs2[pl] * 12);
          sumxa += cos(xxa[pl] * DEGTORAD);
	  sumya += sin(xxa[pl] * DEGTORAD);
        }
	valsav2 = sqrt(sumx * sumx + sumy * sumy);
        valnsav2 = sqrt(sumxn * sumxn + sumyn * sumyn) / n;
        valasav2 = sqrt(sumxa * sumxa + sumya * sumya) / 12;
        dmax = 999;
        do_output = FALSE;
        *aspstr = '\0';
	if (do_print_max && fabs(t - tjd) > fabs(tstep * 2.5)) {
	  if (val < valsave && valsav2 < valsave 
                 && (listflag & LIST_CONJ)) {
            AS_BOOL is_con = TRUE;
            do_output = TRUE;
            tout = find_max(t - tstep, tstep, plansel, 1, 2, xxs);
            qsort((void *) xxs, (size_t) n, sizeof(double),
              (int (*)(const void *, const void *))(pos_compare));
            for (i = 1; i < n; i++) {
              d = swe_degnorm(xxs[i] - xxs[i-1]);
              if (d > maxorb) is_con = FALSE;
            }   
            dmax = swe_degnorm(xxs[n-1] - xxs[0]);
            if (is_con) {
	      strcpy(styp, "conjunction");
	      strcpy(aspstr, "c");
            } else
              do_output = FALSE;
	  } else if (val > valsave && valsav2 > valsave
                 && (listflag & LIST_POLYG)) {
            AS_BOOL is_poly = TRUE;
            AS_BOOL is_asp = TRUE;
            do_output = TRUE;
            tout = find_max(t - tstep, tstep, plansel, 1, 1, xxs);
            qsort((void *) xxs, (size_t) n, sizeof(double),
              (int (*)(const void *, const void *))(pos_compare));
            dmax = 0;
            for (i = 0; i < n; i++) {
              if (i == 0) {
	        d = swe_degnorm(xxs[i] - xxs[n-1]);
	      } else {
	        d = swe_degnorm(xxs[i] - xxs[i-1]);
	      }
              d = fabs(d - ang);
              if (d > maxorb) is_poly = FALSE;
              if (d > dmax)
                dmax = d;
            }   
            if (is_poly) {
              switch (strlen(plansel)) {
              case 2: strcpy(styp, "opposition"); break;
              case 3: strcpy(styp, "trine"); break;
              case 4: strcpy(styp, "square"); break;
              case 5: strcpy(styp, "pentagon"); break;
              case 6: strcpy(styp, "hexagon"); break;
              default: strcpy(styp, "polygon"); break;
              }
#ifdef BALANCE
            } else if (valsave < prmin && do_balance) {
	      strcpy(styp, "balance");
#endif
            } else 
              do_output = FALSE;
          } 
          if (!do_output 
                 && vala < valasave && valasav2 < valasave
                 && (listflag & LIST_ASP) && strlen(plansel) < 11) {
            AS_BOOL is_asp = TRUE;
            do_output = TRUE;
/*        printf("\n  %.9f %.9f %.9f\n", vala, valasave, valasav2);*/
            tout = find_max(t - tstep, tstep, plansel, 12, 0, xxs);
            qsort((void *) xxs, (size_t) n, sizeof(double),
              (int (*)(const void *, const void *))(pos_compare));
            dmax = 0;
            for (i = 0; i < n - 1; i++) {
              for (j = i + 1; j < n; j++) {
                d = swe_degnorm(xxs[j] - xxs[i]);
                while (d > 15)
                  d -= 30;
                d = fabs(d);
                if (d > maxorb) {
                    is_asp = FALSE;
                    j = i = n;
                }
                if (d > dmax)
                  dmax = d;
              }   
            }   
            if (is_asp) {
	      strcpy(styp, "aspect");
              for (i = 0; i < n; i++) {
                j = i + 1;
                if (j == n)
                  j = 0;
                d = fabs(swe_difdeg2n(xxs[j], xxs[i]));
                k = (int) (d / 30 + 0.5);
                sp = sasp + k;
                if (*sp != 'c')
                  sprintf(aspstr + strlen(aspstr), "%c", *sp);
              }
              if (strlen(aspstr) == 2)
                *(aspstr + 1) = '\0';
              if (*aspstr == '\0')
                strcpy(aspstr, "c");
              if (strlen(aspstr) > 2) {
                char c = '\0';
                if (strchr(aspstr, 'o') != NULL) {
                  c = 'o';
                } else if (strchr(aspstr, 'q') != NULL) {
                  c = 'q';
                } else if (strchr(aspstr, 't') != NULL) {
                  c = 't';
                } else if (strchr(aspstr, 's') != NULL) {
                  c = 's';
                } else if (strchr(aspstr, 'i') != NULL) {
                  c = 'i';
		}
                if (c != '\0') {
                  while(*aspstr != c) {
                    sprintf(aspstr + strlen(aspstr), "%c", *aspstr);
                    strcpy(aspstr, aspstr + 1);
                  }
                }
              }
              if (strlen(aspstr) == 1) {
                switch(*aspstr) {
                case 'c': strcpy(styp, "conjunction"); break;
                case 'o': strcpy(styp, "opposition"); break;
                case 'q': strcpy(styp, "square"); break;
                case 't': strcpy(styp, "trine"); break;
                case 's': strcpy(styp, "sextile"); break;
                case 'i': strcpy(styp, "inconjunct"); break;
                case 'h': strcpy(styp, "semisextile"); break;
                }
              }
            } else {
              do_output = FALSE;
              if (listflag == LIST_ASP && plslen != nlist) {
                int c = *plansel;
                while(strchr(plansel, c) != NULL && icomb <= ncomb) {
                  icomb++;
                  for (k = 0, *plansel = '\0'; k < plslen; k++) {
                    if (icomb & (1 << k))
                            sprintf(plansel + strlen(plansel), "%c", *(plsel + k));
                  } 
                }
                icomb--;
              }
            }
          }
          if (do_output 
			  && (*slist == '\0' 
			  || strcmp(slist, aspstr) == 0
			  || strstr(slist, aspstr) != NULL)) {
            char s[AS_MAXCH], splanout[AS_MAXCH];
            *splanout = '\0';
            swe_revjul(tout - swe_deltat(tout), SE_GREG_CAL, &ye, &mo, &da, &jut);
            *sretdir = '\0';
            for (sp = plansel, i = 0; *sp != '\0'; sp++, i++) {
	      ipl = letter_to_ipl((int) *sp);
              if (*sp == 's')
                ipl = atoi(sastno) + 10000;
              iflgret = do_calc(tout, ipl, iflag, lbr, serr);
              xxe[i] = lbr[0];
              xxeb[i] = lbr[1];
			  if (lbr[3] >= 0)
				strcat(sretdir, "d");
			  else
				strcat(sretdir, "r");
              swe_get_planet_name(ipl, s);
              sprintf(splanout + strlen(splanout), "%c%c", *s, *(s+1));
            }
            dmax2 = sqrt((xxe[1] - xxe[0]) * (xxe[1] - xxe[0]) + (xxeb[1] - xxeb[0]) * (xxeb[1] - xxeb[0]));
			printf("%02d.%02d.%d %s %14s %7s %.2f  ", 
                da, mo, ye, hms(jut,0), splanout, aspstr, dmax);
			//printf(sretdir); printf(" ");
            printf(styp);
	    printf("\n");
	    fflush(stdout);
            do_output = FALSE;
          }
	} else if (!do_print_max) {
          if (n > 3)
            printf("%f %f %f\n", t, val, valn);
	  else
            printf("%f %f\n", t, val);
	}
      }
      }
    }           /* for tjd */
    if (*serr_warn != '\0') {
      do_printf("\nwarning: ");
      do_printf(serr_warn);
      do_printf("\n");
    }
  }             /* while 1 */
  /* close open files and free allocated space */
  end_main:
  swe_close();
  return  OK;
}

static double find_max(double tjd, double tstep, char *plsel, 
                       double harm, int recursive,
                       double *xxs)
{
  int ii, i, j, ipl, iflgret, type = 0;
  double t, dtf, xxe[NPLX], lbr[6], dterr;
  double sumx, sumy, y[3], lon;
  char *sp, serr[AS_MAXCH];
  double tout = tjd, toutsv;
  double dt = fabs(tstep);
  for (ii = 0; dt > 0.001; ii++, dt /= 3) {
    for (i = 0, t = tout - dt; i < 3; i++, t += dt) {
      sumx = sumy = 0;
      for (sp = plsel, j = 0; *sp != '\0'; sp++, j++) {
        ipl = letter_to_ipl((int) *sp);
        if (*sp == 's')
          ipl = atoi(sastno) + 10000;
        iflgret = do_calc(t, ipl, iflag, lbr, serr);
        lon = swe_degnorm(lbr[0] * harm) * DEGTORAD;
        sumx += cos(lon);
        sumy += sin(lon);
      }
      y[i] = sqrt(sumx * sumx + sumy * sumy) / harm;
    }
    find_maximum(y[0], y[1], y[2], dt, &dtf, &dterr);
    toutsv = tout;
    tout += dt + dtf;
  }
  for (sp = plsel, j = 0; *sp != '\0'; sp++, j++) {
    ipl = letter_to_ipl((int) *sp);
    if (*sp == 's')
      ipl = atoi(sastno) + 10000;
    iflgret = do_calc(tout, ipl, iflag, lbr, serr);
    xxs[j] = lbr[0];
  }
  return tout;
}

static int pos_compare(double *a1, double *a2)
{
  if (*a1>= *a2) return 1;
  return -1;
}

static char *dms(double x, long iflag)
{
  int izod;
  long k, kdeg, kmin, ksec;
  char *c = ODEGREE_STRING;
  char *sp, s1[50];
  static char s[50];
  int sgn;
  *s = '\0';
  if (iflag & SEFLG_EQUATORIAL)
    c = "h";
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
    sprintf(s, " %3ld%s", kdeg, c);
  }
  x -= kdeg;
  x *= 60;
  kmin = (long) x;
  if ((iflag & BIT_ZODIAC) && (iflag & BIT_ROUND_MIN)) {
    sprintf(s1, "%2ld", kmin);
  } else {
    sprintf(s1, "%2ld'", kmin);
  }
  strcat(s, s1);
  if (iflag & BIT_ROUND_MIN)
    goto return_dms;
  x -= kmin;
  x *= 60;
  ksec = (long) x;
  if (iflag & BIT_ROUND_SEC) {
    sprintf(s1, "%2ld\"", ksec);
  } else {
    sprintf(s1, "%2ld", ksec);
  }
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

static int do_calc(double tjd, int ipl, long iflag, double *x, char *serr)
{
  if (do_nod || do_nos || do_aph || do_per) {
	double xnasc[6], xndsc[6], xperi[6], xaphe[6];
	int i;
	int32 retc = 0;
	int32 method = (int32) do_true_node;
	//retc = swe_node_aps(tjd, ipl, iflag, method, xnasc, xndsc, xperi, xaphe, serr);
    if (do_nod) { 
      for (i = 0; i <= 5; i++) x[i] = xnasc[i];
    } else if (do_nos) {
      for (i = 0; i <= 5; i++) x[i] = xndsc[i];
    } else if (do_aph) {
      for (i = 0; i <= 5; i++) x[i] = xaphe[i];
    } else if (do_per) {
      for (i = 0; i <= 5; i++) x[i] = xperi[i];
    }
    return retc;
  } else
    return swe_calc(tjd, ipl, iflag, x, serr);
}

static long do_fixstar(char *st, double tjd, long iflag, double *x, char *serr)
{
  return swe_fixstar(st, tjd, iflag, x, serr);
}

static void do_printf(char *info)
{
#ifdef _WINDOWS
  fprintf(fp, info);
#else
  printf(info);
#endif
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

static double difdeg2n (double p1, double p2)
{ double dif;
  dif = swe_degnorm(p1 - p2);
  if (dif  >= 180.0) return (dif - 360.0);
  return (dif);
}

/* find_ephemeris_path().
 1. looks for environment variable SE_EPHE_PATH
    containing a path for ephemeris files.
 2. on failure, the current working directory is checked.
 3. on failure, the program directory is checked.
 4. on failure, directory '\sweph\ephe' is checked in
    a) current working drive
    b) program drive
    c) drive C.
 */
static int find_ephemeris_path(long iflag, char *argv0)
{
  char path[AS_MAXCH];
  char s[AS_MAXCH];
  char *sp;
  char *dirglue = DIR_GLUE;
  size_t pathlen; 
  /* moshier needs no ephemeris path */
  if (iflag & SEFLG_MOSEPH)
    return OK;
  /* path from environment variable */
  sp = getenv("SE_EPHE_PATH");
  if (sp != NULL) {
    if (strlen(sp) < AS_MAXCH) {
      strcpy(path, sp);
    } else {
      do_printf("path too long (environment variable SE_EPHE_PATH):\n");
      do_printf(sp);
      exit(1);
    }
    swe_set_ephe_path(path);
    return OK;
  }
  /* try current working directory */ 
  *path = '\0';
  if (search_file(path, iflag) == OK) {
    swe_set_ephe_path(path);
    return OK;
  }
  /* no ephemeris files have been found in current directory.
   * now try program directory */
  sp = strrchr(argv0, *dirglue);
  if (sp == NULL) {
    pathlen = 0;
    *path = '\0';
  } else {
    pathlen = sp - argv0;
    if (pathlen < AS_MAXCH) {
      strncpy(path, argv0, pathlen);
      *(path+pathlen) = '\0';
    } else {
      do_printf("path too long (argv0):\n");
      do_printf(argv0);
      exit(1);
    }
  }
  if (search_file(path, iflag) == OK) {
    swe_set_ephe_path(path);
    return OK;
  }
#if MSDOS
  /* no ephemeris files have been found in program directory.
   * now search default directory (\sweph\ephe) in 
   * - current drive
   * - program drive
   * - drive C
   */
  *s = '\0';
  /* current working drive */
  sp = getcwd(NULL, 0);
  if (sp == NULL) {
    do_printf("error in getcwd()\n");
    exit(1);
  } else {
    if (*sp != 'C')
      sprintf(s, "%c", *sp);
  } 
  /* program drive */
  if (*argv0 != 'C' && (sp == NULL || *sp != *argv0))
    strncat(s, argv0, 1);
  /* drive C */
  strcat(s, "C");
  for(sp = s; *sp != '\0'; sp++) {
      sprintf(path, "%c:%s", *sp, SE_EPHE_PATH);
      if (search_file(path, iflag) == OK) {
        swe_set_ephe_path(path);
        do_printf("\nusing ephemeris files in directory: ");
        do_printf(path);
        do_printf("\n");
        return OK;
      }
  }
#else
    sprintf(path, "%s", SE_EPHE_PATH);
    if (search_file(path, iflag) == OK) {
      swe_set_ephe_path(path);
      return OK;
    }
#endif
  do_printf("\nephemeris directory not found; trying Moshier ephemeris...\n");
  return ERR;
}

#define SE_TEST_FILE    "sepl_18.se1"
#ifdef _MSC_VER
#  define MYSTAT _stat
#else
#  define MYSTAT stat
#endif
static int search_file(char *path, long iflag)
{
  struct MYSTAT statbuf;
  char fname[500];
  char *dirglue = DIR_GLUE;
  size_t pathlen;
  /* first try swiss eph */
  strcpy(fname, path);
  pathlen = strlen(fname);
  if (pathlen > 0 && *(fname + pathlen - 1) != *dirglue)
    strcat(fname, dirglue);
  strcat(fname, SE_TEST_FILE);
  if (MYSTAT(fname, &statbuf) == OK) 
    return OK;
  /* else try JPL */
  if (iflag & SEFLG_JPLEPH) {
    strcpy(fname, path);
    strcat(fname, SE_FNAME_DFT);
    if (MYSTAT(fname, &statbuf) == OK) 
      return OK;
  } 
  return ERR;
}

/* conversion from polar (l[3]) to cartesian coordinates (x[3]).
 * x = l is allowed.
 */
void polcart(double *l, double *x) 
{
  double xx[3];
  double cosl1;
  cosl1 = cos(l[1]);
  xx[0] = l[2] * cosl1 * cos(l[0]);
  xx[1] = l[2] * cosl1 * sin(l[0]);
  xx[2] = l[2] * sin(l[1]);
  x[0] = xx[0];
  x[1] = xx[1];
  x[2] = xx[2];
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
  if (yret != NULL)
    *yret = y;
  return OK;
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
    *dxret = (x1 - 1) * dx;
    *dxret2 = (x2 - 1) * dx;
  return OK;
}

static char *hms(double x, long iflag)
{
  static char s[AS_MAXCH], *sp;
  x += 0.5 / 3600.0; /* round to 1 sec */
  strcpy(s, dms(x, BIT_ROUND_SEC));
  sp = strstr(s, ODEGREE_STRING);
  if (sp != NULL) {
    *sp = ':';
    if (strlen(ODEGREE_STRING) > 1)
      strcpy_overlap(sp + 1, sp + strlen(ODEGREE_STRING));
    *(sp + 3) = ':';
    *(sp + 8) = '\0';
  }
  sp = strchr(s, '\"');
  if (sp != NULL)
    *sp = '\0';
  return s;
}
