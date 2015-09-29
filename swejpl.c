
/* 
 | $Header: /home/dieter/sweph/RCS/swejpl.c,v 1.76 2008/08/26 13:55:36 dieter Exp $
 |
 | Subroutines for reading JPL ephemerides.
 | derived from testeph.f as contained in DE403 distribution July 1995.
 | works with DE200, DE102, DE403, DE404, DE405, DE406, DE431
 | (attention, these ephemerides do not have exactly the same reference frame)

  Authors: Dieter Koch and Alois Treindl, Astrodienst Zurich

************************************************************/
/* Copyright (C) 1997 - 2008 Astrodienst AG, Switzerland.  All rights reserved.
  
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

#if MSDOS
#else
  #define _FILE_OFFSET_BITS 64
#endif

#include <string.h>
#include "swephexp.h"
#include "sweph.h"
#include "swejpl.h"

#if MSDOS
  extern __int64 __cdecl _ftelli64(FILE *);
  extern int __cdecl _fseeki64(FILE *, __int64, int);
  typedef __int64 off_t;
  #define FSEEK _fseeki64
  #define FTELL _ftelli64
#else
  #define FSEEK fseeko
  #define FTELL ftello
#endif

#define DEBUG_DO_SHOW	FALSE

/*
 * local globals
 */
struct jpl_save {
  char *jplfname;
  char *jplfpath;
  FILE *jplfptr;
  short do_reorder;
  double eh_cval[400]; 
  double eh_ss[3], eh_au, eh_emrat;
  int32 eh_denum, eh_ncon, eh_ipt[39];
  char ch_cnam[6*400];
  double pv[78];
  double pvsun[6];
  double buf[1500];
  double pc[18], vc[18], ac[18], jc[18];
  short do_km;
};

static TLS struct jpl_save *js;

static int state (double et, int32 *list, int do_bary, 
		  double *pv, double *pvsun, double *nut, char *serr);
static int interp(double *buf, double t, double intv, int32 ncfin, 
		  int32 ncmin, int32 nain, int32 ifl, double *pv);
static int32 fsizer(char *serr);
static void reorder(char *x, int size, int number);
static int read_const_jpl(double *ss, char *serr);

/* information about eh_ipt[] and buf[]
DE200	DE102		  	DE403
3	3	  ipt[0] 	3	body 0 (mercury) starts at buf[2]
12	15	  ipt[1]	14	body 0, ncf = coefficients per component
4	2	  ipt[2]	4		na = nintervals, tot 14*4*3=168
147	93	  ipt[3]	171	body 1 (venus) starts at buf[170]
12	15	  ipt[4]	10		ncf = coefficients per component
1	1	  ipt[5]	2		total 10*2*3=60
183	138	  ipt[6]	231	body 2 (earth) starts at buf[230]
15	15	  ipt[7]	13		ncf = coefficients per component
2	2	  ipt[8]	2		total 13*2*3=78
273	228	  ipt[9]	309	body 3 (mars) starts at buf[308]
10	10	  ipt[10]	11		ncf = coefficients per component
1	1	  ipt[11]	1		total 11*1*3=33
303	258	  ipt[12]	342	body 4 (jupiter) at buf[341]
9	9	  ipt[13]	8		total 8 * 1 * 3 = 24
1	1	  ipt[14]	1
330	285	  ipt[15]	366	body 5 (saturn) at buf[365]
8	8	  ipt[16]	7		total 7 * 1 * 3 = 21
1	1	  ipt[17]	1
354	309	  ipt[18]	387	body 6 (uranus) at buf[386]
8	8	  ipt[19]	6		total 6 * 1 * 3 = 18
1	1	  ipt[20]	1
378	333	  ipt[21]	405	body 7 (neptune) at buf[404]
6	6	  ipt[22]	6		total 18
1	1	  ipt[23]	1
396	351	  ipt[24]	423	body 8 (pluto) at buf[422]
6	6	  ipt[25]	6		total 18
1	1	  ipt[26]	1
414	369	  ipt[27]	441	body 9 (moon) at buf[440]
12	15	  ipt[28]	13		total 13 * 8 * 3 = 312
8	8	  ipt[29]	8
702	729	  ipt[30]	753	SBARY SUN, starts at buf[752]
15	15	  ipt[31]	11	SBARY SUN, ncf = coeff per component
1	1	  ipt[32]	2		   total 11*2*3=66
747	774	  ipt[33]	819	nutations, starts at buf[818]
10	0	  ipt[34]	10		total 10 * 4 * 2 = 80
4	0	  ipt[35]	4	(nutation only two coordinates)
0	0	  ipt[36]	899	librations, start at buf[898]
0	0	  ipt[37]	10		total 10 * 4 * 3 = 120
0	0	  ipt[38]	4
		
					last element of buf[1017]
  buf[0] contains start jd and buf[1] end jd of segment;
  each segment is 32 days in de403, 64 days in DE102, 32 days in  DE200

  Length of blocks: DE406 = 1456*4=5824 bytes = 728 double
                    DE405 = 2036*4=8144 bytes = 1018 double
		    DE404 = 1456*4=5824 bytes = 728 double
                    DE403 = 2036*4=8144 bytes = 1018 double
		    DE200 = 1652*4=6608 bytes = 826 double
		    DE102 = 1546*4=6184 bytes = 773 double
		    each DE102 record has 53*8=424 fill bytes so that
		    the records have the same length as DE200.
*/

/*
 * This subroutine opens the file jplfname, with a phony record length, 
 * reads the first record, and uses the info to compute ksize, 
 * the number of single precision words in a record. 
 * RETURN: ksize (record size of ephemeris data)
 * jplfptr is opened on return.
 * note 26-aug-2008: now record size is computed by fsizer(), not 
 * set to a fixed value depending as in previous releases. The caller of
 * fsizer() will verify by data comparison whether it computed correctly.
 */
static int32 fsizer(char *serr)
{
  /* Local variables */
  int32 ncon; 
  double emrat;
  int32 numde;
  double au, ss[3];
  int i, kmx, khi, nd;
  int32 ksize, lpt[3];
  char ttl[6*14*3];	
  if ((js->jplfptr = swi_fopen(SEI_FILE_PLANET, js->jplfname, js->jplfpath, serr)) == NULL) {
    return NOT_AVAILABLE;
  }
  /* ttl = ephemeris title, e.g.
   * "JPL Planetary Ephemeris DE404/LE404
   *  Start Epoch: JED=   625296.5-3001 DEC 21 00:00:00
   *  Final Epoch: JED=  2817168.5 3001 JAN 17 00:00:00c */
  fread((void *) &ttl[0], 1, 252, js->jplfptr);
  /* cnam = names of constants */
  fread((void *) js->ch_cnam, 1, 6*400, js->jplfptr);
  /* ss[0] = start epoch of ephemeris
   * ss[1] = end epoch
   * ss[2] = segment size in days */
  fread((void *) &ss[0], sizeof(double), 3, js->jplfptr);
  /* reorder ? */
  if (ss[2] < 1 || ss[2] > 200) 
    js->do_reorder = TRUE;
  else 
    js->do_reorder = 0;
  for (i = 0; i < 3; i++)
    js->eh_ss[i] = ss[i];
  if (js->do_reorder)
    reorder((char *) &js->eh_ss[0], sizeof(double), 3);
  /* plausibility test of these constants. Start and end date must be
   * between -20000 and +20000, segment size >= 1 and <= 200 */
  if (js->eh_ss[0] < -5583942 || js->eh_ss[1] > 9025909 || js->eh_ss[2] < 1 || js->eh_ss[2] > 200) {
    if (serr != NULL) {
      strcpy(serr, "alleged ephemeris file has invalid format.");
      if (strlen(serr) + strlen(js->jplfname) + 3 < AS_MAXCH) {
	sprintf(serr, "alleged ephemeris file (%s) has invalid format.", js->jplfname);
      }
    }
    return(NOT_AVAILABLE);
  }
  /* ncon = number of constants */
  fread((void *) &ncon, sizeof(int32), 1, js->jplfptr);
  if (js->do_reorder)
    reorder((char *) &ncon, sizeof(int32), 1);
  /* au = astronomical unit */
  fread((void *) &au, sizeof(double), 1, js->jplfptr);
  if (js->do_reorder)
    reorder((char *) &au, sizeof(double), 1);
  /* emrat = earth moon mass ratio */
  fread((void *) &emrat, sizeof(double), 1, js->jplfptr);
  if (js->do_reorder)
    reorder((char *) &emrat, sizeof(double), 1);
  /* ipt[i+0]: coefficients of planet i start at buf[ipt[i+0]-1] 
   * ipt[i+1]: number of coefficients (interpolation order - 1)
   * ipt[i+2]: number of intervals in segment */
  fread((void *) &js->eh_ipt[0], sizeof(int32), 36, js->jplfptr);
  if (js->do_reorder)
    reorder((char *) &js->eh_ipt[0], sizeof(int32), 36);
  /* numde = number of jpl ephemeris "404" with de404 */
  fread((void *) &numde, sizeof(int32), 1, js->jplfptr);
  if (js->do_reorder)
    reorder((char *) &numde, sizeof(int32), 1);
  /* read librations */
  fread(&lpt[0], sizeof(int32), 3, js->jplfptr);
  if (js->do_reorder)
    reorder((char *) &lpt[0], sizeof(int32), 3);
  /* fill librations into eh_ipt[36]..[38] */
  for (i = 0; i < 3; ++i) 
    js->eh_ipt[i + 36] = lpt[i];
  rewind(js->jplfptr);
  /*  find the number of ephemeris coefficients from the pointers */
  /* re-activated this code on 26-aug-2008 */
  kmx = 0;
  khi = 0;
  for (i = 0; i < 13; i++) {
    if (js->eh_ipt[i * 3] > kmx) {
      kmx = js->eh_ipt[i * 3];
      khi = i + 1;
    }
  }
  if (khi == 12) 
    nd = 2;
  else
    nd = 3;
  ksize = (js->eh_ipt[khi * 3 - 3] + nd * js->eh_ipt[khi * 3 - 2] * js->eh_ipt[khi * 3 - 1] - 1L) * 2L;
  /*
   * de102 files give wrong ksize, because they contain 424 empty bytes 
   * per record. Fixed by hand!
   */
  if (ksize == 1546) 
    ksize = 1652;
#if 0		/* we prefer to compute ksize to be comaptible
                   with new DE releases */
  switch (numde) {
    case 403:
    case 405:
    case 410:
    case 413:
    case 414:
    case 418:
    case 421:
      ksize = 2036;
      break;
    case 404:
    case 406:
      ksize = 1456;
      break;
    case 200:
      ksize = 1652;
      break;
    case 102:
      ksize = 1652;     /* de102 is filled with blanks to length of de200 */
      break;
    default:
      if (serr != NULL)
	sprintf(serr,"unknown numde value %d;", numde);
      return ERR;
  }
#endif
  if (ksize < 1000 || ksize > 5000) {
    if (serr != NULL)
      sprintf(serr, "JPL ephemeris file does not provide valid ksize (%d)", ksize);/**/
    return NOT_AVAILABLE;
  }
  return ksize;
} 

/*
 *     This subroutine reads the jpl planetary ephemeris 
 *     and gives the position and velocity of the point 'ntarg' 
 *     with respect to 'ncent'. 
 *     calling sequence parameters: 
 *       et = d.p. julian ephemeris date at which interpolation 
 *            is wanted. 
 *       ** note the entry dpleph for a doubly-dimensioned time ** 
 *          the reason for this option is discussed in the 
 *          subroutine state 
 *     ntarg = integer number of 'target' point. 
 *     ncent = integer number of center point. 
 *            the numbering convention for 'ntarg' and 'ncent' is: 
 *                0 = mercury           7 = neptune 
 *                1 = venus             8 = pluto 
 *                2 = earth             9 = moon 
 *                3 = mars             10 = sun 
 *                4 = jupiter          11 = solar-system barycenter 
 *                5 = saturn           12 = earth-moon barycenter 
 *                6 = uranus           13 = nutations (longitude and obliq) 
 *                                     14 = librations, if on eph file 
 *             (if nutations are wanted, set ntarg = 13. for librations, 
 *              set ntarg = 14. set ncent=0.) 
 *      rrd = output 6-word d.p. array containing position and velocity 
 *            of point 'ntarg' relative to 'ncent'. the units are au and 
 *            au/day. for librations the units are radians and radians 
 *            per day. in the case of nutations the first four words of 
 *            rrd will be set to nutations and rates, having units of 
 *            radians and radians/day. 
 *            The option is available to have the units in km and km/sec. 
 *            For this, set do_km=TRUE (default FALSE). 
 */
int swi_pleph(double et, int ntarg, int ncent, double *rrd, char *serr)
{
  int i, retc;
  int32 list[12];
  double *pv = js->pv;
  double *pvsun = js->pvsun;
  for (i = 0; i < 6; ++i) 
    rrd[i] = 0.0;
  if (ntarg == ncent) 
      return 0;
  for (i = 0; i < 12; ++i) 
    list[i] = 0;
  /*     check for nutation call */
  if (ntarg == J_NUT) {
    if (js->eh_ipt[34] > 0) {
      list[10] = 2;
      return(state(et, list, FALSE, pv, pvsun, rrd, serr));
    } else {
      if (serr != NULL) 
	sprintf(serr,"No nutations on the JPL ephemeris file;");
      return (NOT_AVAILABLE);
    }
  }
  if (ntarg == J_LIB) {
    if (js->eh_ipt[37] > 0) {
      list[11] = 2;
      if ((retc = state(et, list, FALSE, pv, pvsun, rrd, serr)) != OK)
	return (retc);
      for (i = 0; i < 6; ++i) 
	rrd[i] = pv[i + 60];
      return 0;
    } else {
      if (serr != NULL) 
	sprintf(serr,"No librations on the ephemeris file;");
      return (NOT_AVAILABLE);
    }
  }
  /* set up proper entries in 'list' array for state call */
  if (ntarg < J_SUN) 
    list[ntarg] = 2;
  if (ntarg == J_MOON) 	/* Mooon needs Earth */
    list[J_EARTH] = 2;
  if (ntarg == J_EARTH) 	/* Earth needs Moon */
    list[J_MOON] = 2;
  if (ntarg == J_EMB) 	/* EMB needs Earth */
    list[J_EARTH] = 2;
  if (ncent < J_SUN) 
    list[ncent] = 2;
  if (ncent == J_MOON) 	/* Mooon needs Earth */
    list[J_EARTH] = 2;
  if (ncent == J_EARTH) 	/* Earth needs Moon */
    list[J_MOON] = 2;
  if (ncent == J_EMB) 	/* EMB needs Earth */
    list[J_EARTH] = 2;
  if ((retc = state(et, list, TRUE, pv, pvsun, rrd, serr)) != OK)
    return (retc);
  if (ntarg == J_SUN || ncent == J_SUN) {
    for (i = 0; i < 6; ++i) 
      pv[i + 6*J_SUN] = pvsun[i];
  }
  if (ntarg == J_SBARY || ncent == J_SBARY) {
    for (i = 0; i < 6; ++i) {
      pv[i + 6*J_SBARY] = 0.;
    }
  }
  if (ntarg == J_EMB || ncent == J_EMB) {
    for (i = 0; i < 6; ++i) 
      pv[i + 6*J_EMB] = pv[i + 6*J_EARTH];
  }
  if ((ntarg==J_EARTH && ncent==J_MOON) || (ntarg == J_MOON && ncent==J_EARTH)){
    for (i = 0; i < 6; ++i) 
      pv[i + 6*J_EARTH] = 0.;
    
  } else {
    if (list[J_EARTH] == 2) {
      for (i = 0; i < 6; ++i) 
	pv[i + 6*J_EARTH] -= pv[i + 6*J_MOON] / (js->eh_emrat + 1.);
    }
    if (list[J_MOON] == 2) {
      for (i = 0; i < 6; ++i) {
	pv[i + 6*J_MOON] += pv[i + 6*J_EARTH];
      }
    }
  }
  for (i = 0; i < 6; ++i) 
    rrd[i] = pv[i + ntarg * 6] - pv[i + ncent * 6];
  return OK;
} 

/*
 *  This subroutine differentiates and interpolates a 
 *  set of chebyshev coefficients to give pos, vel, acc, and jerk 
 *  calling sequence parameters: 
 *    input: 
 *     buf   1st location of array of d.p. chebyshev coefficients of position
 *        t   is dp fractional time in interval covered by 
 *            coefficients at which interpolation is wanted, 0 <= t <= 1 
 *     intv   is dp length of whole interval in input time units. 
 *      ncf   number of coefficients per component 
 *      ncm   number of components per set of coefficients 
 *       na   number of sets of coefficients in full array 
 *            (i.e., number of sub-intervals in full interval) 
 *       ifl   int flag: =1 for positions only 
 *                      =2 for pos and vel 
 *                      =3 for pos, vel, and acc 
 *                      =4 for pos, vel, acc, and jerk 
 *    output: 
 *      pv   d.p. interpolated quantities requested. 
 *           assumed dimension is pv(ncm,fl). 
 */
static int interp(double *buf, double t, double intv, int32 ncfin, 
		  int32 ncmin, int32 nain, int32 ifl, double *pv)
{
  /* Initialized data */
  static TLS int np, nv;
  static TLS int nac;
  static TLS int njk;
  static TLS double twot = 0.;
  double *pc = js->pc;
  double *vc = js->vc;
  double *ac = js->ac;
  double *jc = js->jc;
  int ncf = (int) ncfin;
  int ncm = (int) ncmin;
  int na = (int) nain;
  /* Local variables */
  double temp;
  int i, j, ni;
  double tc;
  double dt1, bma;
  double bma2, bma3;
  /*
   | get correct sub-interval number for this set of coefficients and then
   | get normalized chebyshev time within that subinterval. 
   */
  if (t >= 0)
    dt1 = floor(t);
  else
    dt1 = -floor(-t);
  temp = na * t;
  ni = (int) (temp - dt1);
  /* tc is the normalized chebyshev time (-1 <= tc <= 1) */
  tc = (fmod(temp, 1.0) + dt1) * 2. - 1.;
  /*
   *  check to see whether chebyshev time has changed, 
   *  and compute new polynomial values if it has. 
   *  (the element pc(2) is the value of t1(tc) and hence 
   *  contains the value of tc on the previous call.) 
   */
  if (tc != pc[1]) {
    np = 2;
    nv = 3;
    nac = 4;
    njk = 5;
    pc[1] = tc;
    twot = tc + tc;
  }
  /*
   *  be sure that at least 'ncf' polynomials have been evaluated 
   *  and are stored in the array 'pc'. 
   */
  if (np < ncf) {
    for (i = np; i < ncf; ++i) 
      pc[i] = twot * pc[i - 1] - pc[i - 2];
    np = ncf;
  }
  /*  interpolate to get position for each component */
  for (i = 0; i < ncm; ++i) {
    pv[i] = 0.;
    for (j = ncf-1; j >= 0; --j) 
      pv[i] += pc[j] * buf[j + (i + ni * ncm) * ncf];
  }
  if (ifl <= 1) 
    return 0;
  /*
   *       if velocity interpolation is wanted, be sure enough 
   *       derivative polynomials have been generated and stored. 
   */
  bma = (na + na) / intv;
  vc[2] = twot + twot;
  if (nv < ncf) {
    for (i = nv; i < ncf; ++i) 
      vc[i] = twot * vc[i - 1] + pc[i - 1] + pc[i - 1] - vc[i - 2];
    nv = ncf;
  }
  /*       interpolate to get velocity for each component */
  for (i = 0; i < ncm; ++i) {
    pv[i + ncm] = 0.;
    for (j = ncf-1; j >= 1; --j) 
      pv[i + ncm] += vc[j] * buf[j + (i + ni * ncm) * ncf];
    pv[i + ncm] *= bma;
  }
  if (ifl == 2) 
    return 0;
  /*       check acceleration polynomial values, and */
  /*       re-do if necessary */
  bma2 = bma * bma;
  ac[3] = pc[1] * 24.;
  if (nac < ncf) {
    nac = ncf;
    for (i = nac; i < ncf; ++i) 
      ac[i] = twot * ac[i - 1] + vc[i - 1] * 4. - ac[i - 2];
  }
  /*       get acceleration for each component */
  for (i = 0; i < ncm; ++i) {
    pv[i + ncm * 2] = 0.;
    for (j = ncf-1; j >= 2; --j) 
      pv[i + ncm * 2] += ac[j] * buf[j + (i + ni * ncm) * ncf];
    pv[i + ncm * 2] *= bma2;
  }
  if (ifl == 3) 
      return 0;
  /*       check jerk polynomial values, and */
  /*       re-do if necessary */
  bma3 = bma * bma2;
  jc[4] = pc[1] * 192.;
  if (njk < ncf) {
    njk = ncf;
    for (i = njk; i < ncf; ++i) 
      jc[i] = twot * jc[i - 1] + ac[i - 1] * 6. - jc[i - 2];
  }
  /*       get jerk for each component */
  for (i = 0; i < ncm; ++i) {
    pv[i + ncm * 3] = 0.;
    for (j = ncf-1; j >= 3; --j) 
      pv[i + ncm * 3] += jc[j] * buf[j + (i + ni * ncm) * ncf];
    pv[i + ncm * 3] *= bma3;
  }
  return 0;
}

/*
 | ********** state ********************
 | this subroutine reads and interpolates the jpl planetary ephemeris file 
 |  calling sequence parameters: 
 |  input: 
 |     et    dp julian ephemeris epoch at which interpolation is wanted.
 |     list  12-word integer array specifying what interpolation 
 |           is wanted for each of the bodies on the file. 
 |                      list(i)=0, no interpolation for body i 
 |                             =1, position only 
 |                             =2, position and velocity 
 |            the designation of the astronomical bodies by i is: 
 |                      i = 0: mercury 
 |                        = 1: venus 
 |                        = 2: earth-moon barycenter, NOT earth! 
 |                        = 3: mars 
 |                        = 4: jupiter 
 |                        = 5: saturn 
 |                        = 6: uranus 
 |                        = 7: neptune 
 |                        = 8: pluto 
 |                        = 9: geocentric moon 
 |                        =10: nutations in longitude and obliquity 
 |                        =11: lunar librations (if on file) 
 |            If called with list = NULL, only the header records are read and
 |            stored in the global areas.
 |  do_bary   short, if true, barycentric, if false, heliocentric.
 |              only the 9 planets 0..8 are affected by it.
 |  output: 
 |       pv   dp 6 x 11 array that will contain requested interpolated 
 |            quantities.  the body specified by list(i) will have its 
 |            state in the array starting at pv(1,i).  (on any given 
 |            call, only those words in 'pv' which are affected by the 
 |            first 10 'list' entries (and by list(11) if librations are 
 |            on the file) are set.  the rest of the 'pv' array 
 |            is untouched.)  the order of components starting in 
 |            pv is: x,y,z,dx,dy,dz. 
 |            all output vectors are referenced to the earth mean 
 |            equator and equinox of epoch. the moon state is always 
 |            geocentric; the other nine states are either heliocentric 
 |            or solar-system barycentric, depending on the setting of 
 |            common flags (see below). 
 |            lunar librations, if on file, are put into pv(k,10) if 
 |            list(11) is 1 or 2. 
 |    pvsun   dp 6-word array containing the barycentric position and 
 |            velocity of the sun.
 |      nut   dp 4-word array that will contain nutations and rates, 
 |            depending on the setting of list(10).  the order of 
 |            quantities in nut is: 
 |                     d psi  (nutation in longitude) 
 |                     d epsilon (nutation in obliquity) 
 |                     d psi dot 
 |                     d epsilon dot 
 |  globals used:
 |    do_km   logical flag defining physical units of the output states. 
 |            TRUE = return km and km/sec, FALSE = return au and au/day 
 |            default value = FALSE  (km determines time unit 
 |            for nutations and librations.  angle unit is always radians.)
 */
static int state(double et, int32 *list, int do_bary, 
	  double *pv, double *pvsun, double *nut, char *serr)
{
  int i, j, k;
  int32 nseg;
  off_t flen, nb;
  double *buf = js->buf;
  double aufac, s, t, intv, ts[4];
  int32 nrecl, ksize;
  int32 nr;
  double et_mn, et_fr;
  int32 *ipt = js->eh_ipt;
  char ch_ttl[252];
  static TLS int32 irecsz;
  static TLS int32 nrl, lpt[3], ncoeffs;
  if (js->jplfptr == NULL) {
    ksize = fsizer(serr); /* the number of single precision words in a record */
    nrecl = 4;
    if (ksize == NOT_AVAILABLE)
      return NOT_AVAILABLE;
    irecsz = nrecl * ksize; 	/* record size in bytes */
    ncoeffs = ksize / 2;	/* # of coefficients, doubles */
    /* ttl = ephemeris title, e.g.
     * "JPL Planetary Ephemeris DE404/LE404
     *  Start Epoch: JED=   625296.5-3001 DEC 21 00:00:00
     *  Final Epoch: JED=  2817168.5 3001 JAN 17 00:00:00c */
    fread((void *) ch_ttl, 1, 252, js->jplfptr);
    /* cnam = names of constants */
    fread((void *) js->ch_cnam, 1, 2400, js->jplfptr);
    /* ss[0] = start epoch of ephemeris
     * ss[1] = end epoch
     * ss[2] = segment size in days */
    fread((void *) &js->eh_ss[0], sizeof(double), 3, js->jplfptr);
    if (js->do_reorder)
      reorder((char *) &js->eh_ss[0], sizeof(double), 3);
    /* ncon = number of constants */
    fread((void *) &js->eh_ncon, sizeof(int32), 1, js->jplfptr);
    if (js->do_reorder)
      reorder((char *) &js->eh_ncon, sizeof(int32), 1);
    /* au = astronomical unit */
    fread((void *) &js->eh_au, sizeof(double), 1, js->jplfptr);
    if (js->do_reorder)
      reorder((char *) &js->eh_au, sizeof(double), 1);
    /* emrat = earth moon mass ratio */
    fread((void *) &js->eh_emrat, sizeof(double), 1, js->jplfptr);
    if (js->do_reorder)
      reorder((char *) &js->eh_emrat, sizeof(double), 1);
    /* ipt[i+0]: coefficients of planet i start at buf[ipt[i+0]-1] 
     * ipt[i+1]: number of coefficients (interpolation order - 1)
     * ipt[i+2]: number of intervals in segment */
    fread((void *) &ipt[0], sizeof(int32), 36, js->jplfptr);
    if (js->do_reorder)
      reorder((char *) &ipt[0], sizeof(int32), 36);
    /* numde = number of jpl ephemeris "404" with de404 */
    fread((void *) &js->eh_denum, sizeof(int32), 1, js->jplfptr);
    if (js->do_reorder)
      reorder((char *) &js->eh_denum, sizeof(int32), 1);
    fread((void *) &lpt[0], sizeof(int32), 3, js->jplfptr);
    if (js->do_reorder)
      reorder((char *) &lpt[0], sizeof(int32), 3);
    /* cval[]:  other constants in next record */
    FSEEK(js->jplfptr, (off_t) (1L * irecsz), 0);
    fread((void *) &js->eh_cval[0], sizeof(double), 400, js->jplfptr);
    if (js->do_reorder)
      reorder((char *) &js->eh_cval[0], sizeof(double), 400);
    /* new 26-aug-2008: verify correct block size */
    for (i = 0; i < 3; ++i) 
      ipt[i + 36] = lpt[i];
    nrl = 0;
    /* is file length correct? */
    /* file length */
    FSEEK(js->jplfptr, (off_t) 0L, SEEK_END);
    flen = FTELL(js->jplfptr);
    /* # of segments in file */
    nseg = (int32) ((js->eh_ss[1] - js->eh_ss[0]) / js->eh_ss[2]);	
    /* sum of all cheby coeffs of all planets and segments */
    for(i = 0, nb = 0; i < 13; i++) {		
      k = 3;
      if (i == 11)
	k = 2;
      nb += (ipt[i*3+1] * ipt[i*3+2]) * k * nseg;
    }
    /* add start and end epochs of segments */
    nb += 2 * nseg; 
    /* doubles to bytes */
    nb *= 8;
    /* add size of header and constants section */
    nb += 2 * ksize * nrecl;
    if (flen != nb 
      /* some of our files are one record too long */
      && flen - nb != ksize * nrecl
      ) {
      if (serr != NULL) {
	sprintf(serr, "JPL ephemeris file is mutilated; length = %d instead of %d.", (unsigned int) flen, (unsigned int) nb);
	if (strlen(serr) + strlen(js->jplfname) < AS_MAXCH - 1) {
	  sprintf(serr, "JPL ephemeris file %s is mutilated; length = %d instead of %d.", js->jplfname, (unsigned int) flen, (unsigned int) nb);
	}
      }
      return(NOT_AVAILABLE);
    }
    /* check if start and end dates in segments are the same as in 
     * file header */
    FSEEK(js->jplfptr, (off_t) (2L * irecsz), 0);
    fread((void *) &ts[0], sizeof(double), 2, js->jplfptr);
    if (js->do_reorder)
      reorder((char *) &ts[0], sizeof(double), 2);
    FSEEK(js->jplfptr, (off_t) ((nseg + 2 - 1) * ((off_t) irecsz)), 0);
    fread((void *) &ts[2], sizeof(double), 2, js->jplfptr);
    if (js->do_reorder)
      reorder((char *) &ts[2], sizeof(double), 2);
    if (ts[0] != js->eh_ss[0] || ts[3] != js->eh_ss[1]) {
      if (serr != NULL)
	sprintf(serr, "JPL ephemeris file is corrupt; start/end date check failed. %.1f != %.1f || %.1f != %.1f", ts[0],js->eh_ss[0],ts[3],js->eh_ss[1]);
      return NOT_AVAILABLE;
    }
  }
  if (list == NULL) 
    return 0;
  s = et - .5;
  et_mn = floor(s);
  et_fr = s - et_mn;	/* fraction of days since previous midnight */
  et_mn += .5;	/* midnight before epoch */
  /*       error return for epoch out of range */
  if (et < js->eh_ss[0] || et > js->eh_ss[1]) {
    if (serr != NULL) 
      sprintf(serr,"jd %f outside JPL eph. range %.2f .. %.2f;", et, js->eh_ss[0], js->eh_ss[1]);
    return BEYOND_EPH_LIMITS;
  }
  /*       calculate record # and relative time in interval */
  nr = (int32) ((et_mn - js->eh_ss[0]) / js->eh_ss[2]) + 2;
  if (et_mn == js->eh_ss[1]) 
    --nr;	/* end point of ephemeris, use last record */
  t = (et_mn - ((nr - 2) * js->eh_ss[2] + js->eh_ss[0]) + et_fr) / js->eh_ss[2];
  /* read correct record if not in core */
  if (nr != nrl) {
    nrl = nr;
    if (FSEEK(js->jplfptr, (off_t) (nr * ((off_t) irecsz)), 0) != 0) {
      if (serr != NULL) 
	sprintf(serr, "Read error in JPL eph. at %f\n", et);
      return NOT_AVAILABLE;
    }
    for (k = 1; k <= ncoeffs; ++k) {
      if ( fread((void *) &buf[k - 1], sizeof(double), 1, js->jplfptr) != 1) {
	if (serr != NULL) 
	  sprintf(serr, "Read error in JPL eph. at %f\n", et);
	return NOT_AVAILABLE;
      }
      if (js->do_reorder)
	reorder((char *) &buf[k-1], sizeof(double), 1);
    }
  }
  if (js->do_km) {
    intv = js->eh_ss[2] * 86400.;
    aufac = 1.;
  } else {
    intv = js->eh_ss[2];
    aufac = 1. / js->eh_au;
  }
  /*   interpolate ssbary sun */
  interp(&buf[(int) ipt[30] - 1], t, intv, ipt[31], 3L, ipt[32], 2L, pvsun);
  for (i = 0; i < 6; ++i) {
    pvsun[i] *= aufac;
  }
  /*   check and interpolate whichever bodies are requested */
  for (i = 0; i < 10; ++i) {
    if (list[i] > 0) {
      interp(&buf[(int) ipt[i * 3] - 1], t, intv, ipt[i * 3 + 1], 3L,
	     ipt[i * 3 + 2], list[i], &pv[i * 6]);
      for (j = 0; j < 6; ++j) {
	if (i < 9 && ! do_bary) {
	  pv[j + i * 6] = pv[j + i * 6] * aufac - pvsun[j];
	} else {
	  pv[j + i * 6] *= aufac;
	}
      }
    }
  }
  /*       do nutations if requested (and if on file) */
  if (list[10] > 0 && ipt[34] > 0) {
    interp(&buf[(int) ipt[33] - 1], t, intv, ipt[34], 2L, ipt[35],
	     list[10], nut);
  }
  /*       get librations if requested (and if on file) */
  if (list[11] > 0 && ipt[37] > 0) {
    interp(&buf[(int) ipt[36] - 1], t, intv, ipt[37], 3L, ipt[38], list[1],
	    &pv[60]);
  }
  return OK;
} 

/* 
 *  this entry obtains the constants from the ephemeris file 
 *  call state to initialize the ephemeris and read in the constants 
 */
static int read_const_jpl(double *ss,  char *serr)
{
  int i, retc;
  retc = state(0.0, NULL, FALSE, NULL, NULL, NULL, serr);
  if (retc != OK)
    return (retc);
  for (i = 0; i < 3; i++)
    ss[i] = js->eh_ss[i];
#if DEBUG_DO_SHOW
  {
    static const char *bname[] = {
	"Mercury", "Venus", "EMB", "Mars", "Jupiter", "Saturn", 
	"Uranus", "Neptune", "Pluto", "Moon", "SunBary", "Nut", "Libr"};
    int j, k;
    int32 nb, nc;
    printf(" JPL TEST-EPHEMERIS program.  Version October 1995.\n");
    for (i = 0; i < 13; i++) {
      j = i * 3;
      k = 3;
      if (i == 11) k = 2;
      nb = js->eh_ipt[j+1] * js->eh_ipt[j+2] * k; 
      nc = (int32) (nb * 36525L / js->eh_ss[2] * 8L);
      printf("%s\t%d\tipt[%d]\t%3ld %2ld %2ld,\t",
	bname[i], i, j, js->eh_ipt[j], js->eh_ipt[j+1], js->eh_ipt[j+2]);
      printf("%3ld double, bytes per century = %6ld\n", nb, nc);
      fflush(stdout);
    }
    printf("%16.2f %16.2f %16.2f\n", js->eh_ss[0], js->eh_ss[1], js->eh_ss[2]);
    for (i = 0; i < js->eh_ncon; ++i) 
      printf("%.6s\t%24.16f\n", js->ch_cnam + i * 6, js->eh_cval[i]);
    fflush(stdout);
  }
#endif
  return OK;
} 

static void reorder(char *x, int size, int number) 
{
  int i, j;
  char s[8];
  char *sp1 = x;
  char *sp2 = &s[0];
  for (i = 0; i < number; i++) {
    for (j = 0; j < size; j++) 
      *(sp2 + j) = *(sp1 + size - j - 1);
    for (j = 0; j < size; j++) 
      *(sp1 + j) = *(sp2 + j);
    sp1 += size;
  }
}

void swi_close_jpl_file(void)
{
  if (js != NULL) {
    if (js->jplfptr != NULL)
      fclose(js->jplfptr);
    if (js->jplfname != NULL) 
      FREE((void *) js->jplfname);
    if (js->jplfpath != NULL) 
      FREE((void *) js->jplfpath);
    FREE((void *) js);
    js = NULL;
  }
}

int swi_open_jpl_file(double *ss, char *fname, char *fpath, char *serr)
{
  int retc = OK;
  /* if open, return */
  if (js != NULL && js->jplfptr != NULL)
    return OK;
  if ((js = (struct jpl_save *) CALLOC(1, sizeof(struct jpl_save))) == NULL
    || (js->jplfname = MALLOC(strlen(fname)+1)) == NULL
    || (js->jplfpath = MALLOC(strlen(fpath)+1)) == NULL
    ) {
    if (serr != NULL)
      strcpy(serr, "error in malloc() with JPL ephemeris.");
    return ERR;
  }
  strcpy(js->jplfname, fname);
  strcpy(js->jplfpath, fpath);
  retc = read_const_jpl(ss, serr);
  if (retc != OK) 
    swi_close_jpl_file();
  else {
    /* intializations for function interpol() */
    js->pc[0] = 1;
    js->pc[1] = 2;
    js->vc[1] = 1;
    js->ac[2] = 4;
    js->jc[3] = 24;
  }
  return retc;
}

int32 swi_get_jpl_denum()
{
  return js->eh_denum;
}

