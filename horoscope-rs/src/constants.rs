//! Astronomical constants

use std::f64::consts::PI;

/// Two pi
pub const TWOPI: f64 = 2.0 * PI;

/// Degrees to radians
pub const DEG_TO_RAD: f64 = PI / 180.0;

/// Radians to degrees
pub const RAD_TO_DEG: f64 = 180.0 / PI;

/// Arc seconds to radians
pub const ARCSEC_TO_RAD: f64 = PI / (180.0 * 3600.0);

/// J2000.0 epoch (Julian Day)
pub const J2000: f64 = 2451545.0;

/// J1900.0 epoch
pub const J1900: f64 = 2415020.0;

/// Days per Julian century
pub const DAYS_PER_CENTURY: f64 = 36525.0;

/// Astronomical Unit in km
pub const AU_KM: f64 = 149597870.7;

/// Earth-Moon mass ratio
pub const EARTH_MOON_MRAT: f64 = 81.30056907419062;

/// Mean obliquity of ecliptic at J2000 (degrees)
pub const OBLIQUITY_J2000: f64 = 23.439291111;

/// Gaussian gravitational constant
pub const KGAUSS: f64 = 0.01720209895;

/// Speed of light in AU/day
pub const CLIGHT: f64 = 173.1446326846693;

/// Light time for 1 AU in days
pub const LIGHTTIME_AUNIT: f64 = 1.0 / CLIGHT;

/// Planet calculation speed interval (days)
pub const PLAN_SPEED_INTV: f64 = 0.1;

/// Moshier ephemeris valid range
pub const MOSHIER_START: f64 = 625307.5;  // -3000
pub const MOSHIER_END: f64 = 2817057.5;   // +3000

/// Swiss Ephemeris planet constants
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
pub const SE_MEAN_NODE: i32 = 10;
pub const SE_TRUE_NODE: i32 = 11;

/// Calendar flag: Gregorian
pub const SE_GREG_CAL: i32 = 1;

/// Calculation flags
pub const SEFLG_SPEED: i32 = 256;

/// Timescale for Moshier planets
pub const MOSHIER_TIMESCALE: f64 = 3652500.0;

/// Planet mean orbital elements - semi-major axes (AU)
pub const PLANET_SEMIMAJOR: [f64; 9] = [
    0.3870983,  // Mercury
    0.7233316,  // Venus
    1.0000000,  // Earth
    1.5236883,  // Mars
    5.2025610,  // Jupiter
    9.5549480,  // Saturn
    19.218446,  // Uranus
    30.110387,  // Neptune
    39.543940,  // Pluto
];
