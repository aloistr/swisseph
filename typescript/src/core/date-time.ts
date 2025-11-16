/**
 * Date and Time Calculations
 *
 * Julian Day conversions, sidereal time, and Delta T calculations
 *
 * Clean-room implementation of standard astronomical algorithms
 * Based on Meeus "Astronomical Algorithms" and USNO standards
 */

import {
  J2000,
  DAYS_PER_CENTURY,
  SECONDS_PER_DAY,
  normalizeDegrees,
  julianCenturiesFromJ2000,
} from './astronomical-constants';

// ============================================================================
// Julian Day Conversions
// ============================================================================

/**
 * Convert Gregorian calendar date to Julian Day Number
 *
 * Based on Meeus, Astronomical Algorithms, Chapter 7
 *
 * @param year - Year (negative for BCE)
 * @param month - Month (1-12)
 * @param day - Day (1-31, can include decimal for time)
 * @param hour - Hour (0-23)
 * @param minute - Minute (0-59)
 * @param second - Second (0-59.999...)
 * @returns Julian Day Number
 */
export function gregorianToJulianDay(
  year: number,
  month: number,
  day: number,
  hour: number = 0,
  minute: number = 0,
  second: number = 0
): number {
  // Adjust for January and February
  let y = year;
  let m = month;
  if (month <= 2) {
    y = year - 1;
    m = month + 12;
  }

  // Gregorian calendar adjustment
  const a = Math.floor(y / 100);
  const b = 2 - a + Math.floor(a / 4);

  // Calculate JD
  const jd =
    Math.floor(365.25 * (y + 4716)) +
    Math.floor(30.6001 * (m + 1)) +
    day +
    b -
    1524.5;

  // Add time fraction
  const timeFraction = (hour + minute / 60 + second / 3600) / 24;

  return jd + timeFraction;
}

/**
 * Convert Julian calendar date to Julian Day Number
 */
export function julianToJulianDay(
  year: number,
  month: number,
  day: number,
  hour: number = 0,
  minute: number = 0,
  second: number = 0
): number {
  let y = year;
  let m = month;
  if (month <= 2) {
    y = year - 1;
    m = month + 12;
  }

  const jd =
    Math.floor(365.25 * (y + 4716)) +
    Math.floor(30.6001 * (m + 1)) +
    day -
    1524.5;

  const timeFraction = (hour + minute / 60 + second / 3600) / 24;

  return jd + timeFraction;
}

/**
 * Convert Julian Day to Gregorian calendar date
 *
 * @param jd - Julian Day Number
 * @returns Object with year, month, day, hour, minute, second
 */
export function julianDayToGregorian(jd: number): {
  year: number;
  month: number;
  day: number;
  hour: number;
  minute: number;
  second: number;
} {
  const jd0 = jd + 0.5;
  const z = Math.floor(jd0);
  const f = jd0 - z;

  let a = z;
  if (z >= 2299161) {
    const alpha = Math.floor((z - 1867216.25) / 36524.25);
    a = z + 1 + alpha - Math.floor(alpha / 4);
  }

  const b = a + 1524;
  const c = Math.floor((b - 122.1) / 365.25);
  const d = Math.floor(365.25 * c);
  const e = Math.floor((b - d) / 30.6001);

  const day = b - d - Math.floor(30.6001 * e) + f;
  const month = e < 14 ? e - 1 : e - 13;
  const year = month > 2 ? c - 4716 : c - 4715;

  const dayInt = Math.floor(day);
  const dayFrac = day - dayInt;
  const hours = dayFrac * 24;
  const hour = Math.floor(hours);
  const minutes = (hours - hour) * 60;
  const minute = Math.floor(minutes);
  const second = (minutes - minute) * 60;

  return { year, month, day: dayInt, hour, minute, second };
}

// ============================================================================
// Delta T (TT - UT) Calculations
// ============================================================================

/**
 * Calculate Delta T (Terrestrial Time - Universal Time)
 *
 * Based on polynomial fits and historical data
 * Espenak and Meeus (2006) + Morrison and Stephenson (2004)
 *
 * @param jd - Julian Day (UT)
 * @returns Delta T in seconds
 */
export function calculateDeltaT(jd: number): number {
  const { year } = julianDayToGregorian(jd);
  const y = year + (jd - gregorianToJulianDay(year, 1, 1)) / 365.25;

  // Before -500
  if (y < -500) {
    const u = (y - 1820) / 100;
    return -20 + 32 * u * u;
  }

  // -500 to 500
  if (y < 500) {
    const u = y / 100;
    return (
      10583.6 -
      1014.41 * u +
      33.78311 * u * u -
      5.952053 * u * u * u -
      0.1798452 * u * u * u * u +
      0.022174192 * u * u * u * u * u +
      0.0090316521 * u * u * u * u * u * u
    );
  }

  // 500 to 1600
  if (y < 1600) {
    const u = (y - 1000) / 100;
    return (
      1574.2 -
      556.01 * u +
      71.23472 * u * u +
      0.319781 * u * u * u -
      0.8503463 * u * u * u * u -
      0.005050998 * u * u * u * u * u +
      0.0083572073 * u * u * u * u * u * u
    );
  }

  // 1600 to 1700
  if (y < 1700) {
    const t = y - 1600;
    return 120 - 0.9808 * t - 0.01532 * t * t + t * t * t / 7129;
  }

  // 1700 to 1800
  (y < 1800) {
    const t = y - 1700;
    return (
      8.83 +
      0.1603 * t -
      0.0059285 * t * t +
      0.00013336 * t * t * t -
      t * t * t * t / 1174000
    );
  }

  // 1800 to 1860
  if (y < 1860) {
    const t = y - 1800;
    return (
      13.72 -
      0.332447 * t +
      0.0068612 * t * t +
      0.0041116 * t * t * t -
      0.00037436 * t * t * t * t +
      0.0000121272 * t * t * t * t * t -
      0.0000001699 * t * t * t * t * t * t +
      0.000000000875 * t * t * t * t * t * t * t
    );
  }

  // 1860 to 1900
  if (y < 1900) {
    const t = y - 1860;
    return (
      7.62 +
      0.5737 * t -
      0.251754 * t * t +
      0.01680668 * t * t * t -
      0.0004473624 * t * t * t * t +
      t * t * t * t * t / 233174
    );
  }

  // 1900 to 1920
  if (y < 1920) {
    const t = y - 1900;
    return (
      -2.79 +
      1.494119 * t -
      0.0598939 * t * t +
      0.0061966 * t * t * t -
      0.000197 * t * t * t * t
    );
  }

  // 1920 to 1941
  if (y < 1941) {
    const t = y - 1920;
    return (
      21.20 +
      0.84493 * t -
      0.076100 * t * t +
      0.0020936 * t * t * t
    );
  }

  // 1941 to 1961
  if (y < 1961) {
    const t = y - 1950;
    return (
      29.07 +
      0.407 * t -
      t * t / 233 +
      t * t * t / 2547
    );
  }

  // 1961 to 1986
  if (y < 1986) {
    const t = y - 1975;
    return (
      45.45 +
      1.067 * t -
      t * t / 260 -
      t * t * t / 718
    );
  }

  // 1986 to 2005
  if (y < 2005) {
    const t = y - 2000;
    return (
      63.86 +
      0.3345 * t -
      0.060374 * t * t +
      0.0017275 * t * t * t +
      0.000651814 * t * t * t * t +
      0.00002373599 * t * t * t * t * t
    );
  }

  // 2005 to 2050
  if (y < 2050) {
    const t = y - 2000;
    return 62.92 + 0.32217 * t + 0.005589 * t * t;
  }

  // 2050 to 2150
  if (y < 2150) {
    return -20 + 32 * ((y - 1820) / 100) ** 2 - 0.5628 * (2150 - y);
  }

  // After 2150
  const u = (y - 1820) / 100;
  return -20 + 32 * u * u;
}

/**
 * Convert UT to TT (Terrestrial Time)
 */
export function utToTT(jdUT: number): number {
  const deltaT = calculateDeltaT(jdUT);
  return jdUT + deltaT / SECONDS_PER_DAY;
}

/**
 * Convert TT to UT
 */
export function ttToUT(jdTT: number): number {
  // Iterative solution since deltaT depends on UT
  let jdUT = jdTT;
  for (let i = 0; i < 3; i++) {
    const deltaT = calculateDeltaT(jdUT);
    jdUT = jdTT - deltaT / SECONDS_PER_DAY;
  }
  return jdUT;
}

// ============================================================================
// Sidereal Time Calculations
// ============================================================================

/**
 * Calculate Greenwich Mean Sidereal Time (GMST)
 *
 * Based on IAU 2006 precession model
 * Meeus, Astronomical Algorithms, Chapter 12
 *
 * @param jd - Julian Day (UT)
 * @returns GMST in degrees
 */
export function calculateGMST(jd: number): number {
  const T = julianCenturiesFromJ2000(jd);

  // GMST at 0h UT
  const jd0 = Math.floor(jd - 0.5) + 0.5;
  const T0 = julianCenturiesFromJ2000(jd0);

  let gmst =
    280.46061837 +
    360.98564736629 * (jd - J2000) +
    0.000387933 * T0 * T0 -
    (T0 * T0 * T0) / 38710000;

  return normalizeDegrees(gmst);
}

/**
 * Calculate Greenwich Apparent Sidereal Time (GAST)
 *
 * GAST = GMST + equation of equinoxes
 *
 * @param jd - Julian Day (UT)
 * @param nutationInLongitude - Nutation in longitude (degrees)
 * @param trueObliquity - True obliquity of the ecliptic (degrees)
 * @returns GAST in degrees
 */
export function calculateGAST(
  jd: number,
  nutationInLongitude: number,
  trueObliquity: number
): number {
  const gmst = calculateGMST(jd);

  // Equation of equinoxes
  const eqEq = nutationInLongitude * Math.cos(trueObliquity * Math.PI / 180);

  return normalizeDegrees(gmst + eqEq);
}

/**
 * Calculate Local Mean Sidereal Time (LMST)
 *
 * @param jd - Julian Day (UT)
 * @param longitude - Geographic longitude in degrees (positive East)
 * @returns LMST in degrees
 */
export function calculateLMST(jd: number, longitude: number): number {
  const gmst = calculateGMST(jd);
  return normalizeDegrees(gmst + longitude);
}

/**
 * Calculate Local Apparent Sidereal Time (LAST)
 *
 * @param jd - Julian Day (UT)
 * @param longitude - Geographic longitude in degrees (positive East)
 * @param nutationInLongitude - Nutation in longitude (degrees)
 * @param trueObliquity - True obliquity of the ecliptic (degrees)
 * @returns LAST in degrees
 */
export function calculateLAST(
  jd: number,
  longitude: number,
  nutationInLongitude: number,
  trueObliquity: number
): number {
  const gast = calculateGAST(jd, nutationInLongitude, trueObliquity);
  return normalizeDegrees(gast + longitude);
}

// ============================================================================
// Utility Functions
// ============================================================================

/**
 * Get current Julian Day (UT)
 */
export function getCurrentJulianDay(): number {
  const now = new Date();
  return gregorianToJulianDay(
    now.getUTCFullYear(),
    now.getUTCMonth() + 1,
    now.getUTCDate(),
    now.getUTCHours(),
    now.getUTCMinutes(),
    now.getUTCSeconds() + now.getUTCMilliseconds() / 1000
  );
}

/**
 * Check if a year is a leap year
 */
export function isLeapYear(year: number): boolean {
  if (year % 400 === 0) return true;
  if (year % 100 === 0) return false;
  if (year % 4 === 0) return true;
  return false;
}

/**
 * Get number of days in a month
 */
export function daysInMonth(year: number, month: number): number {
  const days = [31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31];
  if (month === 2 && isLeapYear(year)) return 29;
  return days[month - 1];
}

/**
 * Validate date
 */
export function isValidDate(year: number, month: number, day: number): boolean {
  if (month < 1 || month > 12) return false;
  if (day < 1 || day > daysInMonth(year, month)) return false;
  return true;
}
