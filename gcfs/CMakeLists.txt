cmake_minimum_required(VERSION 2.8)

PROJECT(gcfs)

include_directories(include)

INCLUDE(FindPkgConfig)
cmake_policy(SET CMP0015 NEW)

pkg_check_modules(FUSE REQUIRED fuse)

link_libraries(${FUSE_LIBRARIES})
link_directories(${FUSE_LIBRARY_DIR})
include_directories(${FUSE_INCLUDE_DIRS})

add_definitions(-D_FILE_OFFSET_BITS=64)

ADD_EXECUTABLE(gcfs src/fuse.c src/main.c)

