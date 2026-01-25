# Swiss Ephemeris Rust Migration Plan
## Minimal Horoscope WASM Library

## Executive Summary

This document outlines a focused migration plan to create a **minimal Rust/WASM library** for horoscope generation, replacing the current `sweph-wasm` dependency.

### Current Usage (from Analysis)

```
Functions:     swe_julday(), swe_calc_ut(), swe_houses()
Bodies:        Sun, Moon, Mercury-Pluto, True North Node (12 total)
House System:  Placidus only
Output:        Longitude, speed (for retrograde), house cusps
```

### Target

- **Pure Rust** compiled to WASM
- **No external data files** (self-contained)
- **~50-100 KB** bundle size
- **Drop-in replacement** API for current usage

---

## 1. Exact Requirements

### 1.1 Functions to Implement

| Current Function | Purpose | Implementation |
|-----------------|---------|----------------|
| `swe_julday()` | Date → Julian Day | Simple formula (~20 lines) |
| `swe_calc_ut()` | Planet positions | Moshier analytical ephemeris |
| `swe_houses()` | House cusps | Placidus algorithm |

### 1.2 Celestial Bodies Needed

| ID | Body | Source Algorithm |
|----|------|------------------|
| 0 | Sun | Moshier / VSOP87 |
| 1 | Moon | Moshier (ELP2000-based) |
| 2 | Mercury | Moshier / VSOP87 |
| 3 | Venus | Moshier / VSOP87 |
| 4 | Mars | Moshier / VSOP87 |
| 5 | Jupiter | Moshier / VSOP87 |
| 6 | Saturn | Moshier / VSOP87 |
| 7 | Uranus | Moshier / VSOP87 |
| 8 | Neptune | Moshier / VSOP87 |
| 9 | Pluto | Moshier (special) |
| 11 | True Node | Lunar node calculation |

### 1.3 Output Format

```typescript
// Per planet
{ longitude: number, speed: number }

// Houses
{ cusps: number[13], ascendant: number, mc: number }
```

---

## 2. Implementation Options

### Option A: Port Swiss Ephemeris Moshier Routines (Recommended)

Port the analytical ephemeris code from Swiss Ephemeris:

| Source File | What to Port | Lines |
|-------------|--------------|-------|
| `swedate.c` | Julian day | ~50 |
| `swemplan.c` | Planet positions | ~800 |
| `swemmoon.c` | Moon position | ~1,500 |
| `swehouse.c` | Placidus houses | ~300 |
| `swephlib.c` | Obliquity, sidereal time | ~200 |
| `swemptab.h` | Planetary tables | Data |

**Pros:**
- Battle-tested accuracy (25+ years)
- Already includes Moon, Pluto, nodes
- Self-contained (no file I/O)
- Direct port = predictable results

**Cons:**
- More code to port (~2,800 lines)
- Tables are large (but compress well)

**Estimated WASM size:** ~80-120 KB gzipped

---

### Option B: VSOP87 + ELP2000 + Pluto (More Work)

Use separate theories for each body type:

| Body | Theory | Rust Crate |
|------|--------|------------|
| Sun, Mercury-Neptune | VSOP87 | `vsop87` crate |
| Moon | ELP2000-82 | Need to port or find |
| Pluto | Pluto95 or custom | Need to port |
| True Node | Custom | ~100 lines |

**Pros:**
- Modern, well-documented theories
- Potentially smaller if using truncated series

**Cons:**
- Moon: No good Rust ELP2000 implementation exists
- Pluto: Would need custom implementation
- More integration work
- Different precision characteristics

**Not recommended** due to Moon/Pluto gaps.

---

### Option C: Use Existing Rust Crate

Check for existing implementations:

| Crate | Status | Completeness |
|-------|--------|--------------|
| `astro` | Abandoned | Partial |
| `astronomia` | JavaScript only | N/A |
| `hifitime` + `nyx` | Aerospace focus | Overkill |
| `vsop87` | Active | Planets only, no Moon |

**Conclusion:** No complete Rust crate exists for astrological calculations. We need to build it.

---

## 3. Recommended Approach: Port Moshier

### 3.1 Project Structure

```
horoscope-rs/
├── Cargo.toml
├── src/
│   ├── lib.rs           # Public API: julday, calc, houses
│   ├── julian.rs        # Julian day conversion
│   ├── planets.rs       # Planet calculations (Moshier)
│   ├── moon.rs          # Moon calculations (Moshier)
│   ├── pluto.rs         # Pluto calculations
│   ├── nodes.rs         # Lunar nodes (true node)
│   ├── houses.rs        # Placidus house system
│   ├── sidereal.rs      # Sidereal time, obliquity
│   └── tables/          # Embedded coefficient tables
│       ├── mod.rs
│       ├── sun.rs
│       ├── moon.rs
│       └── planets.rs
└── wasm/
    ├── Cargo.toml
    └── src/lib.rs       # wasm-bindgen exports
```

### 3.2 Public API

```rust
/// Convert calendar date to Julian Day (matches swe_julday)
pub fn julday(year: i32, month: i32, day: i32, hour: f64) -> f64;

/// Calculate planet position (matches swe_calc_ut)
/// Returns [longitude, latitude, distance, speed_lon, speed_lat, speed_dist]
pub fn calc_ut(jd_ut: f64, planet: i32, flags: i32) -> Result<[f64; 6], Error>;

/// Calculate house cusps (matches swe_houses)
/// Returns (cusps[13], ascmc[10])
pub fn houses(jd_ut: f64, lat: f64, lon: f64, hsys: char)
    -> Result<([f64; 13], [f64; 10]), Error>;

// Planet constants (matching Swiss Ephemeris)
pub const SE_SUN: i32 = 0;
pub const SE_MOON: i32 = 1;
pub const SE_MERCURY: i32 = 2;
pub const SE_VENUS: i32 = 3;
pub const SE_MARS: i32 = 4;
pub const SE_JUPITER: i32 = 5;
pub const SE_SATURN: i32 = 6;
pub const SE_URANUS: i32 = 7;
pub const SE_NEPTUNE: i32 = 8;
pub const SE_PLUTO: i32 = 9;
pub const SE_TRUE_NODE: i32 = 11;

// Flags
pub const SEFLG_SPEED: i32 = 256;
```

### 3.3 WASM Bindings

```typescript
// TypeScript interface (drop-in replacement for sweph-wasm)
export function swe_julday(
  year: number, month: number, day: number, hour: number, gregflag: number
): number;

export function swe_calc_ut(
  tjd_ut: number, ipl: number, iflag: number
): { longitude: number; latitude: number; distance: number;
     speedLong: number; speedLat: number; speedDist: number; error?: string };

export function swe_houses(
  tjd_ut: number, lat: number, lon: number, hsys: string
): { cusps: number[]; ascmc: number[]; error?: string };

// Constants
export const SE_GREG_CAL: number;
export const SE_SUN: number;
export const SE_MOON: number;
// ... etc
```

---

## 4. Implementation Phases

### Phase 1: Core Infrastructure
- [ ] Julian day conversion (`julday`)
- [ ] Basic angle/coordinate utilities
- [ ] Obliquity of ecliptic
- [ ] Sidereal time calculation
- [ ] Unit tests against Swiss Ephemeris output

**Files:** `julian.rs`, `sidereal.rs`

### Phase 2: Planet Calculations
- [ ] Port Moshier planetary theory for Sun
- [ ] Port Mercury through Neptune
- [ ] Port Pluto calculation
- [ ] Speed calculations (for retrograde)
- [ ] Accuracy tests against known ephemeris

**Files:** `planets.rs`, `pluto.rs`, `tables/`

### Phase 3: Moon & Nodes
- [ ] Port Moshier lunar theory
- [ ] True lunar node calculation
- [ ] Accuracy tests

**Files:** `moon.rs`, `nodes.rs`

### Phase 4: Houses
- [ ] ARMC (local sidereal time)
- [ ] Ascendant calculation
- [ ] MC calculation
- [ ] Placidus house cusps
- [ ] House tests against Swiss Ephemeris

**Files:** `houses.rs`

### Phase 5: WASM & Integration
- [ ] wasm-bindgen API matching current usage
- [ ] TypeScript type definitions
- [ ] NPM package setup
- [ ] Integration tests with existing codebase
- [ ] Performance benchmarks

**Files:** `wasm/src/lib.rs`, `package.json`

---

## 5. Source Code Mapping

### From `swedate.c`

```c
// Port this function
double swe_julday(int year, int month, int day, double hour, int gregflag)
```

→ Rust:
```rust
pub fn julday(year: i32, month: i32, day: i32, hour: f64) -> f64 {
    // Gregorian calendar assumed
    let a = (14 - month) / 12;
    let y = year + 4800 - a;
    let m = month + 12 * a - 3;

    let jdn = day + (153 * m + 2) / 5 + 365 * y + y / 4 - y / 100 + y / 400 - 32045;
    jdn as f64 + (hour - 12.0) / 24.0
}
```

### From `swemplan.c`

Key functions to port:
- `swi_moshplan()` - Main planetary calculation
- `calc_speed()` - Velocity calculation
- `embofs()` - Earth-Moon barycenter offset

### From `swemmoon.c`

Key functions to port:
- `swi_moshmoon()` - Main lunar calculation
- Uses different coefficient tables than planets

### From `swehouse.c`

Key functions to port:
- `swe_houses()` - Entry point
- `CalcH()` - Calculate single house cusp
- `Asc1()` - Ascendant calculation
- `MC()` - Midheaven calculation

Only implement Placidus (`'P'`) since that's all you use.

---

## 6. Testing Strategy

### 6.1 Generate Reference Data

Use current `sweph-wasm` to generate test fixtures:

```javascript
// generate-fixtures.js
const sweph = require('sweph-wasm');

const testDates = [
  { year: 2000, month: 1, day: 1, hour: 12 },   // J2000
  { year: 2024, month: 6, day: 21, hour: 0 },   // Summer solstice
  { year: 1990, month: 3, day: 15, hour: 6 },   // Random past
  // ... more dates
];

const fixtures = testDates.map(d => {
  const jd = sweph.swe_julday(d.year, d.month, d.day, d.hour, sweph.SE_GREG_CAL);
  const planets = {};

  for (let i = 0; i <= 11; i++) {
    if (i === 10) continue; // Skip mean node
    planets[i] = sweph.swe_calc_ut(jd, i, sweph.SEFLG_SPEED);
  }

  const houses = sweph.swe_houses(jd, 47.38, 8.54, 'P');

  return { date: d, jd, planets, houses };
});

console.log(JSON.stringify(fixtures, null, 2));
```

### 6.2 Rust Tests

```rust
#[cfg(test)]
mod tests {
    use super::*;

    // Load fixtures generated from sweph-wasm
    const FIXTURES: &str = include_str!("../fixtures/reference.json");

    #[test]
    fn test_julday() {
        let jd = julday(2000, 1, 1, 12.0);
        assert!((jd - 2451545.0).abs() < 1e-10);
    }

    #[test]
    fn test_sun_position() {
        let jd = julday(2000, 1, 1, 12.0);
        let pos = calc_ut(jd, SE_SUN, SEFLG_SPEED).unwrap();

        // Compare against fixture
        assert!((pos[0] - 280.373).abs() < 0.001); // longitude
    }

    #[test]
    fn test_all_fixtures() {
        let fixtures: Vec<Fixture> = serde_json::from_str(FIXTURES).unwrap();

        for fix in fixtures {
            let jd = julday(fix.date.year, fix.date.month, fix.date.day, fix.date.hour);
            assert!((jd - fix.jd).abs() < 1e-8);

            for (planet_id, expected) in fix.planets {
                let result = calc_ut(jd, planet_id, SEFLG_SPEED).unwrap();
                assert!((result[0] - expected.longitude).abs() < 0.0001);
                assert!((result[3] - expected.speed_long).abs() < 0.0001);
            }
        }
    }
}
```

---

## 7. Precision Requirements

For astrological purposes:

| Measurement | Required Precision | Moshier Provides |
|-------------|-------------------|------------------|
| Longitude | < 1 arcminute | ~1 arcsecond |
| Speed | < 0.01°/day | ~0.001°/day |
| House cusps | < 1 arcminute | Depends on planet precision |

**Moshier is more than sufficient** for all astrological applications.

---

## 8. Bundle Size Optimization

### Strategies

1. **Truncate coefficient tables** - Use fewer terms for outer planets
2. **LTO + size optimization** - `opt-level = "s"` in release
3. **wasm-opt** - Post-process with Binaryen
4. **Only Placidus** - Skip other house systems

### Expected Sizes

| Component | Estimated Size |
|-----------|---------------|
| Julian day | ~1 KB |
| Planets (Moshier) | ~40 KB |
| Moon (Moshier) | ~30 KB |
| Houses (Placidus) | ~5 KB |
| Tables (compressed) | ~20 KB |
| **Total WASM** | **~100 KB** |
| **Gzipped** | **~40 KB** |

---

## 9. Migration Steps

### Step 1: Build & Test Rust Library
```bash
cargo new horoscope
cd horoscope
# Implement and test
cargo test
```

### Step 2: Build WASM
```bash
cd wasm
wasm-pack build --target web --release
wasm-opt -Os pkg/horoscope_bg.wasm -o pkg/horoscope_bg.wasm
```

### Step 3: Integration Test
```javascript
// Compare outputs side-by-side
import * as sweph from 'sweph-wasm';
import * as horoscope from './horoscope-wasm';

const jd1 = sweph.swe_julday(2024, 1, 1, 12, sweph.SE_GREG_CAL);
const jd2 = horoscope.swe_julday(2024, 1, 1, 12, horoscope.SE_GREG_CAL);
console.assert(Math.abs(jd1 - jd2) < 1e-10);

// ... compare all functions
```

### Step 4: Swap Dependencies
```javascript
// Before
import * as sweph from 'sweph-wasm';

// After (same API)
import * as sweph from 'horoscope-wasm';
```

---

## 10. Deliverables Checklist

### Phase 1-2: Core
- [ ] `julday()` function
- [ ] Obliquity and sidereal time
- [ ] Sun position calculation
- [ ] Mercury through Neptune
- [ ] Pluto calculation
- [ ] Speed calculations

### Phase 3: Moon & Nodes
- [ ] Moon position calculation
- [ ] True lunar node
- [ ] Accuracy tests

### Phase 4: Houses
- [ ] Placidus house calculation
- [ ] Ascendant, MC
- [ ] House cusp tests

### Phase 5: WASM
- [ ] wasm-bindgen exports
- [ ] TypeScript definitions
- [ ] NPM package
- [ ] Drop-in integration test
- [ ] Documentation

---

## Appendix: Key Algorithms Reference

### Julian Day (Gregorian)
```
JD = 367*Y - INT(7*(Y+INT((M+9)/12))/4) + INT(275*M/9) + D + 1721013.5 + H/24
```

### Obliquity of Ecliptic
```
ε = 23°26'21.448" - 46.8150"*T - 0.00059"*T² + 0.001813"*T³
where T = (JD - 2451545.0) / 36525
```

### Placidus Houses
Uses iterative solution for house cusps based on:
- Local sidereal time (ARMC)
- Geographic latitude
- Obliquity of ecliptic

### True Node
Calculated from Moon's orbital elements, accounting for:
- Mean longitude of ascending node
- Perturbations from Sun and planets
