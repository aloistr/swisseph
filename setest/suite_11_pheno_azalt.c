#include "testsuite_facade.h"

TESTSUITE(11,"Phenomena, horizon coordinates, refraction, coordinate transformation")

// Added 2026-11-22. The round trips swe_azalt() -> swe_azalt_rev() and
// swe_cotrans() with eps -> -eps must return the input (CHECK_EQUALS_D).
// swe_refrac() has no round-trip check: SE_APP_TO_TRUE is not the exact
// inverse of SE_TRUE_TO_APP (up to about 2 arcsec).

static double jd, geopos[3], xin[3], xpheno[20], xaz[3], xpo[6], xpn[6], xback[6], xrefr[4];
static char serr[255];

SUITE_INIT {
  swe_set_ephe_path(NULL);
}

SETUP {
  *serr = '\0';
  }

TEARDOWN {
  if (GET_I(initialize)) swe_close( );
  }

TESTCASE(1,"swe_pheno( ) - phase angle, phase, elongation, diameter, magnitude") {
  int ipl = GET_I(ipl);
  int iflag = GET_I(iephe) | GET_I(iflag);
  jd = GET_D(jd);
  int rc = swe_pheno(jd, ipl, iflag, xpheno, serr);
  CHECK_I(rc);
  CHECK_DD(xpheno,5);
  CHECK_S(serr);
  }

TESTCASE(2,"swe_pheno_ut( ) - phase angle, phase, elongation, diameter, magnitude") {
  int ipl = GET_I(ipl);
  int iflag = GET_I(iephe) | GET_I(iflag);
  jd = GET_D(jd);
  int rc = swe_pheno_ut(jd, ipl, iflag, xpheno, serr);
  CHECK_I(rc);
  CHECK_DD(xpheno,5);
  CHECK_S(serr);
  }

TESTCASE(3,"swe_azalt( ) and swe_azalt_rev( ) - ecliptic coordinates") {
  jd = GET_D(jd);
  geopos[0] = GET_D(geolon);
  geopos[1] = GET_D(geolat);
  geopos[2] = GET_D(geoalt);
  xin[0] = GET_D(lon);
  xin[1] = GET_D(lat);
  xin[2] = 1;
  swe_azalt(jd, SE_ECL2HOR, geopos, GET_D(atpress), GET_D(attemp), xin, xaz);
  CHECK_DD(xaz,3);
  // from azimuth and true altitude back to the ecliptic position
  swe_azalt_rev(jd, SE_HOR2ECL, geopos, xaz, xback);
  CHECK_EQUALS_D(xback[0], xin[0]);
  CHECK_EQUALS_D(xback[1], xin[1]);
  }

TESTCASE(4,"swe_azalt( ) and swe_azalt_rev( ) - equatorial coordinates") {
  jd = GET_D(jd);
  geopos[0] = GET_D(geolon);
  geopos[1] = GET_D(geolat);
  geopos[2] = GET_D(geoalt);
  xin[0] = GET_D(ra);
  xin[1] = GET_D(decl);
  xin[2] = 1;
  swe_azalt(jd, SE_EQU2HOR, geopos, GET_D(atpress), GET_D(attemp), xin, xaz);
  CHECK_DD(xaz,3);
  // from azimuth and true altitude back to the equatorial position
  swe_azalt_rev(jd, SE_HOR2EQU, geopos, xaz, xback);
  CHECK_EQUALS_D(xback[0], xin[0]);
  CHECK_EQUALS_D(xback[1], xin[1]);
  }

TESTCASE(5,"swe_refrac( ) - true to apparent altitude and back") {
  double alt = swe_refrac(GET_D(inalt), GET_D(atpress), GET_D(attemp), GET_I(calc_flag));
  CHECK_D(alt);
  }

TESTCASE(6,"swe_refrac_extended( ) - refraction for an observer above sea level") {
  double alt = swe_refrac_extended(GET_D(inalt), GET_D(geoalt), GET_D(atpress), GET_D(attemp), GET_D(lapse_rate), GET_I(calc_flag), xrefr);
  CHECK_D(alt);
  CHECK_DD(xrefr,4);
  }

TESTCASE(7,"swe_cotrans( ) - with eps and back with -eps") {
  double eps = GET_D(eps);
  xpo[0] = GET_D(lon);
  xpo[1] = GET_D(lat);
  xpo[2] = GET_D(dist);
  swe_cotrans(xpo, xpn, eps);
  CHECK_DD(xpn,3);
  swe_cotrans(xpn, xback, -eps);
  CHECK_EQUALS_D(xback[0], xpo[0]);
  CHECK_EQUALS_D(xback[1], xpo[1]);
  CHECK_EQUALS_D(xback[2], xpo[2]);
  }

TESTCASE(8,"swe_cotrans_sp( ) - position and speed, with eps and back with -eps") {
  double eps = GET_D(eps);
  xpo[0] = GET_D(lon);
  xpo[1] = GET_D(lat);
  xpo[2] = GET_D(dist);
  xpo[3] = GET_D(lonspeed);
  xpo[4] = GET_D(latspeed);
  xpo[5] = GET_D(distspeed);
  swe_cotrans_sp(xpo, xpn, eps);
  CHECK_DD(xpn,6);
  swe_cotrans_sp(xpn, xback, -eps);
  CHECK_EQUALS_D(xback[0], xpo[0]);
  CHECK_EQUALS_D(xback[1], xpo[1]);
  CHECK_EQUALS_D(xback[2], xpo[2]);
  CHECK_EQUALS_D(xback[3], xpo[3]);
  CHECK_EQUALS_D(xback[4], xpo[4]);
  CHECK_EQUALS_D(xback[5], xpo[5]);
  }

END_TESTSUITE
