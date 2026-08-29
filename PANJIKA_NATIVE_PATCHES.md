# Panjika Native Swiss Ephemeris Patch Set

Base: official Swiss Ephemeris `v2.10.3final`, commit
`af9823fe7b06ffefe3d3968fdc5680be8b5eec5f`.

## `dt1`: nonuniform Delta-T table interpolation

The upstream Delta-T table is sampled at 00:00 on 1 January, but the upstream
Bessel interpolation maps Julian days to uniform 365.25-day years. It therefore
misses some of its own Jan-1 samples.

This patch uses a standard shape-preserving piecewise cubic Hermite
interpolator on the actual Gregorian Jan-1 Julian-day nodes. The slopes are the
weighted harmonic-mean construction of Fritsch and Butland, including the
shape-preserving endpoint conditions. It:

- exactly reproduces every tabulated epoch;
- handles 365-day and 366-day intervals directly;
- avoids overshoot in the coarse historical table;
- keeps the official table values, tidal-acceleration adjustment, and future
  model unchanged.

## `dt2`: authoritative observed and predicted Earth rotation

The optional `swe_deltat_ext.txt` file is loaded through the normal Swiss
ephemeris search path. Its data are generated, without event hardcoding, from:

- IERS EOP 20 C04 daily `UT1-UTC`, LOD, and their uncertainties;
- official USNO `tai-utc.dat`, including the pre-1972 offset-rate segments;
- the identity `Delta-T = TAI-UTC + 32.184 - (UT1-UTC)`; and
- USNO `deltat.preds` only after the final observed C04 epoch.

Observed daily intervals use cubic Hermite interpolation with published LOD as
the endpoint derivative. Prediction intervals use nonuniform shape-preserving
PCHIP. The final prediction is joined to the existing Swiss
Stephenson-Morrison-Hohenkerk 2016 future model with Swiss's 100-year
value-matching transition. Before C04, the official Swiss historical model and
annual table remain authoritative. A rounded annual/C04 boundary is joined by
one Hermite interval to avoid a discontinuity.

The parser rejects partial, non-finite, non-monotonic, out-of-order, oversized,
or schema-incompatible snapshots. A corrupt or missing optional file falls back
to the official Swiss model; the Panjika managed strict-precision layer requires
the file so production cannot fall back silently.

The version string is suffixed `-panjika-dt2` so runtime audits can distinguish
the patched binary from an unmodified `2.10.03` DLL.

Native verification over 23,615 observed/predicted nodes found a maximum node
error of `7.1054273576e-15` seconds. Against 639 separately published USNO monthly rows,
the C04-derived route reduced MAE from `0.022426585 s` (stock Swiss) to
`0.000144690 s`; published observation/prediction uncertainty remains the
physical accuracy limit.

No changes were made to `CROSS_PRECISION`, Earth ellipsoid constants, or the
observer vector. Those changes require either a representability-safe solver
redesign or external datum/EOP inputs and are not valid global PAC corrections.
