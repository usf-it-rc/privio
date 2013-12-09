/*
 * privio_lines(config_t *cfg)
 *
 * privio_lines() grabs a single path specification from privioReadPaths(),
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

int privio_lines(config_t *cfg, const char **args){
  int in_file, n, k;
  char *buf;
  config_setting_t *read_block_size;
  long long block_size, lines = 0;

  privio_debug(cfg, DBG_INFO, "Reading from %s\n", args[0]);
  in_file = open(args[0], O_RDONLY);

  if (in_file == -1){
    privio_debug(cfg, DBG_VERBOSE, "Error opening file %s: %s\n", args[0], strerror(errno));
    return errno;
  }

  privio_debug(cfg, DBG_DEBUG3, "Opened %s as descriptor %d\n", args[0], in_file);

  read_block_size = config_lookup(cfg, "privio.io.reader_block_size");
  block_size = config_setting_get_int(read_block_size);

  buf = (void*)malloc(block_size);

  /* read from stdin, write to fp */
  while ((n = read(in_file, buf, block_size)) > 0)
    for (k = 0; k < n; k++)
      if (buf[k] == '\n')
        lines++;

  printf("%ld\n", lines);

  if (n == -1)
    privio_debug(cfg, DBG_VERBOSE, "Error reading file %s: %s\n", args[0], strerror(errno));

  privio_debug(cfg, DBG_INFO, "Closing %s\n", args[0]);

  close(in_file);
  
  return 0;
}
