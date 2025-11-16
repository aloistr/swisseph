/**
 * Rise, Set, and Transit Calculations
 *
 * Calculate rising, setting, and meridian transit times for celestial bodies
 *
 * Clean-room implementation based on:
 * - Meeus, Astronomical Algorithms, Chapter 15
 * - USNO algorithms
 */

import {
  toRadians,
  toDegrees,
  normalizeDegrees,
  REFRACTION_HORIZON,
} from './astronomical-constants';

import {
  calculateLMST,
  gregorianToJulianDay,
} from './date-time';

import {
  eclipticToEquatorial,
} from './coordinate-transformations';

import {
  calculateSunPosition,
} from './planetary-calculator';

import {
  calculateMoonPosition,
} from './moon-calculator';

import { calculateMeanObliquity } from './precession-nutation';

// ============================================================================
// Rise/Set/Transit Data Structures
// ============================================================================

export interface RiseSetTransitData {
  rise?: {
    julianDay: number;
    azimuth: number;      // degrees
    utcHours: number;     // hours (0-24)
  };
  set?: {
    julianDay: number;
    azimuth: number;
    utcHours: number;
  };
  transitUpper?: {
    julianDay: number;
    altitude: number;     // degrees above horizon
    utcHours: number;
  };
  transitLower?: {
    julianDay: number;
    altitude: number;     // degrees below horizon
    utcHours: number;
  };
  circumpolar?: boolean;   // Never sets
  neverRises?: boolean;    // Never rises
}

// ============================================================================
// Standard Altitudes for Different Events
// ============================================================================

export const STANDARD_ALTITUDES = {
  ASTRONOMICAL_TWILIGHT: -18,      // degrees
  NAUTICAL_TWILIGHT: -12,
  CIVIL_TWILIGHT: -6,
  SUNRISE_SUNSET: -0.8333,         // -50' (refraction + solar radius)
  MOONRISE_MOONSET: 0.125,         // +7.5' (Moon's radius)
  STAR_PLANET: -0.5667,            // -34' (atmospheric refraction)
};

// ============================================================================
// Rise/Set/Transit Calculation
// ============================================================================

/**
 * Calculate rise, set, and transit times for a celestial body
 *
 * @param jdUT - Julian Day (UT) for the date of interest (0h UT)
 * @param latitude - Observer latitude (degrees, positive North)
 * @param longitude - Observer longitude (degrees, positive East)
 * @param getPosition - Function to get body's position at given JD
 * @param h0 - Standard altitude (degrees, negative below horizon)
 * @returns Rise/set/transit data
 */
export function calculateRiseSetTransit(
  jdUT: number,
  latitude: number,
  longitude: number,
  getPosition: (jd: number) => { rightAscension: number; declination: number },
  h0: number = STANDARD_ALTITUDES.STAR_PLANET
): RiseSetTransitData {
  // Get positions for 0h UT on previous, current, and next days
  const jd0 = Math.floor(jdUT - 0.5) + 0.5; // 0h UT on the day

  const pos0 = getPosition(jd0 - 1);
  const pos1 = getPosition(jd0);
  const pos2 = getPosition(jd0 + 1);

  // Interpolate positions
  const alpha1 = pos1.rightAscension;
  const delta1 = pos1.declination;

  // Calculate approximate times using simple formula
  const latRad = toRadians(latitude);
  const delta1Rad = toRadians(delta1);
  const h0Rad = toRadians(h0);

  // Hour angle at rise/set
  const cosH = (Math.sin(h0Rad) - Math.sin(latRad) * Math.sin(delta1Rad)) /
    (Math.cos(latRad) * Math.cos(delta1Rad));

  // Check for circumpolar or never-rising conditions
  if (cosH < -1) {
    // Circumpolar - never sets
    return {
      circumpolar: true,
      transitUpper: calculateTransit(jd0, longitude, alpha1, latitude, delta1),
    };
  }

  if (cosH > 1) {
    // Never rises
    return {
      neverRises: true,
    };
  }

  const H0 = toDegrees(Math.acos(cosH));

  // Approximate times (in fraction of day)
  const m0 = (alpha1 - longitude - calculateLMST(jd0, 0)) / 360; // Transit
  const m1 = m0 - H0 / 360; // Rise
  const m2 = m0 + H0 / 360; // Set

  // Normalize to 0-1 range
  const normalizeM = (m: number) => {
    let result = m % 1;
    if (result < 0) result += 1;
    return result;
  };

  const m0Norm = normalizeM(m0);
  const m1Norm = normalizeM(m1);
  const m2Norm = normalizeM(m2);

  // Refine times using iteration
  const riseData = refinRiseSetTime(
    jd0,
    m1Norm,
    latitude,
    longitude,
    getPosition,
    h0,
    'rise'
  );

  const setData = refineRiseSetTime(
    jd0,
    m2Norm,
    latitude,
    longitude,
    getPosition,
    h0,
    'set'
  );

  const transitData = refineTransitTime(
    jd0,
    m0Norm,
    latitude,
    longitude,
    getPosition
  );

  return {
    rise: riseData,
    set: setData,
    transitUpper: transitData,
  };
}

/**
 * Refine rise/set time using iteration
 */
function refineRiseSetTime(
  jd0: number,
  m: number,
  latitude: number,
  longitude: number,
  getPosition: (jd: number) => { rightAscension: number; declination: number },
  h0: number,
  type: 'rise' | 'set'
): { julianDay: number; azimuth: number; utcHours: number } | undefined {
  let mRefined = m;

  // Iterate to refine
  for (let i = 0; i < 3; i++) {
    const jd = jd0 + mRefined;
    const pos = getPosition(jd);

    const lst = calculateLMST(jd, longitude);
    const H = normalizeDegrees(lst - pos.rightAscension);

    const latRad = toRadians(latitude);
    const decRad = toRadians(pos.declination);
    const HRad = toRadians(H);

    // Altitude
    const sinH =
      Math.sin(latRad) * Math.sin(decRad) +
      Math.cos(latRad) * Math.cos(decRad) * Math.cos(HRad);
    const altitude = toDegrees(Math.asin(sinH));

    // Correction
    const dm = (altitude - h0) / (360 * Math.cos(decRad) * Math.cos(latRad) * Math.sin(HRad));
    mRefined -= dm;

    if (Math.abs(dm) < 0.0001) break;
  }

  // Calculate azimuth
  const jdFinal = jd0 + mRefined;
  const posFinal = getPosition(jdFinal);
  const lstFinal = calculateLMST(jdFinal, longitude);
  const HFinal = normalizeDegrees(lstFinal - posFinal.rightAscension);

  const azimuth = calculateAzimuth(latitude, posFinal.declination, HFinal);

  return {
    julianDay: jdFinal,
    azimuth,
    utcHours: mRefined * 24,
  };
}

/**
 * Refine transit time
 */
function refineTransitTime(
  jd0: number,
  m: number,
  latitude: number,
  longitude: number,
  getPosition: (jd: number) => { rightAscension: number; declination: number }
): { julianDay: number; altitude: number; utcHours: number } {
  let mRefined = m;

  for (let i = 0; i < 3; i++) {
    const jd = jd0 + mRefined;
    const pos = getPosition(jd);

    const lst = calculateLMST(jd, longitude);
    const H = normalizeDegrees(lst - pos.rightAscension);

    const dm = -H / 360;
    mRefined += dm;

    if (Math.abs(dm) < 0.0001) break;
  }

  const jdFinal = jd0 + mRefined;
  const posFinal = getPosition(jdFinal);

  // Calculate altitude at transit (H = 0)
  const latRad = toRadians(latitude);
  const decRad = toRadians(posFinal.declination);

  const altitude = toDegrees(Math.asin(
    Math.sin(latRad) * Math.sin(decRad) +
    Math.cos(latRad) * Math.cos(decRad)
  ));

  return {
    julianDay: jdFinal,
    altitude,
    utcHours: mRefined * 24,
  };
}

/**
 * Calculate transit (simpler version)
 */
function calculateTransit(
  jd0: number,
  longitude: number,
  alpha: number,
  latitude: number,
  delta: number
): { julianDay: number; altitude: number; utcHours: number } {
  const m0 = (alpha - longitude - calculateLMST(jd0, 0)) / 360;
  let m0Norm = m0 % 1;
  if (m0Norm < 0) m0Norm += 1;

  const jdTransit = jd0 + m0Norm;

  const latRad = toRadians(latitude);
  const decRad = toRadians(delta);

  const altitude = toDegrees(Math.asin(
    Math.sin(latRad) * Math.sin(decRad) +
    Math.cos(latRad) * Math.cos(decRad)
  ));

  return {
    julianDay: jdTransit,
    altitude,
    utcHours: m0Norm * 24,
  };
}

/**
 * Calculate azimuth
 */
function calculateAzimuth(
  latitude: number,
  declination: number,
  hourAngle: number
): number {
  const latRad = toRadians(latitude);
  const decRad = toRadians(declination);
  const HRad = toRadians(hourAngle);

  const azimuth = toDegrees(Math.atan2(
    Math.sin(HRad),
    Math.cos(HRad) * Math.sin(latRad) - Math.tan(decRad) * Math.cos(latRad)
  ));

  return normalizeDegrees(azimuth + 180); // From North
}

// ============================================================================
// Convenience Functions for Specific Bodies
// ============================================================================

/**
 * Calculate Sun rise/set/transit
 *
 * @param year - Year
 * @param month - Month
 * @param day - Day
 * @param latitude - Latitude (degrees)
 * @param longitude - Longitude (degrees)
 * @returns Rise/set/transit data
 */
export function calculateSunRiseSetTransit(
  year: number,
  month: number,
  day: number,
  latitude: number,
  longitude: number
): RiseSetTransitData {
  const jd0 = gregorianToJulianDay(year, month, day, 0, 0, 0);

  const getPosition = (jd: number) => {
    const sunPos = calculateSunPosition(jd);
    const obliquity = calculateMeanObliquity((jd - 2451545.0) / 36525);
    return eclipticToEquatorial(sunPos.longitude, sunPos.latitude, obliquity);
  };

  return calculateRiseSetTransit(
    jd0,
    latitude,
    longitude,
    getPosition,
    STANDARD_ALTITUDES.SUNRISE_SUNSET
  );
}

/**
 * Calculate Moon rise/set/transit
 */
export function calculateMoonRiseSetTransit(
  year: number,
  month: number,
  day: number,
  latitude: number,
  longitude: number
): RiseSetTransitData {
  const jd0 = gregorianToJulianDay(year, month, day, 0, 0, 0);

  const getPosition = (jd: number) => {
    const moonPos = calculateMoonPosition(jd);
    const obliquity = calculateMeanObliquity((jd - 2451545.0) / 36525);
    return eclipticToEquatorial(moonPos.longitude, moonPos.latitude, obliquity);
  };

  return calculateRiseSetTransit(
    jd0,
    latitude,
    longitude,
    getPosition,
    STANDARD_ALTITUDES.MOONRISE_MOONSET
  );
}

/**
 * Calculate twilight times
 *
 * @param year - Year
 * @param month - Month
 * @param day - Day
 * @param latitude - Latitude
 * @param longitude - Longitude
 * @param twilightType - Type of twilight ('civil', 'nautical', 'astronomical')
 * @returns Twilight times
 */
export function calculateTwilight(
  year: number,
  month: number,
  day: number,
  latitude: number,
  longitude: number,
  twilightType: 'civil' | 'nautical' | 'astronomical' = 'civil'
): RiseSetTransitData {
  const jd0 = gregorianToJulianDay(year, month, day, 0, 0, 0);

  const getPosition = (jd: number) => {
    const sunPos = calculateSunPosition(jd);
    const obliquity = calculateMeanObliquity((jd - 2451545.0) / 36525);
    return eclipticToEquatorial(sunPos.longitude, sunPos.latitude, obliquity);
  };

  const altitudes = {
    civil: STANDARD_ALTITUDES.CIVIL_TWILIGHT,
    nautical: STANDARD_ALTITUDES.NAUTICAL_TWILIGHT,
    astronomical: STANDARD_ALTITUDES.ASTRONOMICAL_TWILIGHT,
  };

  return calculateRiseSetTransit(
    jd0,
    latitude,
    longitude,
    getPosition,
    altitudes[twilightType]
  );
}

/**
 * Format time as HH:MM:SS
 */
export function formatTime(hours: number): string {
  const h = Math.floor(hours);
  const m = Math.floor((hours - h) * 60);
  const s = Math.floor(((hours - h) * 60 - m) * 60);

  return `${h.toString().padStart(2, '0')}:${m.toString().padStart(2, '0')}:${s.toString().padStart(2, '0')}`;
}
