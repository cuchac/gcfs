cmake_minimum_required(VERSION 2.6)

PROJECT(gcfs)

include_directories(include ./)

SET(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Find FUSE libray
INCLUDE(FindPkgConfig RESULT_VARIABLE GCFS_PKGCONFIG)

IF(NOT GCFS_PKGCONFIG)
	pkg_check_modules(FUSE REQUIRED fuse)

	link_libraries(${FUSE_LIBRARIES})
	link_directories(${FUSE_LIBRARY_DIR})
	include_directories(${FUSE_INCLUDE_DIRS})
ELSE(NOT GCFS_PKGCONFIG)
	# Pkg-config not found, try FindFuse
	find_package(FUSE REQUIRED)

	link_libraries(${FUSE_LIBRARIES})
	include_directories(${FUSE_INCLUDE_DIR})
	
ENDIF(NOT GCFS_PKGCONFIG)

# Necesary for FUSE to compile
add_definitions(-D_FILE_OFFSET_BITS=64)

# Create main executable
ADD_EXECUTABLE(gcfs src/gcfs_controls.cpp src/gcfs_task.cpp src/gcfs_service.cpp src/gcfs_config.cpp src/gcfs_fuse.cpp src/gcfs.cpp src/gcfs_config_values.cpp)

# Add modules
add_subdirectory(lib)

# Append selected modules as dependency and link app with them
get_target_property(GCFS_MODULES gcfs MODULES)

foreach(module ${GCFS_MODULES})
	add_dependencies(gcfs ${module})
	target_link_libraries(gcfs ${module})
endforeach(module)
	
# Generate configuraton header with selected modules
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/include/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/config.h)
include_directories(include ${CMAKE_CURRENT_BINARY_DIR}) # Include also build directory to allow #inlude of config.h

set( CMAKE_VERBOSE_MAKEFILE 1 )
