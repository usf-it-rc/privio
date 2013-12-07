#include "privio.h"
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

int privio_reader(config_t *cfg){
  char **paths;
  FILE *fp;

  paths = privioReadPaths(cfg, 1);

  if (paths == NULL)
    return EPERM;

  privio_debug(cfg, DBG_INFO, "Reading from %s\n", paths[0]);
  fp = fopen(paths[0], "r");
  if (fp == NULL){
    privio_debug(cfg, DBG_VERBOSE, "Error opening file %s: %s\n", paths[0], strerror(errno));
    return errno;
  }

  /* read from named pipe, write to fp */

  fclose(fp);
  
  return 0;
}
