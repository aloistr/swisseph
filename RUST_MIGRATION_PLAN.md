# Swiss Ephemeris Rust Migration Plan
## Minimal Horoscope WASM Library

## Executive Summary

This document outlines a focused migration plan to create a **minimal Rust/WASM library** for horoscope generation. The scope is intentionally limited to reduce complexity and bundle size.

**Scope:**
- Planets: Sun, Moon, Mercury, Venus, Mars, Jupiter, Saturn, Uranus, Neptune, Pluto
- Zodiac sign positions (tropical zodiac)
- House calculations (Ascendant, MC, house cusps)
- Aspects between planets
- WASM-first, no C FFI or backwards compatibility

**Out of Scope:**
- Asteroids, fixed stars, lunar nodes
- Eclipses, heliacal phenomena
- Sidereal zodiac modes
- JPL/Swiss Ephemeris binary file support
- C FFI compatibility layer

---

## 1. What We Need for Horoscopes

A typical horoscope requires:

| Feature | Description | Priority |
|---------|-------------|----------|
| **Planet Positions** | Longitude of Sun, Moon, and planets in zodiac | Essential |
| **Zodiac Signs** | Which sign (Aries-Pisces) each planet occupies | Essential |
| **Houses** | 12-house division based on birth time/place | Essential |
| **Ascendant/MC** | Rising sign and Midheaven | Essential |
| **Aspects** | Angular relationships between planets | Essential |
| **Retrograde** | Whether planets appear to move backward | Nice-to-have |

---

## 2. Simplified Architecture

### 2.1 What to Port

We only need a subset of the Swiss Ephemeris:

| Original Module | What We Need | Estimated Size |
|-----------------|--------------|----------------|
| `swedate.c` | Julian day conversion | ~200 lines |
| `swephlib.c` | Coordinate transforms, obliquity | ~500 lines |
| `swemmoon.c` | Moon position (Moshier) | ~1,500 lines |
| `swemplan.c` | Planet positions (Moshier) | ~800 lines |
| `swehouse.c` | House calculations (subset) | ~400 lines |
| `swemptab.h` | Planetary tables (embedded) | Data only |

**Total: ~3,500 lines of Rust** (vs 56,000 lines for full library)

### 2.2 What We Skip

- `sweph.c` - Binary ephemeris file reading (use Moshier analytical instead)
- `swejpl.c` - JPL file support
- `swecl.c` - Eclipses
- `swehel.c` - Heliacal phenomena
- `swevents.c` - Event finding

### 2.3 Moshier vs Swiss Ephemeris Files

The Swiss Ephemeris has two calculation modes:
1. **Swiss Ephemeris files** (.se1) - High precision, requires data files
2. **Moshier analytical** - Self-contained, slightly lower precision

For horoscopes, **Moshier is sufficient** (accuracy ~1 arc-second for modern dates). This eliminates all file I/O and makes WASM deployment trivial.

---

## 3. Project Structure

```
horoscope-rs/
├── Cargo.toml
├── src/
│   ├── lib.rs              # Public API
│   ├── date.rs             # Julian day conversions
│   ├── math.rs             # Coordinate transforms, angle math
│   ├── planets.rs          # Planet position calculations
│   ├── moon.rs             # Lunar calculations
│   ├── houses.rs           # House system calculations
│   ├── aspects.rs          # Aspect calculations
│   ├── zodiac.rs           # Zodiac signs and degrees
│   └── tables.rs           # Embedded astronomical tables
├── wasm/
│   ├── Cargo.toml          # WASM-specific build
│   ├── src/
│   │   └── lib.rs          # wasm-bindgen exports
│   └── pkg/                # Generated WASM package
├── tests/
│   └── accuracy.rs         # Compare against known ephemeris data
└── examples/
    └── birth_chart.rs
```

---

## 4. Core Types

### 4.1 Planets

```rust
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
#[cfg_attr(feature = "wasm", wasm_bindgen)]
pub enum Planet {
    Sun,
    Moon,
    Mercury,
    Venus,
    Mars,
    Jupiter,
    Saturn,
    Uranus,
    Neptune,
    Pluto,
}

impl Planet {
    pub fn all() -> &'static [Planet] {
        &[
            Planet::Sun, Planet::Moon, Planet::Mercury, Planet::Venus,
            Planet::Mars, Planet::Jupiter, Planet::Saturn,
            Planet::Uranus, Planet::Neptune, Planet::Pluto,
        ]
    }
}
```

### 4.2 Zodiac Signs

```rust
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[cfg_attr(feature = "wasm", wasm_bindgen)]
pub enum ZodiacSign {
    Aries = 0,
    Taurus = 1,
    Gemini = 2,
    Cancer = 3,
    Leo = 4,
    Virgo = 5,
    Libra = 6,
    Scorpio = 7,
    Sagittarius = 8,
    Capricorn = 9,
    Aquarius = 10,
    Pisces = 11,
}

impl ZodiacSign {
    pub fn from_longitude(lon: f64) -> Self {
        let normalized = lon.rem_euclid(360.0);
        let index = (normalized / 30.0) as usize;
        // Convert index to enum variant
        unsafe { std::mem::transmute(index as u8) }
    }

    pub fn symbol(&self) -> &'static str {
        match self {
            Self::Aries => "♈", Self::Taurus => "♉", Self::Gemini => "♊",
            Self::Cancer => "♋", Self::Leo => "♌", Self::Virgo => "♍",
            Self::Libra => "♎", Self::Scorpio => "♏", Self::Sagittarius => "♐",
            Self::Capricorn => "♑", Self::Aquarius => "♒", Self::Pisces => "♓",
        }
    }
}
```

### 4.3 Planet Position

```rust
#[derive(Debug, Clone, Copy)]
#[cfg_attr(feature = "wasm", wasm_bindgen)]
pub struct PlanetPosition {
    /// Ecliptic longitude in degrees (0-360)
    pub longitude: f64,
    /// Ecliptic latitude in degrees
    pub latitude: f64,
    /// Daily motion in longitude (degrees/day)
    pub speed: f64,
}

impl PlanetPosition {
    pub fn sign(&self) -> ZodiacSign {
        ZodiacSign::from_longitude(self.longitude)
    }

    /// Degrees within the sign (0-30)
    pub fn sign_degree(&self) -> f64 {
        self.longitude.rem_euclid(30.0)
    }

    /// Is the planet retrograde?
    pub fn is_retrograde(&self) -> bool {
        self.speed < 0.0
    }
}
```

### 4.4 Houses

```rust
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[cfg_attr(feature = "wasm", wasm_bindgen)]
pub enum HouseSystem {
    Placidus,
    Koch,
    WholeSign,
    Equal,
    Campanus,
    Regiomontanus,
    Porphyry,
}

#[derive(Debug, Clone)]
#[cfg_attr(feature = "wasm", wasm_bindgen)]
pub struct Houses {
    /// House cusps (index 1-12, index 0 unused)
    pub cusps: [f64; 13],
    /// Ascendant (same as cusp 1)
    pub ascendant: f64,
    /// Midheaven (MC)
    pub mc: f64,
}
```

### 4.5 Aspects

```rust
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
#[cfg_attr(feature = "wasm", wasm_bindgen)]
pub enum AspectType {
    Conjunction,  // 0°
    Sextile,      // 60°
    Square,       // 90°
    Trine,        // 120°
    Opposition,   // 180°
}

impl AspectType {
    pub fn angle(&self) -> f64 {
        match self {
            Self::Conjunction => 0.0,
            Self::Sextile => 60.0,
            Self::Square => 90.0,
            Self::Trine => 120.0,
            Self::Opposition => 180.0,
        }
    }

    pub fn default_orb(&self) -> f64 {
        match self {
            Self::Conjunction | Self::Opposition => 8.0,
            Self::Trine | Self::Square => 6.0,
            Self::Sextile => 4.0,
        }
    }
}

#[derive(Debug, Clone)]
#[cfg_attr(feature = "wasm", wasm_bindgen)]
pub struct Aspect {
    pub planet1: Planet,
    pub planet2: Planet,
    pub aspect_type: AspectType,
    /// Actual angle between planets
    pub angle: f64,
    /// Difference from exact aspect (orb)
    pub orb: f64,
    /// Is the aspect applying (getting closer) or separating?
    pub applying: bool,
}
```

---

## 5. Public API

### 5.1 Main Interface

```rust
/// Calculate a complete horoscope/birth chart
#[cfg_attr(feature = "wasm", wasm_bindgen)]
pub struct Horoscope {
    planets: HashMap<Planet, PlanetPosition>,
    houses: Houses,
    aspects: Vec<Aspect>,
    julian_day: f64,
}

impl Horoscope {
    /// Create a new horoscope for the given date, time, and location
    pub fn new(
        year: i32,
        month: u32,
        day: u32,
        hour: f64,        // 0-24, decimal hours UTC
        latitude: f64,    // Geographic latitude
        longitude: f64,   // Geographic longitude
        house_system: HouseSystem,
    ) -> Self { ... }

    /// Get position of a specific planet
    pub fn planet(&self, planet: Planet) -> PlanetPosition { ... }

    /// Get all planet positions
    pub fn planets(&self) -> &HashMap<Planet, PlanetPosition> { ... }

    /// Get house cusps and angles
    pub fn houses(&self) -> &Houses { ... }

    /// Get all aspects between planets
    pub fn aspects(&self) -> &[Aspect] { ... }

    /// Get aspects for a specific planet
    pub fn aspects_for(&self, planet: Planet) -> Vec<&Aspect> { ... }
}
```

### 5.2 Low-Level Functions

```rust
/// Convert calendar date to Julian Day
pub fn julian_day(year: i32, month: u32, day: u32, hour: f64) -> f64;

/// Calculate position of a single planet
pub fn calc_planet(jd: f64, planet: Planet) -> PlanetPosition;

/// Calculate house cusps
pub fn calc_houses(jd: f64, lat: f64, lon: f64, system: HouseSystem) -> Houses;

/// Find aspects between two sets of positions
pub fn find_aspects(
    positions: &HashMap<Planet, PlanetPosition>,
    orbs: Option<&HashMap<AspectType, f64>>,
) -> Vec<Aspect>;
```

---

## 6. WASM Bindings

### 6.1 JavaScript API

```typescript
// TypeScript definitions (generated)
export interface PlanetPosition {
    longitude: number;
    latitude: number;
    speed: number;
    sign: string;
    signDegree: number;
    isRetrograde: boolean;
}

export interface Houses {
    cusps: number[];  // [_, cusp1, cusp2, ..., cusp12]
    ascendant: number;
    mc: number;
}

export interface Aspect {
    planet1: string;
    planet2: string;
    type: string;
    angle: number;
    orb: number;
    applying: boolean;
}

export interface Horoscope {
    planets: Record<string, PlanetPosition>;
    houses: Houses;
    aspects: Aspect[];
}

// Main function
export function calculateHoroscope(
    year: number,
    month: number,
    day: number,
    hour: number,      // UTC
    latitude: number,
    longitude: number,
    houseSystem?: string,
): Horoscope;
```

### 6.2 WASM Build Configuration

```toml
# wasm/Cargo.toml
[package]
name = "horoscope-wasm"
version = "0.1.0"
edition = "2021"

[lib]
crate-type = ["cdylib", "rlib"]

[dependencies]
horoscope = { path = ".." }
wasm-bindgen = "0.2"
serde = { version = "1.0", features = ["derive"] }
serde-wasm-bindgen = "0.6"

[profile.release]
opt-level = "s"      # Optimize for size
lto = true           # Link-time optimization
```

### 6.3 Build Commands

```bash
# Install wasm-pack
cargo install wasm-pack

# Build for web
wasm-pack build wasm --target web --release

# Build for Node.js
wasm-pack build wasm --target nodejs --release

# Build for bundlers (webpack, etc.)
wasm-pack build wasm --target bundler --release
```

### 6.4 Expected Bundle Size

With optimization for size (`opt-level = "s"`, LTO enabled):
- **Estimated WASM size:** ~100-150 KB (gzipped: ~40-60 KB)
- No external data files required

---

## 7. Implementation Phases

### Phase 1: Core Math & Date
- Julian day conversions
- Angle normalization utilities
- Coordinate transformations
- Obliquity of ecliptic

**Files:** `date.rs`, `math.rs`

### Phase 2: Planet Calculations
- Port Moshier planetary theory from `swemplan.c`
- Port Moshier lunar theory from `swemmoon.c`
- Embed required tables from `swemptab.h`

**Files:** `planets.rs`, `moon.rs`, `tables.rs`

### Phase 3: Houses & Zodiac
- House cusp calculations (subset of systems)
- Ascendant/MC calculation
- Zodiac sign utilities

**Files:** `houses.rs`, `zodiac.rs`

### Phase 4: Aspects & Horoscope
- Aspect detection algorithm
- Orb calculations
- High-level `Horoscope` struct

**Files:** `aspects.rs`, `lib.rs`

### Phase 5: WASM & Release
- wasm-bindgen integration
- TypeScript types generation
- NPM package setup
- Documentation

**Files:** `wasm/src/lib.rs`, package configuration

---

## 8. Testing Strategy

### 8.1 Accuracy Tests

Compare against known ephemeris data:

```rust
#[test]
fn test_sun_position_j2000() {
    // J2000.0 = 2000-01-01 12:00 UTC
    let jd = julian_day(2000, 1, 1, 12.0);
    let sun = calc_planet(jd, Planet::Sun);

    // Expected: ~280.5° (Capricorn)
    assert!((sun.longitude - 280.5).abs() < 0.01);
}

#[test]
fn test_moon_position() {
    let jd = julian_day(2024, 1, 1, 0.0);
    let moon = calc_planet(jd, Planet::Moon);

    // Compare against NASA Horizons or Swiss Ephemeris output
    assert!((moon.longitude - expected).abs() < 0.1);  // 0.1° tolerance
}
```

### 8.2 House Tests

```rust
#[test]
fn test_houses_zurich() {
    // Zurich: 47.38°N, 8.54°E
    let jd = julian_day(2000, 1, 1, 12.0);
    let houses = calc_houses(jd, 47.38, 8.54, HouseSystem::Placidus);

    // Verify Ascendant is reasonable for this time/place
    assert!(houses.ascendant > 0.0 && houses.ascendant < 360.0);
}
```

### 8.3 Aspect Tests

```rust
#[test]
fn test_conjunction_detection() {
    let mut positions = HashMap::new();
    positions.insert(Planet::Sun, PlanetPosition { longitude: 100.0, .. });
    positions.insert(Planet::Moon, PlanetPosition { longitude: 103.0, .. });

    let aspects = find_aspects(&positions, None);

    assert!(aspects.iter().any(|a|
        a.planet1 == Planet::Sun &&
        a.planet2 == Planet::Moon &&
        a.aspect_type == AspectType::Conjunction
    ));
}
```

---

## 9. Usage Examples

### 9.1 Rust

```rust
use horoscope::{Horoscope, HouseSystem, Planet};

fn main() {
    // Birth chart: Jan 1, 2000, 12:00 UTC, Zurich
    let chart = Horoscope::new(
        2000, 1, 1, 12.0,
        47.38, 8.54,
        HouseSystem::Placidus,
    );

    // Print planet positions
    for planet in Planet::all() {
        let pos = chart.planet(*planet);
        println!("{:?}: {} {:.1}° ({})",
            planet,
            pos.sign().symbol(),
            pos.sign_degree(),
            if pos.is_retrograde() { "R" } else { "D" }
        );
    }

    // Print Ascendant
    let asc_sign = ZodiacSign::from_longitude(chart.houses().ascendant);
    println!("Ascendant: {} {:.1}°", asc_sign.symbol(), chart.houses().ascendant % 30.0);

    // Print aspects
    for aspect in chart.aspects() {
        println!("{:?} {:?} {:?} (orb: {:.1}°)",
            aspect.planet1,
            aspect.aspect_type,
            aspect.planet2,
            aspect.orb
        );
    }
}
```

### 9.2 JavaScript/TypeScript

```typescript
import { calculateHoroscope } from 'horoscope-wasm';

// Birth chart: Jan 1, 2000, 12:00 UTC, Zurich
const chart = calculateHoroscope(2000, 1, 1, 12.0, 47.38, 8.54, 'placidus');

// Planet positions
for (const [planet, pos] of Object.entries(chart.planets)) {
    console.log(`${planet}: ${pos.sign} ${pos.signDegree.toFixed(1)}°`);
}

// Ascendant
console.log(`Ascendant: ${chart.houses.ascendant.toFixed(2)}°`);

// Aspects
for (const aspect of chart.aspects) {
    console.log(`${aspect.planet1} ${aspect.type} ${aspect.planet2}`);
}
```

---

## 10. Deliverables

### Phase 1-2 Deliverables
- [ ] Core math utilities
- [ ] Julian day conversions
- [ ] Planet position calculations (Moshier)
- [ ] Moon position calculations
- [ ] Unit tests with accuracy verification

### Phase 3-4 Deliverables
- [ ] House calculations (Placidus, Koch, Whole Sign, Equal)
- [ ] Aspect detection
- [ ] High-level `Horoscope` API
- [ ] Integration tests

### Phase 5 Deliverables
- [ ] WASM build with wasm-bindgen
- [ ] TypeScript type definitions
- [ ] NPM package configuration
- [ ] Usage documentation
- [ ] Example web application

---

## Appendix: Source File Mapping

| This Project | Swiss Ephemeris Source | Lines to Port |
|--------------|----------------------|---------------|
| `date.rs` | `swedate.c` | ~200 |
| `math.rs` | `swephlib.c` (subset) | ~300 |
| `planets.rs` | `swemplan.c` | ~800 |
| `moon.rs` | `swemmoon.c` | ~1,500 |
| `houses.rs` | `swehouse.c` (subset) | ~400 |
| `tables.rs` | `swemptab.h` (subset) | Data |
| **Total** | | **~3,200 lines** |

---

## Appendix: Precision Notes

Using Moshier analytical ephemeris instead of Swiss Ephemeris files:

| Body | Moshier Accuracy | Sufficient for Horoscopes? |
|------|------------------|---------------------------|
| Sun | ~1 arc-second | Yes |
| Moon | ~3 arc-seconds | Yes |
| Mercury-Neptune | ~1 arc-second | Yes |
| Pluto | ~5 arc-seconds | Yes |

For comparison, most astrologers work with 1° orbs, so sub-arc-minute precision is more than adequate.
