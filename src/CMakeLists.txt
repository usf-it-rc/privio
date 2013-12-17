cmake_minimum_required (VERSION 2.6)

set (PRIVIO_OBJS privioUtil.c list.c mkdir.c reader.c writer.c
lines.c cpmv.c rename.c type.c zip.c tail.c auth.c string.c)
set (CMDHASH_OBJS cmdhashes.c ${PRIVIO_OBJS})

include_directories("${privio_SOURCE_DIR}/include")

find_package(OpenMP)
if (OPENMP_FOUND)
  set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

add_executable (privio privio.c ${PRIVIO_OBJS})
add_executable (cmdhashes ${CMDHASH_OBJS})

target_link_libraries(privio config magic ssl crypto)
target_link_libraries(cmdhashes config magic ssl crypto)
