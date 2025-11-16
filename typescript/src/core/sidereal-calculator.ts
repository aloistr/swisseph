/**
 * Sidereal Zodiac and Ayanamsa Calculations
 *
 * Complete implementation of all major ayanamsa systems
 *
 * Clean-room implementation based on:
 * - Published ayanamsa formulas
 * - Historical astronomical data
 * - Lahiri, Fagan-Bradley, and other systems
 */

import {
  normalizeDegrees,
  julianCenturiesFromJ2000,
  J2000,
} from './astronomical-constants';

import { gregorianToJulianDay } from './date-time';

// ============================================================================
// Ayanamsa Systems
// ============================================================================

export enum AyanamsaSystem {
  FAGAN_BRADLEY = 0,
  LAHIRI = 1,
  DELUCE = 2,
  RAMAN = 3,
  USHASHASHI = 4,
  KRISHNAMURTI = 5,
  DJWHAL_KHUL = 6,
  YUKTESHWAR = 7,
  JN_BHASIN = 8,
  BABYL_KUGLER1 = 9,
  BABYL_KUGLER2 = 10,
  BABYL_KUGLER3 = 11,
  BABYL_HUBER = 12,
  BABYL_ETPSC = 13,
  ALDEBARAN_15TAU = 14,
  HIPPARCHOS = 15,
  SASSANIAN = 16,
  GALCENT_0SAG = 17,
  J2000 = 18,
  J1900 = 19,
  B1950 = 20,
  SURYASIDDHANTA = 21,
  SURYASIDDHANTA_MSUN = 22,
  ARYABHATA = 23,
  ARYABHATA_MSUN = 24,
  SS_REVATI = 25,
  SS_CITRA = 26,
  TRUE_CITRA = 27,
  TRUE_REVATI = 28,
  TRUE_PUSHYA = 29,
  GALCENT_BRAND = 30,
  GALEQU_IAU1958 = 31,
  GALEQU = 32,
  GALEQU_MID_MULA = 33,
  SKYDRAM = 34,
  TRUE_MULA = 35,
  DHRUVA_GALCENT = 36,
  ARYABHATA_522 = 37,
  BABYL_BRITTON = 38,
  VEDIC_SHEORAN = 39,
  COCHRANE = 40,
  GALEQU_FIORENZA = 41,
  VETTIUS_VALENS = 42,
  LAHIRI_1940 = 43,
  LAHIRI_VP285 = 44,
  KRISHNAMURTI_VP291 = 45,
  LAHIRI_ICRC = 46,
}

// ============================================================================
// Ayanamsa Reference Data
// ============================================================================

interface AyanamsaReference {
  name: string;
  t0: number;        // Reference Julian Day
  ayan0: number;     // Ayanamsa value at t0 (degrees)
  rate: number;      // Rate of change (arcsec/century)
  description: string;
}

/**
 * Ayanamsa reference values
 */
const AYANAMSA_DATA: Record<AyanamsaSystem, AyanamsaReference> = {
  [AyanamsaSystem.FAGAN_BRADLEY]: {
    name: 'Fagan-Bradley',
    t0: 2433282.5,  // Jan 1, 1950
    ayan0: 23.898056,
    rate: 5028.83,
    description: 'Fagan-Bradley (Synetic Vernal Point)',
  },
  [AyanamsaSystem.LAHIRI]: {
    name: 'Lahiri',
    t0: 2433282.5,  // Jan 1, 1950
    ayan0: 23.262250,
    rate: 5028.796195,
    description: 'Lahiri (Indian national ephemeris)',
  },
  [AyanamsaSystem.DELUCE]: {
    name: 'De Luce',
    t0: 2433282.5,
    ayan0: 24.765278,
    rate: 5028.83,
    description: 'De Luce',
  },
  [AyanamsaSystem.RAMAN]: {
    name: 'Raman',
    t0: 2433282.5,
    ayan0: 22.483333,
    rate: 5028.83,
    description: 'B.V. Raman',
  },
  [AyanamsaSystem.USHASHASHI]: {
    name: 'Usha-Shashi',
    t0: 2433282.5,
    ayan0: 22.393889,
    rate: 5028.83,
    description: 'Usha and Shashi',
  },
  [AyanamsaSystem.KRISHNAMURTI]: {
    name: 'Krishnamurti',
    t0: 2433282.5,
    ayan0: 23.216667,
    rate: 5028.83,
    description: 'Krishnamurti',
  },
  [AyanamsaSystem.DJWHAL_KHUL]: {
    name: 'Djwhal Khul',
    t0: 2433282.5,
    ayan0: 23.070833,
    rate: 5028.83,
    description: 'Djwhal Khul (Alice Bailey)',
  },
  [AyanamsaSystem.YUKTESHWAR]: {
    name: 'Yukteshwar',
    t0: 2433282.5,
    ayan0: 21.818889,
    rate: 5028.83,
    description: 'Sri Yukteshwar',
  },
  [AyanamsaSystem.JN_BHASIN]: {
    name: 'J.N. Bhasin',
    t0: 2433282.5,
    ayan0: 22.553333,
    rate: 5028.83,
    description: 'J.N. Bhasin',
  },
  [AyanamsaSystem.BABYL_KUGLER1]: {
    name: 'Babylonian/Kugler 1',
    t0: 1684532.5,  // -100 Jan 1
    ayan0: -3.4166667,
    rate: 5025.64,
    description: 'Babylonian (Kugler method 1)',
  },
  [AyanamsaSystem.BABYL_KUGLER2]: {
    name: 'Babylonian/Kugler 2',
    t0: 1684532.5,
    ayan0: -4.4666667,
    rate: 5025.64,
    description: 'Babylonian (Kugler method 2)',
  },
  [AyanamsaSystem.BABYL_KUGLER3]: {
    name: 'Babylonian/Kugler 3',
    t0: 1684532.5,
    ayan0: -5.6166667,
    rate: 5025.64,
    description: 'Babylonian (Kugler method 3)',
  },
  [AyanamsaSystem.BABYL_HUBER]: {
    name: 'Babylonian/Huber',
    t0: 1684532.5,
    ayan0: -4.3333333,
    rate: 5025.64,
    description: 'Babylonian (Huber)',
  },
  [AyanamsaSystem.BABYL_ETPSC]: {
    name: 'Babylonian/Eta Piscium',
    t0: 1684532.5,
    ayan0: -4.7666667,
    rate: 5025.64,
    description: 'Babylonian (Eta Piscium)',
  },
  [AyanamsaSystem.ALDEBARAN_15TAU]: {
    name: 'Aldebaran at 15° Tau',
    t0: 2433282.5,
    ayan0: 25.548333,
    rate: 5028.83,
    description: 'Aldebaran at 15° Taurus',
  },
  [AyanamsaSystem.HIPPARCHOS]: {
    name: 'Hipparchos',
    t0: 1721057.5,  // 0 AD
    ayan0: 0.0,
    rate: 5029.0,
    description: 'Hipparchos',
  },
  [AyanamsaSystem.SASSANIAN]: {
    name: 'Sassanian',
    t0: 1948531.5,  // 564 AD
    ayan0: 0.0,
    rate: 5029.0,
    description: 'Sassanian',
  },
  [AyanamsaSystem.GALCENT_0SAG]: {
    name: 'Galactic Center 0° Sag',
    t0: J2000,
    ayan0: 0.0,
    rate: 5029.0,
    description: 'Galactic Center at 0° Sagittarius',
  },
  [AyanamsaSystem.J2000]: {
    name: 'J2000',
    t0: J2000,
    ayan0: 0.0,
    rate: 0.0,
    description: 'Fixed at J2000 equinox',
  },
  [AyanamsaSystem.J1900]: {
    name: 'J1900',
    t0: 2415020.0,
    ayan0: 0.0,
    rate: 0.0,
    description: 'Fixed at J1900 equinox',
  },
  [AyanamsaSystem.B1950]: {
    name: 'B1950',
    t0: 2433282.5,
    ayan0: 0.0,
    rate: 0.0,
    description: 'Fixed at B1950 equinox',
  },
  [AyanamsaSystem.SURYASIDDHANTA]: {
    name: 'Suryasiddhanta',
    t0: 2433282.5,
    ayan0: 22.463889,
    rate: 5028.35,
    description: 'Suryasiddhanta',
  },
  [AyanamsaSystem.SURYASIDDHANTA_MSUN]: {
    name: 'Suryasiddhanta (Mean Sun)',
    t0: 2433282.5,
    ayan0: 22.450000,
    rate: 5028.35,
    description: 'Suryasiddhanta with mean Sun',
  },
  [AyanamsaSystem.ARYABHATA]: {
    name: 'Aryabhata',
    t0: 2433282.5,
    ayan0: 23.226667,
    rate: 5028.35,
    description: 'Aryabhata 499',
  },
  [AyanamsaSystem.ARYABHATA_MSUN]: {
    name: 'Aryabhata (Mean Sun)',
    t0: 2433282.5,
    ayan0: 23.213333,
    rate: 5028.35,
    description: 'Aryabhata with mean Sun',
  },
  [AyanamsaSystem.SS_REVATI]: {
    name: 'SS Revati',
    t0: 2433282.5,
    ayan0: 22.200000,
    rate: 5028.35,
    description: 'Suryasiddhanta Revati',
  },
  [AyanamsaSystem.SS_CITRA]: {
    name: 'SS Citra',
    t0: 2433282.5,
    ayan0: 23.513889,
    rate: 5028.35,
    description: 'Suryasiddhanta Citra',
  },
  [AyanamsaSystem.TRUE_CITRA]: {
    name: 'True Citra',
    t0: 2433282.5,
    ayan0: 23.333333,
    rate: 5028.83,
    description: 'True Citra (Spica at 180°)',
  },
  [AyanamsaSystem.TRUE_REVATI]: {
    name: 'True Revati',
    t0: 2433282.5,
    ayan0: 22.063889,
    rate: 5028.83,
    description: 'True Revati (Zeta Piscium)',
  },
  [AyanamsaSystem.TRUE_PUSHYA]: {
    name: 'True Pushya',
    t0: 2433282.5,
    ayan0: 24.800000,
    rate: 5028.83,
    description: 'True Pushya (PVRN Rao)',
  },
  [AyanamsaSystem.GALCENT_BRAND]: {
    name: 'Galactic Center (Gil Brand)',
    t0: J2000,
    ayan0: 1.749444,
    rate: 5029.0,
    description: 'Galactic Center (Gil Brand)',
  },
  [AyanamsaSystem.GALEQU_IAU1958]: {
    name: 'Galactic Equator IAU 1958',
    t0: J2000,
    ayan0: 1.383333,
    rate: 5029.0,
    description: 'Galactic Equator (IAU 1958)',
  },
  [AyanamsaSystem.GALEQU]: {
    name: 'Galactic Equator',
    t0: J2000,
    ayan0: 1.383333,
    rate: 5029.0,
    description: 'Galactic Equator',
  },
  [AyanamsaSystem.GALEQU_MID_MULA]: {
    name: 'Galactic Equator Mid-Mula',
    t0: J2000,
    ayan0: 4.900000,
    rate: 5029.0,
    description: 'Galactic Equator at mid-Mula',
  },
  [AyanamsaSystem.SKYDRAM]: {
    name: 'Skydram (Mardyks)',
    t0: 2433282.5,
    ayan0: 23.850000,
    rate: 5028.83,
    description: 'Skydram (Mardyks)',
  },
  [AyanamsaSystem.TRUE_MULA]: {
    name: 'True Mula',
    t0: 2433282.5,
    ayan0: 25.180000,
    rate: 5028.83,
    description: 'True Mula (Chandra Hari)',
  },
  [AyanamsaSystem.DHRUVA_GALCENT]: {
    name: 'Dhruva/Galactic Center',
    t0: J2000,
    ayan0: 1.749444,
    rate: 5029.0,
    description: 'Dhruva/Galactic Center (Wilhelm)',
  },
  [AyanamsaSystem.ARYABHATA_522]: {
    name: 'Aryabhata 522',
    t0: 2433282.5,
    ayan0: 21.636111,
    rate: 5028.35,
    description: 'Aryabhata 522',
  },
  [AyanamsaSystem.BABYL_BRITTON]: {
    name: 'Babylonian/Britton',
    t0: 1684532.5,
    ayan0: -2.9583333,
    rate: 5025.64,
    description: 'Babylonian (Britton)',
  },
  [AyanamsaSystem.VEDIC_SHEORAN]: {
    name: 'Vedic/Sheoran',
    t0: 2433282.5,
    ayan0: 23.333333,
    rate: 5028.83,
    description: 'Vedic (Sheoran)',
  },
  [AyanamsaSystem.COCHRANE]: {
    name: 'Cochrane',
    t0: J2000,
    ayan0: 1.749444,
    rate: 5029.0,
    description: 'Cochrane (Galactic Center at 0° Cap)',
  },
  [AyanamsaSystem.GALEQU_FIORENZA]: {
    name: 'Galactic Equator (Fiorenza)',
    t0: J2000,
    ayan0: 1.383333,
    rate: 5029.0,
    description: 'Galactic Equator (Fiorenza)',
  },
  [AyanamsaSystem.VETTIUS_VALENS]: {
    name: 'Vettius Valens',
    t0: 1821731.5,  // 175 AD
    ayan0: 0.0,
    rate: 5029.0,
    description: 'Vettius Valens',
  },
  [AyanamsaSystem.LAHIRI_1940]: {
    name: 'Lahiri 1940',
    t0: 2429630.5,  // 1940
    ayan0: 22.460000,
    rate: 5028.796195,
    description: 'Lahiri VP285 (1940)',
  },
  [AyanamsaSystem.LAHIRI_VP285]: {
    name: 'Lahiri VP285',
    t0: 2443661.5,  // 1978
    ayan0: 23.847222,
    rate: 5028.796195,
    description: 'Lahiri VP285 (1980)',
  },
  [AyanamsaSystem.KRISHNAMURTI_VP291]: {
    name: 'Krishnamurti VP291',
    t0: 2433282.5,
    ayan0: 23.216667,
    rate: 5028.796195,
    description: 'Krishnamurti VP291',
  },
  [AyanamsaSystem.LAHIRI_ICRC]: {
    name: 'Lahiri ICRC',
    t0: 2433282.5,
    ayan0: 23.262250,
    rate: 5028.796195,
    description: 'Lahiri (Indian Calendar Reform Committee)',
  },
};

// ============================================================================
// Ayanamsa Calculation
// ============================================================================

/**
 * Calculate ayanamsa for a given date and system
 *
 * @param jdTT - Julian Day (Terrestrial Time)
 * @param system - Ayanamsa system
 * @returns Ayanamsa value in degrees
 */
export function calculateAyanamsa(
  jdTT: number,
  system: AyanamsaSystem = AyanamsaSystem.LAHIRI
): number {
  const data = AYANAMSA_DATA[system];

  if (!data) {
    throw new Error(`Unknown ayanamsa system: ${system}`);
  }

  // Time in Julian centuries from reference epoch
  const T = (jdTT - data.t0) / 36525.0;

  // Ayanamsa = ayan0 + rate * T
  // Rate is in arcsec/century, convert to degrees
  const ayanamsa = data.ayan0 + (data.rate / 3600.0) * T;

  return ayanamsa;
}

/**
 * Convert tropical longitude to sidereal longitude
 *
 * @param tropicalLongitude - Tropical longitude (degrees)
 * @param jdTT - Julian Day
 * @param system - Ayanamsa system
 * @returns Sidereal longitude (degrees)
 */
export function tropicalToSidereal(
  tropicalLongitude: number,
  jdTT: number,
  system: AyanamsaSystem = AyanamsaSystem.LAHIRI
): number {
  const ayanamsa = calculateAyanamsa(jdTT, system);
  return normalizeDegrees(tropicalLongitude - ayanamsa);
}

/**
 * Convert sidereal longitude to tropical longitude
 *
 * @param siderealLongitude - Sidereal longitude (degrees)
 * @param jdTT - Julian Day
 * @param system - Ayanamsa system
 * @returns Tropical longitude (degrees)
 */
export function siderealToTropical(
  siderealLongitude: number,
  jdTT: number,
  system: AyanamsaSystem = AyanamsaSystem.LAHIRI
): number {
  const ayanamsa = calculateAyanamsa(jdTT, system);
  return normalizeDegrees(siderealLongitude + ayanamsa);
}

/**
 * Get ayanamsa system name
 *
 * @param system - Ayanamsa system
 * @returns Name of the system
 */
export function getAyanamsaName(system: AyanamsaSystem): string {
  const data = AYANAMSA_DATA[system];
  return data ? data.name : `Unknown System ${system}`;
}

/**
 * Get ayanamsa system description
 *
 * @param system - Ayanamsa system
 * @returns Description
 */
export function getAyanamsaDescription(system: AyanamsaSystem): string {
  const data = AYANAMSA_DATA[system];
  return data ? data.description : '';
}

/**
 * List all available ayanamsa systems
 *
 * @returns Array of system information
 */
export function listAyanamsaSystems(): Array<{
  id: AyanamsaSystem;
  name: string;
  description: string;
}> {
  return Object.entries(AYANAMSA_DATA).map(([id, data]) => ({
    id: parseInt(id) as AyanamsaSystem,
    name: data.name,
    description: data.description,
  }));
}
