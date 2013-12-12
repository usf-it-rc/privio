cmake_minimum_required (VERSION 2.6)
project (privio)
subdirs(src)
#subdirs(tests)

set (PRIVIO_VERSION_MAJOR 0)
set (PRIVIO_VERSION_MINOR 1)

configure_file (
  "${privio_SOURCE_DIR}/privioConfig.h.in"
  "${privio_SOURCE_DIR}/include/privioConfig.h"
  )

#enable_testing()
