/*********************************************************
  $Header: swedate.c,v 1.65 2003/06/14 13:09:56 alois Exp $
  version 15-feb-89 16:30
  
  swe_date_conversion()
  swe_revjul()
  swe_julday()
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

/*
  swe_date_conversion():
  This function converts some date+time input {d,m,y,uttime}
  into the Julian day number tjd.
  The function checks that the input is a legal combination
  of dates; for illegal dates like 32 January 1993 it returns ERR
  but still converts the date correctly, i.e. like 1 Feb 1993.
  The function is usually used to convert user input of birth data
  into the Julian day number. Illegal dates should be notified to the user.

  Be aware that we always use astronomical year numbering for the years
  before Christ, not the historical year numbering.
  Astronomical years are done with negative numbers, historical
  years with indicators BC or BCE (before common era).
  Year 0 (astronomical)  	= 1 BC historical.
  year -1 (astronomical) 	= 2 BC
  etc.
  Many users of Astro programs do not know about this difference.

  Return: OK or ERR (for illegal date)
*********************************************************/

# include "swephexp.h"


int FAR PASCAL_CONV swe_date_conversion(int y,
		     int m,
		     int d,		/* day, month, year */
		     double uttime, 	/* UT in hours (decimal) */
		     char c,		/* calendar g[regorian]|j[ulian] */
		     double *tjd)
{
  int rday, rmon, ryear;
  double rut, jd;
  int gregflag = SE_JUL_CAL;
  if (c == 'g')
    gregflag = SE_GREG_CAL;
  rut = uttime;		/* hours UT */
  jd = swe_julday(y, m, d, rut, gregflag);
  swe_revjul(jd, gregflag, &ryear, &rmon, &rday, &rut);
  *tjd = jd;
  if (rmon == m && rday == d && ryear == y) {
    return OK;
  } else {
    return ERR;
  }
}	/* end date_conversion */

/*************** swe_julday ********************************************
 * This function returns the absolute Julian day number (JD)
 * for a given calendar date.
 * The arguments are a calendar date: day, month, year as integers,
 * hour as double with decimal fraction.
 * If gregflag = SE_GREG_CAL (1), Gregorian calendar is assumed,
 * if gregflag = SE_JUL_CAL (0),Julian calendar is assumed.
  
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
 calendar. The 'gregorian' century has a variable lenght.

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
 
 related functions: swe_revjul() reverse Julian day number: compute the
  			       calendar date from a given JD
	            date_conversion() includes test for legal date values
		    and notfies errors like 32 January.
 ****************************************************************/

double FAR PASCAL_CONV swe_julday(int year, int month, int day, double hour, int gregflag) 
{
  double jd;
  double u,u0,u1,u2;
  u = year;
  if (month < 3) u -=1;
  u0 = u + 4712.0;
  u1 = month + 1.0;
  if (u1 < 4) u1 += 12.0;
  jd = floor(u0*365.25)
     + floor(30.6*u1+0.000001)
     + day + hour/24.0 - 63.5;
  if (gregflag == SE_GREG_CAL) {
    u2 = floor(fabs(u) / 100) - floor(fabs(u) / 400);
    if (u < 0.0) u2 = -u2;
    jd = jd - u2 + 2;            
    if ((u < 0.0) && (u/100 == floor(u/100)) && (u/400 != floor(u/400)))
      jd -=1;
  }
  return jd;
}

/*** swe_revjul ******************************************************
  swe_revjul() is the inverse function to swe_julday(), see the description
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
void FAR PASCAL_CONV swe_revjul (double jd, int gregflag,
	     int *jyear, int *jmon, int *jday, double *jut)
{
  double u0,u1,u2,u3,u4;
  u0 = jd + 32082.5;
  if (gregflag == SE_GREG_CAL) {
    u1 = u0 + floor (u0/36525.0) - floor (u0/146100.0) - 38.0;
    if (jd >= 1830691.5) u1 +=1;
    u0 = u0 + floor (u1/36525.0) - floor (u1/146100.0) - 38.0;
  }
  u2 = floor (u0 + 123.0);
  u3 = floor ( (u2 - 122.2) / 365.25);
  u4 = floor ( (u2 - floor (365.25 * u3) ) / 30.6001);
  *jmon = (int) (u4 - 1.0);
  if (*jmon > 12) *jmon -= 12;
  *jday = (int) (u2 - floor (365.25 * u3) - floor (30.6001 * u4));
  *jyear = (int) (u3 + floor ( (u4 - 2.0) / 12.0) - 4800);
  *jut = (jd - floor (jd + 0.5) + 0.5) * 24.0;
}
