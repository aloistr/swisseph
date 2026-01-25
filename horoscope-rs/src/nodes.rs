//! Lunar node calculations
//!
//! Calculates the True (osculating) North Node of the Moon.

use crate::constants::*;
use crate::math::*;
use crate::{Position, Result};

/// Calculate True Lunar Node position
pub fn calc_true_node(jd_et: f64, calc_speed: bool) -> Result<Position> {
    let t = (jd_et - J2000) / DAYS_PER_CENTURY;

    // Mean longitude of ascending node
    let omega_mean = deg_norm(125.0445479 - 1934.1362891 * t
        + 0.0020754 * t * t
        + t * t * t / 467441.0
        - t * t * t * t / 60616000.0);

    // Mean elongation of Moon
    let d = deg_norm(297.8501921 + 445267.1114034 * t
        - 0.0018819 * t * t) * DEG_TO_RAD;

    // Mean anomaly of Sun
    let m = deg_norm(357.5291092 + 35999.0502909 * t) * DEG_TO_RAD;

    // Mean anomaly of Moon
    let mp = deg_norm(134.9633964 + 477198.8675055 * t) * DEG_TO_RAD;

    // Moon's argument of latitude
    let f = deg_norm(93.2720950 + 483202.0175233 * t) * DEG_TO_RAD;

    // Perturbations to the node (in degrees)
    let delta_omega =
        -1.4979 * (2.0 * (d - f)).sin()
        - 0.1500 * m.sin()
        - 0.1226 * (2.0 * d).sin()
        + 0.1176 * (2.0 * f).sin()
        - 0.0801 * (2.0 * (mp - f)).sin();

    // True node
    let omega = deg_norm(omega_mean + delta_omega);

    // Speed calculation
    let speed = if calc_speed {
        let dt = 0.1;
        let pos2 = calc_true_node(jd_et + dt, false)?;
        angle_diff(pos2.longitude, omega) / dt
    } else {
        // Mean daily motion of the node (retrograde)
        -0.0529539
    };

    Ok(Position {
        longitude: omega,
        latitude: 0.0,
        distance: 0.0,
        speed_longitude: speed,
        speed_latitude: 0.0,
        speed_distance: 0.0,
    })
}

/// Calculate Mean Lunar Node position
pub fn calc_mean_node(jd_et: f64) -> f64 {
    let t = (jd_et - J2000) / DAYS_PER_CENTURY;

    deg_norm(125.0445479 - 1934.1362891 * t
        + 0.0020754 * t * t
        + t * t * t / 467441.0)
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::julian::julday_greg;

    #[test]
    fn test_true_node() {
        let jd = julday_greg(2024, 1, 1, 12.0);
        let pos = calc_true_node(jd, true).unwrap();

        // Node should be between 0 and 360
        assert!(pos.longitude >= 0.0 && pos.longitude < 360.0);

        // Node moves retrograde (negative speed)
        assert!(pos.speed_longitude < 0.0);
    }

    #[test]
    fn test_mean_node() {
        // At J2000, mean node should be around 125°
        let node = calc_mean_node(J2000);
        assert!((node - 125.0).abs() < 1.0);
    }
}
