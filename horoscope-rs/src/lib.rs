//! Horoscope - Minimal astronomical calculations for horoscope generation
//!
//! This library provides calculations for:
//! - Planet positions (Sun, Moon, Mercury through Pluto)
//! - True lunar node
//! - House cusps (Placidus system)
//!
//! Uses VSOP87 theory for planets, ELP2000 for Moon.
//! Licensed under GPL-3.0.

#![allow(clippy::excessive_precision)]

pub mod constants;
pub mod julian;
pub mod math;
pub mod planets;
pub mod moon;
pub mod houses;
pub mod nodes;

#[cfg(feature = "wasm")]
pub mod wasm;

pub use constants::*;
pub use julian::*;
pub use math::deg_norm;

/// Planet identifiers (matching Swiss Ephemeris)
#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
#[repr(i32)]
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
    TrueNode = 11,
}

impl Planet {
    pub fn from_i32(id: i32) -> Option<Self> {
        match id {
            0 => Some(Planet::Sun),
            1 => Some(Planet::Moon),
            2 => Some(Planet::Mercury),
            3 => Some(Planet::Venus),
            4 => Some(Planet::Mars),
            5 => Some(Planet::Jupiter),
            6 => Some(Planet::Saturn),
            7 => Some(Planet::Uranus),
            8 => Some(Planet::Neptune),
            9 => Some(Planet::Pluto),
            11 => Some(Planet::TrueNode),
            _ => None,
        }
    }

    pub fn all() -> &'static [Planet] {
        &[
            Planet::Sun,
            Planet::Moon,
            Planet::Mercury,
            Planet::Venus,
            Planet::Mars,
            Planet::Jupiter,
            Planet::Saturn,
            Planet::Uranus,
            Planet::Neptune,
            Planet::Pluto,
            Planet::TrueNode,
        ]
    }
}

/// Position result with longitude, latitude, distance, and speeds
#[derive(Debug, Clone, Copy, Default)]
pub struct Position {
    /// Ecliptic longitude in degrees (0-360)
    pub longitude: f64,
    /// Ecliptic latitude in degrees
    pub latitude: f64,
    /// Distance in AU
    pub distance: f64,
    /// Speed in longitude (degrees/day)
    pub speed_longitude: f64,
    /// Speed in latitude (degrees/day)
    pub speed_latitude: f64,
    /// Speed in distance (AU/day)
    pub speed_distance: f64,
}

impl Position {
    /// Get zodiac sign index (0 = Aries, 11 = Pisces)
    pub fn sign(&self) -> u8 {
        ((self.longitude / 30.0) as u8) % 12
    }

    /// Get degree within sign (0-30)
    pub fn sign_degree(&self) -> f64 {
        self.longitude % 30.0
    }

    /// Is the planet retrograde?
    pub fn is_retrograde(&self) -> bool {
        self.speed_longitude < 0.0
    }
}

/// House cusps result
#[derive(Debug, Clone, Default)]
pub struct Houses {
    /// House cusps in degrees (index 1-12, index 0 unused)
    pub cusps: [f64; 13],
    /// Ascendant in degrees
    pub ascendant: f64,
    /// Midheaven (MC) in degrees
    pub mc: f64,
    /// ARMC (sidereal time * 15)
    pub armc: f64,
    /// Vertex
    pub vertex: f64,
}

/// Error type
#[derive(Debug, Clone)]
pub enum Error {
    InvalidDate,
    InvalidPlanet(i32),
    CalculationError(String),
    OutOfRange,
}

impl std::fmt::Display for Error {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            Error::InvalidDate => write!(f, "Invalid date"),
            Error::InvalidPlanet(id) => write!(f, "Invalid planet: {}", id),
            Error::CalculationError(msg) => write!(f, "Calculation error: {}", msg),
            Error::OutOfRange => write!(f, "Date out of range"),
        }
    }
}

impl std::error::Error for Error {}

pub type Result<T> = std::result::Result<T, Error>;

/// Calculate planet position
///
/// # Arguments
/// * `jd_ut` - Julian day (Universal Time)
/// * `planet` - Planet identifier
/// * `speed` - Whether to calculate speed
///
/// # Returns
/// Position with longitude, latitude, distance, and optionally speeds
pub fn calc_ut(jd_ut: f64, planet: Planet, speed: bool) -> Result<Position> {
    // Convert UT to ET (add delta-T)
    let jd_et = jd_ut + delta_t(jd_ut);

    match planet {
        Planet::Moon => moon::calc_moon(jd_et, speed),
        Planet::TrueNode => nodes::calc_true_node(jd_et, speed),
        _ => planets::calc_planet(jd_et, planet, speed),
    }
}

/// Calculate house cusps (Placidus system)
///
/// # Arguments
/// * `jd_ut` - Julian day (Universal Time)
/// * `lat` - Geographic latitude in degrees
/// * `lon` - Geographic longitude in degrees
///
/// # Returns
/// House cusps and angles
pub fn calc_houses(jd_ut: f64, lat: f64, lon: f64) -> Result<Houses> {
    houses::calc_houses_placidus(jd_ut, lat, lon)
}

/// Simple delta-T approximation (TT - UT in days)
/// Good enough for horoscope accuracy (~1 second)
pub fn delta_t(jd: f64) -> f64 {
    // Approximate delta-T in seconds
    let year = 2000.0 + (jd - J2000) / 365.25;

    let dt_seconds = if year < 1900.0 {
        // Before 1900: rough polynomial
        let t = (year - 1820.0) / 100.0;
        -20.0 + 32.0 * t * t
    } else if year < 1950.0 {
        // 1900-1950
        let t = year - 1900.0;
        -2.79 + 1.494119 * t - 0.0598939 * t * t + 0.0061966 * t * t * t
    } else if year < 2005.0 {
        // 1950-2005
        let t = year - 2000.0;
        63.86 + 0.3345 * t - 0.060374 * t * t + 0.0017275 * t * t * t
    } else if year < 2050.0 {
        // 2005-2050
        let t = year - 2000.0;
        62.92 + 0.32217 * t + 0.005589 * t * t
    } else {
        // After 2050: extrapolate
        let t = (year - 1820.0) / 100.0;
        -20.0 + 32.0 * t * t
    };

    // Convert seconds to days
    dt_seconds / 86400.0
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_planet_enum() {
        assert_eq!(Planet::Sun as i32, 0);
        assert_eq!(Planet::Moon as i32, 1);
        assert_eq!(Planet::TrueNode as i32, 11);

        assert_eq!(Planet::from_i32(0), Some(Planet::Sun));
        assert_eq!(Planet::from_i32(99), None);
    }

    #[test]
    fn test_position_sign() {
        let pos = Position { longitude: 45.0, ..Default::default() };
        assert_eq!(pos.sign(), 1); // Taurus
        assert!((pos.sign_degree() - 15.0).abs() < 0.001);
    }

    #[test]
    fn test_delta_t() {
        // Delta-T around 2000 should be about 63-64 seconds
        let dt = delta_t(J2000);
        assert!((dt * 86400.0 - 63.8).abs() < 1.0);
    }
}
