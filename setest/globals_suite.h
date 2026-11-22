#ifndef GLOBALS_SUITE_H
#define GLOBALS_SUITE_H

// Utility functions for test case code, defined in globals_suite.c.
// Since 2026-11-22 every suite is its own C file, so the suites need
// these declarations (before, globals_suite.c was part of the one
// generated file with all suites).

void check_swecalc_results(int rc,double *xx, char* serr, test_context* ctx);
void check_swehouses_results(int rc, double jd_ut, int ihsy, double *cusps, double *ascmc, char* serr, test_context* ctx);
void check_swehouses_ex2_results(int rc, double jd_ut, int ihsy, double *cusps, double *ascmc, double *cusp_speed, double *ascmc_speed, char* serr, test_context* ctx);
void check_swehouses_armc_results(int rc, double armc, int ihsy, double *cusps, double *ascmc, char* serr, test_context* ctx);
void check_swehouses_armc_ex2_results(int rc, double armc, int ihsy, double *cusps, double *ascmc, double *cusp_speed, double *ascmc_speed, char* serr, test_context* ctx);

#endif
