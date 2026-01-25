# Swiss Ephemeris Rust Migration Plan

## Executive Summary

This document outlines a comprehensive plan to migrate the Swiss Ephemeris library from C to Rust. The migration aims to leverage Rust's memory safety, modern tooling, and excellent performance while maintaining full compatibility with existing ephemeris data files and API semantics.

**Current State:**
- ~56,000 lines of C code
- 100+ public API functions
- 25+ years of development maturity
- Used worldwide by astrological software developers

**Target State:**
- Pure Rust implementation with idiomatic APIs
- C FFI compatibility layer for existing integrations
- WebAssembly support for browser-based applications
- Improved error handling and thread safety

---

## Table of Contents

1. [Project Analysis](#1-project-analysis)
2. [Migration Strategy](#2-migration-strategy)
3. [Module Breakdown](#3-module-breakdown)
4. [Implementation Phases](#4-implementation-phases)
5. [Technical Decisions](#5-technical-decisions)
6. [API Design](#6-api-design)
7. [Testing Strategy](#7-testing-strategy)
8. [Risk Assessment](#8-risk-assessment)
9. [Deliverables](#9-deliverables)

---

## 1. Project Analysis

### 1.1 Current Architecture

The Swiss Ephemeris is organized into the following functional modules:

| Module | File(s) | Size | Purpose |
|--------|---------|------|---------|
| Core Engine | sweph.c/h | 284 KB | Main planetary calculation engine |
| Library Utils | swephlib.c/h | 161 KB | Coordinate transforms, time, delta-t |
| Eclipses | swecl.c | 221 KB | Eclipse, occultation, phenomena |
| Houses | swehouse.c/h | 110 KB | Astrological house systems |
| Lunar | swemmoon.c | 60 KB | Analytical lunar ephemeris |
| Planetary | swemplan.c | 28 KB | Moshier planetary routines |
| JPL Reader | swejpl.c/h | 33 KB | JPL binary file interface |
| Date/Time | swedate.c/h | 20 KB | Julian day conversions |
| Heliacal | swehel.c | 120 KB | Heliacal rising/setting |
| Data Tables | swemptab.h, swenut2000a.h | 372 KB | Embedded astronomical tables |

### 1.2 Dependencies

The C codebase has minimal external dependencies:
- **libc**: Standard C library functions
- **libm**: Mathematical functions (sin, cos, atan2, sqrt, etc.)

This is excellent for Rust migration as Rust's standard library covers all these needs.

### 1.3 Global State

The C library uses significant global state for:
- Ephemeris file paths and handles
- Configuration settings (sidereal mode, topocentric position)
- Cached calculation results
- Delta-T tables

**Rust Approach:** Encapsulate state in a `SwissEph` struct with clear ownership semantics.

### 1.4 Data Files

Binary ephemeris file formats that must be supported:
- `.se1` files (Swiss Ephemeris format)
- `.eph` files (JPL ephemeris format)
- Text files (leap seconds, star catalogs)

---

## 2. Migration Strategy

### 2.1 Approach: Incremental Rewrite with Parallel Testing

We recommend an **incremental rewrite** strategy rather than a big-bang rewrite:

1. **Create Rust module structure** mirroring C organization
2. **Port one module at a time**, starting with lowest-dependency modules
3. **Maintain C FFI bridge** for each ported module
4. **Run parallel tests** comparing C and Rust outputs
5. **Validate numerical precision** at each step

### 2.2 Why Not Bindgen/FFI Wrapper?

While `bindgen` could create Rust bindings quickly, a full rewrite is preferred because:

1. **Memory Safety**: The C code uses raw pointers extensively
2. **Thread Safety**: Global state prevents safe concurrent use
3. **Error Handling**: C uses return codes; Rust can use `Result<T, E>`
4. **API Modernization**: Opportunity to create idiomatic Rust APIs
5. **WebAssembly**: Pure Rust enables easy WASM compilation
6. **Long-term Maintainability**: Rust code with proper types is easier to maintain

### 2.3 Dependency Graph

```
                    ┌──────────────┐
                    │   swedate    │ (no deps)
                    └──────┬───────┘
                           │
              ┌────────────┼────────────┐
              │            │            │
              ▼            ▼            ▼
        ┌──────────┐ ┌──────────┐ ┌──────────┐
        │ swephlib │ │  swejpl  │ │ data     │
        └────┬─────┘ └────┬─────┘ │ tables   │
             │            │       └────┬─────┘
             └─────┬──────┴────────────┘
                   │
                   ▼
            ┌──────────────┐
            │    sweph     │ (core engine)
            └──────┬───────┘
                   │
     ┌─────────────┼─────────────┐
     │             │             │
     ▼             ▼             ▼
┌─────────┐  ┌──────────┐  ┌──────────┐
│ swecl   │  │ swehouse │  │ swehel   │
└─────────┘  └──────────┘  └──────────┘
```

---

## 3. Module Breakdown

### 3.1 Module: `swedate` (Date/Time)

**C Files:** `swedate.c`, `swedate.h` (~20 KB)

**Functions to Port:**
```rust
// Julian Day conversions
fn julday(year: i32, month: i32, day: i32, hour: f64, gregflag: CalendarType) -> f64;
fn revjul(jd: f64, gregflag: CalendarType) -> (i32, i32, i32, f64);

// Date validation
fn date_conversion(year: i32, month: i32, day: i32, ...) -> Result<JulianDay, DateError>;

// Day of week
fn day_of_week(jd: f64) -> Weekday;
```

**Rust Approach:**
- Create `JulianDay` newtype wrapper
- Use `chrono` crate for modern date/time interop (optional dependency)
- Strong typing for calendar types (Gregorian/Julian)

**Complexity:** Low
**Priority:** Phase 1 (no dependencies)

---

### 3.2 Module: `swephlib` (Library Utilities)

**C Files:** `swephlib.c`, `swephlib.h` (~161 KB)

**Functions to Port:**
```rust
// Coordinate transformations
fn cotrans(xpo: [f64; 6], eps: f64) -> [f64; 6];
fn cotrans_sp(xpo: [f64; 6], eps: f64) -> [f64; 6];

// Obliquity of ecliptic
fn epsiln(jd: f64, iflag: i32) -> f64;

// Nutation
fn nutation(jd: f64, iflag: i32) -> (f64, f64); // (nutlo, nuto)

// Sidereal time
fn sidtime(jd_ut: f64) -> f64;
fn sidtime0(jd_ut: f64, eps: f64, nut: f64) -> f64;

// Delta T
fn deltat(jd: f64) -> f64;
fn deltat_ex(jd: f64, iflag: i32) -> Result<f64, DeltaTError>;

// Precession
fn precess(r: [f64; 3], jd: f64, iflag: i32, direction: i32) -> [f64; 3];

// Utility functions
fn radnorm(x: f64) -> f64;
fn degnorm(x: f64) -> f64;
fn difdeg2n(p1: f64, p2: f64) -> f64;
```

**Rust Approach:**
- Create `Coordinates` struct with longitude/latitude/distance
- Use `nalgebra` for matrix operations (optional)
- Implement `From`/`Into` traits for coordinate system conversions

**Complexity:** Medium
**Priority:** Phase 1

---

### 3.3 Module: `swejpl` (JPL Ephemeris Reader)

**C Files:** `swejpl.c`, `swejpl.h` (~33 KB)

**Functions to Port:**
```rust
// File operations
fn jpl_open(path: &Path, denum: i32) -> Result<JplFile, JplError>;
fn jpl_close(file: &mut JplFile);

// Ephemeris computation
fn jpl_plaus(et: f64, list: &[i32], pv: &mut [[f64; 6]], nut: &mut [f64]) -> Result<(), JplError>;
```

**Rust Approach:**
- Use `std::io` for file operations
- Implement proper RAII with `Drop` trait
- Use `byteorder` crate for cross-platform binary reading
- Memory-map large files with `memmap2` crate

**Complexity:** Medium
**Priority:** Phase 1

---

### 3.4 Module: `sweph` (Core Engine)

**C Files:** `sweph.c`, `sweph.h`, `swemptab.h` (~589 KB combined)

**Core Functions to Port:**
```rust
// Main calculation
fn calc(jd_et: f64, ipl: Planet, iflag: CalcFlags) -> Result<PlanetPosition, CalcError>;
fn calc_ut(jd_ut: f64, ipl: Planet, iflag: CalcFlags) -> Result<PlanetPosition, CalcError>;

// Planetocentric
fn calc_pctr(jd_et: f64, ipl: Planet, iplctr: Planet, iflag: CalcFlags)
    -> Result<PlanetPosition, CalcError>;

// Fixed stars
fn fixstar(star: &str, jd_et: f64, iflag: CalcFlags) -> Result<StarPosition, CalcError>;
fn fixstar2(star: &str, jd_et: f64, iflag: CalcFlags) -> Result<(StarPosition, String), CalcError>;

// Configuration
fn set_ephe_path(path: &Path);
fn set_jpl_file(name: &str);
fn set_topo(lng: f64, lat: f64, alt: f64);
fn set_sid_mode(sid_mode: SiderealMode, t0: f64, ayan_t0: f64);
fn close();

// Ayanamsa
fn get_ayanamsa(jd_et: f64) -> f64;
fn get_ayanamsa_ex(jd_et: f64, iflag: CalcFlags) -> Result<f64, CalcError>;
```

**Rust Approach:**
- Create `SwissEph` context struct holding all state
- Use builder pattern for configuration
- Implement `Default` trait for common configurations
- Strong typing for planets, flags, and return values

```rust
pub struct SwissEph {
    ephe_path: PathBuf,
    jpl_file: Option<JplFile>,
    sidereal_mode: Option<SiderealMode>,
    topocentric: Option<TopocentricPosition>,
    // ... cached state
}

impl SwissEph {
    pub fn new() -> SwissEphBuilder { ... }
    pub fn calc(&self, jd: JulianDay, planet: Planet, flags: CalcFlags)
        -> Result<Position, Error> { ... }
}
```

**Complexity:** High (largest module)
**Priority:** Phase 2

---

### 3.5 Module: `swemmoon` / `swemplan` (Analytical Ephemerides)

**C Files:** `swemmoon.c` (~60 KB), `swemplan.c` (~28 KB)

**Functions to Port:**
```rust
// Moshier lunar ephemeris
fn moon(jd: f64) -> [f64; 6];  // position and velocity

// Moshier planetary ephemeris
fn planet(jd: f64, planet: Planet) -> [f64; 6];
```

**Rust Approach:**
- These are pure computational functions with no I/O
- Port directly with careful attention to numerical precision
- Use `#[inline]` for hot paths
- Consider SIMD optimization with `packed_simd` crate

**Complexity:** Medium (computationally intensive)
**Priority:** Phase 2

---

### 3.6 Module: `swehouse` (House Systems)

**C Files:** `swehouse.c`, `swehouse.h` (~110 KB)

**Functions to Port:**
```rust
// House calculation
fn houses(jd_ut: f64, lat: f64, lng: f64, hsys: HouseSystem)
    -> Result<HousePositions, HouseError>;
fn houses_ex(jd_ut: f64, iflag: CalcFlags, lat: f64, lng: f64, hsys: HouseSystem)
    -> Result<HousePositions, HouseError>;
fn houses_ex2(jd_ut: f64, iflag: CalcFlags, lat: f64, lng: f64, hsys: HouseSystem)
    -> Result<(HousePositions, Ascmc), HouseError>;

// House position for a planet
fn house_pos(armc: f64, geolat: f64, eps: f64, hsys: HouseSystem,
             xpin: [f64; 2]) -> f64;

// Gauquelin sectors
fn gauquelin_sector(jd_ut: f64, ipl: Planet, starname: Option<&str>,
                    iflag: CalcFlags, imeth: i32, geopos: [f64; 3])
    -> Result<f64, GauquelinError>;
```

**Rust Approach:**
- Create `HouseSystem` enum with all 40+ supported systems
- Return structured data instead of arrays
- Strong typing for ascendant, MC, vertex, etc.

```rust
pub enum HouseSystem {
    Placidus,
    Koch,
    Porphyry,
    Regiomontanus,
    Campanus,
    Equal,
    WholeSign,
    // ... 40+ systems
}

pub struct HousePositions {
    pub cusps: [f64; 13],  // houses 1-12 (index 0 unused)
    pub ascendant: f64,
    pub mc: f64,
    pub armc: f64,
    pub vertex: f64,
    pub equatorial_ascendant: f64,
    // ...
}
```

**Complexity:** Medium
**Priority:** Phase 3

---

### 3.7 Module: `swecl` (Eclipses & Phenomena)

**C Files:** `swecl.c` (~221 KB)

**Functions to Port:**
```rust
// Solar eclipses
fn sol_eclipse_when_glob(jd_start: f64, iflag: CalcFlags, ifltype: EclipseType, backward: bool)
    -> Result<SolarEclipse, EclipseError>;
fn sol_eclipse_when_loc(jd_start: f64, iflag: CalcFlags, geopos: [f64; 3], backward: bool)
    -> Result<LocalSolarEclipse, EclipseError>;
fn sol_eclipse_how(jd_ut: f64, iflag: CalcFlags, geopos: [f64; 3])
    -> Result<EclipseAttributes, EclipseError>;
fn sol_eclipse_where(jd_ut: f64, iflag: CalcFlags)
    -> Result<EclipseLocation, EclipseError>;

// Lunar eclipses
fn lun_eclipse_when(jd_start: f64, iflag: CalcFlags, ifltype: EclipseType, backward: bool)
    -> Result<LunarEclipse, EclipseError>;
fn lun_eclipse_how(jd_ut: f64, iflag: CalcFlags, geopos: [f64; 3])
    -> Result<EclipseAttributes, EclipseError>;

// Occultations
fn lun_occult_when_glob(jd_start: f64, ipl: Planet, starname: Option<&str>,
                        iflag: CalcFlags, ifltype: i32, backward: bool)
    -> Result<Occultation, OccultError>;

// Rise, set, transit
fn rise_trans(jd_ut: f64, ipl: Planet, starname: Option<&str>,
              iflag: CalcFlags, rsmi: RiseSetType, geopos: [f64; 3],
              atpress: f64, attemp: f64)
    -> Result<f64, RiseSetError>;

// Planetary phenomena
fn pheno(jd_et: f64, ipl: Planet, iflag: CalcFlags) -> Result<Phenomena, PhenoError>;
fn pheno_ut(jd_ut: f64, ipl: Planet, iflag: CalcFlags) -> Result<Phenomena, PhenoError>;

// Refraction
fn refrac(inalt: f64, atpress: f64, attemp: f64, calc_flag: RefractionType) -> f64;
fn refrac_extended(inalt: f64, geoalt: f64, atpress: f64, attemp: f64,
                   lapse_rate: f64, calc_flag: RefractionType) -> (f64, [f64; 4]);

// Azimuth/Altitude
fn azalt(jd_ut: f64, calc_flag: AzAltType, geopos: [f64; 3],
         atpress: f64, attemp: f64, xin: [f64; 3]) -> [f64; 3];
fn azalt_rev(jd_ut: f64, calc_flag: AzAltType, geopos: [f64; 3], xin: [f64; 2]) -> [f64; 3];
```

**Rust Approach:**
- Rich result types with full eclipse data
- Iterator patterns for finding multiple events
- Strong typing for eclipse types and phases

**Complexity:** High
**Priority:** Phase 3

---

### 3.8 Module: `swehel` (Heliacal Phenomena)

**C Files:** `swehel.c` (~120 KB)

**Functions to Port:**
```rust
fn heliacal_ut(jd_utstart: f64, geopos: [f64; 3], datm: [f64; 4],
               dobs: [f64; 6], object_name: &str, event_type: HeliacalEventType,
               helflag: HeliacalFlags) -> Result<HeliacalEvent, HeliacalError>;

fn heliacal_pheno_ut(jd_ut: f64, geopos: [f64; 3], datm: [f64; 4],
                     dobs: [f64; 6], object_name: &str, event_type: HeliacalEventType,
                     helflag: HeliacalFlags) -> Result<HeliacalPheno, HeliacalError>;

fn vis_limit_mag(jd_ut: f64, geopos: [f64; 3], datm: [f64; 4],
                 dobs: [f64; 6], object_name: &str, helflag: HeliacalFlags)
    -> Result<VisibilityLimit, VisLimitError>;
```

**Complexity:** Medium-High
**Priority:** Phase 4

---

### 3.9 Data Tables Module

**C Files:** `swemptab.h` (305 KB), `swenut2000a.h` (67 KB)

**Approach:**
- Convert to `const` arrays in Rust
- Use `include!` macro or build script to generate
- Consider `lazy_static` or `once_cell` for computed tables

```rust
// Example: Planetary mean motion tables
pub static MEAN_MOTION_TABLES: &[PlanetaryTable] = &[
    PlanetaryTable { /* ... */ },
    // ...
];

// Nutation coefficients (IAU 2000A)
pub static NUT_2000A: &[NutationCoeff] = &[
    NutationCoeff { /* ... */ },
    // ...
];
```

**Complexity:** Low (mechanical translation)
**Priority:** Phase 1

---

## 4. Implementation Phases

### Phase 1: Foundation (Modules with No Dependencies)

**Modules:**
1. `swedate` - Date/time conversions
2. Data tables - Constants and lookup tables
3. `swephlib` - Utility functions (partial: math functions only)
4. `swejpl` - JPL file reader

**Deliverables:**
- Basic project structure with Cargo workspace
- Core type definitions
- Julian day calculations
- Coordinate transformation functions
- JPL ephemeris file reader

**Validation:**
- Unit tests comparing output to C implementation
- Property-based tests for mathematical functions

---

### Phase 2: Core Calculation Engine

**Modules:**
1. `swephlib` - Complete utility functions
2. `swemmoon` - Lunar ephemeris
3. `swemplan` - Planetary ephemeris
4. `sweph` - Core calculation engine

**Deliverables:**
- Full planetary position calculations
- Swiss Ephemeris file reader
- Fixed star calculations
- Main `SwissEph` context struct

**Validation:**
- Integration tests with known ephemeris positions
- Comparison against JPL Horizons data
- Performance benchmarks

---

### Phase 3: Derived Calculations

**Modules:**
1. `swehouse` - House systems
2. `swecl` - Eclipses and phenomena

**Deliverables:**
- All house system calculations
- Eclipse prediction
- Rise/set/transit calculations
- Occultation calculations

**Validation:**
- Compare against published eclipse data
- Verify house cusps against reference implementations

---

### Phase 4: Advanced Features

**Modules:**
1. `swehel` - Heliacal phenomena
2. C FFI layer
3. WebAssembly bindings

**Deliverables:**
- Heliacal rising/setting calculations
- C-compatible API for existing integrations
- WASM build with JavaScript bindings

**Validation:**
- Full test suite passes
- API compatibility verification
- Browser-based demo application

---

### Phase 5: Polish and Release

**Activities:**
1. Performance optimization
2. Documentation
3. Example programs
4. CI/CD setup
5. Publishing to crates.io

**Deliverables:**
- Published crate with full documentation
- Benchmark comparisons with C version
- Migration guide for existing users

---

## 5. Technical Decisions

### 5.1 Crate Structure

```
swisseph-rs/
├── Cargo.toml              # Workspace root
├── crates/
│   ├── swisseph/           # Main library crate
│   │   ├── src/
│   │   │   ├── lib.rs
│   │   │   ├── date.rs     # swedate
│   │   │   ├── lib_utils.rs # swephlib
│   │   │   ├── jpl.rs      # swejpl
│   │   │   ├── core.rs     # sweph
│   │   │   ├── moon.rs     # swemmoon
│   │   │   ├── planet.rs   # swemplan
│   │   │   ├── house.rs    # swehouse
│   │   │   ├── eclipse.rs  # swecl
│   │   │   ├── heliacal.rs # swehel
│   │   │   ├── data/       # Embedded tables
│   │   │   └── error.rs    # Error types
│   │   └── Cargo.toml
│   ├── swisseph-sys/       # C FFI bindings (optional)
│   │   └── ...
│   └── swisseph-wasm/      # WebAssembly bindings
│       └── ...
├── examples/
│   ├── basic_calc.rs
│   ├── eclipse_finder.rs
│   └── birth_chart.rs
├── tests/
│   ├── integration/
│   └── comparison/         # Tests comparing to C output
└── benches/
    └── calculations.rs
```

### 5.2 Error Handling

```rust
use thiserror::Error;

#[derive(Error, Debug)]
pub enum SwissEphError {
    #[error("Invalid date: {0}")]
    InvalidDate(String),

    #[error("Ephemeris file not found: {path}")]
    EphemerisNotFound { path: PathBuf },

    #[error("Invalid planet number: {0}")]
    InvalidPlanet(i32),

    #[error("Calculation failed: {0}")]
    CalculationError(String),

    #[error("JPL ephemeris error: {0}")]
    JplError(#[from] JplError),

    #[error("IO error: {0}")]
    IoError(#[from] std::io::Error),
}

pub type Result<T> = std::result::Result<T, SwissEphError>;
```

### 5.3 Numeric Precision

- Use `f64` throughout (matches C `double`)
- Consider `rug` crate for arbitrary precision where needed
- Document precision guarantees
- Use `approx` crate for floating-point comparisons in tests

### 5.4 Thread Safety

```rust
// The main SwissEph struct should be Send + Sync
pub struct SwissEph {
    // Use Arc<RwLock<...>> for shared mutable state
    cache: Arc<RwLock<CalculationCache>>,
    // Use atomics for simple flags
    delta_t_mode: AtomicI32,
    // Immutable config can be plain Arc
    config: Arc<Config>,
}

// Alternatively, make it fully immutable with interior mutability patterns
```

### 5.5 Dependencies

**Required:**
```toml
[dependencies]
thiserror = "1.0"      # Error handling
byteorder = "1.4"      # Binary file reading
```

**Optional:**
```toml
[dependencies]
chrono = { version = "0.4", optional = true }       # DateTime interop
memmap2 = { version = "0.7", optional = true }      # Memory-mapped files
rayon = { version = "1.8", optional = true }        # Parallel iterators
serde = { version = "1.0", optional = true }        # Serialization
```

**Dev Dependencies:**
```toml
[dev-dependencies]
criterion = "0.5"       # Benchmarking
proptest = "1.4"        # Property-based testing
approx = "0.5"          # Float comparisons
```

### 5.6 Feature Flags

```toml
[features]
default = ["std"]
std = []                        # Standard library (disable for no_std)
jpl = []                        # JPL ephemeris support
chrono = ["dep:chrono"]         # chrono integration
serde = ["dep:serde"]           # Serialization
parallel = ["dep:rayon"]        # Parallel calculations
wasm = []                       # WebAssembly optimizations
ffi = []                        # C FFI exports
```

---

## 6. API Design

### 6.1 Builder Pattern for Configuration

```rust
let swe = SwissEph::builder()
    .ephemeris_path("/path/to/ephe")
    .jpl_file("de441.eph")
    .sidereal_mode(SiderealMode::Lahiri)
    .topocentric(47.38, 8.54, 400.0)  // Zurich
    .build()?;
```

### 6.2 Ergonomic Calculation API

```rust
// Simple case
let sun = swe.calc(jd, Planet::Sun, CalcFlags::default())?;
println!("Sun longitude: {:.6}°", sun.longitude);

// With all options
let moon = swe.calc_ut(jd_ut, Planet::Moon,
    CalcFlags::SPEED | CalcFlags::EQUATORIAL)?;
println!("Moon RA: {:.6}°, Dec: {:.6}°", moon.right_ascension, moon.declination);

// Fixed stars
let regulus = swe.fixstar("Regulus", jd, CalcFlags::default())?;
```

### 6.3 Type-Safe Enums

```rust
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Planet {
    Sun = 0,
    Moon = 1,
    Mercury = 2,
    Venus = 3,
    Mars = 4,
    Jupiter = 5,
    Saturn = 6,
    Uranus = 7,
    Neptune = 8,
    Pluto = 9,
    MeanNode = 10,
    TrueNode = 11,
    MeanApogee = 12,    // Mean Lilith
    OscuApogee = 13,    // Osculating Lilith
    Earth = 14,
    Chiron = 15,
    // ...
}

#[derive(Debug, Clone, Copy)]
pub enum HouseSystem {
    Placidus = b'P' as isize,
    Koch = b'K' as isize,
    Porphyry = b'O' as isize,
    Regiomontanus = b'R' as isize,
    Campanus = b'C' as isize,
    Equal = b'E' as isize,
    WholeSign = b'W' as isize,
    // ... 40+ more
}
```

### 6.4 Rich Return Types

```rust
#[derive(Debug, Clone)]
pub struct Position {
    /// Longitude in degrees (0-360)
    pub longitude: f64,
    /// Latitude in degrees (-90 to 90)
    pub latitude: f64,
    /// Distance (AU for planets, AU for stars)
    pub distance: f64,
    /// Speed in longitude (degrees/day)
    pub speed_longitude: f64,
    /// Speed in latitude (degrees/day)
    pub speed_latitude: f64,
    /// Speed in distance (AU/day)
    pub speed_distance: f64,
}

impl Position {
    /// Get zodiac sign (0 = Aries, 11 = Pisces)
    pub fn sign(&self) -> ZodiacSign {
        ZodiacSign::from_longitude(self.longitude)
    }

    /// Get degree within sign (0-30)
    pub fn sign_degree(&self) -> f64 {
        self.longitude % 30.0
    }

    /// Check if planet is retrograde
    pub fn is_retrograde(&self) -> bool {
        self.speed_longitude < 0.0
    }
}
```

### 6.5 Iterator Patterns for Events

```rust
// Find next 10 solar eclipses
let eclipses = swe.solar_eclipses(jd_start)
    .filter(|e| e.eclipse_type == EclipseType::Total)
    .take(10)
    .collect::<Vec<_>>();

// Find Mercury retrograde periods
let retrogrades = swe.retrograde_periods(Planet::Mercury, jd_start, jd_end)
    .collect::<Vec<_>>();
```

---

## 7. Testing Strategy

### 7.1 Unit Tests

Each module should have comprehensive unit tests:

```rust
#[cfg(test)]
mod tests {
    use super::*;
    use approx::assert_relative_eq;

    #[test]
    fn test_julday() {
        // J2000.0 epoch
        let jd = julday(2000, 1, 1, 12.0, CalendarType::Gregorian);
        assert_relative_eq!(jd, 2451545.0, epsilon = 1e-10);
    }

    #[test]
    fn test_revjul() {
        let (y, m, d, h) = revjul(2451545.0, CalendarType::Gregorian);
        assert_eq!((y, m, d), (2000, 1, 1));
        assert_relative_eq!(h, 12.0, epsilon = 1e-10);
    }
}
```

### 7.2 Comparison Tests

Create a test harness that compares Rust output to C output:

```rust
#[test]
fn compare_planet_positions() {
    let test_cases = load_c_reference_data("test_data/planet_positions.json");
    let swe = SwissEph::new();

    for case in test_cases {
        let result = swe.calc(case.jd, case.planet, case.flags).unwrap();
        assert_relative_eq!(result.longitude, case.expected_lon, epsilon = 1e-8);
        assert_relative_eq!(result.latitude, case.expected_lat, epsilon = 1e-8);
        assert_relative_eq!(result.distance, case.expected_dist, epsilon = 1e-10);
    }
}
```

### 7.3 Property-Based Tests

```rust
use proptest::prelude::*;

proptest! {
    #[test]
    fn julday_revjul_roundtrip(
        year in 1..3000i32,
        month in 1..=12i32,
        day in 1..=28i32,  // Safe range
        hour in 0.0..24.0f64
    ) {
        let jd = julday(year, month, day, hour, CalendarType::Gregorian);
        let (y, m, d, h) = revjul(jd, CalendarType::Gregorian);
        prop_assert_eq!((y, m, d), (year, month, day));
        prop_assert!((h - hour).abs() < 1e-10);
    }

    #[test]
    fn longitude_always_normalized(jd in 2000000.0..2500000.0f64) {
        let swe = SwissEph::new();
        let pos = swe.calc(jd, Planet::Sun, CalcFlags::default()).unwrap();
        prop_assert!(pos.longitude >= 0.0 && pos.longitude < 360.0);
    }
}
```

### 7.4 Integration Tests

```rust
#[test]
fn test_eclipse_2024() {
    let swe = SwissEph::builder()
        .ephemeris_path("ephe")
        .build()
        .unwrap();

    // Total solar eclipse of April 8, 2024
    let jd = julday(2024, 4, 8, 18.0, CalendarType::Gregorian);
    let eclipse = swe.sol_eclipse_when_glob(jd - 1.0, CalcFlags::default(),
                                             EclipseType::Total, false)?;

    // Verify it finds the correct eclipse
    assert_relative_eq!(eclipse.maximum_jd, 2460409.286, epsilon = 0.01);
}
```

### 7.5 Benchmarks

```rust
use criterion::{criterion_group, criterion_main, Criterion};

fn benchmark_calc(c: &mut Criterion) {
    let swe = SwissEph::new();
    let jd = 2451545.0;  // J2000

    c.bench_function("calc_sun", |b| {
        b.iter(|| swe.calc(jd, Planet::Sun, CalcFlags::default()))
    });

    c.bench_function("calc_all_planets", |b| {
        b.iter(|| {
            for planet in Planet::iter() {
                swe.calc(jd, planet, CalcFlags::default()).unwrap();
            }
        })
    });
}

criterion_group!(benches, benchmark_calc);
criterion_main!(benches);
```

---

## 8. Risk Assessment

### 8.1 High Risk

| Risk | Mitigation |
|------|------------|
| Numerical precision differences | Extensive comparison testing; document precision guarantees |
| Complex algorithms hard to port | Port incrementally with thorough testing at each step |
| Edge cases in date/time handling | Property-based testing; extensive test coverage |

### 8.2 Medium Risk

| Risk | Mitigation |
|------|------------|
| Performance regression | Benchmark continuously; optimize hot paths |
| Binary file format compatibility | Fuzz testing with various file versions |
| Missing functionality | Maintain feature parity checklist |

### 8.3 Low Risk

| Risk | Mitigation |
|------|------------|
| API design issues | Follow Rust API guidelines; get community feedback early |
| Dependency issues | Minimize dependencies; pin versions |

---

## 9. Deliverables

### 9.1 Phase 1 Deliverables
- [ ] Cargo workspace setup
- [ ] `date` module with Julian day functions
- [ ] Data tables module with embedded constants
- [ ] `lib_utils` module (coordinate transforms, time functions)
- [ ] `jpl` module for JPL ephemeris reading
- [ ] Unit tests for all functions
- [ ] Comparison tests against C implementation

### 9.2 Phase 2 Deliverables
- [ ] `moon` module (Moshier lunar ephemeris)
- [ ] `planet` module (Moshier planetary ephemeris)
- [ ] `core` module (main calculation engine)
- [ ] Swiss Ephemeris file reader
- [ ] Fixed star calculations
- [ ] `SwissEph` context struct with builder
- [ ] Integration tests

### 9.3 Phase 3 Deliverables
- [ ] `house` module (all house systems)
- [ ] `eclipse` module (eclipse/occultation/phenomena)
- [ ] Rise/set/transit calculations
- [ ] Full test suite

### 9.4 Phase 4 Deliverables
- [ ] `heliacal` module
- [ ] C FFI compatibility layer (`swisseph-sys`)
- [ ] WebAssembly bindings (`swisseph-wasm`)
- [ ] JavaScript/TypeScript type definitions

### 9.5 Phase 5 Deliverables
- [ ] Performance optimization
- [ ] API documentation (rustdoc)
- [ ] User guide
- [ ] Example programs
- [ ] CI/CD pipeline
- [ ] Published to crates.io

---

## Appendix A: Function Mapping

| C Function | Rust Function | Module |
|------------|---------------|--------|
| `swe_julday` | `julday` | date |
| `swe_revjul` | `revjul` | date |
| `swe_deltat` | `delta_t` | lib_utils |
| `swe_sidtime` | `sidereal_time` | lib_utils |
| `swe_cotrans` | `cotrans` | lib_utils |
| `swe_calc` | `SwissEph::calc` | core |
| `swe_calc_ut` | `SwissEph::calc_ut` | core |
| `swe_fixstar` | `SwissEph::fixstar` | core |
| `swe_houses` | `SwissEph::houses` | house |
| `swe_sol_eclipse_when_glob` | `SwissEph::solar_eclipse_when_global` | eclipse |
| `swe_rise_trans` | `SwissEph::rise_trans` | eclipse |
| `swe_heliacal_ut` | `SwissEph::heliacal` | heliacal |

---

## Appendix B: Type Mapping

| C Type | Rust Type |
|--------|-----------|
| `double` | `f64` |
| `int` | `i32` |
| `int32` | `i32` |
| `char *` (path) | `&Path` / `PathBuf` |
| `char *` (string) | `&str` / `String` |
| `char[N]` (output) | Return value or `&mut String` |
| `double[6]` | `[f64; 6]` or `Position` struct |
| `double[N]` (output) | Return struct |
| Return code `int` | `Result<T, Error>` |

---

## Appendix C: References

1. Swiss Ephemeris Documentation: https://www.astro.com/swisseph/
2. Rust API Guidelines: https://rust-lang.github.io/api-guidelines/
3. JPL Ephemeris: https://ssd.jpl.nasa.gov/planets/eph_export.html
4. Moshier's Analytical Ephemeris: https://www.moshier.net/
