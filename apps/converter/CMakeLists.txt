############################################################################
# CMakeLists.txt file for building XBOX converter gui package
############################################################################

cmake_minimum_required(VERSION 3.1.0)

# process ``GENERATED`` source files in ``AUTOMOC`` and ``AUTOUIC`` just like regular source files.
cmake_policy(SET CMP0071 NEW) 

# Tell CMake which c++ standard to use
if(cxx11)
  set(CMAKE_CXX_STANDARD 11)
elseif(cxx14)
  set(CMAKE_CXX_STANDARD 14)
elseif(cxx17)
  set(CMAKE_CXX_STANDARD 17)
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# set additional include paths
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/)

# Get source and header files
XBOX_GLOB_HEADERS(headers ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hxx)

# create command line programms
# ------------------------------------------------------------------------------------
set(targetname xboxtmds2root)

XBOX_GLOB_SOURCES(sources ${CMAKE_CURRENT_SOURCE_DIR}/src/XboxTdms2Root.cpp)
                          #${CMAKE_CURRENT_SOURCE_DIR}/src/XboxFileConverter.cxx)

XBOX_EXECUTABLE(${targetname} 
                ${sources} ${moc_sources} ${uic_sources} 
				LIBRARIES ${ROOT_LIBRARIES} xboxcore xboxio)


# create gui programms (qt5 style)
# ------------------------------------------------------------------------------------

if(qt5 AND (EXISTS ${Qt5Widgets_DIR}))

  set(targetname QXboxFileConverterApp) 

  # Tell CMake to run moc when necessary
  set(CMAKE_AUTOMOC ON)

  # As moc files are generated in binary dir, tell CMake to look for includes there
  set(CMAKE_INCLUDE_CURRENT_DIR ON)

  # Create code from a list of Qt designer ui files
  set(CMAKE_AUTOUIC ON)

  # Widgets finds its own dependencies (QtGui and QtCore).
  find_package(Qt5Widgets REQUIRED)

  # Tell moc where to find the specific header files (required for class definitions containg Q_OBJECT macro)
  qt5_wrap_cpp(moc_sources ${headers})

  # Get source and header files
  XBOX_GLOB_SOURCES(sources ${CMAKE_CURRENT_SOURCE_DIR}/src/QXboxFileConverterApp.cxx
                            ${CMAKE_CURRENT_SOURCE_DIR}/src/QXboxFileConverter.cxx
                            ${CMAKE_CURRENT_SOURCE_DIR}/src/XboxFileConverter.cxx)

  XBOX_EXECUTABLE(${targetname} 
                  ${sources} ${moc_sources} ${uic_sources} 
                  LIBRARIES ${ROOT_LIBRARIES} xboxcore xboxio)

  # Tell CMake which c++ standard to use (alternative to above)
  #target_compile_features(${targetname} PRIVATE cxx_std_14)

  # extends include paths and definitions, apply compiler flags and link to libraries all in once for qt5 modules
  qt5_use_modules(${targetname} Widgets) 
 
endif()

# copy all selected headers to the binary path 
# ------------------------------------------------------------------------------------
XBOX_INSTALL_HEADERS()


# testing
# ------------------------------------------------------------------------------------
#include(CTest)
#ctest
if(testing)
  add_subdirectory(test)
endif()





