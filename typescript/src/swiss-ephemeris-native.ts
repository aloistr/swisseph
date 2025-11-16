/**
 * Swiss Ephemeris - Native TypeScript Implementation
 *
 * Complete astronomical calculation library in pure TypeScript
 * NO C library files required!
 *
 * This implementation provides:
 * - Planetary positions (simplified VSOP87 theory)
 * - Moon positions (simplified ELP2000)
 * - 12+ house systems
 * - Precession and nutation (IAU 2006/2000B)
 * - Coordinate transformations
 * - All calculations in native TypeScript
 *
 * Accuracy: ~1-10 arcminutes for modern era (1900-2100)
 * For higher precision, use the full Swiss Ephemeris C library via WebAssembly
 *
 * @license AGPL-3.0
 */

import {
  Planet,
  HouseSystem,
  SiderealMode,
} from './constants';

import {
  DateTime,
  PlanetaryPosition,
  HouseData,
  CalculationOptions,
  Result,
  BatchCalculationRequest,
  BatchCalculationResult,
  GeographicPosition,
} from './types';

import {
  dateTimeToJulianDay,
  getCurrentJulianDay,
  utToTT,
} from './utils';

import { nativeCalculator } from './core/native-calculator';

/**
 * Swiss Ephemeris - Native TypeScript Implementation
 *
 * Usage:
 * ```typescript
 * const swisseph = new SwissEphemerisNative();
 *
 * // Calculate Sun position
 * const result = await swisseph.calculatePosition(Planet.SUN, julianDay);
 *
 * // Calculate with date
 * const result2 = await swisseph.calculatePositionForDate(
 *   Planet.MOON,
 *   { year: 2024, month: 1, day: 1, hour: 12 }
 * );
 *
 * // Calculate houses
 * const houses = await swisseph.calculateHouses(
 *   julianDay,
 *   { longitude: -74.0060, latitude: 40.7128, elevation: 10 },
 *   HouseSystem.PLACIDUS
 * );
 * ```
 */
export class SwissEphemerisNative {
  private sidereal: boolean = false;
  private siderealMode: SiderealMode = SiderealMode.FAGAN_BRADLEY;

  /**
   * Create a new SwissEphemerisNative instance
   *
   * @param options - Configuration options
   */
  constructor(options: {
    sidereal?: boolean;
    siderealMode?: SiderealMode;
  } = {}) {
    this.sidereal = options.sidereal || false;
    this.siderealMode = options.siderealMode || SiderealMode.FAGAN_BRADLEY;

    if (this.sidereal) {
      console.warn('Sidereal mode requested but not yet fully implemented in native calculator');
    }
  }

  // ==========================================================================
  // Core Calculation Methods
  // ==========================================================================

  /**
   * Calculate planetary position for a given Julian Day
   *
   * @param planet - Planet or celestial body
   * @param julianDay - Julian Day number (Terrestrial Time)
   * @param options - Calculation options
   * @returns Planetary position result
   */
  async calculatePosition(
    planet: Planet | number,
    julianDay: number,
    options: CalculationOptions = {}
  ): Promise<Result<PlanetaryPosition>> {
    return nativeCalculator.calculatePosition(planet as Planet, julianDay, options);
  }

  /**
   * Calculate planetary position for a given date/time
   *
   * @param planet - Planet or celestial body
   * @param dateTime - Date and time
   * @param options - Calculation options
   * @returns Planetary position result
   */
  async calculatePositionForDate(
    planet: Planet | number,
    dateTime: DateTime,
    options: CalculationOptions = {}
  ): Promise<Result<PlanetaryPosition>> {
    const jd = dateTimeToJulianDay(dateTime);
    const julianDayTT = utToTT(jd.jd);
    return this.calculatePosition(planet, julianDayTT, options);
  }

  /**
   * Calculate positions for multiple planets at once
   *
   * @param request - Batch calculation request
   * @returns Batch calculation results
   */
  async calculateBatch(
    request: BatchCalculationRequest
  ): Promise<Result<BatchCalculationResult>> {
    return nativeCalculator.calculateBatch(
      request.planets,
      request.julianDay,
      request.options
    );
  }

  // ==========================================================================
  // House Calculations
  // ==========================================================================

  /**
   * Calculate house cusps and angles
   *
   * @param julianDay - Julian Day (UT)
   * @param geoPosition - Geographic position
   * @param houseSystem - House system to use
   * @returns House data
   */
  async calculateHouses(
    julianDay: number,
    geoPosition: GeographicPosition,
    houseSystem: HouseSystem = HouseSystem.PLACIDUS
  ): Promise<Result<HouseData>> {
    return nativeCalculator.calculateHouses(julianDay, geoPosition, houseSystem);
  }

  /**
   * Calculate house position of a planet
   *
   * @param longitude - Ecliptic longitude of planet
   * @param latitude - Ecliptic latitude of planet
   * @param houses - House data
   * @returns House position (1.0 - 12.999...)
   */
  calculateHousePosition(
    longitude: number,
    latitude: number,
    houses: HouseData
  ): number {
    return nativeCalculator.calculateHousePosition(longitude, houses);
  }

  // ==========================================================================
  // Convenience Methods
  // ==========================================================================

  /**
   * Get current planetary positions
   *
   * @param planets - Array of planets to calculate
   * @param options - Calculation options
   * @returns Current positions
   */
  async getCurrentPositions(
    planets: (Planet | number)[],
    options: CalculationOptions = {}
  ): Promise<Result<BatchCalculationResult>> {
    const jd = getCurrentJulianDay();
    const julianDayTT = utToTT(jd.jd);

    return this.calculateBatch({
      planets,
      julianDay: julianDayTT,
      options,
    });
  }

  /**
   * Calculate a complete birth chart
   *
   * @param dateTime - Birth date and time
   * @param geoPosition - Birth location
   * @param houseSystem - House system
   * @returns Complete chart data
   */
  async calculateBirthChart(
    dateTime: DateTime,
    geoPosition: GeographicPosition,
    houseSystem: HouseSystem = HouseSystem.PLACIDUS
  ): Promise<Result<BatchCalculationResult>> {
    const jd = dateTimeToJulianDay(dateTime);
    const julianDayTT = utToTT(jd.jd);

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
      Planet.MEAN_NODE,
      Planet.MEAN_APOG,
    ];

    return this.calculateBatch({
      planets,
      julianDay: julianDayTT,
      options: {
        geoPosition,
        houseSystem,
        includeSpeed: true,
      },
    });
  }
}

/**
 * Create a default instance for convenience
 */
export function createNativeSwissEphemeris(options?: {
  sidereal?: boolean;
  siderealMode?: SiderealMode;
}): SwissEphemerisNative {
  return new SwissEphemerisNative(options);
}
