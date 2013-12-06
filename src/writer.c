#include "privio.h"

int privio_writer(config_t *cfg){
  char **paths;

  paths = privioReadPaths(cfg, 1);

  if (paths != NULL){
    privio_debug(cfg, DBG_INFO, "Writing to %s\n", paths[0]);
  }

  privio_debug(cfg, DBG_INFO, "This is a stub function for privio_writer\n");
  return 0;
}
