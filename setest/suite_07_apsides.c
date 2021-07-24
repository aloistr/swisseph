#include "testsuite_facade.h"

TESTSUITE(7,"Apsides and orbital elements functions")

double jd, xxnasc[6],xxndsc[6],xxperi[6],xxaphe[6], xxdret[20];
int iflag, iephe; // Keep ephemeris selector separate from other flags
char serr[255];
int rc;
swe_set_ephe_path(NULL);

SETUP {
  iflag = GET_I(iflag);
  iephe = GET_I(iephe);
  *serr = '\0';
  }

TEARDOWN {
  if (GET_I(initialize)) swe_close( );
  }

TESTCASE(1,"swe_nod_aps( ) ") {
  jd = GET_D(jd);
  rc = swe_nod_aps(jd, GET_I(ipl), iflag|iephe,  GET_I(method), xxnasc, xxndsc, xxperi, xxaphe, serr);
  CHECK_I(rc);
  CHECK_S(serr);
  CHECK_DD(xxnasc,6);
  CHECK_DD(xxndsc,6);
  CHECK_DD(xxperi,6);
  CHECK_DD(xxaphe,6);
  }

TESTCASE(2,"swe_nod_aps_ut( ) ") {
  jd = GET_D(jd);
  rc = swe_nod_aps_ut(jd, GET_I(ipl), iflag|iephe,  GET_I(method), xxnasc, xxndsc, xxperi, xxaphe, serr);
  CHECK_I(rc);
  CHECK_S(serr);
  CHECK_DD(xxnasc,6);
  CHECK_DD(xxndsc,6);
  CHECK_DD(xxperi,6);
  CHECK_DD(xxaphe,6);
  }

TESTCASE(3,"swe_get_orbital_elements( ) ") {
  jd = GET_D(jd);
  rc = swe_get_orbital_elements(jd, GET_I(ipl), iflag|iephe,  xxdret, serr);
  CHECK_I(rc);
  CHECK_S(serr);
  CHECK_DD(xxdret,17);
  }

TESTCASE(4,"swe_orbit_max_min_true_distance( ) ") {
  double dmax, dmin, dtrue;
  jd = GET_D(jd);
  rc = swe_orbit_max_min_true_distance(jd, GET_I(ipl), iflag|iephe,  &dmax, &dmin, &dtrue, serr);
  CHECK_I(rc);
  CHECK_S(serr);
  CHECK_D(dmax);
  CHECK_D(dmin);
  CHECK_D(dtrue);
  }

END_TESTSUITE
