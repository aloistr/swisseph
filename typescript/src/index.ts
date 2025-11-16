/**
 * Swiss Ephemeris TypeScript Component
 *
 * A modern, type-safe TypeScript wrapper for Swiss Ephemeris astronomical calculations.
 *
 * Based on the Swiss Ephemeris C library by Dieter Koch and Alois Treindl
 * Original source: swetest.c and swephexp.h
 *
 * @license AGPL-3.0 / Swiss Ephemeris Professional License
 * @see https://www.astro.com/swisseph/
 *
 * @example
 * ```typescript
 * import { SwissEphemeris, Planet, HouseSystem } from '@swiss-ephemeris/core';
 *
 * const swisseph = new SwissEphemeris();
 *
 * // Calculate Sun position for current date
 * const result = await swisseph.calculatePosition(Planet.SUN, currentJD);
 * if (result.success) {
 *   console.log(result.data.ecliptic.longitude);
 * }
 *
 * // Calculate houses
 * const houses = await swisseph.calculateHouses(
 *   currentJD,
 *   { longitude: -74.0060, latitude: 40.7128, elevation: 10 },
 *   HouseSystem.PLACIDUS
 * );
 * ```
 */

// ============================================================================
// Main Classes
// ============================================================================

export { SwissEphemeris, createSwissEphemeris } from './swiss-ephemeris';

// ============================================================================
// Types and Interfaces
// ============================================================================

export type {
  // Date/Time
  DateTime,
  JulianDay,

  // Geographic
  GeographicPosition,

  // Calculation Options
  CalculationOptions,

  // Coordinates
  EclipticCoordinates,
  EquatorialCoordinates,
  HorizontalCoordinates,
  CartesianCoordinates,
  PlanetaryPosition,

  // Houses
  HouseData,

  // Eclipses and Events
  SolarEclipse,
  LunarEclipse,
  Occultation,
  RiseSetData,
  HeliacalEvent,

  // Orbital
  OrbitalElements,
  NodesData,
  ApsidesData,

  // Fixed Stars
  FixedStarPosition,

  // Atmospheric
  AtmosphericConditions,
  ObserverData,

  // Batch Operations
  BatchCalculationRequest,
  BatchCalculationResult,

  // Results
  Result,
  CalculationError,
} from './types';

// ============================================================================
// Constants and Enums
// ============================================================================

export {
  // Unit Conversions
  SE_AUNIT_TO_KM,
  SE_AUNIT_TO_LIGHTYEAR,
  SE_AUNIT_TO_PARSEC,

  // Enums
  CalendarType,
  Planet,
  FictitiousPlanet,
  HousePoint,
  CalculationFlag,
  HouseSystem,
  SiderealMode,
  EclipseType,
  EventType,

  // Mappings
  PLANET_NAMES,

  // Defaults
  DEFAULT_EPHEMERIS_PATH,
  DEFAULT_GEOGRAPHIC_POSITION,
} from './constants';

// ============================================================================
// Utility Functions
// ============================================================================

export {
  // Date/Time Conversions
  dateTimeToJulianDay,
  julianDayToDateTime,
  getCurrentJulianDay,
  calculateDeltaT,
  utToET,
  etToUT,

  // Angular Utilities
  normalizeAngle,
  degreesToRadians,
  radiansToDegrees,
  decimalToDMS,
  dmsToDecimal,
  formatDMS,
  formatLongitudeZodiac,
  formatRightAscension,

  // Geographic
  formatGeographicPosition,
  validateGeographicPosition,

  // Distance Conversions
  auToKm,
  auToLightYears,
  kmToAU,

  // Planet Names
  getPlanetName,
  parsePlanet,

  // Date Formatting
  formatDateTime,
  parseDateTime,

  // Time Utilities
  calculateSiderealTime,
  calculateLocalSiderealTime,

  // Validation
  validateJulianDay,
  validateDateTime,

  // Aspects
  calculateAspect,
  isInAspect,
} from './utils';

// ============================================================================
// Version Information
// ============================================================================

export const VERSION = '1.0.0';
export const BASED_ON_SWISSEPH_VERSION = '2.10';

// ============================================================================
// Quick Start Examples
// ============================================================================

/**
 * Quick example: Calculate planetary positions
 *
 * @example
 * ```typescript
 * import { createQuickChart } from '@swiss-ephemeris/core';
 *
 * const chart = await createQuickChart({
 *   year: 2024,
 *   month: 1,
 *   day: 1,
 *   hour: 12,
 *   minute: 0,
 *   longitude: 0,
 *   latitude: 51.5,
 * });
 *
 * console.log(chart.planets);
 * console.log(chart.houses);
 * ```
 */
export async function createQuickChart(params: {
  year: number;
  month: number;
  day: number;
  hour?: number;
  minute?: number;
  second?: number;
  longitude: number;
  latitude: number;
  elevation?: number;
  houseSystem?: HouseSystem;
}): Promise<any> {
  const { SwissEphemeris } = await import('./swiss-ephemeris');
  const { Planet, HouseSystem } = await import('./constants');
  const { dateTimeToJulianDay, utToET } = await import('./utils');

  const swisseph = new SwissEphemeris();

  const dateTime = {
    year: params.year,
    month: params.month,
    day: params.day,
    hour: params.hour || 0,
    minute: params.minute || 0,
    second: params.second || 0,
  };

  const jd = dateTimeToJulianDay(dateTime);
  const julianDayET = utToET(jd.jd);

  const geoPosition = {
    longitude: params.longitude,
    latitude: params.latitude,
    elevation: params.elevation || 0,
  };

  const planets = [
    Planet.SUN,
    Planet.MOON,
    Planet.MERCURY,
    Planet.VENUS,
    Planet.MARS,
    Planet.JUPITER,
    Planet.SATURN,
    Planet.URANUS,
    Planet.NEPTUNE,
    Planet.PLUTO,
  ];

  const batchResult = await swisseph.calculateBatch({
    planets,
    julianDay: julianDayET,
    options: {
      geoPosition,
      houseSystem: params.houseSystem || HouseSystem.PLACIDUS,
      includeSpeed: true,
    },
  });

  return batchResult.success ? batchResult.data : null;
}
