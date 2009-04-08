#include "sweodef.h"
#include "swephexp.h"
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

void main()
{
  double tjd, t; 
  int ipl; int32 iflag;
  double xx[6], nut, eps; 
  int32 iflgret;
  char serr[AS_MAXCH];

/*SWE_CLOSE*/
  swe_close();

/*SWE_SET_EPHE_PATH*/
/*  strcpy(s, "F:\AstroB\EPH\");
  swe_set_ephe_path(s);
  printf("swe_set_ephe_path: path_in = ");  printf(s);
  	printf("\tpath_set = unknown to swetrace\n"); /* unknown to swetrace */

/*SWE_CALC*/
  tjd = 2453822.923545915; ipl = 0; iflag = 2050;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323512 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453822.923545915; ipl = 1; iflag = 2050;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323464 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453822.923545915; ipl = 0; iflag = 6146;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323816 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453822.923545915; ipl = 1; iflag = 6146;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323768 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.923545915; ipl = 0; iflag = 2050;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323512 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.923545915; ipl = 1; iflag = 2050;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323464 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.923545915; ipl = 0; iflag = 6146;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323816 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.923545915; ipl = 1; iflag = 6146;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323768 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453824.923545915; ipl = 0; iflag = 2050;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323512 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453824.923545915; ipl = 1; iflag = 2050;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323464 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453824.923545915; ipl = 0; iflag = 6146;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323816 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453824.923545915; ipl = 1; iflag = 6146;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323768 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.677434885; ipl = 0; iflag = 2050;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323512 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.677434885; ipl = 1; iflag = 2050;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323464 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.677434885; ipl = 0; iflag = 6146;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323816 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.677434885; ipl = 1; iflag = 6146;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323768 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.927434885; ipl = 0; iflag = 2050;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323512 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.927434885; ipl = 1; iflag = 2050;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323464 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.927434885; ipl = 0; iflag = 6146;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323816 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.927434885; ipl = 1; iflag = 6146;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323768 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453824.177434885; ipl = 0; iflag = 2050;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323512 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453824.177434885; ipl = 1; iflag = 2050;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323464 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453824.177434885; ipl = 0; iflag = 6146;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323816 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453824.177434885; ipl = 1; iflag = 6146;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323768 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.863923690; ipl = 0; iflag = 2050;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323512 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.863923690; ipl = 1; iflag = 2050;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323464 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.863923690; ipl = 0; iflag = 6146;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323816 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.863923690; ipl = 1; iflag = 6146;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323768 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.926423690; ipl = 0; iflag = 2050;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323512 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.926423690; ipl = 1; iflag = 2050;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323464 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.926423690; ipl = 0; iflag = 6146;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323816 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.926423690; ipl = 1; iflag = 6146;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323768 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.988923690; ipl = 0; iflag = 2050;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323512 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.988923690; ipl = 1; iflag = 2050;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323464 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.988923690; ipl = 0; iflag = 6146;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323816 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.988923690; ipl = 1; iflag = 6146;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323768 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.909996328; ipl = 0; iflag = 2050;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323512 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.909996328; ipl = 1; iflag = 2050;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323464 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.909996328; ipl = 0; iflag = 6146;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323816 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.909996328; ipl = 1; iflag = 6146;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323768 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.925621328; ipl = 0; iflag = 2050;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323512 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.925621328; ipl = 1; iflag = 2050;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323464 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.925621328; ipl = 0; iflag = 6146;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323816 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.925621328; ipl = 1; iflag = 6146;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323768 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.941246328; ipl = 0; iflag = 2050;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323512 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.941246328; ipl = 1; iflag = 2050;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323464 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.941246328; ipl = 0; iflag = 6146;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323816 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.941246328; ipl = 1; iflag = 6146;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323768 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.921381349; ipl = 0; iflag = 2050;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323512 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.921381349; ipl = 1; iflag = 2050;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323464 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.921381349; ipl = 0; iflag = 6146;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323816 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.921381349; ipl = 1; iflag = 6146;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323768 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.925287599; ipl = 0; iflag = 2050;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323512 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.925287599; ipl = 1; iflag = 2050;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323464 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.925287599; ipl = 0; iflag = 6146;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323816 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.925287599; ipl = 1; iflag = 6146;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323768 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.929193849; ipl = 0; iflag = 2050;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323512 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.929193849; ipl = 1; iflag = 2050;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323464 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.929193849; ipl = 0; iflag = 6146;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323816 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.929193849; ipl = 1; iflag = 6146;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323768 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.924287282; ipl = 0; iflag = 2050;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323512 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.924287282; ipl = 1; iflag = 2050;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323464 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.924287282; ipl = 0; iflag = 6146;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323816 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.924287282; ipl = 1; iflag = 6146;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323768 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.925263844; ipl = 0; iflag = 2050;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323512 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.925263844; ipl = 1; iflag = 2050;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323464 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.925263844; ipl = 0; iflag = 6146;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323816 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.925263844; ipl = 1; iflag = 6146;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323768 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.926240407; ipl = 0; iflag = 2050;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323512 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.926240407; ipl = 1; iflag = 2050;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323464 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.926240407; ipl = 0; iflag = 6146;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323816 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.926240407; ipl = 1; iflag = 6146;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323768 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.925019521; ipl = 0; iflag = 2050;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323512 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.925019521; ipl = 1; iflag = 2050;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323464 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.925019521; ipl = 0; iflag = 6146;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323816 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.925019521; ipl = 1; iflag = 6146;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323768 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.925263662; ipl = 0; iflag = 2050;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323512 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.925263662; ipl = 1; iflag = 2050;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323464 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.925263662; ipl = 0; iflag = 6146;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323816 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.925263662; ipl = 1; iflag = 6146;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323768 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.925507803; ipl = 0; iflag = 2050;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323512 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.925507803; ipl = 1; iflag = 2050;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323464 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.925507803; ipl = 0; iflag = 6146;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323816 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.925507803; ipl = 1; iflag = 6146;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323768 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_DELTAT*/
  tjd = 2453823.925263658; t = swe_deltat(tjd);
  printf("swe_deltat: %f\t%f\t\n", tjd, t);

/*SWE_DELTAT*/
  tjd = 2453823.924513062; t = swe_deltat(tjd);
  printf("swe_deltat: %f\t%f\t\n", tjd, t);

/*SWE_CALC*/
  tjd = 2453823.925263658; ipl = 1; iflag = 6402;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 322648 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.925263658; ipl = 1; iflag = 10498;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323104 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.925263658; ipl = 0; iflag = 6402;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 322848 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.925263658; ipl = 0; iflag = 10498;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323152 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_DELTAT*/
  tjd = 2453823.924513062; t = swe_deltat(tjd);
  printf("swe_deltat: %f\t%f\t\n", tjd, t);

/*SWE_DELTAT*/
  tjd = 2453823.500000000; t = swe_deltat(tjd);
  printf("swe_deltat: %f\t%f\t\n", tjd, t);

/*SWE_SIDTIME0*/
  tjd = 2453823.924513062;  eps = 23.441178772;  nut = -0.000430379;
  t = swe_sidtime0(tjd, eps, nut);
  printf("swe_sidtime0: %f\tsidt = %f\teps = %f\tnut = %f\t\n", tjd, t, eps, nut);

/*SWE_SIDTIME*/
  tjd = 2453823.924513062;
  t = swe_sidtime(tjd);
  printf("swe_sidtime: %f\t%f\t\n", tjd, t);

/*SWE_DELTAT*/
  tjd = 2453823.924513062; t = swe_deltat(tjd);
  printf("swe_deltat: %f\t%f\t\n", tjd, t);

/*SWE_CALC*/
  tjd = 2453823.925263658; ipl = 0; iflag = 34818;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 322840 */

/*SWE_DELTAT*/
  tjd = 2453823.925263658; t = swe_deltat(tjd);
  printf("swe_deltat: %f\t%f\t\n", tjd, t);

/*SWE_DELTAT*/
  tjd = 2453823.500000000; t = swe_deltat(tjd);
  printf("swe_deltat: %f\t%f\t\n", tjd, t);

/*SWE_SIDTIME0*/
  tjd = 2453823.924513062;  eps = 0.409125750;  nut = -0.000007512;
  t = swe_sidtime0(tjd, eps, nut);
  printf("swe_sidtime0: %f\tsidt = %f\teps = %f\tnut = %f\t\n", tjd, t, eps, nut);
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.925263658; ipl = 1; iflag = 34818;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323128 */

/*SWE_DELTAT*/
  tjd = 2453823.925249975; t = swe_deltat(tjd);
  printf("swe_deltat: %f\t%f\t\n", tjd, t);

/*SWE_DELTAT*/
  tjd = 2453823.500000000; t = swe_deltat(tjd);
  printf("swe_deltat: %f\t%f\t\n", tjd, t);

/*SWE_SIDTIME0*/
  tjd = 2453823.924499379;  eps = 0.409125750;  nut = -0.000007512;
  t = swe_sidtime0(tjd, eps, nut);
  printf("swe_sidtime0: %f\tsidt = %f\teps = %f\tnut = %f\t\n", tjd, t, eps, nut);
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.925263658; ipl = 0; iflag = 38914;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323080 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453823.925263658; ipl = 1; iflag = 38914;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 322936 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_DELTAT*/
  tjd = 2453823.924513062; t = swe_deltat(tjd);
  printf("swe_deltat: %f\t%f\t\n", tjd, t);

/*SWE_DELTAT*/
  tjd = 2453823.500000000; t = swe_deltat(tjd);
  printf("swe_deltat: %f\t%f\t\n", tjd, t);

/*SWE_SIDTIME0*/
  tjd = 2453823.924513062;  eps = 23.441178772;  nut = -0.000430379;
  t = swe_sidtime0(tjd, eps, nut);
  printf("swe_sidtime0: %f\tsidt = %f\teps = %f\tnut = %f\t\n", tjd, t, eps, nut);

/*SWE_SIDTIME*/
  tjd = 2453823.924513062;
  t = swe_sidtime(tjd);
  printf("swe_sidtime: %f\t%f\t\n", tjd, t);

/*SWE_CALC*/
  tjd = 2453999.999083831; ipl = 0; iflag = 2050;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323512 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453999.999083831; ipl = 1; iflag = 2050;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323464 */

/*SWE_CLOSE*/
  swe_close();

/*SWE_SET_EPHE_PATH*/
/*  strcpy(s, "F:\AstroB\EPH\");
  swe_set_ephe_path(s);
  printf("swe_set_ephe_path: path_in = ");  printf(s);
  	printf("\tpath_set = unknown to swetrace\n"); /* unknown to swetrace */

/*SWE_CALC*/
  tjd = 2453999.999083831; ipl = 0; iflag = 2050;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323512 */
  printf("swe_calc: %f\t%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t", 
	tjd, ipl, iflgret, xx[0], xx[1], xx[2], xx[3], xx[4], xx[5]);
  if (*serr != '\0') printf(serr); printf("\n");

/*SWE_CALC*/
  tjd = 2453999.999083831; ipl = 1; iflag = 2050;
  iflgret = swe_calc(tjd, ipl, iflag, xx, serr);	/* xx = 323464 */

/*SWE_CLOSE*/
  swe_close();

/*SWE_CLOSE*/
  swe_close();
}
