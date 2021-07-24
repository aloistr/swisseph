#ifndef MULTIVALUES_H
#define MULTIVALUES_H

#define SETEST_MAX_MULTIVARS   200
#define SETEST_MAX_MULTIVALUES 200

typedef struct {
    char name[SETEST_MAX_SYMBOL_SIZE];
    int i;
    bool carry;
    int length;
    typed_value* mvalue_entries;
} mvalues;

typedef struct {
  mvalues* mv;
  int length;
} mvalues_tab;

mvalues_tab* multivalues_create_table( );
void multivalues_destroy_table( mvalues_tab* tab);
int multivalues_get_index(const char* name,mvalues_tab* multivalues);
int multivalues_add_index(const char *name,const st_type type,const char* value,mvalues_tab* multivalues);
bool is_multivalue(const st_type type,const char* value);
const typed_value* multivalues_get_next( int mv,  mvalues_tab* multivalues);
bool multivalues_all_done(mvalues_tab* multivalues);
void parse_int_range( const char* value, mvalues* mval);
void parse_double_range( const char* value, mvalues* mval);
void multivalues_clear(mvalues_tab* multivalues);

bool add_int_to_table(int value, mvalues* mval);
bool add_double_to_table(double value, mvalues* mval);
bool add_value_to_table(typed_value *value, mvalues* mval);

#endif
