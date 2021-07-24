#include "globals.h"

int equals( const char* act, const char* exp) { 
  return ! strcmp( act, exp ); 
  }

bool trimmed_equals( const char* string, const char *sub) {
  char* p =  strstr(string,sub);
  if (p) {
    // Non-whitespace before the substring -> fail
    for (char *p1=p-1;p1 >= string;p1--) {
      if (!isspace(*p1)) return false;
    }
    // Non-whitespace after the substring -> fail
    const char* end = string + strlen(string);
    for (char *p1=p+strlen(sub); p1 < end; p1++) {
      if (!isspace(*p1)) return false;
    }
    // -> string equals sub, save leading and railing whitespace
    return true;
  }
  return false;
}

bool equals_with_escaped_newlines(const char* exp, const char* act) {

// Compare \n in act against "\\n" in exp
// But ignore leading and trailing whitespace, like in trimmed_equals
  const char *pe = exp, *pa = act, *pe_end, *pa_end;
  // Put pa, pe to first non-white character
  for (;isspace(*pa) && *pa !='\0';pa++);
  for (;isspace(*pe) && *pe !='\0';pe++);
  // Put pa_end, pe_end to first trailing white character or \0
  for (pe_end = pe + strlen(pe)-1; pe_end>=pe && isspace(*pe_end);pe_end--);
  pe_end++;
  for (pa_end = pa + strlen(pa)-1; pa_end>=pa && isspace(*pa_end);pa_end--);
  pa_end++;
  // Compare non-white characters
  for (;pa < pa_end && pe < pe_end;pa++,pe++) {
    if (*pa == '\n' && *pe++ == '\\' && *pe == 'n') continue;
    if (*pa != *pe) return false;
  }
  // Strings are equal, if they end at the same point
  return (pa==pa_end && pe == pe_end);
}

bool ends_with(const char *string, const char * sub) {
  const int len = strlen(string);
  const char* sub_start = string + len - strlen(sub);
  for (const char *p = string+len-1; p>=sub_start; p--) {
    if (*p != sub[p-sub_start]) return false;
  } 
  return true;
}

char* trim_trailing_space(char *s) {
  char *sp = s + strlen( s ) - 1;
  while( sp>s && isspace(*sp) ) sp--;
  *(sp+1) = '\0';
  return s;
  }  

char* trim_space(char* s) {
  char* sp = trim_trailing_space(s);
  while (*sp != '\0' && isspace(*sp)) sp++;
  return sp;
  }    

char* copy_string(const char* s) {
  if (!s) return NULL;
  char *new = malloc(strlen(s)+1);
  if (!new) {
    fprintf(stderr,"Could not allocate %d bytes in copy_string()\n",
            (int)strlen(s) );
    exit(EXIT_FAILURE);
  }
 strcpy(new,s);
 return new;
}

bool is_empty(const char *s) {
  return s == NULL || *s == '\0';
} 

bool is_blank(const char *s) {
  for (const char *s1 = s;s1 != '\0';s1++) if (!isspace(*s1)) return false;
  return true;
}
