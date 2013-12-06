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

#include "privio.h"

int privioGetConfig(config_t *cfg){
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

int privioPathValidator(config_t *cfg, privioArgs paths, int path_count){
  /* TODO: Store regex for valid paths in cwa_plugin_settings
   * connect via active record library, use regex to validate
   * path
   */

  char *pathreg;
  const config_setting_t *allowed_paths;
  regex_t *regs = NULL;
  int reg_count;
  int i,j,k;
  
  allowed_paths = config_lookup(cfg, "privio.allowed_paths");
  if (config_setting_type(allowed_paths) != CONFIG_TYPE_ARRAY){
    fprintf(stderr, "privio.allowed_paths is not defined as an array!  Failed...");
    return 0;
  }

  reg_count = config_setting_length(allowed_paths);

  /* Grab and compile our regular expressions */
  for (j = 0; j < reg_count; j++){
    regs = realloc(regs, (j+1)*sizeof(regex_t));
    pathreg = (char*)config_setting_get_string_elem(allowed_paths, j);
    if(regcomp(&regs[j], pathreg, REG_EXTENDED|REG_NOSUB)){
      fprintf(stderr, "Invalid regular expression, %s\n", pathreg);
      return 0;
    }
  }

  /* Match against the compiled regular expressions */
  for (i = 0; i < j; i++){
    for (k = 0; k <= path_count; k++){
      fprintf(stderr, "Compare %s to %s\n", config_setting_get_string_elem(allowed_paths, i), paths[k]);
      if(!regexec(&regs[i], paths[k], strlen(paths[k]), NULL, 0))
        return 1;
    }
  }

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
void *getOpFromCommand(config_t *cfg, const char *cmd){
  int (*fpointer)(config_t *, const char**) = NULL;

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

void privio_debug(config_t *cfg, int dbg_level, const char *fmt, ...){
  va_list arg_list;
  const config_setting_t *debug_level_setting;
  /*const config_setting_t *log_file;*/
  int cfg_debug_level;
  /*char *path;*/

  va_start(arg_list, fmt);

  debug_level_setting = config_lookup(cfg, "privio.debug_level");
  if (config_setting_type(debug_level_setting) != CONFIG_TYPE_INT){
    fprintf(stderr, "privio.debug is not defined as an integer!  Exiting...");
  }

  cfg_debug_level = config_setting_get_int(debug_level_setting);

  if (dbg_level > cfg_debug_level){
    vfprintf(stderr, fmt, arg_list);
  }
}
