#include "privio.h"
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

int privio_writer(config_t *cfg){
  char **paths;
  FILE *fp;

  paths = privioReadPaths(cfg, 1);

  if (paths == NULL)
    return EPERM;

  privio_debug(cfg, DBG_INFO, "Writing to %s\n", paths[0]);
  umask(0077);
  fp = fopen(paths[0], "w");
  if (fp == NULL){
    privio_debug(cfg, DBG_VERBOSE, "Error opening file %s: %s\n", strerror(errno));
    return errno;
  }

  /* read from named pipe, write to fp */

  fclose(fp);
  
  return 0;
}
