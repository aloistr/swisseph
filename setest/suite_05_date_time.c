#include "testsuite_facade.h"

TESTSUITE(5,"Date and time functions")

  char serr[255];
  int rc;
swe_set_ephe_path(NULL);

  SETUP {
    *serr = '\0';
    rc = 0;
  }

  TESTCASE(1,"Julian Date from Calendar date") {
    double jd = swe_julday(
        GET_I(year),
        GET_I(month),
        GET_I(day),
        GET_D(hour),
        GET_I(gregflag));
    CHECK_D(jd);
  }

  TESTCASE(2,"Calendar date from Julian Date") {
    int year,month,day;
    double ut;
    swe_revjul (GET_D(jd), GET_I(gregflag),
           &year, &month, &day, &ut);
    CHECK_I(year);
    CHECK_I(month);
    CHECK_I(day);
    CHECK_D(ut);
  }

  TESTCASE(3,"Delta T") {
    double deltat;
    int iephe = GET_I(iephe);
    double jd = GET_D(jd);
    if (iephe > 0) {
//fprintf(stderr, "jd=%.11f, ie=%d\n", jd, iephe);
      deltat = swe_deltat_ex(jd,iephe,serr);
//fprintf(stderr, "dt=%.11f  s=%s\n", deltat, serr);
      CHECK_S(serr);
    }
    else {
      deltat = swe_deltat(jd);
    }
    CHECK_D(deltat);
  }

  TESTCASE(4,"Equation of time") {
    double E;
    int rc = swe_time_equ(GET_D(jd), &E, serr);
    CHECK_D(E);
    CHECK_I(rc);
  }

  TESTCASE(5,"Local Mean Time to Local Apparent Time") {
    double tjd_lmt, tjd_lat, geolon;
    tjd_lmt = GET_D(tjd_lmt);
    geolon = GET_D(geolon);
    rc = swe_lmt_to_lat(tjd_lmt,geolon,&tjd_lat,serr);
    CHECK_I(rc);
    CHECK_S(serr);
    CHECK_D(tjd_lat);
  }

  TESTCASE(6,"Local Apparent Time to Local Mean Time") {
    double tjd_lmt, tjd_lat, geolon;
    tjd_lat = GET_D(tjd_lat);
    geolon = GET_D(geolon);
    rc = swe_lat_to_lmt(tjd_lat,geolon,&tjd_lmt,serr);
    CHECK_I(rc);
    CHECK_S(serr);
    CHECK_D(tjd_lmt);
  }




END_TESTSUITE
