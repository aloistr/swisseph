/* 
 | $Header: swejpl.h,v 1.65 2003/06/14 13:02:32 alois Exp $
 |
 | Subroutines for reading JPL ephemerides.
 | derived from testeph.f as contained in DE403 distribution July 1995.
 | works with DE200, DE102, DE403, DE404, DE405, DE406
 | (attention, DE102 has 950 reference frame and also DE403 has slightly
 | different reference frame from DE200).

  Authors: Dieter Koch and Alois Treindl, Astrodienst Zürich

*/
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


#include "sweodef.h"

#define J_MERCURY	0	/* jpl body indices, modified by Alois */
#define J_VENUS		1	/* now they start at 0 and not at 1 */
#define J_EARTH		2
#define J_MARS		3
#define J_JUPITER	4
#define J_SATURN	5
#define J_URANUS	6
#define J_NEPTUNE	7
#define J_PLUTO		8
#define J_MOON		9
#define J_SUN		10
#define J_SBARY		11
#define J_EMB		12
#define J_NUT		13
#define J_LIB		14

/*
 * compute position and speed at time et, for body ntarg with center
 * ncent. rrd must be double[6] to contain the return vectors.
 * ntarg can be all of the above, ncent all except J_NUT and J_LIB.
 * Librations and Nutations are not affected by ncent.
 */
extern int swi_pleph(double et, int ntarg, int ncent, double *rrd, char *serr);

/*
 * read the ephemeris constants. ss[0..2] returns start, end and granule size.
 * If do_show is TRUE, a list of constants is printed to stdout.
 */
extern void swi_close_jpl_file(void);

extern int swi_open_jpl_file(double *ss, char *fname, char *fpath, char *serr);

extern int32 swi_get_jpl_denum(void);

extern void swi_IERS_FK5(double *xin, double *xout, int dir);

