cmake_minimum_required (VERSION 2.6)
project (privio)

set (PRIVIO_VERSION_MAJOR 0)
set (PRIVIO_VERSION_MINOR 1)

configure_file (
  "${PROJECT_SOURCE_DIR}/privioConfig.h.in"
  "${PROJECT_BINARY_DIR}/include/privioConfig.h"
  )

include_directories("${PROJECT_BINARY_DIR}/include")

add_executable (bin/privio src/privio.c src/privioUtil.c)
add_executable (bin/cmdhashes util/cmdhashes.c src/privioUtil.c)
target_link_libraries(bin/privio config)
target_link_libraries(bin/cmdhashes config)
