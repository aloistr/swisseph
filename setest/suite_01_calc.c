#include "testsuite_facade.h"


TESTSUITE(1,"Various swe_calc calls in different modes")

double xx[6],jd;
int iflag, iephe; // Keep ephemeris selector separate from other flags
char serr[255];

swe_set_ephe_path(NULL);
swe_set_jpl_file("de431.eph");

SETUP {
  iflag = GET_I(iflag);
  iephe = GET_I(iephe);
  jd = GET_D(jd);
  *serr = '\0';
  }

TEARDOWN {
  if (GET_I(initialize)) swe_close( );
  }

TESTCASE(1,"swe_calc( ) - Julian Date with Ephemeris Time") {
  int rc = swe_calc(jd, GET_I(ipl), iflag | iephe, xx, serr);
  check_swecalc_results(rc,xx,serr,ctx);
  }

TESTCASE(2,"swe_calc_ut( ) - Julian Date with Universal Time") {
  int ipl = GET_I(ipl); 
swe_set_ephe_path(NULL);
  int rc = swe_calc_ut(jd, ipl, iflag | iephe, xx, serr);
  check_swecalc_results(rc,xx,serr,ctx);
  }

TESTCASE(3,"swe_set_topo( ) - Using topographic coordinates") {
  double geolon = GET_D(geolon),
         geolat = GET_D(geolat),
         altitude = GET_D(altitude);
swe_set_ephe_path(NULL);
  int ipl = GET_I(ipl);
  swe_set_topo( geolon, geolat, altitude );
  int rc = swe_calc(jd, ipl, iflag | iephe, xx, serr);
  check_swecalc_results(rc,xx,serr,ctx);
}

TESTCASE(4,"swe_calc( ) - Equatorial followed by Ecliptic") {
  iflag = SEFLG_EQUATORIAL | iephe;
  int ipl = GET_I(ipl);
swe_set_ephe_path(NULL);
  swe_calc(jd, ipl, iflag, xx, serr);
  iflag =  iephe;
  int rc = swe_calc(jd, ipl, iflag, xx, serr);
  CHECK_EQUALS_I(rc,iflag); // we expect rc = iflag = iephe
  }

TESTCASE(5,"swe_calc_pctr( ) - planetocentic position") {
  int rc = swe_calc_pctr(jd, GET_I(ipl), GET_I(iplctr), iflag | iephe, xx, serr);
  check_swecalc_results(rc,xx,serr,ctx);
  }

END_TESTSUITE
