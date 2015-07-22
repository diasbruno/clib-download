#ifndef CLIB_DOWNLOAD_H

#include "asprintf/asprintf.h"
#include "fs/fs.h"
#include "trim/trim.h"
#include "tempdir/tempdir.h"
#include "commander/commander.h"
#include "clib-package/clib-package.h"
#include "http-get/http-get.h"
#include "logger/logger.h"
#include "debug/debug.h"
#include "mkdirp/mkdirp.h"
#include "version.h"

#define format(...) ({                               \
  if (-1 == asprintf(__VA_ARGS__)) {                 \
    rc = 1;                                          \
    fprintf(stderr, "Memory allocation failure\n");  \
  }                                                  \
})

#define CLIB_MAX_PACKAGES 32

struct clib_rscs_t
{
  char*  home;
  char*  dir;
};

#define VERBOSE if (verbose)

/**
 *
 */
void clib_rscs_init(struct clib_rscs_t* rscs);
/**
 *
 */
int clib_rscs_check_dir(struct clib_rscs_t* rscs, int verbose);
/**
 *
 */
const char** clib_find_pkgs(struct clib_rscs_t* rscs,
                            int argc, const char* argv[],
                            int verbose);
/**
 *
 */
void clib_download_packages(struct clib_rscs_t* rscs,
                            int argc, char* argv[],
                            int verbose);
/**
 *
 */
void clib_download_package(struct clib_rscs_t* rscs,
                           const char* pkg, int verbose);
/**
 *
 */
char* clib_package_archive_url(const char* a, const char* n, const char* v);
/**
 *
 */
char* clib_package_archive_file(const char* clibd,
                                const char* a, const char* n, const char* v);
/**
 *
 */
char* clib_package_archive_path(const char* clibd,
                                const char* a, const char* n);
/**
 *
 */
void clib_package_unpack(const char* file, const char* path,
                         const char* pathversion,
                         const char* name, const char* version, int verbose);
/**
 *
 */
void clib_rscs_free(struct clib_rscs_t* rscs);

#define CLIB_DOWNLOAD_H 1
#endif
