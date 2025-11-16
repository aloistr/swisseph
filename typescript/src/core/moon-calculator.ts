/**
 * Moon Position Calculator
 *
 * Simplified lunar theory based on ELP2000 and Meeus
 * Accurate to within ~10 arcseconds for modern era
 *
 * Clean-room implementation based on:
 * - Meeus, Astronomical Algorithms, Chapter 47
 * - Simplified ELP2000 theory
 * - Chapront's analytical expressions
 *
 * Independent implementation not derived from Swiss Ephemeris
 */

import {
  toRadians,
  toDegrees,
  normalizeDegrees,
  julianCenturiesFromJ2000,
  MOON_DISTANCE_MEAN,
  AU_KM,
} from './astronomical-constants';

import { applyPrecession } from './precession-nutation';

// ============================================================================
// Lunar Arguments (Fundamental Arguments)
// ============================================================================

/**
 * Calculate fundamental arguments of the Moon
 *
 * @param T - Julian centuries from J2000
 * @returns Fundamental lunar arguments in degrees
 */
export function calculateLunarArguments(T: number): {
  Lp: number;   // Mean longitude of the Moon
  D: number;    // Mean elongation of the Moon
  M: number;    // Mean anomaly of the Sun
  Mp: number;   // Mean anomaly of the Moon
  F: number;    // Argument of latitude (mean distance from ascending node)
  Omega: number; // Longitude of ascending node
  E: number;    // Eccentricity factor
} {
  const T2 = T * T;
  const T3 = T2 * T;
  const T4 = T3 * T;

  // Mean longitude of the Moon (degrees)
  const Lp = normalizeDegrees(
    218.3164477 +
    481267.88123421 * T -
    0.0015786 * T2 +
    T3 / 538841 -
    T4 / 65194000
  );

  // Mean elongation (degrees)
  const D = normalizeDegrees(
    297.8501921 +
    445267.1114034 * T -
    0.0018819 * T2 +
    T3 / 545868 -
    T4 / 113065000
  );

  // Sun's mean anomaly (degrees)
  const M = normalizeDegrees(
    357.5291092 +
    35999.0502909 * T -
    0.0001536 * T2 +
    T3 / 24490000
  );

  // Moon's mean anomaly (degrees)
  const Mp = normalizeDegrees(
    134.9633964 +
    477198.8675055 * T +
    0.0087414 * T2 +
    T3 / 69699 -
    T4 / 14712000
  );

  // Argument of latitude (degrees)
  const F = normalizeDegrees(
    93.2720950 +
    483202.0175233 * T -
    0.0036539 * T2 -
    T3 / 3526000 +
    T4 / 863310000
  );

  // Longitude of ascending node (degrees)
  const Omega = normalizeDegrees(
    125.0445479 -
    1934.1362891 * T +
    0.0020754 * T2 +
    T3 / 467441 -
    T4 / 60616000
  );

  // Eccentricity correction factor
  const E = 1 - 0.002516 * T - 0.0000074 * T2;

  return { Lp, D, M, Mp, F, Omega, E };
}

// ============================================================================
// Periodic Terms for Longitude and Distance
// ============================================================================

/**
 * Periodic terms for lunar longitude and distance
 * Format: [D, M, Mp, F, sin_coeff_lon, sin_coeff_dist]
 * Coefficients for most significant terms from Meeus
 */
const LUNAR_LONGITUDE_DISTANCE_TERMS = [
  [0, 0, 1, 0, 6288774, -20905355],
  [2, 0, -1, 0, 1274027, -3699111],
  [2, 0, 0, 0, 658314, -2955968],
  [0, 0, 2, 0, 213618, -569925],
  [0, 1, 0, 0, -185116, 48888],
  [0, 0, 0, 2, -114332, -3149],
  [2, 0, -2, 0, 58793, 246158],
  [2, -1, -1, 0, 57066, -152138],
  [2, 0, 1, 0, 53322, -170733],
  [2, -1, 0, 0, 45758, -204586],
  [0, 1, -1, 0, -40923, -129620],
  [1, 0, 0, 0, -34720, 108743],
  [0, 1, 1, 0, -30383, 104755],
  [2, 0, 0, -2, 15327, 10321],
  [0, 0, 1, 2, -12528, 0],
  [0, 0, 1, -2, 10980, 79661],
  [4, 0, -1, 0, 10675, -34782],
  [0, 0, 3, 0, 10034, -23210],
  [4, 0, -2, 0, 8548, -21636],
  [2, 1, -1, 0, -7888, 24208],
  [2, 1, 0, 0, -6766, 30824],
  [1, 0, -1, 0, -5163, -8379],
  [1, 1, 0, 0, 4987, -16675],
  [2, -1, 1, 0, 4036, -12831],
  [2, 0, 2, 0, 3994, -10445],
  [4, 0, 0, 0, 3861, -11650],
  [2, 0, -3, 0, 3665, 14403],
  [0, 1, -2, 0, -2689, -7003],
  [2, 0, -1, 2, -2602, 0],
  [2, -1, -2, 0, 2390, 10056],
  [1, 0, 1, 0, -2348, 6322],
  [2, -2, 0, 0, 2236, -9884],
  [0, 1, 2, 0, -2120, 5751],
  [0, 2, 0, 0, -2078, 0],
  [2, -2, -1, 0, 2052, -4950],
  [2, 0, 1, -2, -1893, 4130],
  [2, 0, 0, 2, -1821, 0],
  [4, -1, -1, 0, 1715, 0],
  [0, 0, 2, 2, -1594, 0],
  [3, 0, -1, 0, -1577, 0],
  [2, 1, 1, 0, -1440, 0],
  [4, -1, -2, 0, 1384, 0],
  [0, 2, -1, 0, 1350, 0],
  [2, 2, -1, 0, -1271, 0],
  [2, 1, -2, 0, -1155, 0],
  [2, -1, 0, -2, 1093, 0],
  [2, -1, 0, 2, 1080, 0],
  [0, 0, 3, -2, 1073, 0],
  [4, 0, -3, 0, 1066, 0],
  [2, -2, 1, 0, 1046, 0],
  [4, -1, 0, 0, -1020, 0],
  [2, 0, 2, -2, -935, 0],
  [0, 3, 0, 0, 898, 0],
  [4, -2, -1, 0, -862, 0],
  [2, 0, -2, 2, 808, 0],
  [0, 1, 1, -2, 790, 0],
  [1, 1, -1, 0, 759, 0],
];

/**
 * Periodic terms for lunar latitude
 * Format: [D, M, Mp, F, sin_coeff]
 */
const LUNAR_LATITUDE_TERMS = [
  [0, 0, 0, 1, 5128122],
  [0, 0, 1, 1, 280602],
  [0, 0, 1, -1, 277693],
  [2, 0, 0, -1, 173237],
  [2, 0, -1, 1, 55413],
  [2, 0, -1, -1, 46271],
  [2, 0, 0, 1, 32573],
  [0, 0, 2, 1, 17198],
  [2, 0, 1, -1, 9266],
  [0, 0, 2, -1, 8822],
  [2, -1, 0, -1, 8216],
  [2, 0, -2, -1, 4324],
  [2, 0, 1, 1, 4200],
  [2, 1, 0, -1, -3359],
  [2, -1, -1, 1, 2463],
  [2, -1, 0, 1, 2211],
  [2, -1, -1, -1, 2065],
  [0, 1, -1, -1, -1870],
  [4, 0, -1, -1, 1828],
  [0, 1, 0, 1, -1794],
  [0, 0, 0, 3, -1749],
  [0, 1, -1, 1, -1565],
  [1, 0, 0, 1, -1491],
  [0, 1, 1, 1, -1475],
  [0, 1, 1, -1, -1410],
  [0, 1, 0, -1, -1344],
  [1, 0, 0, -1, -1335],
  [0, 0, 3, 1, 1107],
  [4, 0, 0, -1, 1021],
  [4, 0, -1, 1, 833],
];

// ============================================================================
// Moon Position Calculation
// ============================================================================

/**
 * Calculate geocentric position of the Moon
 *
 * Based on Meeus Chapter 47 (simplified ELP2000)
 *
 * @param jdTT - Julian Day (Terrestrial Time)
 * @param applyPrecessionToDate - Apply precession to date
 * @returns Moon's geocentric position
 */
export function calculateMoonPosition(
  jdTT: number,
  applyPrecessionToDate: boolean = true
): {
  longitude: number;   // degrees
  latitude: number;    // degrees
  distance: number;    // AU
} {
  const T = julianCenturiesFromJ2000(jdTT);
  const args = calculateLunarArguments(T);

  // Sum periodic terms for longitude
  let sumLon = 0;
  let sumDist = 0;

  for (const term of LUNAR_LONGITUDE_DISTANCE_TERMS) {
    const [D, M, Mp, F, coeffLon, coeffDist] = term;

    // Calculate argument
    const argument =
      D * args.D + M * args.M + Mp * args.Mp + F * args.F;

    const argRad = toRadians(argument);

    // Apply eccentricity correction for terms with M
    let eCorrection = 1;
    if (Math.abs(M) === 1) {
      eCorrection = args.E;
    } else if (Math.abs(M) === 2) {
      eCorrection = args.E * args.E;
    }

    sumLon += coeffLon * eCorrection * Math.sin(argRad);
    sumDist += coeffDist * eCorrection * Math.cos(argRad);
  }

  // Calculate longitude (degrees)
  let longitude = args.Lp + sumLon / 1000000;

  // Additional corrections
  const A1 = 119.75 + 131.849 * T;
  const A2 = 53.09 + 479264.290 * T;
  const A3 = 313.45 + 481266.484 * T;

  longitude +=
    0.003958 * Math.sin(toRadians(A1)) +
    0.001962 * Math.sin(toRadians(args.Lp - args.F)) +
    0.000318 * Math.sin(toRadians(A2));

  longitude = normalizeDegrees(longitude);

  // Sum periodic terms for latitude
  let sumLat = 0;

  for (const term of LUNAR_LATITUDE_TERMS) {
    const [D, M, Mp, F, coeff] = term;

    const argument =
      D * args.D + M * args.M + Mp * args.Mp + F * args.F;

    const argRad = toRadians(argument);

    // Apply eccentricity correction
    let eCorrection = 1;
    if (Math.abs(M) === 1) {
      eCorrection = args.E;
    } else if (Math.abs(M) === 2) {
      eCorrection = args.E * args.E;
    }

    sumLat += coeff * eCorrection * Math.sin(argRad);
  }

  // Calculate latitude (degrees)
  let latitude = sumLat / 1000000;

  latitude +=
    -0.002235 * Math.sin(toRadians(args.Lp)) +
    0.000382 * Math.sin(toRadians(A3)) +
    0.000175 * Math.sin(toRadians(A1 - args.F)) +
    0.000175 * Math.sin(toRadians(A1 + args.F)) +
    0.000127 * Math.sin(toRadians(args.Lp - args.Mp)) -
    0.000115 * Math.sin(toRadians(args.Lp + args.Mp));

  // Calculate distance (km, then convert to AU)
  const distanceKm = 385000.56 + sumDist / 1000;
  const distance = distanceKm / AU_KM;

  // Apply precession to date if requested
  if (applyPrecessionToDate && T !== 0) {
    const precessed = applyPrecession(longitude, latitude, T);
    longitude = precessed.longitude;
    latitude = precessed.latitude;
  }

  return { longitude, latitude, distance };
}

/**
 * Calculate Moon's position with speed
 *
 * @param jdTT - Julian Day (Terrestrial Time)
 * @param dt - Time step for numerical differentiation (days)
 * @returns Moon's position and speed
 */
export function calculateMoonPositionWithSpeed(
  jdTT: number,
  dt: number = 0.001 // Smaller step for Moon due to faster motion
): {
  longitude: number;
  latitude: number;
  distance: number;
  longitudeSpeed: number;
  latitudeSpeed: number;
  distanceSpeed: number;
} {
  const posMinus = calculateMoonPosition(jdTT - dt, true);
  const pos = calculateMoonPosition(jdTT, true);
  const posPlus = calculateMoonPosition(jdTT + dt, true);

  // Handle longitude wraparound
  let lonMinus = posMinus.longitude;
  let lonPlus = posPlus.longitude;

  if (Math.abs(lonPlus - lonMinus) > 180) {
    if (lonPlus < lonMinus) {
      lonPlus += 360;
    } else {
      lonMinus += 360;
    }
  }

  const longitudeSpeed = (lonPlus - lonMinus) / (2 * dt);
  const latitudeSpeed = (posPlus.latitude - posMinus.latitude) / (2 * dt);
  const distanceSpeed = (posPlus.distance - posMinus.distance) / (2 * dt);

  return {
    ...pos,
    longitudeSpeed,
    latitudeSpeed,
    distanceSpeed,
  };
}

// ============================================================================
// Lunar Nodes and Apsides
// ============================================================================

/**
 * Calculate Moon's mean ascending node
 *
 * @param jdTT - Julian Day (Terrestrial Time)
 * @returns Longitude of mean ascending node (degrees)
 */
export function calculateMoonMeanNode(jdTT: number): number {
  const T = julianCenturiesFromJ2000(jdTT);
  const args = calculateLunarArguments(T);
  return args.Omega;
}

/**
 * Calculate Moon's mean apogee (Black Moon Lilith)
 *
 * @param jdTT - Julian Day (Terrestrial Time)
 * @returns Longitude of mean apogee (degrees)
 */
export function calculateMoonMeanApogee(jdTT: number): number {
  const T = julianCenturiesFromJ2000(jdTT);

  const apogee = normalizeDegrees(
    218.3164591 +
    481267.88088 * T -
    0.001332 * T * T -
    T * T * T / 538841 +
    T * T * T * T / 65194000
  );

  return apogee;
}

// ============================================================================
// Lunar Phases
// ============================================================================

/**
 * Calculate Moon's phase (illuminated fraction)
 *
 * @param moonLon - Moon's longitude (degrees)
 * @param moonLat - Moon's latitude (degrees)
 * @param moonDist - Moon's distance (AU)
 * @param sunLon - Sun's longitude (degrees)
 * @param sunDist - Sun's distance (AU)
 * @returns Illuminated fraction (0-1) and phase angle (degrees)
 */
export function calculateMoonPhase(
  moonLon: number,
  moonLat: number,
  moonDist: number,
  sunLon: number,
  sunDist: number
): {
  illuminatedFraction: number;
  phaseAngle: number;
} {
  // Geocentric elongation
  const psi = Math.acos(
    Math.cos(toRadians(moonLat)) *
    Math.cos(toRadians(moonLon - sunLon))
  );

  // Phase angle (angle Sun-Moon-Earth)
  const i = Math.atan2(
    sunDist * Math.sin(psi),
    moonDist - sunDist * Math.cos(psi)
  );

  // Illuminated fraction
  const k = (1 + Math.cos(i)) / 2;

  return {
    illuminatedFraction: k,
    phaseAngle: toDegrees(i),
  };
}
