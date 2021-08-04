#include "testsuite_facade.h"


TESTSUITE(10,"Various swe_*cross calls ")

double xx[6],jd;
int iflag, iephe; // Keep ephemeris selector separate from other flags
char serr[255];

swe_set_ephe_path(NULL);
swe_set_jpl_file("de431.eph");

SETUP {
  iephe = GET_I(iephe);
  jd = GET_D(jd);
  *serr = '\0';
  }

TEARDOWN {
  if (GET_I(initialize)) swe_close( );
  }
TESTCASE(1,"swe_solcross( ) - sun crossing a position") {
  iflag = iephe;
  double  xcross = GET_D(xcross);
  double jx = swe_solcross(xcross, jd,  iflag, serr);
  int rc = swe_calc(jx, SE_SUN, iflag, xx, serr);
  CHECK_I(rc);
  CHECK_D(jx);
  CHECK_EQUALS_D(xcross, xx[0]);
  CHECK_S(serr);
  }

TESTCASE(2,"swe_solcross_ut( ) - sun crossing a position") {
  iflag = iephe;
  double  xcross = GET_D(xcross);
  double jx = swe_solcross_ut(xcross, jd,  iflag, serr);
  int rc = swe_calc_ut(jx, SE_SUN, iflag, xx, serr);
  CHECK_I(rc);
  CHECK_D(jx);
  CHECK_EQUALS_D(xcross, xx[0]);
  CHECK_S(serr);
  }

TESTCASE(3,"swe_mooncross( ) - moon crossing a position") {
  iflag = iephe;
  double  xcross = GET_D(xcross);
  double jx = swe_mooncross(xcross, jd,  iflag, serr);
  int rc = swe_calc(jx, SE_MOON, iflag, xx, serr);
  CHECK_I(rc);
  CHECK_D(jx);
  CHECK_EQUALS_D(xcross, xx[0]);
  CHECK_S(serr);
  }

TESTCASE(4,"swe_mooncross_ut( ) - moon crossing a position") {
  iflag = iephe;
  double  xcross = GET_D(xcross);
  double jx = swe_mooncross_ut(xcross, jd,  iflag, serr);
  int rc = swe_calc_ut(jx, SE_MOON, iflag, xx, serr);
  CHECK_I(rc);
  CHECK_D(jx);
  CHECK_EQUALS_D(xcross, xx[0]);
  CHECK_S(serr);
  }

TESTCASE(5,"swe_mooncross_node( ) - moon crossing node position") {
  iflag = iephe;
  double xlon, xlat;
  double jx = swe_mooncross_node(jd,  iflag, &xlon, &xlat, serr);
  int rc = swe_calc(jx, SE_MOON, iflag, xx, serr);
  CHECK_I(rc);
  CHECK_D(jx);
  CHECK_D(xlon);
  CHECK_D(xlat);
  CHECK_EQUALS_D(xx[1], 0);	// latitude must  be zero
  CHECK_S(serr);
  }

TESTCASE(6,"swe_mooncross_node_ut( ) - moon crossing node position") {
  iflag = iephe;
  double xlon, xlat;
  double jx = swe_mooncross_node_ut(jd,  iflag, &xlon, &xlat, serr);
  int rc = swe_calc_ut(jx, SE_MOON, iflag, xx, serr);
  CHECK_I(rc);
  CHECK_D(jx);
  CHECK_D(xlon);
  CHECK_D(xlat);
  CHECK_EQUALS_D(xx[1], 0);	// latitude must  be zero
  CHECK_S(serr);
  }

TESTCASE(7,"swe_helio_cross( ) - heliocentric crossing planet over a position") {
  iflag = iephe;
  double jx;
  double  xcross = GET_D(xcross);
  int ipl = GET_I(ipl);
  int dir = GET_I(dir);
  int rc = swe_helio_cross(ipl, xcross, jd,  iflag, dir, &jx, serr);
  CHECK_D(jx);
  CHECK_I(rc);
  CHECK_S(serr);
  }

TESTCASE(8,"swe_helio_cross_ut( ) - heliocentric crossing planet over a position") {
  iflag = iephe;
  double jx;
  double  xcross = GET_D(xcross);
  int ipl = GET_I(ipl);
  int dir = GET_I(dir);
  int rc = swe_helio_cross_ut(ipl, xcross, jd,  iflag, dir, &jx, serr);
  CHECK_D(jx);
  CHECK_I(rc);
  CHECK_S(serr);
  }


END_TESTSUITE
