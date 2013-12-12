#include "privio.h"
#define _XOPEN_SOURCE 500
#define __USE_XOPEN_EXTENDED 1
#include <ftw.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

int _copy_file(const char *, const char *);
int _mkdir(const char *, const char *);
static int _nftw_copy_callback(const char *, const struct stat *, int, struct FTW *);

/* declaring these globally should make them accessible to the call-back */
size_t r_block_size;
const char *dpath, *base_path;
config_t *global_cfg;

int privio_cp(config_t *cfg, const char **args){
  char *error_str; 
  config_setting_t *cfg_block_size;

  global_cfg = cfg;

  if (args[0] == NULL || args[1] == NULL){
    privio_debug(cfg, DBG_ERROR, "Not enough arguments for a copy!\n");
    return 2;
  }

  base_path = args[0];
  dpath = args[1];

  /* read size */
  cfg_block_size = config_lookup(cfg, "privio.io.reader_block_size");
  r_block_size = config_setting_get_int64(cfg_block_size);

  /* check if source is a file or directory */

  if(nftw(args[0], _nftw_copy_callback, 1, FTW_PHYS) != 0){
    error_str = strerror(errno);
    printf("{'%s':{'op':'copy','dest':'%s','error':'%s'}}\n", args[0], args[1], error_str);
    privio_debug(cfg, DBG_ERROR, "Problem traversing directory %s: %s\n", args[0], error_str);
    return -1;
  } else {
    printf("{'%s':{'op':'copy','dest':'%s','error':''}}\n", args[0], args[1]);
    return 0;
  }
}

static int _nftw_copy_callback(const char *fpath, const struct stat *sb, int tflag, struct FTW *ftwbuf){

  ssize_t new_path_len = strlen(dpath) + (strlen(fpath) - strlen(base_path));
  int i,j;
  char *new_path;

  new_path = (char*)malloc((new_path_len*sizeof(char))+1);
  new_path = (char*)memset(new_path, 0, new_path_len+1);

  /* substitute old base path with new base path */
  strncpy(new_path, dpath, strlen(dpath));

  j = strlen(dpath);
  for (i = strlen(base_path); i < strlen(fpath); i++)
    new_path[j++] = fpath[i];

  new_path[j] = '\0';

  switch(tflag){
    case FTW_D: return _mkdir(fpath, new_path); break;
    case FTW_NS: break;
    case FTW_DNR: break;
    case FTW_F: return _copy_file(fpath, new_path); break;
    case FTW_SL: break;
  }
  return 0;
}

int _mkdir(const char *src_path, const char *new_path){
  char *error_str;
  struct stat *sb;

  sb = (struct stat *)malloc(sizeof(struct stat));

  stat(src_path, sb);

  privio_debug(global_cfg, DBG_VERBOSE, "mkdir(%s,%s)\n", src_path, new_path);

  if(mkdir(new_path, sb->st_mode) != 0){
    free(sb);
    error_str = strerror(errno);
    privio_debug(global_cfg, DBG_ERROR, "mkdir(): %s\n", error_str);
    return -1;
  } else {
    free(sb);
    return 0;
  }
}

int _copy_file(const char *src_fpath, const char *dst_fpath){
  int src_fdesc, dst_fdesc;
  size_t n, i;
  void *buf;
  char *error_str;
  struct stat *sb;

  privio_debug(global_cfg, DBG_VERBOSE, "copy_file(%s,%s)\n", src_fpath, dst_fpath);

  src_fdesc = open(src_fpath, O_RDONLY);
  if (src_fdesc == -1){
    error_str = strerror(errno);
    privio_debug(global_cfg, DBG_ERROR, "Cannot open source file %s: %s\n", src_fpath, error_str);
    return -1;
  }

  sb = (struct stat*)malloc(sizeof(struct stat));
  fstat(src_fdesc, sb);

  dst_fdesc = open(dst_fpath, O_CREAT|O_WRONLY|O_TRUNC, sb->st_mode);
  if (dst_fdesc == -1){
    error_str = strerror(errno);
    privio_debug(global_cfg, DBG_ERROR, "Cannot open destination file %s: %s\n", dst_fpath, error_str);
    return -1;
  }
  
  free(sb);

  buf = (void*)malloc(r_block_size);

  while ((n = read(src_fdesc, buf, r_block_size)) > 0){
    i = write(dst_fdesc, buf, n);
    if (i == -1){
      privio_debug(global_cfg, DBG_DEBUG3, "Error writing %d bytes to %s: %s\n", n, dst_fpath, strerror(errno));
      break;
    } else {
      privio_debug(global_cfg, DBG_DEBUG3, "Wrote %d blocks to %s\n", i, dst_fpath);
    }
  }

  free(buf);

  if(close(src_fdesc) == -1){
    privio_debug(global_cfg, DBG_ERROR, "Error closing %s: %s\n", src_fpath, strerror(errno));
  }
  if(close(dst_fdesc) == -1){
    privio_debug(global_cfg, DBG_ERROR, "Error closing %s: %s\n", dst_fpath, strerror(errno));
  }

  return 0;
}