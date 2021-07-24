#ifndef TESTDATA_H
#define TESTDATA_H

#include "globals.h"

#define SETEST_MAX_ROWS        1000

typedef enum {
  D,  // double
  I,  // integer
  C,  // single character
  P,  // pointer
  S,  // string (char*)
  FN  // function (not used)
} st_type;

typedef struct {
  struct st_pair {
    char name[SETEST_MAX_SYMBOL_SIZE];
    struct st_typed_value {
      union _st_value {
        double d;
        int i;
        char c;
        void *p;
        const char *s;
      } value;
      st_type type;
      bool on_heap; 
    } tvalue;
  } pair;
  bool not_found;
} entry;


typedef struct st_pair pair;
typedef struct st_typed_value typed_value;
typedef union _st_value value;

typedef struct {
  pair data[SETEST_MAX_ROWS];
  int p_suite;
  int p_testcase;
  int p_iteration;
  int p_current;
} st_data;


void print_if( const bool condition, const pair *rec, FILE *out);
void print_value( const typed_value *value, FILE *out );
void st_free( const typed_value *value);
void print_comment(const pair *rec,FILE *out);



#endif
