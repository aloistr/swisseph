/**
 * Planetary Position Calculator
 *
 * Simplified VSOP87 theory for planetary positions
 * Accurate to within a few arcminutes for modern era
 *
 * Clean-room implementation based on:
 * - Meeus, Astronomical Algorithms
 * - Simon et al. (1994) analytical expressions
 * - Public domain VSOP87 theory
 *
 * This is an independent implementation not derived from Swiss Ephemeris
 */

import {
  MEAN_ELEMENTS_J2000,
  MEAN_ELEMENTS_RATES,
  toRadians,
  toDegrees,
  normalizeDegrees,
  normalizeRadians,
  julianCenturiesFromJ2000,
  AU_KM,
} from './astronomical-constants';

import { applyPrecession } from './precession-nutation';

// ============================================================================
// Planet Identifiers
// ============================================================================

export enum PlanetID {
  MERCURY = 0,
  VENUS = 1,
  EARTH = 2,
  MARS = 3,
  JUPITER = 4,
  SATURN = 5,
  URANUS = 6,
  NEPTUNE = 7,
  PLUTO = 8,
}

// ============================================================================
// Orbital Element Calculation
// ============================================================================

/**
 * Calculate mean orbital elements for a planet
 *
 * @param planet - Planet identifier
 * @param T - Julian centuries from J2000
 * @returns Orbital elements
 */
export function calculateMeanElements(planet: PlanetID, T: number): {
  a: number;          // Semi-major axis (AU)
  e: number;          // Eccentricity
  i: number;          // Inclination (degrees)
  L: number;          // Mean longitude (degrees)
  longPeri: number;   // Longitude of perihelion (degrees)
  longNode: number;   // Longitude of ascending node (degrees)
  M: number;          // Mean anomaly (degrees)
} {
  const planetName = PlanetID[planet] as keyof typeof MEAN_ELEMENTS_J2000;
  const elem0 = MEAN_ELEMENTS_J2000[planetName];
  const elemRates = MEAN_ELEMENTS_RATES[planetName];

  const a = elem0.a + elemRates.a * T;
  const e = elem0.e + elemRates.e * T;
  const i = elem0.i + elemRates.i * T;
  const L = normalizeDegrees(elem0.L + elemRates.L * T);
  const longPeri = normalizeDegrees(elem0.longPeri + elemRates.longPeri * T);
  const longNode = normalizeDegrees(elem0.longNode + elemRates.longNode * T);

  // Mean anomaly = mean longitude - longitude of perihelion
  const M = normalizeDegrees(L - longPeri);

  return { a, e, i, L, longPeri, longNode, M };
}

// ============================================================================
// Kepler's Equation Solver
// ============================================================================

/**
 * Solve Kepler's equation: E - e * sin(E) = M
 *
 * Uses Newton-Raphson iteration
 *
 * @param M - Mean anomaly (radians)
 * @param e - Eccentricity
 * @returns Eccentric anomaly (radians)
 */
export function solveKeplersEquation(M: number, e: number): number {
  const tolerance = 1e-10;
  const maxIterations = 20;

  // Initial guess
  let E = M + e * Math.sin(M);

  for (let i = 0; i < maxIterations; i++) {
    const dE = (M - (E - e * Math.sin(E))) / (1 - e * Math.cos(E));
    E += dE;

    if (Math.abs(dE) < tolerance) {
      break;
    }
  }

  return E;
}

/**
 * Calculate true anomaly from eccentric anomaly
 *
 * @param E - Eccentric anomaly (radians)
 * @param e - Eccentricity
 * @returns True anomaly (radians)
 */
export function calculateTrueAnomaly(E: number, e: number): number {
  const sinNu =
    (Math.sqrt(1 - e * e) * Math.sin(E)) / (1 - e * Math.cos(E));
  const cosNu = (Math.cos(E) - e) / (1 - e * Math.cos(E));
  return Math.atan2(sinNu, cosNu);
}

// ============================================================================
// Heliocentric Position Calculation
// ============================================================================

/**
 * Calculate heliocentric position from orbital elements
 *
 * @param elements - Orbital elements
 * @returns Heliocentric ecliptic coordinates (J2000)
 */
export function calculateHeliocentricPosition(elements: {
  a: number;
  e: number;
  i: number;
  longPeri: number;
  longNode: number;
  M: number;
}): {
  longitude: number;  // degrees
  latitude: number;   // degrees
  distance: number;   // AU
} {
  const { a, e, i, longPeri, longNode, M } = elements;

  // Solve Kepler's equation
  const E = solveKeplersEquation(toRadians(M), e);

  // True anomaly
  const nu = calculateTrueAnomaly(E, e);

  // Radius vector
  const r = a * (1 - e * Math.cos(E));

  // Heliocentric longitude in orbital plane
  const u = toDegrees(nu) + (longPeri - longNode);

  // Convert to ecliptic coordinates
  const uRad = toRadians(u);
  const iRad = toRadians(i);
  const omegaRad = toRadians(longNode);

  // Position in ecliptic coordinates
  const x = r * (Math.cos(omegaRad) * Math.cos(uRad) - Math.sin(omegaRad) * Math.sin(uRad) * Math.cos(iRad));
  const y = r * (Math.sin(omegaRad) * Math.cos(uRad) + Math.cos(omegaRad) * Math.sin(uRad) * Math.cos(iRad));
  const z = r * Math.sin(uRad) * Math.sin(iRad);

  // Convert to spherical
  const longitude = normalizeDegrees(toDegrees(Math.atan2(y, x)));
  const latitude = toDegrees(Math.asin(z / r));
  const distance = r;

  return { longitude, latitude, distance };
}

// ============================================================================
// Perturbation Corrections
// ============================================================================

/**
 * Apply simplified perturbations for improved accuracy
 *
 * These are highly simplified - full VSOP87 has thousands of terms
 *
 * @param planet - Planet ID
 * @param T - Julian centuries from J2000
 * @param position - Unperturbed position
 * @returns Corrected position
 */
export function applyPerturbations(
  planet: PlanetID,
  T: number,
  position: { longitude: number; latitude: number; distance: number }
): { longitude: number; latitude: number; distance: number } {
  // Apply Jupiter-Saturn mutual perturbations (most significant)
  if (planet === PlanetID.JUPITER || planet === PlanetID.SATURN) {
    const jupElements = calculateMeanElements(PlanetID.JUPITER, T);
    const satElements = calculateMeanElements(PlanetID.SATURN, T);

    // Mean motions
    const nJup = 0.0830853 * T; // degrees
    const nSat = 0.0334307 * T; // degrees

    // Great inequality terms (simplified)
    const longCorr =
      0.331 * Math.sin(toRadians(2 * jupElements.M - 5 * satElements.M - 48.0)) +
      0.003 * Math.sin(toRadians(2 * jupElements.M - 6 * satElements.M + 224.0));

    position.longitude += longCorr;
  }

  return position;
}

// ============================================================================
// Geocentric Position Calculation
// ============================================================================

/**
 * Calculate geocentric position of a planet
 *
 * @param planet - Planet ID
 * @param jdTT - Julian Day (Terrestrial Time)
 * @param applyPrecessionToDate - Apply precession to date (default: true)
 * @returns Geocentric ecliptic coordinates
 */
export function calculatePlanetPosition(
  planet: PlanetID,
  jdTT: number,
  applyPrecessionToDate: boolean = true
): {
  longitude: number;        // degrees
  latitude: number;         // degrees
  distance: number;         // AU
  longitudeSpeed?: number;  // degrees/day
  latitudeSpeed?: number;   // degrees/day
  distanceSpeed?: number;   // AU/day
} {
  const T = julianCenturiesFromJ2000(jdTT);

  // Calculate planet's heliocentric position
  const planetElements = calculateMeanElements(planet, T);
  let planetPos = calculateHeliocentricPosition(planetElements);
  planetPos = applyPerturbations(planet, T, planetPos);

  // Calculate Earth's heliocentric position
  const earthElements = calculateMeanElements(PlanetID.EARTH, T);
  const earthPos = calculateHeliocentricPosition(earthElements);

  // Convert heliocentric to geocentric (vector subtraction)
  const geocentricPos = heliocentricToGeocentric(planetPos, earthPos);

  // Apply precession to date if requested
  if (applyPrecessionToDate && T !== 0) {
    const precessed = applyPrecession(
      geocentricPos.longitude,
      geocentricPos.latitude,
      T
    );
    geocentricPos.longitude = precessed.longitude;
    geocentricPos.latitude = precessed.latitude;
  }

  return geocentricPos;
}

/**
 * Convert heliocentric to geocentric coordinates
 */
function heliocentricToGeocentric(
  planet: { longitude: number; latitude: number; distance: number },
  earth: { longitude: number; latitude: number; distance: number }
): { longitude: number; latitude: number; distance: number } {
  // Convert to Cartesian
  const lonPRad = toRadians(planet.longitude);
  const latPRad = toRadians(planet.latitude);
  const lonERad = toRadians(earth.longitude);
  const latERad = toRadians(earth.latitude);

  const xp = planet.distance * Math.cos(latPRad) * Math.cos(lonPRad);
  const yp = planet.distance * Math.cos(latPRad) * Math.sin(lonPRad);
  const zp = planet.distance * Math.sin(latPRad);

  const xe = earth.distance * Math.cos(latERad) * Math.cos(lonERad);
  const ye = earth.distance * Math.cos(latERad) * Math.sin(lonERad);
  const ze = earth.distance * Math.sin(latERad);

  // Geocentric position
  const x = xp - xe;
  const y = yp - ye;
  const z = zp - ze;

  // Convert back to spherical
  const distance = Math.sqrt(x * x + y * y + z * z);
  const longitude = normalizeDegrees(toDegrees(Math.atan2(y, x)));
  const latitude = toDegrees(Math.asin(z / distance));

  return { longitude, latitude, distance };
}

// ============================================================================
// Sun Position (Earth from Sun's perspective)
// ============================================================================

/**
 * Calculate geocentric position of the Sun
 *
 * @param jdTT - Julian Day (Terrestrial Time)
 * @param applyPrecessionToDate - Apply precession to date
 * @returns Sun's geocentric position
 */
export function calculateSunPosition(
  jdTT: number,
  applyPrecessionToDate: boolean = true
): {
  longitude: number;
  latitude: number;
  distance: number;
} {
  const T = julianCenturiesFromJ2000(jdTT);

  // Earth's heliocentric position
  const earthElements = calculateMeanElements(PlanetID.EARTH, T);
  const earthPos = calculateHeliocentricPosition(earthElements);

  // Sun's geocentric position is opposite of Earth's heliocentric position
  let sunLongitude = normalizeDegrees(earthPos.longitude + 180);
  let sunLatitude = -earthPos.latitude;
  const sunDistance = earthPos.distance;

  // Apply precession to date if requested
  if (applyPrecessionToDate && T !== 0) {
    const precessed = applyPrecession(sunLongitude, sunLatitude, T);
    sunLongitude = precessed.longitude;
    sunLatitude = precessed.latitude;
  }

  return {
    longitude: sunLongitude,
    latitude: sunLatitude,
    distance: sunDistance,
  };
}

// ============================================================================
// Speed Calculations
// ============================================================================

/**
 * Calculate planetary speed using numerical differentiation
 *
 * @param planet - Planet ID
 * @param jdTT - Julian Day (Terrestrial Time)
 * @param dt - Time step in days (default: 0.01 days)
 * @returns Position with speed
 */
export function calculatePlanetPositionWithSpeed(
  planet: PlanetID,
  jdTT: number,
  dt: number = 0.01
): {
  longitude: number;
  latitude: number;
  distance: number;
  longitudeSpeed: number;
  latitudeSpeed: number;
  distanceSpeed: number;
} {
  // Calculate positions at t-dt, t, and t+dt
  const posMinus = calculatePlanetPosition(planet, jdTT - dt, true);
  const pos = calculatePlanetPosition(planet, jdTT, true);
  const posPlus = calculatePlanetPosition(planet, jdTT + dt, true);

  // Central difference for speed
  const longitudeSpeed = (posPlus.longitude - posMinus.longitude) / (2 * dt);
  const latitudeSpeed = (posPlus.latitude - posMinus.latitude) / (2 * dt);
  const distanceSpeed = (posPlus.distance - posMinus.distance) / (2 * dt);

  return {
    ...pos,
    longitudeSpeed,
    latitudeSpeed,
    distanceSpeed,
  };
}

/**
 * Calculate Sun's speed
 */
export function calculateSunPositionWithSpeed(
  jdTT: number,
  dt: number = 0.01
): {
  longitude: number;
  latitude: number;
  distance: number;
  longitudeSpeed: number;
  latitudeSpeed: number;
  distanceSpeed: number;
} {
  const posMinus = calculateSunPosition(jdTT - dt, true);
  const pos = calculateSunPosition(jdTT, true);
  const posPlus = calculateSunPosition(jdTT + dt, true);

  const longitudeSpeed = (posPlus.longitude - posMinus.longitude) / (2 * dt);
  const latitudeSpeed = (posPlus.latitude - posMinus.latitude) / (2 * dt);
  const distanceSpeed = (posPlus.distance - posMinus.distance) / (2 * dt);

  return {
    ...pos,
    longitudeSpeed,
    latitudeSpeed,
    distanceSpeed,
  };
}
