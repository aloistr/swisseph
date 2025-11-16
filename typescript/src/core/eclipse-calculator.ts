/**
 * Eclipse Calculations
 *
 * Solar and lunar eclipse detection and characteristics
 *
 * Clean-room implementation based on:
 * - Meeus, Astronomical Algorithms, Chapters 54-55
 * - Explanatory Supplement to the Astronomical Almanac
 */

import {
  toRadians,
  toDegrees,
  normalizeDegrees,
  normalizeRadians,
  julianCenturiesFromJ2000,
  MOON_DISTANCE_MEAN,
  AU_KM,
} from './astronomical-constants';

import {
  calculateSunPosition,
} from './planetary-calculator';

import {
  calculateMoonPosition,
  calculateLunarArguments,
} from './moon-calculator';

import { gregorianToJulianDay } from './date-time';

// ============================================================================
// Eclipse Types
// ============================================================================

export enum EclipseType {
  NONE = 0,
  PARTIAL = 1,
  ANNULAR = 2,
  TOTAL = 3,
  ANNULAR_TOTAL = 4,  // Hybrid
  PENUMBRAL = 5,      // Lunar only
}

// ============================================================================
// Eclipse Data Structures
// ============================================================================

export interface SolarEclipse {
  type: EclipseType;
  julianDayMax: number;
  gamma: number;           // Distance of Moon's shadow axis from Earth center (Earth radii)
  magnitude: number;       // Fraction of Sun's diameter covered
  sarosNumber?: number;
  sarosSeriesNumber?: number;
  centralDuration?: number;  // seconds
  pathWidth?: number;        // km
  latitude?: number;         // degrees (maximum eclipse point)
  longitude?: number;        // degrees (maximum eclipse point)
}

export interface LunarEclipse {
  type: EclipseType;
  julianDayMax: number;
  penumbralMagnitude: number;
  umbralMagnitude: number;
  sarosNumber?: number;
  sarosSeriesNumber?: number;
  totalityDuration?: number;  // seconds
  latitude?: number;           // Moon's zenith latitude
  longitude?: number;          // Moon's zenith longitude
}

// ============================================================================
// New Moon / Full Moon Detection
// ============================================================================

/**
 * Find approximate time of next new moon
 *
 * Based on Meeus Chapter 49
 *
 * @param jdStart - Starting Julian Day
 * @returns Julian Day of next new moon
 */
export function findNextNewMoon(jdStart: number): number {
  const { year } = gregorianToJulianDayReverse(jdStart);

  // Approximate using mean lunation (29.53 days)
  const k = Math.floor((year - 2000) * 12.3685);

  return calculateMeanNewMoon(k + 1);
}

/**
 * Find approximate time of next full moon
 *
 * @param jdStart - Starting Julian Day
 * @returns Julian Day of next full moon
 */
export function findNextFullMoon(jdStart: number): number {
  const { year } = gregorianToJulianDayReverse(jdStart);

  const k = Math.floor((year - 2000) * 12.3685) + 0.5;

  return calculateMeanNewMoon(k);
}

/**
 * Calculate mean new moon time
 *
 * Meeus Chapter 49
 *
 * @param k - Lunation number
 * @returns Julian Day of mean new/full moon
 */
function calculateMeanNewMoon(k: number): number {
  const T = k / 1236.85;
  const T2 = T * T;
  const T3 = T2 * T;
  const T4 = T3 * T;

  // Mean time of phase
  const JDE =
    2451550.09766 +
    29.530588861 * k +
    0.00015437 * T2 -
    0.000000150 * T3 +
    0.00000000073 * T4;

  return JDE;
}

/**
 * Reverse Julian Day to date (simplified)
 */
function gregorianToJulianDayReverse(jd: number): { year: number; month: number; day: number } {
  const z = Math.floor(jd + 0.5);
  const f = jd + 0.5 - z;

  let a = z;
  if (z >= 2299161) {
    const alpha = Math.floor((z - 1867216.25) / 36524.25);
    a = z + 1 + alpha - Math.floor(alpha / 4);
  }

  const b = a + 1524;
  const c = Math.floor((b - 122.1) / 365.25);
  const d = Math.floor(365.25 * c);
  const e = Math.floor((b - d) / 30.6001);

  const day = b - d - Math.floor(30.6001 * e);
  const month = e < 14 ? e - 1 : e - 13;
  const year = month > 2 ? c - 4716 : c - 4715;

  return { year, month, day };
}

// ============================================================================
// Solar Eclipse Detection
// ============================================================================

/**
 * Find next solar eclipse
 *
 * @param jdStart - Starting Julian Day
 * @returns Solar eclipse data or null if none found soon
 */
export function findNextSolarEclipse(jdStart: number): SolarEclipse | null {
  // Search through next 12 new moons (~1 year)
  let jd = jdStart;

  for (let i = 0; i < 12; i++) {
    jd = findNextNewMoon(jd);

    // Check if eclipse occurs at this new moon
    const eclipse = checkSolarEclipse(jd);
    if (eclipse) {
      return eclipse;
    }
  }

  return null;
}

/**
 * Check if solar eclipse occurs near given time
 *
 * @param jdApprox - Approximate Julian Day of new moon
 * @returns Solar eclipse data or null
 */
function checkSolarEclipse(jdApprox: number): SolarEclipse | null {
  // Calculate Sun and Moon positions
  const sunPos = calculateSunPosition(jdApprox);
  const moonPos = calculateMoonPosition(jdApprox);

  // Angular separation
  const separation = calculateAngularSeparation(
    sunPos.longitude,
    sunPos.latitude,
    moonPos.longitude,
    moonPos.latitude
  );

  // Eclipse can only occur if separation < ~2 degrees
  if (separation > 2.0) {
    return null;
  }

  // Refine time using iteration
  const jdMax = refineSolarEclipseTime(jdApprox);

  // Calculate eclipse parameters at maximum
  const sunMax = calculateSunPosition(jdMax);
  const moonMax = calculateMoonPosition(jdMax);

  // Calculate gamma (distance of shadow axis from Earth center)
  const gamma = calculateGamma(jdMax, sunMax, moonMax);

  // Eclipse magnitude
  const magnitude = calculateSolarEclipseMagnitude(jdMax, sunMax, moonMax, gamma);

  // Determine eclipse type
  const type = determineSolarEclipseType(magnitude, gamma, moonMax.distance);

  if (type === EclipseType.NONE) {
    return null;
  }

  return {
    type,
    julianDayMax: jdMax,
    gamma,
    magnitude,
  };
}

/**
 * Refine eclipse maximum time
 */
function refineSolarEclipseTime(jdApprox: number): number {
  let jd = jdApprox;

  // Iterate to find minimum angular separation
  for (let i = 0; i < 5; i++) {
    const dt = 0.01; // 0.01 days = ~15 minutes

    const sunMinus = calculateSunPosition(jd - dt);
    const moonMinus = calculateMoonPosition(jd - dt);
    const sepMinus = calculateAngularSeparation(
      sunMinus.longitude,
      sunMinus.latitude,
      moonMinus.longitude,
      moonMinus.latitude
    );

    const sunPlus = calculateSunPosition(jd + dt);
    const moonPlus = calculateMoonPosition(jd + dt);
    const sepPlus = calculateAngularSeparation(
      sunPlus.longitude,
      sunPlus.latitude,
      moonPlus.longitude,
      moonPlus.latitude
    );

    // Estimate time of minimum using parabolic fit
    const derivative = (sepPlus - sepMinus) / (2 * dt);
    const correction = -derivative * dt;

    jd += correction;

    if (Math.abs(correction) < 0.0001) break;
  }

  return jd;
}

/**
 * Calculate gamma (shadow axis distance from Earth center)
 */
function calculateGamma(
  jd: number,
  sun: { longitude: number; latitude: number; distance: number },
  moon: { longitude: number; latitude: number; distance: number }
): number {
  // Simplified calculation
  // Gamma is approximately the Moon's latitude at conjunction
  // scaled by geometric factors

  const moonDistKm = moon.distance * AU_KM;
  const earthRadii = moonDistKm / 6378.137;

  const gamma = Math.sin(toRadians(moon.latitude)) * earthRadii;

  return gamma;
}

/**
 * Calculate solar eclipse magnitude
 */
function calculateSolarEclipseMagnitude(
  jd: number,
  sun: { longitude: number; latitude: number; distance: number },
  moon: { longitude: number; latitude: number; distance: number },
  gamma: number
): number {
  // Angular diameters
  const sunDiameter = 0.5333; // degrees (approximate)
  const moonDiameter = 0.5181 * (MOON_DISTANCE_MEAN / (moon.distance * AU_KM)); // scaled

  // Separation in degrees
  const separation = calculateAngularSeparation(
    sun.longitude,
    sun.latitude,
    moon.longitude,
    moon.latitude
  );

  // Magnitude = (Moon diameter - separation) / (Sun diameter)
  const magnitude = (moonDiameter - separation) / sunDiameter;

  return Math.max(0, magnitude);
}

/**
 * Determine solar eclipse type
 */
function determineSolarEclipseType(
  magnitude: number,
  gamma: number,
  moonDistance: number
): EclipseType {
  if (magnitude <= 0) {
    return EclipseType.NONE;
  }

  // Check if eclipse is central (gamma < 1)
  const isCentral = Math.abs(gamma) < 1.0;

  if (!isCentral && magnitude < 1.0) {
    return EclipseType.PARTIAL;
  }

  // Moon's apparent diameter vs Sun's
  const moonAngularSize = 0.5181 * (MOON_DISTANCE_MEAN / (moonDistance * AU_KM));
  const sunAngularSize = 0.5333;

  if (moonAngularSize > sunAngularSize) {
    return EclipseType.TOTAL;
  } else if (magnitude >= 1.0) {
    return EclipseType.ANNULAR_TOTAL; // Hybrid
  } else {
    return EclipseType.ANNULAR;
  }
}

// ============================================================================
// Lunar Eclipse Detection
// ============================================================================

/**
 * Find next lunar eclipse
 *
 * @param jdStart - Starting Julian Day
 * @returns Lunar eclipse data or null
 */
export function findNextLunarEclipse(jdStart: number): LunarEclipse | null {
  // Search through next 12 full moons (~1 year)
  let jd = jdStart;

  for (let i = 0; i < 12; i++) {
    jd = findNextFullMoon(jd);

    const eclipse = checkLunarEclipse(jd);
    if (eclipse) {
      return eclipse;
    }
  }

  return null;
}

/**
 * Check if lunar eclipse occurs near given time
 */
function checkLunarEclipse(jdApprox: number): LunarEclipse | null {
  const sunPos = calculateSunPosition(jdApprox);
  const moonPos = calculateMoonPosition(jdApprox);

  // Moon should be opposite Sun (full moon)
  const opposition = Math.abs(normalizeDegrees(moonPos.longitude - sunPos.longitude) - 180);

  if (opposition > 2.0) {
    return null;
  }

  // Refine time
  const jdMax = refineLunarEclipseTime(jdApprox);

  // Calculate magnitudes
  const moonMax = calculateMoonPosition(jdMax);

  // Distance to Earth's shadow
  const shadowDistance = Math.abs(moonMax.latitude);

  // Penumbral radius ≈ 1.3°, Umbral radius ≈ 0.7°
  const penumbralMagnitude = (1.3 - shadowDistance) / 0.545;
  const umbralMagnitude = (0.7 - shadowDistance) / 0.545;

  let type: EclipseType;
  if (umbralMagnitude > 1.0) {
    type = EclipseType.TOTAL;
  } else if (umbralMagnitude > 0) {
    type = EclipseType.PARTIAL;
  } else if (penumbralMagnitude > 0) {
    type = EclipseType.PENUMBRAL;
  } else {
    return null;
  }

  return {
    type,
    julianDayMax: jdMax,
    penumbralMagnitude: Math.max(0, penumbralMagnitude),
    umbralMagnitude: Math.max(0, umbralMagnitude),
  };
}

/**
 * Refine lunar eclipse maximum time
 */
function refineLunarEclipseTime(jdApprox: number): number {
  let jd = jdApprox;

  for (let i = 0; i < 5; i++) {
    const dt = 0.01;

    const moonMinus = calculateMoonPosition(jd - dt);
    const moonPlus = calculateMoonPosition(jd + dt);

    // Find minimum latitude (closest to ecliptic)
    const derivative = (Math.abs(moonPlus.latitude) - Math.abs(moonMinus.latitude)) / (2 * dt);
    const correction = -derivative * dt;

    jd += correction;

    if (Math.abs(correction) < 0.0001) break;
  }

  return jd;
}

// ============================================================================
// Helper Functions
// ============================================================================

/**
 * Calculate angular separation between two celestial points
 */
function calculateAngularSeparation(
  lon1: number,
  lat1: number,
  lon2: number,
  lat2: number
): number {
  const lon1Rad = toRadians(lon1);
  const lat1Rad = toRadians(lat1);
  const lon2Rad = toRadians(lon2);
  const lat2Rad = toRadians(lat2);

  const cosD =
    Math.sin(lat1Rad) * Math.sin(lat2Rad) +
    Math.cos(lat1Rad) * Math.cos(lat2Rad) * Math.cos(lon1Rad - lon2Rad);

  return toDegrees(Math.acos(Math.max(-1, Math.min(1, cosD))));
}

/**
 * List all eclipses in a year
 *
 * @param year - Year to search
 * @returns Array of all eclipses (solar and lunar)
 */
export function findEclipsesInYear(
  year: number
): Array<SolarEclipse | LunarEclipse> {
  const eclipses: Array<SolarEclipse | LunarEclipse> = [];

  const jdStart = gregorianToJulianDay(year, 1, 1, 0, 0, 0);
  const jdEnd = gregorianToJulianDay(year + 1, 1, 1, 0, 0, 0);

  let jd = jdStart;

  // Search for solar eclipses (new moons)
  while (jd < jdEnd) {
    jd = findNextNewMoon(jd);
    if (jd > jdEnd) break;

    const eclipse = checkSolarEclipse(jd);
    if (eclipse) {
      eclipses.push(eclipse);
    }
  }

  // Search for lunar eclipses (full moons)
  jd = jdStart;
  while (jd < jdEnd) {
    jd = findNextFullMoon(jd);
    if (jd > jdEnd) break;

    const eclipse = checkLunarEclipse(jd);
    if (eclipse) {
      eclipses.push(eclipse);
    }
  }

  // Sort by date
  return eclipses.sort((a, b) => a.julianDayMax - b.julianDayMax);
}
