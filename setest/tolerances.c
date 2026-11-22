#include "setest.h"
#include <math.h>

// Tolerances per kind of quantity, in the units of the compared values.
// Decided 2026-11-22 (Alois, setest maintainer); see setest.md.
// Numerical noise between builds with gcc -g, gcc -O2 and clang -O2 was below 1e-13 (x86_64).
#define TOL_ANGLE_DEG  3e-8                   // 0.1 mas; compressed files: 1 mas (swisseph.md)
#define TOL_ANGLE_RAD  (TOL_ANGLE_DEG * M_PI / 180.0)
#define TOL_SEARCH_DEG (1.0 / 3600000.0)      // 1 mas, documented precision of the crossing functions
#define TOL_DIST_AU    1e-10                  // 15 m
#define TOL_KM         0.015                  // 15 m
#define TOL_SPEED      1e-9                   // per day (degrees, radians or AU), as before
#define TOL_TIME_DAY   1e-8                   // about 1 ms; the Moon moves about 0.5 mas in 1 ms
#define TOL_TIME_HOUR  (TOL_TIME_DAY * 24.0)
#define TOL_TIME_YEAR  (TOL_TIME_DAY / 365.25)
#define TOL_DIMLESS    1e-7                   // magnitudes, fractions, eccentricity, saros numbers
#define TOL_HOUSEPOS   1e-9                   // house or sector position; 0.1 mas for a house of 30 degrees
#define TOL_RELATIVE   1e-14                  // lower limit relative to the expected value, for large values

typedef enum {
  Q_UNKNOWN,
  Q_ANGLE,        // degrees, or radians with SEFLG_RADIANS
  Q_ANGLE_DEG,    // always degrees
  Q_SEARCH,       // self-check of a search function against its target position, degrees
  Q_DIST,         // AU
  Q_KM,
  Q_SPEED,        // per day
  Q_TIME_DAY,     // Julian day or days
  Q_TIME_HOUR,
  Q_TIME_YEAR,
  Q_DIMLESS,
  Q_HOUSEPOS
} quantity;

// Split "xxattr[3]" into base "xxattr" and index 3; index -1 if there is none
static int split_name(const char *name, char *base, int maxlen) {
  int n = 0, idx = -1;
  while (name[n] != '\0' && name[n] != '[' && n < maxlen - 1) {
    base[n] = name[n];
    n++;
  }
  base[n] = '\0';
  if (name[n] == '[') idx = atoi(name + n + 1);
  return idx;
}

static bool is(const char *base, const char *s) {
  return strcmp(base, s) == 0;
}

// Position vector as returned by swe_calc(): longitude, latitude, distance and their speeds,
// or x, y, z and their speeds with SEFLG_XYZ
static quantity kind_position(int idx, int iflag) {
  if (iflag & SEFLG_XYZ) return (idx <= 2) ? Q_DIST : Q_SPEED;
  if (idx <= 1) return Q_ANGLE;
  if (idx == 2) return Q_DIST;
  return Q_SPEED;
}

// swe_get_orbital_elements(), dret[0..16] (swephprg.md)
static quantity kind_orbital_element(int idx) {
  if (idx == 0 || idx == 15 || idx == 16) return Q_DIST;  // semimajor axis, perihelion and aphelion distance
  if (idx == 1) return Q_DIMLESS;                         // eccentricity
  if (idx >= 2 && idx <= 9) return Q_ANGLE_DEG;           // inclination, node, perihelion, anomalies, mean longitude
  if (idx == 10 || idx == 12) return Q_TIME_YEAR;         // sidereal and tropical period
  if (idx == 11) return Q_SPEED;                          // mean daily motion
  if (idx == 13 || idx == 14) return Q_TIME_DAY;          // synodic period, time of perihelion passage
  return Q_UNKNOWN;
}

// Eclipse and occultation attributes attr[] (swephprg.md); suite 8,
// test cases 9 and 10 are lunar eclipses, where attr[3] is not used
static quantity kind_eclipse_attr(int tcase, int idx) {
  if (idx >= 4 && idx <= 7) return Q_ANGLE_DEG;           // azimuth, true and apparent altitude, elongation
  if (idx == 3 && tcase != 9 && tcase != 10) return Q_KM; // diameter of core shadow
  return Q_DIMLESS;                                       // fractions, magnitudes, saros numbers
}

// swe_pheno(), attr[0..4] (swephprg.md): phase angle, phase, elongation, apparent diameter, magnitude
static quantity kind_pheno(int idx) {
  if (idx == 0 || idx == 2 || idx == 3) return Q_ANGLE_DEG;
  return Q_DIMLESS;
}

static quantity kind_of(int suite, int tcase, const char *base, int idx, int iflag) {
  if (suite == 10 && (is(base, "xcross") || is(base, "xx"))) return Q_SEARCH;
  if (is(base, "xx")) return (suite == 6) ? Q_ANGLE_DEG : kind_position(idx, iflag);
  if (is(base, "xxaphe") || is(base, "xxperi") || is(base, "xxnasc") || is(base, "xxndsc")) return kind_position(idx, iflag);
  if (is(base, "xxdret")) return kind_orbital_element(idx);
  if (is(base, "dmax") || is(base, "dmin") || is(base, "dtrue")) return Q_DIST;
  if (is(base, "xxtret")) {
    // suite 9: heliacal_pheno_ut() returns altitudes, vis_limit_mag() a magnitude, altitude and azimuth
    if (suite == 9 && tcase == 4) return Q_ANGLE_DEG;
    if (suite == 9 && tcase == 5) return (idx == 0) ? Q_DIMLESS : Q_ANGLE_DEG;
    return Q_TIME_DAY;
  }
  if (is(base, "tret")) return Q_TIME_DAY;
  if (is(base, "xxgeopos")) return Q_ANGLE_DEG;
  if (is(base, "xxattr")) return kind_eclipse_attr(tcase, idx);
  if (is(base, "mag")) return Q_DIMLESS;
  if (is(base, "daya") || is(base, "daya_ut") || is(base, "daya_ex") || is(base, "daya_ex_ut")) return Q_ANGLE_DEG;
  if (is(base, "jd") || is(base, "jd_ut") || is(base, "deltat") || is(base, "E")) return Q_TIME_DAY;
  if (is(base, "tjd_lat") || is(base, "tjd_lmt") || is(base, "jx")) return Q_TIME_DAY;
  if (is(base, "ut")) return Q_TIME_HOUR;  // swe_revjul() returns hours
  if (is(base, "cusps") || is(base, "ascmc")) return Q_ANGLE;
  if (is(base, "cusp_speed") || is(base, "ascmc_speed")) return Q_SPEED;
  if (is(base, "armc") || is(base, "xlon") || is(base, "xlat")) return Q_ANGLE_DEG;
  if (is(base, "hp") || is(base, "gp")) return Q_HOUSEPOS;
  // suite 11 (2026-11-22)
  if (is(base, "xpheno")) return kind_pheno(idx);
  if (is(base, "xaz") || is(base, "alt") || is(base, "xrefr")) return Q_ANGLE_DEG;
  if (is(base, "xpn") || is(base, "xback")) {
    // swe_cotrans(_sp)(): longitude, latitude, distance and their speeds, in degrees;
    // swe_azalt_rev(): longitude and latitude or right ascension and declination
    if (idx <= 1) return Q_ANGLE_DEG;
    if (idx == 2) return Q_DIST;
    return Q_SPEED;
  }
  return Q_UNKNOWN;
}

// Tolerance for a compared value, or -1 if its kind of quantity is unknown
double tolerance_for(const char *name, const double exp, test_context *ctx) {
  char base[SETEST_MAX_SYMBOL_SIZE + 6];
  int idx = split_name(name, base, sizeof(base));
  entry e = get_entry("iflag", I, ctx);
  int iflag = e.not_found ? 0 : e.pair.tvalue.value.i;
  double tol;
  switch (kind_of(ctx->current.suite.id, ctx->current.testcase.id, base, idx, iflag)) {
    case Q_ANGLE:     tol = (iflag & SEFLG_RADIANS) ? TOL_ANGLE_RAD : TOL_ANGLE_DEG; break;
    case Q_ANGLE_DEG: tol = TOL_ANGLE_DEG; break;
    case Q_SEARCH:    tol = TOL_SEARCH_DEG; break;
    case Q_DIST:      tol = TOL_DIST_AU; break;
    case Q_KM:        tol = TOL_KM; break;
    case Q_SPEED:     tol = TOL_SPEED; break;
    case Q_TIME_DAY:  tol = TOL_TIME_DAY; break;
    case Q_TIME_HOUR: tol = TOL_TIME_HOUR; break;
    case Q_TIME_YEAR: tol = TOL_TIME_YEAR; break;
    case Q_DIMLESS:   tol = TOL_DIMLESS; break;
    case Q_HOUSEPOS:  tol = TOL_HOUSEPOS; break;
    default:          return -1;
  }
  if (TOL_RELATIVE * fabs(exp) > tol) tol = TOL_RELATIVE * fabs(exp);
  return tol;
}
