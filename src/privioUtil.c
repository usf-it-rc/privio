/* 
 * privioUtil.c
 *
 * Utility functions like:
 *
 * * Getting configuration data from config file
 * * Validating paths
 * * Bounds checking strings
 * * Validating commands and returning appropriate function pointer
 *
 */

#include <stdlib.h>
#include "privio.h"

int privioGetConfig(const char *path, config_t *cfg){
  FILE *fp;

  fp = fopen(PRIVIO_CONFIG_PATH, "r");
  if (fp == NULL){
    fprintf(stderr, "Cannot open config file %s\n", PRIVIO_CONFIG_PATH);
    return -1;
  }

  config_init(cfg);
  if (config_read(cfg, fp) != CONFIG_TRUE){
    fprintf(stderr, "Configuration error!\n");
    fprintf(stderr, "Line %d: %s\n", config_error_line(cfg), config_error_text(cfg));
    return -1;
  }

  return 0;
}

int privioPathValidator(const char *path, config_t *conf){
  /* TODO: Store regex for valid paths in cwa_plugin_settings
   * connect via active record library, use regex to validate
   * path
   */


  
  return 0;  
}

/* Dan Bernstein's djb2... we just want to hash the command passed
 * and get minimal collisions... enough to describe the necessary
 * file operations 
 */
unsigned int cmdHash(const char *str){
  unsigned int hash = 5381;
  int c;

  while (c = *str++)
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

  return hash;
}

/* return a function pointer for the OP we want to call
 * any of the privio calls will return an int and take 
 * an array of character arrays
 */
void *getOpFromCommand(const char *cmd){
  int (*fpointer)(const char**, config_t *conf) = NULL;

  switch(cmdHash(cmd)){
/*    case CMD_WRITE:   fpointer = &privioReader; break;
    case CMD_READ:    fpointer = &privioWriter; break;
    case CMD_MKDIR:   fpointer = &privioMkdir; break;
    case CMD_RENAME:  fpointer = &privioRename; break;
    case CMD_ZIPREAD: fpointer = &privioZipreaddir; break;
    case CMD_MOVE:    fpointer = &privioMove; break;
    case CMD_LINES:   fpointer = &privioFilelines; break;
    case CMD_TYPE:    fpointer = &privioFiletype; break;
    case CMD_DLIST:   fpointer = &privioDirlist; break;
    case CMD_TAIL :   fpointer = &privioFiletail; break;*/
    default: return NULL;
  }

  return fpointer;
}
