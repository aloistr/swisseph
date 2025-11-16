/**
 * Native Swiss Ephemeris Calculator
 *
 * Main calculator class that integrates all core astronomical calculations
 * This is a complete, native TypeScript implementation requiring NO C library files
 *
 * Features:
 * - Planetary positions (VSOP87 simplified theory)
 * - Moon positions (ELP2000 simplified)
 * - House calculations (12+ systems)
 * - Precession and nutation (IAU 2006/2000B)
 * - Coordinate transformations
 * - All calculations in pure TypeScript
 *
 * @license AGPL-3.0
 */

import {
  Planet,
  HouseSystem as HouseSystemEnum,
  SiderealMode,
  CalendarType,
} from '../constants';

import {
  PlanetaryPosition,
  HouseData,
  CalculationOptions,
  EclipticCoordinates,
  EquatorialCoordinates,
  HorizontalCoordinates,
  Result,
  CalculationError,
  BatchCalculationResult,
  GeographicPosition,
} from '../types';

import {
  gregorianToJulianDay,
  utToTT,
  calculateLMST,
} from './date-time';

import {
  calculatePrecessionNutation,
  calculateMeanObliquity,
} from './precession-nutation';

import {
  PlanetID,
  calculatePlanetPosition,
  calculatePlanetPositionWithSpeed,
  calculateSunPosition,
  calculateSunPositionWithSpeed,
} from './planetary-calculator';

import {
  calculateMoonPosition,
  calculateMoonPositionWithSpeed,
  calculateMoonMeanNode,
  calculateMoonMeanApogee,
} from './moon-calculator';

import {
  HouseSystem,
  calculateHouses as coreCalculateHouses,
} from './house-systems';

import {
  eclipticToEquatorial,
  equatorialToHorizontal,
} from './coordinate-transformations';

import {
  AsteroidID,
  calculateAsteroidPosition,
  calculateAsteroidPositionWithSpeed,
} from './asteroid-calculator';

import { normalizeDegrees, julianCenturiesFromJ2000 } from './astronomical-constants';
import { getPlanetName } from '../utils';

// ============================================================================
// Planet Mapping
// ============================================================================

const PLANET_ID_MAP: Record<Planet, PlanetID | null> = {
  [Planet.SUN]: null, // Special case
  [Planet.MOON]: null, // Special case
  [Planet.MERCURY]: PlanetID.MERCURY,
  [Planet.VENUS]: PlanetID.VENUS,
  [Planet.MARS]: PlanetID.MARS,
  [Planet.JUPITER]: PlanetID.JUPITER,
  [Planet.SATURN]: PlanetID.SATURN,
  [Planet.URANUS]: PlanetID.URANUS,
  [Planet.NEPTUNE]: PlanetID.NEPTUNE,
  [Planet.PLUTO]: PlanetID.PLUTO,
  [Planet.MEAN_NODE]: null, // Calculated from Moon
  [Planet.TRUE_NODE]: null, // Not implemented
  [Planet.MEAN_APOG]: null, // Calculated from Moon
  [Planet.OSCU_APOG]: null, // Not implemented
  [Planet.EARTH]: PlanetID.EARTH,
  [Planet.CHIRON]: null, // Asteroid - handled separately
  [Planet.PHOLUS]: null, // Asteroid - handled separately
  [Planet.CERES]: null, // Asteroid - handled separately
  [Planet.PALLAS]: null, // Asteroid - handled separately
  [Planet.JUNO]: null, // Asteroid - handled separately
  [Planet.VESTA]: null, // Asteroid - handled separately
  [Planet.INTP_APOG]: null, // Not implemented
  [Planet.INTP_PERG]: null, // Not implemented
  [Planet.ECL_NUT]: null, // Special case
};

// Asteroid mapping
const ASTEROID_ID_MAP: Record<Planet, AsteroidID | null> = {
  [Planet.CHIRON]: AsteroidID.CHIRON,
  [Planet.PHOLUS]: AsteroidID.PHOLUS,
  [Planet.CERES]: AsteroidID.CERES,
  [Planet.PALLAS]: AsteroidID.PALLAS,
  [Planet.JUNO]: AsteroidID.JUNO,
  [Planet.VESTA]: AsteroidID.VESTA,
} as any;

// ============================================================================
// Native Calculator Class
// ============================================================================

/**
 * Main native calculator class
 *
 * Provides complete astronomical calculations without requiring
 * any Swiss Ephemeris C library files
 */
export class NativeCalculator {
  /**
   * Calculate planetary position
   *
   * @param planet - Planet identifier
   * @param jdTT - Julian Day (Terrestrial Time)
   * @param options - Calculation options
   * @returns Planetary position
   */
  calculatePosition(
    planet: Planet,
    jdTT: number,
    options: CalculationOptions = {}
  ): Result<PlanetaryPosition> {
    try {
      let ecliptic: EclipticCoordinates;
      let name = getPlanetName(planet);

      // Calculate based on planet type
      if (planet === Planet.SUN) {
        const pos = options.includeSpeed
          ? calculateSunPositionWithSpeed(jdTT)
          : calculateSunPosition(jdTT);

        ecliptic = {
          longitude: pos.longitude,
          latitude: pos.latitude,
          distance: pos.distance,
          longitudeSpeed: pos.longitudeSpeed,
          latitudeSpeed: pos.latitudeSpeed,
          distanceSpeed: pos.distanceSpeed,
        };
      } else if (planet === Planet.MOON) {
        const pos = options.includeSpeed
          ? calculateMoonPositionWithSpeed(jdTT)
          : calculateMoonPosition(jdTT);

        ecliptic = {
          longitude: pos.longitude,
          latitude: pos.latitude,
          distance: pos.distance,
          longitudeSpeed: pos.longitudeSpeed,
          latitudeSpeed: pos.latitudeSpeed,
          distanceSpeed: pos.distanceSpeed,
        };
      } else if (planet === Planet.MEAN_NODE) {
        const lon = calculateMoonMeanNode(jdTT);
        ecliptic = {
          longitude: lon,
          latitude: 0,
          distance: 0,
        };
        name = 'Mean Node';
      } else if (planet === Planet.MEAN_APOG) {
        const lon = calculateMoonMeanApogee(jdTT);
        ecliptic = {
          longitude: lon,
          latitude: 0,
          distance: 0,
        };
        name = 'Mean Apogee';
      } else {
        // Check if it's an asteroid
        const asteroidID = ASTEROID_ID_MAP[planet as keyof typeof ASTEROID_ID_MAP];
        if (asteroidID !== undefined && asteroidID !== null) {
          const pos = options.includeSpeed
            ? calculateAsteroidPositionWithSpeed(asteroidID, jdTT)
            : calculateAsteroidPosition(asteroidID, jdTT);

          ecliptic = {
            longitude: pos.longitude,
            latitude: pos.latitude,
            distance: pos.distance,
            longitudeSpeed: pos.longitudeSpeed,
            latitudeSpeed: pos.latitudeSpeed,
            distanceSpeed: pos.distanceSpeed,
          };
        } else {
          // Try as regular planet
          const planetID = PLANET_ID_MAP[planet];
          if (planetID === null) {
            return {
              success: false,
              error: {
                code: 'NOT_IMPLEMENTED',
                message: `Planet ${planet} (${getPlanetName(planet)}) not yet implemented in native calculator`,
              },
            };
          }

          const pos = options.includeSpeed
            ? calculatePlanetPositionWithSpeed(planetID, jdTT)
            : calculatePlanetPosition(planetID, jdTT);

          ecliptic = {
            longitude: pos.longitude,
            latitude: pos.latitude,
            distance: pos.distance,
            longitudeSpeed: pos.longitudeSpeed,
            latitudeSpeed: pos.latitudeSpeed,
            distanceSpeed: pos.distanceSpeed,
          };
        }
      }

      // Calculate equatorial coordinates if needed
      let equatorial: EquatorialCoordinates | undefined;
      const T = julianCenturiesFromJ2000(jdTT);
      const obliquity = calculateMeanObliquity(T);

      const eq = eclipticToEquatorial(
        ecliptic.longitude,
        ecliptic.latitude,
        obliquity
      );

      equatorial = {
        rightAscension: eq.rightAscension,
        declination: eq.declination,
        distance: ecliptic.distance,
      };

      // Calculate horizontal coordinates if geographic position provided
      let horizontal: HorizontalCoordinates | undefined;
      if (options.geoPosition) {
        const jdUT = jdTT; // Simplified - should convert TT to UT
        const lst = calculateLMST(jdUT, options.geoPosition.longitude);

        const hz = equatorialToHorizontal(
          equatorial.rightAscension,
          equatorial.declination,
          lst,
          options.geoPosition.latitude
        );

        horizontal = {
          azimuth: hz.azimuth,
          altitude: hz.altitude,
        };
      }

      const result: PlanetaryPosition = {
        planet,
        name,
        julianDay: jdTT,
        ecliptic,
        equatorial,
        horizontal,
      };

      return {
        success: true,
        data: result,
      };
    } catch (error) {
      return {
        success: false,
        error: {
          code: 'CALCULATION_ERROR',
          message: error instanceof Error ? error.message : 'Unknown error',
          details: error,
        },
      };
    }
  }

  /**
   * Calculate batch positions
   *
   * @param planets - Array of planets
   * @param jdTT - Julian Day (Terrestrial Time)
   * @param options - Calculation options
   * @returns Batch results
   */
  calculateBatch(
    planets: (Planet | number)[],
    jdTT: number,
    options: CalculationOptions = {}
  ): Result<BatchCalculationResult> {
    try {
      const positions: PlanetaryPosition[] = [];
      const errors: Array<{ planet: Planet | number; error: CalculationError }> = [];

      for (const planet of planets) {
        const result = this.calculatePosition(planet as Planet, jdTT, options);

        if (result.success) {
          positions.push(result.data);
        } else {
          errors.push({ planet, error: result.error });
        }
      }

      // Calculate houses if requested
      let houses: HouseData | undefined;
      if (options.geoPosition && options.houseSystem) {
        const housesResult = this.calculateHouses(
          jdTT,
          options.geoPosition,
          options.houseSystem
        );

        if (housesResult.success) {
          houses = housesResult.data;
        }
      }

      return {
        success: true,
        data: {
          julianDay: jdTT,
          positions,
          houses,
          errors: errors.length > 0 ? errors : undefined,
        },
      };
    } catch (error) {
      return {
        success: false,
        error: {
          code: 'BATCH_ERROR',
          message: error instanceof Error ? error.message : 'Unknown error',
          details: error,
        },
      };
    }
  }

  /**
   * Calculate houses
   *
   * @param jdUT - Julian Day (Universal Time)
   * @param geoPosition - Geographic position
   * @param houseSystem - House system
   * @returns House data
   */
  calculateHouses(
    jdUT: number,
    geoPosition: GeographicPosition,
    houseSystem: HouseSystemEnum
  ): Result<HouseData> {
    try {
      const jdTT = utToTT(jdUT);
      const T = julianCenturiesFromJ2000(jdTT);
      const obliquity = calculateMeanObliquity(T);

      // Map HouseSystemEnum to HouseSystem
      const systemMap: Record<string, HouseSystem> = {
        'P': HouseSystem.PLACIDUS,
        'K': HouseSystem.KOCH,
        'O': HouseSystem.PORPHYRIUS,
        'R': HouseSystem.REGIOMONTANUS,
        'C': HouseSystem.CAMPANUS,
        'A': HouseSystem.EQUAL,
        'D': HouseSystem.EQUAL_MC,
        'W': HouseSystem.WHOLE_SIGN,
        'X': HouseSystem.MERIDIAN,
        'H': HouseSystem.AZIMUTHAL,
        'M': HouseSystem.MORINUS,
        'V': HouseSystem.VEHLOW,
      };

      const system = systemMap[houseSystem] || HouseSystem.PLACIDUS;

      const result = coreCalculateHouses(
        jdUT,
        geoPosition.latitude,
        geoPosition.longitude,
        system,
        obliquity
      );

      const houseData: HouseData = {
        system: houseSystem,
        julianDay: jdUT,
        geoPosition,
        cusps: result.cusps,
        ascendant: result.ascendant,
        mc: result.mc,
        armc: result.armc,
        vertex: result.vertex,
        equatorialAscendant: result.equatorialAscendant,
        coAscendantKoch: result.coAscendant,
        coAscendantMunkasey: result.coAscendant,
        polarAscendant: result.polarAscendant,
      };

      return {
        success: true,
        data: houseData,
      };
    } catch (error) {
      return {
        success: false,
        error: {
          code: 'HOUSE_ERROR',
          message: error instanceof Error ? error.message : 'Unknown error',
          details: error,
        },
      };
    }
  }

  /**
   * Calculate house position of a planet
   *
   * @param longitude - Ecliptic longitude
   * @param houses - House data
   * @returns House number (1.0 - 12.999...)
   */
  calculateHousePosition(longitude: number, houses: HouseData): number {
    const normalizedLon = normalizeDegrees(longitude);

    for (let i = 0; i < 12; i++) {
      const cusp1 = houses.cusps[i];
      const cusp2 = houses.cusps[(i + 1) % 12];

      let inHouse = false;
      if (cusp2 > cusp1) {
        inHouse = normalizedLon >= cusp1 && normalizedLon < cusp2;
      } else {
        // Wraps around 0°
        inHouse = normalizedLon >= cusp1 || normalizedLon < cusp2;
      }

      if (inHouse) {
        const houseSize = cusp2 > cusp1 ? cusp2 - cusp1 : 360 - cusp1 + cusp2;
        const posInHouse =
          normalizedLon >= cusp1
            ? normalizedLon - cusp1
            : 360 - cusp1 + normalizedLon;
        const fraction = posInHouse / houseSize;
        return i + 1 + fraction;
      }
    }

    return 1.0; // Default
  }
}

/**
 * Create a singleton instance
 */
export const nativeCalculator = new NativeCalculator();
