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
#include <regex.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pwd.h>

/* Validate executing user, then setuid to the user we want to be */
int privioUserSwitch(config_t *cfg, const char *uid){
  uid_t ruid;
  struct passwd *user, *switch_user;
  int status, allowed_count, forbidden_count, i, acl_check;
  const config_setting_t *forbidden_accounts, *allowed_users;
  char *check_user;

  ruid = getuid();
  user = getpwuid(ruid);

  switch_user = getpwnam(uid);

  if (switch_user == NULL){
    privio_debug(cfg, DBG_ERROR, "User to switch, %s, does not exist!\n", uid);
    return -1; 
  }

  /* Make sure our executing user is in the allowed_users list */
  allowed_users = config_lookup(cfg, "privio.allowed_users");
  allowed_count = config_setting_length(allowed_users);

  acl_check = 0;

  for (i=0; i<allowed_count; i++){
    check_user = (char*)config_setting_get_string_elem(allowed_users, i);
    if(!strncmp(check_user, user->pw_name, strlen(user->pw_name)))
      acl_check = 1;
  }

  if (acl_check == 0){
    privio_debug(cfg, DBG_ERROR, "User %s is not allowed to execute privio!\n", user->pw_name);
    return -1;
  }

  /* Make sure our switch user is not in the forbidden_accounts list */
  forbidden_accounts = config_lookup(cfg, "privio.forbidden_accounts");
  forbidden_count = config_setting_length(forbidden_accounts);

  acl_check = 0;

  for (i=0; i<forbidden_count; i++){
    check_user = (char*)config_setting_get_string_elem(forbidden_accounts, i);
    if(!strncmp(check_user, switch_user->pw_name, strlen(switch_user->pw_name)))
      acl_check = 1;
  }

  if (acl_check != 0){
    privio_debug(cfg, DBG_ERROR, "privio is not allowed to switch to user %s!\n", switch_user->pw_name);
    return -1;
  }

  status = setuid(switch_user->pw_uid);
  if (status < 0){
    privio_debug(cfg, DBG_ERROR, "Couldn't setuid to \"%s\"!\n", switch_user->pw_name);
    return -1;
  }
  return 0;
}

/* Initialize the config_t structure from the defined PRIVIO_CONFIG_PATH */
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

/* Snag regexes from privio.allowed_paths and ensure that our operands 
 * passed on the command line match at least one of these expressions */
int privioPathValidator(config_t *cfg, char *path){

  char *pathreg;
  const config_setting_t *allowed_paths;
  regex_t *regs = NULL;
  int reg_count;
  int i,j,k,valid_path;
  
  allowed_paths = config_lookup(cfg, "privio.allowed_paths");
  if (config_setting_type(allowed_paths) != CONFIG_TYPE_ARRAY){
    privio_debug(cfg, DBG_ERROR, "privio.allowed_paths is not defined as an array!  Failed...");
    return 0;
  }

  reg_count = config_setting_length(allowed_paths);

  /* Grab and compile our regular expressions */
  for (j = 0; j < reg_count; j++){
    regs = realloc(regs, (j+1)*sizeof(regex_t));
    pathreg = (char*)config_setting_get_string_elem(allowed_paths, j);
    if(regcomp(&regs[j], pathreg, REG_EXTENDED|REG_NOSUB)){
      privio_debug(cfg, DBG_ERROR, "Invalid regular expression, %s\n", pathreg);
      return 0;
    }
  }

  /* Match against the compiled regular expressions */
  for (i = 0; i < j; i++){
    privio_debug(cfg, DBG_DEBUG3, "Compare %s to %s\n", config_setting_get_string_elem(allowed_paths, i), path);
    if(!regexec(&regs[i], path, strlen(path), NULL, 0)){
      free(regs);
      return 0;
    }
  }

  free(regs);
  return -1;  
}

/* Dan Bernstein's djb2... we just want to hash the command passed
 * and get minimal collisions... enough to describe the necessary
 * file operations */
unsigned int cmdHash(const char *str){
  unsigned int hash = 5381;
  int c;

  while (c = *str++)
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

  return hash;
}

/* return a function pointer for the OP we want to call
 * any of the privio calls will return an int and take 
 * a config_t* and a char** */
privioFunction getOpFromCommand(config_t *cfg, const char *cmd){
  int (*fpointer)(config_t *) = NULL;

  privio_debug(cfg, DBG_DEBUG3, "Getting function pointer from command argument %s\n", cmd);

  switch(cmdHash(cmd)){
    case CMD_WRITE:   fpointer = &privio_writer; break;
    case CMD_READ:    fpointer = &privio_reader; break;
    case CMD_MKDIR:   fpointer = &privio_mkdir; break;
    case CMD_RENAME:  fpointer = &privio_rename; break;
    case CMD_ZIP:     fpointer = &privio_zip; break;
    case CMD_MV:      fpointer = &privio_mv; break;
    case CMD_LINES:   fpointer = &privio_lines; break;
    case CMD_TYPE:    fpointer = &privio_type; break;
    case CMD_LIST:    fpointer = &privio_list; break;
    case CMD_TAIL :   fpointer = &privio_tail; break;
    default: privio_debug(cfg, DBG_INFO, "Invalid command specified!\n"); return NULL;
  }

  return fpointer;
}

/* Make it easy to print out debugging statements, info, errors, etc. */
void privio_debug(config_t *cfg, int dbg_level, const char *fmt, ...){
  va_list arg_list;
  const config_setting_t *debug_level_setting;
  int cfg_debug_level;

  va_start(arg_list, fmt);

  debug_level_setting = config_lookup(cfg, "privio.debug_level");
  if (config_setting_type(debug_level_setting) != CONFIG_TYPE_INT){
    fprintf(stderr, "privio.debug is not defined as an integer!  Exiting...");
  }

  cfg_debug_level = config_setting_get_int(debug_level_setting);

  if (dbg_level <= cfg_debug_level){
    fprintf(stderr, "DEBUG %d: ", dbg_level);
    vfprintf(stderr, fmt, arg_list);
  }
  va_end(arg_list);
}

char **privioReadPaths(config_t *cfg, int path_count){
  int i,j;
  char buf[8192];
  char **paths = NULL;

  memset(buf, 0, 8192);

  privio_debug(cfg, DBG_DEBUG3, "Getting required paths.\n");

  for (i = 0; i < path_count; i++){
    fgets(buf, 8192, stdin);

    for (j = 0; j <= strlen(buf); j++){
      if (buf[j] == '\n' || buf[j] == '\r')
        buf[j] = 0;
    }

    if (privioPathValidator(cfg, buf)){
      privio_debug(cfg, DBG_ERROR, "Path %s is not allowed!\n", buf);
      return NULL;
    } else {
      paths = (char**)realloc(paths,sizeof(char*));
      paths[i] = (char*)malloc(sizeof(char)*strlen(buf));
      strncpy(paths[i], buf, strlen(buf));
    }
  }

  return paths;
}
