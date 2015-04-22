#
#  Copyright 2009-2015, Jack Poulson
#  All rights reserved.
#
#  This file is part of Elemental and is under the BSD 2-Clause License,
#  which can be found in the LICENSE file in the root directory, or at
#  http://opensource.org/licenses/BSD-2-Clause
#
include(ExternalProject)

if(NOT BUILD_METIS)
  find_package(METIS)
endif()

if(METIS_FOUND)
  # find_package returns 'METIS_LIBRARIES' but METIS's CMakeLists.txt
  # returns 'METIS_LIBS'
  set(METIS_LIBS ${METIS_LIBRARIES})
else()
  if(NOT DEFINED METIS_URL)
    set(METIS_URL https://github.com/poulson/Metis.git)
  endif()
  message(STATUS "Will pull METIS from ${METIS_URL}")

  set(METIS_SOURCE_DIR ${PROJECT_BINARY_DIR}/download/metis/source)
  set(METIS_BINARY_DIR ${PROJECT_BINARY_DIR}/download/metis/build)

  ExternalProject_Add(project_metis 
    PREFIX ${CMAKE_INSTALL_PREFIX}
    GIT_REPOSITORY ${METIS_URL}
    STAMP_DIR  ${METIS_BINARY_DIR}/stamp
    SOURCE_DIR ${METIS_SOURCE_DIR}
    BINARY_DIR ${METIS_BINARY_DIR}
    TMP_DIR    ${METIS_BINARY_DIR}/tmp
    CMAKE_ARGS -D CMAKE_C_COMPILER=${CMAKE_C_COMPILER}
               -D CMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
               -D BUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}
    INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
    INSTALL_COMMAND ""
    UPDATE_COMMAND "" 
  )
  add_dependencies(External project_metis)

  # Extract the installation directory
  ExternalProject_Get_Property(project_metis install_dir)

  # Add a target for libmetis (either shared or static)
  if(BUILD_SHARED_LIBS)
    add_library(libmetis SHARED IMPORTED)
    set_property(TARGET libmetis PROPERTY IMPORTED_LOCATION ${install_dir}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}metis${CMAKE_SHARED_LIBRARY_SUFFIX})
  else()
    add_library(libmetis STATIC IMPORTED)
    set_property(TARGET libmetis PROPERTY IMPORTED_LOCATION ${install_dir}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}metis${CMAKE_STATIC_LIBRARY_SUFFIX})
  endif() 

  set(METIS_LIBS libmetis)
  set(EL_BUILT_METIS TRUE)
endif()

set(EXTERNAL_LIBS ${EXTERNAL_LIBS} ${METIS_LIBS})

set(EL_HAVE_METIS TRUE)