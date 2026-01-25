//! Moon position calculations using simplified ELP2000 theory
//!
//! Accuracy: ~10 arcseconds, sufficient for horoscope purposes.

use crate::constants::*;
use crate::math::*;
use crate::{Position, Result};

/// Calculate Moon position (geocentric ecliptic)
pub fn calc_moon(jd_et: f64, calc_speed: bool) -> Result<Position> {
    let t = (jd_et - J2000) / DAYS_PER_CENTURY;

    // Fundamental arguments (degrees)
    // Mean longitude of Moon
    let lp = deg_norm(218.3164477 + 481267.88123421 * t
        - 0.0015786 * t * t + t * t * t / 538841.0);

    // Mean elongation of Moon from Sun
    let d = deg_norm(297.8501921 + 445267.1114034 * t
        - 0.0018819 * t * t + t * t * t / 545868.0);

    // Mean anomaly of Sun
    let m = deg_norm(357.5291092 + 35999.0502909 * t
        - 0.0001536 * t * t);

    // Mean anomaly of Moon
    let mp = deg_norm(134.9633964 + 477198.8675055 * t
        + 0.0087414 * t * t + t * t * t / 69699.0);

    // Moon's argument of latitude
    let f = deg_norm(93.2720950 + 483202.0175233 * t
        - 0.0036539 * t * t);

    // Convert to radians
    let d_r = d * DEG_TO_RAD;
    let m_r = m * DEG_TO_RAD;
    let mp_r = mp * DEG_TO_RAD;
    let f_r = f * DEG_TO_RAD;

    // Additional arguments
    let a1 = deg_norm(119.75 + 131.849 * t) * DEG_TO_RAD;
    let a2 = deg_norm(53.09 + 479264.290 * t) * DEG_TO_RAD;
    let a3 = deg_norm(313.45 + 481266.484 * t) * DEG_TO_RAD;

    // Eccentricity correction
    let e = 1.0 - 0.002516 * t - 0.0000074 * t * t;
    let e2 = e * e;

    // Longitude terms (sum of sines)
    let sum_l =
          6288774.0 * mp_r.sin()
        + 1274027.0 * (2.0 * d_r - mp_r).sin()
        + 658314.0 * (2.0 * d_r).sin()
        + 213618.0 * (2.0 * mp_r).sin()
        - 185116.0 * m_r.sin() * e
        - 114332.0 * (2.0 * f_r).sin()
        + 58793.0 * (2.0 * d_r - 2.0 * mp_r).sin()
        + 57066.0 * (2.0 * d_r - m_r - mp_r).sin() * e
        + 53322.0 * (2.0 * d_r + mp_r).sin()
        + 45758.0 * (2.0 * d_r - m_r).sin() * e
        - 40923.0 * (m_r - mp_r).sin() * e
        - 34720.0 * d_r.sin()
        - 30383.0 * (m_r + mp_r).sin() * e
        + 15327.0 * (2.0 * d_r - 2.0 * f_r).sin()
        - 12528.0 * (mp_r + 2.0 * f_r).sin()
        + 10980.0 * (mp_r - 2.0 * f_r).sin()
        + 10675.0 * (4.0 * d_r - mp_r).sin()
        + 10034.0 * (3.0 * mp_r).sin()
        + 8548.0 * (4.0 * d_r - 2.0 * mp_r).sin()
        - 7888.0 * (2.0 * d_r + m_r - mp_r).sin() * e
        - 6766.0 * (2.0 * d_r + m_r).sin() * e
        - 5163.0 * (d_r - mp_r).sin()
        + 4987.0 * (d_r + m_r).sin() * e
        + 4036.0 * (2.0 * d_r - m_r + mp_r).sin() * e
        + 3994.0 * (2.0 * d_r + 2.0 * mp_r).sin()
        + 3861.0 * (4.0 * d_r).sin()
        + 3665.0 * (2.0 * d_r - 3.0 * mp_r).sin()
        - 2689.0 * (m_r - 2.0 * mp_r).sin() * e
        - 2602.0 * (2.0 * d_r - mp_r + 2.0 * f_r).sin()
        + 2390.0 * (2.0 * d_r - m_r - 2.0 * mp_r).sin() * e
        - 2348.0 * (d_r + mp_r).sin()
        + 2236.0 * (2.0 * d_r - 2.0 * m_r).sin() * e2
        - 2120.0 * (m_r + 2.0 * mp_r).sin() * e
        - 2069.0 * (2.0 * m_r).sin() * e2
        + 2048.0 * (2.0 * d_r - 2.0 * m_r - mp_r).sin() * e2
        - 1773.0 * (2.0 * d_r + mp_r - 2.0 * f_r).sin()
        - 1595.0 * (2.0 * d_r + 2.0 * f_r).sin()
        + 1215.0 * (4.0 * d_r - m_r - mp_r).sin() * e
        - 1110.0 * (2.0 * mp_r + 2.0 * f_r).sin()
        - 892.0 * (3.0 * d_r - mp_r).sin()
        - 810.0 * (2.0 * d_r + m_r + mp_r).sin() * e
        + 759.0 * (4.0 * d_r - m_r - 2.0 * mp_r).sin() * e
        - 713.0 * (2.0 * m_r - mp_r).sin() * e2
        - 700.0 * (2.0 * d_r + 2.0 * m_r - mp_r).sin() * e2
        + 691.0 * (2.0 * d_r + m_r - 2.0 * mp_r).sin() * e
        + 596.0 * (2.0 * d_r - m_r - 2.0 * f_r).sin() * e
        + 549.0 * (4.0 * d_r + mp_r).sin()
        + 537.0 * (4.0 * mp_r).sin()
        + 520.0 * (4.0 * d_r - m_r).sin() * e
        - 487.0 * (d_r - 2.0 * mp_r).sin()
        - 399.0 * (2.0 * d_r + m_r - 2.0 * f_r).sin() * e
        - 381.0 * (2.0 * mp_r - 2.0 * f_r).sin()
        + 351.0 * (d_r + m_r + mp_r).sin() * e
        - 340.0 * (3.0 * d_r - 2.0 * mp_r).sin()
        + 330.0 * (4.0 * d_r - 3.0 * mp_r).sin()
        + 327.0 * (2.0 * d_r - m_r + 2.0 * mp_r).sin() * e
        - 323.0 * (2.0 * m_r + mp_r).sin() * e2
        + 299.0 * (d_r + m_r - mp_r).sin() * e
        + 294.0 * (2.0 * d_r + 3.0 * mp_r).sin();

    // Additional longitude corrections
    let al = 3958.0 * a1.sin() + 1962.0 * (lp * DEG_TO_RAD - f_r).sin() + 318.0 * a2.sin();

    // Latitude terms
    let sum_b =
          5128122.0 * f_r.sin()
        + 280602.0 * (mp_r + f_r).sin()
        + 277693.0 * (mp_r - f_r).sin()
        + 173237.0 * (2.0 * d_r - f_r).sin()
        + 55413.0 * (2.0 * d_r - mp_r + f_r).sin()
        + 46271.0 * (2.0 * d_r - mp_r - f_r).sin()
        + 32573.0 * (2.0 * d_r + f_r).sin()
        + 17198.0 * (2.0 * mp_r + f_r).sin()
        + 9266.0 * (2.0 * d_r + mp_r - f_r).sin()
        + 8822.0 * (2.0 * mp_r - f_r).sin()
        + 8216.0 * (2.0 * d_r - m_r - f_r).sin() * e
        + 4324.0 * (2.0 * d_r - 2.0 * mp_r - f_r).sin()
        + 4200.0 * (2.0 * d_r + mp_r + f_r).sin()
        - 3359.0 * (2.0 * d_r + m_r - f_r).sin() * e
        + 2463.0 * (2.0 * d_r - m_r - mp_r + f_r).sin() * e
        + 2211.0 * (2.0 * d_r - m_r + f_r).sin() * e
        + 2065.0 * (2.0 * d_r - m_r - mp_r - f_r).sin() * e
        - 1870.0 * (m_r - mp_r - f_r).sin() * e
        + 1828.0 * (4.0 * d_r - mp_r - f_r).sin()
        - 1794.0 * (m_r + f_r).sin() * e
        - 1749.0 * (3.0 * f_r).sin()
        - 1565.0 * (m_r - mp_r + f_r).sin() * e
        - 1491.0 * (d_r + f_r).sin()
        - 1475.0 * (m_r + mp_r + f_r).sin() * e
        - 1410.0 * (m_r + mp_r - f_r).sin() * e
        - 1344.0 * (m_r - f_r).sin() * e
        - 1335.0 * (d_r - f_r).sin()
        + 1107.0 * (3.0 * mp_r + f_r).sin()
        + 1021.0 * (4.0 * d_r - f_r).sin()
        + 833.0 * (4.0 * d_r - mp_r + f_r).sin()
        + 777.0 * (mp_r - 3.0 * f_r).sin()
        + 671.0 * (4.0 * d_r - 2.0 * mp_r + f_r).sin()
        + 607.0 * (2.0 * d_r - 3.0 * f_r).sin()
        + 596.0 * (2.0 * d_r + 2.0 * mp_r - f_r).sin()
        + 491.0 * (2.0 * d_r - m_r + mp_r - f_r).sin() * e
        - 451.0 * (2.0 * d_r - 2.0 * mp_r + f_r).sin()
        + 439.0 * (3.0 * mp_r - f_r).sin()
        + 422.0 * (2.0 * d_r + 2.0 * mp_r + f_r).sin()
        + 421.0 * (2.0 * d_r - 3.0 * mp_r - f_r).sin()
        - 366.0 * (2.0 * d_r + m_r - mp_r + f_r).sin() * e
        - 351.0 * (2.0 * d_r + m_r + f_r).sin() * e
        + 331.0 * (4.0 * d_r + f_r).sin()
        + 315.0 * (2.0 * d_r - m_r + mp_r + f_r).sin() * e
        + 302.0 * (2.0 * d_r - 2.0 * m_r - f_r).sin() * e2
        - 283.0 * (mp_r + 3.0 * f_r).sin()
        - 229.0 * (2.0 * d_r + m_r + mp_r - f_r).sin() * e
        + 223.0 * (d_r + m_r - f_r).sin() * e
        + 223.0 * (d_r + m_r + f_r).sin() * e
        - 220.0 * (m_r - 2.0 * mp_r - f_r).sin() * e
        - 220.0 * (2.0 * d_r + m_r - mp_r - f_r).sin() * e
        - 185.0 * (d_r + mp_r + f_r).sin()
        + 181.0 * (2.0 * d_r - m_r - 2.0 * mp_r - f_r).sin() * e
        - 177.0 * (m_r + 2.0 * mp_r + f_r).sin() * e
        + 176.0 * (4.0 * d_r - 2.0 * mp_r - f_r).sin()
        + 166.0 * (4.0 * d_r - m_r - mp_r - f_r).sin() * e
        - 164.0 * (d_r + mp_r - f_r).sin()
        + 132.0 * (4.0 * d_r + mp_r - f_r).sin()
        - 119.0 * (d_r - mp_r - f_r).sin()
        + 115.0 * (4.0 * d_r - m_r - f_r).sin() * e
        + 107.0 * (2.0 * d_r - 2.0 * m_r + f_r).sin() * e2;

    // Additional latitude corrections
    let ab = -2235.0 * (lp * DEG_TO_RAD).sin()
        + 382.0 * a3.sin()
        + 175.0 * (a1 - f_r).sin()
        + 175.0 * (a1 + f_r).sin()
        + 127.0 * (lp * DEG_TO_RAD - mp_r).sin()
        - 115.0 * (lp * DEG_TO_RAD + mp_r).sin();

    // Distance terms
    let sum_r =
        -20905355.0 * mp_r.cos()
        - 3699111.0 * (2.0 * d_r - mp_r).cos()
        - 2955968.0 * (2.0 * d_r).cos()
        - 569925.0 * (2.0 * mp_r).cos()
        + 48888.0 * m_r.cos() * e
        - 3149.0 * (2.0 * f_r).cos()
        + 246158.0 * (2.0 * d_r - 2.0 * mp_r).cos()
        - 152138.0 * (2.0 * d_r - m_r - mp_r).cos() * e
        - 170733.0 * (2.0 * d_r + mp_r).cos()
        - 204586.0 * (2.0 * d_r - m_r).cos() * e
        - 129620.0 * (m_r - mp_r).cos() * e
        + 108743.0 * d_r.cos()
        + 104755.0 * (m_r + mp_r).cos() * e
        + 10321.0 * (2.0 * d_r - 2.0 * f_r).cos()
        + 79661.0 * (mp_r - 2.0 * f_r).cos()
        - 34782.0 * (4.0 * d_r - mp_r).cos()
        - 23210.0 * (3.0 * mp_r).cos()
        - 21636.0 * (4.0 * d_r - 2.0 * mp_r).cos()
        + 24208.0 * (2.0 * d_r + m_r - mp_r).cos() * e
        + 30824.0 * (2.0 * d_r + m_r).cos() * e
        - 8379.0 * (d_r - mp_r).cos()
        - 16675.0 * (d_r + m_r).cos() * e
        - 12831.0 * (2.0 * d_r - m_r + mp_r).cos() * e
        - 10445.0 * (2.0 * d_r + 2.0 * mp_r).cos()
        - 11650.0 * (4.0 * d_r).cos()
        + 14403.0 * (2.0 * d_r - 3.0 * mp_r).cos()
        - 7003.0 * (m_r - 2.0 * mp_r).cos() * e
        + 10056.0 * (2.0 * d_r - m_r - 2.0 * mp_r).cos() * e
        + 6322.0 * (d_r + mp_r).cos()
        - 9884.0 * (2.0 * d_r - 2.0 * m_r).cos() * e2
        + 5751.0 * (m_r + 2.0 * mp_r).cos() * e;

    // Final values
    let longitude = deg_norm(lp + (sum_l + al) / 1000000.0);
    let latitude = (sum_b + ab) / 1000000.0;
    let distance = (385000.56 + sum_r / 1000.0) / AU_KM; // Convert km to AU

    // Speed calculation
    let speed = if calc_speed {
        let dt = 0.01;
        let pos2 = calc_moon(jd_et + dt, false)?;
        angle_diff(pos2.longitude, longitude) / dt
    } else {
        0.0
    };

    Ok(Position {
        longitude,
        latitude,
        distance,
        speed_longitude: speed,
        speed_latitude: 0.0,
        speed_distance: 0.0,
    })
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::julian::julday_greg;

    #[test]
    fn test_moon_position() {
        // Test Moon position at J2000
        let pos = calc_moon(J2000, true).unwrap();

        // Moon should return valid coordinates
        assert!(pos.longitude >= 0.0 && pos.longitude < 360.0);
        assert!(pos.latitude > -6.0 && pos.latitude < 6.0);
        assert!(pos.distance > 0.002 && pos.distance < 0.003); // ~0.0026 AU
    }

    #[test]
    fn test_moon_speed() {
        let jd = julday_greg(2024, 1, 15, 12.0);
        let pos = calc_moon(jd, true).unwrap();

        // Moon moves about 12-15 degrees per day
        assert!(pos.speed_longitude > 10.0 && pos.speed_longitude < 16.0);
    }
}
