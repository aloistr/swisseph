// Indicator "all (testsuites|testcases of a testsuite)"

const int ALL = 0;

const double DEFAULT_PRECISION = 1.e-9;

const char* SECTION_KEYWORDS[] = {
  "GENERAL",
  "TESTSUITE",
  "TESTCASE",
  "ITERATION",
  ""
};

const char* PARAMETER_SECTIONS[] = {
  "GENERAL",
  ""
};

const char* DEFAULT_TEST_COLLECTION = "test";

const char* PREPARE_FIXTURE_COMMAND = "m4 prepare_fix.m4";

const char* SEFLG[] = {
    "SEFLG_JPLEPH",
    "SEFLG_SWIEPH",
    "SEFLG_MOSEPH",
    "SEFLG_HELCTR",
    "SEFLG_TRUEPOS",
    "SEFLG_J2000",
    "SEFLG_NONUT",
    "SEFLG_SPEED3",
    "SEFLG_SPEED",
    "SEFLG_NOGDEFL",
    "SEFLG_NOABERR",
    "SEFLG_EQUATORIAL",
    "SEFLG_XYZ",
    "SEFLG_RADIANS",
    "SEFLG_BARYCTR",
    "SEFLG_TOPOCTR",
    "SEFLG_SIDEREAL",
    "SEFLG_ICRS",
    //"SEFLG_DPSIDEPS_1980", don't do that, it is equal to SEFLG_JPLHOR
    "SEFLG_JPLHOR",
    "SEFLG_JPLHOR_APPROX",
    "SEFLG_CENTER_BODY",
    ""
};

const char* HELPFILE = "setest.help";

