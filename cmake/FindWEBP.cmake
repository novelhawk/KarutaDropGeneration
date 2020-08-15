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

#]=======================================================================]

FIND_PATH(WEBP_INCLUDE_DIR decode.h
    HINTS
    ENV WEBP_DIR
    PATH_SUFFIXES include/webp include webp
    PATHS
    /usr
    /usr/local
    ~/Library/Frameworks
    /Library/Frameworks)

FIND_LIBRARY(WEBP_LIBRARY
    NAMES webp libwebp
    HINTS
    ENV WEBP_DIR
    PATH_SUFFIXES lib
    PATHS
    /usr/local
    /usr
    ~/Library/Frameworks
    /Library/Frameworks)

set(WEBP_INCLUDE_DIRS "${WEBP_INCLUDE_DIR}")
set(WEBP_LIBRARIES "${WEBP_LIBRARY}")

MARK_AS_ADVANCED(WEBP_INCLUDE_DIR WEBP_LIBRARIES WEBP_LIBRARY)