/*
 * privio_rename(config_t *, const char **)
 *
 * This is a very basic file rename function.  It simply renames args[0] to args[1]
 *
 */

#include "privio.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>

int privio_rename(config_t *cfg, const char **args){
  char *error_str;
  if (rename(args[0], args[1]) != 0){
    error_str = strerror(errno);
    printf("{'%s':{'new_name':'','error':'%s'}}\n", args[0], error_str);
    privio_debug(cfg, DBG_ERROR, "rename('%s','%s'): %s\n", args[0], args[1], error_str);
    return -1;
  } else {
    printf("{'%s':{'new_name':'%s','error':''}}\n", args[0], args[1]);
  }
  return 0;
}
