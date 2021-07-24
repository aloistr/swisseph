`#' Template for this fixture file was generated at: syscmd(`date')divert(-1)dnl
define(`TESTSUITE',`divert(0)
  define(`PARINDENT',`    ')`TESTSUITE'
    section-id:$1
    section-descr: patsubst(`regexp(`$*',`"\(.*\)"',`\1')',`,',`, ')`'divert(-1)')dnl
define(`TESTCASE',`divert(0)
    define(`PARINDENT',`          ')`TESTCASE'
      section-id:$1
      section-descr: patsubst(`regexp(`$*',`"\(.*\)"',`\1')',`,',`, ')
        ITERATION`'divert(-1)')dnl
define(`END_TESTSUITE',`divert(0)
divert(-1)')dnl
define(`GET_I',`divert(0)
PARINDENT`'translit($1,`"',`'):divert(-1)')dnl
define(`GET_D',`divert(0)
PARINDENT`'translit($1,`"',`'):divert(-1)')dnl
define(`GET_S',`divert(0)
PARINDENT`'$1:divert(-1)')dnl
