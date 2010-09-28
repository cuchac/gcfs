cmake_minimum_required(VERSION 2.6)

PROJECT(gcfs)

include_directories(include ./)

INCLUDE(FindPkgConfig)

pkg_check_modules(FUSE REQUIRED fuse)

link_libraries(${FUSE_LIBRARIES})
link_directories(${FUSE_LIBRARY_DIR})
include_directories(${FUSE_INCLUDE_DIRS})

add_definitions(-D_FILE_OFFSET_BITS=64)

add_subdirectory(lib)

ADD_EXECUTABLE(gcfs lib/module_condor/gcfs_servicecondor.cpp src/gcfs_task.cpp src/gcfs_service.cpp src/gcfs_config.cpp src/gcfs_fuse.cpp src/gcfs.cpp src/gcfs_config_values.cpp)