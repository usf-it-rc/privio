#include "privio.h"
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

int privio_tail(config_t *cfg, const char **args){
  FILE *fp;
  unsigned long offset;

  if (offset < 1)
    return 10;

  privio_debug(cfg, DBG_INFO, "Reading tail of %s starting at %ld\n", args[0], offset);
  fp = fopen(args[0], "r");
  if (fp == NULL){
    privio_debug(cfg, DBG_VERBOSE, "Error opening file %s: %s\n", args[0], strerror(errno));
    return errno;
  }

  /* read from named pipe, write to fp */

  fclose(fp);
  
  return 0;
}
