/**
 * Astronomical Constants
 *
 * Standard astronomical constants and conversion factors
 * Based on IAU and IERS standards
 *
 * This is a clean-room implementation using publicly available
 * astronomical constants and does not depend on Swiss Ephemeris.
 */

// ============================================================================
// Mathematical Constants
// ============================================================================

export const PI = Math.PI;
export const TWO_PI = 2 * Math.PI;
export const PI_OVER_2 = Math.PI / 2;
export const DEG_TO_RAD = Math.PI / 180;
export const RAD_TO_DEG = 180 / Math.PI;
export const ARCSEC_TO_RAD = Math.PI / 648000;
export const RAD_TO_ARCSEC = 648000 / Math.PI;

// ============================================================================
// Time Constants
// ============================================================================

/** Julian Date of J2000.0 epoch */
export const J2000 = 2451545.0;

/** Julian Date of J1900.0 epoch */
export const J1900 = 2415020.0;

/** Days per Julian century */
export const DAYS_PER_CENTURY = 36525.0;

/** Days per Julian millennium */
export const DAYS_PER_MILLENNIUM = 365250.0;

/** Seconds per day */
export const SECONDS_PER_DAY = 86400.0;

// ============================================================================
// Astronomical Unit Conversions
// ============================================================================

/** Astronomical Unit in kilometers (IAU 2012) */
export const AU_KM = 149597870.700;

/** Astronomical Unit to light years */
export const AU_TO_LIGHT_YEARS = 1.0 / 63241.077;

/** Astronomical Unit to parsecs */
export const AU_TO_PARSEC = 1.0 / 206264.806;

/** Speed of light in AU per day */
export const C_LIGHT_AU_PER_DAY = 173.144632674;

/** Light time for 1 AU in days */
export const LIGHT_TIME_AU = 1.0 / C_LIGHT_AU_PER_DAY;

// ============================================================================
// Earth Constants
// ============================================================================

/** Earth's equatorial radius in km (WGS84) */
export const EARTH_RADIUS_EQUATOR = 6378.137;

/** Earth's polar radius in km (WGS84) */
export const EARTH_RADIUS_POLAR = 6356.752;

/** Earth's flattening (WGS84) */
export const EARTH_FLATTENING = 1.0 / 298.257223563;

/** Earth's rotation rate (radians per day) */
export const EARTH_ROTATION_RATE = 2 * Math.PI * 1.00273790935;

// ============================================================================
// Obliquity Constants (IAU 2006)
// ============================================================================

/** Mean obliquity at J2000.0 in degrees */
export const OBLIQUITY_J2000 = 23.439291;

/** Mean obliquity at J2000.0 in arcseconds */
export const OBLIQUITY_J2000_ARCSEC = 84381.406;

// ============================================================================
// Precession Constants (IAU 2006)
// ============================================================================

/** Precession in longitude (arcsec per century) */
export const PRECESSION_RATE_LON = 5028.796195;

/** Precession in obliquity (arcsec per century) */
export const PRECESSION_RATE_OBL = -46.836769;

// ============================================================================
// Gravitational Constants
// ============================================================================

/** Gaussian gravitational constant */
export const GAUSS_K = 0.01720209895;

/** Solar mass parameter (AU^3/day^2) */
export const GM_SUN = GAUSS_K * GAUSS_K;

// ============================================================================
// Planetary Mass Ratios (1/mass relative to Sun)
// ============================================================================

export const MASS_RATIO = {
  MERCURY: 6023600,
  VENUS: 408523.71,
  EARTH: 328900.56,
  MARS: 3098708,
  JUPITER: 1047.3486,
  SATURN: 3497.898,
  URANUS: 22902.98,
  NEPTUNE: 19412.24,
  PLUTO: 135200000,
};

// ============================================================================
// Synodic Periods (days)
// ============================================================================

export const SYNODIC_PERIOD = {
  MERCURY: 115.88,
  VENUS: 583.92,
  MARS: 779.94,
  JUPITER: 398.88,
  SATURN: 378.09,
  URANUS: 369.66,
  NEPTUNE: 367.49,
};

// ============================================================================
// Mean Orbital Elements at J2000.0
// ============================================================================

/** Simplified mean orbital elements for quick approximations */
export const MEAN_ELEMENTS_J2000 = {
  MERCURY: {
    a: 0.38709927,      // Semi-major axis (AU)
    e: 0.20563593,      // Eccentricity
    i: 7.00497902,      // Inclination (degrees)
    L: 252.25032350,    // Mean longitude (degrees)
    longPeri: 77.45779628,   // Longitude of perihelion (degrees)
    longNode: 48.33076593,   // Longitude of ascending node (degrees)
  },
  VENUS: {
    a: 0.72333566,
    e: 0.00677672,
    i: 3.39467605,
    L: 181.97909950,
    longPeri: 131.60246718,
    longNode: 76.67984255,
  },
  EARTH: {
    a: 1.00000261,
    e: 0.01671123,
    i: -0.00001531,
    L: 100.46457166,
    longPeri: 102.93768193,
    longNode: 0.0,
  },
  MARS: {
    a: 1.52371034,
    e: 0.09339410,
    i: 1.84969142,
    L: -4.55343205,
    longPeri: -23.94362959,
    longNode: 49.55953891,
  },
  JUPITER: {
    a: 5.20288700,
    e: 0.04838624,
    i: 1.30439695,
    L: 34.39644051,
    longPeri: 14.72847983,
    longNode: 100.47390909,
  },
  SATURN: {
    a: 9.53667594,
    e: 0.05386179,
    i: 2.48599187,
    L: 49.95424423,
    longPeri: 92.59887831,
    longNode: 113.66242448,
  },
  URANUS: {
    a: 19.18916464,
    e: 0.04725744,
    i: 0.77263783,
    L: 313.23810451,
    longPeri: 170.95427630,
    longNode: 74.01692503,
  },
  NEPTUNE: {
    a: 30.06992276,
    e: 0.00859048,
    i: 1.77004347,
    L: -55.12002969,
    longPeri: 44.96476227,
    longNode: 131.78422574,
  },
  PLUTO: {
    a: 39.48211675,
    e: 0.24882730,
    i: 17.14001206,
    L: 238.92903833,
    longPeri: 224.06891629,
    longNode: 110.30393684,
  },
};

// ============================================================================
// Rates of Change (per century)
// ============================================================================

export const MEAN_ELEMENTS_RATES = {
  MERCURY: {
    a: 0.00000037,
    e: 0.00001906,
    i: -0.00594749,
    L: 149472.67411175,
    longPeri: 0.16047689,
    longNode: -0.12534081,
  },
  VENUS: {
    a: 0.00000390,
    e: -0.00004107,
    i: -0.00078890,
    L: 58517.81538729,
    longPeri: 0.00268329,
    longNode: -0.27769418,
  },
  EARTH: {
    a: 0.00000562,
    e: -0.00004392,
    i: -0.01294668,
    L: 35999.37244981,
    longPeri: 0.32327364,
    longNode: 0.0,
  },
  MARS: {
    a: 0.00001847,
    e: 0.00007882,
    i: -0.00813131,
    L: 19140.30268499,
    longPeri: 0.44441088,
    longNode: -0.29257343,
  },
  JUPITER: {
    a: -0.00011607,
    e: -0.00013253,
    i: -0.00183714,
    L: 3034.74612775,
    longPeri: 0.21252668,
    longNode: 0.20469106,
  },
  SATURN: {
    a: -0.00125060,
    e: -0.00050991,
    i: 0.00193609,
    L: 1222.49362201,
    longPeri: -0.41897216,
    longNode: -0.28867794,
  },
  URANUS: {
    a: -0.00196176,
    e: -0.00004397,
    i: -0.00242939,
    L: 428.48202785,
    longPeri: 0.40805281,
    longNode: 0.04240589,
  },
  NEPTUNE: {
    a: 0.00026291,
    e: 0.00005105,
    i: 0.00035372,
    L: 218.45945325,
    longPeri: -0.32241464,
    longNode: -0.00508664,
  },
  PLUTO: {
    a: -0.00031596,
    e: 0.00005170,
    i: 0.00004818,
    L: 145.20780515,
    longPeri: -0.04062942,
    longNode: -0.01183482,
  },
};

// ============================================================================
// Moon Constants
// ============================================================================

/** Mean distance Earth-Moon in km */
export const MOON_DISTANCE_MEAN = 384400;

/** Moon's semi-major axis in AU */
export const MOON_SEMI_MAJOR_AXIS = MOON_DISTANCE_MEAN / AU_KM;

/** Moon's orbital period in days */
export const MOON_PERIOD = 27.321661;

/** Moon's synodic period in days */
export const MOON_SYNODIC_PERIOD = 29.530589;

/** Moon's mean longitude at J2000 */
export const MOON_MEAN_LONGITUDE_J2000 = 218.3164477;

/** Moon's mean longitude rate (degrees per day) */
export const MOON_MEAN_LONGITUDE_RATE = 13.176396;

// ============================================================================
// Delta T (TT - UT) Approximation Parameters
// ============================================================================

/** Delta T polynomial coefficients for different epochs */
export const DELTA_T_PARAMS = {
  // Before 1600: parabolic approximation
  ANCIENT: {
    u_factor: 0.01,
    coeffs: [-20, 0, 32],
  },
  // 1600-2000: polynomial fit
  MODERN: {
    coeffs: [
      120, -0.9808, -0.01532, 0.000140,
    ],
  },
  // 2000+: linear extrapolation
  FUTURE: {
    base: 62.92,
    rate: 0.32217,
  },
};

// ============================================================================
// Nutation Constants (IAU 2000B)
// ============================================================================

/** Number of nutation terms in simplified model */
export const NUTATION_TERMS_COUNT = 77;

// ============================================================================
// House System Constants
// ============================================================================

/** Maximum iterations for house cusp calculations */
export const HOUSE_ITERATION_MAX = 50;

/** Convergence tolerance for house calculations */
export const HOUSE_ITERATION_TOLERANCE = 1e-8;

// ============================================================================
// Atmospheric Refraction Constants
// ============================================================================

/** Standard atmospheric pressure in hPa */
export const STANDARD_PRESSURE = 1013.25;

/** Standard temperature in Celsius */
export const STANDARD_TEMPERATURE = 15.0;

/** Refraction constant at horizon */
export const REFRACTION_HORIZON = 34.0 / 60.0; // ~34 arcminutes in degrees

// ============================================================================
// Helper Functions
// ============================================================================

/**
 * Normalize angle to [0, 2π) range
 */
export function normalizeRadians(angle: number): number {
  let result = angle % TWO_PI;
  if (result < 0) result += TWO_PI;
  return result;
}

/**
 * Normalize angle to [0, 360) range
 */
export function normalizeDegrees(angle: number): number {
  let result = angle % 360;
  if (result < 0) result += 360;
  return result;
}

/**
 * Normalize angle to [-180, 180) range
 */
export function normalizeDegreesSymmetric(angle: number): number {
  let result = normalizeDegrees(angle);
  if (result >= 180) result -= 360;
  return result;
}

/**
 * Convert degrees to radians
 */
export function toRadians(degrees: number): number {
  return degrees * DEG_TO_RAD;
}

/**
 * Convert radians to degrees
 */
export function toDegrees(radians: number): number {
  return radians * RAD_TO_DEG;
}

/**
 * Calculate Julian centuries from J2000
 */
export function julianCenturiesFromJ2000(jd: number): number {
  return (jd - J2000) / DAYS_PER_CENTURY;
}

/**
 * Calculate Julian millennia from J2000
 */
export function julianMillenniaFromJ2000(jd: number): number {
  return (jd - J2000) / DAYS_PER_MILLENNIUM;
}
