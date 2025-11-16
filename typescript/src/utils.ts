/**
 * Swiss Ephemeris TypeScript Component - Utility Functions
 *
 * Based on Swiss Ephemeris C library by Dieter Koch and Alois Treindl
 * Ported to TypeScript as a reusable component
 *
 * @license AGPL-3.0 / Swiss Ephemeris Professional License
 * @see https://www.astro.com/swisseph/
 */

import { DateTime, JulianDay, GeographicPosition } from './types';
import { CalendarType, PLANET_NAMES } from './constants';

// ============================================================================
// Date/Time Conversion Utilities
// ============================================================================

/**
 * Convert calendar date to Julian Day Number
 * Based on swe_julday() from Swiss Ephemeris
 */
export function dateTimeToJulianDay(
  dateTime: DateTime,
  isUT: boolean = true
): JulianDay {
  const {
    year,
    month,
    day,
    hour = 0,
    minute = 0,
    second = 0,
    calendar = CalendarType.GREGORIAN,
  } = dateTime;

  let y = year;
  let m = month;

  // Algorithm: convert month to range 3-14 (March = 3, Feb = 14)
  if (m <= 2) {
    y -= 1;
    m += 12;
  }

  // Determine if Gregorian or Julian calendar
  let b = 0;
  if (calendar === CalendarType.GREGORIAN) {
    // Gregorian calendar correction
    const a = Math.floor(y / 100);
    b = 2 - a + Math.floor(a / 4);
  }

  // Calculate Julian Day
  const jd =
    Math.floor(365.25 * (y + 4716)) +
    Math.floor(30.6001 * (m + 1)) +
    day +
    b -
    1524.5 +
    (hour + minute / 60 + second / 3600) / 24;

  return { jd, isUT };
}

/**
 * Convert Julian Day Number to calendar date
 * Based on swe_revjul() from Swiss Ephemeris
 */
export function julianDayToDateTime(
  julianDay: JulianDay,
  calendar: CalendarType = CalendarType.GREGORIAN
): DateTime {
  const { jd } = julianDay;

  let z = Math.floor(jd + 0.5);
  const f = jd + 0.5 - z;

  if (calendar === CalendarType.GREGORIAN) {
    const alpha = Math.floor((z - 1867216.25) / 36524.25);
    z = z + 1 + alpha - Math.floor(alpha / 4);
  }

  const a = z + 1524;
  const b = Math.floor((a - 122.1) / 365.25);
  const c = Math.floor(365.25 * b);
  const d = Math.floor((a - c) / 30.6001);

  const day = Math.floor(a - c - Math.floor(30.6001 * d) + f);
  const month = d < 14 ? d - 1 : d - 13;
  const year = month > 2 ? b - 4716 : b - 4715;

  // Extract time components
  const dayFraction = (jd + 0.5 - Math.floor(jd + 0.5));
  const hours = dayFraction * 24;
  const hour = Math.floor(hours);
  const minutes = (hours - hour) * 60;
  const minute = Math.floor(minutes);
  const second = Math.round((minutes - minute) * 60);

  return {
    year,
    month,
    day,
    hour,
    minute,
    second,
    calendar,
  };
}

/**
 * Get current Julian Day (now)
 */
export function getCurrentJulianDay(isUT: boolean = true): JulianDay {
  const now = new Date();
  const dateTime: DateTime = {
    year: now.getUTCFullYear(),
    month: now.getUTCMonth() + 1,
    day: now.getUTCDate(),
    hour: now.getUTCHours(),
    minute: now.getUTCMinutes(),
    second: now.getUTCSeconds(),
    calendar: CalendarType.GREGORIAN,
  };
  return dateTimeToJulianDay(dateTime, isUT);
}

/**
 * Calculate Delta T (difference between TT and UT)
 * Simplified approximation - actual calculation requires Swiss Ephemeris library
 */
export function calculateDeltaT(julianDay: number): number {
  // Approximate formula for Delta T
  // For precise values, this should call swe_deltat()
  const year = julianDayToDateTime({ jd: julianDay }).year;
  const t = (year - 2000) / 100;

  // Simple polynomial approximation
  return 62.92 + t * (32.217 + t * 55.89);
}

/**
 * Convert UT to ET (Ephemeris Time / TT)
 */
export function utToET(julianDayUT: number): number {
  const deltaT = calculateDeltaT(julianDayUT);
  return julianDayUT + deltaT / 86400.0;
}

/**
 * Convert ET to UT
 */
export function etToUT(julianDayET: number): number {
  const deltaT = calculateDeltaT(julianDayET);
  return julianDayET - deltaT / 86400.0;
}

// ============================================================================
// Angular Utilities
// ============================================================================

/**
 * Normalize angle to 0-360 range
 */
export function normalizeAngle(degrees: number): number {
  let result = degrees % 360;
  if (result < 0) result += 360;
  return result;
}

/**
 * Convert degrees to radians
 */
export function degreesToRadians(degrees: number): number {
  return (degrees * Math.PI) / 180;
}

/**
 * Convert radians to degrees
 */
export function radiansToDegrees(radians: number): number {
  return (radians * 180) / Math.PI;
}

/**
 * Convert decimal degrees to DMS (degrees, minutes, seconds)
 */
export function decimalToDMS(decimal: number): {
  degrees: number;
  minutes: number;
  seconds: number;
  sign: 1 | -1;
} {
  const sign = decimal >= 0 ? 1 : -1;
  const abs = Math.abs(decimal);
  const degrees = Math.floor(abs);
  const minutesDecimal = (abs - degrees) * 60;
  const minutes = Math.floor(minutesDecimal);
  const seconds = (minutesDecimal - minutes) * 60;

  return { degrees, minutes, seconds, sign };
}

/**
 * Convert DMS to decimal degrees
 */
export function dmsToDecimal(
  degrees: number,
  minutes: number,
  seconds: number,
  sign: 1 | -1 = 1
): number {
  return sign * (Math.abs(degrees) + minutes / 60 + seconds / 3600);
}

/**
 * Format angle as degrees, minutes, seconds string
 */
export function formatDMS(
  decimal: number,
  precision: 'degrees' | 'minutes' | 'seconds' = 'seconds'
): string {
  const { degrees, minutes, seconds, sign } = decimalToDMS(decimal);
  const signStr = sign < 0 ? '-' : '';

  if (precision === 'degrees') {
    return `${signStr}${degrees}°`;
  } else if (precision === 'minutes') {
    return `${signStr}${degrees}°${minutes}'`;
  } else {
    return `${signStr}${degrees}°${minutes}'${Math.round(seconds)}"`;
  }
}

/**
 * Format longitude with zodiac sign
 */
export function formatLongitudeZodiac(longitude: number): string {
  const signs = [
    'Ari', 'Tau', 'Gem', 'Can', 'Leo', 'Vir',
    'Lib', 'Sco', 'Sag', 'Cap', 'Aqu', 'Pis'
  ];

  const normalized = normalizeAngle(longitude);
  const signIndex = Math.floor(normalized / 30);
  const degreeInSign = normalized % 30;
  const { minutes, seconds } = decimalToDMS(degreeInSign);

  return `${Math.floor(degreeInSign)}° ${signs[signIndex]} ${minutes}'${Math.round(seconds)}"`;
}

/**
 * Format right ascension in hours, minutes, seconds
 */
export function formatRightAscension(degrees: number): string {
  const hours = degrees / 15; // 360 degrees = 24 hours
  const h = Math.floor(hours);
  const minutesDecimal = (hours - h) * 60;
  const m = Math.floor(minutesDecimal);
  const s = Math.round((minutesDecimal - m) * 60);

  return `${h}h${m}m${s}s`;
}

// ============================================================================
// Geographic Utilities
// ============================================================================

/**
 * Format geographic coordinates
 */
export function formatGeographicPosition(pos: GeographicPosition): string {
  const latDir = pos.latitude >= 0 ? 'N' : 'S';
  const lonDir = pos.longitude >= 0 ? 'E' : 'W';

  const lat = formatDMS(Math.abs(pos.latitude));
  const lon = formatDMS(Math.abs(pos.longitude));

  return `${lat} ${latDir}, ${lon} ${lonDir}`;
}

/**
 * Validate geographic coordinates
 */
export function validateGeographicPosition(
  pos: GeographicPosition
): boolean {
  return (
    pos.latitude >= -90 &&
    pos.latitude <= 90 &&
    pos.longitude >= -180 &&
    pos.longitude <= 180
  );
}

// ============================================================================
// Distance Conversions
// ============================================================================

/**
 * Convert AU to kilometers
 */
export function auToKm(au: number): number {
  return au * 149597870.700;
}

/**
 * Convert AU to light years
 */
export function auToLightYears(au: number): number {
  return au * (1.0 / 63241.07708427);
}

/**
 * Convert kilometers to AU
 */
export function kmToAU(km: number): number {
  return km / 149597870.700;
}

// ============================================================================
// Planet Name Utilities
// ============================================================================

/**
 * Get planet name from number
 */
export function getPlanetName(planetNumber: number): string {
  return PLANET_NAMES[planetNumber] || `Planet ${planetNumber}`;
}

/**
 * Parse planet name or number
 */
export function parsePlanet(input: string | number): number {
  if (typeof input === 'number') {
    return input;
  }

  // Try to find by name
  const lowerInput = input.toLowerCase();
  for (const [num, name] of Object.entries(PLANET_NAMES)) {
    if (name.toLowerCase() === lowerInput) {
      return parseInt(num, 10);
    }
  }

  // Try to parse as number
  const parsed = parseInt(input, 10);
  if (!isNaN(parsed)) {
    return parsed;
  }

  throw new Error(`Unknown planet: ${input}`);
}

// ============================================================================
// Date Formatting Utilities
// ============================================================================

/**
 * Format DateTime as string
 */
export function formatDateTime(
  dt: DateTime,
  format: 'iso' | 'short' | 'long' = 'iso'
): string {
  const pad = (n: number, width: number = 2) =>
    n.toString().padStart(width, '0');

  if (format === 'iso') {
    return `${dt.year}-${pad(dt.month)}-${pad(dt.day)}T${pad(dt.hour || 0)}:${pad(dt.minute || 0)}:${pad(dt.second || 0)}`;
  } else if (format === 'short') {
    return `${dt.day}.${dt.month}.${dt.year}`;
  } else {
    const months = [
      'Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun',
      'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec'
    ];
    return `${dt.day} ${months[dt.month - 1]} ${dt.year} ${pad(dt.hour || 0)}:${pad(dt.minute || 0)}`;
  }
}

/**
 * Parse date string to DateTime
 */
export function parseDateTime(dateStr: string): DateTime {
  // Handle Julian Day format (e.g., "j2451545.0")
  if (dateStr.toLowerCase().startsWith('j')) {
    const jd = parseFloat(dateStr.substring(1));
    return julianDayToDateTime({ jd });
  }

  // Try ISO format first
  const isoMatch = dateStr.match(
    /^(\d{4})-(\d{2})-(\d{2})(?:T(\d{2}):(\d{2}):(\d{2}))?/
  );
  if (isoMatch) {
    return {
      year: parseInt(isoMatch[1], 10),
      month: parseInt(isoMatch[2], 10),
      day: parseInt(isoMatch[3], 10),
      hour: isoMatch[4] ? parseInt(isoMatch[4], 10) : 0,
      minute: isoMatch[5] ? parseInt(isoMatch[5], 10) : 0,
      second: isoMatch[6] ? parseInt(isoMatch[6], 10) : 0,
      calendar: CalendarType.GREGORIAN,
    };
  }

  // Try European format (dd.mm.yyyy)
  const euroMatch = dateStr.match(/^(\d{1,2})\.(\d{1,2})\.(\d{4})/);
  if (euroMatch) {
    return {
      year: parseInt(euroMatch[3], 10),
      month: parseInt(euroMatch[2], 10),
      day: parseInt(euroMatch[1], 10),
      calendar: CalendarType.GREGORIAN,
    };
  }

  throw new Error(`Cannot parse date: ${dateStr}`);
}

// ============================================================================
// Time Utilities
// ============================================================================

/**
 * Calculate sidereal time at Greenwich
 * Simplified - actual implementation requires Swiss Ephemeris
 */
export function calculateSiderealTime(julianDay: number): number {
  const t = (julianDay - 2451545.0) / 36525.0;

  // Mean sidereal time at Greenwich (simplified formula)
  let gmst =
    280.46061837 +
    360.98564736629 * (julianDay - 2451545.0) +
    0.000387933 * t * t -
    (t * t * t) / 38710000.0;

  return normalizeAngle(gmst);
}

/**
 * Calculate local sidereal time
 */
export function calculateLocalSiderealTime(
  julianDay: number,
  longitude: number
): number {
  const gmst = calculateSiderealTime(julianDay);
  return normalizeAngle(gmst + longitude);
}

// ============================================================================
// Validation Utilities
// ============================================================================

/**
 * Validate Julian Day number
 */
export function validateJulianDay(jd: number): boolean {
  // Julian Day 0 = Jan 1, 4713 BC
  // Reasonable range: -3000 BC to 3000 AD
  return jd >= 625673.5 && jd <= 2816788.5;
}

/**
 * Validate DateTime
 */
export function validateDateTime(dt: DateTime): boolean {
  return (
    dt.year >= -3000 &&
    dt.year <= 3000 &&
    dt.month >= 1 &&
    dt.month <= 12 &&
    dt.day >= 1 &&
    dt.day <= 31 &&
    (dt.hour === undefined || (dt.hour >= 0 && dt.hour < 24)) &&
    (dt.minute === undefined || (dt.minute >= 0 && dt.minute < 60)) &&
    (dt.second === undefined || (dt.second >= 0 && dt.second < 60))
  );
}

// ============================================================================
// Aspect Calculations
// ============================================================================

/**
 * Calculate angular difference (aspect) between two longitudes
 */
export function calculateAspect(lon1: number, lon2: number): number {
  let diff = Math.abs(normalizeAngle(lon1) - normalizeAngle(lon2));
  if (diff > 180) diff = 360 - diff;
  return diff;
}

/**
 * Check if two planets are within orb of a specific aspect
 */
export function isInAspect(
  lon1: number,
  lon2: number,
  aspectAngle: number,
  orb: number
): boolean {
  const aspect = calculateAspect(lon1, lon2);
  return Math.abs(aspect - aspectAngle) <= orb;
}
