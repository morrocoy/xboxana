#---Check for installed packages depending on the build options/components eamnbled -
include(ExternalProject)
include(FindPackageHandleStandardArgs)

# external software packages from CERN EP-SFT (SoFTware Development for Experiments)
set(lcgpackages http://lcgpackages.web.cern.ch/lcgpackages/tarFiles/sources)

#---On MacOSX, try to find frameworks after standard libraries or headers------------
set(CMAKE_FIND_FRAMEWORK LAST)

#---Guess under which lib directory the external packages will install the libraires
set(_LIBDIR_DEFAULT "lib")
if(CMAKE_SYSTEM_NAME MATCHES "Linux" AND NOT CMAKE_CROSSCOMPILING AND NOT EXISTS "/etc/debian_version")
  if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
    set(_LIBDIR_DEFAULT "lib64")
  endif()
endif()

#---Check ROOT--------------------------------------------------------------------------
if(root)
  message(STATUS "Looking for ROOT")
  find_package(ROOT 6.10.00)
  if(NOT ROOT_FOUND)
    if(fail-on-missing)
      message(FATAL_ERROR "ROOT libraries not found and they are required")
    else()
      message(FATAL_ERROR "ROOT libraries not found and they are required")
      set(root OFF CACHE BOOL "" FORCE)
    endif()
  endif()
endif()

#---Check BOOST--------------------------------------------------------------------------
if(boost)
  message(STATUS "Looking for BOOST")
#  find_package (Boost COMPONENTS system filesystem date_time)
  #find_package (Boost COMPONENTS system) 

  if(WIN32) 
    find_package (Boost) 
    if(Boost_FOUND)
      set(Boost_LIBRARY_DIR ${Boost_INCLUDE_DIRS}/lib) # manual created folder with renamed libs 
    endif()
  endif()
  
  find_package (Boost COMPONENTS system filesystem date_time)
  if(NOT Boost_FOUND)
    if(fail-on-missing)
      message(FATAL_ERROR "BOOST libraries not found and they are required")
    else()
      message(STATUS "Boost not found. Switching off boost option")
      set(boost OFF CACHE BOOL "" FORCE)
    endif()
  endif()
endif()
#message("boost version: " ${Boost_VERSION})
#message("boost include: " ${Boost_INCLUDE_DIRS})
#message("boost libs:" ${Boost_LIBRARIES})

#---Check HDF5--------------------------------------------------------------------------
if(hdf5)
  message(STATUS "Looking for HDF5 CXX binding")
  find_package (HDF5 COMPONENTS CXX) # Find non-cmake built HDF5
  #find_package (HDF5 NAMES hdf5 COMPONENTS CXX ${SEARCH_TYPE}) # Find non-cmake built HDF5
  if(NOT HDF5_FOUND)
    if(fail-on-missing)
      message(FATAL_ERROR "HDF5 libraries not found and they are required")
    else()
      message(STATUS "HDF5 not found. Switching off hdf5 option")
      set(hdf5 OFF CACHE BOOL "" FORCE)
    endif()
  endif()
endif()
#message("hdf5 version: " ${HDF5_VERSION})
#message("hdf5 include (deprecated): " ${HDF5_INCLUDE_DIR})
#message("hdf5 include: " ${HDF5_INCLUDE_DIRS})
#message("hdf5 libs:" ${HDF5_LIBRARIES})
#message("hdf5 defs: " ${HDF5_DEFINITIONS})
#message("hdf5 C defs: " ${HDF5_C_DEFINITIONS})
#message("hdf5 CXX defs: " ${HDF5_CXX_DEFINITIONS})

#---Check QT5--------------------------------------------------------------------------
if(qt5)
  message(STATUS "Looking for QT5") 
  #find_package(Qt5 COMPONENTS Core Widgets Qt5DataVisualization REQUIRED) # find QT5 dependencies.
  find_package(Qt5 COMPONENTS Core Widgets Charts)
  if(NOT EXISTS ${Qt5_DIR})
    if(fail-on-missing)
      message(FATAL_ERROR "QT5 libraries not found and they are required")
    else()
      message(STATUS "QT5 not found. Switching off qt5 option")
      set(qt5 OFF CACHE BOOL "" FORCE)
    endif()
  else()
    message("-- Found Qt5: " ${Qt5_DIR})
  endif()
endif()

#---Check for Eigen3 library-------------------------------------------------------------
if(eigen3)
  message(STATUS "Looking for Eigen3")
  find_package(Eigen3 3.3 NO_MODULE)
  if (TARGET Eigen3::Eigen)
    message("-- Found Eigen3: " ${EIGEN3_INCLUDE_DIRS})
  else(TARGET Eigen3::Eigen)
    if(fail-on-missing)
      message(FATAL_ERROR "Eigen3 libraries not found and they are required")
    else()
      message(STATUS "Eigen3 not found. Switching off eigen3 option")
      set(eigen3 OFF CACHE BOOL "" FORCE)
    endif()
  endif(TARGET Eigen3::Eigen)
endif()

#---Check for GSL library---------------------------------------------------------------
if(gsl)
  message(STATUS "Looking for GSL")
  find_package(GSL 1.10)
  if(NOT GSL_FOUND)
    if(fail-on-missing)
      message(FATAL_ERROR "GSL package not found and they are required")
    else()
      message(STATUS "GSL not found. Switching off 'gsl' and 'analyses' options")
      set(gsl OFF CACHE BOOL "" FORCE)
      set(analyses OFF CACHE BOOL "" FORCE)
    endif()
  endif()
else()
  message(STATUS "Switching off 'analyses' options since GSL package not selected")
  set(analyses OFF CACHE BOOL "" FORCE)
endif()


#---Download googletest--------------------------------------------------------------
if (testing AND NOT WIN32 AND NOT 1)
  # FIXME: Remove our version of gtest in xboxtest. We can reuse this one.
  # Add googletest
  # http://stackoverflow.com/questions/9689183/cmake-googletest

  set(_gtest_byproduct_binary_dir
    ${CMAKE_CURRENT_BINARY_DIR}/googletest-prefix/src/googletest-build/googlemock/)
  set(_gtest_byproducts
    ${_gtest_byproduct_binary_dir}/gtest/libgtest.a
    ${_gtest_byproduct_binary_dir}/gtest/libgtest_main.a
    ${_gtest_byproduct_binary_dir}/libgmock.a
    ${_gtest_byproduct_binary_dir}/libgmock_main.a
    )

  if(MSVC)
    set(EXTRA_GTEST_OPTS
      -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG:PATH=\\\"\\\"
      -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE:PATH=\\\"\\\")
  endif()

  ExternalProject_Add(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.8.0
    UPDATE_COMMAND ""
    # TIMEOUT 10
    # # Force separate output paths for debug and release builds to allow easy
    # # identification of correct lib in subsequent TARGET_LINK_LIBRARIES commands
    # CMAKE_ARGS -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG:PATH=DebugLibs
    #            -DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE:PATH=ReleaseLibs
    #            -Dgtest_force_shared_crt=ON
    CMAKE_ARGS -G ${CMAKE_GENERATOR}
                  -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                  -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
                  -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
                  -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
                  -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
                  -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
                  ${EXTRA_GTEST_OPTS}
    # Disable install step
    INSTALL_COMMAND ""
    BUILD_BYPRODUCTS ${_gtest_byproducts}
    # Wrap download, configure and build steps in a script to log output
    LOG_DOWNLOAD ON
    LOG_CONFIGURE ON
    LOG_BUILD ON)

  # Specify include dirs for gtest and gmock
  ExternalProject_Get_Property(googletest source_dir)
  set(GTEST_INCLUDE_DIR ${source_dir}/googletest/include)
  set(GMOCK_INCLUDE_DIR ${source_dir}/googlemock/include)

  # Libraries
  ExternalProject_Get_Property(googletest binary_dir)
  set(_G_LIBRARY_PATH ${binary_dir}/googlemock/)

  # Register gtest, gtest_main, gmock, gmock_main
  foreach (lib gtest gtest_main gmock gmock_main)
    add_library(${lib} IMPORTED STATIC GLOBAL)
    add_dependencies(${lib} googletest)
  endforeach()
  set_property(TARGET gtest PROPERTY IMPORTED_LOCATION ${_G_LIBRARY_PATH}/gtest/${CMAKE_STATIC_LIBRARY_PREFIX}gtest${CMAKE_STATIC_LIBRARY_SUFFIX})
  set_property(TARGET gtest_main PROPERTY IMPORTED_LOCATION ${_G_LIBRARY_PATH}/gtest/${CMAKE_STATIC_LIBRARY_PREFIX}gtest_main${CMAKE_STATIC_LIBRARY_SUFFIX})
  set_property(TARGET gmock PROPERTY IMPORTED_LOCATION ${_G_LIBRARY_PATH}/${CMAKE_STATIC_LIBRARY_PREFIX}gmock${CMAKE_STATIC_LIBRARY_SUFFIX})
  set_property(TARGET gmock_main PROPERTY IMPORTED_LOCATION ${_G_LIBRARY_PATH}/${CMAKE_STATIC_LIBRARY_PREFIX}gmock_main${CMAKE_STATIC_LIBRARY_SUFFIX})

endif()


