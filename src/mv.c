#include "privio.h"
#include "privio_string.h"
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
#include <unistd.h>
#include <libgen.h>
#include <omp.h>

int _move_file(const char *, const char *);
int _mvdir(const struct stat *, const char *, const char *);
static int _nftw_move_callback(const char *, const struct stat *, int, struct FTW *);
static int _nftw_mv_size_callback(const char *, const struct stat *, int, struct FTW *);
char *_readlink_malloc(const char *);

/* declaring these globally should make them accessible to the call-back */
size_t r_block_size, dir_size, tot_written;
char *dpath, *base_path;
config_t *global_cfg;

int privio_mv(config_t *cfg, const char **args){
  char *error_str, *dst_path; 
  struct stat sb_src, sb_dst;
  config_setting_t *cfg_block_size;
  int worker_status = 0;

  global_cfg = cfg;

  if (args[0] == NULL || args[1] == NULL){
    privio_debug(cfg, DBG_ERROR, "Not enough arguments for a move!\n");
    return 2;
  }

  base_path = (char*)args[0];

  /* construct our new path destination */
  dst_path = (char *)malloc(strlen(args[1])*sizeof(char)+pathconf(args[1], _PC_NAME_MAX)+1);
  dst_path = (char *)memset(dst_path, 0, strlen(args[1])*sizeof(char)+pathconf(args[1], _PC_NAME_MAX)+1);

  strncpy(dst_path, args[1], strlen(args[1]));
  strcat(dst_path, "/");
  strcat(dst_path, basename(base_path));

  dpath = dst_path;

  stat(base_path, &sb_src);
  stat(args[1], &sb_dst);

  /* If this is true, a simple rename will suffice :) */
  if (sb_dst.st_dev == sb_src.st_dev){
    if (rename(base_path, dst_path) != 0){
      error_str = strerror(errno);
      printf("{'%s':{'new_name':'','error':'%s'}}\n", base_path, error_str);
      privio_debug(cfg, DBG_ERROR, "rename('%s','%s'): %s\n", base_path, dst_path, error_str);
      return -1;
    } else {
      printf("{'%s':{'new_name':'%s','error':''}}\n", base_path, dst_path);
    }
    return 0;
  }

  /* read size */
  cfg_block_size = config_lookup(cfg, "privio.io.reader_block_size");
  r_block_size = config_setting_get_int64(cfg_block_size);

  /* let's get the total size of the source directory */
  dir_size = 0;
  tot_written = 0;

#pragma omp parallel sections shared(tot_written, worker_status)
  {

#pragma omp section
    {
      if(nftw(args[0], _nftw_mv_size_callback, 1, FTW_PHYS) != 0){
        error_str = strerror(errno);
        privio_debug(cfg, DBG_ERROR, "Problem traversing directory %s: %s\n", args[0], error_str);
        worker_status = 1;
      }

      if (worker_status == 0){
        if((nftw(args[0], _nftw_move_callback, 1, FTW_PHYS|FTW_DEPTH) != 0)){
          error_str = strerror(errno);
          privio_debug(cfg, DBG_ERROR, "Problem traversing directory %s: %s\n", args[0], error_str);
        }
      }
      worker_status = 1;
    }
#pragma omp section
    {
      while (1){
        if (worker_status == 0)
          printf("{'%s':{'written':%lld}}\n", dpath, tot_written);
        else
          break;

        sleep(1);
      }
    }
  }
}

static int _nftw_mv_size_callback(const char *fpath, const struct stat *sb, int tflag, struct FTW *ftwbuf){
  char *error_str, *npath;

  /* we need to create our directory tree when walking forward */
  if(sb->st_mode & S_IFDIR){

    npath = strsr(fpath, base_path, dpath);

    if(mkdir(npath, sb->st_mode) == -1){
      error_str = strerror(errno);
      privio_debug(global_cfg, DBG_ERROR, "Couldn't make destination directory %s: %s\n", npath, error_str);
      printf("{'%s':{'mkdstdir':'','error':'%s'}}\n", npath, error_str);
      free(npath);
      return -1;
    }
    free(npath);
  }

  dir_size += sb->st_size;
  tot_written += sb->st_size;
  return 0;
}

static int _nftw_move_callback(const char *fpath, const struct stat *sb, int tflag, struct FTW *ftwbuf){
  char *new_path;
  int retval = 0;

  new_path = strsr(fpath, base_path, dpath);

  switch(tflag){
    case FTW_DP: retval = _unlink_dir(fpath); break;
    case FTW_F: retval = _move_file(fpath, new_path); break;
    case FTW_SL: retval = _link_file(fpath, new_path); break;
  }

  free(new_path);
  return retval;
}

int _unlink_dir(const char *fpath){
  privio_debug(global_cfg, DBG_INFO, "unlink(%s)\n", fpath);
  return rmdir(fpath);
}

int _link_file(const char *src_fpath, const char * dst_fpath){
  int ret_rl, ret_sl, ret_ul;
  char *buf;

  /* readlink src_fpath to get link destination */
  if ((buf = _readlink_malloc(src_fpath)) == NULL){
    privio_debug(global_cfg, DBG_ERROR, "Problem reading link information for %s\n", src_fpath);
    return -1;
  }

  /* create symlink */
  if (symlink(buf, dst_fpath) != 0){
    privio_debug(global_cfg, DBG_ERROR, "Problem creating symlink %s: %s\n", dst_fpath, strerror(errno));
    return -1;
  }

  free(buf);

  /* remove old symlink */
  if (unlink(src_fpath) != 0){
    privio_debug(global_cfg, DBG_ERROR, "Problem removing symlink %s: %s\n", src_fpath, strerror(errno));
    return -1;
  }
}

int _move_file(const char *src_fpath, const char *dst_fpath){
  int src_fdesc, dst_fdesc;
  size_t n, i, tot_bytes = 0;
  void *buf;
  char *error_str;
  struct stat *sb;

  privio_debug(global_cfg, DBG_VERBOSE, "move_file(%s,%s)\n", src_fpath, dst_fpath);

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
    tot_bytes += i;
  }

  free(buf);

  if(close(src_fdesc) == -1){
    privio_debug(global_cfg, DBG_ERROR, "Error closing %s: %s\n", src_fpath, strerror(errno));
  }
  if(close(dst_fdesc) == -1){
    privio_debug(global_cfg, DBG_ERROR, "Error closing %s: %s\n", dst_fpath, strerror(errno));
  }

  if(unlink(src_fpath) == -1){
    privio_debug(global_cfg, DBG_ERROR, "Error unlinking %s: %s\n", src_fpath, strerror(errno));
    return -1;
  }

  tot_written += tot_bytes;

  return 0;
}

/* TODO: This code needs to be fixed... the returned string is not null-terminated */
char *_readlink_malloc (const char *filename){
  int size = 100;
  char *buffer = NULL;
          
  while (1){
    buffer = (char *) realloc(buffer, size);
    int nchars = readlink (filename, buffer, size);

    if (nchars < 0){
      free (buffer);
      return NULL;
    }
    if (nchars < size)
      return buffer;

    size *= 2;
  }
}
