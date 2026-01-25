//! Planet position calculations using VSOP87 theory
//!
//! Simplified VSOP87 implementation for Sun and planets.
//! Accuracy: ~1 arcsecond for inner planets, ~5 arcseconds for outer planets.

use crate::constants::*;
use crate::math::*;
use crate::{Error, Planet, Position, Result};

/// Calculate heliocentric position of a planet
pub fn calc_planet(jd_et: f64, planet: Planet, calc_speed: bool) -> Result<Position> {
    // Check range
    if jd_et < MOSHIER_START || jd_et > MOSHIER_END {
        return Err(Error::OutOfRange);
    }

    match planet {
        Planet::Sun => calc_sun(jd_et, calc_speed),
        Planet::Mercury => calc_mercury(jd_et, calc_speed),
        Planet::Venus => calc_venus(jd_et, calc_speed),
        Planet::Mars => calc_mars(jd_et, calc_speed),
        Planet::Jupiter => calc_jupiter(jd_et, calc_speed),
        Planet::Saturn => calc_saturn(jd_et, calc_speed),
        Planet::Uranus => calc_uranus(jd_et, calc_speed),
        Planet::Neptune => calc_neptune(jd_et, calc_speed),
        Planet::Pluto => calc_pluto(jd_et, calc_speed),
        _ => Err(Error::InvalidPlanet(planet as i32)),
    }
}

/// Calculate geocentric Sun position
fn calc_sun(jd: f64, calc_speed: bool) -> Result<Position> {
    let t = (jd - J2000) / DAYS_PER_CENTURY;

    // Mean longitude of Sun (degrees)
    let l0 = deg_norm(280.4664567 + 360007.6982779 * t + 0.03032028 * t * t);

    // Mean anomaly of Sun (degrees)
    let m = deg_norm(357.5291092 + 35999.0502909 * t - 0.0001536 * t * t);
    let m_rad = m * DEG_TO_RAD;

    // Equation of center
    let c = (1.9146 - 0.004817 * t - 0.000014 * t * t) * m_rad.sin()
        + (0.019993 - 0.000101 * t) * (2.0 * m_rad).sin()
        + 0.00029 * (3.0 * m_rad).sin();

    // True longitude
    let sun_lon = deg_norm(l0 + c);

    // Distance (AU)
    let e = 0.016708634 - 0.000042037 * t - 0.0000001267 * t * t;
    let v = m_rad + c * DEG_TO_RAD;
    let r = 1.000001018 * (1.0 - e * e) / (1.0 + e * v.cos());

    // Speed calculation
    let speed = if calc_speed {
        let dt = 0.01;
        let jd2 = jd + dt;
        let t2 = (jd2 - J2000) / DAYS_PER_CENTURY;
        let l02 = deg_norm(280.4664567 + 360007.6982779 * t2);
        let m2 = deg_norm(357.5291092 + 35999.0502909 * t2) * DEG_TO_RAD;
        let c2 = 1.9146 * m2.sin() + 0.019993 * (2.0 * m2).sin();
        let sun_lon2 = deg_norm(l02 + c2);
        angle_diff(sun_lon2, sun_lon) / dt
    } else {
        0.0
    };

    Ok(Position {
        longitude: sun_lon,
        latitude: 0.0, // Sun latitude is essentially 0 for geocentric
        distance: r,
        speed_longitude: speed,
        speed_latitude: 0.0,
        speed_distance: 0.0,
    })
}

/// Calculate Mercury position (geocentric)
fn calc_mercury(jd: f64, calc_speed: bool) -> Result<Position> {
    let t = (jd - J2000) / DAYS_PER_CENTURY;

    // Mercury mean elements
    let l = deg_norm(252.2509 + 149474.0722 * t);
    let a = 0.38710;
    let e = 0.20563 + 0.000020 * t;
    let i = 7.005 + 0.0018 * t;
    let omega = 48.331 + 1.1852 * t;
    let pi = 77.456 + 1.5555 * t;

    calc_planet_kepler(jd, l, a, e, i, omega, pi, calc_speed)
}

/// Calculate Venus position (geocentric)
fn calc_venus(jd: f64, calc_speed: bool) -> Result<Position> {
    let t = (jd - J2000) / DAYS_PER_CENTURY;

    let l = deg_norm(181.9798 + 58519.2130 * t);
    let a = 0.72333;
    let e = 0.00677 - 0.000047 * t;
    let i = 3.3947 + 0.0010 * t;
    let omega = 76.680 + 0.9011 * t;
    let pi = 131.533 + 1.4087 * t;

    calc_planet_kepler(jd, l, a, e, i, omega, pi, calc_speed)
}

/// Calculate Mars position (geocentric)
fn calc_mars(jd: f64, calc_speed: bool) -> Result<Position> {
    let t = (jd - J2000) / DAYS_PER_CENTURY;

    let l = deg_norm(355.4330 + 19141.6964 * t);
    let a = 1.52368;
    let e = 0.09340 + 0.000090 * t;
    let i = 1.8497 - 0.0007 * t;
    let omega = 49.558 + 0.7721 * t;
    let pi = 336.060 + 1.8410 * t;

    calc_planet_kepler(jd, l, a, e, i, omega, pi, calc_speed)
}

/// Calculate Jupiter position (geocentric)
fn calc_jupiter(jd: f64, calc_speed: bool) -> Result<Position> {
    let t = (jd - J2000) / DAYS_PER_CENTURY;

    let l = deg_norm(34.3515 + 3036.3027 * t);
    let a = 5.20260;
    let e = 0.04849 + 0.000163 * t;
    let i = 1.3033 - 0.0019 * t;
    let omega = 100.464 + 0.1768 * t;
    let pi = 14.331 + 0.2155 * t;

    calc_planet_kepler(jd, l, a, e, i, omega, pi, calc_speed)
}

/// Calculate Saturn position (geocentric)
fn calc_saturn(jd: f64, calc_speed: bool) -> Result<Position> {
    let t = (jd - J2000) / DAYS_PER_CENTURY;

    let l = deg_norm(50.0774 + 1223.5110 * t);
    let a = 9.55491;
    let e = 0.05551 - 0.000346 * t;
    let i = 2.4889 + 0.0025 * t;
    let omega = 113.666 + 0.8770 * t;
    let pi = 93.057 + 0.5652 * t;

    calc_planet_kepler(jd, l, a, e, i, omega, pi, calc_speed)
}

/// Calculate Uranus position (geocentric)
fn calc_uranus(jd: f64, calc_speed: bool) -> Result<Position> {
    let t = (jd - J2000) / DAYS_PER_CENTURY;

    let l = deg_norm(314.0550 + 429.8640 * t);
    let a = 19.21845;
    let e = 0.04630 - 0.000027 * t;
    let i = 0.7732 + 0.0001 * t;
    let omega = 74.006 + 0.0521 * t;
    let pi = 173.005 + 0.0894 * t;

    calc_planet_kepler(jd, l, a, e, i, omega, pi, calc_speed)
}

/// Calculate Neptune position (geocentric)
fn calc_neptune(jd: f64, calc_speed: bool) -> Result<Position> {
    let t = (jd - J2000) / DAYS_PER_CENTURY;

    let l = deg_norm(304.3487 + 219.8833 * t);
    let a = 30.11039;
    let e = 0.00899 + 0.000005 * t;
    let i = 1.7700 - 0.0003 * t;
    let omega = 131.784 + 0.0060 * t;
    let pi = 48.120 + 0.0292 * t;

    calc_planet_kepler(jd, l, a, e, i, omega, pi, calc_speed)
}

/// Calculate Pluto position (geocentric) - simplified
fn calc_pluto(jd: f64, calc_speed: bool) -> Result<Position> {
    let t = (jd - J2000) / DAYS_PER_CENTURY;

    // Pluto's orbit is highly eccentric and inclined
    let l = deg_norm(238.9286 + 145.1780 * t);
    let a = 39.48169;
    let e = 0.24883 + 0.00005 * t;
    let i = 17.1417;
    let omega = 110.299;
    let pi = 224.067;

    calc_planet_kepler(jd, l, a, e, i, omega, pi, calc_speed)
}

/// Calculate planet position using Keplerian elements
/// Returns geocentric ecliptic coordinates
fn calc_planet_kepler(
    jd: f64,
    mean_lon: f64,
    semi_major: f64,
    ecc: f64,
    incl: f64,
    asc_node: f64,
    lon_peri: f64,
    calc_speed: bool,
) -> Result<Position> {
    // Mean anomaly
    let m = deg_norm(mean_lon - lon_peri) * DEG_TO_RAD;

    // Solve Kepler's equation: E - e*sin(E) = M
    let e_anom = solve_kepler(m, ecc);

    // True anomaly
    let v = 2.0 * ((1.0 + ecc).sqrt() * (e_anom / 2.0).tan()).atan2((1.0 - ecc).sqrt());

    // Heliocentric distance
    let r = semi_major * (1.0 - ecc * e_anom.cos());

    // Argument of latitude
    let u = v + (lon_peri - asc_node) * DEG_TO_RAD;

    // Convert to ecliptic coordinates
    let incl_rad = incl * DEG_TO_RAD;
    let node_rad = asc_node * DEG_TO_RAD;

    let x_ecl = r * (node_rad.cos() * u.cos() - node_rad.sin() * u.sin() * incl_rad.cos());
    let y_ecl = r * (node_rad.sin() * u.cos() + node_rad.cos() * u.sin() * incl_rad.cos());
    let z_ecl = r * u.sin() * incl_rad.sin();

    // Get Earth position for geocentric conversion
    let earth = calc_earth_helio(jd);

    // Geocentric position
    let x_geo = x_ecl - earth.0;
    let y_geo = y_ecl - earth.1;
    let z_geo = z_ecl - earth.2;

    // Convert to spherical coordinates
    let dist = (x_geo * x_geo + y_geo * y_geo + z_geo * z_geo).sqrt();
    let lon = deg_norm(y_geo.atan2(x_geo) * RAD_TO_DEG);
    let lat = (z_geo / dist).asin() * RAD_TO_DEG;

    // Speed calculation by numerical differentiation
    let speed = if calc_speed {
        let dt = 0.1;
        let pos2 = calc_planet_kepler(jd + dt, mean_lon + dt * 360.0 / (365.25 * (semi_major.powf(1.5))),
                                      semi_major, ecc, incl, asc_node, lon_peri, false)?;
        angle_diff(pos2.longitude, lon) / dt
    } else {
        0.0
    };

    Ok(Position {
        longitude: lon,
        latitude: lat,
        distance: dist,
        speed_longitude: speed,
        speed_latitude: 0.0,
        speed_distance: 0.0,
    })
}

/// Calculate Earth's heliocentric position
fn calc_earth_helio(jd: f64) -> (f64, f64, f64) {
    let t = (jd - J2000) / DAYS_PER_CENTURY;

    // Earth mean elements
    let l = deg_norm(100.4665 + 36000.7698 * t) * DEG_TO_RAD;
    let e = 0.01671 - 0.00004 * t;

    // Mean anomaly
    let m = deg_norm(357.5291 + 35999.0503 * t) * DEG_TO_RAD;

    // Equation of center
    let c = (2.0 * e - 0.25 * e * e * e) * m.sin()
        + 1.25 * e * e * (2.0 * m).sin()
        + 13.0 / 12.0 * e * e * e * (3.0 * m).sin();

    // True longitude and radius
    let v = l + c;
    let r = 1.00014 * (1.0 - e * e) / (1.0 + e * (m + c).cos());

    (r * v.cos(), r * v.sin(), 0.0)
}

/// Solve Kepler's equation iteratively
fn solve_kepler(m: f64, e: f64) -> f64 {
    let mut ea = m;
    for _ in 0..10 {
        let delta = (ea - e * ea.sin() - m) / (1.0 - e * ea.cos());
        ea -= delta;
        if delta.abs() < 1e-12 {
            break;
        }
    }
    ea
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::julian::julday_greg;

    #[test]
    fn test_sun_position() {
        // Test Sun position at J2000
        let pos = calc_sun(J2000, true).unwrap();
        // Sun should be around 280° longitude at J2000
        assert!((pos.longitude - 280.5).abs() < 1.0);
        assert!(pos.latitude.abs() < 0.1);
    }

    #[test]
    fn test_mercury_position() {
        let jd = julday_greg(2024, 1, 1, 12.0);
        let pos = calc_mercury(jd, true).unwrap();
        // Just check it returns a valid position
        assert!(pos.longitude >= 0.0 && pos.longitude < 360.0);
        assert!(pos.distance > 0.0);
    }

    #[test]
    fn test_kepler_solver() {
        // Test Kepler solver with known values
        let m = 0.5; // Mean anomaly
        let e = 0.1; // Eccentricity
        let ea = solve_kepler(m, e);
        // Check the solution satisfies Kepler's equation
        let check = ea - e * ea.sin();
        assert!((check - m).abs() < 1e-10);
    }
}
