/**
 * Swiss Ephemeris TypeScript - Basic Usage Examples
 *
 * This file demonstrates how to use the Swiss Ephemeris TypeScript component
 * for common astronomical and astrological calculations.
 */

import {
  SwissEphemeris,
  Planet,
  HouseSystem,
  SiderealMode,
  dateTimeToJulianDay,
  getCurrentJulianDay,
  formatLongitudeZodiac,
  formatDateTime,
  formatDMS,
} from '../src/index';

// ============================================================================
// Example 1: Calculate Sun Position for Current Date
// ============================================================================

async function example1_CurrentSunPosition() {
  console.log('\n=== Example 1: Current Sun Position ===\n');

  const swisseph = new SwissEphemeris();
  const jd = getCurrentJulianDay();

  const result = await swisseph.calculatePosition(Planet.SUN, jd.jd);

  if (result.success) {
    const { ecliptic, name } = result.data;
    console.log(`${name} Position:`);
    console.log(`  Longitude: ${ecliptic.longitude.toFixed(6)}°`);
    console.log(`  Formatted: ${formatLongitudeZodiac(ecliptic.longitude)}`);
    console.log(`  Latitude:  ${ecliptic.latitude.toFixed(6)}°`);
    console.log(`  Distance:  ${ecliptic.distance.toFixed(6)} AU`);
  } else {
    console.error('Calculation failed:', result.error.message);
  }
}

// ============================================================================
// Example 2: Calculate All Major Planets for a Specific Date
// ============================================================================

async function example2_PlanetaryPositions() {
  console.log('\n=== Example 2: Planetary Positions for Jan 1, 2024 ===\n');

  const swisseph = new SwissEphemeris();

  // Define the date
  const dateTime = {
    year: 2024,
    month: 1,
    day: 1,
    hour: 12,
    minute: 0,
    second: 0,
  };

  console.log(`Date: ${formatDateTime(dateTime, 'long')}\n`);

  // List of planets to calculate
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

  const jd = dateTimeToJulianDay(dateTime);

  // Calculate all planets
  const batchResult = await swisseph.calculateBatch({
    planets,
    julianDay: jd.jd,
    options: {
      includeSpeed: true,
    },
  });

  if (batchResult.success) {
    console.log('Planet Positions:\n');
    for (const position of batchResult.data.positions) {
      const lon = position.ecliptic.longitude;
      const speed = position.ecliptic.longitudeSpeed || 0;
      console.log(
        `${position.name.padEnd(10)}: ${formatLongitudeZodiac(lon).padEnd(20)} (${speed.toFixed(4)}°/day)`
      );
    }
  }
}

// ============================================================================
// Example 3: Calculate Houses for a Geographic Location
// ============================================================================

async function example3_HouseCalculation() {
  console.log('\n=== Example 3: House Calculation ===\n');

  const swisseph = new SwissEphemeris();

  // New York City coordinates
  const geoPosition = {
    longitude: -74.0060,
    latitude: 40.7128,
    elevation: 10,
  };

  const dateTime = {
    year: 2024,
    month: 1,
    day: 1,
    hour: 12,
    minute: 0,
  };

  const jd = dateTimeToJulianDay(dateTime);

  console.log(`Location: New York City`);
  console.log(`Date: ${formatDateTime(dateTime, 'long')}\n`);

  // Calculate houses using Placidus system
  const housesResult = await swisseph.calculateHouses(
    jd.jd,
    geoPosition,
    HouseSystem.PLACIDUS
  );

  if (housesResult.success) {
    const { cusps, ascendant, mc } = housesResult.data;

    console.log(`House System: Placidus`);
    console.log(`\nAngles:`);
    console.log(`  Ascendant: ${formatLongitudeZodiac(ascendant)}`);
    console.log(`  MC:        ${formatLongitudeZodiac(mc)}`);

    console.log(`\nHouse Cusps:`);
    cusps.forEach((cusp, index) => {
      console.log(`  House ${(index + 1).toString().padStart(2)}: ${formatLongitudeZodiac(cusp)}`);
    });
  }
}

// ============================================================================
// Example 4: Sidereal (Vedic) Calculations
// ============================================================================

async function example4_SiderealCalculation() {
  console.log('\n=== Example 4: Sidereal (Vedic) Calculation ===\n');

  // Create instance with Lahiri ayanamsa
  const swisseph = new SwissEphemeris({
    sidereal: true,
    siderealMode: SiderealMode.LAHIRI,
  });

  const dateTime = {
    year: 2024,
    month: 1,
    day: 1,
    hour: 12,
  };

  const jd = dateTimeToJulianDay(dateTime);

  console.log(`Using Lahiri Ayanamsa\n`);

  const planets = [Planet.SUN, Planet.MOON, Planet.MARS];

  const batchResult = await swisseph.calculateBatch({
    planets,
    julianDay: jd.jd,
  });

  if (batchResult.success) {
    console.log('Sidereal Positions:\n');
    for (const position of batchResult.data.positions) {
      const lon = position.ecliptic.longitude;
      console.log(`${position.name.padEnd(10)}: ${formatLongitudeZodiac(lon)}`);
    }
  }
}

// ============================================================================
// Example 5: Coordinate Transformations
// ============================================================================

async function example5_CoordinateTransformation() {
  console.log('\n=== Example 5: Coordinate Transformations ===\n');

  const swisseph = new SwissEphemeris();

  // Calculate Sun position
  const jd = getCurrentJulianDay();
  const result = await swisseph.calculatePosition(Planet.SUN, jd.jd);

  if (result.success) {
    const { ecliptic } = result.data;

    console.log('Ecliptic Coordinates:');
    console.log(`  Longitude: ${formatDMS(ecliptic.longitude)}`);
    console.log(`  Latitude:  ${formatDMS(ecliptic.latitude)}`);

    // Convert to equatorial
    const equatorial = swisseph.eclipticToEquatorial(
      ecliptic.longitude,
      ecliptic.latitude,
      jd.jd
    );

    console.log('\nEquatorial Coordinates:');
    console.log(`  Right Ascension: ${formatDMS(equatorial.rightAscension)}`);
    console.log(`  Declination:     ${formatDMS(equatorial.declination)}`);

    // Convert to horizontal (for New York)
    const geoPosition = {
      longitude: -74.0060,
      latitude: 40.7128,
      elevation: 10,
    };

    const horizontal = swisseph.equatorialToHorizontal(
      equatorial,
      jd.jd,
      geoPosition
    );

    console.log('\nHorizontal Coordinates (New York):');
    console.log(`  Azimuth:  ${formatDMS(horizontal.azimuth)}`);
    console.log(`  Altitude: ${formatDMS(horizontal.altitude)}`);
  }
}

// ============================================================================
// Example 6: Heliocentric vs Geocentric
// ============================================================================

async function example6_HeliocentricVsGeocentric() {
  console.log('\n=== Example 6: Heliocentric vs Geocentric ===\n');

  const swisseph = new SwissEphemeris();
  const jd = getCurrentJulianDay();

  // Geocentric Mars
  const geocentric = await swisseph.calculatePosition(Planet.MARS, jd.jd, {
    heliocentric: false,
  });

  // Heliocentric Mars
  const heliocentric = await swisseph.calculatePosition(Planet.MARS, jd.jd, {
    heliocentric: true,
  });

  if (geocentric.success && heliocentric.success) {
    console.log('Mars Position:\n');
    console.log(`Geocentric:  ${formatLongitudeZodiac(geocentric.data.ecliptic.longitude)}`);
    console.log(`Heliocentric: ${formatLongitudeZodiac(heliocentric.data.ecliptic.longitude)}`);

    const diff = Math.abs(
      geocentric.data.ecliptic.longitude - heliocentric.data.ecliptic.longitude
    );
    console.log(`\nDifference: ${diff.toFixed(2)}°`);
  }
}

// ============================================================================
// Example 7: Birth Chart Calculation
// ============================================================================

async function example7_BirthChart() {
  console.log('\n=== Example 7: Complete Birth Chart ===\n');

  const swisseph = new SwissEphemeris();

  // Birth data
  const birthData = {
    dateTime: {
      year: 1990,
      month: 6,
      day: 15,
      hour: 14,
      minute: 30,
    },
    location: {
      name: 'London, UK',
      longitude: -0.1278,
      latitude: 51.5074,
      elevation: 11,
    },
  };

  console.log(`Birth Data:`);
  console.log(`  Date: ${formatDateTime(birthData.dateTime, 'long')}`);
  console.log(`  Location: ${birthData.location.name}\n`);

  const jd = dateTimeToJulianDay(birthData.dateTime);

  // Calculate all planets
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
    Planet.CHIRON,
  ];

  const chartResult = await swisseph.calculateBatch({
    planets,
    julianDay: jd.jd,
    options: {
      geoPosition: birthData.location,
      houseSystem: HouseSystem.PLACIDUS,
      includeSpeed: true,
    },
  });

  if (chartResult.success) {
    const { positions, houses } = chartResult.data;

    console.log('=== PLANETARY POSITIONS ===\n');
    for (const position of positions) {
      const lon = formatLongitudeZodiac(position.ecliptic.longitude);
      const speed = position.ecliptic.longitudeSpeed?.toFixed(4) || 'N/A';
      const houseNum = houses
        ? swisseph.calculateHousePosition(
            position.ecliptic.longitude,
            position.ecliptic.latitude,
            houses
          ).toFixed(2)
        : 'N/A';

      console.log(
        `${position.name.padEnd(12)}: ${lon.padEnd(20)} House ${houseNum.padStart(5)} (${speed}°/day)`
      );
    }

    if (houses) {
      console.log('\n=== HOUSE CUSPS ===\n');
      console.log(`Ascendant: ${formatLongitudeZodiac(houses.ascendant)}`);
      console.log(`MC:        ${formatLongitudeZodiac(houses.mc)}`);
      console.log();
      houses.cusps.forEach((cusp, index) => {
        console.log(`House ${(index + 1).toString().padStart(2)}: ${formatLongitudeZodiac(cusp)}`);
      });
    }
  }
}

// ============================================================================
// Run All Examples
// ============================================================================

async function runAllExamples() {
  console.log('\n');
  console.log('╔═══════════════════════════════════════════════════════════════╗');
  console.log('║     Swiss Ephemeris TypeScript - Usage Examples              ║');
  console.log('║     Based on swetest.c by Dieter Koch and Alois Treindl      ║');
  console.log('╚═══════════════════════════════════════════════════════════════╝');

  try {
    await example1_CurrentSunPosition();
    await example2_PlanetaryPositions();
    await example3_HouseCalculation();
    await example4_SiderealCalculation();
    await example5_CoordinateTransformation();
    await example6_HeliocentricVsGeocentric();
    await example7_BirthChart();

    console.log('\n\n✓ All examples completed successfully!\n');
  } catch (error) {
    console.error('\n✗ Error running examples:', error);
  }
}

// Run examples if executed directly
if (require.main === module) {
  runAllExamples();
}

export {
  example1_CurrentSunPosition,
  example2_PlanetaryPositions,
  example3_HouseCalculation,
  example4_SiderealCalculation,
  example5_CoordinateTransformation,
  example6_HeliocentricVsGeocentric,
  example7_BirthChart,
};
