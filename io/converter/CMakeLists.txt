############################################################################
# CMakeLists.txt file for building XBOX tdms io sub package
############################################################################

set(libname converter)

XBOX_GLOB_SOURCES(sources ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cxx)
XBOX_GLOB_HEADERS(headers ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hxx)

# remove items from list if required if not wished to install
#list(REMOVE_ITEM headers ${CMAKE_CURRENT_SOURCE_DIR}/include/XboxTdmsReader.hxx)

# forward headers to the parent scope
# set(core_dict_headers ${headers} PARENT_SCOPE)

# add include directory. This is necessary for headers excluded from the 
# installation, that is copying to the binary path.
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)


# generate object library
if(hdf5)
  XBOX_OBJECT_LIBRARY(${libname} 
                      ${sources}
                      COMPILEDEF HDF5_FOUND
                     )
else(hdf5)
  XBOX_OBJECT_LIBRARY(${libname} 
                      ${sources}
                     )
endif(hdf5)


# build library and add to the lib folder
#XBOX_LINKER_LIBRARY(${libname} ${sources})

# copy all selected headers to the binary path 
XBOX_INSTALL_HEADERS()

#include(CTest)
#ctest
if(testing)
  add_subdirectory(test)
endif()

