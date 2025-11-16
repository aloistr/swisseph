/**
 * Swiss Ephemeris - Native TypeScript Implementation
 *
 * Complete astronomical calculation library in pure TypeScript
 * NO C library files required!
 *
 * This implementation provides:
 *
 * **Core Calculations:**
 * - Planetary positions (Sun through Pluto) - VSOP87 simplified theory
 * - Moon positions - ELP2000 simplified (~60 periodic terms)
 * - Asteroids & minor planets (22+) - Chiron, Ceres, Pallas, Juno, Vesta, etc.
 * - 12+ house systems (Placidus, Koch, Whole Sign, Equal, etc.)
 * - Precession and nutation (IAU 2006/2000B)
 * - Coordinate transformations (Ecliptic, Equatorial, Horizontal)
 *
 * **Fixed Stars:**
 * - 30+ bright and astrologically significant stars
 * - Proper motion corrections
 * - Search by name, constellation, or magnitude
 *
 * **Eclipses:**
 * - Solar eclipses (Total, Partial, Annular, Hybrid)
 * - Lunar eclipses (Total, Partial, Penumbral)
 * - Eclipse magnitude, gamma, duration calculations
 *
 * **Rise/Set/Transit:**
 * - Sun/Moon rise, set, and meridian transit times
 * - Twilight calculations (Civil, Nautical, Astronomical)
 * - Circumpolar and never-rising detection
 * - Azimuth at rise/set
 *
 * **Sidereal Zodiac:**
 * - 47 complete ayanamsa systems
 * - Lahiri, Fagan-Bradley, Krishnamurti, and more
 * - Tropical ↔ Sidereal conversions
 *
 * **Accuracy:** ~1-10 arcminutes for modern era (1900-2100)
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

import {
  calculateFixedStarPosition,
  getFixedStarByName,
  listFixedStars,
  getBrightestStars,
  getStarsInConstellation,
  FixedStarData,
} from './core/fixed-stars';

import {
  findNextSolarEclipse,
  findNextLunarEclipse,
  findEclipsesInYear,
  SolarEclipse,
  LunarEclipse,
  EclipseType as CoreEclipseType,
} from './core/eclipse-calculator';

import {
  calculateSunRiseSetTransit,
  calculateMoonRiseSetTransit,
  calculateTwilight,
  RiseSetTransitData,
  STANDARD_ALTITUDES,
} from './core/rise-set-transit';

import {
  AyanamsaSystem,
  calculateAyanamsa,
  tropicalToSidereal,
  siderealToTropical,
  getAyanamsaName,
  listAyanamsaSystems,
} from './core/sidereal-calculator';

import { normalizeDegrees } from './core/astronomical-constants';

/**
 * Convert SiderealMode enum to AyanamsaSystem enum
 * These enums have the same numeric values, but we validate for safety
 */
function siderealModeToAyanamsaSystem(mode: SiderealMode): AyanamsaSystem {
  // Direct numeric cast is safe as the enums have matching values
  const ayanamsa = mode as number as AyanamsaSystem;

  // Validate it's within range
  if (ayanamsa < 0 || ayanamsa > 46) {
    throw new Error(`Invalid sidereal mode: ${mode}`);
  }

  return ayanamsa;
}

/**
 * Swiss Ephemeris - Native TypeScript Implementation
 *
 * Usage:
 * ```typescript
 * const swisseph = new SwissEphemerisNative();
 *
 * // Calculate planetary positions (including asteroids)
 * const sun = await swisseph.calculatePosition(Planet.SUN, julianDay);
 * const chiron = await swisseph.calculatePosition(Planet.CHIRON, julianDay);
 *
 * // Calculate with date
 * const moon = await swisseph.calculatePositionForDate(
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
 *
 * // Fixed stars
 * const sirius = await swisseph.calculateFixedStar('Sirius', julianDay);
 *
 * // Eclipses
 * const nextSolarEclipse = await swisseph.findNextSolarEclipse(julianDay);
 * const allEclipses2024 = await swisseph.findEclipsesInYear(2024);
 *
 * // Rise/Set/Transit
 * const sunrise = await swisseph.calculateSunRiseSetTransit(
 *   { year: 2024, month: 6, day: 21 },
 *   { latitude: 51.5, longitude: 0 }
 * );
 *
 * // Sidereal zodiac
 * const ayanamsa = swisseph.calculateAyanamsa(julianDay, AyanamsaSystem.LAHIRI);
 * const sidereal = swisseph.tropicalToSidereal(120.5, julianDay);
 * ```
 */
export class SwissEphemerisNative {
  private sidereal: boolean = false;
  private siderealMode: SiderealMode = SiderealMode.FAGAN_BRADLEY;

  /**
   * Create a new SwissEphemerisNative instance
   *
   * @param options - Configuration options
   * @param options.sidereal - Use sidereal zodiac (default: false, tropical)
   * @param options.siderealMode - Ayanamsa system to use (default: Fagan-Bradley)
   *
   * @example
   * ```typescript
   * // Tropical zodiac (default)
   * const tropical = new SwissEphemerisNative();
   *
   * // Sidereal zodiac with Lahiri ayanamsa
   * const sidereal = new SwissEphemerisNative({
   *   sidereal: true,
   *   siderealMode: SiderealMode.LAHIRI
   * });
   * ```
   */
  constructor(options: {
    sidereal?: boolean;
    siderealMode?: SiderealMode;
  } = {}) {
    this.sidereal = options.sidereal || false;
    this.siderealMode = options.siderealMode || SiderealMode.FAGAN_BRADLEY;
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
    const result = nativeCalculator.calculatePosition(planet as Planet, julianDay, options);

    // Apply sidereal conversion if enabled
    if (result.success && this.sidereal && result.data.ecliptic) {
      const ayanamsa = calculateAyanamsa(
        julianDay,
        siderealModeToAyanamsaSystem(this.siderealMode)
      );

      // Convert tropical longitude to sidereal
      result.data.ecliptic.longitude = normalizeDegrees(
        result.data.ecliptic.longitude - ayanamsa
      );
    }

    return result;
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

  // ==========================================================================
  // Fixed Stars
  // ==========================================================================

  /**
   * Calculate fixed star position
   *
   * @param starName - Star name (e.g., 'Sirius', 'Regulus')
   * @param julianDay - Julian Day (TT)
   * @param applyPrecession - Apply precession to date (default: true)
   * @returns Star position with ecliptic and equatorial coordinates
   */
  async calculateFixedStar(
    starName: string,
    julianDay: number,
    applyPrecession: boolean = true
  ): Promise<Result<{
    star: FixedStarData;
    position: {
      longitude: number;
      latitude: number;
      rightAscension: number;
      declination: number;
      distance: number;
    };
  }>> {
    try {
      const star = getFixedStarByName(starName);
      if (!star) {
        return {
          success: false,
          error: {
            code: 'NOT_FOUND',
            message: `Star '${starName}' not found in catalog`,
          },
        };
      }

      const position = calculateFixedStarPosition(star, julianDay, applyPrecession);

      return {
        success: true,
        data: { star, position },
      };
    } catch (error) {
      return {
        success: false,
        error: {
          code: 'CALCULATION_ERROR',
          message: error instanceof Error ? error.message : 'Unknown error',
        },
      };
    }
  }

  /**
   * List all fixed stars in catalog
   */
  listFixedStars(): string[] {
    return listFixedStars();
  }

  /**
   * Get brightest stars
   */
  getBrightestStars(limit: number = 20): FixedStarData[] {
    return getBrightestStars(limit);
  }

  /**
   * Get stars in a constellation
   */
  getStarsInConstellation(constellation: string): FixedStarData[] {
    return getStarsInConstellation(constellation);
  }

  // ==========================================================================
  // Eclipses
  // ==========================================================================

  /**
   * Find next solar eclipse
   *
   * @param julianDayStart - Starting Julian Day to search from
   * @returns Next solar eclipse data
   */
  async findNextSolarEclipse(
    julianDayStart: number
  ): Promise<Result<SolarEclipse | null>> {
    try {
      const eclipse = findNextSolarEclipse(julianDayStart);
      return {
        success: true,
        data: eclipse,
      };
    } catch (error) {
      return {
        success: false,
        error: {
          code: 'CALCULATION_ERROR',
          message: error instanceof Error ? error.message : 'Unknown error',
        },
      };
    }
  }

  /**
   * Find next lunar eclipse
   *
   * @param julianDayStart - Starting Julian Day to search from
   * @returns Next lunar eclipse data
   */
  async findNextLunarEclipse(
    julianDayStart: number
  ): Promise<Result<LunarEclipse | null>> {
    try {
      const eclipse = findNextLunarEclipse(julianDayStart);
      return {
        success: true,
        data: eclipse,
      };
    } catch (error) {
      return {
        success: false,
        error: {
          code: 'CALCULATION_ERROR',
          message: error instanceof Error ? error.message : 'Unknown error',
        },
      };
    }
  }

  /**
   * Find all eclipses in a year
   *
   * @param year - Year to search
   * @returns Array of all eclipses (solar and lunar) in the year
   */
  async findEclipsesInYear(
    year: number
  ): Promise<Result<Array<SolarEclipse | LunarEclipse>>> {
    try {
      const eclipses = findEclipsesInYear(year);
      return {
        success: true,
        data: eclipses,
      };
    } catch (error) {
      return {
        success: false,
        error: {
          code: 'CALCULATION_ERROR',
          message: error instanceof Error ? error.message : 'Unknown error',
        },
      };
    }
  }

  // ==========================================================================
  // Rise/Set/Transit
  // ==========================================================================

  /**
   * Calculate Sun rise, set, and transit times
   *
   * @param dateTime - Date to calculate for
   * @param geoPosition - Observer location
   * @returns Rise/set/transit data
   */
  async calculateSunRiseSetTransit(
    dateTime: DateTime,
    geoPosition: GeographicPosition
  ): Promise<Result<RiseSetTransitData>> {
    try {
      const data = calculateSunRiseSetTransit(
        dateTime.year,
        dateTime.month,
        dateTime.day,
        geoPosition.latitude,
        geoPosition.longitude
      );

      return {
        success: true,
        data,
      };
    } catch (error) {
      return {
        success: false,
        error: {
          code: 'CALCULATION_ERROR',
          message: error instanceof Error ? error.message : 'Unknown error',
        },
      };
    }
  }

  /**
   * Calculate Moon rise, set, and transit times
   *
   * @param dateTime - Date to calculate for
   * @param geoPosition - Observer location
   * @returns Rise/set/transit data
   */
  async calculateMoonRiseSetTransit(
    dateTime: DateTime,
    geoPosition: GeographicPosition
  ): Promise<Result<RiseSetTransitData>> {
    try {
      const data = calculateMoonRiseSetTransit(
        dateTime.year,
        dateTime.month,
        dateTime.day,
        geoPosition.latitude,
        geoPosition.longitude
      );

      return {
        success: true,
        data,
      };
    } catch (error) {
      return {
        success: false,
        error: {
          code: 'CALCULATION_ERROR',
          message: error instanceof Error ? error.message : 'Unknown error',
        },
      };
    }
  }

  /**
   * Calculate twilight times
   *
   * @param dateTime - Date to calculate for
   * @param geoPosition - Observer location
   * @param twilightType - Type of twilight ('civil', 'nautical', 'astronomical')
   * @returns Twilight times
   */
  async calculateTwilight(
    dateTime: DateTime,
    geoPosition: GeographicPosition,
    twilightType: 'civil' | 'nautical' | 'astronomical' = 'civil'
  ): Promise<Result<RiseSetTransitData>> {
    try {
      const data = calculateTwilight(
        dateTime.year,
        dateTime.month,
        dateTime.day,
        geoPosition.latitude,
        geoPosition.longitude,
        twilightType
      );

      return {
        success: true,
        data,
      };
    } catch (error) {
      return {
        success: false,
        error: {
          code: 'CALCULATION_ERROR',
          message: error instanceof Error ? error.message : 'Unknown error',
        },
      };
    }
  }

  // ==========================================================================
  // Sidereal/Ayanamsa
  // ==========================================================================

  /**
   * Calculate ayanamsa value for a given date
   *
   * @param julianDay - Julian Day (TT)
   * @param system - Ayanamsa system (default: uses instance's siderealMode)
   * @returns Ayanamsa value in degrees
   */
  calculateAyanamsa(
    julianDay: number,
    system?: AyanamsaSystem
  ): number {
    const ayanamsaSystem = system !== undefined ? system : siderealModeToAyanamsaSystem(this.siderealMode);
    return calculateAyanamsa(julianDay, ayanamsaSystem);
  }

  /**
   * Convert tropical longitude to sidereal
   *
   * @param tropicalLongitude - Tropical longitude in degrees
   * @param julianDay - Julian Day (TT)
   * @param system - Ayanamsa system (default: uses instance's siderealMode)
   * @returns Sidereal longitude in degrees
   */
  tropicalToSidereal(
    tropicalLongitude: number,
    julianDay: number,
    system?: AyanamsaSystem
  ): number {
    const ayanamsaSystem = system !== undefined ? system : siderealModeToAyanamsaSystem(this.siderealMode);
    return tropicalToSidereal(tropicalLongitude, julianDay, ayanamsaSystem);
  }

  /**
   * Convert sidereal longitude to tropical
   *
   * @param siderealLongitude - Sidereal longitude in degrees
   * @param julianDay - Julian Day (TT)
   * @param system - Ayanamsa system (default: uses instance's siderealMode)
   * @returns Tropical longitude in degrees
   */
  siderealToTropical(
    siderealLongitude: number,
    julianDay: number,
    system?: AyanamsaSystem
  ): number {
    const ayanamsaSystem = system !== undefined ? system : siderealModeToAyanamsaSystem(this.siderealMode);
    return siderealToTropical(siderealLongitude, julianDay, ayanamsaSystem);
  }

  /**
   * Get name of an ayanamsa system
   */
  getAyanamsaName(system: AyanamsaSystem): string {
    return getAyanamsaName(system);
  }

  /**
   * List all available ayanamsa systems
   */
  listAyanamsaSystems(): Array<{ id: AyanamsaSystem; name: string }> {
    return listAyanamsaSystems();
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

// Export additional types for convenience
export type {
  FixedStarData,
  SolarEclipse,
  LunarEclipse,
  RiseSetTransitData,
};

export { AyanamsaSystem, CoreEclipseType as EclipseType, STANDARD_ALTITUDES };
