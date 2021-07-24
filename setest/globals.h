#ifndef GLOBALS_H
#define GLOBALS_H

#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

// Globals to be contained in every C source

extern const int ALL;
extern const char* PREPARE_FIXTURE_COMMAND;
extern const char* DEFAULT_TEST_COLLECTION;
extern const char* SEFLG[];

// Always keep enum in sync with indices of constant SECTION_KEYWORDS
extern const char* SECTION_KEYWORDS[];
extern const char* PARAMETER_SECTIONS[];
typedef enum {
  UNDEFINED    = -2,
  END_OF_FILE  = -1,
  GENERAL      = 0,
  TESTSUITE    = 1,
  TESTCASE     = 2,
  ITERATION    = 3
  } t_section;

int equals( const char* act, const char* exp);
bool trimmed_equals( const char* string, const char *sub);
bool equals_with_escaped_newlines(const char* exp, const char* act);
bool ends_with(const char *string, const char * sub);
char* trim_trailing_space(char *s);
char* trim_space(char* s);
bool is_empty(const char *s);
char* copy_string(const char* s);
bool is_blank(const char *s);
 
#endif
