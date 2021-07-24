#define READER_H_PRIVATE
#include "reader.h"

rdr_ctx  open_reader(
  const char *fname, 
  const char* prepare_cmd, 
  const char** section_keywords) {

  rdr_ctx c = { .section_keywords = section_keywords };

  if (access(fname,F_OK) == -1) {
    fprintf(stderr,"File '%s' does not exist.\n",fname);
    exit(EXIT_FAILURE);
  }

  if (is_empty(prepare_cmd)) {
    c.with_prepare = false;
    c.source = fopen(fname,"r");
    if (c.source == NULL) {
      fprintf(stderr,"Can't open input file %s\n",fname);
      exit(EXIT_FAILURE);
    }
  }
  else {
    c.with_prepare = true;
    c.source = popen(prepare_cmd,"r");
    if (c.source == NULL) {
      fprintf(stderr,"Error when executing command %s\n",prepare_cmd);
      exit(EXIT_FAILURE);
    }
  }

  return c;

}

rdr_ctx open_reader_from_stream(FILE *source, const char** section_keywords) {
  rdr_ctx c = {
      .section_keywords = section_keywords,
      .source           = source,
      .with_prepare     = false
  };
  return c;
}


void read_next_block(rdr_ctx *rdr, t_section section) {

// End of input reached: There is nothing to do
  if (rdr->next_section == END_OF_FILE) return;

// Next level deeper than requested: skip (i.e. ignore)
  while (rdr->next_section > section) read_next_block( rdr, rdr->next_section);

// In any other case: Reset existing entries,
// starting with the requested section
  for (int i=section; i<READER_MAX_LEVELS;i++) {
    clear_table( & rdr->block[i] );
  }
 
// Next chunk fits: read it
  if (rdr->next_section == section) {
    clear_table( & rdr->block[section] );
    rdr->next_section = read_block(
			   rdr->source,
			   & rdr->block[section],
			   rdr->section_keywords);
  }

}

// Look in several blocks, ascending levels, for a symbol
char* read_value(
  const rdr_ctx *reader,
  const char *name, 
  const t_section from, 
  const t_section to) {

  for (int i = from; i >= to; i--) {
    char *value = find_value(name, & reader->block[i]);
    if (value) return value;   
  }

  return NULL; // NOT FOUND

}

// Look only in the specified level
char* read_value_in_section(
  const rdr_ctx *reader,
  const char* name,
  const t_section section) {

  return find_value(name,&reader->block[section]);

}

// Scan a table of name/value-pairs for a given name
char* find_value(const char *name, const t_rawtab *table) { 
  
  for (int i=0; i<table->length; i++) {
    if (equals(name,table->entry[i]->name)) {
      return table->entry[i]->value;
    }
  }

 return NULL;

}

t_section read_block(
  FILE* source, 
  t_rawtab *table, 
  const char **section_keywords) {
  
  char line[READER_MAX_LINESIZE];
  for (int i = 1;
       fgets(line ,READER_MAX_LINESIZE , source ) != NULL;
       i++) {

// Exit condition: Next section marker
    t_section section = section_line( line, section_keywords );
    if (section != UNDEFINED) {
      return section;
    }

// Ignore empty lines
    if (is_blank(line)) continue;

// Ignore comments    
    if (sscanf(line," #%c",&line[0])>0) continue; 

// Find and store "name:value"
    t_entry e;
    int n = sscanf(line," %50[^ :] : %[^\n]",e.name,e.value);
    switch (n) {
      case 1:
        e.value[0] = '\0';  // ... and continue with case 2:
      case 2:
        push_row(e,table);
        break;
      case 0:
	      printf("# reader: ignoring line %d: %s",i,line);
    }
  }

 return END_OF_FILE;

}

t_section section_line(const char* line, const char** section_keywords) {
  int i = 0;
  while (true) {
    const char *section = section_keywords[i];
    if (is_empty(section)) return UNDEFINED;
    if (trimmed_equals(line,section)) return i; 
    i++;
  }
}

void push_row(t_entry entry, t_rawtab *table) {

  if (table->length == READER_MAX_LINES_INPUT_FILE - 1) {
    fprintf(stderr,
            "reader.c: max input lines (%d) exceeded\n", 
            READER_MAX_LINES_INPUT_FILE);
  }

  t_entry *new = malloc( sizeof(t_entry ) );
  if (new == NULL) {
    fprintf(stderr,"Heap memory overflow in reader.c:push_row()\n");
    exit(EXIT_FAILURE);
    } 

  *new = entry;
  table->entry[table->length] = new;
  table->length++;

}

void clear_table(t_rawtab *table) {
  for (int i=0; i<table->length; i++) {
    free( table->entry[i]);
    }
  table->length = 0;
  }

void close_reader(rdr_ctx *c) {

  for (int i=0;i<=4;i++) clear_table( & c->block[i] );  

  if (c->source) {
    int rc = (c->with_prepare ? pclose( c->source ) : fclose( c->source ) );
    if (rc == -1) {
      fprintf(stderr,"Error code %d when closing %sfixture file\n",
         rc,
         c->with_prepare ? "preprocessed " : "");
    } 
  }

}

#undef READER_H_PRIVATE
