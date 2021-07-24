#define _GNU_SOURCE // Needed for shared library info (see print_library_info() below)
#define __USE_GNU
#include <dlfcn.h>       // Needed for shared library info
#include <openssl/sha.h> // Needed for SHA1 hash of library binary
#include <time.h>       
#include <unistd.h>
#include <getopt.h>      // For long options
#include <sys/types.h>
#include <pwd.h>         // Needed for getting current username
#include "setest.h"

extern const char* HELPFILE;

int main(int argc, char** argv) {
  test_context ctx;
  initialize_test_context( &ctx );
  evaluate_cmdline_options(argc,argv,&ctx);
  run(&ctx);
  int exit_code = ctx.count.failures > 0 ? EXIT_FAILURE : EXIT_SUCCESS;
  cleanup(&ctx);
  return exit_code;
}

#if 1
void run( test_context* ctx ) {
  auto void general_setup(), general_cleanup(), print_stats( ), open_exp_or_fix_reader(), open_output_stream(), check_file_existence( ) ;
  general_setup( );
  read_next_block( & ctx->reader, GENERAL );
  if (ctx->parameters.source) read_next_block( & ctx->parameters, GENERAL );
  run_tests( ctx );
  print_stats( );
  general_cleanup( );
// --- Done. Details follow
  void general_setup() {
    check_file_existence( );
    open_exp_or_fix_reader( );
    open_output_stream( );
    if (!ctx->testmode) print_header_data( ctx );
  }
  void general_cleanup() {
    close_reader( & ctx->reader );
    close_reader( & ctx->parameters );
    if (ctx->out && ctx->out != stdout) fclose(ctx->out);
  }
  void open_exp_or_fix_reader() {
    char file[SETEST_MAX_SYMBOL_SIZE];
    strcpy(file,ctx->test_collection);
    strcat(file,ctx->testmode ? ".exp" : ".fix" );
    if (is_empty(ctx->prepare_fixture_command)) {
      sprintf(ctx->prepare_fixture_command,
        "%s %s",
        PREPARE_FIXTURE_COMMAND,
        file);
    }
    ctx->reader = open_reader( 
      file,
      ctx->testmode ? NULL :  ctx->prepare_fixture_command,
      SECTION_KEYWORDS
    ); 
  }
  void open_output_stream() {
    char file[SETEST_MAX_SYMBOL_SIZE];
    if (!ctx->testmode) {
      strcpy(file,ctx->test_collection);
      strcat(file,".exp");
      ctx->out = fopen(file,"w");
      if (!ctx->out) {
        fprintf(stderr,
                "Can't open file '%s' for output... using stdout\n"
                ,file);
        ctx->out = stdout;
      }  
    }
    else {
      ctx->out = stdout;
    }
  }
  void print_stats( ) {
    printf( "%s '%s': Total %d/%d/%d", 
      ctx->testmode ? "Testmode" : "gen-mode",
      ctx->test_collection,
      ctx->count.testsuite,
      ctx->count.testcase,
      ctx->count.iteration);
    if (ctx->testmode) {
      printf( " ... %d failures in %d iterations",ctx->count.failures, ctx->count.failed_iterations );
    }
    printf(".\n");
    }
  void check_file_existence( ) {
    char file[SETEST_MAX_SYMBOL_SIZE];
    sprintf(file,"%s.fix",ctx->test_collection);
    if (ctx->testmode) {
// Test mode: Need expectations file
      sprintf(file,"%s.exp",ctx->test_collection);
      if (access(file,F_OK)==-1) {
        fprintf(stderr,
            "Cannot read expectations '%s'.\nGenerate it with 'setest -g %s\n",
            file,ctx->test_collection);
        exit(EXIT_FAILURE);
      }
    } else {
// Gen mode: Need fixture file
      if (access(file,F_OK)==-1) {
        fprintf(stderr,
            "There is no fixture file '%s'.\nYou may generate it with 'make %s'.\n",
            file,file);
        exit(EXIT_FAILURE);
      }
    }
  }
}

void initialize_test_context( test_context *ctx ) {
  *ctx = (test_context) { 
    .testmode = true, 
    .selected = { 
      .testsuite = ALL, 
      .testcase = ALL, 
      .iteration = ALL 
    },
    .current = {
    },
    .precisions = { .all = DEFAULT_PRECISION, .xx = { [0 ... 5] = DEFAULT_PRECISION } },
    .multivalues = multivalues_create_table( ),
    .reader     = { .source = NULL },
    .parameters = { .source = NULL }
  };
  // The best readable way to initialize a char array:
  strcpy(ctx->test_collection, DEFAULT_TEST_COLLECTION );
}

void evaluate_cmdline_options(int argc, char** argv, test_context *ctx) {
  int c;
  auto void parse_select(const char *optarg,test_key *selected);
  struct option long_options[] = {
    { "gen",        no_argument,       0, 'g' },
    { "verbose",    no_argument,       0, 'v' },
    { "select",     required_argument, 0, 's' },
    { "preprocess", required_argument, 0, 'P' },
    { "parameters", required_argument, 0, 'p' },
    { "help",       no_argument,       0, '?' },
    { 0,0,0,0 } 
  };
  while (1) {
    int option_index = 0;
    c = getopt_long_only( argc, argv, "gvs:rp:P:?", long_options, &option_index );
    if (c == -1) break;
    switch(c) {
      case 0:
        /* If this option set a flag, do nothing else now. */
        if (long_options[option_index].flag != 0) break;
      case 'g':
        ctx->testmode = false;
        break;
      case 'p':
        ctx->parameters = get_param_reader(optarg);
        break;
      case 'P':
        strcpy(ctx->prepare_fixture_command,optarg);
        break;
      case 's':
        parse_select(optarg,&ctx->selected);
        break;
      case 'v':
        ctx->verbose = true;
        break;
      case '?':  // Means 'unknown option'
        print_help( );
        exit(EXIT_SUCCESS);
      default:   // shouldn't happen in general:
        fprintf(stderr,"Weird getopt result: '%c'\n", c);
        exit(EXIT_FAILURE);
    }
  }
// Remaining arguments from argv[optind] onwards
  if (optind < argc) {
     for (int i = 0; i < argc - optind; i++ ) {
       switch (i) {
         case 0:
// The remaining argument is the test collection name
           strcpy(ctx->test_collection,argv[optind]);
// Strip off file extension if given
           if (ends_with(ctx->test_collection,".fix") ||
               ends_with(ctx->test_collection,".exp")) {
             ctx->test_collection[strlen(ctx->test_collection)-4]='\0';
           }
           break;
         default:
           printf("Argument '%s' ignored.\n",argv[optind+i]);
       }
     }
  }
  void parse_select(const char *optarg,test_key *selected) {
    int n = sscanf(optarg,"%d.%d.%d",
                   &selected->testsuite,
                   &selected->testcase,
                   &selected->iteration);
    if (n == 0) {
      printf("Don't understand argument '%s' - ignored.\n",optarg);
    }
  }   
}
#endif

void open_testsuite(int id, char* description, test_context *ctx) {
  ctx->current.suite.id = id;
  ctx->current.suite.descr = description; // Transfer pointer only
  ctx->current.section = TESTSUITE;
  if (ctx->verbose) {
    printf("Opening test suite \n  %d: %s\n", id, description);
    printf("  located in %s\n",ctx->current.file);
  }
  if (!read_next_section( & ctx->reader, id, TESTSUITE )) {
    fprintf(stderr,"Could not read next testsuite section (%d)\n",id);
    exit(EXIT_FAILURE);
  }
  int disabled = get_disabled(TESTSUITE, ctx);
  ctx->current.suite.disabled = disabled > 0;
  if (!ctx->testmode) {
    fprintf(ctx->out,"TESTSUITE\n");
    fprintf(ctx->out,"  %s: %d\n","section-id",id);
    fprintf(ctx->out,"  %s: %s\n","section-descr",description);
    if (disabled) {
      fprintf( ctx->out,"  disabled: %d\n", disabled);
    }
  }
}

void prepare_precisions( test_context *ctx ) {
  const char *prec_all = NULL;
  if (ctx->parameters.source) {
    prec_all = read_value(&ctx->parameters, "precision",GENERAL,GENERAL);
  }
  if (!prec_all) {
    prec_all = read_value(&ctx->reader, "precision",TESTCASE,GENERAL);
  }
  if (prec_all) {
    if (!sscanf(prec_all,"%lf",&ctx->precisions.all)) {
      fprintf(stderr,"Can't read double from precision value '%s'\n",prec_all);
      exit(EXIT_FAILURE);
    }
    for (int i=0;i<6;i++) ctx->precisions.xx[i] = ctx->precisions.all;
    if (!ctx->testmode) {
      fprintf(ctx->out,"    precision: %s\n",prec_all);
    }
    if (ctx->verbose) {
      printf("    precision: %s\n", prec_all);
    }
  }
  const char* prec_xx = NULL;
  if (ctx->parameters.source) {
    prec_xx = read_value(&ctx->parameters, "precision-xx",GENERAL,GENERAL);
  }
  if (!prec_xx) {
    prec_xx = read_value(&ctx->reader, "precision-xx",TESTCASE,GENERAL);
  }
  if (prec_xx) {
    if (sscanf(prec_xx," %lf , %lf , %lf , %lf , %lf , %lf ",
          &ctx->precisions.xx[0],
          &ctx->precisions.xx[1],
          &ctx->precisions.xx[2],
          &ctx->precisions.xx[3],
          &ctx->precisions.xx[4],
          &ctx->precisions.xx[5]
          )!=6) {
      fprintf(stderr,"Couldn't read 6 values for precision-xx in '%s'\n",prec_xx);
      exit(EXIT_FAILURE);
    }
    if (!ctx->testmode) {
      fprintf(ctx->out,"    precision-xx: %s\n",prec_xx);
    }
    if (ctx->verbose) {
      printf("    precision-xx: %s\n", prec_xx);
    }
  }
}

void close_testsuite( test_context *ctx ) {
  if (!ctx->testmode || !ctx->current.suite.disabled)
  ctx->count.testsuite++;
}

void open_testcase(int id, char* description, test_context *ctx) {
  ctx->current.testcase.id = id;
  ctx->current.testcase.descr = description; // Transfer pointer only
  ctx->current.section = TESTCASE;
  if (ctx->verbose) {
    printf("  Opening test case \n    %d: %s\n", id, description);
  }
  if (!read_next_section( & ctx->reader, id,  TESTCASE )) {
    fprintf(stderr,"Could not read next testcase section (%d)\n",id);
    exit(EXIT_FAILURE);
  }
  int disabled = get_disabled(TESTCASE,ctx);
  ctx->current.testcase.disabled = disabled > 0;
  if (!ctx->testmode) {
    fprintf(ctx->out,"  %s","TESTCASE\n");
    fprintf(ctx->out,"    %s: %d\n","section-id",id);
    fprintf(ctx->out,"    %s: %s\n","section-descr",description);
    if (disabled) {
      fprintf(ctx->out,"    disabled: %d\n",disabled);
    }
  }
// Prepare precisions for this test case
  prepare_precisions( ctx );
}

void close_testcase( test_context *ctx ) {
  multivalues_clear(ctx->multivalues);
  ctx->count.testcase++;
}

void open_iteration( int id, test_context *ctx ) {
  ctx->current.section = ITERATION;
  const bool enabled = is_iteration_selected(ctx);
  if (ctx->verbose && enabled) {
    printf("      Iteration %d \n",id);
  }
  if (multivalues_all_done(ctx->multivalues)) {
    multivalues_clear(ctx->multivalues);
    // Read next iteration section if present (no error if nothing found)
    read_next_section( & ctx->reader, id, ITERATION );
  }
  if (!ctx->testmode && enabled) {
    fprintf(ctx->out,"    ITERATION\n");
    fprintf(ctx->out,"      section-id: %d  #%d.%d.%d\n",
        ctx->current.iteration.id,
        ctx->current.suite.id,
        ctx->current.testcase.id,
        ctx->current.iteration.id
        );
    // Transfer a comment "section-descr" if present
    const char* section_descr = read_value(&ctx->reader,"section-descr",ITERATION,ITERATION);
    if (section_descr) {
      fprintf(ctx->out,"      section-descr: %s\n",section_descr);
    }
  }
}

bool read_next_section( rdr_ctx *reader, int id, t_section section) {
  const char* section_id;
  do { 
    if (reader->next_section == END_OF_FILE  ) {
      return false;
    }
    read_next_block( reader, section );
    // Done if no ID specified by caller
    if (id == 0) return true;
    section_id = read_value_in_section(reader,"section-id",section);
    // Done if no ID specified in section
    if (!section_id) return true;
    // Done if ID matches
    if ( atoi(section_id) == id ) return true;
    // Failed if next section is of higher level
    if ( reader->next_section < section ) return false;
    } while(true);
    return false;  // --> Will never be reached
}

void close_iteration( test_context *ctx ) {
  handle_failures( ctx );
  clear_failures( & ctx->current.iteration.failures );  
  ctx->count.iteration++;
}

bool is_iteration_selected( test_context *ctx ) {
  return ( ctx->selected.iteration == ALL ||
           ctx->selected.iteration == ctx->current.iteration.id )
        && is_test_enabled( ctx );
}

bool is_testcase_selected( int id, test_context *ctx ) {
  return   (  ctx->selected.testcase == ALL ||
              ctx->selected.testcase == id   );
}

bool is_test_enabled(test_context* ctx) {
  return
         ! ctx->current.suite.disabled
      && ! ctx->current.testcase.disabled;
}

bool is_suite_selected( int id, test_context *ctx ) {
  return (  ctx->selected.testsuite == ALL ||
            ctx->selected.testsuite == id   );
  }

bool has_more_iterations(test_context* ctx) {
  return ( ! multivalues_all_done(ctx->multivalues)) ||
           ctx->reader.next_section == ITERATION;
}

void print_header_data(test_context *ctx) {
  print_date_time_user(ctx);
  print_version(ctx);
  print_library_info(ctx);
}

void print_date_time_user(test_context *ctx) {
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  fprintf(ctx->out,"localtime: %02d.%02d.%04d %02d:%02d:%02d\n",  tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec);

  struct passwd* p = getpwuid(getuid());
  fprintf(ctx->out,"user: %s\n",p->pw_name);

}

void print_version(test_context *ctx) {
  char version[20];
  swe_version(version);
  fprintf(ctx->out,"swisseph-version: %s\n", version);
}

void print_library_info(test_context *ctx) {

  Dl_info dl_info;
  dladdr((void *)swe_version, &dl_info);
  fprintf(ctx->out,"shared-lib-fname: %s\n",dl_info.dli_fname);

  SHA1HASH hash;
  sha1_from_file( dl_info.dli_fname, &hash );
  fprintf(ctx->out,"shared-lib-sha1: ");
  for (int i=0;i<20;i++) fprintf(ctx->out, "%02X", hash.key[i]);
  fprintf(ctx->out,"\n");

}


void sha1_from_file(const char *name, SHA1HASH* hash) {
  FILE *file;
  unsigned char *buffer;
  unsigned long fileLen;

  //Open file
  file = fopen(name, "rb");
  if (!file) {
    fprintf(stderr, "sha1_from_file: unable to open file %s", name);
    return;
  }

  //Get file length
  fseek(file, 0, SEEK_END);
  fileLen=ftell(file);
  fseek(file, 0, SEEK_SET);

  //Allocate memory
  buffer=(unsigned char *)malloc(fileLen+1);
  if (!buffer) {
    fprintf(stderr, "sha1_from_file: memory allocation problem!");
    fclose(file);
    return;
  }

  //Read file contents into buffer
  fread(buffer, fileLen, 1, file);
  fclose(file);

  SHA1(buffer, fileLen, hash->key );

  free(buffer);
}

void cleanup(test_context* ctx) {

  multivalues_destroy_table( ctx->multivalues );

}

void print_help() {
  FILE* help = fopen(HELPFILE,"r");
  char line[256];
  if (help) {
    while (fgets(line,255,help)) {
      printf("%s",line);
    }
  }

}

rdr_ctx get_param_reader(char* arg) {
// If the arg contains a ':',
// we consider it as direct input data and use a memory buffer stream
  if (strchr(arg,':')) {
// Make a local copy of arg
    char a[strlen(arg)+1];
    strcpy(a,arg);
// In the copy, replace all ';' by newlines
    for (char *p = a; *p != '\0'; p++) {
      if (*p == ';') *p = '\n';
    }
    FILE *f = fmemopen( NULL, strlen(a)+1, "r+");
    fprintf(f,"%s\n",a);
    fflush(f);
    rewind(f);
    return open_reader_from_stream(f,PARAMETER_SECTIONS);
  }
  else {
// If the arg contains no ':',
// it denotes a file name containing the parameters
    return open_reader( arg, NULL, PARAMETER_SECTIONS );
  }
}

int get_disabled(const t_section section,test_context *ctx) {
  const char* disabled_value = read_value_in_section( & ctx->reader, "disabled", section);
  if (disabled_value == NULL) return 0;
  int disabled = 0;
  if (! sscanf(disabled_value,"%d",&disabled)) return 0;
  if (disabled == 1) {
    switch (section) {
      case TESTSUITE:
        fprintf(stderr,"Suite %d (%s) disabled\n",
                  ctx->current.suite.id,
                  ctx->current.suite.descr);
        break;
      case TESTCASE:
        fprintf(stderr,"Test case %d.%d (%s) disabled\n",
                  ctx->current.suite.id,
                  ctx->current.testcase.id,
                  ctx->current.testcase.descr);
        break;
      default:
        break;
    }
  }
  return disabled;
}

