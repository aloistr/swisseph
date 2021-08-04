# Swiss Ephemeris test cases

Developed in cooperation with Rüdiger Plantiko, by Dieter Koch.
Maintained by Alois Treindl since July 2021.

The test procedure with git is:

- master branch is the released SE branch

- branch 'devel' is a branch with correctes or additions to the source code

- checkout master branch, command: make text.exp

  It builds setest/setest, statically linked with SE

  It  creates expectations values setest/t.exp

- checkout branch devel, command: make test


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


