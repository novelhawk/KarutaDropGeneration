# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindWebP
-------

Finds the WebP library.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``WEBP_FOUND``
  True if the system has the WebP library.
``WEBP_INCLUDE_DIRS``
  Include directories needed to use WebP.
``WEBP_LIBRARIES``
  Libraries needed to link to WebP.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``WEBP_INCLUDE_DIR``
  The directory containing ``decode.h``.
``WEBP_LIBRARY``
  The path to the WebP library.
``WEBP_NAMES``
  The names of the WebP library.

#]=======================================================================]

include(FindZLIB)

find_path(WEBP_INCLUDE_DIR decode.h
    HINTS ENV WEBP_DIR
    PATH_SUFFIXES include/webp include webp
    PATHS
    /usr
    /usr/local
    ~/Library/Frameworks
    /Library/Frameworks)

set(WEBP_NAMES ${WEBP_NAMES} webp)    
find_library(WEBP_LIBRARY
    NAMES ${WEBP_NAMES}
    HINTS ENV WEBP_DIR
    PATH_SUFFIXES lib
    PATHS
    /usr/local
    /usr
    ~/Library/Frameworks
    /Library/Frameworks)

if (WEBP_LIBRARY AND WEBP_INCLUDE_DIR)
  set(WEBP_INCLUDE_DIRS "${WEBP_INCLUDE_DIR}")
  set(WEBP_LIBRARIES "${WEBP_LIBRARY}")
  set(WEBP_FOUND "YES")
endif (WEBP_LIBRARY AND WEBP_INCLUDE_DIR)

if (WEBP_FOUND)
  if (NOT WEBP_FIND_QUIETLY)
    message(STATUS "Found WEBP: ${WEBP_LIBRARIES}")
  endif (NOT WEBP_FIND_QUIETLY)
else (WEBP_FOUND)
  if (WEBP_FIND_REQUIRED)
    message(FATAL_ERROR "Could not find WEBP library")
  endif (WEBP_FIND_REQUIRED)
endif (WEBP_FOUND)

mark_as_advanced(WEBP_INCLUDE_DIR WEBP_LIBRARY WEBP_NAMES)
