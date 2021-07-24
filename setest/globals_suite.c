// This source file (i.e. globals_suite.c) contains utility functions for writing test case code

#include "testsuite_facade.h"

void check_swecalc_results(int rc,double *xx, char* serr, test_context* ctx) {
  CHECK_DD(xx,6);
  CHECK_I(rc);
  CHECK_S(serr);
}

void check_swehouses_results(int rc, double jd_ut, int ihsy, double *cusps, double *ascmc, char* serr, test_context* ctx) {
  CHECK_D(jd_ut);
  if (ihsy == 'G') {
    CHECK_DD(cusps,37);
  } else {
    CHECK_DD(cusps,13);
  }
  CHECK_DD(ascmc,6);	// 6 = do not test coasc2, 8 = all 
  CHECK_I(rc);
}

void check_swehouses_ex2_results(int rc, double jd_ut, int ihsy, double *cusps, double *ascmc, double *cusp_speed, double *ascmc_speed, char* serr, test_context* ctx) {
  CHECK_D(jd_ut);
  if (ihsy == 'G') {
    CHECK_DD(cusps,37);
    CHECK_DD(cusp_speed,37);
  } else {
    CHECK_DD(cusps,13);
    CHECK_DD(cusp_speed,13);
  }
  CHECK_DD(ascmc,6);	// 6 = do not test coasc2, 8 = all 
  CHECK_DD(ascmc_speed,6);	// 6 = do not test coasc2, 8 = all 
  CHECK_I(rc);
}

void check_swehouses_armc_results(int rc, double armc, int ihsy, double *cusps, double *ascmc, char* serr, test_context* ctx) {
  CHECK_D(armc);
  if (ihsy == 'G') {
    CHECK_DD(cusps,37);
  } else {
    CHECK_DD(cusps,13);
  }
  CHECK_DD(ascmc,6);	// 6 = do not test coasc2, 8 = all 
  CHECK_I(rc);
}

void check_swehouses_armc_ex2_results(int rc, double armc, int ihsy, double *cusps, double *ascmc, double *cusp_speed, double *ascmc_speed, char* serr, test_context* ctx) {
  CHECK_D(armc);
  if (ihsy == 'G') {
    CHECK_DD(cusps,37);
    CHECK_DD(cusp_speed,37);
  } else {
    CHECK_DD(cusps,13);
    CHECK_DD(cusp_speed,13);
  }
  CHECK_DD(ascmc,6);	// 6 = do not test coasc2, 8 = all 
  CHECK_DD(ascmc_speed,6);	// 6 = do not test coasc2, 8 = all 
  CHECK_I(rc);
}

// End of source file globals_suite.c

