#include "setest.h"
#include <sys/stat.h>
#include <time.h>

// Environment of a test run: the data files the library uses (2026-11-22).
// In gen mode it is written to the header of the .exp file; in test mode it
// is compared with that header, and every difference gives a warning on stderr.
// The values hold file name, DE number and SHA1; full paths go into comment
// lines, which the reader ignores.

#define ENV_MAX 6
#define ENV_J2000 2451545.0
#define ENV_JPL_FILE "de441.eph"   // as set by suites 1 and 10

typedef struct {
  char key[51];
  char value[AS_MAXCH];
  char path[AS_MAXCH];
} env_entry;

static const char *base_name(const char *path) {
  const char *p = strrchr(path, '/');
  return p ? p + 1 : path;
}

// Name, DE number and SHA1 of the file the library used for body ipl
static void probe_file(env_entry *e, const char *key, int ifno, int ipl) {
  double x[6], tfstart = 0, tfend = 0;
  int denum = 0;
  char serr[AS_MAXCH] = "";
  SHA1HASH hash = { { 0 } };
  const char *path;
  strcpy(e->key, key);
  swe_calc(ENV_J2000, ipl, SEFLG_SWIEPH, x, serr);
  path = swe_get_current_file_data(ifno, &tfstart, &tfend, &denum);
  if (path == NULL || *path == '\0') {
    strcpy(e->value, "none");
    e->path[0] = '\0';
    return;
  }
  snprintf(e->path, sizeof(e->path), "%s", path);
  sha1_from_file(path, &hash);
  int n = snprintf(e->value, sizeof(e->value), "%s DE%d SHA1:", base_name(path), denum);
  for (int i = 0; i < 20 && n + 2 < (int) sizeof(e->value); i++, n += 2) sprintf(e->value + n, "%02X", hash.key[i]);
}

// JPL file (name, size and date only; it is too large for SHA1) and whether
// the EOP files for the JPL Horizons mode were found
static void probe_jpl(env_entry *jpl, env_entry *eop) {
  double x[6], tfstart = 0, tfend = 0;
  int denum = 0;
  char serr[AS_MAXCH] = "";
  struct stat st;
  strcpy(jpl->key, "env-jpl-file");
  strcpy(eop->key, "env-eop-files");
  eop->path[0] = '\0';
  swe_set_jpl_file(ENV_JPL_FILE);
  int32 rc = swe_calc(ENV_J2000, SE_SUN, SEFLG_JPLEPH | SEFLG_JPLHOR, x, serr);
  const char *path = swe_get_current_file_data(0, &tfstart, &tfend, &denum);
  if (rc < 0 || !(rc & SEFLG_JPLEPH)) {
    sprintf(jpl->value, "%s not found", ENV_JPL_FILE);
    jpl->path[0] = '\0';
  } else if (path != NULL && *path != '\0' && stat(path, &st) == 0) {
    struct tm tm = *localtime(&st.st_mtime);
    snprintf(jpl->path, sizeof(jpl->path), "%s", path);
    snprintf(jpl->value, sizeof(jpl->value), "%s size:%lld mtime:%04d-%02d-%02d",
             base_name(path), (long long) st.st_size, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
  } else {
    sprintf(jpl->value, "%s", ENV_JPL_FILE);
    jpl->path[0] = '\0';
  }
  if (rc >= 0 && (rc & SEFLG_JPLHOR))
    strcpy(eop->value, "present");
  else
    strcpy(eop->value, "missing");
}

static int collect_environment(env_entry *env) {
  int n = 0;
  swe_set_ephe_path(NULL);
  probe_file(&env[n++], "env-planet-file", 0, SE_SUN);
  probe_file(&env[n++], "env-moon-file", 1, SE_MOON);
  probe_file(&env[n++], "env-asteroid-file", 2, SE_CERES);
  probe_file(&env[n++], "env-eros-file", 3, SE_AST_OFFSET + 433);
  probe_jpl(&env[n], &env[n + 1]);
  n += 2;
// The tests start from the same state as without these probes
  swe_close();
  return n;
}

// Gen mode: write the environment to the header of the .exp file
void print_environment(test_context *ctx) {
  env_entry env[ENV_MAX];
  int n = collect_environment(env);
  for (int i = 0; i < n; i++) {
    if (env[i].path[0] != '\0') fprintf(ctx->out, "# %s: %s\n", env[i].key, env[i].path);
    fprintf(ctx->out, "%s: %s\n", env[i].key, env[i].value);
  }
}

// Value without trailing blanks
static void trimmed(const char *s, char *out, int maxlen) {
  int n = snprintf(out, maxlen, "%s", s);
  if (n >= maxlen) n = maxlen - 1;
  while (n > 0 && (out[n - 1] == ' ' || out[n - 1] == '\t' || out[n - 1] == '\r')) out[--n] = '\0';
}

// Test mode: compare the environment and the library version with the
// header of the .exp file; a difference gives a warning, the tests continue
void check_environment(test_context *ctx) {
  env_entry env[ENV_MAX];
  char version[20], expected[AS_MAXCH];
  int ndiff = 0;
  const char *exp = read_value(&ctx->reader, "swisseph-version", GENERAL, GENERAL);
  swe_version(version);
  if (exp != NULL) {
    trimmed(exp, expected, sizeof(expected));
    if (strcmp(expected, version) != 0) {
      fprintf(stderr, "Warning: swisseph-version is %s, %s.exp was made with %s\n", version, ctx->test_collection, expected);
      ndiff++;
    }
  }
  int n = collect_environment(env);
  for (int i = 0; i < n; i++) {
    exp = read_value(&ctx->reader, env[i].key, GENERAL, GENERAL);
    if (exp == NULL) {
      fprintf(stderr, "Warning: %s.exp records no %s (made before 2026-11-22?)\n", ctx->test_collection, env[i].key);
      ndiff++;
      continue;
    }
    trimmed(exp, expected, sizeof(expected));
    if (strcmp(expected, env[i].value) != 0) {
      fprintf(stderr, "Warning: %s differs from %s.exp\n  expected: %s\n  now:      %s%s%s\n",
              env[i].key, ctx->test_collection, expected, env[i].value,
              env[i].path[0] ? " # " : "", env[i].path);
      ndiff++;
    }
  }
  if (ndiff > 0) fprintf(stderr, "Warning: %d difference(s) in the environment; failures may come from them\n", ndiff);
}
