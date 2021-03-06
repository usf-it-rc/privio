/* 
 * privio.c
 *
 * provide low-level file I/O operations while allowing user switching
 * Copyright (C) 2013  Brian Lindblom
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * This program provides a privilege-hopping back-end for use by
 * any web application that wishes to execute priveleged file operations
 * on local filesystems (well, as long as they are mounted and provide
 * proper POSIX semantics).  It provides reasonable performance for use
 * by higher-level web applications, providing high-throughput for streaming
 * reads, writes, and acceptable metadata performance.
 *
 * An example use case is the Redmine/CWA plugin which needs access to user 
 * files.  This program is typically called setuid root by a privileged user
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
 *       - mv
 *       - lines
 *       - list
 *       - rm
 *       - chmod
 *       - chattr
 *
 * Calls return JSON-formatted output, for easy serialization by
 * web applications.  JSON structures include necessary metadata and
 * heavy error reporting.
 * 
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

  if (argc < 5){
    privio_debug(&cfg, DBG_ERROR, "Insufficient arguments.\n");
    return 3;
  }

  /* authenticate the provided user&key hash */
  if(privio_auth(&cfg, argv[2], argv[1]) != 0){
    privio_debug(&cfg, DBG_ERROR, "Invalid auth key specified!\n");
    return 5;
  } 

  /* Change to the appropriate user, following config file rules */ 
  if (privioUserSwitch(&cfg, argv[2]) != 0){
    ruid = getuid();
    switch_user = getpwuid(ruid);
    privio_debug(&cfg, DBG_ERROR, "Couldn't switch from %s to %s\n", switch_user->pw_name, argv[2]);
    return -1;
  }

  for (i = 4; i < argc; i++){
    /* We need a signed integer as a second argument for tail, so stop after the first arg */
    if ((i > 4) && cmdHash(argv[3]) == CMD_TAIL)
      break;
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
