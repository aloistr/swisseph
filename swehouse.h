/*******************************************************
$Header: swehouse.h,v 1.26 98/11/29 21:08:03 dieter Exp $
module swehouse.h
house and (simple) aspect calculation 

*******************************************************/

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
struct  houses {
	  double cusp[13];
	  double ac;
	  double mc;
	  double vertex;
	  double equasc;
	  double coasc1;
	  double coasc2;
	  double polasc;
	};

#define HOUSES 	struct houses
#define VERY_SMALL	1E-10

#define degtocs(x)    (d2l((x) * DEG))
#define cstodeg(x)    (double)((x) * CS2DEG)

#define sind(x) sin(x * DEGTORAD)
#define cosd(x) cos(x * DEGTORAD)
#define tand(x) tan(x * DEGTORAD)
#define asind(x) (asin(x) * RADTODEG)
#define acosd(x) (acos(x) * RADTODEG)
#define atand(x) (atan(x) * RADTODEG)
#define atan2d(y, x) (atan2(y, x) * RADTODEG)
