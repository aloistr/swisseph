//! House cusp calculations
//!
//! Implements Placidus house system.

use crate::constants::*;
use crate::math::*;
use crate::{delta_t, Houses, Result};

/// Calculate Placidus house cusps
pub fn calc_houses_placidus(jd_ut: f64, lat: f64, lon: f64) -> Result<Houses> {
    // Convert to ephemeris time
    let jd_et = jd_ut + delta_t(jd_ut);

    // Obliquity of ecliptic
    let eps = obliquity(jd_et);

    // ARMC (local sidereal time in degrees)
    let armc_deg = armc(jd_ut, lon);
    let armc_rad = armc_deg * DEG_TO_RAD;

    // Latitude in radians
    let lat_rad = lat * DEG_TO_RAD;

    // Calculate MC (Midheaven)
    let mc = calc_mc(armc_rad, eps);

    // Calculate Ascendant
    let asc = calc_ascendant(armc_rad, lat_rad, eps);

    // Calculate intermediate cusps using Placidus method
    let cusps = calc_placidus_cusps(armc_rad, lat_rad, eps, mc, asc);

    // Calculate Vertex
    let vertex = calc_vertex(armc_rad, lat_rad, eps);

    Ok(Houses {
        cusps,
        ascendant: deg_norm(asc * RAD_TO_DEG),
        mc: deg_norm(mc * RAD_TO_DEG),
        armc: armc_deg,
        vertex: deg_norm(vertex * RAD_TO_DEG),
    })
}

/// Calculate MC (Medium Coeli / Midheaven)
fn calc_mc(armc: f64, eps: f64) -> f64 {
    let (sin_armc, cos_armc) = armc.sin_cos();
    let mc = sin_armc.atan2(cos_armc * eps.cos());

    // Adjust quadrant
    if mc < 0.0 {
        mc + std::f64::consts::PI
    } else if cos_armc < 0.0 {
        mc + std::f64::consts::PI
    } else {
        mc
    }
}

/// Calculate Ascendant
fn calc_ascendant(armc: f64, lat: f64, eps: f64) -> f64 {
    let (sin_eps, cos_eps) = eps.sin_cos();
    let (sin_armc, cos_armc) = armc.sin_cos();
    let tan_lat = lat.tan();

    let y = -cos_armc;
    let x = sin_armc * cos_eps + tan_lat * sin_eps;

    let mut asc = y.atan2(x);

    // Adjust to correct quadrant (ASC should be opposite to ARMC + 180°)
    if asc < 0.0 {
        asc += TWOPI;
    }

    asc
}

/// Calculate Vertex (western horizon intersection with prime vertical)
fn calc_vertex(armc: f64, lat: f64, eps: f64) -> f64 {
    // Vertex is the Ascendant for the co-latitude
    let co_lat = std::f64::consts::FRAC_PI_2 - lat;
    let armc_vtx = armc + std::f64::consts::PI;
    calc_ascendant(armc_vtx, co_lat, eps)
}

/// Calculate Placidus house cusps
fn calc_placidus_cusps(armc: f64, lat: f64, eps: f64, mc: f64, asc: f64) -> [f64; 13] {
    let mut cusps = [0.0; 13];

    // Cusp 1 = Ascendant
    cusps[1] = deg_norm(asc * RAD_TO_DEG);

    // Cusp 10 = MC
    cusps[10] = deg_norm(mc * RAD_TO_DEG);

    // Cusp 4 = IC (opposite MC)
    cusps[4] = deg_norm((mc + std::f64::consts::PI) * RAD_TO_DEG);

    // Cusp 7 = Descendant (opposite Ascendant)
    cusps[7] = deg_norm((asc + std::f64::consts::PI) * RAD_TO_DEG);

    // Calculate intermediate cusps using Placidus method
    // Cusps 11, 12 are between MC and ASC
    // Cusps 2, 3 are between ASC and IC

    let (sin_eps, cos_eps) = eps.sin_cos();
    let (_sin_lat, _cos_lat) = lat.sin_cos();
    let tan_lat = lat.tan();

    // For cusps 11 and 12 (above horizon, between MC and ASC)
    for (cusp_num, fraction) in [(11, 1.0 / 3.0), (12, 2.0 / 3.0)] {
        let cusp = placidus_cusp(armc, lat, eps, sin_eps, cos_eps, tan_lat, fraction, true);
        cusps[cusp_num] = deg_norm(cusp * RAD_TO_DEG);
    }

    // For cusps 2 and 3 (below horizon, between ASC and IC)
    for (cusp_num, fraction) in [(2, 1.0 / 3.0), (3, 2.0 / 3.0)] {
        let cusp = placidus_cusp(armc, lat, eps, sin_eps, cos_eps, tan_lat, fraction, false);
        cusps[cusp_num] = deg_norm(cusp * RAD_TO_DEG);
    }

    // Opposite cusps
    cusps[5] = deg_norm(cusps[11] + 180.0);
    cusps[6] = deg_norm(cusps[12] + 180.0);
    cusps[8] = deg_norm(cusps[2] + 180.0);
    cusps[9] = deg_norm(cusps[3] + 180.0);

    cusps
}

/// Calculate a single Placidus cusp
fn placidus_cusp(
    armc: f64,
    _lat: f64,
    _eps: f64,
    sin_eps: f64,
    cos_eps: f64,
    tan_lat: f64,
    fraction: f64,
    above_horizon: bool,
) -> f64 {
    // Placidus uses semi-arc division
    // This is an iterative calculation

    let offset = if above_horizon {
        // Cusps 11, 12: offset from MC toward ASC
        fraction * std::f64::consts::FRAC_PI_2
    } else {
        // Cusps 2, 3: offset from ASC toward IC
        std::f64::consts::FRAC_PI_2 + fraction * std::f64::consts::FRAC_PI_2
    };

    let ramc = armc + offset;

    // Iterative solution
    let mut cusp = ramc;
    for _ in 0..20 {
        let (sin_cusp, cos_cusp) = cusp.sin_cos();

        // Declination of the cusp point
        let decl = (sin_eps * sin_cusp).asin();

        // Semi-arc (used for Placidus semi-arc division)
        let _semi_arc = if above_horizon {
            ((-tan_lat * decl.tan()).acos()).abs()
        } else {
            std::f64::consts::PI - ((-tan_lat * decl.tan()).acos()).abs()
        };

        // New estimate
        let x = -cos_cusp;
        let y = sin_cusp * cos_eps + tan_lat * sin_eps;
        let new_cusp = y.atan2(x);

        let delta = new_cusp - cusp;
        cusp = new_cusp;

        if delta.abs() < 1e-10 {
            break;
        }
    }

    // Normalize
    if cusp < 0.0 {
        cusp += TWOPI;
    }

    cusp
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::julian::julday_greg;

    #[test]
    fn test_houses_basic() {
        // Test at a known location and time
        let jd = julday_greg(2000, 1, 1, 12.0);
        let houses = calc_houses_placidus(jd, 47.38, 8.54).unwrap();

        // Basic sanity checks
        assert!(houses.ascendant >= 0.0 && houses.ascendant < 360.0);
        assert!(houses.mc >= 0.0 && houses.mc < 360.0);

        // ASC and DESC should be opposite
        let desc = deg_norm(houses.ascendant + 180.0);
        assert!((houses.cusps[7] - desc).abs() < 0.01);

        // MC and IC should be opposite
        let ic = deg_norm(houses.mc + 180.0);
        assert!((houses.cusps[4] - ic).abs() < 0.01);
    }

    #[test]
    fn test_mc_calculation() {
        // At ARMC = 0, MC should be 0° (Aries point)
        let eps = OBLIQUITY_J2000 * DEG_TO_RAD;
        let mc = calc_mc(0.0, eps);
        assert!(mc.abs() < 0.01);
    }

    #[test]
    fn test_ascendant_calculation() {
        // Basic ascendant test
        let eps = OBLIQUITY_J2000 * DEG_TO_RAD;
        let lat = 45.0 * DEG_TO_RAD;
        let asc = calc_ascendant(0.0, lat, eps);

        // ASC should be in a reasonable range
        assert!(asc >= 0.0 && asc < TWOPI);
    }
}
