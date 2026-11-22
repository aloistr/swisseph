# Swiss Ephemeris test cases

Developed in cooperation with Rüdiger Plantiko, by Dieter Koch.
Maintained by Alois Treindl since July 2021.

The test procedure with git is:

- master branch is the released SE branch

- branch 'devel' is a branch with correctes or additions to the source code

- checkout master branch, command: make test.exp

  It builds setest/setest, statically linked with SE

  It  creates expectations values setest/t.exp

- checkout branch devel, command: make test


# How to make a new t.exp

t.exp holds the expected values. setest writes it itself (`setest -g t`)
from the fixture file t.fix, with the library setest is linked with and
the data files found in the ephemeris path. Generate it again whenever
the library or the data files change (e.g. new *.se1 files); otherwise
`setest t` reports failures.

From the top directory of the repository:

    make libswe.a              # static library from the current sources
    (cd setest && make clean)  # make does not relink setest after a library change
    make test.exp              # builds setest, runs ./setest -g t, writes setest/t.exp
    make test                  # runs ./setest t, should report 0 failures

Notes:

- Keep a copy of the old t.exp if you want to compare.
- Inside setest/, `make t.exp` does nothing if t.exp exists (the rule
  has no prerequisites); use `./setest -g t` instead.
- Inside setest/, `make test` runs the self-tests of setest (t/test.sh),
  not the Swiss Ephemeris tests.
- If constants in swephexp.h have changed, run generate_prepare_fix_m4.pl
  first (see the history of 25-jan-21).


# Tolerances

Since 2026-11-22 every compared floating-point value has a tolerance
according to its kind of quantity (tolerances.c), in the units of the
value:

    angles (positions, cusps, ayanamsa, azimuth, altitude)  3e-8 deg = 0.1 mas
                                       (radians with SEFLG_RADIANS)
    position at a found crossing time (suite 10)     1 mas
    distances                                        1e-10 AU = 15 m
    diameter of the core shadow                      0.015 km
    speeds (per day)                                 1e-9
    times                                            1e-8 day = about 1 ms
    magnitudes, fractions, eccentricity, saros       1e-7
    house and sector positions                       1e-9

For large values the tolerance is at least 1e-14 of the expected value.
Integers and strings must be equal.

`precision` and `precision-xx` in t.fix are used only for values that
tolerances.c does not know; they now apply only to their own test case
(before, they carried over to all later test cases). With
`setest -p "precision:X" t` one tolerance X is used for all values.


# Environment in the header of t.exp

Since 2026-11-22 `setest -g` writes the data files the library uses into
the header of the .exp file (environment.c): for planets, Moon, main
asteroids and Eros the file used for J2000 with its DE number and SHA1,
for the JPL file name, size and date, and whether the EOP files for the
JPL Horizons mode exist (env-eop-files). Comment lines give the full paths.
In test mode setest computes the same and compares it, and the library
version, with the header; every difference gives a warning on stderr,
and the tests continue. Files of other centuries are not checked.


# History
---------------
## 25-jan-21 Dieter

Test cases added for:
  - Jupiter center of body, geocentric, apparent positions
  - Io geocentric, apparent positions

For new function swe_calc_pctr() (planetocentric positions):
  - Mars Jupiter-centric, apparent
  - Io Jupiter-centric, apparent

For new functions with cusp speeds:
  - swe_houses_ex2()
  - swe_houses_armc_ex2()

For the new function swe_calc_pctr() I had to add 
TESTCASE(5,"swe_calc_pctr( ) - planetocentic position") ... 
in "suite_01_calc.c"

For the new functions swe_calc_houses_ex2() and swe_calc_houses_armc_ex2(),
TESTCASE(8,... and TEXTCASE(9,... in suite_06_houses.c.
In addition, globals_suite.c required the following new functions:
check_swehouses_ex2_results(), check_swehouses_armc_ex2_results().

For the new SEFLG_CENTER_BODY, I had to add "SEFLG_CENTER_BODY"
in the list of seflgs in constants.c.
In addition, I had to run generate_prepare_fix_m4.pl.

Then, in t.fix, for the additions in suite_01_calc.c, I searched 
TESTSUITE with section-id:1 and added TESTCASE with section-id:5 
for the planetocentric calculations.

Analogously, for the additions in suite_06_houses.c, I searched
for TESTSUITE with section-id:6 and added TESTCASES with section-id
8 and 9.

## 3-aug-21 Alois

added suite_10_solcross.c for the new swe_solcross function group


