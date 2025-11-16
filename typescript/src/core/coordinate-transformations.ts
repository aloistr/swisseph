/**
 * Coordinate Transformations
 *
 * Convert between different coordinate systems:
 * - Ecliptic ↔ Equatorial
 * - Equatorial ↔ Horizontal (Altitude/Azimuth)
 * - Cartesian ↔ Spherical
 *
 * Clean-room implementation based on standard astronomical formulas
 */

import { toRadians, toDegrees, normalizeDegrees } from './astronomical-constants';

// ============================================================================
// Ecliptic ↔ Equatorial Transformations
// ============================================================================

/**
 * Convert ecliptic coordinates to equatorial coordinates
 *
 * @param longitude - Ecliptic longitude (degrees)
 * @param latitude - Ecliptic latitude (degrees)
 * @param obliquity - Obliquity of the ecliptic (degrees)
 * @returns Equatorial coordinates (RA, Dec)
 */
export function eclipticToEquatorial(
  longitude: number,
  latitude: number,
  obliquity: number
): {
  rightAscension: number;  // degrees
  declination: number;     // degrees
} {
  const lonRad = toRadians(longitude);
  const latRad = toRadians(latitude);
  const oblRad = toRadians(obliquity);

  const sinDec =
    Math.sin(latRad) * Math.cos(oblRad) +
    Math.cos(latRad) * Math.sin(oblRad) * Math.sin(lonRad);

  const declination = toDegrees(Math.asin(sinDec));

  const y = Math.sin(lonRad) * Math.cos(oblRad) - Math.tan(latRad) * Math.sin(oblRad);
  const x = Math.cos(lonRad);

  const rightAscension = normalizeDegrees(toDegrees(Math.atan2(y, x)));

  return { rightAscension, declination };
}

/**
 * Convert equatorial coordinates to ecliptic coordinates
 *
 * @param rightAscension - Right ascension (degrees)
 * @param declination - Declination (degrees)
 * @param obliquity - Obliquity of the ecliptic (degrees)
 * @returns Ecliptic coordinates (longitude, latitude)
 */
export function equatorialToEcliptic(
  rightAscension: number,
  declination: number,
  obliquity: number
): {
  longitude: number;  // degrees
  latitude: number;   // degrees
} {
  const raRad = toRadians(rightAscension);
  const decRad = toRadians(declination);
  const oblRad = toRadians(obliquity);

  const sinLat =
    Math.sin(decRad) * Math.cos(oblRad) -
    Math.cos(decRad) * Math.sin(oblRad) * Math.sin(raRad);

  const latitude = toDegrees(Math.asin(sinLat));

  const y = Math.sin(raRad) * Math.cos(oblRad) + Math.tan(decRad) * Math.sin(oblRad);
  const x = Math.cos(raRad);

  const longitude = normalizeDegrees(toDegrees(Math.atan2(y, x)));

  return { longitude, latitude };
}

// ============================================================================
// Equatorial ↔ Horizontal Transformations
// ============================================================================

/**
 * Convert equatorial coordinates to horizontal coordinates (Alt/Az)
 *
 * @param rightAscension - Right ascension (degrees)
 * @param declination - Declination (degrees)
 * @param localSiderealTime - Local sidereal time (degrees)
 * @param latitude - Observer latitude (degrees, positive North)
 * @returns Horizontal coordinates (azimuth, altitude)
 */
export function equatorialToHorizontal(
  rightAscension: number,
  declination: number,
  localSiderealTime: number,
  latitude: number
): {
  azimuth: number;    // degrees (from North, eastward)
  altitude: number;   // degrees
} {
  // Hour angle
  const hourAngle = normalizeDegrees(localSiderealTime - rightAscension);

  const haRad = toRadians(hourAngle);
  const decRad = toRadians(declination);
  const latRad = toRadians(latitude);

  // Altitude
  const sinAlt =
    Math.sin(latRad) * Math.sin(decRad) +
    Math.cos(latRad) * Math.cos(decRad) * Math.cos(haRad);

  const altitude = toDegrees(Math.asin(sinAlt));

  // Azimuth
  const y = Math.sin(haRad);
  const x =
    Math.cos(haRad) * Math.sin(latRad) -
    Math.tan(decRad) * Math.cos(latRad);

  let azimuth = toDegrees(Math.atan2(y, x));
  azimuth = normalizeDegrees(azimuth + 180); // Measured from North

  return { azimuth, altitude };
}

/**
 * Convert horizontal coordinates to equatorial coordinates
 *
 * @param azimuth - Azimuth (degrees, from North eastward)
 * @param altitude - Altitude (degrees)
 * @param localSiderealTime - Local sidereal time (degrees)
 * @param latitude - Observer latitude (degrees, positive North)
 * @returns Equatorial coordinates (RA, Dec)
 */
export function horizontalToEquatorial(
  azimuth: number,
  altitude: number,
  localSiderealTime: number,
  latitude: number
): {
  rightAscension: number;
  declination: number;
} {
  const azRad = toRadians(azimuth - 180); // Convert from North to South
  const altRad = toRadians(altitude);
  const latRad = toRadians(latitude);

  // Declination
  const sinDec =
    Math.sin(latRad) * Math.sin(altRad) +
    Math.cos(latRad) * Math.cos(altRad) * Math.cos(azRad);

  const declination = toDegrees(Math.asin(sinDec));

  // Hour angle
  const y = Math.sin(azRad);
  const x =
    Math.cos(azRad) * Math.sin(latRad) -
    Math.tan(altRad) * Math.cos(latRad);

  const hourAngle = toDegrees(Math.atan2(y, x));

  // Right ascension
  const rightAscension = normalizeDegrees(localSiderealTime - hourAngle);

  return { rightAscension, declination };
}

// ============================================================================
// Spherical ↔ Cartesian Transformations
// ============================================================================

/**
 * Convert spherical coordinates to Cartesian coordinates
 *
 * @param longitude - Longitude (degrees)
 * @param latitude - Latitude (degrees)
 * @param distance - Distance (any unit)
 * @returns Cartesian coordinates (x, y, z)
 */
export function sphericalToCartesian(
  longitude: number,
  latitude: number,
  distance: number
): {
  x: number;
  y: number;
  z: number;
} {
  const lonRad = toRadians(longitude);
  const latRad = toRadians(latitude);

  const x = distance * Math.cos(latRad) * Math.cos(lonRad);
  const y = distance * Math.cos(latRad) * Math.sin(lonRad);
  const z = distance * Math.sin(latRad);

  return { x, y, z };
}

/**
 * Convert Cartesian coordinates to spherical coordinates
 *
 * @param x - X coordinate
 * @param y - Y coordinate
 * @param z - Z coordinate
 * @returns Spherical coordinates (longitude, latitude, distance)
 */
export function cartesianToSpherical(
  x: number,
  y: number,
  z: number
): {
  longitude: number;
  latitude: number;
  distance: number;
} {
  const distance = Math.sqrt(x * x + y * y + z * z);
  const longitude = normalizeDegrees(toDegrees(Math.atan2(y, x)));
  const latitude = toDegrees(Math.asin(z / distance));

  return { longitude, latitude, distance };
}

// ============================================================================
// Atmospheric Refraction
// ============================================================================

/**
 * Calculate atmospheric refraction correction
 *
 * Uses Bennett's formula (accurate to 0.07 arcmin for altitudes > 15°)
 *
 * @param trueAltitude - True altitude (degrees)
 * @param temperature - Temperature (Celsius)
 * @param pressure - Atmospheric pressure (hPa/mbar)
 * @returns Refraction correction (degrees, always positive)
 */
export function calculateRefraction(
  trueAltitude: number,
  temperature: number = 15,
  pressure: number = 1013.25
): number {
  if (trueAltitude < -2) {
    return 0; // Below horizon, no refraction
  }

  // Bennett's formula
  const h = trueAltitude + 7.31 / (trueAltitude + 4.4);
  const refraction = 1.0 / Math.tan(toRadians(h));

  // Pressure and temperature correction
  const pressureCorrection = pressure / 1010;
  const tempCorrection = 283 / (273 + temperature);

  return (refraction / 60) * pressureCorrection * tempCorrection; // Convert to degrees
}

/**
 * Calculate apparent altitude (observed altitude with refraction)
 *
 * @param trueAltitude - True geometric altitude (degrees)
 * @param temperature - Temperature (Celsius)
 * @param pressure - Pressure (hPa)
 * @returns Apparent altitude (degrees)
 */
export function trueToApparentAltitude(
  trueAltitude: number,
  temperature: number = 15,
  pressure: number = 1013.25
): number {
  const refraction = calculateRefraction(trueAltitude, temperature, pressure);
  return trueAltitude + refraction;
}

/**
 * Calculate true altitude from apparent altitude
 *
 * @param apparentAltitude - Observed altitude (degrees)
 * @param temperature - Temperature (Celsius)
 * @param pressure - Pressure (hPa)
 * @returns True geometric altitude (degrees)
 */
export function apparentToTrueAltitude(
  apparentAltitude: number,
  temperature: number = 15,
  pressure: number = 1013.25
): number {
  // Iterative solution
  let trueAlt = apparentAltitude;

  for (let i = 0; i < 3; i++) {
    const refr = calculateRefraction(trueAlt, temperature, pressure);
    trueAlt = apparentAltitude - refr;
  }

  return trueAlt;
}

// ============================================================================
// Parallax Corrections
// ============================================================================

/**
 * Calculate horizontal parallax for the Moon
 *
 * @param distance - Distance in AU
 * @returns Horizontal parallax in degrees
 */
export function calculateHorizontalParallax(distance: number): number {
  // Earth's equatorial radius in AU
  const earthRadius = 6378.137 / 149597870.7;

  // Horizontal parallax in radians
  const parallax = Math.asin(earthRadius / distance);

  return toDegrees(parallax);
}

/**
 * Apply parallax correction to equatorial coordinates
 *
 * @param ra - Right ascension (degrees)
 * @param dec - Declination (degrees)
 * @param distance - Distance (AU)
 * @param lst - Local sidereal time (degrees)
 * @param latitude - Observer latitude (degrees)
 * @param elevation - Observer elevation (meters)
 * @returns Topocentric coordinates
 */
export function applyParallax(
  ra: number,
  dec: number,
  distance: number,
  lst: number,
  latitude: number,
  elevation: number = 0
): {
  rightAscension: number;
  declination: number;
} {
  const hp = calculateHorizontalParallax(distance);
  const hourAngle = normalizeDegrees(lst - ra);

  const haRad = toRadians(hourAngle);
  const decRad = toRadians(dec);
  const latRad = toRadians(latitude);
  const hpRad = toRadians(hp);

  // Earth's radius correction for elevation
  const earthRadius = 6378.137; // km
  const u = Math.atan(0.99664719 * Math.tan(latRad));
  const rhoSinPhi = 0.99664719 * Math.sin(u) + (elevation / 1000 / earthRadius) * Math.sin(latRad);
  const rhoCosPhi = Math.cos(u) + (elevation / 1000 / earthRadius) * Math.cos(latRad);

  // Parallax in RA and Dec
  const deltaRA = Math.atan2(
    -rhoCosPhi * Math.sin(hpRad) * Math.sin(haRad),
    Math.cos(decRad) - rhoCosPhi * Math.sin(hpRad) * Math.cos(haRad)
  );

  const deltaDec = Math.atan2(
    (Math.sin(decRad) - rhoSinPhi * Math.sin(hpRad)) * Math.cos(deltaRA),
    Math.cos(decRad) - rhoCosPhi * Math.sin(hpRad) * Math.cos(haRad)
  );

  return {
    rightAscension: normalizeDegrees(ra + toDegrees(deltaRA)),
    declination: dec + toDegrees(deltaDec) - dec,
  };
}
