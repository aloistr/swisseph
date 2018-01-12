/*****************************************************
$Header: swepdate.c,v 1.65 2003/06/14 13:02:01 alois Exp $
Placalc compatibility interface for Swiss Ephemeris.
date functions

*******************************************************/
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
 * This file is part of the PLACALC compatibility interface for Swiss Ephemeris.
 * It allows very easy porting of older Placalc application to the SwissEph.
 * A user has to replace #include "placalc.h" and "housasp.h" with
 * #include "swepcalc.h"
 * If he has used "ourdef.h" he replaces it with "sweodef.h".
 * Then he links his application with swepcalc.o and runs it against the
 * Swiss Ephemeris DLL or linkable library.
 *
 * All calls which were present in the placalc sources are contained 
 * here, and either implemented directly or translated into Swiss Ephemeris
 * calls.
 *
 * 
 */

#include "swepcalc.h"
#include "swephexp.h"

/*************** julday ********************************************
 * This function returns the absolute Julian day number (JD)
 * for a given calendar date.
 * The arguments are a calendar date: day, month, year as integers,
 * hour as double with decimal fraction.
 * If gregflag = 1, Gregorian calendar is assumed, gregflag = 0
 * Julian calendar is assumed.
 *
 The Julian day number is system of numbering all days continously
 within the time range of known human history. It should be familiar
 for every astrological or astronomical programmer. The time variable
 in astronomical theories is usually expressed in Julian days or
 Julian centuries (36525 days per century) relative to some start day;
 the start day is called 'the epoch'.
 The Julian day number is a double representing the number of
 days since JD = 0.0 on 1 Jan -4712, 12:00 noon.
 
 Midnight has always a JD with fraction .5, because traditionally
 the astronomical day started at noon. This was practical because
 then there was no change of date during a night at the telescope.
 From this comes also the fact the noon ephemerides were printed
 before midnight ephemerides were introduced early in the 20th century.
 
 NOTE: The Julian day number is named after the monk Julianus. It must
 not be confused with the Julian calendar system, which is named after
 Julius Cesar, the Roman politician who introduced this calendar.
 The Julian century is named after Cesar, i.e. a century in the Julian
 calendar. The 'gregorian' century has a variable length.

 Be aware the we always use astronomical year numbering for the years
 before Christ, not the historical year numbering.
 Astronomical years are done with negative numbers, historical
 years with indicators BC or BCE (before common era).
 Year 0 (astronomical)  	= 1 BC
 year -1 (astronomical) 	= 2 BC
 etc.

 Original author: Marc Pottenger, Los Angeles.
 with bug fix for year < -4711   15-aug-88 by Alois Treindl
 (The parameter sequence m,d,y still indicates the US origin,
  be careful because the similar function date_conversion() uses
  other parameter sequence and also Astrodienst relative juldate.)
 
 References: Oliver Montenbruck, Grundlagen der Ephemeridenrechnung,
             Verlag Sterne und Weltraum (1987), p.49 ff
 
 related functions: revjul() reverse Julian day number: compute the
  			       calendar date from a given JD
	            date_conversion() includes test for legal date values
		    and notifies errors like 32 January.
 ****************************************************************/

double julday(int month, int day, int year, double hour, int gregflag) 
{
  double jd;
  jd = swe_julday(year, month, day, hour, gregflag);
  return jd;
}

/*
 * monday = 0, ... sunday = 6
 */
int day_of_week(double jd)
{
  return (((int) floor (jd - 2433282 - 1.5) %7) + 7) % 7;
}


/*** revjul ******************************************************
  revjul() is the inverse function to julday(), see the description
  there.
  Arguments are julian day number, calendar flag (0=julian, 1=gregorian)
  return values are the calendar day, month, year and the hour of
  the day with decimal fraction (0 .. 23.999999).

  Be aware the we use astronomical year numbering for the years
  before Christ, not the historical year numbering.
  Astronomical years are done with negative numbers, historical
  years with indicators BC or BCE (before common era).
  Year  0 (astronomical)  	= 1 BC historical year
  year -1 (astronomical) 	= 2 BC historical year
  year -234 (astronomical) 	= 235 BC historical year
  etc.

  Original author Mark Pottenger, Los Angeles.
  with bug fix for year < -4711 16-aug-88 Alois Treindl
*************************************************************************/
void revjul (double jd, int gregflag,
	     int *jmon, int *jday, int *jyear, double *jut)
{
  swe_revjul(jd, gregflag, jyear, jmon, jday, jut);
}

/*********************************************************
  $Header: swepdate.c,v 1.65 2003/06/14 13:02:01 alois Exp $
  version 15-feb-89 16:30

  This function converts some date+time input {d,m,y,utime}
  into the Julian day number tgmt, which is an Astrodienst relative
  Julian date.
  The function checks that the input is a legal combination
  of dates; for illegal dates like 32 January 1993 it returns ERR
  but still converts the date correctly, i.e. like 1 Feb 1993.
  The function is usually used to convert user input of birth data
  into the Julian day number. Illegal dates should be notified to the user.

  Be aware the we always use astronomical year numbering for the years
  before Christ, not the historical year numbering.
  Astronomical years are done with negative numbers, historical
  years with indicators BC or BCE (before common era).
  Year 0 (astronomical)  	= 1 BC historical.
  year -1 (astronomical) 	= 2 BC
  etc.
  Many users of Astro programs do not know about this difference.

  Return: OK or ERR (for illegal date)
*********************************************************/

int date_conversion (int d ,
                     int m ,
                     int y ,		/* day, month, year */
                     centisec gutime, 	/* greenwich time in centiseconds */
                     char c,  /* calendar g[regorian]|j[ulian]|a[stro = greg] */
                     double *tgmt	
			/* julian date relative 0.Jan.1950 12:00 gmt */
			/* shift is 2433282 from absolute Julian date */
		    ) 
{ 
  int rday, rmon, ryear;
  double rut, jd;
  int gregflag = SE_JUL_CAL;
  if (c == 'g' || c == 'a')
    gregflag = SE_GREG_CAL;
  rut = gutime / 360000.0;	/* hours GMT */
  jd = julday(m, d, y, rut, gregflag);
  revjul(jd, gregflag, &rmon, &rday, &ryear, &rut);
  *tgmt = jd - JUL_OFFSET;
  if (rmon == m && rday == d && ryear == y) {
    return OK;
  } else {
    return ERR;
  }
}	/* end date_conversion */
