/**
 * Swiss Ephemeris TypeScript Component - Constants
 *
 * Based on Swiss Ephemeris C library by Dieter Koch and Alois Treindl
 * Ported to TypeScript as a reusable component
 *
 * @license AGPL-3.0 / Swiss Ephemeris Professional License
 * @see https://www.astro.com/swisseph/
 */

// ============================================================================
// Unit Conversions
// ============================================================================

export const SE_AUNIT_TO_KM = 149597870.700;
export const SE_AUNIT_TO_LIGHTYEAR = 1.0 / 63241.07708427;
export const SE_AUNIT_TO_PARSEC = 1.0 / 206264.8062471;

// ============================================================================
// Calendar Types
// ============================================================================

export enum CalendarType {
  JULIAN = 0,
  GREGORIAN = 1,
}

// ============================================================================
// Planet Numbers
// ============================================================================

export enum Planet {
  ECL_NUT = -1,
  SUN = 0,
  MOON = 1,
  MERCURY = 2,
  VENUS = 3,
  MARS = 4,
  JUPITER = 5,
  SATURN = 6,
  URANUS = 7,
  NEPTUNE = 8,
  PLUTO = 9,
  MEAN_NODE = 10,
  TRUE_NODE = 11,
  MEAN_APOG = 12,
  OSCU_APOG = 13,
  EARTH = 14,
  CHIRON = 15,
  PHOLUS = 16,
  CERES = 17,
  PALLAS = 18,
  JUNO = 19,
  VESTA = 20,
  INTP_APOG = 21,
  INTP_PERG = 22,
}

// ============================================================================
// Fictitious Bodies (Uranian/Hamburger Planets)
// ============================================================================

export enum FictitiousPlanet {
  CUPIDO = 40,
  HADES = 41,
  ZEUS = 42,
  KRONOS = 43,
  APOLLON = 44,
  ADMETOS = 45,
  VULKANUS = 46,
  POSEIDON = 47,
  ISIS = 48,
  NIBIRU = 49,
  HARRINGTON = 50,
  NEPTUNE_LEVERRIER = 51,
  NEPTUNE_ADAMS = 52,
  PLUTO_LOWELL = 53,
  PLUTO_PICKERING = 54,
  VULCAN = 55,
  WHITE_MOON = 56,
  PROSERPINA = 57,
  WALDEMATH = 58,
}

// ============================================================================
// House Cusps and Angles
// ============================================================================

export enum HousePoint {
  ASC = 0,
  MC = 1,
  ARMC = 2,
  VERTEX = 3,
  EQUASC = 4,  // Equatorial Ascendant
  COASC1 = 5,  // Co-Ascendant (W. Koch)
  COASC2 = 6,  // Co-Ascendant (M. Munkasey)
  POLASC = 7,  // Polar Ascendant (M. Munkasey)
}

// ============================================================================
// Calculation Flags
// ============================================================================

export enum CalculationFlag {
  JPLEPH = 1,        // Use JPL ephemeris
  SWIEPH = 2,        // Use Swiss Ephemeris
  MOSEPH = 4,        // Use Moshier ephemeris
  HELCTR = 8,        // Heliocentric position
  TRUEPOS = 16,      // True/geometric position
  J2000 = 32,        // J2000 equinox
  NONUT = 64,        // No nutation
  SPEED3 = 128,      // Speed from 3 positions (deprecated)
  SPEED = 256,       // High precision speed
  NOGDEFL = 512,     // No gravitational deflection
  NOABERR = 1024,    // No aberration
  EQUATORIAL = 2048, // Equatorial coordinates
  XYZ = 4096,        // Cartesian coordinates
  RADIANS = 8192,    // Return angles in radians
  BARYCTR = 16384,   // Barycentric position
  TOPOCTR = 32768,   // Topocentric position
  SIDEREAL = 65536,  // Sidereal mode
  ICRS = 131072,     // ICRS reference frame
  DPSIDEPS_1980 = 262144,   // IAU 1980 nutation
  JPLHOR = 524288,   // JPL Horizons mode
  JPLHOR_APPROX = 1048576,  // JPL Horizons approx
  CENTER_BODY = 2097152,    // Center of body
}

// ============================================================================
// House Systems
// ============================================================================

export enum HouseSystem {
  PLACIDUS = 'P',
  KOCH = 'K',
  PORPHYRIUS = 'O',
  REGIOMONTANUS = 'R',
  CAMPANUS = 'C',
  EQUAL = 'A',
  EQUAL_MC = 'D',
  VEHLOW = 'V',
  WHOLE_SIGN = 'W',
  MERIDIAN = 'X',
  AZIMUTHAL = 'H',
  POLICH_PAGE = 'T',
  ALCABITIUS = 'B',
  MORINUS = 'M',
  KRUSINSKI = 'U',
  GAUQUELIN = 'G',
  APC = 'Y',
  SUNSHINE = 'I',
  SUNSHINE_ALT = 'i',
  PULLEN_SD = 'L',
  PULLEN_SR = 'Q',
  SRIPATI = 'S',
  CARTER = 'F',
  WHOLE_ARIES = 'N',
}

// ============================================================================
// Sidereal Modes (Ayanamsa)
// ============================================================================

export enum SiderealMode {
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
// Eclipse Types
// ============================================================================

export enum EclipseType {
  NONE = 0,
  PARTIAL = 1,
  ANNULAR = 2,
  TOTAL = 3,
  ANNULAR_TOTAL = 4,  // Hybrid
  PENUMBRAL = 5,       // Lunar only
  CENTRAL = 1,
  NON_CENTRAL = 2,
}

// ============================================================================
// Event Types
// ============================================================================

export enum EventType {
  SOLAR_ECLIPSE = 'SOLAR_ECLIPSE',
  LUNAR_ECLIPSE = 'LUNAR_ECLIPSE',
  OCCULTATION = 'OCCULTATION',
  RISE = 'RISE',
  SET = 'SET',
  MERIDIAN_TRANSIT = 'MERIDIAN_TRANSIT',
  HELIACAL_RISING = 'HELIACAL_RISING',
  HELIACAL_SETTING = 'HELIACAL_SETTING',
  EVENING_FIRST = 'EVENING_FIRST',
  MORNING_LAST = 'MORNING_LAST',
}

// ============================================================================
// Planet Name Mappings
// ============================================================================

export const PLANET_NAMES: Record<number, string> = {
  [Planet.SUN]: 'Sun',
  [Planet.MOON]: 'Moon',
  [Planet.MERCURY]: 'Mercury',
  [Planet.VENUS]: 'Venus',
  [Planet.MARS]: 'Mars',
  [Planet.JUPITER]: 'Jupiter',
  [Planet.SATURN]: 'Saturn',
  [Planet.URANUS]: 'Uranus',
  [Planet.NEPTUNE]: 'Neptune',
  [Planet.PLUTO]: 'Pluto',
  [Planet.MEAN_NODE]: 'Mean Node',
  [Planet.TRUE_NODE]: 'True Node',
  [Planet.MEAN_APOG]: 'Mean Apogee',
  [Planet.OSCU_APOG]: 'Osculating Apogee',
  [Planet.EARTH]: 'Earth',
  [Planet.CHIRON]: 'Chiron',
  [Planet.PHOLUS]: 'Pholus',
  [Planet.CERES]: 'Ceres',
  [Planet.PALLAS]: 'Pallas',
  [Planet.JUNO]: 'Juno',
  [Planet.VESTA]: 'Vesta',
};

// ============================================================================
// Default Values
// ============================================================================

export const DEFAULT_EPHEMERIS_PATH = './ephe';
export const DEFAULT_GEOGRAPHIC_POSITION = {
  longitude: 0.0,      // Greenwich
  latitude: 51.5,
  elevation: 0,
};
