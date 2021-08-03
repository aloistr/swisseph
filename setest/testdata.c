#include "setest.h"
entry get_required_entry( const char *name, const st_type type, const test_context *ctx);
#include "multivalues.h"


entry get_entry( 
           const char *name, 
           const st_type type, 
           const test_context *ctx) {

  int mv = -1, n;
  char *value = NULL;

  entry result = { .not_found = false };
  strcpy(result.pair.name,name);

// Is it an already known multivalue entry?
  mv = multivalues_get_index(name, ctx->multivalues);
  if (mv >= 0) {

    result.pair.tvalue = *(multivalues_get_next(mv,ctx->multivalues));

  } else {

  // Get entry from parameter file if given
    if (ctx->parameters.source) {
      value = read_value( &ctx->parameters, name, GENERAL, GENERAL);
    }

  // Get entry from reader if found
    if (!value) {
      value = read_value( &ctx->reader, name, ctx->current.section, GENERAL);
    }

  // Not found - leave function with not_found = true
    if (!value) {
      if (ctx->verbose) {
        printf("Couldn't read value for '%s'\n",name);
      }
      result.not_found = true;
      return result;
    }

  // Is it a (new) multivalue field?
    if (is_multivalue(type,value)) {

      mv = multivalues_add_index(name,type,value,ctx->multivalues);
      result.pair.tvalue = *(multivalues_get_next(mv,ctx->multivalues));

    } else {

      switch (type) {
        case D:
          result.pair.tvalue.type = D;
          n = sscanf(value,"%lf", &result.pair.tvalue.value.d);
          if (n == 0) {
            fprintf(stderr,"Couldn't scan double from '%s' in get_entry()\n",value);
            exit(EXIT_FAILURE);
          }
          break;
        case I:
          result.pair.tvalue.type = I;
          n = sscanf(value,"%d", &result.pair.tvalue.value.i);
	  if (n == 0 && *value == '\'') {
	    n = sscanf(value + 1,"%c", &result.pair.tvalue.value.c);
	  }
          if (n == 0) {
            fprintf(stderr,"Couldn't scan integer from '%s' in get_entry()\n",value);
            exit(EXIT_FAILURE);
          }
          break;
        case S:
          result.pair.tvalue.type = S;
          result.pair.tvalue.value.s = value; // copy_string(value);
          break;
        default:
          fprintf(stderr,"Illegal type %d in get_entry()\n",type);
          exit(EXIT_FAILURE);
      }
    }
  }

  if (ctx->verbose) {
    printf("        read %s -> ",name);
    print_value( &result.pair.tvalue, stdout );
    printf(" %s\n", (mv < 0) ? "" : "(multivalue)" );
  }

  return result;

}

// Like get_entry, but with error on 'not found'
entry get_required_entry( 
           const char *name, 
           const st_type type, 
           const test_context *ctx) {

  entry result = get_entry(name, type, ctx);
  if (result.not_found) {
    fprintf(stderr,
      "Error (%s %d.%d.%d): Name '%s' could not be resolved.\n",
      ctx->current.file,
      ctx->current.suite.id,
      ctx->current.testcase.id,
      ctx->current.iteration.id,
      name);
    exit(EXIT_FAILURE);  
  }

  return result;

}

double get_d( const char* name, const test_context *ctx) {
  pair p = get_required_entry(name,D,ctx).pair;
  print_if(!ctx->testmode,&p, ctx->out);
  return p.tvalue.value.d;
}

int get_i( const char* name, const test_context *ctx) {
  pair p = get_required_entry(name,I,ctx).pair;
  print_if(!ctx->testmode,&p, ctx->out);
  return p.tvalue.value.i;
}

const char* get_s( const char* name, const test_context *ctx) {
  pair p = get_required_entry(name,S,ctx).pair;
  print_if(!ctx->testmode,&p, ctx->out);
  return p.tvalue.value.s;
}

void print_if( const bool condition, const pair *rec, FILE *out) {
  if (condition) {
    fprintf(out,"      %s: ",rec->name);
    print_value( & rec->tvalue, out );
    print_comment( rec, out );
    fprintf(out,"\n");
  }
}

void print_value( const typed_value *value, FILE *out ) {
  switch (value->type) {
    case D:
      fprintf(out,"%.20lf",value->value.d);
      break;
    case I:
      fprintf(out,"%d",value->value.i);
      break;
    case S:
      fprintf(out,"%s",value->value.s);
    default: 
      break;
  }
}

void print_comment(const pair *rec,FILE *out) {
  int ival = rec->tvalue.value.i;
  if (equals(rec->name,"ipl")) {
    char name[50];
    swe_get_planet_name(rec->tvalue.value.i,name);
    fprintf(out," # %s",name);
    return;
  } else if (equals(rec->name,"iflag")||equals(rec->name,"iephe")) {
    int iflags = rec->tvalue.value.i,
        i = 0;
    bool first = true;
    fprintf(out," # ");
    while (iflags != 0) {
      if (equals(SEFLG[i],"")) {
        fprintf(out,"%s2^%d",first?"":"+",i);
        break;
      }
      else {
        if (iflags % 2 == 1) {
          fprintf(out,"%s%s",first?"":"+",SEFLG[i]);
          first = false;
        }
      i++;
      }
      iflags >>= 1;
    }
  } else if (equals(rec->name,"jd") || equals(rec->name,"jd_ut") || equals(rec->name,"jx")) {
    int day, month, year, ihour, imin, isec, isgn;
    double ut, dfr;
    swe_revjul(rec->tvalue.value.d,1,&year,&month,&day,&ut);
    swe_split_deg(ut, SE_SPLIT_DEG_ROUND_SEC, &ihour, &imin, &isec, &dfr, &isgn);
    fprintf(out," # %d.%d.%d %02d:%02d:%02d",day,month,year,ihour, imin, isec);
  } else if (rec->tvalue.type == I  && ival >=32 && ival < 128 ) {
    fprintf(out," # '%c'", ival);
  }
}


void st_free( const typed_value *v) {
  if (!v->on_heap) return;
  if (v->type == P && v->value.p != 0) free(v->value.p);
  // Normally, it's weird casting const to non-const, but at the end of it's lifecycle it's OK:
  if (v->type == S && v->value.s != 0) free((char *)v->value.s);
}

#undef SETEST_H_PRIVATE
