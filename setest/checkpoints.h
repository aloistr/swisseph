#ifndef CHECKPOINTS_H
#define CHECKPOINTS_H

extern double DEFAULT_PRECISION;

typedef struct {
  void *act;
  int length;
  char *name;
  void *exp;
  test_context *ctx;
} check_data;

void check_d(const double field,const char* name, test_context *ctx);
void check_dd(const double *field, const int length, const char *name, test_context *ctx);
void check_i(const int field,const char* name, test_context *ctx);
void check_ii(const int *field, const int length, const char* name, test_context *ctx);
void check_s(const char* field,const char* name, test_context *ctx);

void check_equals_i(const int field,const int exp, const char* name, test_context *ctx);
void check_equals_s(const char* field,const char* exp, const char* name, test_context *ctx);
void check_equals_d(const double act, const double exp, const char* name, test_context *ctx);
void check_equals_ii(const int *act, const int length, const int *exp,const char *name, test_context *ctx);
void check_equals_dd(const double *act, const int length, const double *exp,const char *name, test_context *ctx);

void check_d_internal(const double field,const char* name,double precision,test_context *ctx);
void check_i_internal(const int field,const char* name,test_context *ctx);
void check_equals_d_internal(const double act, const double exp, const char* name, const double precision, test_context *ctx);
void check_equals_i_internal(const int act, const int exp, const char* name, test_context *ctx);
void check_equals_s_internal(const char* act, const char* exp, const char* name, test_context *ctx);


#endif
