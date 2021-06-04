set(xbox_build_options)

#---------------------------------------------------------------------------------------------------
#---XBOX_BUILD_OPTION( name defvalue [description] )
#---------------------------------------------------------------------------------------------------
function(XBOX_BUILD_OPTION opt defvalue)
  if(ARGN)
    set(description ${ARGN})
  else()
    set(description " ")
  endif()
  set(${opt}_defvalue    ${defvalue} PARENT_SCOPE)
  set(${opt}_description ${description} PARENT_SCOPE)
  set(xbox_build_options  ${xbox_build_options} ${opt} PARENT_SCOPE )
endfunction()

#---------------------------------------------------------------------------------------------------
#---XBOX_APPLY_OPTIONS()
#---------------------------------------------------------------------------------------------------
function(XBOX_APPLY_OPTIONS)
  foreach(opt ${xbox_build_options})
     option(${opt} "${${opt}_description}" ${${opt}_defvalue})
  endforeach()  
endfunction()

#---------------------------------------------------------------------------------------------------
#---XBOX_SHOW_OPTIONS([var] )
#---------------------------------------------------------------------------------------------------
function(XBOX_SHOW_OPTIONS)
  set(enabled)
  foreach(opt ${xbox_build_options})
    if(${opt})
      set(enabled "${enabled} ${opt}")
    endif()
  endforeach()
  if(NOT ARGN)
    message(STATUS "Enabled support for: ${enabled}")
  else()
    set(${ARGN} "${enabled}" PARENT_SCOPE)
  endif()
endfunction()

#---------------------------------------------------------------------------------------------------
#---XBOX_WRITE_OPTIONS(file )
#---------------------------------------------------------------------------------------------------
function(XBOX_WRITE_OPTIONS file)
  file(WRITE ${file} "#---Options enabled for the build of XBOX-----------------------------------------------\n")
  foreach(opt ${xbox_build_options})
    if(${opt})
      file(APPEND ${file} "set(${opt} ON)\n")
    else()
      file(APPEND ${file} "set(${opt} OFF)\n")
    endif()
  endforeach()
endfunction()

#--------------------------------------------------------------------------------------------------
#---Full list of options with their descriptios and default values
#   The default value can be changed as many times as we wish before calling XBOX_APPLY_OPTIONS()
#--------------------------------------------------------------------------------------------------

XBOX_BUILD_OPTION(cxx11 OFF "Build using C++11 compatible mode, requires gcc > 4.7.x or clang")
XBOX_BUILD_OPTION(cxx14 OFF "Build using C++14 compatible mode, requires gcc > 4.9.x or clang")
XBOX_BUILD_OPTION(cxx17 ON "Build using C++17 compatible mode, requires gcc >= 7.2.0 or clang")
XBOX_BUILD_OPTION(exceptions ON "Turn on compiler exception handling capability")
XBOX_BUILD_OPTION(explicitlink ON "Explicitly link with all dependent libraries")
XBOX_BUILD_OPTION(libcxx OFF "Build using libc++, requires cxx11 option (MacOS X only, for the time being)")
#XBOX_BUILD_OPTION(macos_native OFF "Disable looking for libraries, includes and binaries in locations other than a native installation (MacOS only)")
#XBOX_BUILD_OPTION(soversion OFF "Set version number in sonames (recommended)")

#XBOX_BUILD_OPTION(tcmalloc OFF "Using the tcmalloc allocator")
#XBOX_BUILD_OPTION(jemalloc ON "Using the jemalloc allocator")
#XBOX_BUILD_OPTION(memory_termination OFF "Free internal ROOT memory before process termination (experimental, used for leak checking)")
#XBOX_BUILD_OPTION(memstat OFF "A memory statistics utility, helps to detect memory leaks")

#XBOX_BUILD_OPTION(imt ON "Implicit multi-threading support")
#XBOX_BUILD_OPTION(thread ON "Using thread library (cannot be disabled)")
#XBOX_BUILD_OPTION(winrtdebug OFF "Link against the Windows debug runtime library")

XBOX_BUILD_OPTION(root ON "Use ROOT library, required version > 6.10.00")
XBOX_BUILD_OPTION(boost OFF "Use BOOST library")
XBOX_BUILD_OPTION(hdf5 ON "Use HDF5 library")
XBOX_BUILD_OPTION(qt5 ON "Use QT5 library")
XBOX_BUILD_OPTION(gsl ON "Use GSL library")
XBOX_BUILD_OPTION(eigen3 ON "Use Eigen library")
#XBOX_BUILD_OPTION(opengl OFF "OpenGL support, requires libGL and libGLU")
#XBOX_BUILD_OPTION(oracle OFF "Oracle support, requires libocci")
#XBOX_BUILD_OPTION(x11 OFF "X11 support")
#XBOX_BUILD_OPTION(xft OFF "Xft support (X11 antialiased fonts)")
#XBOX_BUILD_OPTION(xml ON "XML parser interface")
#XBOX_BUILD_OPTION(gsl_shared OFF "Enable linking against shared libraries for GSL (default OFF)")
#XBOX_BUILD_OPTION(builtin_gsl OFF "Build the GSL library internally (downloading tarfile from the Web)")
#XBOX_BUILD_OPTION(cocoa OFF "Use native Cocoa/Quartz graphics backend (MacOS X only)")
#XBOX_BUILD_OPTION(mysql OFF "MySQL support, requires libmysqlclient")
#XBOX_BUILD_OPTION(sqlite OFF "SQLite support, requires libsqlite3")

#XBOX_BUILD_OPTION(coverage OFF "Test coverage")

XBOX_BUILD_OPTION(analyses ON "Build analyses package, requires GSL")

option(fail-on-missing "Fail the configure step if a required external package is missing" OFF)
#option(minimal "Do not automatically search for support libraries" OFF)
#option(gminimal "Do not automatically search for support libraries, but include X11" OFF)
#option(all "Enable all optional components" OFF)
option(testing "Enable testing with CTest" ON)
#option(xboxtest "Include xboxtest, if xboxtest exists in xbox or if it is a sibling directory." OFF)
#option(xboxbench "Include xboxbench, if xboxbench exists in xbox or if it is a sibling directory." OFF)

#--- Minor chnages in defaults due to platform--------------------------------------------------
if(WIN32)
#  set(x11_defvalue OFF)
#  set(memstat_defvalue OFF)
  set(cxx11_defvalue ON)
  set(cxx14_defvalue OFF)
  set(cxx17_defvalue OFF)
#  set(imt_defvalue OFF)
#  set(tcmalloc_defvalue OFF)
#  set(jemalloc_defvalue OFF)
#  set(root_defvalue ON)
#  set(xml_defvalue ON)
  set(boost_defvalue OFF)
  set(hdf5_defvalue OFF)
  set(qt5_defvalue ON)
  set(gsl_defvalue ON)
  set(eigen3_defvalue ON)
  set(testing_defvalue ON)
#  set(xboxtest_defvalue OFF)
elseif(APPLE)
#  set(x11_defvalue OFF)
#  set(cocoa_defvalue OFF)
#  set(tcmalloc_defvalue OFF)
#  set(jemalloc_defvalue ON)
  set(root_defvalue ON)
#  set(xml_defvalue ON)
  set(boost_defvalue OFF)
  set(hdf5_defvalue OFF)
  set(qt5_defvalue ON)
  set(gsl_defvalue ON)
  set(eigen3_defvalue ON)
  set(testing_defvalue ON)
#  set(xboxtest_defvalue OFF)
endif()

#---Apply minimal or gminimal------------------------------------------------------------------
#foreach(opt ${xbox_build_options})
#  if(NOT opt MATCHES "thread|cxx14|explicitlink")
#    if(minimal)
#      set(${opt}_defvalue OFF)
#    elseif(gminimal AND NOT opt MATCHES "x11|cocoa")
#      set(${opt}_defvalue OFF)
#    endif()
#  endif()
#endforeach()


#---xboxtest option implies testing
#if(xboxtest OR xboxbench)
#  set(testing ON CACHE BOOL "" FORCE)
#endif()

#---Define at moment the options with the selected default values-----------------------------
XBOX_APPLY_OPTIONS()

#---Avoid creating dependencies to 'non-standard' header files -------------------------------
include_regular_expression("^[^.]+$|[.]h$|[.]icc$|[.]hxx$|[.]hpp$")

#---Add Installation Variables------------------------------------------------------------------
include(XBOXInstallDirs)

#---RPATH options-------------------------------------------------------------------------------
#  When building, don't use the install RPATH already (but later on when installing)
set(CMAKE_SKIP_BUILD_RPATH FALSE)         # don't skip the full RPATH for the build tree
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) # use always the build RPATH for the build tree
set(CMAKE_MACOSX_RPATH TRUE)              # use RPATH for MacOSX
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE) # point to directories outside the build tree to the install RPATH

# Check whether to add RPATH to the installation (the build tree always has the RPATH enabled)
if(rpath)
  set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_FULL_LIBDIR}) # install LIBDIR
  set(CMAKE_SKIP_INSTALL_RPATH FALSE)          # don't skip the full RPATH for the install tree
elseif(APPLE)
  set(CMAKE_INSTALL_NAME_DIR "@rpath")
  if(gnuinstall)
    set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_FULL_LIBDIR}) # install LIBDIR
  else()
    set(CMAKE_INSTALL_RPATH "@loader_path/../lib")    # self relative LIBDIR
  endif()
  set(CMAKE_SKIP_INSTALL_RPATH FALSE)          # don't skip the full RPATH for the install tree
else()
  set(CMAKE_SKIP_INSTALL_RPATH TRUE)           # skip the full RPATH for the install tree
endif()

#---deal with the DCMAKE_IGNORE_PATH------------------------------------------------------------
if(macos_native)
  if(APPLE)
    set(CMAKE_IGNORE_PATH)
    foreach(_prefix /sw /opt/local /usr/local) # Fink installs in /sw, and MacPort in /opt/local and Brew in /usr/local
      list(APPEND CMAKE_IGNORE_PATH ${_prefix}/bin ${_prefix}/include ${_prefix}/lib)
    endforeach()
  else()
    message(STATUS "Option 'macos_native' is only for MacOS systems. Ignoring it.")
  endif()
endif()



