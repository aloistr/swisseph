#include "testsuite_facade.h"

TESTSUITE(6,"Houses functions")

double cusps[37],ascmc[10], jd_ut, geolat, geolon, armc,xx[6];
double cusp_speed[37], ascmc_speed[10];
char serr[255], *sp;
int rc, ihsy;
swe_close();
swe_set_ephe_path(NULL);

SETUP {
  *serr = '\0';
  jd_ut = GET_D(jd) + GET_D(ut) / 24.0;
  ihsy = GET_I(ihsy);
  geolat = GET_D(geolat);
  geolon = GET_D(geolon);
  }

TEARDOWN {
  if (GET_I(initialize)) swe_close( );
  }

TESTCASE(1,"swe_houses( ) ") {
  rc = swe_houses(jd_ut, geolat, geolon, ihsy, cusps, ascmc);
  check_swehouses_results(rc,jd_ut,ihsy,cusps,ascmc,serr,ctx);
  }
TESTCASE(2,"swe_houses_ex( ) - degrees and radians") {
  rc = swe_houses_ex(jd_ut, GET_I(iflag), geolat, geolon, ihsy, cusps, ascmc);
  check_swehouses_results(rc,jd_ut,ihsy,cusps,ascmc,serr,ctx);
  }
TESTCASE(3,"swe_houses_ex( ) - sidereal ") {
  swe_set_sid_mode(GET_I(isid), 0, 0);
  rc = swe_houses_ex(jd_ut, GET_I(iflag), geolat, geolon, ihsy, cusps, ascmc);
//if (ihsy == 'P') fprintf(stderr, " ==> %f\n", cusps[1]);
  check_swehouses_results(rc,jd_ut,ihsy,cusps,ascmc,serr,ctx);
  }
TESTCASE(4,"swe_houses_armc( ) - Koch and Placidus") {
  double eps;
  rc = swe_calc(jd_ut, SE_ECL_NUT, 0, xx, serr);	// 
  eps = xx[0];
  armc = swe_degnorm(swe_sidtime(jd_ut) + geolon);
  rc = swe_houses_armc(armc, geolat, eps, ihsy, cusps, ascmc);
  check_swehouses_armc_results(rc,armc,ihsy,cusps,ascmc,serr,ctx);
  }
TESTCASE(5,"swe_house_name( ) ") {
  sp = swe_house_name(ihsy);
  CHECK_S(sp);
 }
TESTCASE(6,"swe_house_pos( ) ") {
  double hp;
  double eps;
  // jd_ut loops through a day, 
  rc = swe_calc(jd_ut, SE_ECL_NUT, 0, xx, serr);	// obliquity eps
  eps = xx[0];
  armc = swe_degnorm(swe_sidtime(jd_ut) * 15 + geolon);
  rc = swe_calc(jd_ut, SE_SUN, 0, xx, serr);	// sun pos xx
  hp = swe_house_pos(armc, geolat, eps, ihsy, xx, serr);
  CHECK_D(armc);	// to show
  CHECK_D(xx[0]);	// to show
  CHECK_D(hp);
  }
TESTCASE(7,"swe_gauquelin_sector( ) ") {
  // jd_ut loops through a day, 
  int imeth = GET_I(imeth);
  double gp;
  double geopos[3] = {geolon, geolat, 100};
  rc = swe_gauquelin_sector(jd_ut, SE_SUN, NULL, 0, imeth, geopos, 0.0, 20.0, &gp, serr);	
  CHECK_D(jd_ut);	// to show
  CHECK_D(gp);
  CHECK_I(rc);
  CHECK_S(serr);
  }
TESTCASE(8,"swe_houses_ex2( ) - with speeds of cusps ") {
  rc = swe_houses_ex2(jd_ut, GET_I(iflag), geolat, geolon, ihsy, cusps, ascmc, cusp_speed, ascmc_speed, serr);
//if (ihsy == 'P') fprintf(stderr, " ==> %f\n", cusps[1]);
  check_swehouses_ex2_results(rc,jd_ut,ihsy,cusps,ascmc,cusp_speed,ascmc_speed,serr,ctx);
  }
TESTCASE(9,"swe_houses_armc_ex2( ) - Koch and Placidus with cusp speeds") {
  double eps;
  rc = swe_calc(jd_ut, SE_ECL_NUT, 0, xx, serr);	// 
  eps = xx[0];
  armc = swe_degnorm(swe_sidtime(jd_ut) + geolon);
  rc = swe_houses_armc_ex2(armc, geolat, eps, ihsy, cusps, ascmc,cusp_speed,ascmc_speed,serr);
  check_swehouses_armc_ex2_results(rc,armc,ihsy,cusps,ascmc,cusp_speed,ascmc_speed,serr,ctx);
  }

END_TESTSUITE
