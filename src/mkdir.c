#include "privio.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>

int privio_mkdir(config_t *cfg, const char **args){
  char *error_str;
  if(mkdir(args[0], 0770) != 0){
    error_str = strerror(errno);
    privio_debug(cfg, DBG_ERROR, "mkdir(): %s\n", error_str);
    printf("{'%s':{error:'%s'}}\n", args[0], error_str);
    return -1;
  } else {
    printf("{'%s':{error:''}}\n", args[0]);
    return 0;
  }
}
