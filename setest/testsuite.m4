/* Automatically generated file - do not change manually */
/* Create or change source files named suite_*.c         */
/* Generate this file using "make"                       */

divert(-1)
# From now on, M4 comments start with a ¢ symbol (my five cents):
changecom(`¢',
)dnl
¢ Replace provisional header file testsuite_facade by final version
¢ The include stmt will appear multiple times in the target file
¢ (once for each suite_*.c source file), which is unavoidable
¢ But there is no problem, due to the IFNDEF TESTSUITE_FINAL_H construct
define(testsuite_facade,`testsuite_final')

¢Test for numeric (with specific message as argument)§
define(`TEST_NUMERIC',
`ifelse(regexp(`$1',`^[0-9]+$'),`-1',
`errprint(__file__:__line__:` $2
')m4exit(1)')')dnl

¢ TESTSUITE - generates a wrapping function for test cases
define(`TESTSUITE',`dnl
ifdef(`SUITE_ID_'$1`',`errprint(__file__:__line__:` Error: suite id $1 already used
')m4exit(1)')dnl
TEST_NUMERIC($1,`Error: suite id $1 is not a positive integer')
define(`SUITE_ID_'$1`')dnl
define(`SUITE_NO',`$1')dnl
define(`SUITE_DESCR',$2)dnl
define(`HAS_SETUP',`0')dnl
define(`HAS_TEARDOWN',`0')dnl
void testsuite_$1(test_context *ctx) {divert(1)
  open_testsuite(SUITE_NO,patsubst(`regexp(`$*',`\(".*"\)',`\1')',`,',`, '),ctx);
divert(2)dnl
  if (is_suite_selected(SUITE_NO,ctx)) {
    strcpy(ctx->current.file,"__file__");
    testsuite_`'SUITE_NO`'(ctx);
    close_testsuite(ctx);
  }  
divert(0)')

¢ We need macro END_TESTSUITE to explicitly finish the testsuite
define(`END_TESTSUITE',`undivert(3)undivert(1)
}')

¢ SETUP and TEARDOWN are optional
define(`SETUP',`define(`HAS_SETUP',`1')divert(3)void setup_`'SUITE_NO`'()')
define(`TEARDOWN',`define(`HAS_TEARDOWN',`1')divert(3)void teardown_`'SUITE_NO`'()')

¢ TESTCASE - a single test function
¢   - writes its own call in stream divert(1)
¢   - generates its declaration in standard stream divert(0)
define(`TESTCASE',`divert(1)
ifdef(`TESTCASE_ID_'SUITE_NO`_'$1`',`errprint(__file__:__line__:` Error: ID $1 already used in suite 'SUITE_NO `
')m4exit(1)')define(`TESTCASE_ID_'SUITE_NO`_'$1`')dnl
TEST_NUMERIC($1,` Error: testcase id $1 in suite 'SUITE_NO` is not a positive integer')
  if (is_testcase_selected( $1, ctx)) { 
    open_testcase($1,patsubst(`regexp(`$*',`\(".*"\)',`\1')',`,',`, '),ctx);   
    ctx->current.iteration.id = 0;
    do {
      ctx->current.iteration.id++;
      open_iteration( ctx->current.iteration.id, ctx );
      if (is_iteration_selected(ctx)) {
        ifelse(HAS_SETUP,`1',`setup_`'SUITE_NO`'();')
        testcase_`'SUITE_NO`'_$1(ctx);
        ifelse(HAS_TEARDOWN,`1',`teardown_`'SUITE_NO`'();')
        close_iteration(ctx);
      }
    } while (has_more_iterations(ctx)); 
    close_testcase(ctx);
  }
divert(0)void testcase_`'SUITE_NO`'_$1(test_context *ctx)')

¢ CHECK_xx macro family
¢   - macro is necessary, since the symbol is needed twice: As a string "$1", 
¢     AND as its data object $1
define(`CHECK_S',`check_s($1,"regexp($1,`\(.*[^ ]\)',`\1')",ctx)')
define(`CHECK_I',`check_i($1,"regexp($1,`\(.*[^ ]\)',`\1')",ctx)')
define(`CHECK_D',`check_d($1,"regexp($1,`\(.*[^ ]\)',`\1')",ctx)')
define(`CHECK_DD',`check_dd($1,$2,"regexp($1,`\(.*[^ ]\)',`\1')",ctx)')
define(`CHECK_II',`check_ii($1,$2,"regexp($1,`\(.*[^ ]\)',`\1')",ctx)')
define(`CHECK_EQUALS_S',`check_equals_s($1,$2,"regexp($1,`\(.*[^ ]\)',`\1')",ctx)')
define(`CHECK_EQUALS_I',`check_equals_i($1,$2,"regexp($1,`\(.*[^ ]\)',`\1')",ctx)')
define(`CHECK_EQUALS_D',`check_equals_d($1,$2,"regexp($1,`\(.*[^ ]\)',`\1')",ctx)')
define(`CHECK_EQUALS_II',`check_equals_ii($1,$2,regexp(`$*',`[^\,]*\,[^\,]*\,\(.*\)',`\1'),"regexp($1,`\(.*[^ ]\)',`\1')",ctx)')
define(`CHECK_EQUALS_DD',`check_equals_dd($1,$2,regexp(`$*',`[^\,]*\,[^\,]*\,\(.*\)',`\1'),"regexp($1,`\(.*[^ ]\)',`\1')",ctx)')
define(`GET_I',`get_i("regexp($1,`\(.*[^ ]\)',`\1')",ctx)')
define(`GET_D',`get_d("regexp($1,`\(.*[^ ]\)',`\1')",ctx)')
define(`GET_P',`get_p("regexp($1,`\(.*[^ ]\)',`\1')",ctx)')
define(`GET_S',`strcpy($1,get_s("regexp($1,`\(.*[^ ]\)',`\1')",ctx))')

divert(0)dnl
