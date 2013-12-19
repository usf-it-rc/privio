/*
 * privio_reader(config_t *cfg)
 *
 * privio_reader() grabs a single path specification from privioReadPaths(),
 * reads block_size bytes at a time from the first path returned by 
 * privioReadPaths() and writes the read bytes to stdout
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

int privio_tail(config_t *cfg, const char **args){
  int in_file, n;
  size_t offset;
  void *buf;
  config_setting_t *read_block_size;
  long long block_size;

  offset = (size_t)atol(args[1]);

  privio_debug(cfg, DBG_INFO, "Tailing last %d bytes from %s\n", offset, args[0]);

  in_file = open(args[0], O_RDONLY);
  if (in_file == -1){
    privio_debug(cfg, DBG_VERBOSE, "Error opening file %s: %s\n", args[0], strerror(errno));
    return -1;
  }

  if (lseek(in_file, offset, SEEK_SET) == -1){
    privio_debug(cfg, DBG_VERBOSE, "Error seeking file %s: %s\n", args[0], strerror(errno));
    return -1;
  }

  privio_debug(cfg, DBG_DEBUG3, "Opened %s as descriptor %d\n", args[0], in_file);

  read_block_size = config_lookup(cfg, "privio.io.reader_block_size");
  block_size = config_setting_get_int(read_block_size);

  buf = (void*)malloc(block_size);



  /* read from stdin, write to fp */
  while ((n = read(in_file, buf, block_size)) > 0){
    write(fileno(stdout), buf, n);
  }

  if (n == -1)
    privio_debug(cfg, DBG_VERBOSE, "Error reading file %s: %s\n", args[0], strerror(errno));

  privio_debug(cfg, DBG_INFO, "Closing %s\n", args[0]);

  close(in_file);
  
  return 0;
}
