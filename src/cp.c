#include "privio.h"
#include <ftw.h>

int _copy_file(int src_fd, int dst_fd);

int privio_cp(config_t *cfg, const char **args){

  privio_debug(cfg, DBG_INFO, "This is a stub function for privio_cp\n");
  return 0;
}

int _init_src_dest(int *src_fd, int *dst_fd){
}

int _copy_file(size_t block_size, int src_fd, int dst_fd){
  size_t n;
  size_t tot_bytes;

  while ((n = read(src_fd, buf, block_size)) > 0){
    write(dst_fd, buf, n);
    tot_bytes += n;
  }
  return tot_bytes;
}
