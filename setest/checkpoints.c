#include "setest.h"

static pair make_pair(const char* name, const typed_value *v);
static void push_failure( failure *f, failures *failures);

void check_d(const double field,const char* name, test_context *ctx) {
  check_d_internal(field,name,ctx->precisions.all,ctx);
}

void check_dd(const double *field, const int length, const char *name, test_context *ctx) {
  char member_name[SETEST_MAX_SYMBOL_SIZE+6];
  for (int i=0; i<length; i++) {
    sprintf(member_name,"%s[%d]",name,i);
    if (strncmp(name,"xx", 2) == 0 && i < 6) {
      check_d_internal(field[i],member_name,ctx->precisions.xx[i],ctx);
      }
    else {
      check_d_internal(field[i],member_name,ctx->precisions.all,ctx);
      }
    }
}

void check_d_internal(const double field,const char* name, double precision, test_context *ctx) {
  if (!ctx->testmode) {
    typed_value act = { .value.d = field, .type = D };
    pair p = make_pair(name, & act );
    print_if( true, & p, ctx->out);
  }
  else {
    entry exp = get_entry( name, D, ctx );
    if ( exp.not_found ) {
      failure f = { .class = EXP_VALUE_MISSING };
      strcpy(f.name,name);
      push_failure( &f, &ctx->current.iteration.failures );
      }
    else {
      check_equals_d_internal(field,exp.pair.tvalue.value.d,name,precision,ctx );
    }
  }
}

void check_i(const int field,const char* name, test_context *ctx) {
  if (! ctx->testmode ) {
    typed_value act = { .value.i = field, .type = I };
    pair p = make_pair(name, &act);
    print_if( true, & p, ctx->out );
  }
  else {
    entry exp = get_entry( name, I, ctx );
    if ( exp.not_found ) {
      failure f = { .class = EXP_VALUE_MISSING };
      strcpy(f.name,name);
      push_failure( &f, &ctx->current.iteration.failures );
      }
    else {
      check_equals_i_internal(field,exp.pair.tvalue.value.i,name,ctx);
    }
  }
}

void check_i_internal(const int field,const char* name, test_context *ctx) {
  if (!ctx->testmode) {
    typed_value act = { .value.i = field, .type = I };
    pair p = make_pair(name, & act );
    print_if( true, & p, ctx->out);
  }
  else {
    entry exp = get_entry( name, I, ctx );
    if ( exp.not_found ) {
      failure f = { .class = EXP_VALUE_MISSING };
      strcpy(f.name,name);
      push_failure( &f, &ctx->current.iteration.failures );
      }
    else {
      check_equals_i_internal(field,exp.pair.tvalue.value.i,name,ctx);
    }
  }
}

void check_equals_ii(const int *act, const int length, const int *exp,const char *name, test_context *ctx) {
  if (ctx->testmode) {
    char member_name[SETEST_MAX_SYMBOL_SIZE+6];
    for (int i=0; i<length; i++) {
      sprintf(member_name,"%s[%d]",name,i);
      check_equals_i_internal(act[i],exp[i],member_name,ctx);
    }
  }
}


void check_equals_i(const int act, const int exp, const char* name, test_context *ctx) {
  if (ctx->verbose) {
    printf("check-equals for %s: %d == %d\n",
      name,
      act,
      exp
    );
  }
  if (ctx->testmode) {
    check_equals_i_internal(act,exp,name,ctx);
  }
}

void check_equals_i_internal(const int act, const int exp, const char* name, test_context *ctx) {
  if (act != exp) {
    failure f = {
      .class = WRONG_VALUE,
      .exp   =  { .value.i = exp, .type = I},
      .act   =  { .value.i = act, .type = I}
      };
    strcpy(f.name,name);
    push_failure(&f, &ctx->current.iteration.failures );
  }
}

void check_equals_d(const double act, const double exp, const char* name, test_context *ctx) {
  if (ctx->verbose) {
    printf("check-equals for %s: %lf == %lf\n",
      name,
      act,
      exp
    );
  }
  if (ctx->testmode) {
    check_equals_d_internal(act,exp,name,ctx->precisions.all,ctx);
  }
}

void check_equals_d_internal(const double act, const double exp, const char* name, const double precision, test_context *ctx) {
  if (isnan(act) || isnan(exp) || fabs(exp - act) > precision ) {
    failure f = {
      .class = WRONG_VALUE,
      .exp   = { .value.d = exp, .type = D },
      .act   = { .value.d = act, .type = D }
      };
    strcpy(f.name,name);
    push_failure(&f, &ctx->current.iteration.failures );
  }
}

void check_equals_dd(const double *act, const int length, const double *exp,const char *name, test_context *ctx) {
  if (ctx->testmode) {
    char member_name[SETEST_MAX_SYMBOL_SIZE+6];
    for (int i=0; i<length; i++) {
      sprintf(member_name,"%s[%d]",name,i);
      if (equals(name,"xx")) {
        check_equals_d_internal(act[i],exp[i],member_name,ctx->precisions.xx[i],ctx);
      }
      else {
        check_equals_d_internal(act[i],exp[i],member_name,ctx->precisions.all,ctx);
      }
    }
  }
}


void check_equals_s(const char* act, const char* exp, const char* name, test_context *ctx) {

  if (ctx->verbose) {
    printf("check-equals for %s: '%s' == '%s'\n",
      name,
      act,
      exp
    );
  }

  if (ctx->testmode) {
    check_equals_s_internal(act,exp,name,ctx);
  }

}

void check_equals_s_internal(const char* act, const char* exp, const char* name, test_context *ctx) {
  if (!equals_with_escaped_newlines(exp,act)) {
    failure f = {
      .class = WRONG_VALUE,
      .exp   = { .value.s = copy_string(exp),
                 .on_heap = true,
                 .type = S },
      .act   = { .value.s = copy_string(act),
                 .on_heap = true,
                 .type = S }
      };
    strcpy(f.name,name);
    push_failure(&f, &ctx->current.iteration.failures );
  }
}

void check_ii(const int *field, const int length, const char* name, test_context *ctx) {
  char member_name[SETEST_MAX_SYMBOL_SIZE+6];
  for (int i=0; i<length; i++) {
    sprintf(member_name,"%s[%d]",name,i);
    check_i_internal(field[i],member_name,ctx);
  }
}

void check_s(const char* field,const char* name, test_context *ctx) {
  if (! ctx->testmode ) {
    fprintf(ctx->out,"      %s: ",name);
    for (const char *p = field; *p!='\0';p++) {
      if (*p == '\n') fprintf(ctx->out,"\\n");
      else fprintf(ctx->out,"%c",*p);
    }
    fprintf(ctx->out,"\n");
  }
  else {
    const typed_value act = { .value.s = copy_string(field),
                              .on_heap = true,
                              .type = S };
    entry exp = get_entry( name, S, ctx );
    if ( exp.not_found ) {
      failure f = { .class = EXP_VALUE_MISSING };
      strcpy(f.name,name);
      push_failure( &f, &ctx->current.iteration.failures );
      }
    else {
      if (!equals_with_escaped_newlines(exp.pair.tvalue.value.s,field)) {
        failure f = {
          .class = WRONG_VALUE,
          .exp   = exp.pair.tvalue,
          .act   = act
          };
        strcpy(f.name,name);
        push_failure(&f, &ctx->current.iteration.failures );
      }
    }
  }
}

static pair make_pair(const char* name, const typed_value *v) {
  pair p = {
    .tvalue = *v,
    };
  strcpy(p.name,name);
  trim_trailing_space(p.name);
  return p;
}

static void push_failure( failure *f, failures *failures) {

  if (failures->length == SETEST_MAX_FAILURE_ROWS) {
    fprintf(stderr,"Maximum number %d of failures per iteration exceeded\n",
             SETEST_MAX_FAILURE_ROWS);
    exit(EXIT_FAILURE);
  }

 failures->entry[failures->length] = *f;
 failures->length++;

}

void clear_failures( failures *failures) {
  for (int i=failures->length-1; i >= 0; i--) {
    failure *f = & failures->entry[i];
    st_free(&f->exp);
    st_free(&f->act);
    *f = (failure) {};
  }
  failures->length = 0;
}

void handle_failures( test_context *ctx) {
  failures *ff = & ctx->current.iteration.failures;

  if (ff->length > 0) {
    printf("# setest -s %d.%d.%d %s / tdbg %s %d %d %d  [%s]\n",
      ctx->current.suite.id,
      ctx->current.testcase.id,
      ctx->current.iteration.id,
      ctx->test_collection,
      ctx->test_collection,
      ctx->current.suite.id,
      ctx->current.testcase.id,
      ctx->current.iteration.id,
      ctx->current.file);
  }     

  ctx->count.failures += ff->length;
  if (ff->length > 0) ctx->count.failed_iterations++;

  for (int i=0;i<ff->length;i++) {
    failure *f = &ff->entry[i];
    switch (f->class) {
      case WRONG_VALUE:
        printf("#   %s unexpected value\n#     expected: ",
          f->name);
        print_value(&f->exp,stdout);
        printf("\n#       actual: ");
        print_value(&f->act,stdout);
        printf("\n");
        if (f->exp.type == D && !isnan(f->exp.value.d) && !isnan(f->act.value.d)) {
          double diff = f->exp.value.d - f->act.value.d;
          printf("#         diff: %.2e\n", diff);
        }
        break;
      case EXP_VALUE_MISSING:
        printf("# Expected value '%s' not found in test data\n",
          f->name);
        break;
      default:
        fprintf(stderr,"Program error: unkown exception\n");
        exit(EXIT_FAILURE);
      }    
  }

}
