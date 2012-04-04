#---------------------------------------------------------------------------------------------------
#
#  Copyright (C) 2010  Artem Rodygin
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#---------------------------------------------------------------------------------------------------
#
#  This module finds if C API of SAGA library is installed and determines where required
#  include files and libraries are. The module sets the following variables:
#
#    SAGA_FOUND         - system has SAGA
#    SAGA_INCLUDE_DIR   - the SAGA include directory
#    SAGA_LIBRARIES     - the libraries needed to use SAGA
#    SAGA_DEFINITIONS   - the compiler definitions, required for building with SAGA
#    SAGA_VERSION_MAJOR - the major version of the SAGA release
#    SAGA_VERSION_MINOR - the minor version of the SAGA release
#
#  You can help the module to find SAGA by specifying its root path
#  in environment variable named "SAGA_ROOTDIR". If this variable is not set
#  then module will search for files in "/usr/local" and "/usr" by default.
#
#---------------------------------------------------------------------------------------------------

set(SAGA_FOUND TRUE)

# search for header

find_path(SAGA_INCLUDE_DIR
          NAMES "saga/saga.hpp"
                "saga/saga-config.hpp"
          PATHS "/usr/local"
                "/usr"
          ENV SAGA_ROOTDIR
          PATH_SUFFIXES "include")

# header is found

if (SAGA_INCLUDE_DIR)

    # retrieve version information from the header

    file(READ "${SAGA_INCLUDE_DIR}/saga/saga-config.hpp" SAGA_COMMON_H_FILE)

    string(REGEX REPLACE ".*#define[ \t]+SAGA_VERSION_MAJOR[ \t]+([0-9]+).*" "\\1" SAGA_VERSION_MAJOR "${SAGA_COMMON_H_FILE}")
    string(REGEX REPLACE ".*#define[ \t]+SAGA_VERSION_MINOR[ \t]+([0-9]+).*" "\\1" SAGA_VERSION_MINOR "${SAGA_COMMON_H_FILE}")

    set(SAGA_LOCATION ${SAGA_INCLUDE_DIR}/../)
    set(SAGA_INCLUDE_DIR ${SAGA_INCLUDE_DIR}/saga/ ${SAGA_INCLUDE_DIR}/)

    # search for library

    find_library(SAGA_LIBRARIES_CORE
                 NAMES "libsaga_core.so"
                       "libsaga_core.dylib"
                       "libsaga_engine.so"
                       "libsaga_engine.dylib"
                 PATHS "/usr/local"
                       "/usr"
                 ENV SAGA_ROOTDIR
                 PATH_SUFFIXES "lib")
	 
    find_library(SAGA_LIBRARIES_JOB
                 NAMES "libsaga_package_job.so"
                       "libsaga_package_job.dylib"
                 PATHS "/usr/local"
                       "/usr"
                 ENV SAGA_ROOTDIR
                 PATH_SUFFIXES "lib")

    GET_FILENAME_COMPONENT(SAGA_LIBRARIES_DIR ${SAGA_LIBRARIES_CORE} PATH)	
    SET(SAGA_LIBRARIES ${SAGA_LIBRARIES_CORE} ${SAGA_LIBRARIES_JOB})

endif (SAGA_INCLUDE_DIR)

# header is not found

if (NOT SAGA_INCLUDE_DIR)
    set(SAGA_FOUND FALSE)
endif (NOT SAGA_INCLUDE_DIR)

# library is not found

if (NOT SAGA_LIBRARIES)
    set(SAGA_FOUND FALSE)
endif (NOT SAGA_LIBRARIES)

# set default error message

if (SAGA_FIND_VERSION)
    set(SAGA_ERROR_MESSAGE "Unable to find SAGA library v${SAGA_FIND_VERSION}")
else (SAGA_FIND_VERSION)
    set(SAGA_ERROR_MESSAGE "Unable to find SAGA library")
endif (SAGA_FIND_VERSION)

# check found version

if (SAGA_FIND_VERSION AND SAGA_FOUND)

    set(SAGA_FOUND_VERSION "${SAGA_VERSION_MAJOR}.${SAGA_VERSION_MINOR}")

    if (SAGA_FIND_VERSION_EXACT)
        if (NOT ${SAGA_FOUND_VERSION} VERSION_EQUAL ${SAGA_FIND_VERSION})
            set(SAGA_FOUND FALSE)
        endif (NOT ${SAGA_FOUND_VERSION} VERSION_EQUAL ${SAGA_FIND_VERSION})
    else (SAGA_FIND_VERSION_EXACT)
        if (${SAGA_FOUND_VERSION} VERSION_LESS ${SAGA_FIND_VERSION})
            set(SAGA_FOUND FALSE)
        endif (${SAGA_FOUND_VERSION} VERSION_LESS ${SAGA_FIND_VERSION})
    endif (SAGA_FIND_VERSION_EXACT)

    if (NOT SAGA_FOUND)
        set(SAGA_ERROR_MESSAGE "Unable to find SAGA library v${SAGA_FIND_VERSION} (${SAGA_FOUND_VERSION} was found)")
    endif (NOT SAGA_FOUND)

endif (SAGA_FIND_VERSION AND SAGA_FOUND)

# add definitions

if (SAGA_FOUND)
        set(SAGA_DEFINITIONS "")
endif (SAGA_FOUND)

# final status messages

if (SAGA_FOUND)

    if (NOT SAGA_FIND_QUIETLY)
        message(STATUS "SAGA ${SAGA_VERSION_MAJOR}.${SAGA_VERSION_MINOR}")
    endif (NOT SAGA_FIND_QUIETLY)

    mark_as_advanced(SAGA_INCLUDE_DIR
                     SAGA_LIBRARIES
                     SAGA_DEFINITIONS)

else (SAGA_FOUND)

    if (SAGA_FIND_REQUIRED)
        message(SEND_ERROR "${SAGA_ERROR_MESSAGE}")
    endif (SAGA_FIND_REQUIRED)

endif (SAGA_FOUND)
