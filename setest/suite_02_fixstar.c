#include "testsuite_facade.h"

TESTSUITE(2,"Fixed stars")

double xx[6],jd;
int rc, iflag, ipl;
char serr[255], star[100];

// swe_fixstar doesn't read env.var. SE_EPHE_PATH
// swe_calc does. So for this suite, we have to read it explicitly
swe_set_ephe_path(NULL);


TESTCASE(1,"Fixed stars with Ephemeris Time") {

  jd = GET_D(jd);
  iflag = GET_I(iflag);
  GET_S(star);

  rc = swe_fixstar(star,jd,iflag,xx,serr);

  check_swecalc_results(rc,xx,serr,ctx);

  }

TESTCASE(2,"Fixed stars with Universal Time") {

  jd = GET_D(jd);
  GET_S(star);

  rc = swe_fixstar_ut(star,jd,iflag,xx,serr);

  check_swecalc_results(rc,xx,serr,ctx);

  }

TESTCASE(3,"Magnitude of a fixed star") {
  double mag;

  GET_S(star);
  rc = swe_fixstar_mag(star,&mag,serr);

  CHECK_D(mag);
  CHECK_I(rc);
  CHECK_S(serr); 

  }

TESTCASE(4, "Mercury, then Betelgeuze") {
  swe_set_ephe_path(NULL);
  jd = GET_D(jd);
  ipl = 2;
  GET_S(star);
  rc = swe_calc(jd, ipl, iflag, xx, serr);
  rc = swe_fixstar(star,jd,iflag,xx,serr);

  check_swecalc_results(rc,xx,serr,ctx);

  }

TESTCASE(5, "Algol, then Betelgeuze") {
  swe_set_ephe_path(NULL);
  jd = GET_D(jd);
  ipl = 2;
  strcpy(star, "Algol");
  rc = swe_fixstar(star,jd,iflag,xx,serr);
  GET_S(star);
  rc = swe_fixstar(star,jd,iflag,xx,serr);

  check_swecalc_results(rc,xx,serr,ctx);

  }

TESTCASE(6,"Fixed stars (swe_fixstar2) with Ephemeris Time") {

  swe_set_ephe_path(NULL);
  jd = GET_D(jd);
  iflag = GET_I(iflag);
  GET_S(star);

  rc = swe_fixstar2(star,jd,iflag,xx,serr);

  check_swecalc_results(rc,xx,serr,ctx);

  }

TESTCASE(7,"Fixed stars (swe_fixstar2) with Universal Time") {

  swe_set_ephe_path(NULL);
  jd = GET_D(jd);
  GET_S(star);

  rc = swe_fixstar2_ut(star,jd,iflag,xx,serr);

  check_swecalc_results(rc,xx,serr,ctx);

  }

END_TESTSUITE
