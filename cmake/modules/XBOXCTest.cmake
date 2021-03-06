#---------------------------------------------------------------------------------------------------
#  XBOXCTest.cmake
#   - basic setup for testing XBOX using CTest
#---------------------------------------------------------------------------------------------------

#---Deduce the build name--------------------------------------------------------
set(BUILDNAME ${XBOX_ARCHTECTURE}-${CMAKE_BUILD_TYPE})

enable_testing()
include(CTest)

#---A number of operations to allow running the tests from the build directory-----------------------
set(XBOX_DIR ${CMAKE_BINARY_DIR})

#---Test products should not be poluting the standard destinations--------------------------------
unset(CMAKE_LIBRARY_OUTPUT_DIRECTORY)
unset(CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
unset(CMAKE_RUNTIME_OUTPUT_DIRECTORY)

if(WIN32)
  foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    unset(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG})
    unset(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG})
    unset(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG})
  endforeach()
endif()

#---Add all subdirectories with tests-----------------------------------------------------------

get_property(test_dirs GLOBAL PROPERTY XBOX_TEST_SUBDIRS)
foreach(d ${test_dirs})
  list(APPEND test_list ${d})
endforeach()

if(test_list)
  list(SORT test_list)
endif()

foreach(d ${test_list})
  if(d STREQUAL tutorials)
    add_subdirectory(${d} runtutorials)  # to avoid clashes with the tutorial sources copied to binary tree
  else()
    add_subdirectory(${d})
  endif()
endforeach()
