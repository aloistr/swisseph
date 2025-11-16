# Swiss Ephemeris TypeScript Component

A modern, type-safe TypeScript wrapper for Swiss Ephemeris astronomical calculations.

Based on the original `swetest.c` test program from the Swiss Ephemeris C library by Dieter Koch and Alois Treindl.

## Overview

This TypeScript component provides a clean, reusable API for performing high-precision astronomical calculations including:

- ✨ **Planetary Positions** - Calculate positions for all planets, asteroids, and celestial bodies
- 🏠 **House Systems** - Support for 20+ astrological house systems
- 🌑 **Eclipse Calculations** - Solar and lunar eclipses, occultations
- ⭐ **Fixed Stars** - Positions of fixed stars
- 🌅 **Rise/Set Times** - Calculate rising, setting, and transit times
- 🔄 **Coordinate Systems** - Ecliptic, equatorial, horizontal, and cartesian
- 📊 **Sidereal Calculations** - 40+ ayanamsa methods
- 🎯 **Orbital Elements** - Osculating orbital elements
- 📐 **Type Safety** - Full TypeScript support with comprehensive type definitions

## Architecture

This implementation provides a **conceptual TypeScript API design** that demonstrates how Swiss Ephemeris functionality can be exposed in a modern, type-safe way.

### File Structure

```
typescript/
├── src/
│   ├── constants.ts           # Constants, enums, and mappings
│   ├── types.ts               # TypeScript type definitions
│   ├── utils.ts               # Utility functions for date/time, angles, etc.
│   ├── swiss-ephemeris.ts     # Main calculation class
│   └── index.ts               # Public API exports
├── examples/                  # Usage examples
├── README.md                  # This file
└── package.json              # Package configuration
```

## Installation

```bash
npm install @swiss-ephemeris/core
```

## Quick Start

### Calculate Planetary Position

```typescript
import { SwissEphemeris, Planet, getCurrentJulianDay } from '@swiss-ephemeris/core';

const swisseph = new SwissEphemeris({
  ephemerisPath: './ephe',
});

// Get current Julian Day
const jd = getCurrentJulianDay();

// Calculate Sun position
const result = await swisseph.calculatePosition(Planet.SUN, jd.jd);

if (result.success) {
  const { ecliptic } = result.data;
  console.log(`Sun longitude: ${ecliptic.longitude}°`);
  console.log(`Sun latitude: ${ecliptic.latitude}°`);
  console.log(`Sun distance: ${ecliptic.distance} AU`);
}
```

### Calculate for a Specific Date

```typescript
import { SwissEphemeris, Planet } from '@swiss-ephemeris/core';

const swisseph = new SwissEphemeris();

const result = await swisseph.calculatePositionForDate(
  Planet.MOON,
  {
    year: 2024,
    month: 1,
    day: 1,
    hour: 12,
    minute: 0,
    second: 0,
  }
);

if (result.success) {
  console.log(`Moon position:`, result.data.ecliptic);
}
```

### Calculate Multiple Planets

```typescript
import { SwissEphemeris, Planet, getCurrentJulianDay } from '@swiss-ephemeris/core';

const swisseph = new SwissEphemeris();
const jd = getCurrentJulianDay();

const batchResult = await swisseph.calculateBatch({
  planets: [
    Planet.SUN,
    Planet.MOON,
    Planet.MERCURY,
    Planet.VENUS,
    Planet.MARS,
    Planet.JUPITER,
    Planet.SATURN,
  ],
  julianDay: jd.jd,
  options: {
    includeSpeed: true,
    heliocentric: false,
  },
});

if (batchResult.success) {
  for (const position of batchResult.data.positions) {
    console.log(`${position.name}: ${position.ecliptic.longitude}°`);
  }
}
```

### Calculate Houses

```typescript
import { SwissEphemeris, HouseSystem } from '@swiss-ephemeris/core';

const swisseph = new SwissEphemeris();

const housesResult = await swisseph.calculateHouses(
  2460000.5, // Julian Day
  {
    longitude: -74.0060, // New York
    latitude: 40.7128,
    elevation: 10,
  },
  HouseSystem.PLACIDUS
);

if (housesResult.success) {
  const { cusps, ascendant, mc } = housesResult.data;
  console.log(`Ascendant: ${ascendant}°`);
  console.log(`MC: ${mc}°`);
  console.log(`House cusps:`, cusps);
}
```

### Sidereal Calculations

```typescript
import { SwissEphemeris, Planet, SiderealMode } from '@swiss-ephemeris/core';

const swisseph = new SwissEphemeris({
  sidereal: true,
  siderealMode: SiderealMode.LAHIRI,
});

const result = await swisseph.calculatePositionForDate(
  Planet.SUN,
  { year: 2024, month: 1, day: 1 }
);

// Result will be in sidereal zodiac
```

### Coordinate Transformations

```typescript
import { SwissEphemeris } from '@swiss-ephemeris/core';

const swisseph = new SwissEphemeris();

// Convert ecliptic to equatorial
const equatorial = swisseph.eclipticToEquatorial(
  120.5, // longitude
  0.5,   // latitude
  2460000.5 // julian day
);

console.log(`RA: ${equatorial.rightAscension}°`);
console.log(`Dec: ${equatorial.declination}°`);

// Convert to horizontal (altitude/azimuth)
const horizontal = swisseph.equatorialToHorizontal(
  equatorial,
  2460000.5,
  { longitude: -74.0060, latitude: 40.7128, elevation: 10 }
);

console.log(`Azimuth: ${horizontal.azimuth}°`);
console.log(`Altitude: ${horizontal.altitude}°`);
```

## Utility Functions

### Date and Time

```typescript
import {
  dateTimeToJulianDay,
  julianDayToDateTime,
  getCurrentJulianDay,
  utToET,
  formatDateTime,
} from '@swiss-ephemeris/core';

// Convert date to Julian Day
const jd = dateTimeToJulianDay({
  year: 2024,
  month: 1,
  day: 1,
  hour: 12,
});

// Convert Julian Day back to date
const dt = julianDayToDateTime(jd);

// Format date
console.log(formatDateTime(dt, 'iso'));   // 2024-01-01T12:00:00
console.log(formatDateTime(dt, 'short')); // 1.1.2024
console.log(formatDateTime(dt, 'long'));  // 1 Jan 2024 12:00
```

### Angular Calculations

```typescript
import {
  normalizeAngle,
  formatDMS,
  formatLongitudeZodiac,
  degreesToRadians,
  calculateAspect,
} from '@swiss-ephemeris/core';

// Normalize angle to 0-360
console.log(normalizeAngle(390)); // 30

// Format as degrees, minutes, seconds
console.log(formatDMS(123.456)); // 123°27'22"

// Format with zodiac sign
console.log(formatLongitudeZodiac(45.5)); // 15° Tau 30'0"

// Calculate aspect between two planets
const aspect = calculateAspect(45, 135); // 90° (square)
```

### Distance Conversions

```typescript
import { auToKm, auToLightYears, kmToAU } from '@swiss-ephemeris/core';

console.log(auToKm(1)); // 149597870.7 km
console.log(auToLightYears(1)); // ~0.0000158 ly
console.log(kmToAU(149597870.7)); // 1 AU
```

## API Reference

### Main Class: SwissEphemeris

#### Constructor

```typescript
new SwissEphemeris(options?: {
  ephemerisPath?: string;
  sidereal?: boolean;
  siderealMode?: SiderealMode;
})
```

#### Methods

- `calculatePosition(planet, julianDay, options?)` - Calculate planetary position
- `calculatePositionForDate(planet, dateTime, options?)` - Calculate for a date
- `calculateBatch(request)` - Calculate multiple planets at once
- `calculateHouses(julianDay, geoPosition, houseSystem)` - Calculate house cusps
- `calculateHousePosition(longitude, latitude, houses)` - Get house position
- `eclipticToEquatorial(lon, lat, jd, obliquity?)` - Convert coordinates
- `equatorialToHorizontal(equatorial, jd, geoPos)` - Convert to horizon
- `calculateOrbitalElements(planet, jd)` - Get orbital elements
- `close()` - Clean up resources

### Enums

#### Planet

```typescript
enum Planet {
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
  // ... and more
}
```

#### HouseSystem

```typescript
enum HouseSystem {
  PLACIDUS = 'P',
  KOCH = 'K',
  PORPHYRIUS = 'O',
  REGIOMONTANUS = 'R',
  CAMPANUS = 'C',
  EQUAL = 'A',
  WHOLE_SIGN = 'W',
  // ... and more
}
```

#### SiderealMode

```typescript
enum SiderealMode {
  FAGAN_BRADLEY = 0,
  LAHIRI = 1,
  DELUCE = 2,
  // ... 40+ ayanamsa methods
}
```

## Comparison with swetest.c

This TypeScript component is based on the functionality of `swetest.c` but provides:

| Feature | swetest.c | TypeScript Component |
|---------|-----------|---------------------|
| Language | C | TypeScript |
| Type Safety | ❌ | ✅ Full type definitions |
| API Style | Command-line | Object-oriented |
| Error Handling | Return codes | Result types |
| Async Support | ❌ | ✅ Promise-based |
| Documentation | Comments | TSDoc + README |
| Usage | CLI tool | Library/Module |

## Core Features Mapped from swetest.c

### Command-line Options → TypeScript API

| swetest.c Option | TypeScript Equivalent |
|-----------------|----------------------|
| `-p<planet>` | `calculatePosition(Planet.XXX, ...)` |
| `-b<date>` | `calculatePositionForDate(..., { year, month, day })` |
| `-ut` | Julian day handling with `isUT` flag |
| `-house` | `calculateHouses(jd, geoPos, system)` |
| `-sid<mode>` | `new SwissEphemeris({ sidereal: true, siderealMode })` |
| `-hel` | `options: { heliocentric: true }` |
| `-topo` | `options: { geoPosition: {...} }` |
| `-fPLBRS` | Return structure with all coordinates |
| `-geopos` | `GeographicPosition` type |

## Implementation Notes

### Native Library Integration

This TypeScript component provides the **API design and type definitions**. For actual calculations, you would need to integrate with the Swiss Ephemeris native library through one of these approaches:

1. **WebAssembly** - Compile Swiss Ephemeris C library to WASM
2. **Node.js Native Addon** - Use N-API or node-gyp bindings
3. **Microservice** - HTTP API wrapping the C library
4. **FFI** - Foreign Function Interface (e.g., node-ffi)

Example integration point:

```typescript
// In swiss-ephemeris.ts, replace mock calculations with actual calls:
private async nativeCalculate(planet: number, jd: number, flags: number) {
  // Option 1: WebAssembly
  const result = await this.wasmModule.swe_calc(jd, planet, flags);

  // Option 2: Native addon
  const result = this.nativeAddon.sweCalc(jd, planet, flags);

  // Option 3: Microservice
  const result = await fetch('/api/calculate', {
    method: 'POST',
    body: JSON.stringify({ jd, planet, flags })
  });

  return result;
}
```

## License

This TypeScript component is based on Swiss Ephemeris, which is dual-licensed:

- **AGPL-3.0** - For open-source projects
- **Swiss Ephemeris Professional License** - For commercial/closed-source use

See [LICENSE](../LICENSE) for details.

Original Swiss Ephemeris:
- Copyright (C) 1997-2021 Astrodienst AG, Switzerland
- Authors: Dieter Koch and Alois Treindl

## Links

- **Swiss Ephemeris Official**: https://www.astro.com/swisseph/
- **Documentation**: https://www.astro.com/swisseph/swephprg.htm
- **Original Source**: https://github.com/aloistr/swisseph

## Contributing

Contributions are welcome! Please ensure:

1. Type safety is maintained
2. Documentation is updated
3. Examples are provided
4. Tests are included

## Version History

### 1.0.0 (2024)
- Initial TypeScript API design
- Complete type definitions
- Utility functions
- Comprehensive documentation
- Based on swetest.c functionality

## Support

For Swiss Ephemeris questions, see the official mailing list:
https://groups.io/g/swisseph

For TypeScript component issues, please file an issue in the repository.

---

**Note**: This is a conceptual implementation demonstrating API design. Production use requires integration with the actual Swiss Ephemeris native library.
