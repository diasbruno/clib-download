//
// clib-download.c
//
// Copyright (c) 2012-2015 clib authors
// MIT licensed
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "trim/trim.h"
#include "clib-download.h"
#include "shcmd/shcmd.h"

static const char*
get_home_dir(void)
{
  // should not be free'd
  return getenv("HOME");
}

void
clib_rscs_init(struct clib_rscs_t* rscs)
{
  char* clibdir = ".clib.d";
  char* home    = (char*)get_home_dir();
  int   rc      = 0;

  rscs->home = strdup(home);
  format(&rscs->dir, "%s/%s", home, clibdir);
}

int
clib_rscs_check_dir(struct clib_rscs_t* rscs, int verbose)
{
  int r = fs_exists(rscs->dir);

  VERBOSE { printf("- checking for \"./clib.d\".\n"); }

  if (-1 == r) {
    VERBOSE { printf("- creating \"./.clib.d\" folder at %s.\
\n- %s\n", rscs->home, rscs->dir); }
    r = mkdirp(rscs->dir, 0755);
  }

  return r;
}

char*
clib_package_archive_url(const char* host, const char* a,
                         const char* n, const char* v)
{
  char *url = malloc(sizeof(char) * 1024);
  char* cmd = NULL;
  memset(url, '\0', 1024);

  if (url) {
    asprintf(&cmd, "sh ./scripts/archive.sh %s %s %s %s", host, a, n, v);

    if (cmd) {
      shcmd(cmd, url);
    }
  }

  free(cmd);

  return url;
}

char*
clib_package_archive_file(const char* clibd,
                          const char* a, const char* n, const char* v)
{
  int size = strlen(clibd) + 1 // /
    + strlen(a) + 1 // /
    + strlen(n) + 1 // /
    + strlen(v) + 1 // /
    + sizeof(".zip") + 1 // \0
    ;

  char *path = malloc(size);
  if (path) {
    memset(path, '\0', size);
    sprintf(path, "%s/%s/%s/%s.zip", clibd, a, n, v);
  }

  return path;
}

char*
clib_package_archive_path(const char* clibd,
                          const char* a, const char* n)
{
  int size = strlen(clibd) + 1 // /
    + strlen(a) + 1 // /
    + strlen(n) + 1 // \0
    ;

  char *path = malloc(size);
  if (path) {
    memset(path, '\0', size);
    sprintf(path, "%s/%s/%s", clibd, a, n);
  }

  return path;
}

char*
clib_package_archive_version_path(const char* clibd,
                                  const char* a, const char* n, const char* v)
{
  int size = strlen(clibd) + 1 // /
    + strlen(a) + 1 // /
    + strlen(n) + 1 // /
    + strlen(v) + 1 // \0
    ;

  char *path = malloc(size);
  if (path) {
    memset(path, '\0', size);
    sprintf(path, "%s/%s/%s/%s", clibd, a, n, v);
  }

  return path;
}

void
clib_download_packages(struct clib_rscs_t* rscs, int argc, char* argv[], int verbose)
{
  const char** pks =
    (const char**)clib_find_pkgs(rscs, argc, (const char**)argv, verbose);
  char** c = NULL;

  VERBOSE { printf("[downloading]\n"); }

  if (pks) {
    c = (char**)pks;
    while (*c != NULL) {
      clib_download_package(rscs, *c, verbose);
      free((void*)*c);
      c++;
    }
    free((void*)pks);
  }
}

void
clib_package_unpack(const char* file, const char* path, const char* pathversion,
                    const char* name, const char* version, int verbose)
{
  char* cmd     = NULL;
  char* unpname = NULL;
  char* fixname = NULL;
  char* mvcmd   = NULL;
  char* rmcmd   = NULL;
  asprintf(&cmd, "%s -qq %s -d %s", "unzip", file, path);
  asprintf(&unpname, "%s/%s-%s", path, name, version);
  asprintf(&fixname, "%s/%s", path, version);
  asprintf(&mvcmd, "%s %s %s", "mv", unpname, fixname);
  asprintf(&rmcmd, "%s %s", "rm", file);

  int r = system(cmd);
  if (r == 0) {
    r = system(mvcmd);
    if (r == 0) {
      VERBOSE { printf("- unpacked %s\n", file); }
      r = system(rmcmd);
      if (r == 0) {
        VERBOSE { printf("- removed zip %s\n", file); }
      }
    }
  }

  free(cmd);
  free(unpname);
  free(fixname);
  free(mvcmd);
  free(rmcmd);
}

void
clib_download_package(struct clib_rscs_t* rscs, const char* pkg, int verbose)
{
  char* a = clib_package_parse_author(pkg);
  char* n = clib_package_parse_name(pkg);
  char* v = clib_package_parse_version(pkg);
  char* d = rscs->dir;

  char* pathversion = clib_package_archive_version_path(d, a, n, v);
  char* url  = NULL;
  char* file = NULL;
  char* path = NULL;

  // user/package/version
  if (0 == fs_exists(pathversion)) {
    printf("- %s/%s@%s is already installed.\n", a, n, v);
    free((void*)pathversion);
    return;
  }

  url  = clib_package_archive_url("github", a, n, v);
  url  = trim(url);
  file = clib_package_archive_file(d, a, n, v);
  path = clib_package_archive_path(d, a, n);

  VERBOSE { printf("- download from %s\n", url); }

  // user/package
  if (-1 == mkdirp(path, 0755)) {
    fprintf(stderr, "Failed to create the path %s\n", path);
    goto cleanup;
  }

  // user/package
  if (-1 == fs_exists(path)) {
    fprintf(stderr, "Failed to create the path %s\n", path);
    goto cleanup;
  }

  if (-1 == fs_exists(pathversion)) {
    printf("- downloading %s/%s@%s\n", a, n, v);
    if (0 == http_get_file(url, file)) {
      printf("- downloaded!\n");
      clib_package_unpack(file, path, pathversion, n, v, verbose);
    } else {
      fprintf(stderr, "- failed to download %s\n", file);
    }
  }

cleanup:
  free((void*)file);
  free((void*)path);
  free((void*)url);
  free((void*)v);
  free((void*)n);
  free((void*)a);
}

const char**
clib_find_pkgs(struct clib_rscs_t* rscs, int argc, const char* argv[], int verbose)
{
  VERBOSE { printf("[finding packages]\n"); }

  const char** pk = (const char**)calloc(sizeof(char**), CLIB_MAX_PACKAGES);
  char** c        = (char**)pk;
  int piter       = 0;
  int cc          = 0;

  while (piter < argc) {
    cc = piter + 2;
    if (cc > argc) { break; }
    if (argv[cc] != 0 && '-' != argv[cc][0]) {
      *c = strdup(argv[cc]);
      VERBOSE { printf("- package %s\n", *c); }
      c++;
    }
    ++piter;
  }

  return pk;
}

void
clib_rscs_free(struct clib_rscs_t* rscs)
{
  if (rscs->dir)
    free((void*)rscs->dir);
  if (rscs->home)
    free((void*)rscs->home);
}
