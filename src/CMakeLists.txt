cmake_minimum_required (VERSION 2.6)

set (PRIVIO_OBJS privioUtil.c list.c mkdir.c reader.c writer.c
lines.c mv.c rename.c type.c zip.c tail.c)
set (CMDHASH_OBJS cmdhashes.c ${PRIVIO_OBJS})

include_directories("${privio_SOURCE_DIR}/include")

add_executable (privio privio.c ${PRIVIO_OBJS})
add_executable (cmdhashes ${CMDHASH_OBJS})

target_link_libraries(privio config)
target_link_libraries(cmdhashes config)
