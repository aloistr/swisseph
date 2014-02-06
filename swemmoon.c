/* SWISSEPH
   $Header: /home/dieter/sweph/RCS/swemmoon.c,v 1.74 2008/06/16 10:07:20 dieter Exp $
 *
 * Steve Moshier's analytical lunar ephemeris

**************************************************************/
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


 /*
 * Expansions for the geocentric ecliptic longitude,
 * latitude, and distance of the Moon referred to the mean equinox
 * and ecliptic of date.
 *
 * This version of cmoon.c adjusts the ELP2000-85 analytical Lunar
 * theory of Chapront-Touze and Chapront to fit the Jet Propulsion
 * Laboratory's DE404 long ephemeris on the interval from 3000 B.C.
 * to 3000 A.D.
 *
 * The fit is much better in the remote past and future if
 * secular terms are included in the arguments of the oscillatory
 * perturbations.  Such adjustments cannot easily be incorporated
 * into the 1991 lunar tables.  In this program the traditional
 * literal arguments are used instead, with mean elements adjusted
 * for a best fit to the reference ephemeris.
 *
 * This program omits many oscillatory terms from the analytical
 * theory which, if they were included, would yield a much higher
 * accuracy for modern dates.  Detailed statistics of the precision
 * are given in the table below.  Comparing at 64-day intervals
 * over the period -3000 to +3000, the maximum discrepancies noted
 * were 7" longitude, 5" latitude, and 5 x 10^-8 au radius.
 * The expressions used for precession in this comparision were
 * those of Simon et al (1994).
 * 
 * The adjusted coefficients were found by an unweighted least squares
 * fit to the numerical ephemeris in the mentioned test interval.
 * The approximation error increases rapidly outside this interval.
 * J. Chapront (1994) has described the basic fitting procedure.
 *
 * A major change from DE200 to DE404 is in the coefficient
 * of tidal acceleration of the Moon, which causes the Moon's
 * longitude to depart by about -0.9" per century squared
 * from DE200.  Uncertainty in this quantity continues to
 * be the limiting factor in long term projections of the Moon's
 * ephemeris.
 *
 * Since the Lunar theory is cast in the ecliptic of date, it makes
 * some difference what formula you use for precession.  The adjustment
 * to DE404 was carried out relative to the mean equinox and ecliptic
 * of date as defined in Williams (1994).  An earlier version of this
 * program used the precession given by Simon et al (1994).  The difference
 * between these two precession formulas amounts to about 12" in Lunar
 * longitude at 3000 B.C.
 *
 *    Maximum deviations between DE404 and this program
 *    in a set of 34274 samples spaced 64 days apart
 *
 *   Interval     Longitude  Latitude  Radius
 *   Julian Year   arc sec   arc sec   10^-8 au             
 * -3000 to -2500    5.66      4.66     4.93
 * -2500 to -2000    5.49      3.98     4.56
 * -2000 to -1500    6.98      4.17     4.81
 * -1500 to -1000    5.74      3.53     4.87
 * -1000 to -500     5.95      3.42     4.67
 * -500 to     0     4.94      3.07     4.04
 *    0 to   500     4.42      2.65     4.55
 *  500 to  1000     5.68      3.30     3.99
 * 1000 to  1500     4.32      3.21     3.83
 * 1500 to  2000     2.70      2.69     3.71
 * 2000 to  2500     3.35      2.32     3.85
 * 2500 to  3000     4.62      2.39     4.11
 *
 *
 *
 * References:
 *
 *   James G. Williams, "Contributions to the Earth's obliquity rate,
 *   precession, and nutation,"  Astron. J. 108, 711-724 (1994)
 *
 *   DE403 and DE404 ephemerides by E. M. Standish, X. X. Newhall, and
 *   J. G. Williams are at the JPL computer site navigator.jpl.nasa.gov.
 *
 *   J. L. Simon, P. Bretagnon, J. Chapront, M. Chapront-Touze', G. Francou,
 *   and J. Laskar, "Numerical Expressions for precession formulae and
 *   mean elements for the Moon and the planets," Astronomy and Astrophysics
 *   282, 663-683 (1994)
 *
 *   P. Bretagnon and Francou, G., "Planetary theories in rectangular
 *   and spherical variables. VSOP87 solutions," Astronomy and
 *   Astrophysics 202, 309-315 (1988)
 *
 *   M. Chapront-Touze' and J. Chapront, "ELP2000-85: a semi-analytical
 *   lunar ephemeris adequate for historical times," Astronomy and
 *   Astrophysics 190, 342-352 (1988).
 *
 *   M. Chapront-Touze' and J. Chapront, _Lunar Tables and
 *   Programs from 4000 B.C. to A.D. 8000_, Willmann-Bell (1991)
 *
 *   J. Laskar, "Secular terms of classical planetary theories
 *   using the results of general theory," Astronomy and Astrophysics
 *   157, 59070 (1986)
 *
 *   S. L. Moshier, "Comparison of a 7000-year lunar ephemeris
 *   with analytical theory," Astronomy and Astrophysics 262,
 *   613-616 (1992)
 *
 *   J. Chapront, "Representation of planetary ephemerides by frequency
 *   analysis.  Application to the five outer planets,"  Astronomy and
 *   Astrophysics Suppl. Ser. 109, 181-192 (1994)
 *
 *
 * Entry swi_moshmoon2() returns the geometric position of the Moon
 * relative to the Earth.  Its calling procedure is as follows:
 *
 * double JD;       input Julian Ephemeris Date
 * double pol[3];   output ecliptic polar coordinatees in radians and au
 *                  pol[0] longitude, pol[1] latitude, pol[2] radius
 * swi_moshmoon2( JD, pol );
 *
 * - S. L. Moshier, August, 1991
 * DE200 fit: July, 1992
 * DE404 fit: October, 1995
 *
 * Dieter Koch: adaptation to SWISSEPH, April 1996
 * 18-feb-2006  replaced LP by SWELP because of name collision
 */

#include <string.h>
#include "swephexp.h"
#include "sweph.h"
#include "swephlib.h"

static void mean_elements(void);
static void mean_elements_pl(void);
static double mods3600(double x);
static void ecldat_equ2000(double tjd, double *xpm);
static void chewm(short *pt, int nlines, int nangles, 
  				     int typflg, double *ans );
static void sscc(int k, double arg, int n );
static void moon1(void);
static void moon2(void);
static void moon3(void);
static void moon4(void);


#ifdef MOSH_MOON_200
/* The following coefficients were calculated by a simultaneous least
 * squares fit between the analytical theory and the continued DE200
 * numerically integrated ephemeris from 9000 BC to 13000 AD.
 * See references to the array z[] later on in the program.
 * The 71 coefficients were estimated from 42,529 Lunar positions.
 */
static double z[] = {
-1.225346551567e+001, /* F, t^2 */
-1.096676093208e-003, /* F, t^3 */
-2.165750777942e-006, /* F, t^4 */
-2.790392351314e-009, /* F, t^5 */
 4.189032191814e-011, /* F, t^6 */
 4.474984866301e-013, /* F, t^7 */
 3.239398410335e+001, /* l, t^2 */
 5.185305877294e-002, /* l, t^3 */
-2.536291235258e-004, /* l, t^4 */
-2.506365935364e-008, /* l, t^5 */
 3.452144225877e-011, /* l, t^6 */
-1.755312760154e-012, /* l, t^7 */
-5.870522364514e+000, /* D, t^2 */
 6.493037519768e-003, /* D, t^3 */
-3.702060118571e-005, /* D, t^4 */
 2.560078201452e-009, /* D, t^5 */
 2.555243317839e-011, /* D, t^6 */
-3.207663637426e-013, /* D, t^7 */
-4.776684245026e+000, /* L, t^2 */
 6.580112707824e-003, /* L, t^3 */
-6.073960534117e-005, /* L, t^4 */
-1.024222633731e-008, /* L, t^5 */
 2.235210987108e-010, /* L, t^6 */
 7.200592540556e-014, /* L, t^7 */
-8.552017636339e+001, /* t^2 cos(18V - 16E - l) */
-2.055794304596e+002, /* t^2 sin(18V - 16E - l) */
-1.097555241866e+000, /* t^3 cos(18V - 16E - l) */
 5.219423171002e-001, /* t^3 sin(18V - 16E - l) */
 2.088802640755e-003, /* t^4 cos(18V - 16E - l) */
 4.616541527921e-003, /* t^4 sin(18V - 16E - l) */
 4.794930645807e+000, /* t^2 cos(10V - 3E - l) */
-4.595134364283e+001, /* t^2 sin(10V - 3E - l) */
-6.659812174691e-002, /* t^3 cos(10V - 3E - l) */
-2.570048828246e-001, /* t^3 sin(10V - 3E - l) */
 6.229863046223e-004, /* t^4 cos(10V - 3E - l) */
 5.504368344700e-003, /* t^4 sin(10V - 3E - l) */
-3.084830597278e+000, /* t^2 cos(8V - 13E) */
-1.000471012253e+001, /* t^2 sin(8V - 13E) */
 6.590112074510e-002, /* t^3 cos(8V - 13E) */
-3.212573348278e-003, /* t^3 sin(8V - 13E) */
 5.409038312567e-004, /* t^4 cos(8V - 13E) */
 1.293377988163e-003, /* t^4 sin(8V - 13E) */
 2.311794636111e+001, /* t^2 cos(4E - 8M + 3J) */
-3.157036220040e+000, /* t^2 sin(4E - 8M + 3J) */
-3.019293162417e+000, /* t^2 cos(18V - 16E) */
-9.211526858975e+000, /* t^2 sin(18V - 16E) */
-4.993704215784e-002, /* t^3 cos(18V - 16E) */
 2.991187525454e-002, /* t^3 sin(18V - 16E) */
-3.827414182969e+000, /* t^2 cos(18V - 16E - 2l) */
-9.891527703219e+000, /* t^2 sin(18V - 16E - 2l) */
-5.322093802878e-002, /* t^3 cos(18V - 16E - 2l) */
 3.164702647371e-002, /* t^3 sin(18V - 16E - 2l) */
 7.713905234217e+000, /* t^2 cos(2J - 5S) */
-6.077986950734e+000, /* t^3 sin(2J - 5S) */
-1.278232501462e-001, /* t^2 cos(L - F) */
 4.760967236383e-001, /* t^2 sin(L - F) */
-6.759005756460e-001, /* t^3 sin(l') */
 1.655727996357e-003, /* t^4 sin(l') */
 1.646526117252e-001, /* t^3 sin(2D - l') */
-4.167078100233e-004, /* t^4 sin(2D - l') */
 2.067529538504e-001, /* t^3 sin(2D - l' - l) */
-5.219127398748e-004, /* t^4 sin(2D - l' - l) */
-1.526335222289e-001, /* t^3 sin(l' - l) */
-1.120545131358e-001, /* t^3 sin(l' + l) */
 4.619472391553e-002, /* t^3 sin(2D - 2l') */
 4.863621236157e-004, /* t^4 sin(2D - 2l') */
-4.280059182608e-002, /* t^3 sin(2l') */
-4.328378207833e-004, /* t^4 sin(2l') */
-8.371028286974e-003, /* t^3 sin(2D - l) */
 4.089447328174e-002, /* t^3 sin(2D - 2l' - l) */
-1.238363006354e-002, /* t^3 sin(2D + 2l' - l) */
};
#else
/* The following coefficients were calculated by a simultaneous least
 * squares fit between the analytical theory and DE404 on the finite
 * interval from -3000 to +3000.
 * The coefficients were estimated from 34,247 Lunar positions.
 */
static double FAR z[] = {
/* The following are scaled in arc seconds, time in Julian centuries.
   They replace the corresponding terms in the mean elements.  */
-1.312045233711e+01, /* F, t^2 */
-1.138215912580e-03, /* F, t^3 */
-9.646018347184e-06, /* F, t^4 */
 3.146734198839e+01, /* l, t^2 */
 4.768357585780e-02, /* l, t^3 */
-3.421689790404e-04, /* l, t^4 */
-6.847070905410e+00, /* D, t^2 */
-5.834100476561e-03, /* D, t^3 */
-2.905334122698e-04, /* D, t^4 */
-5.663161722088e+00, /* L, t^2 */
 5.722859298199e-03, /* L, t^3 */
-8.466472828815e-05, /* L, t^4 */
/* The following longitude terms are in arc seconds times 10^5.  */
-8.429817796435e+01, /* t^2 cos(18V - 16E - l) */
-2.072552484689e+02, /* t^2 sin(18V - 16E - l) */
 7.876842214863e+00, /* t^2 cos(10V - 3E - l) */
 1.836463749022e+00, /* t^2 sin(10V - 3E - l) */
-1.557471855361e+01, /* t^2 cos(8V - 13E) */
-2.006969124724e+01, /* t^2 sin(8V - 13E) */
 2.152670284757e+01, /* t^2 cos(4E - 8M + 3J) */
-6.179946916139e+00, /* t^2 sin(4E - 8M + 3J) */
-9.070028191196e-01, /* t^2 cos(18V - 16E) */
-1.270848233038e+01, /* t^2 sin(18V - 16E) */
-2.145589319058e+00, /* t^2 cos(2J - 5S) */
 1.381936399935e+01, /* t^2 sin(2J - 5S) */
-1.999840061168e+00, /* t^3 sin(l') */
};
#endif	/* ! MOSH_MOON_200 */

/* Perturbation tables
 */
#define NLR 118
static short FAR LR[8*NLR] = {
/*
               Longitude    Radius
 D  l' l  F    1"  .0001"  1km  .0001km */

 0, 0, 1, 0, 22639, 5858,-20905,-3550,
 2, 0,-1, 0,  4586, 4383, -3699,-1109,
 2, 0, 0, 0,  2369, 9139, -2955,-9676,
 0, 0, 2, 0,   769,  257,  -569,-9251,
 0, 1, 0, 0,  -666,-4171,    48, 8883,
 0, 0, 0, 2,  -411,-5957,    -3,-1483,
 2, 0,-2, 0,   211, 6556,   246, 1585,
 2,-1,-1, 0,   205, 4358,  -152,-1377,
 2, 0, 1, 0,   191, 9562,  -170,-7331,
 2,-1, 0, 0,   164, 7285,  -204,-5860,
 0, 1,-1, 0,  -147,-3213,  -129,-6201,
 1, 0, 0, 0,  -124,-9881,   108, 7427,
 0, 1, 1, 0,  -109,-3803,   104, 7552,
 2, 0, 0,-2,    55, 1771,    10, 3211,
 0, 0, 1, 2,   -45, -996,     0,    0,
 0, 0, 1,-2,    39, 5333,    79, 6606,
 4, 0,-1, 0,    38, 4298,   -34,-7825,
 0, 0, 3, 0,    36, 1238,   -23,-2104,
 4, 0,-2, 0,    30, 7726,   -21,-6363,
 2, 1,-1, 0,   -28,-3971,    24, 2085,
 2, 1, 0, 0,   -24,-3582,    30, 8238,
 1, 0,-1, 0,   -18,-5847,    -8,-3791,
 1, 1, 0, 0,    17, 9545,   -16,-6747,
 2,-1, 1, 0,    14, 5303,   -12,-8314,
 2, 0, 2, 0,    14, 3797,   -10,-4448,
 4, 0, 0, 0,    13, 8991,   -11,-6500,
 2, 0,-3, 0,    13, 1941,    14, 4027,
 0, 1,-2, 0,    -9,-6791,    -7,  -27,
 2, 0,-1, 2,    -9,-3659,     0, 7740,
 2,-1,-2, 0,     8, 6055,    10,  562,
 1, 0, 1, 0,    -8,-4531,     6, 3220,
 2,-2, 0, 0,     8,  502,    -9,-8845,
 0, 1, 2, 0,    -7,-6302,     5, 7509,
 0, 2, 0, 0,    -7,-4475,     1,  657,
 2,-2,-1, 0,     7, 3712,    -4,-9501,
 2, 0, 1,-2,    -6,-3832,     4, 1311,
 2, 0, 0, 2,    -5,-7416,     0,    0,
 4,-1,-1, 0,     4, 3740,    -3,-9580,
 0, 0, 2, 2,    -3,-9976,     0,    0,
 3, 0,-1, 0,    -3,-2097,     3, 2582,
 2, 1, 1, 0,    -2,-9145,     2, 6164,
 4,-1,-2, 0,     2, 7319,    -1,-8970,
 0, 2,-1, 0,    -2,-5679,    -2,-1171,
 2, 2,-1, 0,    -2,-5212,     2, 3536,
 2, 1,-2, 0,     2, 4889,     0, 1437,
 2,-1, 0,-2,     2, 1461,     0, 6571,
 4, 0, 1, 0,     1, 9777,    -1,-4226,
 0, 0, 4, 0,     1, 9337,    -1,-1169,
 4,-1, 0, 0,     1, 8708,    -1,-5714,
 1, 0,-2, 0,    -1,-7530,    -1,-7385,
 2, 1, 0,-2,    -1,-4372,     0,-1357,
 0, 0, 2,-2,    -1,-3726,    -4,-4212,
 1, 1, 1, 0,     1, 2618,     0,-9333,
 3, 0,-2, 0,    -1,-2241,     0, 8624,
 4, 0,-3, 0,     1, 1868,     0,-5142,
 2,-1, 2, 0,     1, 1770,     0,-8488,
 0, 2, 1, 0,    -1,-1617,     1, 1655,
 1, 1,-1, 0,     1,  777,     0, 8512,
 2, 0, 3, 0,     1,  595,     0,-6697,
 2, 0, 1, 2,     0,-9902,     0,    0,
 2, 0,-4, 0,     0, 9483,     0, 7785,
 2,-2, 1, 0,     0, 7517,     0,-6575,
 0, 1,-3, 0,     0,-6694,     0,-4224,
 4, 1,-1, 0,     0,-6352,     0, 5788,
 1, 0, 2, 0,     0,-5840,     0, 3785,
 1, 0, 0,-2,     0,-5833,     0,-7956,
 6, 0,-2, 0,     0, 5716,     0,-4225,
 2, 0,-2,-2,     0,-5606,     0, 4726,
 1,-1, 0, 0,     0,-5569,     0, 4976,
 0, 1, 3, 0,     0,-5459,     0, 3551,
 2, 0,-2, 2,     0,-5357,     0, 7740,
 2, 0,-1,-2,     0, 1790,     8, 7516,
 3, 0, 0, 0,     0, 4042,    -1,-4189,
 2,-1,-3, 0,     0, 4784,     0, 4950,
 2,-1, 3, 0,     0,  932,     0, -585,
 2, 0, 2,-2,     0,-4538,     0, 2840,
 2,-1,-1, 2,     0,-4262,     0,  373,
 0, 0, 0, 4,     0, 4203,     0,    0,
 0, 1, 0, 2,     0, 4134,     0,-1580,
 6, 0,-1, 0,     0, 3945,     0,-2866,
 2,-1, 0, 2,     0,-3821,     0,    0,
 2,-1, 1,-2,     0,-3745,     0, 2094,
 4, 1,-2, 0,     0,-3576,     0, 2370,
 1, 1,-2, 0,     0, 3497,     0, 3323,
 2,-3, 0, 0,     0, 3398,     0,-4107,
 0, 0, 3, 2,     0,-3286,     0,    0,
 4,-2,-1, 0,     0,-3087,     0,-2790,
 0, 1,-1,-2,     0, 3015,     0,    0,
 4, 0,-1,-2,     0, 3009,     0,-3218,
 2,-2,-2, 0,     0, 2942,     0, 3430,
 6, 0,-3, 0,     0, 2925,     0,-1832,
 2, 1, 2, 0,     0,-2902,     0, 2125,
 4, 1, 0, 0,     0,-2891,     0, 2445,
 4,-1, 1, 0,     0, 2825,     0,-2029,
 3, 1,-1, 0,     0, 2737,     0,-2126,
 0, 1, 1, 2,     0, 2634,     0,    0,
 1, 0, 0, 2,     0, 2543,     0,    0,
 3, 0, 0,-2,     0,-2530,     0, 2010,
 2, 2,-2, 0,     0,-2499,     0,-1089,
 2,-3,-1, 0,     0, 2469,     0,-1481,
 3,-1,-1, 0,     0,-2314,     0, 2556,
 4, 0, 2, 0,     0, 2185,     0,-1392,
 4, 0,-1, 2,     0,-2013,     0, 0,
 0, 2,-2, 0,     0,-1931,     0, 0,
 2, 2, 0, 0,     0,-1858,     0, 0,
 2, 1,-3, 0,     0, 1762,     0, 0,
 4, 0,-2, 2,     0,-1698,     0, 0,
 4,-2,-2, 0,     0, 1578,     0,-1083,
 4,-2, 0, 0,     0, 1522,     0,-1281,
 3, 1, 0, 0,     0, 1499,     0,-1077,
 1,-1,-1, 0,     0,-1364,     0, 1141,
 1,-3, 0, 0,     0,-1281,     0, 0,
 6, 0, 0, 0,     0, 1261,     0, -859,
 2, 0, 2, 2,     0,-1239,     0, 0,
 1,-1, 1, 0,     0,-1207,     0, 1100,
 0, 0, 5, 0,     0, 1110,     0, -589,
 0, 3, 0, 0,     0,-1013,     0,  213,
 4,-1,-3, 0,     0,  998,     0, 0,
};


#ifdef MOSH_MOON_200
#define NMB 56
static short FAR MB[6*NMB] = {
/*
               Latitude
 D  l' l  F    1"  .0001" */

 0, 0, 0, 1,18461, 2387,
 0, 0, 1, 1, 1010, 1671,
 0, 0, 1,-1,  999, 6936,
 2, 0, 0,-1,  623, 6524,
 2, 0,-1, 1,  199, 4837,
 2, 0,-1,-1,  166, 5741,
 2, 0, 0, 1,  117, 2607,
 0, 0, 2, 1,   61, 9120,
 2, 0, 1,-1,   33, 3572,
 0, 0, 2,-1,   31, 7597,
 2,-1, 0,-1,   29, 5766,
 2, 0,-2,-1,   15, 5663,
 2, 0, 1, 1,   15, 1216,
 2, 1, 0,-1,  -12, -941,
 2,-1,-1, 1,    8, 8681,
 2,-1, 0, 1,    7, 9586,
 2,-1,-1,-1,    7, 4346,
 0, 1,-1,-1,   -6,-7314,
 4, 0,-1,-1,    6, 5796,
 0, 1, 0, 1,   -6,-4601,
 0, 0, 0, 3,   -6,-2965,
 0, 1,-1, 1,   -5,-6324,
 1, 0, 0, 1,   -5,-3684,
 0, 1, 1, 1,   -5,-3113,
 0, 1, 1,-1,   -5, -759,
 0, 1, 0,-1,   -4,-8396,
 1, 0, 0,-1,   -4,-8057,
 0, 0, 3, 1,    3, 9841,
 4, 0, 0,-1,    3, 6745,
 4, 0,-1, 1,    2, 9985,
 0, 0, 1,-3,    2, 7986,
 4, 0,-2, 1,    2, 4139,
 2, 0, 0,-3,    2, 1863,
 2, 0, 2,-1,    2, 1462,
 2,-1, 1,-1,    1, 7660,
 2, 0,-2, 1,   -1,-6244,
 0, 0, 3,-1,    1, 5813,
 2, 0, 2, 1,    1, 5198,
 2, 0,-3,-1,    1, 5156,
 2, 1,-1, 1,   -1,-3178,
 2, 1, 0, 1,   -1,-2643,
 4, 0, 0, 1,    1, 1919,
 2,-1, 1, 1,    1, 1346,
 2,-2, 0,-1,    1,  859,
 0, 0, 1, 3,   -1, -194,
 2, 1, 1,-1,    0,-8227,
 1, 1, 0,-1,    0, 8042,
 1, 1, 0, 1,    0, 8026,
 0, 1,-2,-1,    0,-7932,
 2, 1,-1,-1,    0,-7910,
 1, 0, 1, 1,    0,-6674,
 2,-1,-2,-1,    0, 6502,
 0, 1, 2, 1,    0,-6388,
 4, 0,-2,-1,    0, 6337,
 4,-1,-1,-1,    0, 5958,
 1, 0, 1,-1,    0,-5889,
};
#else
#define NMB 77
static short FAR MB[6*NMB] = {
/*
               Latitude
 D  l' l  F    1"  .0001" */

 0, 0, 0, 1,18461, 2387,
 0, 0, 1, 1, 1010, 1671,
 0, 0, 1,-1,  999, 6936,
 2, 0, 0,-1,  623, 6524,
 2, 0,-1, 1,  199, 4837,
 2, 0,-1,-1,  166, 5741,
 2, 0, 0, 1,  117, 2607,
 0, 0, 2, 1,   61, 9120,
 2, 0, 1,-1,   33, 3572,
 0, 0, 2,-1,   31, 7597,
 2,-1, 0,-1,   29, 5766,
 2, 0,-2,-1,   15, 5663,
 2, 0, 1, 1,   15, 1216,
 2, 1, 0,-1,  -12, -941,
 2,-1,-1, 1,    8, 8681,
 2,-1, 0, 1,    7, 9586,
 2,-1,-1,-1,    7, 4346,
 0, 1,-1,-1,   -6,-7314,
 4, 0,-1,-1,    6, 5796,
 0, 1, 0, 1,   -6,-4601,
 0, 0, 0, 3,   -6,-2965,
 0, 1,-1, 1,   -5,-6324,
 1, 0, 0, 1,   -5,-3684,
 0, 1, 1, 1,   -5,-3113,
 0, 1, 1,-1,   -5, -759,
 0, 1, 0,-1,   -4,-8396,
 1, 0, 0,-1,   -4,-8057,
 0, 0, 3, 1,    3, 9841,
 4, 0, 0,-1,    3, 6745,
 4, 0,-1, 1,    2, 9985,
 0, 0, 1,-3,    2, 7986,
 4, 0,-2, 1,    2, 4139,
 2, 0, 0,-3,    2, 1863,
 2, 0, 2,-1,    2, 1462,
 2,-1, 1,-1,    1, 7660,
 2, 0,-2, 1,   -1,-6244,
 0, 0, 3,-1,    1, 5813,
 2, 0, 2, 1,    1, 5198,
 2, 0,-3,-1,    1, 5156,
 2, 1,-1, 1,   -1,-3178,
 2, 1, 0, 1,   -1,-2643,
 4, 0, 0, 1,    1, 1919,
 2,-1, 1, 1,    1, 1346,
 2,-2, 0,-1,    1,  859,
 0, 0, 1, 3,   -1, -194,
 2, 1, 1,-1,    0,-8227,
 1, 1, 0,-1,    0, 8042,
 1, 1, 0, 1,    0, 8026,
 0, 1,-2,-1,    0,-7932,
 2, 1,-1,-1,    0,-7910,
 1, 0, 1, 1,    0,-6674,
 2,-1,-2,-1,    0, 6502,
 0, 1, 2, 1,    0,-6388,
 4, 0,-2,-1,    0, 6337,
 4,-1,-1,-1,    0, 5958,
 1, 0, 1,-1,    0,-5889,
 4, 0, 1,-1,    0, 4734,
 1, 0,-1,-1,    0,-4299,
 4,-1, 0,-1,    0, 4149,
 2,-2, 0, 1,    0, 3835,
 3, 0, 0,-1,    0,-3518,
 4,-1,-1, 1,    0, 3388,
 2, 0,-1,-3,    0, 3291,
 2,-2,-1, 1,    0, 3147,
 0, 1, 2,-1,    0,-3129,
 3, 0,-1,-1,    0,-3052,
 0, 1,-2, 1,    0,-3013,
 2, 0, 1,-3,    0,-2912,
 2,-2,-1,-1,    0, 2686,
 0, 0, 4, 1,    0, 2633,
 2, 0,-3, 1,    0, 2541,
 2, 0,-1, 3,    0,-2448,
 2, 1, 1, 1,    0,-2370,
 4,-1,-2, 1,    0, 2138,
 4, 0, 1, 1,    0, 2126,
 3, 0,-1, 1,    0,-2059,
 4, 1,-1,-1,    0,-1719,
};
#endif	/* ! MOSH_MOON_200 */

#define NLRT 38
static short FAR LRT[8*NLRT] = {
/*
Multiply by T
               Longitude    Radius
 D  l' l  F   .1"  .00001" .1km  .00001km */

 0, 1, 0, 0,    16, 7680,    -1,-2302,
 2,-1,-1, 0,    -5,-1642,     3, 8245,
 2,-1, 0, 0,    -4,-1383,     5, 1395,
 0, 1,-1, 0,     3, 7115,     3, 2654,
 0, 1, 1, 0,     2, 7560,    -2,-6396,
 2, 1,-1, 0,     0, 7118,     0,-6068,
 2, 1, 0, 0,     0, 6128,     0,-7754,
 1, 1, 0, 0,     0,-4516,     0, 4194,
 2,-2, 0, 0,     0,-4048,     0, 4970,
 0, 2, 0, 0,     0, 3747,     0, -540,
 2,-2,-1, 0,     0,-3707,     0, 2490,
 2,-1, 1, 0,     0,-3649,     0, 3222,
 0, 1,-2, 0,     0, 2438,     0, 1760,
 2,-1,-2, 0,     0,-2165,     0,-2530,
 0, 1, 2, 0,     0, 1923,     0,-1450,
 0, 2,-1, 0,     0, 1292,     0, 1070,
 2, 2,-1, 0,     0, 1271,     0,-6070,
 4,-1,-1, 0,     0,-1098,     0,  990,
 2, 0, 0, 0,     0, 1073,     0,-1360,
 2, 0,-1, 0,     0,  839,     0, -630,
 2, 1, 1, 0,     0,  734,     0, -660,
 4,-1,-2, 0,     0, -688,     0,  480,
 2, 1,-2, 0,     0, -630,     0,    0,
 0, 2, 1, 0,     0,  587,     0, -590,
 2,-1, 0,-2,     0, -540,     0, -170,
 4,-1, 0, 0,     0, -468,     0,  390,
 2,-2, 1, 0,     0, -378,     0,  330,
 2, 1, 0,-2,     0,  364,     0,    0,
 1, 1, 1, 0,     0, -317,     0,  240,
 2,-1, 2, 0,     0, -295,     0,  210,
 1, 1,-1, 0,     0, -270,     0, -210,
 2,-3, 0, 0,     0, -256,     0,  310,
 2,-3,-1, 0,     0, -187,     0,  110,
 0, 1,-3, 0,     0,  169,     0,  110,
 4, 1,-1, 0,     0,  158,     0, -150,
 4,-2,-1, 0,     0, -155,     0,  140,
 0, 0, 1, 0,     0,  155,     0, -250,
 2,-2,-2, 0,     0, -148,     0, -170,
};

#define NBT 16
static short FAR BT[5*NBT] = {
/*
Multiply by T
             Latitude
 D  l' l  F  .00001"  */

 2,-1, 0,-1, -7430,
 2, 1, 0,-1,  3043,
 2,-1,-1, 1, -2229,
 2,-1, 0, 1, -1999,
 2,-1,-1,-1, -1869,
 0, 1,-1,-1,  1696,
 0, 1, 0, 1,  1623,
 0, 1,-1, 1,  1418,
 0, 1, 1, 1,  1339,
 0, 1, 1,-1,  1278,
 0, 1, 0,-1,  1217,
 2,-2, 0,-1,  -547,
 2,-1, 1,-1,  -443,
 2, 1,-1, 1,   331,
 2, 1, 0, 1,   317,
 2, 0, 0,-1,   295,
};

#define NLRT2 25
static short FAR LRT2[6*NLRT2] = {
/*
Multiply by T^2
           Longitude    Radius
 D  l' l  F  .00001" .00001km   */

 0, 1, 0, 0,  487,   -36,
 2,-1,-1, 0, -150,   111,
 2,-1, 0, 0, -120,   149,
 0, 1,-1, 0,  108,    95,
 0, 1, 1, 0,   80,   -77,
 2, 1,-1, 0,   21,   -18,
 2, 1, 0, 0,   20,   -23,
 1, 1, 0, 0,  -13,    12,
 2,-2, 0, 0,  -12,    14,
 2,-1, 1, 0,  -11,     9,
 2,-2,-1, 0,  -11,     7,
 0, 2, 0, 0,   11,     0,
 2,-1,-2, 0,   -6,    -7,
 0, 1,-2, 0,    7,     5,
 0, 1, 2, 0,    6,    -4,
 2, 2,-1, 0,    5,    -3,
 0, 2,-1, 0,    5,     3,
 4,-1,-1, 0,   -3,     3,
 2, 0, 0, 0,    3,    -4,
 4,-1,-2, 0,   -2,     0,
 2, 1,-2, 0,   -2,     0,
 2,-1, 0,-2,   -2,     0,
 2, 1, 1, 0,    2,    -2,
 2, 0,-1, 0,    2,     0,
 0, 2, 1, 0,    2,     0,
};

#define NBT2 12
static short FAR BT2[5*NBT2] = {
/*
Multiply by T^2
           Latitiude
 D  l' l  F  .00001" */

 2,-1, 0,-1,  -22,
 2, 1, 0,-1,    9,
 2,-1, 0, 1,   -6,
 2,-1,-1, 1,   -6,
 2,-1,-1,-1,   -5,
 0, 1, 0, 1,    5,
 0, 1,-1,-1,    5,
 0, 1, 1, 1,    4,
 0, 1, 1,-1,    4,
 0, 1, 0,-1,    4,
 0, 1,-1, 1,    4,
 2,-2, 0,-1,   -2,
};

/* corrections for mean lunar node in degrees, from -13100 to 17200,
 * in 100-year steps. corrections are set to 0 between the years 0 and 3000 */
static double mean_node_corr[] = {
-2.56,
-2.473, -2.392347, -2.316425, -2.239639, -2.167764, -2.095100, -2.024810, -1.957622, -1.890097, -1.826389,
-1.763335, -1.701047, -1.643016, -1.584186, -1.527309, -1.473352, -1.418917, -1.367736, -1.317202, -1.267269,
-1.221121, -1.174218, -1.128862, -1.086214, -1.042998, -1.002491, -0.962635, -0.923176, -0.887191, -0.850403,
-0.814929, -0.782117, -0.748462, -0.717241, -0.686598, -0.656013, -0.628726, -0.600460, -0.573219, -0.548634,
-0.522931, -0.499285, -0.476273, -0.452978, -0.432663, -0.411386, -0.390788, -0.372825, -0.353681, -0.336230,
-0.319520, -0.302343, -0.287794, -0.272262, -0.257166, -0.244534, -0.230635, -0.218126, -0.206365, -0.194000,
-0.183876, -0.172782, -0.161877, -0.153254, -0.143371, -0.134501, -0.126552, -0.117932, -0.111199, -0.103716,
-0.096160, -0.090718, -0.084046, -0.078007, -0.072959, -0.067235, -0.062990, -0.058102, -0.053070, -0.049786,
-0.045381, -0.041317, -0.038165, -0.034501, -0.031871, -0.028844, -0.025701, -0.024018, -0.021427, -0.018881,
-0.017291, -0.015186, -0.013755, -0.012098, -0.010261, -0.009688, -0.008218, -0.006670, -0.005979, -0.004756,
-0.003991, -0.002996, -0.001974, -0.001975, -0.001213, -0.000377, -0.000356, 5.779e-05, 0.000378, 0.000710,
0.001092, 0.000767, 0.000985, 0.001443, 0.001069, 0.001141, 0.001321, 0.001462, 0.001695, 0.001319,
0.001567, 0.001873, 0.001376, 0.001336, 0.001347, 0.001330, 0.001256, 0.000813, 0.000946, 0.001079,
#if 0
0.000509, 0.000375, 0.000477, 0.000321, 0.000279, 5.998e-05, 0.000251, 0.000623, 0.000180, 0.000225,
0.000506, 0.000331, 0.000253, 4.156e-05, 0.000247, 0.000394, -9.294e-05, -2.738e-05, 0.000140, -6.193e-05,
-0.000232, -0.000361, -0.000152, -3.571e-05, -0.000395, -0.000218, 0.000127, -0.000125, -0.000254, -0.000339,
#endif
0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
-0.000364, -0.000452, -0.001091, -0.001159, -0.001136, -0.001798, -0.002249, -0.002622, -0.002990, -0.003555,
-0.004425, -0.004758, -0.005134, -0.006065, -0.006839, -0.007474, -0.008283, -0.009411, -0.010786, -0.011810,
-0.012989, -0.014825, -0.016426, -0.017922, -0.019774, -0.021881, -0.024194, -0.026190, -0.028440, -0.031285,
-0.033817, -0.036318, -0.039212, -0.042456, -0.045799, -0.048994, -0.052710, -0.056948, -0.061017, -0.065181,
-0.069843, -0.074922, -0.079976, -0.085052, -0.090755, -0.096840, -0.102797, -0.108939, -0.115568, -0.122636,
-0.129593, -0.136683, -0.144641, -0.152825, -0.161044, -0.169758, -0.178916, -0.188712, -0.198401, -0.208312,
-0.219395, -0.230407, -0.241577, -0.253508, -0.265640, -0.278556, -0.291330, -0.304353, -0.318815, -0.332882,
-0.347316, -0.362895, -0.378421, -0.395061, -0.411748, -0.428666, -0.447477, -0.465636, -0.484277, -0.504600,
-0.524405, -0.545533, -0.567020, -0.588404, -0.612099, -0.634965, -0.658262, -0.683866, -0.708526, -0.734719,
-0.761800, -0.788562, -0.818092, -0.846885, -0.876177, -0.908385, -0.939371, -0.972027, -1.006149, -1.039634,
-1.076135, -1.112156, -1.148490, -1.188312, -1.226761, -1.266821, -1.309156, -1.350583, -1.395223, -1.440028,
-1.485047, -1.534104, -1.582023, -1.631506, -1.684031, -1.735687, -1.790421, -1.846039, -1.901951, -1.961872,
-2.021179, -2.081987, -2.146259, -2.210031, -2.276609, -2.344904, -2.413795, -2.486559, -2.559564, -2.634215,
-2.712692, -2.791289, -2.872533, -2.956217, -3.040965, -3.129234, -3.218545, -3.309805, -3.404827, -3.5008,
-3.601, -3.7, -3.8,
};

/* corrections for mean lunar apsides in degrees, from -13100 to 17200,
 * in 100-year steps. corrections are set to 0 between the years 0 and 3000 */
static double mean_apsis_corr[] = {
7.525,
7.290, 7.057295, 6.830813, 6.611723, 6.396775, 6.189569, 5.985968, 5.788342, 5.597304, 5.410167,
5.229946, 5.053389, 4.882187, 4.716494, 4.553532, 4.396734, 4.243718, 4.094282, 3.950865, 3.810366,
3.674978, 3.543284, 3.414270, 3.290526, 3.168775, 3.050904, 2.937541, 2.826189, 2.719822, 2.616193,
2.515431, 2.419193, 2.323782, 2.232545, 2.143635, 2.056803, 1.974913, 1.893874, 1.816201, 1.741957,
1.668083, 1.598335, 1.529645, 1.463016, 1.399693, 1.336905, 1.278097, 1.220965, 1.165092, 1.113071,
1.060858, 1.011007, 0.963701, 0.916523, 0.872887, 0.829596, 0.788486, 0.750017, 0.711177, 0.675589,
0.640303, 0.605303, 0.573490, 0.541113, 0.511482, 0.483159, 0.455210, 0.430305, 0.404643, 0.380782,
0.358524, 0.335405, 0.315244, 0.295131, 0.275766, 0.259223, 0.241586, 0.225890, 0.210404, 0.194775,
0.181573, 0.167246, 0.154514, 0.143435, 0.131131, 0.121648, 0.111835, 0.102474, 0.094284, 0.085204,
0.078240, 0.070697, 0.063696, 0.058894, 0.052390, 0.047632, 0.043129, 0.037823, 0.034143, 0.029188,
0.025648, 0.021972, 0.018348, 0.017127, 0.013989, 0.011967, 0.011003, 0.007865, 0.007033, 0.005574,
0.004060, 0.003699, 0.002465, 0.002889, 0.002144, 0.001018, 0.001757, -9.67e-05, -0.000734, -0.000392,
-0.001546, -0.000863, -0.001266, -0.000933, -0.000503, -0.001304, 0.000238, -0.000507, -0.000897, 0.000647,
#if 0
-0.000247, 0.000938, 0.001373, 0.001159, 0.001644, 0.000691, 0.001454, 0.000532, -0.000249, 0.000871,
-0.000210, 0.000171, 0.000702, 0.000389, 0.000609, -0.000250, 0.000426, 0.000123, -0.000339, 0.001200,
0.000413, 0.000612, 0.001169, 0.000163, 0.000553, -0.000330, -0.000498, -0.000224, -0.000948, 0.000863,
#endif
0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 
0.000514, 0.000683, 0.002228, 0.001974, 0.003485, 0.004280, 0.005409, 0.007468, 0.007938, 0.011012,
0.012525, 0.013757, 0.016757, 0.017932, 0.020780, 0.023416, 0.026386, 0.030428, 0.033512, 0.038789,
0.043126, 0.047778, 0.054175, 0.058891, 0.065878, 0.072345, 0.079668, 0.088238, 0.095307, 0.104873,
0.113533, 0.122336, 0.133205, 0.142922, 0.154871, 0.166488, 0.179234, 0.193928, 0.207262, 0.223089,
0.238736, 0.254907, 0.273232, 0.291085, 0.311046, 0.331025, 0.351955, 0.374422, 0.396341, 0.420772,
0.444867, 0.469984, 0.497448, 0.524717, 0.554752, 0.584581, 0.616272, 0.649744, 0.682947, 0.719405,
0.755834, 0.793780, 0.833875, 0.873893, 0.917340, 0.960429, 1.005471, 1.052384, 1.099317, 1.149508,
1.200130, 1.253038, 1.307672, 1.363480, 1.422592, 1.481900, 1.544111, 1.607982, 1.672954, 1.741025,
1.809727, 1.882038, 1.955243, 2.029956, 2.108428, 2.186805, 2.268697, 2.352071, 2.437370, 2.525903,
2.615415, 2.709082, 2.804198, 2.901704, 3.002606, 3.104412, 3.210406, 3.317733, 3.428386, 3.541634,
3.656634, 3.775988, 3.896306, 4.020480, 4.146814, 4.275356, 4.408257, 4.542282, 4.681174, 4.822524,
4.966424, 5.114948, 5.264973, 5.419906, 5.577056, 5.737688, 5.902347, 6.069138, 6.241065, 6.415155,
6.593317, 6.774853, 6.959322, 7.148845, 7.340334, 7.537156, 7.737358, 7.940882, 8.149932, 8.361576,
8.579150, 8.799591, 9.024378, 9.254584, 9.487362, 9.726535, 9.968784, 10.216089, 10.467716, 10.725293,
10.986, 11.25, 11.52,
};

/* The following times are set up by update() and refer
 * to the same instant.  The distinction between them
 * is required by altaz().
 */
static double FAR ss[5][8]; 
static double FAR cc[5][8];

static double l;		/* Moon's ecliptic longitude */
static double B;		/* Ecliptic latitude */

static double moonpol[3];

/* Orbit calculation begins.
 */
static double SWELP;
static double M;
static double MP;
static double D;
static double NF;
static double T;
static double T2;

static double T3;
static double T4;
static double f;
static double g;
static double Ve;
static double Ea;
static double Ma;
static double Ju;
static double Sa;
static double cg;
static double sg;
static double l1;
static double l2;
static double l3;
static double l4;

/* Calculate geometric coordinates of Moon
 * without light time or nutation correction.
 */
int swi_moshmoon2(double J, double *pol)
{
int i;
T = (J-J2000)/36525.0;
T2 = T*T;
mean_elements();
mean_elements_pl();
moon1();
moon2();
moon3();
moon4();
for( i=0; i<3; i++ ) 
  pol[i] = moonpol[i];
return(0);
}

/* Moshier's moom
 * tjd		julian day
 * xpm		array of 6 doubles for moon's position and speed vectors
 * serr		pointer to error string
 */
int swi_moshmoon(double tjd, AS_BOOL do_save, double *xpmret, char *serr) 
{
  int i;
  double a, b, x1[6], x2[6], t;
  double xx[6], *xpm;
  struct plan_data *pdp = &swed.pldat[SEI_MOON];
  char s[AS_MAXCH];
  if (do_save)
    xpm = pdp->x;
  else
    xpm = xx;
  /* allow 0.2 day tolerance so that true node interval fits in */
  if (tjd < MOSHLUEPH_START - 0.2 || tjd > MOSHLUEPH_END + 0.2) {
    if (serr != NULL) {
      sprintf(s, "jd %f outside Moshier's Moon range %.2f .. %.2f ",
		    tjd, MOSHLUEPH_START, MOSHLUEPH_END);
      if (strlen(serr) + strlen(s) < AS_MAXCH)
	strcat(serr, s);
    }
    return(ERR);
  }  
  /* if moon has already been computed */
  if (tjd == pdp->teval && pdp->iephe == SEFLG_MOSEPH) {
    if (xpmret != NULL)
      for (i = 0; i <= 5; i++)
	xpmret[i] = pdp->x[i];
    return(OK);
  }
  /* else compute moon */
  swi_moshmoon2(tjd, xpm);
  if (do_save) {
    pdp->teval = tjd;
    pdp->xflgs = -1;
    pdp->iephe = SEFLG_MOSEPH;
  }
  /* Moshier moon is referred to ecliptic of date. But we need 
   * equatorial positions for several reasons.
   * e.g. computation of earth from emb and moon
   *                  of heliocentric moon
   * Besides, this helps to keep the program structure simpler 
   */
  ecldat_equ2000(tjd, xpm);
  /* speed */
  /* from 2 other positions. */
  /* one would be good enough for computation of osculating node, 
   * but not for osculating apogee */
  t = tjd + MOON_SPEED_INTV;
  swi_moshmoon2(t, x1);
  ecldat_equ2000(t, x1);
  t = tjd - MOON_SPEED_INTV;
  swi_moshmoon2(t, x2);
  ecldat_equ2000(t, x2);
  for (i = 0; i <= 2; i++) {
#if 0
    xpm[i+3] = (x1[i] - x2[i]) / MOON_SPEED_INTV / 2;
#else
    b = (x1[i] - x2[i]) / 2;
    a = (x1[i] + x2[i]) / 2 - xpm[i];
    xpm[i+3] = (2 * a + b) / MOON_SPEED_INTV;
#endif
  }
  if (xpmret != NULL)
    for (i = 0; i <= 5; i++)
      xpmret[i] = xpm[i];
  return(OK);
}

#ifdef MOSH_MOON_200
static void  moon1()
{
double a;

sscc( 0, STR*D, 6 );
sscc( 1, STR*M,  4 );
sscc( 2, STR*MP, 4 );
sscc( 3, STR*NF, 4 );

moonpol[0] = 0.0;
moonpol[1] = 0.0;
moonpol[2] = 0.0;

/* terms in T^2, scale 1.0 = 10^-5" */
chewm( LRT2, NLRT2, 4, 2, moonpol );
chewm( BT2, NBT2, 4, 4, moonpol );

f = 18 * Ve - 16 * Ea;

g = STR*(f - MP );  /* 18V - 16E - l */
cg = cos(g);
sg = sin(g);
l = 6.367278 * cg + 12.747036 * sg;  /* t^0 */
l1 = 23123.70 * cg - 10570.02 * sg;  /* t^1 */
l2 = z[24] * cg + z[25] * sg;        /* t^2 */
l3 = z[26] * cg + z[27] * sg;        /* t^3 */
l4 = z[28] * cg + z[29] * sg;        /* t^4 */
moonpol[2] += 5.01 * cg + 2.72 * sg;

g = STR * (10.*Ve - 3.*Ea - MP);
cg = cos(g);
sg = sin(g);
l += -0.253102 * cg + 0.503359 * sg;
l1 += 1258.46 * cg + 707.29 * sg;
l2 += z[30] * cg + z[31] * sg;
l3 += z[32] * cg + z[33] * sg;
l4 += z[34] * cg + z[35] * sg;

g = STR*(8.*Ve - 13.*Ea);
cg = cos(g);
sg = sin(g);
l += -0.187231 * cg - 0.127481 * sg;
l1 += -319.87 * cg - 18.34 * sg;
l2 += z[36] * cg + z[37] * sg;
l3 += z[38] * cg + z[39] * sg;
l4 += z[40] * cg + z[41] * sg;

a = 4.0*Ea - 8.0*Ma + 3.0*Ju;
g = STR * a;
cg = cos(g);
sg = sin(g);
l += -0.866287 * cg + 0.248192 * sg;
l1 += 41.87 * cg + 1053.97 * sg;
l2 += z[42] * cg + z[43] * sg;

g = STR*(a - MP);
cg = cos(g);
sg = sin(g);
l += -0.165009 * cg + 0.044176 * sg;
l1 += 4.67 * cg + 201.55 * sg;


g = STR*f;  /* 18V - 16E */
cg = cos(g);
sg = sin(g);
l += 0.330401 * cg + 0.661362 * sg;
l1 += 1202.67 * cg - 555.59 * sg;
l2 += z[44] * cg + z[45] * sg;
l3 += z[46] * cg + z[47] * sg;

g = STR*(f - 2.0*MP );  /* 18V - 16E - 2l */
cg = cos(g);
sg = sin(g);
l += 0.352185 * cg + 0.705041 * sg;
l1 += 1283.59 * cg - 586.43 * sg;
l2 += z[48] * cg + z[49] * sg;
l3 += z[50] * cg + z[51] * sg;

g = STR * (2.0*Ju - 5.0*Sa);
cg = cos(g);
sg = sin(g);
l += -0.034700 * cg + 0.160041 * sg;
l2 += z[52] * cg + z[53] * sg;

g = STR * (SWELP - NF);
cg = cos(g);
sg = sin(g);
l += 0.000116 * cg + 7.063040 * sg;
l1 +=  298.8 * sg;
l2 += z[54] * cg + z[55] * sg;


/* T^3 terms */
sg = sin( STR * M );
l3 +=  z[56] * sg;
l4 +=  z[57] * sg;

g = STR * (2.0*D - M);
sg = sin(g);
cg = cos(g);
l3 +=  z[58] * sg;
l4 +=  z[59] * sg;
moonpol[2] +=  -0.2655 * cg * T;

g = g - STR * MP;
sg = sin(g);
l3 +=  z[60] * sg;
l4 +=  z[61] * sg;

g = STR * (M - MP);
l3 +=  z[62] * sin( g );
moonpol[2] +=  -0.1568 * cos( g ) * T;

g = STR * (M + MP);
l3 +=  z[63] * sin( g );
moonpol[2] +=  0.1309 * cos( g ) * T;

g = STR * 2.0 * (D - M);
sg = sin(g);
l3 +=  z[64] * sg;
l4 +=  z[65] * sg;

g = STR * 2.0 * M;
sg = sin(g);
l3 +=  z[66] * sg;
l4 +=  z[67] * sg;

g = STR * (2.0*D - MP);
sg = sin(g);
l3 +=  z[68] * sg;

g = STR * (2.0*(D - M) - MP);
sg = sin(g);
l3 +=  z[69] * sg;

g = STR * (2.0*(D + M) - MP);
sg = sin(g);
cg = cos(g);
l3 +=  z[70] * sg;
moonpol[2] +=   0.5568 * cg * T;

l2 += moonpol[0];

g = STR*(2.0*D - M - MP);
moonpol[2] +=  -0.1910 * cos( g ) * T;


moonpol[1] *= T;
moonpol[2] *= T;

/* terms in T */
moonpol[0] = 0.0;
chewm( BT, NBT, 4, 4, moonpol );
chewm( LRT, NLRT, 4, 1, moonpol );
g = STR*(f - MP - NF - 2355767.6); /* 18V - 16E - l - F */
moonpol[1] +=  -1127. * sin(g);
g = STR*(f - MP + NF - 235353.6); /* 18V - 16E - l + F */
moonpol[1] +=  -1123. * sin(g);
g = STR*(Ea + D + 51987.6);
moonpol[1] +=  1303. * sin(g);
g = STR*SWELP;
moonpol[1] +=  342. * sin(g);


g = STR*(2.*Ve - 3.*Ea);
cg = cos(g);
sg = sin(g);
l +=  -0.343550 * cg - 0.000276 * sg;
l1 +=  105.90 * cg + 336.53 * sg;

g = STR*(f - 2.*D); /* 18V - 16E - 2D */
cg = cos(g);
sg = sin(g);
l += 0.074668 * cg + 0.149501 * sg;
l1 += 271.77 * cg - 124.20 * sg;

g = STR*(f - 2.*D - MP);
cg = cos(g);
sg = sin(g);
l += 0.073444 * cg + 0.147094 * sg;
l1 += 265.24 * cg - 121.16 * sg;

g = STR*(f + 2.*D - MP);
cg = cos(g);
sg = sin(g);
l += 0.072844 * cg + 0.145829 * sg;
l1 += 265.18 * cg - 121.29 * sg;

g = STR*(f + 2.*(D - MP));
cg = cos(g);
sg = sin(g);
l += 0.070201 * cg + 0.140542 * sg;
l1 += 255.36 * cg - 116.79 * sg;

g = STR*(Ea + D - NF);
cg = cos(g);
sg = sin(g);
l += 0.288209 * cg - 0.025901 * sg;
l1 += -63.51 * cg - 240.14 * sg;

g = STR*(2.*Ea - 3.*Ju + 2.*D - MP);
cg = cos(g);
sg = sin(g);
l += 0.077865 * cg + 0.438460 * sg;
l1 += 210.57 * cg + 124.84 * sg;

g = STR*(Ea - 2.*Ma);
cg = cos(g);
sg = sin(g);
l += -0.216579 * cg + 0.241702 * sg;
l1 += 197.67 * cg + 125.23 * sg;

g = STR*(a + MP);
cg = cos(g);
sg = sin(g);
l += -0.165009 * cg + 0.044176 * sg;
l1 += 4.67 * cg + 201.55 * sg;

g = STR*(a + 2.*D - MP);
cg = cos(g);
sg = sin(g);
l += -0.133533 * cg + 0.041116 * sg;
l1 +=  6.95 * cg + 187.07 * sg;

g = STR*(a - 2.*D + MP);
cg = cos(g);
sg = sin(g);
l += -0.133430 * cg + 0.041079 * sg;
l1 +=  6.28 * cg + 169.08 * sg;

g = STR*(3.*Ve - 4.*Ea);
cg = cos(g);
sg = sin(g);
l += -0.175074 * cg + 0.003035 * sg;
l1 +=  49.17 * cg + 150.57 * sg;

g = STR*(2.*(Ea + D - MP) - 3.*Ju + 213534.);
l1 +=  158.4 * sin(g);
l1 += moonpol[0];

a = 0.1 * T; /* set amplitude scale of 1.0 = 10^-4 arcsec */
moonpol[1] *= a;
moonpol[2] *= a;
}
#else
static void moon1()
{
double a;
/* This code added by Bhanu Pinnamaneni, 17-aug-2009 */
/* Note by Dieter: Bhanu noted that ss and cc are not sufficiently
 * initialised and random values are used for the calculation.
 * However, this may be only part of the bug.
 * The bug could be in sscc(). Or may be the bug is rather in
 * the 116th line of NLR, where the value "5" may be wrong.
 * Still, this will make a maximum difference of only 0.1", while the error
 * of the Moshier lunar ephemeris can reach 7". */
int i, j;
for (i = 0; i < 5; i++) {
  for (j = 0; j < 8; j++) {
    ss[i][j] = 0;
    cc[i][j] = 0;
  }
}
/* End of code addition */
sscc( 0, STR*D, 6 );
sscc( 1, STR*M,  4 );
sscc( 2, STR*MP, 4 );
sscc( 3, STR*NF, 4 );
moonpol[0] = 0.0;
moonpol[1] = 0.0;
moonpol[2] = 0.0;
/* terms in T^2, scale 1.0 = 10^-5" */
chewm( LRT2, NLRT2, 4, 2, moonpol );
chewm( BT2, NBT2, 4, 4, moonpol );
f = 18 * Ve - 16 * Ea;
g = STR*(f - MP );  /* 18V - 16E - l */
cg = cos(g);
sg = sin(g);
l = 6.367278 * cg + 12.747036 * sg;  /* t^0 */
l1 = 23123.70 * cg - 10570.02 * sg;  /* t^1 */
l2 = z[12] * cg + z[13] * sg;        /* t^2 */
moonpol[2] += 5.01 * cg + 2.72 * sg;
g = STR * (10.*Ve - 3.*Ea - MP);
cg = cos(g);
sg = sin(g);
l += -0.253102 * cg + 0.503359 * sg;
l1 += 1258.46 * cg + 707.29 * sg;
l2 += z[14] * cg + z[15] * sg;
g = STR*(8.*Ve - 13.*Ea);
cg = cos(g);
sg = sin(g);
l += -0.187231 * cg - 0.127481 * sg;
l1 += -319.87 * cg - 18.34 * sg;
l2 += z[16] * cg + z[17] * sg;
a = 4.0*Ea - 8.0*Ma + 3.0*Ju;
g = STR * a;
cg = cos(g);
sg = sin(g);
l += -0.866287 * cg + 0.248192 * sg;
l1 += 41.87 * cg + 1053.97 * sg;
l2 += z[18] * cg + z[19] * sg;
g = STR*(a - MP);
cg = cos(g);
sg = sin(g);
l += -0.165009 * cg + 0.044176 * sg;
l1 += 4.67 * cg + 201.55 * sg;
g = STR*f;  /* 18V - 16E */
cg = cos(g);
sg = sin(g);
l += 0.330401 * cg + 0.661362 * sg;
l1 += 1202.67 * cg - 555.59 * sg;
l2 += z[20] * cg + z[21] * sg;
g = STR*(f - 2.0*MP );  /* 18V - 16E - 2l */
cg = cos(g);
sg = sin(g);
l += 0.352185 * cg + 0.705041 * sg;
l1 += 1283.59 * cg - 586.43 * sg;
g = STR * (2.0*Ju - 5.0*Sa);
cg = cos(g);
sg = sin(g);
l += -0.034700 * cg + 0.160041 * sg;
l2 += z[22] * cg + z[23] * sg;
g = STR * (SWELP - NF);
cg = cos(g);
sg = sin(g);
l += 0.000116 * cg + 7.063040 * sg;
l1 +=  298.8 * sg;
/* T^3 terms */
sg = sin( STR * M );
/* l3 +=  z[24] * sg;			moshier! l3 not initialized! */
l3 =  z[24] * sg;			
l4 = 0;					
g = STR * (2.0*D - M);
sg = sin(g);
cg = cos(g);
moonpol[2] +=  -0.2655 * cg * T;
g = STR * (M - MP);
moonpol[2] +=  -0.1568 * cos( g ) * T;
g = STR * (M + MP);
moonpol[2] +=  0.1309 * cos( g ) * T;
g = STR * (2.0*(D + M) - MP);
sg = sin(g);
cg = cos(g);
moonpol[2] +=   0.5568 * cg * T;
l2 += moonpol[0];
g = STR*(2.0*D - M - MP);
moonpol[2] +=  -0.1910 * cos( g ) * T;
moonpol[1] *= T;
moonpol[2] *= T;
/* terms in T */
moonpol[0] = 0.0;
chewm( BT, NBT, 4, 4, moonpol );
chewm( LRT, NLRT, 4, 1, moonpol );
g = STR*(f - MP - NF - 2355767.6); /* 18V - 16E - l - F */
moonpol[1] +=  -1127. * sin(g);
g = STR*(f - MP + NF - 235353.6); /* 18V - 16E - l + F */
moonpol[1] +=  -1123. * sin(g);
g = STR*(Ea + D + 51987.6);
moonpol[1] +=  1303. * sin(g);
g = STR*SWELP;
moonpol[1] +=  342. * sin(g);
g = STR*(2.*Ve - 3.*Ea);
cg = cos(g);
sg = sin(g);
l +=  -0.343550 * cg - 0.000276 * sg;
l1 +=  105.90 * cg + 336.53 * sg;
g = STR*(f - 2.*D); /* 18V - 16E - 2D */
cg = cos(g);
sg = sin(g);
l += 0.074668 * cg + 0.149501 * sg;
l1 += 271.77 * cg - 124.20 * sg;
g = STR*(f - 2.*D - MP);
cg = cos(g);
sg = sin(g);
l += 0.073444 * cg + 0.147094 * sg;
l1 += 265.24 * cg - 121.16 * sg;
g = STR*(f + 2.*D - MP);
cg = cos(g);
sg = sin(g);
l += 0.072844 * cg + 0.145829 * sg;
l1 += 265.18 * cg - 121.29 * sg;
g = STR*(f + 2.*(D - MP));
cg = cos(g);
sg = sin(g);
l += 0.070201 * cg + 0.140542 * sg;
l1 += 255.36 * cg - 116.79 * sg;
g = STR*(Ea + D - NF);
cg = cos(g);
sg = sin(g);
l += 0.288209 * cg - 0.025901 * sg;
l1 += -63.51 * cg - 240.14 * sg;
g = STR*(2.*Ea - 3.*Ju + 2.*D - MP);
cg = cos(g);
sg = sin(g);
l += 0.077865 * cg + 0.438460 * sg;
l1 += 210.57 * cg + 124.84 * sg;
g = STR*(Ea - 2.*Ma);
cg = cos(g);
sg = sin(g);
l += -0.216579 * cg + 0.241702 * sg;
l1 += 197.67 * cg + 125.23 * sg;
g = STR*(a + MP);
cg = cos(g);
sg = sin(g);
l += -0.165009 * cg + 0.044176 * sg;
l1 += 4.67 * cg + 201.55 * sg;
g = STR*(a + 2.*D - MP);
cg = cos(g);
sg = sin(g);
l += -0.133533 * cg + 0.041116 * sg;
l1 +=  6.95 * cg + 187.07 * sg;
g = STR*(a - 2.*D + MP);
cg = cos(g);
sg = sin(g);
l += -0.133430 * cg + 0.041079 * sg;
l1 +=  6.28 * cg + 169.08 * sg;
g = STR*(3.*Ve - 4.*Ea);
cg = cos(g);
sg = sin(g);
l += -0.175074 * cg + 0.003035 * sg;
l1 +=  49.17 * cg + 150.57 * sg;
g = STR*(2.*(Ea + D - MP) - 3.*Ju + 213534.);
l1 +=  158.4 * sin(g);
l1 += moonpol[0];
a = 0.1 * T; /* set amplitude scale of 1.0 = 10^-4 arcsec */
moonpol[1] *= a;
moonpol[2] *= a;
}
#endif	/* MOSH_MOON_200 */

static void moon2()
{
/* terms in T^0 */
g = STR*(2*(Ea-Ju+D)-MP+648431.172);
l += 1.14307 * sin(g);
g = STR*(Ve-Ea+648035.568);
l += 0.82155 * sin(g);
g = STR*(3*(Ve-Ea)+2*D-MP+647933.184);
l += 0.64371 * sin(g);
g = STR*(Ea-Ju+4424.04);
l += 0.63880 * sin(g);
g = STR*(SWELP + MP - NF + 4.68);
l += 0.49331 * sin(g);
g = STR*(SWELP - MP - NF + 4.68);
l += 0.4914 * sin(g);
g = STR*(SWELP+NF+2.52);
l += 0.36061 * sin(g);
g = STR*(2.*Ve - 2.*Ea + 736.2);
l += 0.30154 * sin(g);
g = STR*(2.*Ea - 3.*Ju + 2.*D - 2.*MP + 36138.2);
l += 0.28282 * sin(g);
g = STR*(2.*Ea - 2.*Ju + 2.*D - 2.*MP + 311.0);
l += 0.24516 * sin(g);
g = STR*(Ea - Ju - 2.*D + MP + 6275.88);
l += 0.21117 * sin(g);
g = STR*(2.*(Ea - Ma) - 846.36);
l += 0.19444 * sin(g);
g = STR*(2.*(Ea - Ju) + 1569.96);
l -= 0.18457 * sin(g);
g = STR*(2.*(Ea - Ju) - MP - 55.8);
l += 0.18256 * sin(g);
g = STR*(Ea - Ju - 2.*D + 6490.08);
l += 0.16499 * sin(g);
g = STR*(Ea - 2.*Ju - 212378.4);
l += 0.16427 * sin(g);
g = STR*(2.*(Ve - Ea - D) + MP + 1122.48);
l += 0.16088 * sin(g);
g = STR*(Ve - Ea - MP + 32.04);
l -= 0.15350 * sin(g);
g = STR*(Ea - Ju - MP + 4488.88);
l += 0.14346 * sin(g);
g = STR*(2.*(Ve - Ea + D) - MP - 8.64);
l += 0.13594 * sin(g);
g = STR*(2.*(Ve - Ea - D) + 1319.76);
l += 0.13432 * sin(g);
g = STR*(Ve - Ea - 2.*D + MP - 56.16);
l -= 0.13122 * sin(g);
g = STR*(Ve - Ea + MP + 54.36);
l -= 0.12722 * sin(g);
g = STR*(3.*(Ve - Ea) - MP + 433.8);
l += 0.12539 * sin(g);
g = STR*(Ea - Ju + MP + 4002.12);
l += 0.10994 * sin(g);
g = STR*(20.*Ve - 21.*Ea - 2.*D + MP - 317511.72);
l += 0.10652 * sin(g);
g = STR*(26.*Ve - 29.*Ea - MP + 270002.52);
l += 0.10490 * sin(g);
g = STR*(3.*Ve - 4.*Ea + D - MP - 322765.56);
l += 0.10386 * sin(g);
g = STR*(SWELP+648002.556);
B =  8.04508 * sin(g);
g = STR*(Ea+D+996048.252);
B += 1.51021 * sin(g);
g = STR*(f - MP + NF + 95554.332);
B += 0.63037 * sin(g);
g = STR*(f - MP - NF + 95553.792);
B += 0.63014 * sin(g);
g = STR*(SWELP - MP + 2.9);
B +=  0.45587 * sin(g);
g = STR*(SWELP + MP + 2.5);
B +=  -0.41573 * sin(g);
g = STR*(SWELP - 2.0*NF + 3.2);
B +=  0.32623 * sin(g);
g = STR*(SWELP - 2.0*D + 2.5);
B +=  0.29855 * sin(g);
}

static void moon3()
{
/* terms in T^0 */
moonpol[0] = 0.0;
chewm( LR, NLR, 4, 1, moonpol );
chewm( MB, NMB, 4, 3, moonpol );
l += (((l4 * T + l3) * T + l2) * T + l1) * T * 1.0e-5;
moonpol[0] = SWELP + l + 1.0e-4 * moonpol[0];
moonpol[1] = 1.0e-4 * moonpol[1] + B;
moonpol[2] = 1.0e-4 * moonpol[2] + 385000.52899; /* kilometers */
}

/* Compute final ecliptic polar coordinates
 */
static void moon4()
{
moonpol[2] /= AUNIT / 1000;
moonpol[0] = STR * mods3600( moonpol[0] );
moonpol[1] = STR * moonpol[1];
B = moonpol[1];
}

#define CORR_MNODE_JD_T0GREG  -3063616.5   /* 1 jan -13100 greg. */
#define CORR_MNODE_JD_T1GREG    844477.5   /* 1 jan  -2400 jul. */
#define CORR_MNODE_JD_T2GREG   2780263.5   /* 1 jan   2900 jul. */
#define CORR_MNODE_JD_T3GREG   7930182.5   /* 1 jan  17000 greg. */
static double corr_mean_node(double J)
{
  double J0, dJ, dayscty, dcor, dcor0, dcor1, dfrac;
  int i;
  J0 = CORR_MNODE_JD_T0GREG; /* 1-jan--13000 greg */
  dayscty = 36524.25; /* days per Gregorian century */
  if (J < JPL_DE431_START) return 0;
  if (J > JPL_DE431_END) return 0;
/*if (J > CORR_MNODE_JD_T1GREG && J < CORR_MNODE_JD_T2GREG) return 0;*/
  dJ = J - J0;
  i = (int) floor(dJ / dayscty); /* centuries = index of lower correction value */
  dfrac = (dJ - i * dayscty) / dayscty;
  dcor0 = mean_node_corr[i];
  dcor1 = mean_node_corr[i + 1];
  dcor = dcor0 + dfrac * (dcor1 - dcor0);
  return dcor;
}

/* mean lunar node
 * J		julian day
 * pol		return array for position and velocity
 *              (polar coordinates of ecliptic of date)
 */
int swi_mean_node(double J, double *pol, char *serr)
{
#if 0
  double a, b, c;
#endif
  char s[AS_MAXCH];
  double dcor;
  T = (J-J2000)/36525.0;
  T2 = T*T;
  T3 = T*T2;
  T4 = T2*T2;
  /* with elements from swi_moshmoon2(), which are fitted to jpl-ephemeris */
  if (J < MOSHNDEPH_START || J > MOSHNDEPH_END) {
    if (serr != NULL) {
      sprintf(s, "jd %f outside mean node range %.2f .. %.2f ",
		    J, MOSHNDEPH_START, MOSHNDEPH_END);
      if (strlen(serr) + strlen(s) < AS_MAXCH)
	strcat(serr, s);
    }
    return ERR;
  }
  mean_elements();
  dcor = corr_mean_node(J) * 3600;
  /* longitude */
  pol[0] = swi_mod2PI((SWELP - NF - dcor) * STR);
  /* latitude */
  pol[1] = 0.0;
  /* distance */
  pol[2] = MOON_MEAN_DIST / AUNIT; /* or should it be derived from mean
				    * orbital ellipse? */
#if 0
  a = pol[0];
  /* Chapront, according to Meeus, German, p. 339 */
  pol[0] = 125.0445550 - 1934.1361849 * T + 0.0020762 * T2 +
	   T3 / 467410 - T4 / 60616000;
  pol[0] = swi_mod2PI(pol[0] * DEGTORAD);
  c = pol[0];
  printf ("mean node\n");
  printf ("moshier de404 - chapront %f\"\n", (a-c) * RADTODEG * 3600);
#endif
  return OK;
}

#define CORR_MAPOG_JD_T0GREG  -3063616.5   /* 1 jan -13100 greg. */
#define CORR_MAPOG_JD_T1GREG   1209720.5   /* 1 jan  -1400 greg. */
#define CORR_MAPOG_JD_T2GREG   2780263.5   /* 1 jan   2900 greg. */
#define CORR_MAPOG_JD_T3GREG   7930182.5   /* 1 jan  17000 greg. */
static double corr_mean_apog(double J)
{
  double J0, dJ, dayscty, dcor, dcor0, dcor1, dfrac;
  int i;
  J0 = CORR_MAPOG_JD_T0GREG; /* 1-jan--13000 greg */
  dayscty = 36524.25; /* days per Gregorian century */
  if (J < JPL_DE431_START) return 0;
  if (J > JPL_DE431_END) return 0;
  /*if (J > CORR_MAPOG_JD_T1GREG && J < CORR_MAPOG_JD_T2GREG) return 0;*/
  dJ = J - J0;
  i = (int) floor(dJ / dayscty); /* centuries = index of lower correction value */
  dfrac = (dJ - i * dayscty) / dayscty;
  dcor0 = mean_apsis_corr[i];
  dcor1 = mean_apsis_corr[i + 1];
  dcor = dcor0 + dfrac * (dcor1 - dcor0);
  return dcor;
}

/* mean lunar apogee ('dark moon', 'lilith')
 * J		julian day
 * pol		return array for position
 *              (polar coordinates of ecliptic of date)
 * serr         error return string
 */
int swi_mean_apog(double J, double *pol, char *serr)
{
#if 0
  int i;
  double a, b;
  double x[3];
#endif
  double node, dcor;
  char s[AS_MAXCH];
  T = (J-J2000)/36525.0;
  T2 = T*T;
  T3 = T*T2;
  T4 = T2*T2;
  /* with elements from swi_moshmoon2(), which are fitted to jpl-ephemeris */
  if (J < MOSHNDEPH_START || J > MOSHNDEPH_END) {
    if (serr != NULL) {
      sprintf(s, "jd %f outside mean apogee range %.2f .. %.2f ",
		    J, MOSHNDEPH_START, MOSHNDEPH_END);
      if (strlen(serr) + strlen(s) < AS_MAXCH)
	strcat(serr, s);
    }
    return(ERR);
  }
  mean_elements();
  pol[0] = swi_mod2PI((SWELP - MP) * STR + PI);
  pol[1] = 0;
  pol[2] = MOON_MEAN_DIST * (1 + MOON_MEAN_ECC) / AUNIT; /* apogee */
  /* Lilith or Dark Moon is either the empty focal point of the mean 
   * lunar ellipse or, for some people, its apogee ("aphelion"). 
   * This is 180 degrees from the perigee.
   *
   * Since the lunar orbit is not in the ecliptic, the apogee must be
   * projected onto the ecliptic.
   * Joelle de Gravelaine has in her book "Lilith der schwarze Mond"
   * (Astrodata, 1990) an ephemeris which gives noon (12.00) positions
   * but does not project them onto the ecliptic.
   * This results in a mistake of several arc minutes.
   *
   * There is also another problem. The other focal point doesn't
   * coincide with the geocenter but with the barycenter of the
   * earth-moon-system. The difference is about 4700 km. If one
   * took this into account, it would result in an oscillation
   * of the Black Moon. If defined as the apogee, this oscillation 
   * would be about +/- 40 arcmin.
   * If defined as the second focus, the effect is very large:
   * +/- 6 deg!
   * We neglect this influence.
   */
  dcor = corr_mean_apog(J) * DEGTORAD;
  pol[0] = swi_mod2PI(pol[0] - dcor);
  /* apogee is now projected onto ecliptic */
  node = (SWELP - NF) * STR;
  dcor = corr_mean_node(J) * DEGTORAD;
  node = swi_mod2PI(node - dcor);
  pol[0] = swi_mod2PI(pol[0] - node);
  swi_polcart(pol, pol);
  swi_coortrf(pol, pol, -MOON_MEAN_INCL * DEGTORAD);
  swi_cartpol(pol, pol);
  pol[0] = swi_mod2PI(pol[0] + node);
  return OK;
}

/* Program to step through the perturbation table
 */
static void chewm(short *pt, int nlines, int nangles, int typflg, double *ans )
{
  int i, j, k, k1, m;
  double cu, su, cv, sv, ff;
  for( i=0; i<nlines; i++ ) {
    k1 = 0;
    sv = 0.0;
    cv = 0.0;
    for( m=0; m<nangles; m++ ) {
      j = *pt++; /* multiple angle factor */
      if( j ) {
	k = j;
	if( j < 0 ) k = -k; /* make angle factor > 0 */
	/* sin, cos (k*angle) from lookup table */
	su = ss[m][k-1];
	cu = cc[m][k-1];
	if( j < 0 ) su = -su; /* negative angle factor */
	if( k1 == 0 ) {
	  /* Set sin, cos of first angle. */
	  sv = su;
	  cv = cu;
	  k1 = 1;
	}
	else {
	  /* Combine angles by trigonometry. */
	  ff =  su*cv + cu*sv;
	  cv = cu*cv - su*sv;
	  sv = ff;
	}
      }
    }
    /* Accumulate
     */
    switch( typflg ) {
    /* large longitude and radius */
    case 1:
      j = *pt++;
      k = *pt++;
      ans[0] += (10000.0 * j  + k) * sv;
      j = *pt++;
      k = *pt++;
      if( k ) ans[2] += (10000.0 * j  + k) * cv;
      break;
    /* longitude and radius */
    case 2:
      j = *pt++;
      k = *pt++;
      ans[0] += j * sv;
      ans[2] += k * cv;
      break;
    /* large latitude */
    case 3:
      j = *pt++;
      k = *pt++;
      ans[1] += ( 10000.0*j + k)*sv;
      break;
    /* latitude */
    case 4:
      j = *pt++;
      ans[1] += j * sv;
      break;
    }
  }
}

/* Prepare lookup table of sin and cos ( i*Lj )
 * for required multiple angles
 */
static void sscc(int k, double arg, int n )
{
  double cu, su, cv, sv, s;
  int i;
  su = sin(arg);
  cu = cos(arg);
  ss[k][0] = su;			/* sin(L) */
  cc[k][0] = cu;			/* cos(L) */
  sv = 2.0*su*cu;
  cv = cu*cu - su*su;
  ss[k][1] = sv;			/* sin(2L) */
  cc[k][1] = cv;
  for( i=2; i<n; i++ ) {
    s =  su*cv + cu*sv;
    cv = cu*cv - su*sv;
    sv = s;
    ss[k][i] = sv;		/* sin( i+1 L ) */
    cc[k][i] = cv;
  }
}

/* converts from polar coordinates of ecliptic of date
 *          to   cartesian coordinates of equator 2000
 * tjd		date
 * x 		array of position
 */
static void ecldat_equ2000(double tjd, double *xpm) {
  /* cartesian */
  swi_polcart(xpm, xpm);
  /* equatorial */
  swi_coortrf2(xpm, xpm, -swed.oec.seps, swed.oec.ceps);
  /* j2000 */
  swi_precess(xpm, tjd, 0, J_TO_J2000);/**/
}

/* Reduce arc seconds modulo 360 degrees
 * answer in arc seconds
 */
static double mods3600(double x)
{
double lx;
lx = x;
lx = lx - 1296000.0 * floor( lx/1296000.0 );
return( lx );
}

void swi_mean_lunar_elements(double tjd, 
							 double *node, double *dnode, 
							 double *peri, double *dperi)
{
  double dcor;
  T = (tjd - J2000) / 36525.0;
  T2 = T*T;
  mean_elements();
  *node = swe_degnorm((SWELP - NF) * STR * RADTODEG);
  *peri = swe_degnorm((SWELP - MP) * STR * RADTODEG);
  T -= 1.0 / 36525;
  mean_elements();
  *dnode = swe_degnorm(*node - (SWELP - NF) * STR * RADTODEG);
  *dnode -= 360;
  *dperi = swe_degnorm(*peri - (SWELP - MP) * STR * RADTODEG);
  dcor = corr_mean_node(tjd);
  *node = swe_degnorm(*node - dcor);
  dcor = corr_mean_apog(tjd);
  *peri = swe_degnorm(*peri - dcor);
}

static void mean_elements()
{
double fracT = fmod(T, 1);
/* Mean anomaly of sun = l' (J. Laskar) */
/*M =  mods3600(129596581.038354 * T +  1287104.76154);*/
M =  mods3600(129600000.0 * fracT - 3418.961646 * T +  1287104.76154);
M += ((((((((
  1.62e-20 * T
- 1.0390e-17 ) * T
- 3.83508e-15 ) * T
+ 4.237343e-13 ) * T
+ 8.8555011e-11 ) * T
- 4.77258489e-8 ) * T
- 1.1297037031e-5 ) * T
+ 1.4732069041e-4 ) * T
- 0.552891801772 ) * T2;
#ifdef MOSH_MOON_200
/* Mean distance of moon from its ascending node = F */
NF = mods3600( 1739527263.0983 * T + 335779.55755 );
/* Mean anomaly of moon = l */
MP = mods3600( 1717915923.4728 * T +  485868.28096 );
/* Mean elongation of moon = D */
D = mods3600( 1602961601.4603 * T + 1072260.73512 );
/* Mean longitude of moon */
SWELP = mods3600( 1732564372.83264 * T +  785939.95571 );                      
/* Higher degree secular terms found by least squares fit */
NF += (((((z[5] *T+z[4] )*T + z[3] )*T + z[2] )*T + z[1] )*T + z[0] )*T2;
MP += (((((z[11]*T+z[10])*T + z[9] )*T + z[8] )*T + z[7] )*T + z[6] )*T2;
D  += (((((z[17]*T+z[16])*T + z[15])*T + z[14])*T + z[13])*T + z[12])*T2;
SWELP += (((((z[23]*T+z[22])*T + z[21])*T + z[20])*T + z[19])*T + z[18])*T2;    
#else
/* Mean distance of moon from its ascending node = F */
/*NF = mods3600((1739527263.0983 - 2.079419901760e-01) * T + 335779.55755);*/
NF = mods3600(1739232000.0 * fracT + 295263.0983 * T - 2.079419901760e-01 * T + 335779.55755);
/* Mean anomaly of moon = l */
/*MP = mods3600((1717915923.4728 - 2.035946368532e-01) * T +  485868.28096);*/
MP = mods3600(1717200000.0 * fracT + 715923.4728 * T - 2.035946368532e-01 * T +  485868.28096);
/* Mean elongation of moon = D */
/*D = mods3600((1602961601.4603 + 3.962893294503e-01) * T + 1072260.73512);*/
D = mods3600(1601856000.0 * fracT + 1105601.4603 * T + 3.962893294503e-01 * T + 1072260.73512);
/* Mean longitude of moon, referred to the mean ecliptic and equinox of date */
/*SWELP = mods3600((1732564372.83264 - 6.784914260953e-01) * T +  785939.95571);*/
SWELP = mods3600(1731456000.0 * fracT + 1108372.83264 * T - 6.784914260953e-01 * T +  785939.95571);
/* Higher degree secular terms found by least squares fit */
NF += ((z[2]*T + z[1])*T + z[0])*T2;
MP += ((z[5]*T + z[4])*T + z[3])*T2;
D  += ((z[8]*T + z[7])*T + z[6])*T2;
SWELP += ((z[11]*T + z[10])*T + z[9])*T2;
#endif	/* ! MOSH_MOON_200 */
/* sensitivity of mean elements
 *    delta argument = scale factor times delta amplitude (arcsec)
 * cos l  9.0019 = mean eccentricity
 * cos 2D 43.6
 * cos F  11.2 (latitude term)
 */
}

void mean_elements_pl() 
{
/* Mean longitudes of planets (Laskar, Bretagnon) */
Ve = mods3600( 210664136.4335482 * T + 655127.283046 );
Ve += ((((((((
  -9.36e-023 * T
 - 1.95e-20 ) * T
 + 6.097e-18 ) * T
 + 4.43201e-15 ) * T
 + 2.509418e-13 ) * T
 - 3.0622898e-10 ) * T
 - 2.26602516e-9 ) * T
 - 1.4244812531e-5 ) * T
 + 0.005871373088 ) * T2;
Ea = mods3600( 129597742.26669231  * T +  361679.214649 );
Ea += (((((((( -1.16e-22 * T
 + 2.976e-19 ) * T
 + 2.8460e-17 ) * T
 - 1.08402e-14 ) * T
 - 1.226182e-12 ) * T
 + 1.7228268e-10 ) * T
 + 1.515912254e-7 ) * T
 + 8.863982531e-6 ) * T
 - 2.0199859001e-2 ) * T2;
Ma = mods3600(  68905077.59284 * T + 1279559.78866 );
Ma += (-1.043e-5*T + 9.38012e-3)*T2;
Ju = mods3600( 10925660.428608 * T +  123665.342120 );
Ju += (1.543273e-5*T - 3.06037836351e-1)*T2;
Sa = mods3600( 4399609.65932 * T + 180278.89694 );
Sa += (( 4.475946e-8*T - 6.874806E-5 ) * T + 7.56161437443E-1)*T2;
}

/* Calculate geometric coordinates of true interpolated Moon apsides
 */
int swi_intp_apsides(double J, double *pol, int ipli)
{
double dd;
double rsv[3];
double sNF, sD, sLP, sMP, sM, sVe, sEa, sMa, sJu, sSa, fM, fVe, fEa, fMa, fJu, fSa, cMP, zMP, fNF, fD, fLP;
double dMP, mLP, mNF, mD, mMP;
int i, ii, iii, niter = 4;	/* niter: silence compiler warning */
ii=1;
zMP=27.55454988;
fNF = 27.212220817/zMP;/**/
fD  = 29.530588835/zMP;/**/
fLP = 27.321582/zMP;/**/
fM  = 365.2596359/zMP;
fVe = 224.7008001/zMP;
fEa = 365.2563629/zMP;
fMa = 686.9798519/zMP;
fJu = 4332.589348/zMP;
fSa = 10759.22722/zMP;
T = (J-J2000)/36525.0;
T2 = T*T;
T4 = T2*T2;
mean_elements();
mean_elements_pl();
sNF = NF;
sD  = D;
sLP = SWELP;
sMP = MP;
sM  = M ;
sVe = Ve;
sEa = Ea;
sMa = Ma;
sJu = Ju;
sSa = Sa;
sNF = mods3600(NF);
sD  = mods3600(D);
sLP = mods3600(SWELP);
sMP = mods3600(MP);
if (ipli == SEI_INTP_PERG) {MP = 0.0; niter = 5;}
if (ipli == SEI_INTP_APOG) {MP = 648000.0; niter = 4;}
cMP = 0;
dd = 18000.0;
for (iii= 0; iii<=niter; iii++) {/**/
 dMP = sMP - MP;
 mLP = sLP - dMP;
 mNF = sNF - dMP;
 mD  = sD  - dMP;
 mMP = sMP - dMP;
 for (ii = 0; ii <=2; ii++) {/**/
   MP = mMP + (ii-1)*dd;       /**/
   NF = mNF + (ii-1)*dd/fNF;
   D  = mD  + (ii-1)*dd/fD;
   SWELP = mLP + (ii-1)*dd/fLP;
   M  = sM  + (ii-1)*dd/fM ;
   Ve = sVe + (ii-1)*dd/fVe;
   Ea = sEa + (ii-1)*dd/fEa;
   Ma = sMa + (ii-1)*dd/fMa;
   Ju = sJu + (ii-1)*dd/fJu;
   Sa = sSa + (ii-1)*dd/fSa;
   moon1();
   moon2();
   moon3();
   moon4();
   if (ii==1) {
     for( i=0; i<3; i++ ) pol[i] = moonpol[i];
   }
   rsv[ii] = moonpol[2];
 }
 cMP = (1.5*rsv[0] - 2*rsv[1] + 0.5*rsv[2]) / (rsv[0] + rsv[2] - 2*rsv[1]);/**/
 cMP *= dd;
 cMP = cMP - dd;
 mMP += cMP;
 MP = mMP;
 dd /= 10;
}
return(0);
}

