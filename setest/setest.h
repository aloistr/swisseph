#ifndef SETEST_H
#define SETEST_H

#include <stdio.h>
#include <unistd.h>
#include "swephexp.h"
#include "stdbool.h"

#define SETEST_MAX_FAILURE_ROWS 100
#define SETEST_MAX_SYMBOL_SIZE 50
#define SETEST_MAX_COMMAND_LENGTH 100

#include "globals.h"
#include "reader.h"
#include "testdata.h"
#include "multivalues.h"


typedef struct {
  int testsuite;
  int testcase;
  int iteration;
  } test_key;

typedef struct {
   int id;
   char* descr;
} named_id;

// Failure data
typedef enum {
  WRONG_VALUE,
  EXP_VALUE_MISSING
  } failure_class;

// Failure data - single row
typedef struct {
  char name[SETEST_MAX_SYMBOL_SIZE];
  typed_value exp, act;
  failure_class class;
  } failure;

// Failure table
typedef struct {
  failure entry[SETEST_MAX_FAILURE_ROWS];
  int length;
  } failures;

// Full declaration of test_context
typedef struct {

  bool testmode;

// Current test suite, testcase
  struct {
    struct {
      named_id;
      bool disabled;
      } suite;
    struct {
      named_id;
      bool disabled;
      } testcase;
    struct {
      int id;
      failures failures;
      } iteration;
    t_section section;
    char file[SETEST_MAX_SYMBOL_SIZE];
  } current;

// For restriction to a single test case or test suite:
    test_key selected;

// Name of the test collection. 
// 'test' will be expanded to 'test.fix'/'test.exp'    
    char test_collection[SETEST_MAX_SYMBOL_SIZE];

// In gen mode: pointer to output file
    FILE *out;

// Scanner for the .fix or .exp file
    rdr_ctx reader;

// Scanner for a prioritary parameter file (passed via '-p <filename>')
    rdr_ctx parameters;

// A table tracking multiple values
    mvalues_tab* multivalues;

// Precisions for doubles
    struct st_precisions {
        double all;
        double xx[6];
    } precisions;

// Preprocess command
    char prepare_fixture_command[SETEST_MAX_COMMAND_LENGTH];

// Statistics
    struct {
      test_key;
      int failures;
      int failed_iterations;
      } count;

    bool verbose;

  } test_context;

typedef void (*testFunction)(test_context*);

typedef struct { unsigned char key[20]; } SHA1HASH;

void run_tests(test_context* ctx);

void initialize_test_context( test_context *ctx );
void evaluate_cmdline_options(int argc, char** argv, test_context *ctx);
void run( test_context* ctx ) ;
void cleanup(test_context* ctx);
void print_help();

void print_header_data();
void print_version();
void print_library_info();
void print_date_time_user( );
void sha1_from_file(const char *name, SHA1HASH* hash);

void open_testsuite(int id, char* description, test_context *ctx);
void close_testsuite( test_context *ctx );
void open_testcase(int id, char* description, test_context *ctx);
void close_testcase( test_context *ctx );
void open_iteration(int id, test_context *ctx );
void close_iteration( test_context *ctx );
bool has_more_iterations(test_context* ctx);
bool is_iteration_selected( test_context *ctx );
bool is_testcase_selected( int id, test_context *ctx );
bool is_test_enabled(test_context* ctx);
bool is_suite_selected( int id, test_context *ctx );
bool read_next_section( rdr_ctx *reader, int id, t_section );

void prepare_precisions( test_context *ctx );
rdr_ctx get_param_reader(char* arg);
int get_disabled(const t_section section,test_context *ctx);



#include "checkpoints.h"

entry get_entry( const char *name, const st_type type, const test_context *ctx);

double get_d( const char* name, const test_context *ctx);
int get_i( const char* name, const test_context *ctx);
const char* get_s( const char* name, const test_context *ctx);

void clear_failures( failures *failures);
void handle_failures( test_context *ctx);


#endif
