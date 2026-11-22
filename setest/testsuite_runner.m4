/* Automatically generated file - do not change manually         */
/* run_tests() calls the function testsuite_N() of every suite   */
/* Generate this file using "make"                               */
#include "testsuite_final.h"
divert(-1)
# From now on, M4 comments start with a ¢ symbol (as in testsuite.m4):
changecom(`¢',
)dnl
¢ testsuite_runner.m4 reads all suite_*.c files, followed by testsuite_end.m4,
¢ and makes run_tests() (since 2026-11-22; before, testsuite.m4 made it
¢ together with all suites in one file). The code of the suites is made by
¢ testsuite.m4, one file per suite. Here the suite text is discarded
¢ (divert -1); TESTSUITE writes the declaration of testsuite_N() to
¢ divert(3) and its call to divert(2), in the order of the suite files.

¢Test for numeric (with specific message as argument)
define(`TEST_NUMERIC',
`ifelse(regexp(`$1',`^[0-9]+$'),`-1',
`errprint(__file__:__line__:` $2
')m4exit(1)')')dnl

define(`TESTSUITE',`dnl
ifdef(`SUITE_ID_'$1`',`errprint(__file__:__line__:` Error: suite id $1 already used
')m4exit(1)')dnl
TEST_NUMERIC($1,`Error: suite id $1 is not a positive integer')dnl
define(`SUITE_ID_'$1`')dnl
divert(3)void testsuite_$1(test_context *ctx);
divert(2)  if (is_suite_selected($1,ctx)) {
    strcpy(ctx->current.file,"__file__");
    testsuite_$1(ctx);
    close_testsuite(ctx);
  }
divert(-1)')
