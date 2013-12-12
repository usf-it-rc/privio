#include "privio.h"
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <magic.h>

int privio_type(config_t *cfg, const char **args){
  const char *mime;
  magic_t magic;

  privio_debug(cfg, DBG_INFO, "Getting magic from %s\n", args[0]);

  magic = magic_open(MAGIC_MIME_TYPE); 
  magic_load(magic, NULL);
  magic_compile(magic, NULL);
  mime = magic_file(magic, args[0]);

  if (mime == NULL){
    printf("{'%s':{'mime':'','error':'error accessing file'}}\n", args[0]);
  } else {
    printf("{'%s':{'mime':'%s','error':''}\n", args[0], mime);
  }

  magic_close(magic);
  
  return 0;
}
