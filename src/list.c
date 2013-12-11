/*
 * privio_list(config_t *cfg, const char **args)
 *
 * return a JSON-formatted list of file items for the specified path
 *
 */

#include "privio.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <dirent.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <string.h>

size_t dirent_buf_size(DIR *dirp);

int privio_list(config_t *cfg, const char **args){
  struct dirent *buf, *dentry;
  struct stat stat_info;
  struct passwd *pwd;
  struct group *grp;

  DIR *dp;
  char *stat_path = NULL;
  long name_max, len, item_no = 0;
  int error;

  /* TODO: Handle errors */
  dp = opendir(args[0]);
  if (dp == NULL){
    privio_debug(cfg, DBG_ERROR, "Problem reading %s: %s\n", strerror(errno));
    return -1;
  }

  len = dirent_buf_size(dp);
  buf = (struct dirent *)malloc(len);

  printf("{");
  name_max = fpathconf(dirfd(dp), _PC_NAME_MAX);

  while((error = readdir_r(dp, buf, &dentry)) == 0 && dentry != NULL){
    /* json formatting */
    if (item_no++ > 0)
      printf(",");

    stat_path = (char*)malloc((strlen(args[0]) + name_max) * sizeof(char)); 
    memset(stat_path, 0, (strlen(args[0]) + name_max) * sizeof(char));
    sprintf(stat_path, "%s/%s", args[0], dentry->d_name);

    if (stat(stat_path, &stat_info) == -1){
      privio_debug(cfg, DBG_ERROR, "Error on file stat %s: %s\n", stat_path, strerror(errno));
      return -1;
    }

    printf("'%s':{", dentry->d_name);

    switch(stat_info.st_mode & S_IFMT) {
      case S_IFBLK: printf("'type':'blockdev',"); break;
      case S_IFCHR: printf("'type':'chardev',"); break;
      case S_IFDIR: printf("'type':'dir',"); break;
      case S_IFIFO: printf("'type':'fifo',"); break;
      case S_IFLNK: printf("'type':'symlink',"); break;
      case S_IFREG: printf("'type':'file',"); break;
      case S_IFSOCK: printf("'type':'socket',"); break;
      default: printf("'type':'unknown',"); break;
    }

    printf("'mode':%lo,", stat_info.st_mode);

    grp = getgrgid(stat_info.st_gid);
    if (grp != NULL){
      printf("'group':'%s',", grp->gr_name);
    } else {
      printf("'group':%lld,", stat_info.st_gid);
    }

    pwd = getpwuid(stat_info.st_uid);
    if (pwd != NULL){
      printf("'owner':'%s',", pwd->pw_name);
    } else {
      printf("'owner':%lld,", stat_info.st_uid);
    }

    printf("'size':%lld,", (long long) stat_info.st_size);
    printf("'ctime':%lld,", stat_info.st_ctime);
    printf("'atime':%lld,", stat_info.st_atime);
    printf("'mtime':%lld", stat_info.st_mtime);

    /* free(stat_path); */
    printf("}");
    free(stat_path);
  }
  printf("}\n");

  closedir(dp);

  return 0;
}

/* Grabbed from readdir_r advisory, located here: 
 * http://womble.decadent.org.uk/readdir_r-advisory.html
 */
size_t dirent_buf_size(DIR *dirp){
  long name_max;
  size_t name_end;
# if defined(HAVE_FPATHCONF) && defined(HAVE_DIRFD) \
     && defined(_PC_NAME_MAX)
  name_max = fpathconf(dirfd(dirp), _PC_NAME_MAX);
  if (name_max == -1)
#   if defined(NAME_MAX)
    name_max = (NAME_MAX > 255) ? NAME_MAX : 255;
#   else
    return (size_t)(-1);
#   endif
# else
#   if defined(NAME_MAX)
  name_max = (NAME_MAX > 255) ? NAME_MAX : 255;
#   else
#   error "buffer size for readdir_r cannot be determined"
#   endif
# endif
  name_end = (size_t)offsetof(struct dirent, d_name) + name_max + 1;
  return (name_end > sizeof(struct dirent) ? name_end : sizeof(struct dirent));
}
