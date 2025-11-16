

 # Swiss Ephemeris - Native TypeScript Implementation

## 🎉 NO C Library Required!

This is a **complete, native TypeScript implementation** of astronomical calculations. Unlike traditional Swiss Ephemeris wrappers, this implementation requires **ZERO** files from the original Swiss Ephemeris C library.

## Features

✅ **Pure TypeScript** - No native bindings, no WebAssembly, no C library
✅ **Planetary Positions** - Sun, Moon, Mercury, Venus, Mars, Jupiter, Saturn, Uranus, Neptune, Pluto
✅ **Lunar Calculations** - Moon position, nodes, apogee
✅ **12+ House Systems** - Placidus, Koch, Porphyrius, Regiomontanus, Campanus, Equal, Whole Sign, and more
✅ **Precession & Nutation** - IAU 2006/2000B models
✅ **Coordinate Transformations** - Ecliptic ↔ Equatorial ↔ Horizontal
✅ **Date/Time Utilities** - Julian Day conversions, sidereal time, Delta T
✅ **Type-Safe** - Full TypeScript type definitions
✅ **Standalone** - Works in Node.js, browsers, Deno, Bun - anywhere TypeScript runs

## Accuracy

| Time Period | Accuracy |
|-------------|----------|
| 1900-2100 | ~1-5 arcminutes for major planets |
| 1900-2100 | ~10 arcseconds for Moon |
| 1700-2200 | ~10 arcminutes for major planets |
| Ancient/Future | Degrades outside tested range |

**Note:** This implementation uses simplified VSOP87 and ELP2000 theories. For research-grade precision (arcsecond level), use the full Swiss Ephemeris C library via WebAssembly bindings.

## Installation

```bash
npm install @swiss-ephemeris/core
```

## Quick Start

```typescript
import {
  SwissEphemerisNative,
  Planet,
  HouseSystem,
  getCurrentJulianDay,
  utToTT,
} from '@swiss-ephemeris/core';

// Create calculator instance
const swisseph = new SwissEphemerisNative();

// Get current Julian Day
const jd = getCurrentJulianDay();
const jdTT = utToTT(jd.jd);

// Calculate Sun position
const result = await swisseph.calculatePosition(Planet.SUN, jdTT);

if (result.success) {
  console.log(`Sun longitude: ${result.data.ecliptic.longitude}°`);
  console.log(`Sun latitude: ${result.data.ecliptic.latitude}°`);
  console.log(`Sun distance: ${result.data.ecliptic.distance} AU`);
}
```

## Examples

### Calculate Planet Positions

```typescript
import { SwissEphemerisNative, Planet } from '@swiss-ephemeris/core';

const swisseph = new SwissEphemerisNative();

// Calculate for a specific date
const result = await swisseph.calculatePositionForDate(
  Planet.MARS,
  {
    year: 2024,
    month: 1,
    day: 1,
    hour: 12,
    minute: 0,
  },
  {
    includeSpeed: true, // Include daily motion
  }
);

if (result.success) {
  const { ecliptic, equatorial } = result.data;

  console.log('Ecliptic:');
  console.log(`  Longitude: ${ecliptic.longitude}°`);
  console.log(`  Latitude: ${ecliptic.latitude}°`);
  console.log(`  Speed: ${ecliptic.longitudeSpeed}°/day`);

  console.log('Equatorial:');
  console.log(`  RA: ${equatorial?.rightAscension}°`);
  console.log(`  Dec: ${equatorial?.declination}°`);
}
```

### Calculate Multiple Planets (Batch)

```typescript
const batch = await swisseph.calculateBatch({
  planets: [
    Planet.SUN,
    Planet.MOON,
    Planet.MERCURY,
    Planet.VENUS,
    Planet.MARS,
    Planet.JUPITER,
    Planet.SATURN,
  ],
  julianDay: jdTT,
  options: {
    includeSpeed: true,
  },
});

if (batch.success) {
  for (const position of batch.data.positions) {
    console.log(`${position.name}: ${position.ecliptic.longitude}°`);
  }
}
```

### Calculate Houses

```typescript
const houses = await swisseph.calculateHouses(
  jd.jd, // Universal Time
  {
    longitude: -74.0060, // New York
    latitude: 40.7128,
    elevation: 10,
  },
  HouseSystem.PLACIDUS
);

if (houses.success) {
  console.log(`Ascendant: ${houses.data.ascendant}°`);
  console.log(`MC: ${houses.data.mc}°`);

  houses.data.cusps.forEach((cusp, i) => {
    console.log(`House ${i + 1}: ${cusp}°`);
  });
}
```

### Complete Birth Chart

```typescript
const chart = await swisseph.calculateBirthChart(
  {
    year: 1990,
    month: 6,
    day: 15,
    hour: 14,
    minute: 30,
  },
  {
    longitude: -0.1278, // London
    latitude: 51.5074,
    elevation: 11,
  },
  HouseSystem.PLACIDUS
);

if (chart.success) {
  // Planet positions
  for (const planet of chart.data.positions) {
    console.log(`${planet.name}: ${planet.ecliptic.longitude}°`);
  }

  // Houses
  if (chart.data.houses) {
    console.log(`Ascendant: ${chart.data.houses.ascendant}°`);
    console.log(`MC: ${chart.data.houses.mc}°`);
  }
}
```

### Coordinate Transformations

```typescript
import {
  eclipticToEquatorial,
  equatorialToHorizontal,
  calculateLMST,
} from '@swiss-ephemeris/core';

// Ecliptic to Equatorial
const equatorial = eclipticToEquatorial(
  120.5, // longitude
  0.5,   // latitude
  23.44  // obliquity
);

console.log(`RA: ${equatorial.rightAscension}°`);
console.log(`Dec: ${equatorial.declination}°`);

// Equatorial to Horizontal
const lst = calculateLMST(jd.jd, -74.0060); // New York

const horizontal = equatorialToHorizontal(
  equatorial.rightAscension,
  equatorial.declination,
  lst,
  40.7128 // latitude
);

console.log(`Azimuth: ${horizontal.azimuth}°`);
console.log(`Altitude: ${horizontal.altitude}°`);
```

## Implementation Details

### Core Modules

| Module | Description |
|--------|-------------|
| `astronomical-constants.ts` | Physical and astronomical constants |
| `date-time.ts` | Julian Day, sidereal time, Delta T |
| `precession-nutation.ts` | IAU 2006 precession, IAU 2000B nutation |
| `planetary-calculator.ts` | VSOP87 simplified planetary theory |
| `moon-calculator.ts` | ELP2000 simplified lunar theory |
| `house-systems.ts` | 12+ house system implementations |
| `coordinate-transformations.ts` | Coordinate system conversions |
| `native-calculator.ts` | Main calculator wrapper |

### Algorithms Used

#### Planetary Positions
- **VSOP87 Simplified Theory** (Simon et al. 1994)
- Mean orbital elements at J2000
- Kepler's equation solver (Newton-Raphson)
- Perturbation corrections (Jupiter-Saturn great inequality)

#### Moon Position
- **ELP2000 Simplified** (Chapront)
- ~60 periodic terms for longitude
- ~30 periodic terms for latitude
- Meeus Chapter 47 implementation

#### Precession
- **IAU 2006 Precession Model** (Capitaine et al.)
- 5-term polynomial for high accuracy
- Precession matrix calculation

#### Nutation
- **IAU 2000B Nutation** (77-term model)
- Fundamental arguments
- Periodic terms with eccentricity corrections

#### House Systems
- **Placidus** - Time-based division (most popular)
- **Koch** - Birthplace system
- **Porphyrius** - Space-based division
- **Regiomontanus** - Rational method
- **Campanus** - Prime vertical division
- **Equal** - 30° divisions from Ascendant
- **Whole Sign** - One house = one zodiac sign
- **Meridian** - Axial rotation
- **And more...**

### Delta T Calculation

Uses polynomial fits from:
- Morrison & Stephenson (2004)
- Espenak & Meeus (2006)

Accurate from -500 to +3000 CE

## Architecture

```
Core Layer (Pure Calculations)
├── astronomical-constants.ts
├── date-time.ts
├── precession-nutation.ts
├── planetary-calculator.ts
├── moon-calculator.ts
├── house-systems.ts
├── coordinate-transformations.ts
└── native-calculator.ts

API Layer (User Interface)
├── swiss-ephemeris-native.ts
├── constants.ts
├── types.ts
└── utils.ts

Examples & Documentation
├── examples/
├── NATIVE_IMPLEMENTATION.md
└── README.md
```

## Comparison with Swiss Ephemeris C Library

| Feature | Native TS | Swiss Ephemeris C |
|---------|-----------|-------------------|
| **Accuracy** | ~1-10 arcminutes | ~0.001 arcseconds |
| **Speed** | Fast (pure JS) | Very fast (C) |
| **Dependencies** | None | C library + ephemeris files |
| **Setup** | `npm install` | Compile C + download files |
| **Platforms** | Any JS runtime | Platform-specific |
| **File Size** | ~100KB | ~5MB + data files |
| **Asteroids** | No | Yes (100,000+) |
| **Fixed Stars** | No | Yes (all catalogs) |
| **Eclipses** | No | Yes (detailed) |
| **Historic Range** | 1700-2200 | -13000 to +17000 |
| **Best For** | Modern charts | Research & historic |

## When to Use This vs. Full Swiss Ephemeris

### Use Native TypeScript When:
- ✅ Calculating modern era charts (1900-2100)
- ✅ Need simple setup (no compilation)
- ✅ Working in browsers or edge environments
- ✅ Accuracy of 1-10 arcminutes is acceptable
- ✅ Want zero external dependencies
- ✅ Need portable, cross-platform code

### Use Full Swiss Ephemeris When:
- ✅ Need research-grade accuracy (arcsecond level)
- ✅ Calculating ancient or far-future dates
- ✅ Need asteroids or fixed stars
- ✅ Require detailed eclipse calculations
- ✅ Working on historical astronomy research
- ✅ Have access to compile C code or use WebAssembly

## Limitations

Current native implementation does NOT include:
- ❌ Asteroids (Chiron, Ceres, etc.)
- ❌ Fixed stars
- ❌ Eclipse calculations
- ❌ Rise/set/transit times
- ❌ Heliacal phenomena
- ❌ Full sidereal modes (ayanamsa calculations)
- ❌ Topocentric corrections
- ❌ Full VSOP87 (thousands of terms)

These features require either:
1. Future implementation in TypeScript
2. Integration with C library via WebAssembly
3. External astronomical libraries

## Contributing

Contributions welcome! Priority areas:
- Eclipse calculation algorithms
- Rise/set/transit calculations
- More house systems
- Sidereal/ayanamsa implementations
- Improved accuracy (more VSOP87/ELP2000 terms)
- Performance optimizations

## License

AGPL-3.0 - Same as Swiss Ephemeris

This is a clean-room implementation based on published astronomical algorithms and does not include any code from the Swiss Ephemeris C library.

## References

### Algorithms
- Meeus, J. (1998). *Astronomical Algorithms*. Willmann-Bell.
- Simon, J.L. et al. (1994). *Numerical expressions for precession formulae and mean elements for the Moon and planets*. A&A 282, 663-683.
- Capitaine, N. et al. (2003). *Expressions for IAU 2000 precession quantities*. A&A 412, 567-586.
- Chapront, J. et al. (2002). *A new determination of lunar orbital parameters*. A&A 387, 700-709.

### Standards
- IAU 2006 Precession
- IAU 2000B Nutation
- VSOP87 Theory
- ELP2000 Lunar Theory

### Swiss Ephemeris
- Original: https://www.astro.com/swisseph/
- Authors: Dieter Koch and Alois Treindl
- License: AGPL-3.0 / Professional License

## Support

For issues with the native TypeScript implementation, please file an issue in the repository.

For Swiss Ephemeris questions, see: https://groups.io/g/swisseph

---

**Made with ❤️ in TypeScript** - No C compilation required!
