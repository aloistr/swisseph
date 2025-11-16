/**
 * Swiss Ephemeris TypeScript Component - Type Definitions
 *
 * Based on Swiss Ephemeris C library by Dieter Koch and Alois Treindl
 * Ported to TypeScript as a reusable component
 *
 * @license AGPL-3.0 / Swiss Ephemeris Professional License
 * @see https://www.astro.com/swisseph/
 */

import { Planet, HouseSystem, SiderealMode, CalendarType, EventType } from './constants';

// ============================================================================
// Date and Time Types
// ============================================================================

/**
 * Date and time representation
 */
export interface DateTime {
  year: number;
  month: number;
  day: number;
  hour?: number;
  minute?: number;
  second?: number;
  calendar?: CalendarType;
}

/**
 * Julian Day Number (JDN) representation
 */
export interface JulianDay {
  jd: number;
  isUT?: boolean;  // Universal Time vs Ephemeris Time
}

// ============================================================================
// Geographic Position
// ============================================================================

/**
 * Geographic position for topocentric or house calculations
 */
export interface GeographicPosition {
  longitude: number;   // Decimal degrees, positive East, negative West
  latitude: number;    // Decimal degrees, positive North, negative South
  elevation?: number;  // Meters above sea level
}

// ============================================================================
// Calculation Options
// ============================================================================

/**
 * Options for planetary calculations
 */
export interface CalculationOptions {
  /** Calculation flags (bitwise OR of CalculationFlag values) */
  flags?: number;

  /** Ephemeris files directory path */
  ephemerisPath?: string;

  /** Use sidereal zodiac */
  sidereal?: boolean;

  /** Sidereal mode/ayanamsa */
  siderealMode?: SiderealMode;

  /** Custom ayanamsa settings */
  customAyanamsa?: {
    t0: number;      // Reference Julian Day
    ayan0: number;   // Initial ayanamsa value
    isUT?: boolean;  // Reference date is UT
  };

  /** Geographic position for topocentric calculations */
  geoPosition?: GeographicPosition;

  /** House system */
  houseSystem?: HouseSystem;

  /** Include speed calculations */
  includeSpeed?: boolean;

  /** Return coordinates in radians instead of degrees */
  useRadians?: boolean;

  /** Heliocentric instead of geocentric */
  heliocentric?: boolean;

  /** Barycentric instead of geocentric */
  barycentric?: boolean;

  /** True/geometric position instead of apparent */
  truePosition?: boolean;

  /** J2000 equinox (no precession) */
  j2000?: boolean;

  /** No nutation */
  noNutation?: boolean;

  /** No aberration */
  noAberration?: boolean;

  /** No gravitational deflection */
  noDeflection?: boolean;
}

// ============================================================================
// Coordinate Results
// ============================================================================

/**
 * Ecliptic coordinates
 */
export interface EclipticCoordinates {
  longitude: number;         // Degrees (0-360)
  latitude: number;          // Degrees (-90 to +90)
  distance: number;          // AU
  longitudeSpeed?: number;   // Degrees per day
  latitudeSpeed?: number;    // Degrees per day
  distanceSpeed?: number;    // AU per day
}

/**
 * Equatorial coordinates
 */
export interface EquatorialCoordinates {
  rightAscension: number;    // Degrees (0-360) or Hours (0-24)
  declination: number;       // Degrees (-90 to +90)
  distance: number;          // AU
  raSpeed?: number;          // Degrees per day
  decSpeed?: number;         // Degrees per day
  distanceSpeed?: number;    // AU per day
}

/**
 * Horizontal coordinates (altitude/azimuth)
 */
export interface HorizontalCoordinates {
  azimuth: number;           // Degrees (0-360), from South
  altitude: number;          // Degrees (-90 to +90)
  trueAltitude?: number;     // With atmospheric refraction
}

/**
 * Cartesian coordinates
 */
export interface CartesianCoordinates {
  x: number;
  y: number;
  z: number;
  dx?: number;  // Speed components
  dy?: number;
  dz?: number;
}

/**
 * Complete planetary position result
 */
export interface PlanetaryPosition {
  planet: Planet | number;
  name: string;
  julianDay: number;
  ecliptic: EclipticCoordinates;
  equatorial?: EquatorialCoordinates;
  horizontal?: HorizontalCoordinates;
  cartesian?: CartesianCoordinates;
  housePosition?: number;  // House number (1.0-12.999...)
  phaseAngle?: number;     // Degrees
  phase?: number;          // Illuminated fraction (0-1)
  elongation?: number;     // Degrees from Sun
  apparentDiameter?: number;  // Arc seconds
  magnitude?: number;      // Visual magnitude
}

// ============================================================================
// House Calculations
// ============================================================================

/**
 * House cusps and angles
 */
export interface HouseData {
  system: HouseSystem;
  julianDay: number;
  geoPosition: GeographicPosition;
  cusps: number[];         // Array of 12 house cusps (longitude in degrees)
  ascendant: number;       // Ascendant longitude
  mc: number;              // Midheaven longitude
  armc: number;            // ARMC (sidereal time)
  vertex: number;          // Vertex longitude
  equatorialAscendant: number;
  coAscendantKoch: number;
  coAscendantMunkasey: number;
  polarAscendant: number;
}

// ============================================================================
// Eclipse Calculations
// ============================================================================

/**
 * Solar eclipse data
 */
export interface SolarEclipse {
  type: EventType.SOLAR_ECLIPSE;
  eclipseType: 'partial' | 'annular' | 'total' | 'annular-total';
  julianDayMax: number;
  timeMaximum: DateTime;
  magnitude: number;
  sarosNumber?: number;
  sarosSeriesNumber?: number;
  centralLine?: {
    latitude: number;
    longitude: number;
  };
  coreWidth?: number;      // km (negative for total)
  duration?: number;       // seconds
  phases: {
    partialBegin?: number;
    totalBegin?: number;
    maximum: number;
    totalEnd?: number;
    partialEnd?: number;
  };
  localData?: {
    visible: boolean;
    obscuration: number;
    altitude: number;
  };
}

/**
 * Lunar eclipse data
 */
export interface LunarEclipse {
  type: EventType.LUNAR_ECLIPSE;
  eclipseType: 'penumbral' | 'partial' | 'total';
  julianDayMax: number;
  timeMaximum: DateTime;
  magnitudeUmbral: number;
  magnitudePenumbral: number;
  sarosNumber?: number;
  sarosSeriesNumber?: number;
  phases: {
    penumbralBegin?: number;
    partialBegin?: number;
    totalBegin?: number;
    maximum: number;
    totalEnd?: number;
    partialEnd?: number;
    penumbralEnd?: number;
  };
  zenithPosition: GeographicPosition;
}

/**
 * Occultation data
 */
export interface Occultation {
  type: EventType.OCCULTATION;
  planet: Planet | number;
  julianDayMax: number;
  timeMaximum: DateTime;
  magnitude: number;
  phases: {
    begin?: number;
    maximum: number;
    end?: number;
  };
  localData?: {
    visible: boolean;
    altitude: number;
  };
}

// ============================================================================
// Rise/Set and Transit Calculations
// ============================================================================

/**
 * Rising and setting times
 */
export interface RiseSetData {
  rise?: {
    julianDay: number;
    dateTime: DateTime;
    azimuth: number;
  };
  set?: {
    julianDay: number;
    dateTime: DateTime;
    azimuth: number;
  };
  transitUpper?: {
    julianDay: number;
    dateTime: DateTime;
    altitude: number;
  };
  transitLower?: {
    julianDay: number;
    dateTime: DateTime;
    altitude: number;
  };
}

/**
 * Heliacal phenomena
 */
export interface HeliacalEvent {
  type: EventType;
  planet: Planet | number;
  julianDay: number;
  dateTime: DateTime;
  visibility: number;
}

// ============================================================================
// Orbital Elements
// ============================================================================

/**
 * Osculating orbital elements
 */
export interface OrbitalElements {
  semimajorAxis: number;           // AU
  eccentricity: number;
  inclination: number;             // Degrees
  ascendingNode: number;           // Degrees
  argumentOfPerihelion: number;    // Degrees
  meanAnomaly: number;             // Degrees
  trueAnomaly: number;             // Degrees
  perihelionDistance: number;      // AU
  aphelionDistance: number;        // AU
  meanDailyMotion: number;         // Degrees per day
  siderealPeriod: number;          // Days
  timePerihelion: number;          // Julian Day
}

// ============================================================================
// Fixed Stars
// ============================================================================

/**
 * Fixed star position
 */
export interface FixedStarPosition {
  name: string;
  nomenclature: string;
  julianDay: number;
  ecliptic: EclipticCoordinates;
  equatorial: EquatorialCoordinates;
  magnitude: number;
}

// ============================================================================
// Nodes and Apsides
// ============================================================================

/**
 * Planetary nodes
 */
export interface NodesData {
  planet: Planet | number;
  ascending: {
    longitude: number;
    latitude: number;
    distance: number;
  };
  descending: {
    longitude: number;
    latitude: number;
    distance: number;
  };
  type: 'mean' | 'osculating';
}

/**
 * Planetary apsides
 */
export interface ApsidesData {
  planet: Planet | number;
  perihelion: {
    longitude: number;
    latitude: number;
    distance: number;
  };
  aphelion: {
    longitude: number;
    latitude: number;
    distance: number;
  };
  secondFocus: {
    longitude: number;
    latitude: number;
    distance: number;
  };
  type: 'mean' | 'osculating';
}

// ============================================================================
// Error Handling
// ============================================================================

/**
 * Calculation error
 */
export interface CalculationError {
  code: string;
  message: string;
  details?: unknown;
}

/**
 * Result type with error handling
 */
export type Result<T> = {
  success: true;
  data: T;
} | {
  success: false;
  error: CalculationError;
};

// ============================================================================
// Atmospheric Conditions (for heliacal events)
// ============================================================================

/**
 * Atmospheric conditions for visibility calculations
 */
export interface AtmosphericConditions {
  pressure: number;          // hPa
  temperature: number;       // Celsius
  relativeHumidity: number;  // Percentage (0-100)
  visualRange?: number;      // km or atmospheric coefficient
}

/**
 * Observer characteristics for visibility calculations
 */
export interface ObserverData {
  age: number;              // Years
  snellenRatio: number;     // Vision acuity (1.0 = normal)
  binocular: boolean;
  magnification: number;    // 1.0 for naked eye
  aperture: number;         // mm, 0 for naked eye
  transmission: number;     // 0-1, 0 for naked eye
}

// ============================================================================
// Calculation Batch Operations
// ============================================================================

/**
 * Request for multiple planet calculations
 */
export interface BatchCalculationRequest {
  planets: (Planet | number)[];
  julianDay: number;
  options?: CalculationOptions;
}

/**
 * Batch calculation results
 */
export interface BatchCalculationResult {
  julianDay: number;
  positions: PlanetaryPosition[];
  houses?: HouseData;
  errors?: Array<{
    planet: Planet | number;
    error: CalculationError;
  }>;
}
