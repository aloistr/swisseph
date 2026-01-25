//! WebAssembly bindings for horoscope calculations
//!
//! Provides a JavaScript-compatible API matching sweph-wasm interface.

use wasm_bindgen::prelude::*;
use serde::{Deserialize, Serialize};

use crate::{calc_ut, calc_houses, julian, Planet, Position, Houses, constants};

/// Planet position result for JavaScript
#[derive(Serialize, Deserialize)]
pub struct JsPosition {
    pub longitude: f64,
    pub latitude: f64,
    pub distance: f64,
    #[serde(rename = "longitudeSpeed")]
    pub longitude_speed: f64,
}

impl From<Position> for JsPosition {
    fn from(p: Position) -> Self {
        JsPosition {
            longitude: p.longitude,
            latitude: p.latitude,
            distance: p.distance,
            longitude_speed: p.speed_longitude,
        }
    }
}

/// House calculation result for JavaScript
#[derive(Serialize, Deserialize)]
pub struct JsHouses {
    pub cusps: Vec<f64>,
    pub ascendant: f64,
    pub mc: f64,
    pub armc: f64,
    pub vertex: f64,
}

impl From<Houses> for JsHouses {
    fn from(h: Houses) -> Self {
        JsHouses {
            cusps: h.cusps[1..=12].to_vec(),
            ascendant: h.ascendant,
            mc: h.mc,
            armc: h.armc,
            vertex: h.vertex,
        }
    }
}

/// Calculate Julian Day from calendar date
///
/// # Arguments
/// * `year` - Year (negative for BC)
/// * `month` - Month (1-12)
/// * `day` - Day of month
/// * `hour` - Hour as decimal (0.0-24.0)
/// * `gregflag` - Calendar flag: 1 for Gregorian (default), 0 for Julian
///
/// # Returns
/// Julian Day number
#[wasm_bindgen(js_name = swe_julday)]
pub fn swe_julday(year: i32, month: i32, day: i32, hour: f64, gregflag: Option<i32>) -> f64 {
    let flag = gregflag.unwrap_or(constants::SE_GREG_CAL);
    julian::julday(year, month, day, hour, flag)
}

/// Convert Julian Day to calendar date
///
/// # Arguments
/// * `jd` - Julian Day number
/// * `gregflag` - Calendar flag: 1 for Gregorian (default), 0 for Julian
///
/// # Returns
/// Object with year, month, day, hour
#[wasm_bindgen(js_name = swe_revjul)]
pub fn swe_revjul(jd: f64, gregflag: Option<i32>) -> JsValue {
    let flag = gregflag.unwrap_or(constants::SE_GREG_CAL);
    let (year, month, day, hour) = julian::revjul(jd, flag);

    let result = serde_json::json!({
        "year": year,
        "month": month,
        "day": day,
        "hour": hour
    });

    serde_wasm_bindgen::to_value(&result).unwrap_or(JsValue::NULL)
}

/// Calculate planet position
///
/// # Arguments
/// * `jd_ut` - Julian Day in Universal Time
/// * `ipl` - Planet number (0=Sun, 1=Moon, 2=Mercury, ..., 11=True Node)
/// * `iflag` - Calculation flags (256 = SEFLG_SPEED for speed calculation)
///
/// # Returns
/// Position object with longitude, latitude, distance, longitudeSpeed
#[wasm_bindgen(js_name = swe_calc_ut)]
pub fn swe_calc_ut(jd_ut: f64, ipl: i32, iflag: Option<i32>) -> JsValue {
    let planet = match Planet::from_i32(ipl) {
        Some(p) => p,
        None => return JsValue::NULL,
    };

    let flags = iflag.unwrap_or(0);
    let calc_speed = (flags & constants::SEFLG_SPEED) != 0;

    match calc_ut(jd_ut, planet, calc_speed) {
        Ok(pos) => {
            let js_pos: JsPosition = pos.into();
            serde_wasm_bindgen::to_value(&js_pos).unwrap_or(JsValue::NULL)
        }
        Err(_) => JsValue::NULL,
    }
}

/// Calculate house cusps (Placidus system)
///
/// # Arguments
/// * `jd_ut` - Julian Day in Universal Time
/// * `lat` - Geographic latitude in degrees
/// * `lon` - Geographic longitude in degrees
/// * `hsys` - House system (only 'P' for Placidus supported, ignored)
///
/// # Returns
/// Object with cusps array (12 elements), ascendant, mc, armc, vertex
#[wasm_bindgen(js_name = swe_houses)]
pub fn swe_houses(jd_ut: f64, lat: f64, lon: f64, _hsys: Option<String>) -> JsValue {
    // Note: We only support Placidus, hsys parameter is ignored
    match calc_houses(jd_ut, lat, lon) {
        Ok(houses) => {
            let js_houses: JsHouses = houses.into();
            serde_wasm_bindgen::to_value(&js_houses).unwrap_or(JsValue::NULL)
        }
        Err(_) => JsValue::NULL,
    }
}

/// Get planet name
#[wasm_bindgen(js_name = swe_get_planet_name)]
pub fn swe_get_planet_name(ipl: i32) -> String {
    match ipl {
        0 => "Sun".to_string(),
        1 => "Moon".to_string(),
        2 => "Mercury".to_string(),
        3 => "Venus".to_string(),
        4 => "Mars".to_string(),
        5 => "Jupiter".to_string(),
        6 => "Saturn".to_string(),
        7 => "Uranus".to_string(),
        8 => "Neptune".to_string(),
        9 => "Pluto".to_string(),
        10 => "Mean Node".to_string(),
        11 => "True Node".to_string(),
        _ => format!("Planet {}", ipl),
    }
}

/// Planet constants for JavaScript
#[wasm_bindgen]
pub struct SE {
    _private: (),
}

#[wasm_bindgen]
#[allow(non_snake_case)]
impl SE {
    #[wasm_bindgen(getter)]
    pub fn SUN() -> i32 { 0 }

    #[wasm_bindgen(getter)]
    pub fn MOON() -> i32 { 1 }

    #[wasm_bindgen(getter)]
    pub fn MERCURY() -> i32 { 2 }

    #[wasm_bindgen(getter)]
    pub fn VENUS() -> i32 { 3 }

    #[wasm_bindgen(getter)]
    pub fn MARS() -> i32 { 4 }

    #[wasm_bindgen(getter)]
    pub fn JUPITER() -> i32 { 5 }

    #[wasm_bindgen(getter)]
    pub fn SATURN() -> i32 { 6 }

    #[wasm_bindgen(getter)]
    pub fn URANUS() -> i32 { 7 }

    #[wasm_bindgen(getter)]
    pub fn NEPTUNE() -> i32 { 8 }

    #[wasm_bindgen(getter)]
    pub fn PLUTO() -> i32 { 9 }

    #[wasm_bindgen(getter)]
    pub fn MEAN_NODE() -> i32 { 10 }

    #[wasm_bindgen(getter)]
    pub fn TRUE_NODE() -> i32 { 11 }

    #[wasm_bindgen(getter)]
    pub fn GREG_CAL() -> i32 { 1 }

    #[wasm_bindgen(getter)]
    pub fn JUL_CAL() -> i32 { 0 }

    #[wasm_bindgen(getter)]
    pub fn FLG_SPEED() -> i32 { 256 }
}

/// Batch calculation for multiple planets
///
/// # Arguments
/// * `jd_ut` - Julian Day in Universal Time
/// * `planets` - Array of planet numbers
/// * `iflag` - Calculation flags
///
/// # Returns
/// Object mapping planet numbers to position objects
#[wasm_bindgen(js_name = swe_calc_ut_batch)]
pub fn swe_calc_ut_batch(jd_ut: f64, planets: &[i32], iflag: Option<i32>) -> JsValue {
    use std::collections::HashMap;

    let flags = iflag.unwrap_or(0);
    let calc_speed = (flags & constants::SEFLG_SPEED) != 0;

    let mut results: HashMap<i32, JsPosition> = HashMap::new();

    for &ipl in planets {
        if let Some(planet) = Planet::from_i32(ipl) {
            if let Ok(pos) = calc_ut(jd_ut, planet, calc_speed) {
                results.insert(ipl, pos.into());
            }
        }
    }

    serde_wasm_bindgen::to_value(&results).unwrap_or(JsValue::NULL)
}

/// Calculate all planets at once
///
/// # Arguments
/// * `jd_ut` - Julian Day in Universal Time
/// * `iflag` - Calculation flags
///
/// # Returns
/// Object with all planet positions
#[wasm_bindgen(js_name = swe_calc_ut_all)]
pub fn swe_calc_ut_all(jd_ut: f64, iflag: Option<i32>) -> JsValue {
    let all_planets = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 11]; // All planets + True Node
    swe_calc_ut_batch(jd_ut, &all_planets, iflag)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_julday() {
        let jd = swe_julday(2000, 1, 1, 12.0, Some(1));
        assert!((jd - 2451545.0).abs() < 0.0001);
    }
}
