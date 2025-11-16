# Swiss Ephemeris TypeScript Component - Architecture

## Overview

This document describes the architecture of the TypeScript component based on the Swiss Ephemeris `swetest.c` program.

## Design Philosophy

The TypeScript component transforms the procedural, command-line interface of `swetest.c` into a modern, object-oriented, type-safe library suitable for integration into web applications, Node.js services, and TypeScript projects.

## Core Architectural Decisions

### 1. From Procedural to Object-Oriented

**swetest.c Approach:**
```c
// Command-line parsing and direct function calls
int main(int argc, char *argv[]) {
  // Parse command-line arguments
  // Call swe_calc() directly
  iflgret = swe_calc(te, ipl, iflag, x, serr);
  // Format and print output
}
```

**TypeScript Approach:**
```typescript
// Clean class-based API with method chaining
const swisseph = new SwissEphemeris({
  sidereal: true,
  siderealMode: SiderealMode.LAHIRI
});

const result = await swisseph.calculatePosition(
  Planet.SUN,
  julianDay,
  { includeSpeed: true }
);
```

### 2. Type Safety

**swetest.c:**
- Uses `int` for planet numbers, flags, and error codes
- Magic numbers throughout (e.g., `0` for Sun, `256` for speed flag)
- Array indices for coordinate access (`x[0]`, `x[1]`, `x[2]`)
- String-based error handling

**TypeScript:**
- Strongly typed enums for all constants
- Named interfaces for all data structures
- Compile-time type checking
- Discriminated unions for results

```typescript
// Instead of magic numbers:
const result = await swisseph.calculatePosition(Planet.SUN, jd);
// result.data.ecliptic.longitude (not x[0])

// Type-safe flags:
const flags = CalculationFlag.SPEED | CalculationFlag.HELCTR;

// Result types with error handling:
type Result<T> =
  | { success: true; data: T }
  | { success: false; error: CalculationError };
```

### 3. Async/Promise-Based API

The TypeScript component uses async/await patterns suitable for:
- Web applications (non-blocking UI)
- WebAssembly integration
- Microservice architectures
- Node.js applications

```typescript
// All calculations are async
const result = await swisseph.calculatePosition(Planet.MARS, jd);

// Batch operations
const batch = await swisseph.calculateBatch({
  planets: [Planet.SUN, Planet.MOON],
  julianDay: jd
});
```

### 4. Separation of Concerns

The component is organized into logical modules:

```
constants.ts     → Enums, constants, and mappings
types.ts         → Type definitions and interfaces
utils.ts         → Helper functions (date, angles, formatting)
swiss-ephemeris.ts → Main calculation class
index.ts         → Public API exports
```

Compare to `swetest.c` which has everything in one ~5000 line file.

## Module Structure

### constants.ts

Defines all enums and constants from the C library:

- `Planet` enum (SE_SUN, SE_MOON, etc.)
- `HouseSystem` enum (Placidus, Koch, etc.)
- `CalculationFlag` enum (SEFLG_SPEED, SEFLG_HELCTR, etc.)
- `SiderealMode` enum (ayanamsa methods)
- Unit conversion constants

**Mapping Example:**

| C Constant | TypeScript Enum |
|------------|----------------|
| `SE_SUN 0` | `Planet.SUN = 0` |
| `SEFLG_SPEED 256` | `CalculationFlag.SPEED = 256` |
| `'P'` (Placidus) | `HouseSystem.PLACIDUS = 'P'` |

### types.ts

Defines all data structures with proper TypeScript types:

**swetest.c** uses arrays:
```c
double x[6];  // x[0]=lon, x[1]=lat, x[2]=dist, x[3-5]=speeds
```

**TypeScript** uses interfaces:
```typescript
interface EclipticCoordinates {
  longitude: number;
  latitude: number;
  distance: number;
  longitudeSpeed?: number;
  latitudeSpeed?: number;
  distanceSpeed?: number;
}
```

### utils.ts

Utility functions for common operations:

**Date/Time:**
- `dateTimeToJulianDay()` - Replaces `swe_julday()`
- `julianDayToDateTime()` - Replaces `swe_revjul()`
- `getCurrentJulianDay()` - Get current JD

**Angular:**
- `normalizeAngle()` - Normalize to 0-360
- `formatDMS()` - Format as degrees/minutes/seconds
- `formatLongitudeZodiac()` - Format with zodiac signs

**Conversions:**
- `auToKm()`, `auToLightYears()`
- `degreesToRadians()`, `radiansToDegrees()`

### swiss-ephemeris.ts

Main calculation class that encapsulates all Swiss Ephemeris functionality:

**Key Methods:**

1. **Core Calculations**
   - `calculatePosition()` - Wraps `swe_calc()`
   - `calculatePositionForDate()` - Convenience method
   - `calculateBatch()` - Multiple planets at once

2. **Houses**
   - `calculateHouses()` - Wraps `swe_houses()`
   - `calculateHousePosition()` - Wraps `swe_house_pos()`

3. **Coordinate Transformations**
   - `eclipticToEquatorial()`
   - `equatorialToHorizontal()`

4. **Special Events** (require native library)
   - `findNextSolarEclipse()` - Wraps `swe_sol_eclipse_when_glob()`
   - `calculateRiseSet()` - Wraps `swe_rise_trans()`
   - `calculateFixedStar()` - Wraps `swe_fixstar2()`

## Data Flow

### swetest.c Flow:

```
Command Line Args → Parse → swe_calc() → Format → stdout
```

### TypeScript Flow:

```
API Call → Options Object → Build Flags →
Native Calculation → Parse Result → Return Typed Object
```

## Feature Comparison Matrix

| Feature | swetest.c | TypeScript Component |
|---------|-----------|---------------------|
| **Interface** | Command-line | Library API |
| **Input** | argc/argv parsing | Typed parameters |
| **Output** | Formatted strings to stdout | Typed return objects |
| **Error Handling** | Return codes + stderr | Result<T> type with errors |
| **Flags** | Bitwise integer | Enum OR'd together |
| **Planets** | Integer codes | Enum values |
| **Houses** | Letter codes | Enum values |
| **Dates** | String parsing | DateTime interface |
| **Coordinates** | Arrays (x[0], x[1], x[2]) | Named interfaces |
| **Async** | Synchronous | Promise-based async |
| **Type Safety** | None (C) | Full TypeScript types |
| **Documentation** | Comments | TSDoc + README |
| **Testing** | Manual CLI testing | Unit/integration tests |

## Command-Line to API Mapping

### Example 1: Basic Planet Position

**swetest.c:**
```bash
swetest -p0 -b1.1.2024 -ut12:0:0 -fPLBRS
```

**TypeScript:**
```typescript
const result = await swisseph.calculatePositionForDate(
  Planet.SUN,
  { year: 2024, month: 1, day: 1, hour: 12, minute: 0 },
  { includeSpeed: true }
);
```

### Example 2: Houses

**swetest.c:**
```bash
swetest -b1.1.2024 -ut12:0 -house-74.0060,40.7128,P
```

**TypeScript:**
```typescript
const houses = await swisseph.calculateHouses(
  julianDay,
  { longitude: -74.0060, latitude: 40.7128, elevation: 0 },
  HouseSystem.PLACIDUS
);
```

### Example 3: Sidereal

**swetest.c:**
```bash
swetest -p0 -b1.1.2024 -sid1
```

**TypeScript:**
```typescript
const swisseph = new SwissEphemeris({
  sidereal: true,
  siderealMode: SiderealMode.LAHIRI
});
const result = await swisseph.calculatePositionForDate(...);
```

### Example 4: Heliocentric

**swetest.c:**
```bash
swetest -p4 -b1.1.2024 -hel
```

**TypeScript:**
```typescript
const result = await swisseph.calculatePosition(
  Planet.MARS,
  julianDay,
  { heliocentric: true }
);
```

## Native Library Integration Strategy

The TypeScript component is designed to integrate with the native C library through several possible approaches:

### Option 1: WebAssembly (Recommended for Web)

```typescript
// Compile Swiss Ephemeris to WASM
private async nativeCalculate(planet: number, jd: number, flags: number) {
  const result = await this.wasmModule.ccall(
    'swe_calc',
    'number',
    ['number', 'number', 'number', 'number', 'number'],
    [jd, planet, flags, xPtr, serrPtr]
  );
  return this.parseResult(result, xPtr);
}
```

### Option 2: Node.js Native Addon (N-API)

```typescript
// Use node-gyp or N-API bindings
import { sweCalc } from './build/Release/swisseph-native';

private async nativeCalculate(planet: number, jd: number, flags: number) {
  return sweCalc(jd, planet, flags);
}
```

### Option 3: Microservice API

```typescript
// HTTP API wrapper around C library
private async nativeCalculate(planet: number, jd: number, flags: number) {
  const response = await fetch('/api/swisseph/calc', {
    method: 'POST',
    body: JSON.stringify({ jd, planet, flags })
  });
  return response.json();
}
```

## Error Handling

### swetest.c Approach:

```c
iflgret = swe_calc(te, ipl, iflag, x, serr);
if (iflgret < 0) {
  fprintf(stderr, "error: %s\n", serr);
  return ERR;
}
```

### TypeScript Approach:

```typescript
const result = await swisseph.calculatePosition(planet, jd);

if (result.success) {
  // Use result.data
  console.log(result.data.ecliptic.longitude);
} else {
  // Handle result.error
  console.error(result.error.message);
  console.error(result.error.code);
}
```

## Performance Considerations

1. **Batching**: The `calculateBatch()` method allows calculating multiple planets in one call, reducing overhead

2. **Caching**: Implementations can cache frequently-used calculations

3. **Async Operations**: Non-blocking operations allow UI to remain responsive

4. **Lazy Loading**: Can dynamically import modules only when needed

## Extension Points

The component is designed for extensibility:

1. **Custom Planets**: Support for asteroids, fixed stars, hypothetical bodies
2. **Custom Calculations**: Extend `SwissEphemeris` class with new methods
3. **Custom Formatting**: Utility functions can be extended
4. **Custom Ayanamsas**: Support for user-defined sidereal modes

## Testing Strategy

Unlike the monolithic `swetest.c`, the modular design enables:

1. **Unit Tests**: Test each utility function independently
2. **Integration Tests**: Test calculation methods with known values
3. **Type Tests**: Ensure type safety at compile time
4. **Mock Tests**: Test without native library using mocks

Example:

```typescript
describe('DateUtils', () => {
  test('dateTimeToJulianDay converts correctly', () => {
    const jd = dateTimeToJulianDay({
      year: 2000, month: 1, day: 1, hour: 12
    });
    expect(jd.jd).toBeCloseTo(2451545.0);
  });
});
```

## Future Enhancements

Potential additions to the component:

1. **Reactive Streams**: Observable-based APIs for continuous calculations
2. **Worker Pool**: Web Workers for parallel calculations
3. **Chart Visualization**: SVG/Canvas chart rendering
4. **Aspect Calculation**: Built-in aspect detection
5. **Transit Detection**: Automated transit calculations
6. **Ephemeris Table Generation**: Bulk data generation
7. **Storage Integration**: Save/load calculations

## Conclusion

The TypeScript component maintains the computational power of Swiss Ephemeris while providing:

- **Modern API**: Object-oriented, promise-based interface
- **Type Safety**: Compile-time error detection
- **Developer Experience**: IntelliSense, autocomplete, inline documentation
- **Maintainability**: Modular, testable code structure
- **Flexibility**: Multiple integration options
- **Performance**: Async, batchable operations

This architecture bridges the gap between the battle-tested Swiss Ephemeris C library and modern TypeScript applications.
