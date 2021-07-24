#include "testsuite_facade.h"

TESTSUITE(9,"risings, heliacal risings")

double jd, xxtret[10];
double geolat, geolon, altitude, tret;
double atpress, attemp, athumid, atktot, obsage, obsSN;
char serr[255], star[100], object[100];
int rc, ipl;
swe_set_ephe_path(NULL);

SETUP {
  *serr = '\0';
  geolat = GET_D(geolat);
  geolon = GET_D(geolon);
  altitude = GET_D(altitude);
  atpress = GET_D(atpress);
  attemp = GET_D(attemp);
  athumid = GET_D(athumid);
  atktot = GET_D(atktot);
  obsage = GET_D(obsage);
  obsSN = GET_D(obsSN);
  }

TEARDOWN {
  if (GET_I(initialize)) swe_close( );
  }

TESTCASE(1,"swe_rise_trans( ) ") {
  double geopos[3] = {geolon, geolat, altitude};
  int rsmi = GET_I(ifltype) | GET_I(method);
  jd = GET_D(jd);
  ipl = GET_I(ipl);
  *star = '\0';
  if (ipl == -1)
    GET_S(star);
  rc = swe_rise_trans(jd, GET_I(ipl), star, GET_I(iephe), rsmi,
       geopos, GET_D(atpress), GET_D(attemp), &tret, serr);
  CHECK_I(rc);
  CHECK_D(tret);
  }

TESTCASE(2,"swe_rise_trans_true_hor( ) ") {
  double geopos[3] = {geolon, geolat, altitude};
  int rsmi = GET_I(ifltype) | GET_I(method);
  jd = GET_D(jd);
  ipl = GET_I(ipl);
  *star = '\0';
  if (ipl == -1)
    GET_S(star);
  rc = swe_rise_trans_true_hor(jd, GET_I(ipl), star, GET_I(iephe), rsmi,
       geopos, GET_D(atpress), GET_D(attemp), GET_D(horhgt), &tret, serr);
  CHECK_I(rc);
  CHECK_D(tret);
  }

TESTCASE(3,"swe_heliacal_ut( ) ") {
  double geopos[3] = {geolon, geolat, altitude};
  double datm[4] = {atpress, attemp, athumid, atktot};
  double dobs[5] = {obsage, obsSN,};
  jd = GET_D(jd);
  rc = swe_heliacal_ut(jd, geopos, datm, dobs, GET_S(object), GET_I(evtype), GET_I(helflag), xxtret, serr);
  CHECK_I(rc);
  CHECK_DD(xxtret,3);
  }

TESTCASE(4,"swe_heliacal_pheno_ut()( ) ") {
  double geopos[3] = {geolon, geolat, altitude};
  double datm[4] = {atpress, attemp, athumid, atktot};
  double dobs[5] = {obsage, obsSN,};
  jd = GET_D(jd);
  rc = swe_heliacal_pheno_ut(jd, geopos, datm, dobs, GET_S(object), GET_I(evtype), GET_I(helflag), xxtret, serr);
  CHECK_I(rc);
  CHECK_DD(xxtret,3);
  }

TESTCASE(5,"swe_vis_limit_mag()( ) ") {
  double geopos[3] = {geolon, geolat, altitude};
  double datm[4] = {atpress, attemp, athumid, atktot};
  double dobs[5] = {obsage, obsSN,};
  jd = GET_D(jd);
  rc = swe_vis_limit_mag(jd, geopos, datm, dobs, GET_S(object), GET_I(helflag), xxtret, serr);
  CHECK_I(rc);
  CHECK_DD(xxtret,3);
  }

END_TESTSUITE
