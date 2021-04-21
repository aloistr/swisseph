/********************************************************************
sweephgen4.c	

Create ephemeris file type 4   ep4_
a fast precoomputed ephemeris used in some Astrodienst applications.

options: -fYYY	(start) file number, required option
	 -nNN	number of files to be created, default 1
	 -v	verbose: print differences (default: no)
	 -t     test by reading


File format: 
	1000 blocks of xxx bytes
File names: ep4_243, ep4_244
	corresponding to the absolute julian day number

*********************************************************************/
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


# include "swephexp.h"
# include "sweephe4.h"

# define EPHR_NPL (PLACALC_CHIRON + 1)

char *arg0;
int32 	max_dd[EP_CALC_N];	/* remember maximum of second  dfifferences */
double	max_err[EP_CALC_N];	/* remember maximum error */
AS_BOOL verbose = FALSE;
char errtext[AS_MAXCH];

int split(w, m, min, sec)
int32	w;	/* position in seconds/m */
int	m;	/* factor for seconds */
short	*min,	/* storage for degrees and minutes */
	*sec;	/* storage for seconds * m */
{
  if (w >= 0) {
    *sec = w % (60 * m);
    *min = w / (60 * m);
  } else {
    *sec = -(-w % (60 * m));
    *min = -(-w / (60 * m));
  }
  return OK;
}


/*************************************************************
Pack positions of 10 days and write to file
The longitude is packed with second differences in such a way,
that the accumulating rounding erros do not exceed half of
the last stored digit, i.e. 0.05" moon, 0.005" other planets
**************************************************************/
int eph4_pack (int32 jd, double (*l)[NDB], double ecliptic[],
	       double nutation[]) 
{
  int i, p,ps;
  int32 d1, d2, dd, d_ret, w0, w_ret;
  double err;
  struct ep4 e;
  e.j_10000 = jd / 10000.0;
  e.j_rest  = jd - 10000.0 * e.j_10000; 
  w0 = swe_d2l( ecliptic[0] * DEG);
  split( w0, 100, &e.ecl0m, &e.ecl0s );
  for (i = 1; i < NDB; i++)
    e.ecld1[i-1] =  swe_d2l(ecliptic[i] * DEG - w0);  
  for (i = 0; i < NDB; i++)
    e.nuts[i] = swe_d2l( nutation[i] * DEG );	/* int32 casted into short */
  for (p = PLACALC_SUN; p <= PLACALC_CHIRON ; p++) {
    ps = p;
    w0 = swe_d2l( l[ps][0] * DEG);
    d1 = swe_d2l( l[ps][1] * DEG - w0);
    if (d1 >= DEG180) 
      d1 -= DEG360;
    else if (d1 <= -DEG180)
      d1 += DEG360;
    split(w0, 100, &e.elo[p].p0m, &e.elo[p].p0s);
    split(d1, 100, &e.elo[p].pd1m, &e.elo[p].pd1s);
    d_ret = d1;		/* recalculated diff */
    w_ret = w0 + d_ret; /* recalculated position */
    for (i = 2; i < NDB; i++) {
      d2 = swe_d2l( l[ps][i] * DEG - w_ret);
      if (d2 >= DEG180) 
	d2 -= DEG360;
      else if (d2 <= -DEG180)
	d2 += DEG360;
      dd = d2 - d_ret;	/* second difference */
      if (p == PLACALC_MOON || p == PLACALC_MERCURY) 
	dd = swe_d2l(dd / 10.0);	/* moon only 0.1" */
      if (verbose && abs(dd) > abs(max_dd[ps]))
	max_dd[ps] = dd;
      e.elo[p].pd2[i-2] = dd;
      if (p == PLACALC_MOON || p == PLACALC_MERCURY)
	d_ret += e.elo[p].pd2[i-2] * 10L;
      else
	d_ret += e.elo[p].pd2[i-2];
      w_ret += d_ret;
      if (verbose) {
	err = swe_difdeg2n(w_ret/360000.0, l[ps][i]);	/* error */
	if (fabs(err) > fabs(max_err[ps]))
	  max_err[ps] = err;
      }
    }
  }	/* for p */
#ifdef INTEL_BYTE_ORDER
  shortreorder((UCHAR *) &e, sizeof(struct ep4));
#endif
  fwrite (&e, sizeof(struct ep4), 1, ephfp);
  return (OK);
}


/*************************************/
char *degstr (t)
double t;
{
  static char a[20];	/* must survive call */
  double min, sec;
  int ideg, imin;
  char sign = ' ';
  if ( t < 0) sign = '-';
  t =  fabs (t);
  ideg = (int) floor (t);
  min = ( t - ideg ) * 60.0;
  imin = (int) floor(min);
  sec = ( min - imin ) * 60.0;
  sprintf (a, "%c%3d %2d'%5.2f\"", sign, ideg, imin, sec);
  return (a);
} /* degstr */

/********************************************************/
int eph_test()
{
  char cal;
  int  p, jday, jmon, jyear;
  double al, jd;
  centisec *cp;
  while (TRUE) {
    printf ("date ?");
    if (scanf ("%d%d%d", &jday,&jmon,&jyear) < 1) exit(1);
    if (jyear < 1600)
      cal = 'j';
    else
      cal = 'g';
    swe_date_conversion (jyear, jmon, jday, 0, cal, &jd);
    if ((cp = ephread(jd, 0,0, errtext)) == NULL) {
      fprintf (stderr,"%s: %s", arg0, errtext);
      exit (1);
    }
    printf ("ephgen test d=%12.1f  dmy %d.%d.%d", jd, jday, jmon, jyear);
    if (cal == 'g')
      printf (" greg");
    else
      printf (" julian");
    printf ("\n\tecliptic %s ", degstr(cp[EP_ECL_INDEX]*CS2DEG));
    printf ("nutation %s\n", degstr(cp[EP_NUT_INDEX] * CS2DEG));
    for (p = 0; p <= PLACALC_CHIRON; p++) {
      al = cp[p] * CS2DEG;
      printf ("%2d%18s\n", p, degstr(al));
    }
  }
}	/* end ephtest */

int main(int argc, char **argv)
{
  int day, i, n, p;
  char serr[AS_MAXCH];
  double l[EPHR_NPL][NDB], ecliptic[NDB], nutation[NDB];
  double jd0, jd;
  double x[6];
  int32 jlong;
  int file;
  int nfiles = 1;
  int fnr = -10000;
  int32 iflagret;
  arg0 = argv[0];
  for (i = 1; i < argc; i++) {
    if (strncmp(argv[i], "-f", 2) == 0) {
      fnr = atoi (argv[i] + 2);
      if (fnr < -20 || fnr > 300) {
	printf("file number out of range -20 ... 300");
	exit (1);
      }
    } 
    if (strncmp(argv[i], "-n", 2) == 0) {
      nfiles = atoi (argv[i] + 2);
    }
    if (strncmp(argv[i], "-v", 2) == 0) {
      verbose = TRUE;
    }
    if (strncmp(argv[i], "-t", 2) == 0) {
      eph_test();
      exit(0);
    }
  }
  if (fnr == -10000) {
    fprintf(stderr,"missing file number -fNNN\n");
    exit(1);
  }
  for (file = fnr; file < fnr + nfiles; file++) {
    if (file > fnr) printf ("\n");
    printf ("file = %d\n", file);
    jd0 = EP4_NDAYS * file + 0.5;
    jlong = floor(jd0);
    if (eph4_posit (jlong, TRUE, errtext) != OK) {
      fprintf (stderr,"%s: %s", arg0, errtext);
      exit(1);
    }
    for (n = 0; n < EP4_NDAYS; n += NDB, jd0 += NDB) {
      if (n % 500 == 0) {
	if ( n > 0 && verbose) {
	  printf ("\ndd");
	  for (p = 0; p < 11; p++) {
	    printf("%6d ",max_dd[p]);
	    max_dd[p] = 0;
	  }
	  printf("\ner");
	  for (p = 0; p < 11; p++) {
	    printf("%6.3f ",max_err[p] * 3600);
	    max_err[p] = 0;
	  }
	}
	printf ("\n%d ", n);
      } else {
	printf (".");
      }
      fflush( stdout );
      for (day = 0; day < NDB; day++) { /* compute positions for 10 days */
	jd = jd0 + day;
	for (p = PLACALC_SUN; p <= EP_CALC_N; p++) {
	  if ((iflagret = swe_calc(jd, ephe_plac2swe(p), 0, x, serr)) == ERR) {
	    swe_close();
	    printf("error in swe_calc() %s\n", serr);
	    exit (1);
	  } 
	  l[p][day] = x[0];
	}
	if ((iflagret = swe_calc(jd, SE_ECL_NUT, 0, x, serr)) == ERR) {
	  swe_close();
	  printf("error in swe_calc() %s\n", serr);
	  exit (1);
	}
	ecliptic[day] = x[0];
	nutation[day] = x[2];
      }
      jlong = floor(jd0);
      eph4_pack (jlong, l, ecliptic, nutation);
    }	
    putchar('\n');
    fclose (ephfp);
    ephfp = NULL;
  }	/* for file */
  swe_close();
  return(0);
}	/* end main */
