/*******************************************************
module swehouse.h
house and (simple) aspect calculation 

*******************************************************/

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

struct houses {
	  double cusp[37];
	  double cusp_speed[37];
	  double ac;
	  double ac_speed;	// speed of ac
	  double mc;
	  double mc_speed;	// speed of mc
	  double armc_speed;	// speed of armc
	  double vertex;
	  double vertex_speed;	// speed of vertex
	  double equasc;
	  double equasc_speed;	// speed
	  double coasc1;
	  double coasc1_speed;	// speed
	  double coasc2;
	  double coasc2_speed;	// speed
	  double polasc;
	  double polasc_speed;	// speed
	  double sundec;	// declination of Sun for Sunshine houses
	  AS_BOOL do_speed;
	  AS_BOOL do_hspeed;
	  AS_BOOL do_interpol;
	  char serr[AS_MAXCH];
	};

#define HOUSES 	struct houses
#define VERY_SMALL	1E-10

#define degtocs(x)    (d2l((x) * DEG))
#define cstodeg(x)    (double)((x) * CS2DEG)

#define sind(x) sin((x) * DEGTORAD)
#define cosd(x) cos((x) * DEGTORAD)
#define tand(x) tan((x) * DEGTORAD)
#define asind(x) (asin(x) * RADTODEG)
#define acosd(x) (acos(x) * RADTODEG)
#define atand(x) (atan(x) * RADTODEG)
#define atan2d(y, x) (atan2(y, x) * RADTODEG)
