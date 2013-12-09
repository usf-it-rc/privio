/*
 * privio_writer(config_t *cfg)
 *
 * privio_writer() grabs a single path specification from privioReadPaths(),
 * reads block_size bytes at a time from stdin and writes the read bytes to 
 * the first path returned by privioReadPaths()
 * 
 */

#include "privio.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

int privio_writer(config_t *cfg, const char **args){
  int out_file, n;
  void *buf;
  config_setting_t *write_block_size;
  long long block_size;

  privio_debug(cfg, DBG_INFO, "Writing to %s\n", args[0]);
  out_file = open(args[0], O_CREAT|O_WRONLY|O_TRUNC, 00600);

  if (out_file == -1){
    privio_debug(cfg, DBG_VERBOSE, "Error opening file %s: %s\n", args[0], strerror(errno));
    return errno;
  }

  privio_debug(cfg, DBG_DEBUG3, "Opened %s as descriptor %d\n", args[0], out_file);

  write_block_size = config_lookup(cfg, "privio.io.writer_block_size");
  block_size = config_setting_get_int(write_block_size);

  buf = (void*)malloc(block_size);

  /* read from stdin, write to fp */
  while ((n = read(fileno(stdin), buf, block_size)) > 0){
    write(out_file, buf, n);
  }

  if (n == -1)
    privio_debug(cfg, DBG_VERBOSE, "Error writing file %s: %s\n", args[0], strerror(errno));

  privio_debug(cfg, DBG_INFO, "Closing %s\n", args[0]);

  close(out_file);
  
  return 0;
}
