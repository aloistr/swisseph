//! Julian day calculations
//!
//! Standard astronomical algorithms for Julian Day conversion.

use crate::constants::SE_GREG_CAL;

/// Convert calendar date to Julian Day number
///
/// # Arguments
/// * `year` - Year (astronomical: negative for BC)
/// * `month` - Month (1-12)
/// * `day` - Day of month
/// * `hour` - Hour as decimal (0.0-24.0)
/// * `gregflag` - Calendar: SE_GREG_CAL (1) for Gregorian, 0 for Julian
///
/// # Returns
/// Julian Day number
///
/// # Example
/// ```
/// use horoscope::julian::julday;
/// use horoscope::constants::SE_GREG_CAL;
///
/// // J2000.0 epoch: 2000-01-01 12:00 UT
/// let jd = julday(2000, 1, 1, 12.0, SE_GREG_CAL);
/// assert!((jd - 2451545.0).abs() < 0.0001);
/// ```
pub fn julday(year: i32, month: i32, day: i32, hour: f64, gregflag: i32) -> f64 {
    let mut u = year as f64;
    if month < 3 {
        u -= 1.0;
    }
    let u0 = u + 4712.0;
    let mut u1 = month as f64 + 1.0;
    if u1 < 4.0 {
        u1 += 12.0;
    }

    let mut jd = (u0 * 365.25).floor()
        + (30.6 * u1 + 0.000001).floor()
        + day as f64
        + hour / 24.0
        - 63.5;

    if gregflag == SE_GREG_CAL {
        let u2 = ((u).abs() / 100.0).floor() - ((u).abs() / 400.0).floor();
        let u2 = if u < 0.0 { -u2 } else { u2 };
        jd = jd - u2 + 2.0;
        if u < 0.0 && (u / 100.0) == (u / 100.0).floor() && (u / 400.0) != (u / 400.0).floor() {
            jd -= 1.0;
        }
    }

    jd
}

/// Convert Julian Day to calendar date
///
/// # Arguments
/// * `jd` - Julian Day number
/// * `gregflag` - Calendar: SE_GREG_CAL (1) for Gregorian, 0 for Julian
///
/// # Returns
/// (year, month, day, hour) tuple
pub fn revjul(jd: f64, gregflag: i32) -> (i32, i32, i32, f64) {
    let mut u0 = jd + 32082.5;

    if gregflag == SE_GREG_CAL {
        let mut u1 = u0 + (u0 / 36525.0).floor() - (u0 / 146100.0).floor() - 38.0;
        if jd >= 1830691.5 {
            u1 += 1.0;
        }
        u0 = u0 + (u1 / 36525.0).floor() - (u1 / 146100.0).floor() - 38.0;
    }

    let u2 = (u0 + 123.0).floor();
    let u3 = ((u2 - 122.2) / 365.25).floor();
    let u4 = ((u2 - (365.25 * u3).floor()) / 30.6001).floor();

    let mut month = (u4 - 1.0) as i32;
    if month > 12 {
        month -= 12;
    }

    let day = (u2 - (365.25 * u3).floor() - (30.6001 * u4).floor()) as i32;
    let year = (u3 + ((u4 - 2.0) / 12.0).floor() - 4800.0) as i32;
    let hour = (jd - (jd + 0.5).floor() + 0.5) * 24.0;

    (year, month, day, hour)
}

/// Convenience function: Julian day for Gregorian calendar
#[inline]
pub fn julday_greg(year: i32, month: i32, day: i32, hour: f64) -> f64 {
    julday(year, month, day, hour, SE_GREG_CAL)
}

/// Get calendar year from Julian Day (approximate)
#[inline]
pub fn jd_to_year(jd: f64) -> f64 {
    2000.0 + (jd - crate::constants::J2000) / 365.25
}

/// Get Julian Day from decimal year (approximate)
#[inline]
pub fn year_to_jd(year: f64) -> f64 {
    crate::constants::J2000 + (year - 2000.0) * 365.25
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_julday_j2000() {
        // J2000.0 = 2000-01-01 12:00 TT
        let jd = julday(2000, 1, 1, 12.0, SE_GREG_CAL);
        assert!((jd - 2451545.0).abs() < 1e-10);
    }

    #[test]
    fn test_julday_revjul_roundtrip() {
        let jd = julday(2024, 6, 15, 14.5, SE_GREG_CAL);
        let (y, m, d, h) = revjul(jd, SE_GREG_CAL);
        assert_eq!(y, 2024);
        assert_eq!(m, 6);
        assert_eq!(d, 15);
        assert!((h - 14.5).abs() < 1e-6);
    }

    #[test]
    fn test_julday_negative_years() {
        // Historical date: 44 BC (year -43 astronomical)
        let jd = julday(-43, 3, 15, 12.0, SE_GREG_CAL);
        let (y, _, _, _) = revjul(jd, SE_GREG_CAL);
        assert_eq!(y, -43);
    }

    #[test]
    fn test_known_dates() {
        // Unix epoch: 1970-01-01 00:00
        let jd = julday(1970, 1, 1, 0.0, SE_GREG_CAL);
        assert!((jd - 2440587.5).abs() < 0.0001);

        // A known date
        let jd = julday(1999, 12, 31, 0.0, SE_GREG_CAL);
        assert!((jd - 2451543.5).abs() < 0.0001);
    }
}
