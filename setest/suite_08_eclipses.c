#include "testsuite_facade.h"

TESTSUITE(8,"Eclipses")

double jd, xxtret[10], xxgeopos[3], xxattr[20];
double geolat, geolon, altitude;
char serr[255], star[100];;
int rc, ipl;
swe_set_ephe_path(NULL);

SETUP {
  *serr = '\0';
  geolat = GET_D(geolat);
  geolon = GET_D(geolon);
  altitude = GET_D(altitude);
  }

TEARDOWN {
  if (GET_I(initialize)) swe_close( );
  }

TESTCASE(1,"swe_sol_eclipse_when_glob( ) ") {
  jd = GET_D(jd);
  rc = swe_sol_eclipse_when_glob(jd, GET_I(iephe), GET_I(ifltype),
       xxtret, GET_I(backward), serr);
  CHECK_I(rc);
  CHECK_DD(xxtret,10);
  }

TESTCASE(2,"swe_sol_eclipse_where( ) ") {
  jd = GET_D(jd);
  rc = swe_sol_eclipse_where(jd, GET_I(iephe), xxgeopos, xxattr, serr);
  CHECK_I(rc);
  CHECK_DD(xxgeopos,2);
  CHECK_DD(xxattr,8);
  }

TESTCASE(3,"swe_sol_eclipse_when_loc( ) ") {
  jd = GET_D(jd);
  double geopos[3] = {geolon, geolat, altitude};
  rc = swe_sol_eclipse_when_loc(jd, GET_I(iephe), geopos, xxtret, xxattr, GET_I(backward), serr);
  CHECK_I(rc);
  CHECK_DD(xxtret,7);
  CHECK_DD(xxattr,11);
  }

TESTCASE(4,"swe_sol_eclipse_how( ) ") {
  jd = GET_D(jd);
  double geopos[3] = {geolon, geolat, altitude};
  rc = swe_sol_eclipse_how(jd, GET_I(iephe), geopos, xxattr, serr);
  CHECK_I(rc);
  CHECK_DD(xxattr,11);
  }

TESTCASE(5,"swe_lun_occult_when_glob( ) ") {
  jd = GET_D(jd);
  ipl = GET_I(ipl);
  *star = '\0';
  if (ipl == -1)
    GET_S(star);
  rc = swe_lun_occult_when_glob(jd, ipl, star, GET_I(iephe), GET_I(ifltype),
       xxtret, GET_I(backward), serr);
  CHECK_I(rc);
  CHECK_DD(xxtret,10);
  }

TESTCASE(6,"swe_lun_occult_where( ) ") {
  jd = GET_D(jd);
  ipl = GET_I(ipl);
  *star = '\0';
  if (ipl == -1)
    GET_S(star);
  rc = swe_lun_occult_where(jd, ipl, star, GET_I(iephe), xxgeopos, xxattr, serr);
  CHECK_DD(xxgeopos,2);
  CHECK_DD(xxattr,8);
  }

TESTCASE(7,"swe_lun_occult_when_loc( ) ") {
  double geopos[3] = {geolon, geolat, altitude};
  jd = GET_D(jd);
  ipl = GET_I(ipl);
  *star = '\0';
  if (ipl == -1)
    GET_S(star);
  rc = swe_lun_occult_when_loc(jd, ipl, star, GET_I(iephe), geopos,
       xxtret, xxattr, GET_I(backward), serr);
  CHECK_I(rc);
  CHECK_DD(xxtret,7);
  CHECK_DD(xxattr,11);
  }

TESTCASE(8,"swe_lun_eclipse_when( ) ") {
  jd = GET_D(jd);
  rc = swe_lun_eclipse_when(jd, GET_I(iephe), GET_I(ifltype),
       xxtret, GET_I(backward), serr);
  CHECK_I(rc);
  CHECK_DD(xxtret,10);
  }

TESTCASE(9,"swe_lun_eclipse_when_loc( ) ") {
  jd = GET_D(jd);
  double geopos[3] = {geolon, geolat, altitude};
  rc = swe_sol_eclipse_when_loc(jd, GET_I(iephe), geopos, xxtret, xxattr, GET_I(backward), serr);
  CHECK_I(rc);
  CHECK_DD(xxtret,10);
  CHECK_DD(xxattr,11);
  }

TESTCASE(10,"swe_lun_eclipse_how( ) ") {
  jd = GET_D(jd);
  double geopos[3] = {geolon, geolat, altitude};
  rc = swe_lun_eclipse_how(jd, GET_I(iephe), geopos, xxattr, serr);
  CHECK_I(rc);
  CHECK_DD(xxattr,11);
  }

END_TESTSUITE
