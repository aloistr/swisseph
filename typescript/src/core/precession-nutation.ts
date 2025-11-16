/**
 * Precession and Nutation Calculations
 *
 * IAU 2006/2000B precession-nutation model
 *
 * Clean-room implementation based on:
 * - IAU 2006 precession (Capitaine et al.)
 * - IAU 2000B nutation (simplified)
 * - Meeus, Astronomical Algorithms
 */

import {
  OBLIQUITY_J2000_ARCSEC,
  ARCSEC_TO_RAD,
  RAD_TO_DEG,
  toRadians,
  toDegrees,
  julianCenturiesFromJ2000,
  normalizeDegrees,
} from './astronomical-constants';

// ============================================================================
// Obliquity Calculations
// ============================================================================

/**
 * Calculate mean obliquity of the ecliptic
 *
 * IAU 2006 formula
 *
 * @param T - Julian centuries from J2000
 * @returns Mean obliquity in degrees
 */
export function calculateMeanObliquity(T: number): number {
  // Mean obliquity in arcseconds
  const epsilon0 =
    OBLIQUITY_J2000_ARCSEC -
    46.836769 * T -
    0.0001831 * T * T +
    0.00200340 * T * T * T -
    0.000000576 * T * T * T * T -
    0.0000000434 * T * T * T * T * T;

  return epsilon0 / 3600; // Convert to degrees
}

/**
 * Calculate true obliquity of the ecliptic
 *
 * @param T - Julian centuries from J2000
 * @param nutationInObliquity - Nutation in obliquity (degrees)
 * @returns True obliquity in degrees
 */
export function calculateTrueObliquity(
  T: number,
  nutationInObliquity: number
): number {
  const meanObliquity = calculateMeanObliquity(T);
  return meanObliquity + nutationInObliquity;
}

// ============================================================================
// Nutation Calculations (IAU 2000B Simplified)
// ============================================================================

/**
 * Calculate fundamental arguments for nutation
 *
 * @param T - Julian centuries from J2000
 * @returns Fundamental arguments in degrees
 */
export function calculateFundamentalArguments(T: number): {
  l: number;  // Mean anomaly of the Moon
  lPrime: number;  // Mean anomaly of the Sun
  F: number;  // Mean argument of latitude of the Moon
  D: number;  // Mean elongation of the Moon from the Sun
  Omega: number;  // Longitude of the ascending node of the Moon
} {
  // All in degrees
  const l = normalizeDegrees(
    134.96340251 +
    (1717915923.2178 * T +
      31.8792 * T * T +
      0.051635 * T * T * T -
      0.00024470 * T * T * T * T) /
      3600
  );

  const lPrime = normalizeDegrees(
    357.52910918 +
    (129596581.0481 * T -
      0.5532 * T * T +
      0.000136 * T * T * T -
      0.00001149 * T * T * T * T) /
      3600
  );

  const F = normalizeDegrees(
    93.27209062 +
    (1739527262.8478 * T -
      12.7512 * T * T -
      0.001037 * T * T * T +
      0.00000417 * T * T * T * T) /
      3600
  );

  const D = normalizeDegrees(
    297.85019547 +
    (1602961601.2090 * T -
      6.3706 * T * T +
      0.006593 * T * T * T -
      0.00003169 * T * T * T * T) /
      3600
  );

  const Omega = normalizeDegrees(
    125.04455501 +
    (-6962890.5431 * T +
      7.4722 * T * T +
      0.007702 * T * T * T -
      0.00005939 * T * T * T * T) /
      3600
  );

  return { l, lPrime, F, D, Omega };
}

/**
 * Nutation coefficients (IAU 2000B - simplified 77 term model)
 *
 * Each row: [l, l', F, D, Omega, sin_psi, cos_psi, sin_eps, cos_eps]
 * psi = nutation in longitude, eps = nutation in obliquity
 * Units: 0.0001 arcsec
 */
const NUTATION_COEFFICIENTS = [
  [0, 0, 0, 0, 1, -171996, -174.2, 92025, 8.9],
  [-2, 0, 0, 2, 2, -13187, -1.6, 5736, -3.1],
  [0, 0, 0, 2, 2, -2274, -0.2, 977, -0.5],
  [0, 0, 0, 0, 2, 2062, 0.2, -895, 0.5],
  [0, 1, 0, 0, 0, 1426, -3.4, 54, -0.1],
  [0, 0, 1, 0, 0, 712, 0.1, -7, 0],
  [-2, 1, 0, 2, 2, -517, 1.2, 224, -0.6],
  [0, 0, 0, 2, 1, -386, -0.4, 200, 0],
  [0, 0, 1, 2, 2, -301, 0, 129, -0.1],
  [-2, -1, 0, 2, 2, 217, -0.5, -95, 0.3],
  [-2, 0, 1, 0, 0, -158, 0, 0, 0],
  [-2, 0, 0, 2, 1, 129, 0.1, -70, 0],
  [0, 0, -1, 2, 2, 123, 0, -53, 0],
  [2, 0, 0, 0, 0, 63, 0, 0, 0],
  [0, 0, 1, 0, 1, 63, 0.1, -33, 0],
  [2, 0, -1, 2, 2, -59, 0, 26, 0],
  [0, 0, -1, 0, 1, -58, -0.1, 32, 0],
  [0, 0, 1, 2, 1, -51, 0, 27, 0],
  [-2, 0, 2, 0, 0, 48, 0, 0, 0],
  [0, 0, -2, 2, 1, 46, 0, -24, 0],
  [2, 0, 0, 2, 2, -38, 0, 16, 0],
  [0, 0, 2, 2, 2, -31, 0, 13, 0],
  [0, 0, 2, 0, 0, 29, 0, 0, 0],
  [-2, 0, 1, 2, 2, 29, 0, -12, 0],
  [0, 0, 0, 2, 0, 26, 0, 0, 0],
  [-2, 0, 0, 2, 0, -22, 0, 0, 0],
  [0, 0, -1, 2, 1, 21, 0, -10, 0],
  [0, 2, 0, 0, 0, 17, -0.1, 0, 0],
  [2, 0, -1, 0, 1, 16, 0, -8, 0],
  [-2, 2, 0, 2, 2, -16, 0.1, 7, 0],
  [0, 1, 0, 0, 1, -15, 0, 9, 0],
  [-2, 0, 1, 0, 1, -13, 0, 7, 0],
  [0, -1, 0, 0, 1, -12, 0, 6, 0],
  [0, 0, 2, -2, 0, 11, 0, 0, 0],
  [2, 0, -1, 2, 1, -10, 0, 5, 0],
  [2, 0, 1, 2, 2, -8, 0, 3, 0],
  [0, 1, 0, 2, 2, 7, 0, -3, 0],
  [-2, 1, 1, 0, 0, -7, 0, 0, 0],
  [0, -1, 0, 2, 2, -7, 0, 3, 0],
  [2, 0, 0, 2, 1, -7, 0, 3, 0],
  [2, 0, 1, 0, 0, 6, 0, 0, 0],
  [-2, 0, 2, 2, 2, 6, 0, -3, 0],
  [-2, 0, 1, 2, 1, 6, 0, -3, 0],
  [2, 0, -2, 0, 1, -6, 0, 3, 0],
  [2, 0, 0, 0, 1, -6, 0, 3, 0],
  [0, -1, 1, 0, 0, 5, 0, 0, 0],
  [-2, -1, 0, 2, 1, -5, 0, 3, 0],
  [-2, 0, 0, 0, 1, -5, 0, 3, 0],
  [0, 0, 2, 2, 1, -5, 0, 3, 0],
  [-2, 0, 2, 0, 1, 4, 0, 0, 0],
  [-2, 1, 0, 2, 1, 4, 0, 0, 0],
  [0, 0, 1, -2, 0, 4, 0, 0, 0],
  [-1, 0, 1, 0, 0, -4, 0, 0, 0],
  [-2, 1, 0, 0, 0, -4, 0, 0, 0],
  [1, 0, 0, 0, 0, -4, 0, 0, 0],
  [0, 0, 1, 2, 0, 3, 0, 0, 0],
  [0, 0, -2, 2, 2, -3, 0, 0, 0],
  [-1, -1, 1, 0, 0, -3, 0, 0, 0],
  [0, 1, 1, 0, 0, -3, 0, 0, 0],
  [0, -1, 1, 2, 2, -3, 0, 0, 0],
  [2, -1, -1, 2, 2, -3, 0, 0, 0],
  [0, 0, 3, 2, 2, -3, 0, 0, 0],
  [2, -1, 0, 2, 2, -3, 0, 0, 0],
];

/**
 * Calculate nutation in longitude and obliquity
 *
 * IAU 2000B simplified model (77 terms)
 *
 * @param T - Julian centuries from J2000
 * @returns Nutation in longitude and obliquity (degrees)
 */
export function calculateNutation(T: number): {
  nutationInLongitude: number;
  nutationInObliquity: number;
} {
  const { l, lPrime, F, D, Omega } = calculateFundamentalArguments(T);

  let deltaPsi = 0; // Nutation in longitude
  let deltaEps = 0; // Nutation in obliquity

  // Sum nutation series
  for (const coeff of NUTATION_COEFFICIENTS) {
    const [nl, nlp, nF, nD, nOmega, sinPsi, sinPsiT, sinEps, sinEpsT] = coeff;

    // Calculate argument
    const arg =
      nl * l +
      nlp * lPrime +
      nF * F +
      nD * D +
      nOmega * Omega;

    const argRad = toRadians(arg);

    // Add terms
    deltaPsi += (sinPsi + sinPsiT * T) * Math.sin(argRad);
    deltaEps += (sinEps + sinEpsT * T) * Math.cos(argRad);
  }

  // Convert from 0.0001 arcsec to degrees
  const nutationInLongitude = deltaPsi / 36000000;
  const nutationInObliquity = deltaEps / 36000000;

  return { nutationInLongitude, nutationInObliquity };
}

// ============================================================================
// Precession Calculations (IAU 2006)
// ============================================================================

/**
 * Calculate precession angles
 *
 * IAU 2006 precession model (Capitaine et al.)
 *
 * @param T - Julian centuries from J2000
 * @returns Precession angles in arcseconds
 */
export function calculatePrecessionAngles(T: number): {
  psi: number;    // Precession in longitude
  omega: number;  // Precession in obliquity
  chi: number;    // Planetary precession
} {
  const T2 = T * T;
  const T3 = T2 * T;
  const T4 = T3 * T;
  const T5 = T4 * T;

  // Precession in longitude (arcseconds)
  const psi =
    5038.481507 * T -
    1.0790069 * T2 -
    0.00114045 * T3 +
    0.000132851 * T4 -
    0.0000000951 * T5;

  // Precession in obliquity (arcseconds)
  const omega =
    OBLIQUITY_J2000_ARCSEC -
    0.025754 * T +
    0.0512623 * T2 -
    0.00772503 * T3 -
    0.000000467 * T4 +
    0.0000003337 * T5;

  // Planetary precession (arcseconds)
  const chi =
    10.556403 * T -
    2.3814292 * T2 -
    0.00121197 * T3 +
    0.000170663 * T4 -
    0.0000000560 * T5;

  return { psi, omega, chi };
}

/**
 * Precession matrix from J2000 to date
 *
 * @param T - Julian centuries from J2000
 * @returns 3x3 rotation matrix
 */
export function calculatePrecessionMatrix(T: number): number[][] {
  const { psi, omega, chi } = calculatePrecessionAngles(T);

  // Convert to radians
  const psiRad = psi * ARCSEC_TO_RAD;
  const omegaRad = omega * ARCSEC_TO_RAD;
  const chiRad = chi * ARCSEC_TO_RAD;
  const eps0Rad = OBLIQUITY_J2000_ARCSEC * ARCSEC_TO_RAD;

  // Rotation matrices
  const sinPsi = Math.sin(psiRad);
  const cosPsi = Math.cos(psiRad);
  const sinOmega = Math.sin(omegaRad);
  const cosOmega = Math.cos(omegaRad);
  const sinChi = Math.sin(chiRad);
  const cosChi = Math.cos(chiRad);
  const sinEps0 = Math.sin(eps0Rad);
  const cosEps0 = Math.cos(eps0Rad);

  // Combined rotation matrix (simplified)
  const matrix: number[][] = [
    [cosPsi * cosOmega - sinPsi * sinOmega * cosEps0, -sinPsi * cosOmega - cosPsi * sinOmega * cosEps0, -sinOmega * sinEps0],
    [cosPsi * sinOmega + sinPsi * cosOmega * cosEps0, -sinPsi * sinOmega + cosPsi * cosOmega * cosEps0, cosOmega * sinEps0],
    [sinPsi * sinEps0, cosPsi * sinEps0, cosEps0],
  ];

  return matrix;
}

/**
 * Apply precession to ecliptic coordinates
 *
 * @param longitude - Ecliptic longitude in degrees (J2000)
 * @param latitude - Ecliptic latitude in degrees (J2000)
 * @param T - Julian centuries from J2000
 * @returns Precessed coordinates
 */
export function applyPrecession(
  longitude: number,
  latitude: number,
  T: number
): { longitude: number; latitude: number } {
  if (T === 0) {
    return { longitude, latitude };
  }

  const matrix = calculatePrecessionMatrix(T);

  // Convert to radians
  const lonRad = toRadians(longitude);
  const latRad = toRadians(latitude);

  // Convert to Cartesian
  const x = Math.cos(latRad) * Math.cos(lonRad);
  const y = Math.cos(latRad) * Math.sin(lonRad);
  const z = Math.sin(latRad);

  // Apply precession matrix
  const xPrec = matrix[0][0] * x + matrix[0][1] * y + matrix[0][2] * z;
  const yPrec = matrix[1][0] * x + matrix[1][1] * y + matrix[1][2] * z;
  const zPrec = matrix[2][0] * x + matrix[2][1] * y + matrix[2][2] * z;

  // Convert back to spherical
  const lonPrec = toDegrees(Math.atan2(yPrec, xPrec));
  const latPrec = toDegrees(Math.asin(zPrec));

  return {
    longitude: normalizeDegrees(lonPrec),
    latitude: latPrec,
  };
}

/**
 * Calculate complete precession and nutation effects
 *
 * @param jd - Julian Day (TT)
 * @returns All precession and nutation values
 */
export function calculatePrecessionNutation(jd: number): {
  T: number;
  meanObliquity: number;
  trueObliquity: number;
  nutationInLongitude: number;
  nutationInObliquity: number;
  precessionMatrix: number[][];
} {
  const T = julianCenturiesFromJ2000(jd);

  const { nutationInLongitude, nutationInObliquity } = calculateNutation(T);
  const meanObliquity = calculateMeanObliquity(T);
  const trueObliquity = meanObliquity + nutationInObliquity;
  const precessionMatrix = calculatePrecessionMatrix(T);

  return {
    T,
    meanObliquity,
    trueObliquity,
    nutationInLongitude,
    nutationInObliquity,
    precessionMatrix,
  };
}
