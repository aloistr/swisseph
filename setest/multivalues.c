#include "setest.h"
#include "testdata.h"
#include "multivalues.h"

int multivalues_get_index(const char* name,mvalues_tab* multivalues) {

  if (!multivalues) return -1;

  for (int i=0; i<multivalues->length;i++) {
    if (equals(name,multivalues->mv[i].name)) {
      return i;
    }
  }

  return -1;
}

const typed_value* multivalues_get_next( int n, mvalues_tab* multivalues) {

  auto bool previous_finished( );

  mvalues* mv = & multivalues->mv[n];

  bool just_created = (mv->i == -1);

  if (just_created) {
    mv->i = 0;
  }
  else if (previous_finished(n,multivalues)) {
// Cyclically increment index
    mv->i     = ( mv-> i + 1 ) % mv->length;
    mv->carry = mv->i == 0;
  }

// Return ref to next (or current) value
  return & mv->mvalue_entries[mv->i];

// Check if all the previous multivalues just had a cycle finished
  bool previous_finished() {
    for (int j=0; j<n; j++) {
      if (!multivalues->mv[j].carry) return false;
    }
    return true;
  }

}


bool is_multivalue(const st_type type, const char* value) {
// Only integers and doubles
  if (type != I && type != D) return false;
// Value array like "1,2,3,4"
  if (strchr(value,',')) return true;
// ... or value range like "5-10"
  const char* p = strchr(value,'-');
  if ( p && p > value && strpbrk(value,"0123456789")<p) return true;
  return false;
}

mvalues_tab* multivalues_create_table( ) {
  mvalues_tab *tab = malloc( sizeof( mvalues_tab ) );
  tab->length = 0;
  tab->mv = malloc( sizeof( mvalues ) * SETEST_MAX_MULTIVARS );
  return tab;
}

void multivalues_destroy_table( mvalues_tab* tab) {
  free( tab->mv );
  free( tab );
}

int multivalues_add_index(const char *name,const st_type type,const char* value,mvalues_tab* multivalues) {
  int i = multivalues->length;
  multivalues->mv[i] = (mvalues) {
    .length = 0,
    .mvalue_entries = malloc( sizeof(typed_value) * SETEST_MAX_MULTIVALUES ),
    .i = -1
  };
  strcpy(multivalues->mv[i].name,name);
  multivalues->length++;
  switch (type) {
    case I:
      parse_int_range(value,&multivalues->mv[i]);
      break;
    case D:
      parse_double_range(value,&multivalues->mv[i]);
      break;
    default:
      break;
  }
  return i;
}

void parse_int_range( const char* value, mvalues* mval) {

  const char *p0 = value,
             *p1,
             *end = p0 + strlen(value);
  int i0, i1, istep = 0;
  bool ok = true;
  do {
    int n = sscanf(p0,"%d - %d : %d", &i0, &i1, &istep);
    if (n == 0 && *p0 == '\'') {
      n = sscanf(p0 + 1,"%c", (char *) &i0);	// try 'P','C' ..
    }
    switch (n) {
      case 1:
        ok = add_int_to_table( i0, mval);
        break;
      case 2:
        istep = 1;
      case 3:
        if (istep == 0) istep = 1;
        if (istep > 0) {
          for (int i=i0;i<=i1 && ok;i+=istep) {
            ok = add_int_to_table(i, mval );
          }
        } else {	// loop downwards
          for (int i=i0;i>=i1 && ok;i+=istep) {
            ok = add_int_to_table(i, mval);
          }
        }
        break;
      default:
        fprintf(stderr,"Could not parse multivalue expression '%s'\n",p0);
        return;
    }
    if (!ok) {
      fprintf(stderr,"Error while processing subexpression '%s'\n",p0);
      exit(EXIT_FAILURE);
    }
  } while(
       (p1 = strchr(p0,',')) != NULL &&
       (p0 = p1+1)           <  end  &&
       mval->length          < SETEST_MAX_MULTIVALUES );
}

bool add_int_to_table(int value, mvalues* mval) {
  return add_value_to_table( & (typed_value) {
    .type = I,
    .value.i = value
  }, mval);
}

bool add_double_to_table(double value, mvalues* mval) {
  return add_value_to_table( & (typed_value) {
    .type = D,
    .value.d = value
  }, mval);
}

bool add_value_to_table(typed_value *value, mvalues* mval) {
  if (mval->length >= SETEST_MAX_MULTIVALUES) {
    fprintf(stderr,"Warning: adding value resulted in more than %d cases\n",
            SETEST_MAX_MULTIVALUES);
    return false;
  }
  mval->mvalue_entries[mval->length] = (typed_value) {
    .type  = value->type,
    .value = value->value
  };
  (mval->length)++;
  return true;
}

void parse_double_range( const char* value, mvalues* mval) {

  const char *p0 = value,
             *p1,
             *end = p0 + strlen(value);
  double d0, d1, dstep;
  bool ok = true;
  do {
    int n = sscanf(p0,"%lf - %lf : %lf", &d0, &d1, &dstep);
    switch (n) {
      case 1:
        ok = add_double_to_table(d0,mval);
        break;
      case 2:
        dstep = 1;
      case 3:
        if (dstep == 0) dstep = 1;
        if (dstep > 0) {
          for (double d=d0; d<=d1 && ok; d+=dstep) {
            ok = add_double_to_table(d,mval);
          }
        } else {	// loop downwards
          for (double d=d0; d>=d1 && ok; d+=dstep) {
            ok = add_double_to_table(d,mval);
          }
        }
        break;
      default:
        fprintf(stderr,"Could not parse multivalue expression '%s'\n",p0);
        exit(EXIT_FAILURE);
    }
    if (!ok) {
      fprintf(stderr,"Error while processing multivalue expression '%s'",p0);
      return;
    }
  } while(
       (p1 = strchr(p0,',')) != NULL &&
       (p0 = p1+1)           <  end  &&
       mval->length          < SETEST_MAX_MULTIVALUES );
}

bool multivalues_all_done(mvalues_tab* multivalues) {
  for (int j=0; j<multivalues->length; j++) {
    if (multivalues->mv[j].i != multivalues->mv[j].length-1) return false;
  }
  return true;
}

void multivalues_clear(mvalues_tab* multivalues) {
  for (int i=0; i<multivalues->length;i++) {
    free( multivalues->mv[i].mvalue_entries );
  }
  multivalues->length = 0;
}

