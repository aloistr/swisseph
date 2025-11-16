/**
 * Asteroid Orbital Elements and Calculations
 *
 * Implements orbital calculations for major asteroids and minor planets
 *
 * Clean-room implementation based on:
 * - JPL Small-Body Database
 * - MPC (Minor Planet Center) orbital elements
 * - Meeus, Astronomical Algorithms
 */

import {
  toRadians,
  toDegrees,
  normalizeDegrees,
  julianCenturiesFromJ2000,
  J2000,
} from './astronomical-constants';

import {
  solveKeplersEquation,
  calculateTrueAnomaly,
} from './planetary-calculator';

import { applyPrecession } from './precession-nutation';

// ============================================================================
// Asteroid Identifiers
// ============================================================================

export enum AsteroidID {
  CERES = 1,
  PALLAS = 2,
  JUNO = 3,
  VESTA = 4,
  ASTRAEA = 5,
  HEBE = 6,
  IRIS = 7,
  FLORA = 8,
  METIS = 9,
  HYGIEA = 10,
  CHIRON = 2060,
  PHOLUS = 5145,
  NESSUS = 7066,
  IXION = 28978,
  HUYA = 38628,
  VARUNA = 20000,
  QUAOAR = 50000,
  SEDNA = 90377,
  ORCUS = 90482,
  ERIS = 136199,
  HAUMEA = 136108,
  MAKEMAKE = 136472,
}

// ============================================================================
// Orbital Elements at Epoch J2000
// ============================================================================

interface OrbitalElements {
  epoch: number;           // Julian Day of epoch
  a: number;               // Semi-major axis (AU)
  e: number;               // Eccentricity
  i: number;               // Inclination (degrees)
  Omega: number;           // Longitude of ascending node (degrees)
  omega: number;           // Argument of perihelion (degrees)
  M0: number;              // Mean anomaly at epoch (degrees)
  n: number;               // Mean daily motion (degrees/day)
  epochYear?: number;      // Year of epoch for display
}

/**
 * Orbital elements for major asteroids
 * Data from JPL Small-Body Database (osculating elements at J2000)
 */
export const ASTEROID_ELEMENTS: Record<number, OrbitalElements> = {
  // Main Belt Asteroids
  [AsteroidID.CERES]: {
    epoch: J2000,
    a: 2.7653485,
    e: 0.0789936,
    i: 10.58682,
    Omega: 80.39320,
    omega: 73.11528,
    M0: 95.98736,
    n: 0.21407877,
    epochYear: 2000,
  },
  [AsteroidID.PALLAS]: {
    epoch: J2000,
    a: 2.7723907,
    e: 0.2305419,
    i: 34.83646,
    Omega: 173.09298,
    omega: 310.04308,
    M0: 78.16402,
    n: 0.21343636,
    epochYear: 2000,
  },
  [AsteroidID.JUNO]: {
    epoch: J2000,
    a: 2.6693989,
    e: 0.2563690,
    i: 12.98888,
    Omega: 169.87136,
    omega: 248.18034,
    M0: 139.54918,
    n: 0.22588431,
    epochYear: 2000,
  },
  [AsteroidID.VESTA]: {
    epoch: J2000,
    a: 2.3615942,
    e: 0.0895023,
    i: 7.14043,
    Omega: 103.80937,
    omega: 151.19853,
    M0: 205.54991,
    n: 0.27154224,
    epochYear: 2000,
  },
  [AsteroidID.ASTRAEA]: {
    epoch: J2000,
    a: 2.5747492,
    e: 0.1908556,
    i: 5.36950,
    Omega: 141.57622,
    omega: 358.63918,
    M0: 295.91382,
    n: 0.23559924,
    epochYear: 2000,
  },
  [AsteroidID.HEBE]: {
    epoch: J2000,
    a: 2.4258086,
    e: 0.2023149,
    i: 14.74418,
    Omega: 138.64503,
    omega: 239.50292,
    M0: 269.55652,
    n: 0.26095803,
    epochYear: 2000,
  },

  // Centaurs
  [AsteroidID.CHIRON]: {
    epoch: J2000,
    a: 13.6973,
    e: 0.38279,
    i: 6.93519,
    Omega: 209.35046,
    omega: 339.62313,
    M0: 120.73529,
    n: 0.04664,
    epochYear: 2000,
  },
  [AsteroidID.PHOLUS]: {
    epoch: J2000,
    a: 20.3244,
    e: 0.57124,
    i: 24.66498,
    Omega: 119.42848,
    omega: 354.83686,
    M0: 246.92751,
    n: 0.02734,
    epochYear: 2000,
  },
  [AsteroidID.NESSUS]: {
    epoch: J2000,
    a: 24.5887,
    e: 0.51839,
    i: 15.65821,
    Omega: 130.38257,
    omega: 171.14642,
    M0: 43.69108,
    n: 0.02221,
    epochYear: 2000,
  },

  // Trans-Neptunian Objects (TNOs)
  [AsteroidID.VARUNA]: {
    epoch: J2000,
    a: 42.8285,
    e: 0.05085,
    i: 17.16690,
    Omega: 97.42541,
    omega: 276.33114,
    M0: 91.25408,
    n: 0.01306,
    epochYear: 2000,
  },
  [AsteroidID.QUAOAR]: {
    epoch: J2000,
    a: 43.4054,
    e: 0.03904,
    i: 8.00425,
    Omega: 188.84281,
    omega: 155.93528,
    M0: 220.46783,
    n: 0.01278,
    epochYear: 2000,
  },
  [AsteroidID.SEDNA]: {
    epoch: J2000,
    a: 518.57,
    e: 0.85491,
    i: 11.92872,
    Omega: 144.51425,
    omega: 311.12353,
    M0: 357.83925,
    n: 0.00128,
    epochYear: 2000,
  },
  [AsteroidID.ORCUS]: {
    epoch: J2000,
    a: 39.1726,
    e: 0.22706,
    i: 20.57429,
    Omega: 268.57967,
    omega: 73.29961,
    M0: 73.66903,
    n: 0.01449,
    epochYear: 2000,
  },

  // Dwarf Planets
  [AsteroidID.ERIS]: {
    epoch: J2000,
    a: 67.8641,
    e: 0.43607,
    i: 44.04084,
    Omega: 35.95111,
    omega: 151.43117,
    M0: 205.98917,
    n: 0.00774,
    epochYear: 2000,
  },
  [AsteroidID.HAUMEA]: {
    epoch: J2000,
    a: 43.1356,
    e: 0.18951,
    i: 28.21375,
    Omega: 121.90005,
    omega: 239.21990,
    M0: 218.26106,
    n: 0.01287,
    epochYear: 2000,
  },
  [AsteroidID.MAKEMAKE]: {
    epoch: J2000,
    a: 45.4368,
    e: 0.15900,
    i: 28.98479,
    Omega: 79.62001,
    omega: 294.48447,
    M0: 165.51489,
    n: 0.01180,
    epochYear: 2000,
  },
};

// ============================================================================
// Asteroid Position Calculation
// ============================================================================

/**
 * Calculate asteroid position from orbital elements
 *
 * @param elements - Orbital elements
 * @param jdTT - Julian Day (Terrestrial Time)
 * @returns Heliocentric ecliptic coordinates
 */
export function calculateAsteroidHeliocentricPosition(
  elements: OrbitalElements,
  jdTT: number
): {
  longitude: number;
  latitude: number;
  distance: number;
} {
  // Time since epoch in days
  const dt = jdTT - elements.epoch;

  // Mean anomaly at time t
  const M = normalizeDegrees(elements.M0 + elements.n * dt);

  // Solve Kepler's equation for eccentric anomaly
  const E = solveKeplersEquation(toRadians(M), elements.e);

  // True anomaly
  const nu = calculateTrueAnomaly(E, elements.e);

  // Radius vector
  const r = elements.a * (1 - elements.e * Math.cos(E));

  // Convert to heliocentric ecliptic coordinates
  const iRad = toRadians(elements.i);
  const OmegaRad = toRadians(elements.Omega);
  const omegaRad = toRadians(elements.omega);

  // Argument of latitude
  const u = nu + omegaRad;

  // Position in orbital plane
  const xOrb = r * Math.cos(u);
  const yOrb = r * Math.sin(u);

  // Transform to ecliptic plane
  const cosOmega = Math.cos(OmegaRad);
  const sinOmega = Math.sin(OmegaRad);
  const cosI = Math.cos(iRad);
  const sinI = Math.sin(iRad);

  const x = (cosOmega * Math.cos(u) - sinOmega * Math.sin(u) * cosI) * r;
  const y = (sinOmega * Math.cos(u) + cosOmega * Math.sin(u) * cosI) * r;
  const z = Math.sin(u) * sinI * r;

  // Convert to spherical coordinates
  const longitude = normalizeDegrees(toDegrees(Math.atan2(y, x)));
  const latitude = toDegrees(Math.asin(z / r));
  const distance = r;

  return { longitude, latitude, distance };
}

/**
 * Calculate geocentric position of an asteroid
 *
 * @param asteroidID - Asteroid identifier
 * @param jdTT - Julian Day (Terrestrial Time)
 * @param earthPosition - Earth's heliocentric position
 * @param applyPrecessionToDate - Apply precession to date
 * @returns Geocentric ecliptic coordinates
 */
export function calculateAsteroidPosition(
  asteroidID: AsteroidID,
  jdTT: number,
  earthPosition: { longitude: number; latitude: number; distance: number },
  applyPrecessionToDate: boolean = true
): {
  longitude: number;
  latitude: number;
  distance: number;
  longitudeSpeed?: number;
  latitudeSpeed?: number;
  distanceSpeed?: number;
} {
  const elements = ASTEROID_ELEMENTS[asteroidID];
  if (!elements) {
    throw new Error(`No orbital elements found for asteroid ${asteroidID}`);
  }

  // Calculate heliocentric position
  const helioPos = calculateAsteroidHeliocentricPosition(elements, jdTT);

  // Convert to geocentric
  const geocentricPos = heliocentricToGeocentric(helioPos, earthPosition);

  // Apply precession if requested
  if (applyPrecessionToDate) {
    const T = julianCenturiesFromJ2000(jdTT);
    if (T !== 0) {
      const precessed = applyPrecession(
        geocentricPos.longitude,
        geocentricPos.latitude,
        T
      );
      geocentricPos.longitude = precessed.longitude;
      geocentricPos.latitude = precessed.latitude;
    }
  }

  return geocentricPos;
}

/**
 * Calculate asteroid position with speed
 *
 * @param asteroidID - Asteroid identifier
 * @param jdTT - Julian Day (Terrestrial Time)
 * @param earthPosition - Earth's heliocentric position
 * @param dt - Time step for numerical differentiation
 * @returns Position with speed
 */
export function calculateAsteroidPositionWithSpeed(
  asteroidID: AsteroidID,
  jdTT: number,
  earthPosition: { longitude: number; latitude: number; distance: number },
  dt: number = 0.01
): {
  longitude: number;
  latitude: number;
  distance: number;
  longitudeSpeed: number;
  latitudeSpeed: number;
  distanceSpeed: number;
} {
  // Calculate Earth positions at t-dt, t, t+dt
  // (In practice, would need Earth calculator, simplified here)

  const posMinus = calculateAsteroidPosition(asteroidID, jdTT - dt, earthPosition, true);
  const pos = calculateAsteroidPosition(asteroidID, jdTT, earthPosition, true);
  const posPlus = calculateAsteroidPosition(asteroidID, jdTT + dt, earthPosition, true);

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

/**
 * Convert heliocentric to geocentric coordinates
 */
function heliocentricToGeocentric(
  asteroid: { longitude: number; latitude: number; distance: number },
  earth: { longitude: number; latitude: number; distance: number }
): { longitude: number; latitude: number; distance: number } {
  // Convert to Cartesian
  const lonARad = toRadians(asteroid.longitude);
  const latARad = toRadians(asteroid.latitude);
  const lonERad = toRadians(earth.longitude);
  const latERad = toRadians(earth.latitude);

  const xa = asteroid.distance * Math.cos(latARad) * Math.cos(lonARad);
  const ya = asteroid.distance * Math.cos(latARad) * Math.sin(lonARad);
  const za = asteroid.distance * Math.sin(latARad);

  const xe = earth.distance * Math.cos(latERad) * Math.cos(lonERad);
  const ye = earth.distance * Math.cos(latERad) * Math.sin(lonERad);
  const ze = earth.distance * Math.sin(latERad);

  // Geocentric position
  const x = xa - xe;
  const y = ya - ye;
  const z = za - ze;

  // Convert back to spherical
  const distance = Math.sqrt(x * x + y * y + z * z);
  const longitude = normalizeDegrees(toDegrees(Math.atan2(y, x)));
  const latitude = toDegrees(Math.asin(z / distance));

  return { longitude, latitude, distance };
}

/**
 * Get asteroid name
 */
export function getAsteroidName(asteroidID: AsteroidID): string {
  const names: Record<number, string> = {
    [AsteroidID.CERES]: 'Ceres',
    [AsteroidID.PALLAS]: 'Pallas',
    [AsteroidID.JUNO]: 'Juno',
    [AsteroidID.VESTA]: 'Vesta',
    [AsteroidID.ASTRAEA]: 'Astraea',
    [AsteroidID.HEBE]: 'Hebe',
    [AsteroidID.IRIS]: 'Iris',
    [AsteroidID.FLORA]: 'Flora',
    [AsteroidID.METIS]: 'Metis',
    [AsteroidID.HYGIEA]: 'Hygiea',
    [AsteroidID.CHIRON]: 'Chiron',
    [AsteroidID.PHOLUS]: 'Pholus',
    [AsteroidID.NESSUS]: 'Nessus',
    [AsteroidID.IXION]: 'Ixion',
    [AsteroidID.HUYA]: 'Huya',
    [AsteroidID.VARUNA]: 'Varuna',
    [AsteroidID.QUAOAR]: 'Quaoar',
    [AsteroidID.SEDNA]: 'Sedna',
    [AsteroidID.ORCUS]: 'Orcus',
    [AsteroidID.ERIS]: 'Eris',
    [AsteroidID.HAUMEA]: 'Haumea',
    [AsteroidID.MAKEMAKE]: 'Makemake',
  };

  return names[asteroidID] || `Asteroid ${asteroidID}`;
}
