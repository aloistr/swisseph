/**
 * Fixed Stars Catalog and Calculations
 *
 * Bright star catalog with proper motion corrections
 *
 * Clean-room implementation based on:
 * - Hipparcos/Tycho catalogs
 * - Yale Bright Star Catalog
 * - Standard astronomical star data
 */

import {
  toRadians,
  toDegrees,
  normalizeDegrees,
  julianCenturiesFromJ2000,
  ARCSEC_TO_RAD,
  J2000,
} from './astronomical-constants';

import { applyPrecession } from './precession-nutation';

// ============================================================================
// Fixed Star Data Structure
// ============================================================================

export interface FixedStarData {
  name: string;              // Common name
  bayer?: string;            // Bayer designation (e.g., "Alpha Tauri")
  constellation: string;     // Constellation abbreviation
  ra2000: number;            // Right ascension J2000 (degrees)
  dec2000: number;           // Declination J2000 (degrees)
  properMotionRA: number;    // Proper motion in RA (mas/year)
  properMotionDec: number;   // Proper motion in Dec (mas/year)
  parallax: number;          // Parallax (mas)
  radialVelocity: number;    // Radial velocity (km/s)
  magnitude: number;         // Visual magnitude
  spectralType?: string;     // Spectral type
}

// ============================================================================
// Bright Stars Catalog
// ============================================================================

/**
 * Catalog of bright and astrologically significant fixed stars
 * Data from Hipparcos catalog and traditional sources
 */
export const FIXED_STARS_CATALOG: Record<string, FixedStarData> = {
  // First Magnitude Stars
  'Sirius': {
    name: 'Sirius',
    bayer: 'Alpha Canis Majoris',
    constellation: 'CMa',
    ra2000: 101.287155,
    dec2000: -16.716116,
    properMotionRA: -546.01,
    properMotionDec: -1223.08,
    parallax: 379.21,
    radialVelocity: -5.5,
    magnitude: -1.46,
    spectralType: 'A1V',
  },
  'Canopus': {
    name: 'Canopus',
    bayer: 'Alpha Carinae',
    constellation: 'Car',
    ra2000: 95.987958,
    dec2000: -52.695661,
    properMotionRA: 19.93,
    properMotionDec: 23.24,
    parallax: 10.43,
    radialVelocity: 20.3,
    magnitude: -0.74,
    spectralType: 'F0Ib',
  },
  'Arcturus': {
    name: 'Arcturus',
    bayer: 'Alpha Boötis',
    constellation: 'Boo',
    ra2000: 213.915300,
    dec2000: 19.182409,
    properMotionRA: -1093.45,
    properMotionDec: -1999.40,
    parallax: 88.83,
    radialVelocity: -5.2,
    magnitude: -0.05,
    spectralType: 'K1.5III',
  },
  'Vega': {
    name: 'Vega',
    bayer: 'Alpha Lyrae',
    constellation: 'Lyr',
    ra2000: 279.234735,
    dec2000: 38.783689,
    properMotionRA: 200.94,
    properMotionDec: 286.23,
    parallax: 130.23,
    radialVelocity: -20.6,
    magnitude: 0.03,
    spectralType: 'A0V',
  },
  'Capella': {
    name: 'Capella',
    bayer: 'Alpha Aurigae',
    constellation: 'Aur',
    ra2000: 79.172328,
    dec2000: 45.997991,
    properMotionRA: 75.52,
    properMotionDec: -426.89,
    parallax: 76.20,
    radialVelocity: 30.2,
    magnitude: 0.08,
    spectralType: 'G8III',
  },
  'Rigel': {
    name: 'Rigel',
    bayer: 'Beta Orionis',
    constellation: 'Ori',
    ra2000: 78.634467,
    dec2000: -8.201638,
    properMotionRA: 1.31,
    properMotionDec: -0.56,
    parallax: 3.78,
    radialVelocity: 17.8,
    magnitude: 0.13,
    spectralType: 'B8Ia',
  },
  'Procyon': {
    name: 'Procyon',
    bayer: 'Alpha Canis Minoris',
    constellation: 'CMi',
    ra2000: 114.825493,
    dec2000: 5.224993,
    properMotionRA: -716.57,
    properMotionDec: -1034.58,
    parallax: 284.56,
    radialVelocity: -3.2,
    magnitude: 0.34,
    spectralType: 'F5IV-V',
  },
  'Betelgeuse': {
    name: 'Betelgeuse',
    bayer: 'Alpha Orionis',
    constellation: 'Ori',
    ra2000: 88.792939,
    dec2000: 7.407064,
    properMotionRA: 27.33,
    properMotionDec: 10.86,
    parallax: 5.95,
    radialVelocity: 21.0,
    magnitude: 0.50,
    spectralType: 'M1-2Ia-Iab',
  },
  'Achernar': {
    name: 'Achernar',
    bayer: 'Alpha Eridani',
    constellation: 'Eri',
    ra2000: 24.428523,
    dec2000: -57.236753,
    properMotionRA: 88.02,
    properMotionDec: -40.08,
    parallax: 22.68,
    radialVelocity: 16.0,
    magnitude: 0.46,
    spectralType: 'B6Vep',
  },
  'Altair': {
    name: 'Altair',
    bayer: 'Alpha Aquilae',
    constellation: 'Aql',
    ra2000: 297.695827,
    dec2000: 8.868321,
    properMotionRA: 536.23,
    properMotionDec: 385.29,
    parallax: 194.44,
    radialVelocity: -26.1,
    magnitude: 0.77,
    spectralType: 'A7V',
  },

  // Royal Stars
  'Aldebaran': {
    name: 'Aldebaran',
    bayer: 'Alpha Tauri',
    constellation: 'Tau',
    ra2000: 68.980163,
    dec2000: 16.509302,
    properMotionRA: 62.78,
    properMotionDec: -189.36,
    parallax: 50.09,
    radialVelocity: 54.3,
    magnitude: 0.85,
    spectralType: 'K5III',
  },
  'Regulus': {
    name: 'Regulus',
    bayer: 'Alpha Leonis',
    constellation: 'Leo',
    ra2000: 152.092962,
    dec2000: 11.967208,
    properMotionRA: -249.40,
    properMotionDec: 5.59,
    parallax: 42.09,
    radialVelocity: 5.9,
    magnitude: 1.35,
    spectralType: 'B7V',
  },
  'Antares': {
    name: 'Antares',
    bayer: 'Alpha Scorpii',
    constellation: 'Sco',
    ra2000: 247.351915,
    dec2000: -26.432003,
    properMotionRA: -12.11,
    properMotionDec: -23.30,
    parallax: 5.40,
    radialVelocity: -3.4,
    magnitude: 1.09,
    spectralType: 'M1.5Iab-Ib',
  },
  'Fomalhaut': {
    name: 'Fomalhaut',
    bayer: 'Alpha Piscis Austrini',
    constellation: 'PsA',
    ra2000: 344.412693,
    dec2000: -29.622237,
    properMotionRA: 329.22,
    properMotionDec: -164.22,
    parallax: 129.81,
    radialVelocity: 6.5,
    magnitude: 1.16,
    spectralType: 'A3V',
  },

  // Other Notable Stars
  'Spica': {
    name: 'Spica',
    bayer: 'Alpha Virginis',
    constellation: 'Vir',
    ra2000: 201.298247,
    dec2000: -11.161319,
    properMotionRA: -42.50,
    properMotionDec: -31.73,
    parallax: 12.44,
    radialVelocity: 1.0,
    magnitude: 1.04,
    spectralType: 'B1III-IV',
  },
  'Pollux': {
    name: 'Pollux',
    bayer: 'Beta Geminorum',
    constellation: 'Gem',
    ra2000: 116.328958,
    dec2000: 28.026199,
    properMotionRA: -625.69,
    properMotionDec: -45.95,
    parallax: 96.54,
    radialVelocity: 3.3,
    magnitude: 1.14,
    spectralType: 'K0III',
  },
  'Deneb': {
    name: 'Deneb',
    bayer: 'Alpha Cygni',
    constellation: 'Cyg',
    ra2000: 310.357980,
    dec2000: 45.280339,
    properMotionRA: 1.99,
    properMotionDec: 1.95,
    parallax: 2.31,
    radialVelocity: -4.5,
    magnitude: 1.25,
    spectralType: 'A2Ia',
  },
  'AlphaCrucis': {
    name: 'Acrux',
    bayer: 'Alpha Crucis',
    constellation: 'Cru',
    ra2000: 186.649563,
    dec2000: -63.099093,
    properMotionRA: -35.83,
    properMotionDec: -14.86,
    parallax: 10.17,
    radialVelocity: -11.2,
    magnitude: 0.77,
    spectralType: 'B0.5IV',
  },

  // Pleiades
  'Alcyone': {
    name: 'Alcyone',
    bayer: 'Eta Tauri',
    constellation: 'Tau',
    ra2000: 56.871152,
    dec2000: 24.105136,
    properMotionRA: 19.35,
    properMotionDec: -43.11,
    parallax: 8.09,
    radialVelocity: 9.2,
    magnitude: 2.87,
    spectralType: 'B7IIIe',
  },
};

// ============================================================================
// Fixed Star Position Calculation
// ============================================================================

/**
 * Calculate fixed star position with proper motion
 *
 * @param starData - Fixed star data
 * @param jdTT - Julian Day (Terrestrial Time)
 * @param applyPrecession - Apply precession (default: true)
 * @returns Ecliptic coordinates
 */
export function calculateFixedStarPosition(
  starData: FixedStarData,
  jdTT: number,
  applyPrecession: boolean = true
): {
  longitude: number;
  latitude: number;
  rightAscension: number;
  declination: number;
  distance: number;  // parsecs
} {
  // Time in Julian centuries from J2000
  const T = julianCenturiesFromJ2000(jdTT);

  // Apply proper motion (convert mas/year to degrees/century)
  const pmRA = (starData.properMotionRA / 1000 / 3600) * 100; // degrees/century
  const pmDec = (starData.properMotionDec / 1000 / 3600) * 100; // degrees/century

  let ra = starData.ra2000 + pmRA * T;
  let dec = starData.dec2000 + pmDec * T;

  // Normalize RA to 0-360
  ra = normalizeDegrees(ra);

  // Calculate distance from parallax (parallax in mas)
  const distance = starData.parallax > 0 ? 1000 / starData.parallax : 1000000; // parsecs

  // Convert equatorial to ecliptic
  const T0 = 0; // J2000 obliquity
  const obliquity = 23.439291; // Mean obliquity at J2000

  const raRad = toRadians(ra);
  const decRad = toRadians(dec);
  const oblRad = toRadians(obliquity);

  // Ecliptic coordinates
  const sinLat =
    Math.sin(decRad) * Math.cos(oblRad) -
    Math.cos(decRad) * Math.sin(oblRad) * Math.sin(raRad);

  const latitude = toDegrees(Math.asin(sinLat));

  const y = Math.sin(raRad) * Math.cos(oblRad) + Math.tan(decRad) * Math.sin(oblRad);
  const x = Math.cos(raRad);

  let longitude = normalizeDegrees(toDegrees(Math.atan2(y, x)));

  // Apply precession to date if requested
  if (applyPrecession && T !== 0) {
    const precessed = applyPrecession(longitude, latitude, T);
    longitude = precessed.longitude;
    // Latitude changes very little with precession for most stars
  }

  return {
    longitude,
    latitude,
    rightAscension: ra,
    declination: dec,
    distance,
  };
}

/**
 * Get fixed star by name
 *
 * @param name - Star name (case-insensitive partial match)
 * @returns Star data or undefined
 */
export function getFixedStarByName(name: string): FixedStarData | undefined {
  const searchName = name.toLowerCase().replace(/\s+/g, '');

  for (const [key, star] of Object.entries(FIXED_STARS_CATALOG)) {
    const starName = star.name.toLowerCase().replace(/\s+/g, '');
    const bayerName = star.bayer?.toLowerCase().replace(/\s+/g, '') || '';

    if (
      starName.includes(searchName) ||
      searchName.includes(starName) ||
      bayerName.includes(searchName) ||
      searchName.includes(bayerName)
    ) {
      return star;
    }
  }

  return undefined;
}

/**
 * List all fixed stars
 *
 * @returns Array of star names
 */
export function listFixedStars(): string[] {
  return Object.values(FIXED_STARS_CATALOG).map((star) => star.name);
}

/**
 * Get brightest fixed stars
 *
 * @param limit - Maximum number of stars to return
 * @returns Array of brightest stars
 */
export function getBrightestStars(limit: number = 20): FixedStarData[] {
  return Object.values(FIXED_STARS_CATALOG)
    .sort((a, b) => a.magnitude - b.magnitude)
    .slice(0, limit);
}

/**
 * Get stars in a constellation
 *
 * @param constellation - Constellation abbreviation (e.g., 'Tau', 'Leo')
 * @returns Array of stars in constellation
 */
export function getStarsInConstellation(constellation: string): FixedStarData[] {
  const constUpper = constellation.toUpperCase();
  return Object.values(FIXED_STARS_CATALOG).filter(
    (star) => star.constellation.toUpperCase() === constUpper
  );
}
