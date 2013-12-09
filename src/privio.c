/* 
 * privio.c
 *
 * This program provides a privilege-hopping back-end for use by
 * any web application that wishes to execute priveleged file operations
 * on local filesystems (well, as long as they are mounted and provide
 * proper POSIX semantics.  It provides reasonable performance for use 
 * by upper-level web applications, providing high-throughput for streaming
 * reads, writes, and acceptable metadata performance.
 *
 * An example use case is the Redmine/CWA plugin which needs access to user 
 * files.  This program is typically called with sudo by a privileged user
 * (in the case of Redmine/CWA, typically 'redmine') allowing it to hop to
 * other user accounts based on authentication done by a higher-level web
 * application.  Here is the layout w/ Redmine/CWA:
 *
 * Redmine/CWA
 *
 * - cwa_browser # cwa_ipa_authorize <- ensures mapping to POSIX user, and 
 *                                      provides RESTful API for typical 
 *                                      filesystem access patterns
 *   - cwa_browser_helper
 *     - Open3 -> /path/to/privio <user> <operation>
 *       - read
 *       - write
 *       - mkdir
 *       - tail
 *       - type
 *       - zipreaddir
 *       - move
 *       - lines
 *       - list
 *       - chmod
 */

#include "privio.h"
#include <sys/types.h>
#include <pwd.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[]){

  config_t cfg;
  config_setting_t *secret_key;
  const char *secret_key_val;
  char **arguments = NULL;
  int i, j, last_j;
  privioFunction f2call = NULL;
  static uid_t ruid;
  struct passwd *user, *switch_user;

  if(privioGetConfig(&cfg) != 0){
    fprintf(stderr, "Problem reading configuration!\n");
    return -10;
  }
  
  privio_debug(&cfg, DBG_VERBOSE, "Successfully read configuration!\n");

  secret_key = config_lookup(&cfg, "privio.secret_key");
  secret_key_val = config_setting_get_string(secret_key);

  if(strncmp(secret_key_val, argv[1], strlen(secret_key_val))){
    privio_debug(&cfg, DBG_ERROR, "Invalid secret key specified!\n");
    return 1;
  } 

  /* Change to the appropriate user, following config file rules */ 
  if (privioUserSwitch(&cfg, argv[2]) != 0){
    ruid = getuid();
    switch_user = getpwuid(ruid);
    privio_debug(&cfg, DBG_ERROR, "Couldn't switch from %s to %s\n", switch_user->pw_name, argv[2]);
    return -1;
  }

  for (i = 4; i < argc; i++){
    if(privioPathValidator(&cfg, argv[i])){
      privio_debug(&cfg, DBG_ERROR, "Path %s is not allowed!\n", argv[i]);
      return 2;
    }
  }

  /* Get function call based on cmd passed in command args */
  f2call = getOpFromCommand(&cfg, argv[3]);

  /* Call it! */
  if (f2call != NULL)
    return (*f2call)(&cfg, (const char**)&argv[4]);
  else
    return -1;
}
