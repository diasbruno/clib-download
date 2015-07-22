#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "clib-download.h"

#define PROGRAM_NAME "clib-download"

debug_t debugger;

struct options {
  int verbose;
};

static struct options opts;

static void
setopt_verbose(struct command* self)
{
  opts.verbose = 1;
  debug(&debugger, "set verbose flag");
}

int main(int argc, char *argv[])
{
  char* cmd   = NULL;
  int verbose = 0;
  struct clib_rscs_t rscs;
  rscs.home = rscs.dir = 0;

  opts.verbose = 0;

  debug_init(&debugger, PROGRAM_NAME);
  
  command_t program;
  program.usage = "[options]";
  command_init(&program, PROGRAM_NAME, CLIB_DONWLOAD_VERSION);
  command_option(&program, "-v", "--verbose", "set verbose output", setopt_verbose);
  command_parse(&program, argc, argv);

  debug(&debugger, "%d arguments", program.argc);

  if (argc == 1) {
    command_help(&program);
    exit(1);
  }
  
  verbose = opts.verbose;
  
  VERBOSE {
    printf("clib-download (verbose)\n");
    printf("[check clib env]\n");
  }

  clib_rscs_init(&rscs);

  if (-1 == clib_rscs_check_dir(&rscs, verbose)) {
    VERBOSE { printf("- fail.\n"); }
    goto cleanup;
  } else {
    VERBOSE { printf("- created.\n"); }
  }

  cmd = strdup(argv[1]);
  if (cmd == 0) {
    fprintf(stderr, "Memory allocation failed.\n");
  }
  cmd = trim(cmd);

  if (0 == strcmp(cmd, "pkg")) {
    clib_download_packages(&rscs, argc, argv, verbose);
  }

cleanup:
  free(cmd);
  command_free(&program);
  clib_rscs_free(&rscs);

  return 0;
}
