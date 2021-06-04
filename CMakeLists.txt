cmake_minimum_required (VERSION 3.4.3)

#---preserve backward compatible behavior across multiple releases-----------------------------
set(policy_new 
  CMP0068
#  CMP0040 
#  CMP0053 
#  CMP0054
#  CMP0075
)
foreach(policy ${policy_new})
  if(POLICY ${policy})
    cmake_policy(SET ${policy} NEW)
  endif()
endforeach()


#---capture command line------------------------------------------------------------------------
include(cmake/modules/CaptureCommandLine.cmake)

#---set name of the project to "XBOX". Has to be done after check of cmake version--------------
project(XBOX)

#---set the locale to default C to prevent issued due to localization of commands---------------
# This is necessary as we for example call `clang -v` and parse its output. But on a localized
# program, the output parsing is much more error prone as certrain strings we're looking for
# could be missing or be in a different order. To prevent those errors, let's just force all
# output to use the default C locale which is more or less identical on all systems.
set(ENV{LANG} C)

#---set pathes where to put the libraries, executables and headers------------------------------
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(XBOXSYS ${CMAKE_BINARY_DIR})
set(HEADER_OUTPUT_PATH ${CMAKE_BINARY_DIR}/include)
set(XBOX_INCLUDE_DIR ${HEADER_OUTPUT_PATH})

#---set the project version --------------------------------------------------------------------
message("-- Looking for project version")
file(READ ${CMAKE_SOURCE_DIR}/build/version_number versionstr)
string(STRIP ${versionstr} versionstr)
string(REGEX REPLACE "([0-9]+)[.][0-9]+[.][0-9]+" "\\1" XBOX_MAJOR_VERSION ${versionstr})
string(REGEX REPLACE "[0-9]+[.]([0-9]+)[.][0-9]+" "\\1" XBOX_MINOR_VERSION ${versionstr})
string(REGEX REPLACE "[0-9]+[.][0-9]+[.]([0-9]+)" "\\1" XBOX_PATCH_VERSION ${versionstr})
set(XBOX_VERSION "${XBOX_MAJOR_VERSION}.${XBOX_MINOR_VERSION}.${XBOX_PATCH_VERSION}")
message("-- Looking for project version - found " ${XBOX_VERSION})

#---set cmake module path to look first before ${CMAKE_ROOT}/Modules/ is checked----------------
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")

#---Enable Folders in IDE like Visual Studio----------------------------------------------------
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

#---Load some basic macros which are needed later for the confiuration and build----------------
include(XBOXBuildOptions)
include(XBOXMacros)
include(CheckCompiler)
include(MacroEnsureVersion)
#include(FairMacros)

#---Enable CCache ------------------------------------------------------------------------------
if(ccache)
   set(CMAKE_IGNORE_PATH_TMP ${CMAKE_IGNORE_PATH})
   unset(CMAKE_IGNORE_PATH)

   find_program(ccache_cmd NAMES ccache ccache-swig)
   mark_as_advanced(ccache_cmd ${ccache_cmd})

   set(CMAKE_IGNORE_PATH ${CMAKE_IGNORE_PATH_TMP})

   if(ccache_cmd)
      message(STATUS "Using ccache for building")
      set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
   else()
      message(STATUS "Could NOT find ccache")
   endif()
endif()


#--- Enable build timing -----------------------------------------------------------------------
if (build_timing)
  # FIXME: This currently will override the use of ccache if -Dbuild_timing=On -Dccache=On is passed.
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CMAKE_COMMAND} -E time")
  set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK "${CMAKE_COMMAND} -E time")
  #set_property(GLOBAL PROPERTY RULE_LAUNCH_CUSTOM "${CMAKE_COMMAND} -E time") 
endif()

#---Enable CTest package -----------------------------------------------------------------------
include(CTest)
if(testing)
  enable_testing()
endif()

#---ensure that the user wants to build the project outside the source directory----------------
XBOX_CHECK_OUT_OF_SOURCE_BUILD()

#---look for installed software and switch on and of the different build options----------------
include(SearchInstalledSoftware)
XBOX_SHOW_OPTIONS()

#---add tcmalloc to the linker flags if needed--------------------------------------------------
if (TCMALLOC_FOUND)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -ltcmalloc -L ${TCMALLOC_LIBRARY_PATH}")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -ltcmalloc -L ${TCMALLOC_LIBRARY_PATH}")
endif()

#---add jemalloc to the linker flags if needed--------------------------------------------------
if (JEMALLOC_FOUND)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -ljemalloc -L ${JEMALLOC_LIBRARY_PATH}")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -ljemalloc -L ${JEMALLOC_LIBRARY_PATH}")
endif()

#---populate the configure arguments returned by 'xbox-config --config'-------------------------
get_cmake_property(variables CACHE_VARIABLES)
foreach(var ${variables})
  if((var MATCHES "_(LIBRARIES|LIBRARY|INCLUDE)") AND
     (NOT ${${var}} STREQUAL "") AND
     (NOT ${var} MATCHES "NOTFOUND"))
    if (var MATCHES "^QT_")
      # filter out the very long list of Qt libraries and include dirs
      if (var MATCHES "(QT_LIBRARY_DIR|QT_QTCORE_INCLUDE_DIR)")
        set(XBOX_CONFIGARGS "${XBOX_CONFIGARGS}${var}=${${var}} ")
      endif()
    else()
      if ((NOT var MATCHES "_(DOCS|TESTS|INSTALL)") AND (NOT var MATCHES "^_"))
        set(XBOX_CONFIGARGS "${XBOX_CONFIGARGS}${var}=${${var}} ")
      endif()
    endif()
  endif()
endforeach()

#---copy directories to binary tree-----------------------------------------------------------
set(stamp_file ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/move_artifacts.stamp)
add_custom_command(OUTPUT ${stamp_file}
                  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/macros ${CMAKE_BINARY_DIR}/macros
                  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/notebooks ${CMAKE_BINARY_DIR}/notebooks
                  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/python ${CMAKE_BINARY_DIR}/python
                  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/LICENSE ${CMAKE_BINARY_DIR}/LICENSE
                  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/README.md ${CMAKE_BINARY_DIR}/README.md
                  COMMAND ${CMAKE_COMMAND} -E touch ${stamp_file}
                  COMMENT "Copying directories such as etc, icons, fonts, etc. to build area")


#---Copy the macros to the build area, with dependency---------------------------------
file(GLOB_RECURSE macro_files RELATIVE ${CMAKE_SOURCE_DIR} macros/*)
set(macros_files_builddir)
foreach(macro_file ${macros_files})
  # Filter out hsimple.root; someone might have created it in the src dir, and the hsimple.root
  # target below will interfere.
  add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/${macro_file}
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/${macro_file} ${CMAKE_BINARY_DIR}/${macro_file}
  COMMENT "Copying ${CMAKE_SOURCE_DIR}/${macro_file}"
  DEPENDS ${CMAKE_SOURCE_DIR}/${macro_file})
  list(APPEND macro_files_builddir ${CMAKE_BINARY_DIR}/${macro_file})
endforeach()
add_custom_target(move_artifacts DEPENDS ${stamp_file} ${macro_files_builddir})


#---CXX MODULES-----------------------------------------------------------------------------------
if(MSVC)
  set(_os_cat "type")
else()
  set(_os_cat "cat")
endif()
file(TO_NATIVE_PATH "${CMAKE_BINARY_DIR}/include/module.modulemap.extra" _from_native)
file(TO_NATIVE_PATH "${CMAKE_BINARY_DIR}/include/module.modulemap" _to_native)

add_custom_target(copymodulemap DEPENDS "${CMAKE_BINARY_DIR}/include/module.modulemap")
add_custom_command(
                  OUTPUT "${CMAKE_BINARY_DIR}/include/module.modulemap"
                  DEPENDS build/unix/module.modulemap "${CMAKE_BINARY_DIR}/include/module.modulemap.extra"
                  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_SOURCE_DIR}/build/unix/module.modulemap" "${CMAKE_BINARY_DIR}/include/module.modulemap"
                  COMMAND ${_os_cat} "${_from_native}" >> "${_to_native}"
)
install(FILES "${CMAKE_BINARY_DIR}/include/module.modulemap" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR} COMPONENT headers)

add_dependencies(move_artifacts copymodulemap)



include_directories(${ROOT_INCLUDE_DIR})
#---Recurse walk through subdirectories ------------------------------------------------------
#add_subdirectory(utilities)
add_subdirectory(core)
#add_subdirectory(core2)
add_subdirectory(io)

if(analyses)
  add_subdirectory(analyses)
endif(analyses)

add_subdirectory(apps)
# add_subdirectory(test)

#---CXX MODULES-----------------------------------------------------------------------------------
# Take all the modulemap contents we collected from the packages and append them to our modulemap.
# We have to delay this because the XBOX_CXXMODULES_EXTRA_MODULEMAP_CONTENT is filled in the
# add_subdirectory calls above.
get_property(__modulemap_extra_content GLOBAL PROPERTY XBOX_CXXMODULES_EXTRA_MODULEMAP_CONTENT)
string(REPLACE ";" "" __modulemap_extra_content "${__modulemap_extra_content}")
file(WRITE "${CMAKE_BINARY_DIR}/include/module.modulemap.extra" "${__modulemap_extra_content}")

# From now on we handled all exposed module and want to make all new modulemaps private to XBOX.
set(XBOX_CXXMODULES_WRITE_TO_CURRENT_DIR ON)

get_property(__allHeaders GLOBAL PROPERTY XBOX_HEADER_TARGETS)
add_custom_target(move_headers ALL DEPENDS ${__allHeaders})

#---Global PCH-----------------------------------------------------------------------------------
get_property(__allTargets GLOBAL PROPERTY ROOT_DICTIONARY_TARGETS)
get_property(__allFiles GLOBAL PROPERTY ROOT_DICTIONARY_FILES)
get_property(__clingetcpch GLOBAL PROPERTY CLINGETCPCH)


set (CMAKE_CXX_FLAGS_SEPARATE "${CMAKE_CXX_FLAGS}")
string(REGEX REPLACE "[ ]-" ";-" CMAKE_CXX_FLAGS_SEPARATE "${CMAKE_CXX_FLAGS_SEPARATE} ${CORE_OS_DICT_CXX_FLAGS}")
if(MSVC)
  string(REPLACE "-nologo" "" CMAKE_CXX_FLAGS_SEPARATE "${CMAKE_CXX_FLAGS_SEPARATE}")
  string(REPLACE "-EHsc-" "" CMAKE_CXX_FLAGS_SEPARATE "${CMAKE_CXX_FLAGS_SEPARATE}")
  string(REPLACE "-GR" "" CMAKE_CXX_FLAGS_SEPARATE "${CMAKE_CXX_FLAGS_SEPARATE}")
endif()

