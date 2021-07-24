#include "testsuite_facade.h"

TESTSUITE(3, "Various small functions (names et al.)")

TESTCASE(1, "swe_get_planet_name( ) - Getting a planet name") {
  int ipl = GET_I(ipl);
  char name[255];
  swe_get_planet_name(ipl,name);
  CHECK_S(name);
}

TESTCASE(2, "swe_get_aya_name( ) - Getting an ayanamsa name") {
  int sid_mode = GET_I(sid_mode);
  const char *name = swe_get_ayanamsa_name(sid_mode);
  CHECK_S(name);
}


END_TESTSUITE




