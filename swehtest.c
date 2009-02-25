/* SWISSEPH
   $Header: swehtest.c,v 1.30 97/07/08 15:16:43 dieter Exp $

  tests the swehouse.c module
*************************************************/
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


#include <string.h>
#include "swephexp.h"

#ifdef _WINDOWS
#include <windows.h>
#endif


static void do_printf(char *info);
static char *dms(double x, long iflag);

char *zod_nam[] = {
		"ar","ta","ge","cn","le","vi",
	        "li","sc","sa","cp","aq","pi"
		};

FILE *fp = NULL;

#ifdef _WINDOWS
int PASCAL WinMain(HANDLE hInstance, HANDLE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#else
int main()
#endif
{
#ifdef _WINDOWS
  struct _wopeninfo wo;
#endif
  char s[AS_MAXCH];
  char hsy[AS_MAXCH], slat[AS_MAXCH], ssidt[AS_MAXCH];
  double cusp[1+12+3];	/* empty + 12 houses */
  double ascmc[10];	/* asc, mc, armc, vertex ... */
  double armc, lat, sidt;
  double eps = 23.45;
  int i;
#ifdef _WINDOWS
  wo._version = _QWINVER;
  wo._title = "swehdll";
  wo._wbufsize = _WINBUFDEF;
  fp = _fwopen(&wo, NULL, "w+");
#endif
  while (1) {
    do_printf("lat., sid.time, house system (dd.fraction hh.fraction character):");
#ifdef _WINDOWS
    rewind(fp);
    fgets(s, AS_MAXCH, fp);
    rewind(fp);
#else
    gets(s);
#endif
    if (*s == '.' || *s == '\0' || *s == 'q' || *s == 'e')
      goto end_program;
    else if (sscanf(s, "%s %s %s",slat,ssidt,hsy) == EOF) 
      exit(0);/**/
    lat = atof(slat);
    sidt = atof(ssidt);
    armc = atof(ssidt) * 15;
    sprintf (s, "latitude= %f, sid. time= %f, hsy=%c (eps = 23%c27')\n",lat, sidt, hsy[0], (unsigned int) ODEGREE_CHAR);
    do_printf(s);
    swe_houses_armc(armc, lat, eps, (int) hsy[0], cusp, ascmc);
    /* to compute houses directly from a date and geogr. position, 
     * you can call
     * swe_houses(tjd_ut, lat, lon, (int) hsy[0], cusp, ascmc);
     */
    for (i = 1; i <= 12; i++)  {
      sprintf(s, "house %2d  %s\n", i, dms(cusp[i], 0));
      do_printf(s);
    }
    sprintf(s, "AC        %s\n", dms(ascmc[0], 0));
    do_printf(s);
    sprintf(s, "MC        %s\n", dms(ascmc[1], 0));
    do_printf(s);
    sprintf(s, "Vert.     %s\n", dms(ascmc[3], 0));
    do_printf(s);
  }
  end_program:
  return OK;
}

static void do_printf(char *info)
{
#ifdef _WINDOWS
  fprintf(fp, info);
#else
  printf(info);
#endif
}

#define BIT_ROUND_SEC   1
#define BIT_ROUND_MIN   2
#define BIT_ZODIAC      4
static char *dms(double x, long iflag)
{
  int izod;
  long k, kdeg, kmin, ksec;
  char c = (unsigned int) ODEGREE_CHAR;
  char *sp, s1[50];
  static char s[50];
  int sgn;
  *s = '\0';
  if (iflag & SEFLG_EQUATORIAL)
    c = 'h';
  if (x < 0) {
    x = -x;
    sgn = -1;
  } else 
    sgn = 1;
  if (iflag & BIT_ROUND_MIN)
    x += 0.5/60;
  if (iflag & BIT_ROUND_SEC)
    x += 0.5/3600;
  if (iflag & BIT_ZODIAC) {
    izod = (int) (x / 30); 
    x = fmod(x, 30);
    kdeg = (long) x;
    sprintf(s, "%2ld %s ", kdeg, zod_nam[izod]);
  } else {
    kdeg = (long) x;
    sprintf(s, " %3ld%c", kdeg, c);
  }
  x -= kdeg;
  x *= 60;
  kmin = (long) x;
  if ((iflag & BIT_ZODIAC) && (iflag & BIT_ROUND_MIN))
    sprintf(s1, "%2ld", kmin);
  else
    sprintf(s1, "%2ld'", kmin);
  strcat(s, s1);
  if (iflag & BIT_ROUND_MIN)
    goto return_dms;
  x -= kmin;
  x *= 60;
  ksec = (long) x;
  if (iflag & BIT_ROUND_SEC)
    sprintf(s1, "%2ld\"", ksec);
  else
    sprintf(s1, "%2ld", ksec);
  strcat(s, s1);
  if (iflag & BIT_ROUND_SEC)
    goto return_dms;
  x -= ksec;
  k = (long) (x * 10000);
  sprintf(s1, ".%04ld", k);
  strcat(s, s1);
  return_dms:;
  if (sgn < 0) {
    sp = strpbrk(s, "0123456789"); 
    *(sp-1) = '-';
  }
  return(s);
}

