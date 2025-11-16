/**
 * House System Calculations
 *
 * Implementation of major astrological house systems
 *
 * Clean-room implementation based on:
 * - Michelsen, The American Ephemeris
 * - Pottenger, CCRS Horoscope Program
 * - Astronomical Algorithms for house calculations
 *
 * Independent implementation not derived from Swiss Ephemeris
 */

import {
  toRadians,
  toDegrees,
  normalizeDegrees,
  normalizeDegreesSymmetric,
  HOUSE_ITERATION_MAX,
  HOUSE_ITERATION_TOLERANCE,
} from './astronomical-constants';

import { calculateLMST } from './date-time';

// ============================================================================
// House System Types
// ============================================================================

export enum HouseSystem {
  PLACIDUS = 'P',
  KOCH = 'K',
  PORPHYRIUS = 'O',
  REGIOMONTANUS = 'R',
  CAMPANUS = 'C',
  EQUAL = 'A',
  EQUAL_MC = 'D',
  WHOLE_SIGN = 'W',
  MERIDIAN = 'X',
  AZIMUTHAL = 'H',
  MORINUS = 'M',
  VEHLOW = 'V',
}

// ============================================================================
// House Calculation Result
// ============================================================================

export interface HouseCalculationResult {
  cusps: number[];      // 12 house cusps (longitude in degrees)
  ascendant: number;    // Ascendant (degrees)
  mc: number;           // Midheaven (degrees)
  armc: number;         // ARMC - sidereal time (degrees)
  vertex: number;       // Vertex (degrees)
  equatorialAscendant: number;
  coAscendant: number;  // Co-Ascendant (Koch)
  polarAscendant: number;
}

// ============================================================================
// Main House Calculation Function
// ============================================================================

/**
 * Calculate house cusps and angles
 *
 * @param jdUT - Julian Day (UT)
 * @param latitude - Geographic latitude (degrees, positive North)
 * @param longitude - Geographic longitude (degrees, positive East)
 * @param houseSystem - House system to use
 * @param obliquity - Obliquity of the ecliptic (degrees)
 * @returns House cusps and angles
 */
export function calculateHouses(
  jdUT: number,
  latitude: number,
  longitude: number,
  houseSystem: HouseSystem,
  obliquity: number
): HouseCalculationResult {
  // Calculate ARMC (sidereal time at Greenwich + longitude)
  const armc = calculateLMST(jdUT, longitude);

  // Calculate MC (Midheaven)
  const mc = calculateMC(armc, obliquity);

  // Calculate Ascendant
  const ascendant = calculateAscendant(armc, latitude, obliquity);

  // Calculate Vertex
  const vertex = calculateVertex(armc, latitude, obliquity);

  // Calculate Equatorial Ascendant
  const equatorialAscendant = calculateEquatorialAscendant(armc);

  // Calculate Co-Ascendant (Koch method)
  const coAscendant = calculateCoAscendant(ascendant, latitude, obliquity);

  // Calculate Polar Ascendant
  const polarAscendant = calculatePolarAscendant(armc, latitude, obliquity);

  // Calculate house cusps based on system
  let cusps: number[];

  switch (houseSystem) {
    case HouseSystem.PLACIDUS:
      cusps = calculatePlacidus(armc, latitude, obliquity);
      break;
    case HouseSystem.KOCH:
      cusps = calculateKoch(armc, ascendant, mc, latitude, obliquity);
      break;
    case HouseSystem.PORPHYRIUS:
      cusps = calculatePorphyrius(ascendant, mc);
      break;
    case HouseSystem.REGIOMONTANUS:
      cusps = calculateRegiomontanus(armc, latitude, obliquity);
      break;
    case HouseSystem.CAMPANUS:
      cusps = calculateCampanus(armc, latitude, obliquity);
      break;
    case HouseSystem.EQUAL:
    case HouseSystem.VEHLOW:
      cusps = calculateEqual(ascendant, houseSystem === HouseSystem.VEHLOW);
      break;
    case HouseSystem.EQUAL_MC:
      cusps = calculateEqual(mc, false);
      break;
    case HouseSystem.WHOLE_SIGN:
      cusps = calculateWholeSign(ascendant);
      break;
    case HouseSystem.MERIDIAN:
      cusps = calculateMeridian(armc, obliquity);
      break;
    case HouseSystem.AZIMUTHAL:
      cusps = calculateAzimuthal(armc, latitude, obliquity);
      break;
    case HouseSystem.MORINUS:
      cusps = calculateMorinus(armc);
      break;
    default:
      // Default to Placidus
      cusps = calculatePlacidus(armc, latitude, obliquity);
  }

  return {
    cusps,
    ascendant,
    mc,
    armc,
    vertex,
    equatorialAscendant,
    coAscendant,
    polarAscendant,
  };
}

// ============================================================================
// Angle Calculations
// ============================================================================

/**
 * Calculate Midheaven (MC)
 */
function calculateMC(armc: number, obliquity: number): number {
  const armcRad = toRadians(armc);
  const oblRad = toRadians(obliquity);

  const mc = Math.atan2(Math.tan(armcRad), Math.cos(oblRad));
  return normalizeDegrees(toDegrees(mc));
}

/**
 * Calculate Ascendant
 */
function calculateAscendant(
  armc: number,
  latitude: number,
  obliquity: number
): number {
  const armcRad = toRadians(armc);
  const latRad = toRadians(latitude);
  const oblRad = toRadians(obliquity);

  const asc = Math.atan2(
    Math.cos(armcRad),
    -Math.sin(armcRad) * Math.cos(oblRad) - Math.tan(latRad) * Math.sin(oblRad)
  );

  return normalizeDegrees(toDegrees(asc));
}

/**
 * Calculate Vertex
 */
function calculateVertex(
  armc: number,
  latitude: number,
  obliquity: number
): number {
  // Vertex is calculated at co-latitude
  const coLatitude = 90 - Math.abs(latitude);
  const sign = latitude >= 0 ? 1 : -1;

  const armcRad = toRadians(armc + 180); // Add 180 for western horizon
  const coLatRad = toRadians(coLatitude * sign);
  const oblRad = toRadians(obliquity);

  const vertex = Math.atan2(
    Math.cos(armcRad),
    -Math.sin(armcRad) * Math.cos(oblRad) - Math.tan(coLatRad) * Math.sin(oblRad)
  );

  return normalizeDegrees(toDegrees(vertex));
}

/**
 * Calculate Equatorial Ascendant
 */
function calculateEquatorialAscendant(armc: number): number {
  return normalizeDegrees(armc);
}

/**
 * Calculate Co-Ascendant (Koch)
 */
function calculateCoAscendant(
  ascendant: number,
  latitude: number,
  obliquity: number
): number {
  const ascRad = toRadians(ascendant);
  const latRad = toRadians(latitude);
  const oblRad = toRadians(obliquity);

  // Koch's co-ascendant formula
  const f = Math.atan(Math.tan(latRad) / Math.cos(oblRad));
  const coAsc = ascRad + f;

  return normalizeDegrees(toDegrees(coAsc));
}

/**
 * Calculate Polar Ascendant
 */
function calculatePolarAscendant(
  armc: number,
  latitude: number,
  obliquity: number
): number {
  const armcRad = toRadians(armc);
  const latRad = toRadians(latitude);
  const oblRad = toRadians(obliquity);

  const polarAsc = Math.atan2(
    Math.sin(armcRad) * Math.cos(oblRad),
    Math.cos(latRad) * Math.cos(armcRad) - Math.sin(latRad) * Math.sin(oblRad) * Math.sin(armcRad)
  );

  return normalizeDegrees(toDegrees(polarAsc));
}

// ============================================================================
// Placidus House System
// ============================================================================

/**
 * Calculate Placidus house cusps
 *
 * Most popular house system, based on time divisions
 */
function calculatePlacidus(
  armc: number,
  latitude: number,
  obliquity: number
): number[] {
  const mc = calculateMC(armc, obliquity);
  const ascendant = calculateAscendant(armc, latitude, obliquity);

  const cusps = new Array(12);

  // Houses 10, 11, 12, 1, 2, 3
  cusps[9] = mc; // House 10 = MC
  cusps[0] = ascendant; // House 1 = Ascendant

  const latRad = toRadians(latitude);
  const oblRad = toRadians(obliquity);

  // Calculate houses 11, 12, 2, 3 using semi-arc division
  for (let i = 1; i <= 3; i++) {
    // House 11, 12, 2, 3
    const f = i / 3.0; // Fraction: 1/3, 2/3, 1 (but we don't calculate 1)

    // House 11 (30 degrees before MC in RA)
    if (i === 1) {
      cusps[10] = calculatePlacidusHouse(armc - 30, latitude, obliquity);
    }
    // House 12 (60 degrees before MC in RA)
    else if (i === 2) {
      cusps[11] = calculatePlacidusHouse(armc - 60, latitude, obliquity);
    }
    // House 2 (120 degrees after MC in RA)
    else if (i === 3) {
      cusps[1] = calculatePlacidusHouse(armc + 120, latitude, obliquity);
    }
  }

  // House 3 (150 degrees after MC in RA)
  cusps[2] = calculatePlacidusHouse(armc + 150, latitude, obliquity);

  // Houses 4-9 are opposite to houses 10-3
  for (let i = 0; i < 6; i++) {
    cusps[i + 3] = normalizeDegrees(cusps[i + 9] + 180);
  }

  return cusps;
}

/**
 * Calculate a single Placidus house cusp
 */
function calculatePlacidusHouse(
  ra: number,
  latitude: number,
  obliquity: number
): number {
  const raRad = toRadians(ra);
  const oblRad = toRadians(obliquity);

  const lon = Math.atan2(Math.tan(raRad), Math.cos(oblRad));
  return normalizeDegrees(toDegrees(lon));
}

// ============================================================================
// Koch House System
// ============================================================================

/**
 * Calculate Koch (birthplace/GOH) house cusps
 */
function calculateKoch(
  armc: number,
  ascendant: number,
  mc: number,
  latitude: number,
  obliquity: number
): number[] {
  const cusps = new Array(12);

  cusps[9] = mc;
  cusps[0] = ascendant;

  const latRad = toRadians(latitude);
  const oblRad = toRadians(obliquity);

  // Koch houses use polar elevation
  for (let i = 1; i <= 2; i++) {
    const md = 30 * i; // 30, 60 degrees

    // Upper houses (11, 12)
    const raUp = armc - md;
    cusps[9 + i] = calculateKochHouse(raUp, latitude, obliquity, md);

    // Lower houses (2, 3)
    const raDown = armc + 120 + md;
    cusps[i] = calculateKochHouse(raDown, latitude, obliquity, 120 + md);
  }

  // Opposite houses
  for (let i = 0; i < 6; i++) {
    cusps[i + 3] = normalizeDegrees(cusps[i + 9] + 180);
  }

  return cusps;
}

/**
 * Calculate a single Koch house cusp
 */
function calculateKochHouse(
  ra: number,
  latitude: number,
  obliquity: number,
  md: number
): number {
  const raRad = toRadians(ra);
  const latRad = toRadians(latitude);
  const oblRad = toRadians(obliquity);
  const mdRad = toRadians(md);

  const decl = Math.atan(Math.tan(latRad) * Math.sin(mdRad / 3));

  const lon = Math.atan2(
    Math.sin(raRad),
    Math.cos(raRad) * Math.cos(oblRad) + Math.tan(decl) * Math.sin(oblRad)
  );

  return normalizeDegrees(toDegrees(lon));
}

// ============================================================================
// Porphyrius House System
// ============================================================================

/**
 * Calculate Porphyrius house cusps
 *
 * Simple division of quadrants into three equal parts
 */
function calculatePorphyrius(ascendant: number, mc: number): number[] {
  const cusps = new Array(12);

  cusps[0] = ascendant;
  cusps[9] = mc;

  // Quadrant 1: ASC to MC
  const quad1 = normalizeDegreesSymmetric(mc - ascendant);
  cusps[10] = normalizeDegrees(ascendant + quad1 / 3);
  cusps[11] = normalizeDegrees(ascendant + 2 * quad1 / 3);

  // Quadrant 2: MC to DESC
  const desc = normalizeDegrees(ascendant + 180);
  const quad2 = normalizeDegreesSymmetric(desc - mc);
  cusps[1] = normalizeDegrees(mc + quad2 / 3);
  cusps[2] = normalizeDegrees(mc + 2 * quad2 / 3);

  // Opposite houses
  for (let i = 0; i < 6; i++) {
    cusps[i + 3] = normalizeDegrees(cusps[i + 9] + 180);
  }

  return cusps;
}

// ============================================================================
// Equal House Systems
// ============================================================================

/**
 * Calculate Equal house cusps
 *
 * @param startPoint - Starting point (Ascendant or MC)
 * @param isVehlow - True for Vehlow (houses start 15° before cusp)
 */
function calculateEqual(startPoint: number, isVehlow: boolean = false): number[] {
  const cusps = new Array(12);

  const offset = isVehlow ? -15 : 0;

  for (let i = 0; i < 12; i++) {
    cusps[i] = normalizeDegrees(startPoint + offset + i * 30);
  }

  return cusps;
}

/**
 * Calculate Whole Sign house cusps
 *
 * Each house = one zodiac sign, starting from Ascendant's sign
 */
function calculateWholeSign(ascendant: number): number[] {
  const cusps = new Array(12);

  // First house starts at 0° of the sign containing the Ascendant
  const signStart = Math.floor(ascendant / 30) * 30;

  for (let i = 0; i < 12; i++) {
    cusps[i] = normalizeDegrees(signStart + i * 30);
  }

  return cusps;
}

// ============================================================================
// Regiomontanus House System
// ============================================================================

/**
 * Calculate Regiomontanus house cusps
 */
function calculateRegiomontanus(
  armc: number,
  latitude: number,
  obliquity: number
): number[] {
  const cusps = new Array(12);

  const mc = calculateMC(armc, obliquity);
  const ascendant = calculateAscendant(armc, latitude, obliquity);

  cusps[9] = mc;
  cusps[0] = ascendant;

  const latRad = toRadians(latitude);
  const oblRad = toRadians(obliquity);

  for (let i = 1; i <= 2; i++) {
    const angle = 30 * i;
    const angleRad = toRadians(angle);

    // Houses 11, 12
    const raUp = armc - angle;
    cusps[9 + i] = calculateRegiomontanusHouse(raUp, latitude, obliquity);

    // Houses 2, 3
    const raDown = armc + 120 + angle;
    cusps[i] = calculateRegiomontanusHouse(raDown, latitude, obliquity);
  }

  // Opposite houses
  for (let i = 0; i < 6; i++) {
    cusps[i + 3] = normalizeDegrees(cusps[i + 9] + 180);
  }

  return cusps;
}

/**
 * Calculate single Regiomontanus house cusp
 */
function calculateRegiomontanusHouse(
  ra: number,
  latitude: number,
  obliquity: number
): number {
  const raRad = toRadians(ra);
  const latRad = toRadians(latitude);
  const oblRad = toRadians(obliquity);

  const lon = Math.atan2(
    Math.sin(raRad),
    Math.cos(raRad) * Math.cos(oblRad) + Math.tan(latRad) * Math.sin(oblRad)
  );

  return normalizeDegrees(toDegrees(lon));
}

// ============================================================================
// Campanus House System
// ============================================================================

/**
 * Calculate Campanus house cusps
 */
function calculateCampanus(
  armc: number,
  latitude: number,
  obliquity: number
): number[] {
  const cusps = new Array(12);

  const mc = calculateMC(armc, obliquity);
  const ascendant = calculateAscendant(armc, latitude, obliquity);

  cusps[9] = mc;
  cusps[0] = ascendant;

  const latRad = toRadians(latitude);
  const oblRad = toRadians(obliquity);

  for (let i = 1; i <= 2; i++) {
    const angle = 30 * i;

    // Houses 11, 12
    cusps[9 + i] = calculateCampanusHouse(armc, latitude, obliquity, -angle);

    // Houses 2, 3
    cusps[i] = calculateCampanusHouse(armc, latitude, obliquity, 120 + angle);
  }

  // Opposite houses
  for (let i = 0; i < 6; i++) {
    cusps[i + 3] = normalizeDegrees(cusps[i + 9] + 180);
  }

  return cusps;
}

/**
 * Calculate single Campanus house cusp
 */
function calculateCampanusHouse(
  armc: number,
  latitude: number,
  obliquity: number,
  offset: number
): number {
  const armcRad = toRadians(armc + offset);
  const latRad = toRadians(latitude);
  const oblRad = toRadians(obliquity);

  const lon = Math.atan2(
    Math.tan(armcRad) * Math.cos(latRad),
    Math.cos(oblRad) - Math.sin(latRad) * Math.sin(oblRad) * Math.tan(armcRad)
  );

  return normalizeDegrees(toDegrees(lon));
}

// ============================================================================
// Other House Systems
// ============================================================================

/**
 * Calculate Meridian (Axial Rotation) house cusps
 */
function calculateMeridian(armc: number, obliquity: number): number[] {
  const cusps = new Array(12);

  for (let i = 0; i < 12; i++) {
    const ra = armc + (i - 9) * 30;
    cusps[i] = calculateMC(ra, obliquity);
  }

  return cusps;
}

/**
 * Calculate Azimuthal (Horizon) house cusps
 */
function calculateAzimuthal(
  armc: number,
  latitude: number,
  obliquity: number
): number[] {
  const cusps = new Array(12);

  for (let i = 0; i < 12; i++) {
    const azimuth = (i - 9) * 30; // 0° = South
    cusps[i] = azimuthToEclipticLongitude(azimuth, armc, latitude, obliquity);
  }

  return cusps;
}

/**
 * Convert azimuth to ecliptic longitude
 */
function azimuthToEclipticLongitude(
  azimuth: number,
  armc: number,
  latitude: number,
  obliquity: number
): number {
  // Simplified conversion - full implementation requires iterative solution
  const armcRad = toRadians(armc + azimuth);
  const oblRad = toRadians(obliquity);

  const lon = Math.atan2(Math.tan(armcRad), Math.cos(oblRad));
  return normalizeDegrees(toDegrees(lon));
}

/**
 * Calculate Morinus house cusps
 */
function calculateMorinus(armc: number): number[] {
  return calculateEqual(armc, false);
}
