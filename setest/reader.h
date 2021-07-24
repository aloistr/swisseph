#ifndef READER_H
#define READER_H

#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "globals.h"

#define READER_MAX_SYMBOL_NAME_LENGTH 50
#define READER_MAX_LINES_INPUT_FILE 100
#define READER_MAX_LINESIZE 1000
#define READER_MAX_LEVELS 10

typedef struct {
  char name[READER_MAX_SYMBOL_NAME_LENGTH];
  char value[READER_MAX_LINESIZE];
} t_entry;

typedef struct {
  int length;
  t_entry *entry[READER_MAX_LINES_INPUT_FILE];
} t_rawtab;

typedef struct {
  FILE *source;
  const char **section_keywords;
  t_section next_section;
  bool with_prepare;
  t_rawtab block[READER_MAX_LEVELS];
} rdr_ctx;

rdr_ctx  open_reader(
  const char *fname, 
  const char* prepare_fix_m4, 
  const char** section_keywords);
rdr_ctx open_reader_from_stream(
   FILE *source,
   const char** section_keywords);

void read_next_block(rdr_ctx *reader_context,t_section);
void close_reader(rdr_ctx *reader_context);
char* read_value(
  const rdr_ctx *reader,
  const char *name, 
  const t_section from, 
  const t_section to);

char* read_value_in_section(
  const rdr_ctx *reader,
  const char *name, 
  const t_section section);


#ifdef READER_H_PRIVATE

char* find_value(const char *name, const t_rawtab *table);
t_section read_block(
  FILE *source, 
  t_rawtab *rawtab, 
  const char** section_keywords);
t_section section_line(const char* line, const char** section_keywords);
void push_row(t_entry entry, t_rawtab *rawtab);
void clear_table(t_rawtab *rawtab);


#endif
#endif
