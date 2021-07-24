// These are provisional definitions, just to make the C source code look complete for the IDE
// In the end, M4 macros with these names will do the work
#ifndef TESTSUITE_FACADE_H
#define TESTSUITE_FACADE_H
#include "setest.h"

#define TESTSUITE(suiteno,suitedescription) void testsuite_ ## suiteno(test_context* ctx) {
#define END_TESTSUITE }
#define TESTCASE(testno,description) void testcase_ ## testno(test_context* ctx)
#define SETUP void setup(test_context *ctx)
#define TEARDOWN void teardown(test_context *ctx)

// Will never print anything (only for compiler)
#define CHECK_DD(symbol,length) printf( "%s: %p\n", #symbol, (void *)symbol);
#define CHECK_D(symbol) printf( "%s: %lf\n", #symbol, symbol);
#define CHECK_I(symbol) printf( "%s: %i", #symbol, symbol);
#define CHECK_II(symbol,length) printf( "%s: %p", #symbol, (void *)symbol);
#define CHECK_S(symbol) printf( "%s: %s", #symbol, symbol);

#define CHECK_EQUALS_I(symbol,exp_in) printf( "%s: %i  : %d", #symbol, symbol, exp_in);
#define CHECK_EQUALS_II(symbol,length,...) printf( "%s: %p  : %s", #symbol, symbol, "exp_in");
#define CHECK_EQUALS_D(symbol,exp_in) printf( "%s: %lf :%lf", #symbol, symbol, (double)exp_in);
#define CHECK_EQUALS_DD(symbol,length,...) printf( "%s: %p  : %s", #symbol, symbol, "exp_in");
#define CHECK_EQUALS_S(symbol,exp_in) printf( "%s: %s  : %s", #symbol, symbol, exp_in);

// Data getter
#define GET_I(symbol) get_i("symbol", ctx )
#define GET_D(symbol) get_d("symbol", ctx )
#define GET_P(symbol) get_p("symbol", ctx )
#define GET_S(var) strcpy(var,get_s("var", ctx ))



#include "globals_suite.c"

#endif


