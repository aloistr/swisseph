=head1 Swiss Ephemeris test cases

This needs to be rewritten so that it works with git branches.

The future procedure should be:

- master branch is the released SE branch

- branch 'devel' is a branch with correctes or additions to the source code

- checkout master branch, build and run setest 
  it should be statically linked with SE

- checkout branch devel, build and run setestx
  it should be statically linked with SE

=head2 update 4-dec-20	translated to English

In collaboration with Rüdiger Plantiko, we created a test suite for Swiss Ephemeris

----------- German original written by Dieter Koch
----------- is translated below into English by Google
----------- Memo will be continued in English only
----------- German version can be removed once English is verified

------------
=head 2 19-may-16

After a few extensions everything seems stable.

Our setup is as follows
/uds ->/home/dieter/sweph
         SE development work directory

/udss ->/home/dieter/sweph/setest
         Directory for SE testing


in /udss
make setest	build setest, uses libswe.so from/usr/local/lib64/swe
                 i.e. the 'released version' of SE

make setestx	build setestx, uses libswex.so from/usr/local/lib64/swe

libswex is made in /uds with: make libswex
t.fix fixture file with the test data for the test suites suite_*.c
        For each test case in the test suite, a fixture
        exist with the variables for this test.

        The first 5 test suites are from Rüdiger Plantiko, suite_06
        von Alois with the house tests.

        This exists in a lean version and a fat version.
        The bold version has been renamed to Testsuite 66 within t.fix
        and is therefore not addressed. If you want to use them, swap them
        one in t.fix the numbers of the two suites 6 and 66.

If you have made changes to the code suite*c, you have to do the following again:
make setest setestx

Then you have to generate new expected values ​​with
setest -g t

Even if you have made changes to t.fix, you need new expected values
to generate.
make t.exp

To run the test:
setestx t
or
setestx t> t.out

In t.fix you can switch off entire suites or individual cases with
disabled: 1 or 2 (1 = with warning, 2 = without warning).

If changes are made to the suites, send them to the repository:
git commit suite_06*c -m 'comment'
git push

-------
Test the new version of swisseph:

1. Update the SE_VERSION variable in sweph.h
    also the variable SWEVERSION in makefile
2. make libswex
3. cd setest
4. if necessary regenerate the exp file (expected results):
    setest -g t
5. setestx t
    or
    setestx t> t.out

------------------------------
=head 2 4-dec-20 Alois

translated to english
git commit and push to remote origin (on Plantiko private host)

test cases for new features of release 2.09 and 2.10 need to be written

---------------
=head 2 25-jan-21 Dieter

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

Then I recompiled setest and setestx. 

