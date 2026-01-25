//! Mathematical utilities for astronomical calculations
//!
//! Coordinate transformations, angle normalization, etc.

use crate::constants::*;

/// Normalize angle to 0..360 degrees
#[inline]
pub fn deg_norm(x: f64) -> f64 {
    let mut y = x % 360.0;
    if y.abs() < 1e-13 {
        y = 0.0;
    }
    if y < 0.0 {
        y += 360.0;
    }
    y
}

/// Normalize angle to 0..2π radians
#[inline]
pub fn rad_norm(x: f64) -> f64 {
    let mut y = x % TWOPI;
    if y < 0.0 {
        y += TWOPI;
    }
    y
}

/// Normalize angle to -180..180 degrees
#[inline]
pub fn deg_norm_180(x: f64) -> f64 {
    let mut y = deg_norm(x);
    if y > 180.0 {
        y -= 360.0;
    }
    y
}

/// Convert polar to Cartesian coordinates
/// Input: [longitude_rad, latitude_rad, distance]
/// Output: [x, y, z]
pub fn pol_to_cart(pol: &[f64; 3]) -> [f64; 3] {
    let (sin_lon, cos_lon) = pol[0].sin_cos();
    let (sin_lat, cos_lat) = pol[1].sin_cos();
    [
        pol[2] * cos_lat * cos_lon,
        pol[2] * cos_lat * sin_lon,
        pol[2] * sin_lat,
    ]
}

/// Convert Cartesian to polar coordinates
/// Input: [x, y, z]
/// Output: [longitude_rad, latitude_rad, distance]
pub fn cart_to_pol(cart: &[f64; 3]) -> [f64; 3] {
    let rxy = (cart[0] * cart[0] + cart[1] * cart[1]).sqrt();
    let mut lon = 0.0;
    let mut lat = 0.0;

    if rxy.abs() > 1e-16 || cart[2].abs() > 1e-16 {
        lon = cart[1].atan2(cart[0]);
        if lon < 0.0 {
            lon += TWOPI;
        }
        lat = cart[2].atan2(rxy);
    }

    let dist = (cart[0] * cart[0] + cart[1] * cart[1] + cart[2] * cart[2]).sqrt();
    [lon, lat, dist]
}

/// Coordinate transformation (rotation around X axis)
/// Used for ecliptic <-> equatorial conversion
/// eps > 0: equatorial -> ecliptic
/// eps < 0: ecliptic -> equatorial
pub fn coord_trans(x_in: &[f64; 3], eps: f64) -> [f64; 3] {
    let (sin_eps, cos_eps) = eps.sin_cos();
    [
        x_in[0],
        x_in[1] * cos_eps + x_in[2] * sin_eps,
        -x_in[1] * sin_eps + x_in[2] * cos_eps,
    ]
}

/// Obliquity of the ecliptic (mean, IAU 2006)
/// Returns obliquity in radians
pub fn obliquity(jd: f64) -> f64 {
    // Julian centuries from J2000
    let t = (jd - J2000) / DAYS_PER_CENTURY;

    // IAU 2006 polynomial (arcseconds)
    let eps = 84381.406
        - 46.836769 * t
        - 0.0001831 * t * t
        + 0.00200340 * t * t * t
        - 0.000000576 * t * t * t * t
        - 0.0000000434 * t * t * t * t * t;

    eps * ARCSEC_TO_RAD
}

/// Sidereal time at Greenwich (mean, in hours)
/// jd_ut: Julian Day in UT
pub fn sidereal_time(jd_ut: f64) -> f64 {
    // Julian centuries from J2000 at 0h UT
    let jd0 = (jd_ut - 0.5).floor() + 0.5;
    let t = (jd0 - J2000) / DAYS_PER_CENTURY;
    let ut = (jd_ut - jd0) * 24.0;

    // Mean sidereal time at 0h UT (hours)
    let gmst0 = 6.697374558
        + 2400.051336 * t
        + 0.000025862 * t * t
        - 0.0000000017 * t * t * t;

    // Add UT
    let gmst = gmst0 + ut * 1.00273790935;

    // Normalize to 0..24
    let mut result = gmst % 24.0;
    if result < 0.0 {
        result += 24.0;
    }
    result
}

/// Local sidereal time in hours
pub fn local_sidereal_time(jd_ut: f64, longitude: f64) -> f64 {
    let gmst = sidereal_time(jd_ut);
    let lst = gmst + longitude / 15.0;
    let mut result = lst % 24.0;
    if result < 0.0 {
        result += 24.0;
    }
    result
}

/// ARMC (local sidereal time in degrees)
pub fn armc(jd_ut: f64, longitude: f64) -> f64 {
    local_sidereal_time(jd_ut, longitude) * 15.0
}

/// Difference between two angles (result in -180..180)
pub fn angle_diff(a1: f64, a2: f64) -> f64 {
    deg_norm_180(a1 - a2)
}

/// Sine and cosine lookup optimization helper
pub struct SinCosTable {
    pub sin: [f64; 24],
    pub cos: [f64; 24],
}

impl SinCosTable {
    /// Compute sin(k*arg) and cos(k*arg) for k = 1..n
    pub fn new(arg: f64, n: usize) -> Self {
        let mut table = SinCosTable {
            sin: [0.0; 24],
            cos: [0.0; 24],
        };

        if n == 0 {
            return table;
        }

        let (su, cu) = arg.sin_cos();
        table.sin[0] = su;
        table.cos[0] = cu;

        if n >= 2 {
            let sv = 2.0 * su * cu;
            let cv = cu * cu - su * su;
            table.sin[1] = sv;
            table.cos[1] = cv;

            let mut sv_prev = sv;
            let mut cv_prev = cv;

            for i in 2..n.min(24) {
                let s_new = su * cv_prev + cu * sv_prev;
                let c_new = cu * cv_prev - su * sv_prev;
                table.sin[i] = s_new;
                table.cos[i] = c_new;
                sv_prev = s_new;
                cv_prev = c_new;
            }
        }

        table
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_deg_norm() {
        assert!((deg_norm(0.0) - 0.0).abs() < 1e-10);
        assert!((deg_norm(360.0) - 0.0).abs() < 1e-10);
        assert!((deg_norm(-90.0) - 270.0).abs() < 1e-10);
        assert!((deg_norm(720.0) - 0.0).abs() < 1e-10);
        assert!((deg_norm(450.0) - 90.0).abs() < 1e-10);
    }

    #[test]
    fn test_pol_cart_roundtrip() {
        let pol = [1.0, 0.5, 2.0]; // lon, lat (radians), dist
        let cart = pol_to_cart(&pol);
        let pol2 = cart_to_pol(&cart);

        assert!((pol[0] - pol2[0]).abs() < 1e-10);
        assert!((pol[1] - pol2[1]).abs() < 1e-10);
        assert!((pol[2] - pol2[2]).abs() < 1e-10);
    }

    #[test]
    fn test_obliquity() {
        // Obliquity at J2000 should be about 23.439°
        let eps = obliquity(J2000) * RAD_TO_DEG;
        assert!((eps - 23.4393).abs() < 0.001);
    }

    #[test]
    fn test_sidereal_time() {
        // At J2000 (2000-01-01 12:00 UT), GMST should be about 18.7 hours
        let gmst = sidereal_time(J2000);
        assert!((gmst - 18.7).abs() < 0.1);
    }

    #[test]
    fn test_sincos_table() {
        let arg = 1.0;
        let table = SinCosTable::new(arg, 5);

        for i in 0..5 {
            let k = (i + 1) as f64;
            assert!((table.sin[i] - (k * arg).sin()).abs() < 1e-10);
            assert!((table.cos[i] - (k * arg).cos()).abs() < 1e-10);
        }
    }
}
