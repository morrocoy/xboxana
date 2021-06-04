############################################################################
# CMakeLists.txt file for building tdms utility package
############################################################################

set(libname xboxanalyses)

# get all header and linkdef files
# ------------------------------------------------------------------------------------
XBOX_GLOB_SOURCES(sources ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cxx)
XBOX_GLOB_HEADERS(headers ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hxx *h)
XBOX_GLOB_FILES(linkdefs ${CMAKE_CURRENT_SOURCE_DIR}/include/*LinkDef.h)

# generate ROOT dictionary
# ------------------------------------------------------------------------------------
set(headers_dict ${CMAKE_CURRENT_SOURCE_DIR}/include/XboxAnalyserEntry.hxx
                 ${CMAKE_CURRENT_SOURCE_DIR}/include/XboxAnalyserResult.hxx
)

XBOX_GENERATE_DICTIONARY(${CMAKE_CURRENT_BINARY_DIR}/G__${libname} ${headers_dict}
                      MODULE ${libname}
                      LINKDEF ${linkdefs}
                      #OPTIONS "-writeEmptyRootPCM"
                      DEPENDENCIES xboxcore xboxio)


# build library and add to the lib folder
# ------------------------------------------------------------------------------------
#include_directories(${EIGEN3_INCLUDE_DIRS})

if(WIN32)
  XBOX_LINKER_LIBRARY(${libname}
                      ${sources} ${CMAKE_CURRENT_BINARY_DIR}/G__${libname}.cxx
                      LIBRARIES ${ROOT_LIBRARIES} #libMathMore.lib
                      #COMPILEDEF XBOX_SHARED_EXPORTS
                      DEPENDENCIES xboxcore xboxio)
else(WIN32)
  XBOX_LINKER_LIBRARY(${libname}
                      ${sources} ${CMAKE_CURRENT_BINARY_DIR}/G__${libname}.cxx
                      LIBRARIES ${ROOT_LIBRARIES} #-lMathMore Eigen3::Eigen
                      #COMPILEDEF XBOX_SHARED_EXPORTS
                      DEPENDENCIES xboxcore xboxio)
endif(WIN32)



# copy all selected headers to the binary path
# ------------------------------------------------------------------------------------
XBOX_INSTALL_HEADERS()

# ctest
if(testing)
  add_subdirectory(test)
endif()
