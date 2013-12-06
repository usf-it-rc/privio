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
 *     - Open3 -> sudo -u <authenticated_user> cwaiohelper
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

int main(int argc, char *argv[]){

  config_t cfg;
  privioArgs arguments;
  int i, j = 0;
  privioFunction f2call = NULL;
  static uid_t ruid;
  struct passwd *user, *switch_user;

  if(privioGetConfig(&cfg) != 0){
    fprintf(stderr, "Problem reading configuration!\n");
    return -10;
  }
  
  privio_debug(&cfg, DBG_VERBOSE, "Successfully read configuration!\n");

  /* Parse arguments to determine command requested and 
     path arguments to said command. 

     Path arguments should be separated by a double hyphen --
     so that we can easily handle paths and items that 
     contain spaces
  */
  for (i=3; i < argc; i++){
    if (!strcmp(argv[i], "--")){
      if (j >= 7){
        fprintf(stderr, "Too many arguments passed.");
        return -1;
      }
      j++;
    } else {
      strcat(arguments[j], argv[i]);
      if (i < argc-1)
        strcat(arguments[j], " ");
    }
  }

  /* Validate path arguments */
  if (!privioPathValidator(&cfg, arguments, j+1)){
    privio_debug(&cfg, DBG_ERROR, "Invalid path argument specified!\n");
    return -2;
  }

  /* Change to the appropriate user, following config file rules */ 
  if (privioUserSwitch(&cfg, argv[2]) != 0){
    ruid = getuid();
    switch_user = getpwuid(ruid);
    privio_debug(&cfg, DBG_ERROR, "Couldn't switch from %s to %s\n", switch_user->pw_name, argv[2]);
    return -1;
  }

  /* Get function call based on cmd passed in command args */
  f2call = getOpFromCommand(&cfg, argv[1]);

  /* Call it! */
  if (f2call != NULL)
    return (*f2call)(&cfg, &arguments);
  else
    return -1;
}

/*
case "$cmd" in
  write)  dd bs=$((1024*128)) of="${argv[0]}" ;;
  read)   dd bs=$((1024*128)) if="${argv[0]}" ;;
  zip)    cd ${argv[0]}; zip -q -r - "${argv[1]}" ;;
  rm)     rm -r "${argv[0]}" ;;
  rename) mv "${argv[0]}" "${argv[1]}" ;;
  cp)     count=$(find "${argv[0]}" | wc -l)
          rsync -ai "${argv[0]}" "${argv[1]}" 2>&1 | pv -nls $(((count)+4)) > /dev/null
    ;;  
  mkdir)  mkdir "${argv[0]}" ;;
  mv)     count=$(find "${argv[0]}" | wc -l)
          rsync -ai --remove-source-files "${argv[0]}" "${argv[1]}" 2>&1 | pv -nls $(((count)+4)) > /dev/null && ([ -d "${argv[0]}"] && rmdir -r "${argv[0]}")
    ;;  
  stat)   stat -c "%s" "${argv[0]}" ;;
  mkdir)  mkdir "${argv[0]}" ;;
  lines)  wc -l "${argv[0]}" | awk '{ print $1 }' ;;
  type)   file -bi "${argv[0]}" ;;
  list)   find "${argv[0]}" ! -path "${argv[0]}" ! -type l ! -iname '.*' -maxdepth 1 -type ${argv[1]} -printf "%f::%s::%u::%g::%m::%CD %Cr %CZ\n" | sort -f ;;
  *) echo "No way, Jose!"; exit 1 ;;
esac */
