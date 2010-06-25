
/* 
  $Header: /home/dieter/sweph/RCS/swemini.c,v 1.74 2008/06/16 10:07:20 dieter Exp $

  swemini.c	A minimal program to test the Swiss Ephemeris.

  Input: a date (in gregorian calendar, sequence day.month.year)
  Output: Planet positions at midnight Universal time, ecliptic coordinates,
          geocentric apparent positions relative to true equinox of date, as 
          usual in western astrology.
		
   
  Authors: Dieter Koch and Alois Treindl, Astrodienst Zurich

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


#include "swephexp.h" 	/* this includes  "sweodef.h" */

int main()
{
  char *sp, sdate[AS_MAXCH], snam[40], serr[AS_MAXCH];  
  int jday = 1, jmon = 1, jyear = 2000;
  double jut = 0.0;
  double tjd, te, x2[6];
  int32 iflag, iflgret;
  int p;
  iflag = SEFLG_SPEED;
  while (TRUE) {
    printf("\nDate (d.m.y) ?");
    gets(sdate);
		/*
		 * stop if a period . is entered
		 */
    if (*sdate == '.') 
      return OK;
    if (sscanf (sdate, "%d%*c%d%*c%d", &jday,&jmon,&jyear) < 1) exit(1);
	    /*
	     * we have day, month and year and convert to Julian day number
	     */
    tjd = swe_julday(jyear,jmon,jday,jut,SE_GREG_CAL);        
	    /*
	     * compute Ephemeris time from Universal time by adding delta_t
	     */
    te = tjd + swe_deltat(tjd);
    printf("date: %02d.%02d.%d at 0:00 Universal time\n", jday, jmon, jyear);
    printf("planet     \tlongitude\tlatitude\tdistance\tspeed long.\n");
	    /*
	     * a loop over all planets
	     */
    for (p = SE_SUN; p <= SE_CHIRON; p++) {
      if (p == SE_EARTH) continue;
		/*
		 * do the coordinate calculation for this planet p
		 */
      iflgret = swe_calc(te, p, iflag, x2, serr);
	      /*
	       * if there is a problem, a negative value is returned and an 
	       * errpr message is in serr.
	       */
      if (iflgret < 0) 
	printf("error: %s\n", serr);
      else if (iflgret != iflag)
	printf("warning: iflgret != iflag. %s\n", serr);
	      /*
	       * get the name of the planet p
	       */
      swe_get_planet_name(p, snam);
	      /*
	       * print the coordinates
	       */
      printf("%10s\t%11.7f\t%10.7f\t%10.7f\t%10.7f\n",
	     snam, x2[0], x2[1], x2[2], x2[3]);
    }
  }
  return OK;
}
