
/* 

  swemini.c	A minimal program to test the Swiss Ephemeris.

  Input: a date (in gregorian calendar, sequence day.month.year)
  	if no date is entered, 1 Jan 2022 is used. Next time, the date
	advances by one day.
  Output: Planet positions at midnight Universal time, ecliptic coordinates,
          geocentric apparent positions relative to true equinox of date, as 
          usual in western astrology.
		
   
  Authors: Dieter Koch and Alois Treindl.

  The code of sample program swemini.c is in the public domain.
  (But not the code of the library functions called by it.)

**************************************************************/


#include "swephexp.h" 	

char *smon[] = {NULL, "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

int main(void)
{
  char sdate[AS_MAXCH], snam[40], serr[AS_MAXCH];  
  int jday = 1, jmon = 1, jyear = 2022;
  double jut = 0.0;
  double tjd, te, x2[6];
  int32 iflag, iflgret;
  int p;
  swe_set_ephe_path(NULL);
  iflag = SEFLG_SPEED;
  while (TRUE) {
    printf("\nDate (d.m.y) ?");
    fgets(sdate, sizeof(sdate)-1, stdin);
    // stop if a period . is entered
    if (*sdate == '.') 
      return OK;
    sscanf (sdate, "%d%*c%d%*c%d", &jday,&jmon,&jyear);
    // we have day, month and year and convert to Julian day number
    if (jmon < 1 || jmon > 12) {
      printf("illegal month %d\n", jmon);
      continue;
    }
    tjd = swe_julday(jyear,jmon,jday,jut,SE_GREG_CAL);        
    // compute Ephemeris time from Universal time by adding delta_t
    te = tjd + swe_deltat(tjd);
    printf("date: %02d %s %04d at 0:00 Universal time, jd=%.1lf\n", jday, smon[jmon], jyear, tjd);
    printf("planet     \tlongitude\tlatitude\tdistance\tspeed long.\n");
    for (p = SE_SUN; p <= SE_CHIRON; p++) { // a loop over all planets
      if (p == SE_EARTH) continue;
      swe_get_planet_name(p, snam); //  get the name of the planet p
      // do the coordinate calculation for this planet p
      iflgret = swe_calc(te, p, iflag, x2, serr);
      // if there is a problem, a negative value is returned and an error message is in serr.
      if (iflgret < 0) {
	printf("%10s\terror: %s\n", snam, serr);
	continue;
      }
      if (iflgret != iflag)
	printf("warning: iflgret != iflag. %s\n", serr);
      // print the coordinates
      printf("%10s\t%11.7f\t%10.7f\t%10.7f\t%10.7f\n",
	     snam, x2[0], x2[1], x2[2], x2[3]);
    }
    tjd++;	// if date entry is empty, take next day
    swe_revjul(tjd, SE_GREG_CAL, &jyear, &jmon, &jday, &jut);
  }
  return OK;
}
