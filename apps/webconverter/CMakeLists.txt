############################################################################
# CMakeLists.txt file for building XBOX converter package
############################################################################

set(libname xboxconverter)

XBOX_GLOB_SOURCES(sources ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cxx)
XBOX_GLOB_HEADERS(headers ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hxx)

# remove items from list if required if not wished to install
#list(REMOVE_ITEM headers ${CMAKE_CURRENT_SOURCE_DIR}/include/XboxTdmsReader.hxx)

# forward headers to the parent scope
# set(core_dict_headers ${headers} PARENT_SCOPE)

# add include directory. This is necessary for headers excluded from the 
# installation, that is copying to the binary path.
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

include_directories (${HDF5_INCLUDE_DIRS})

# generate object library
#XBOX_OBJECT_LIBRARY(${libname} 
#		            ${sources}
#                    DEPENDENCIES xboxcore xboxio
#				   )

# build library and add to the lib folder
XBOX_LINKER_LIBRARY(${libname} 
		            ${sources}
                    DEPENDENCIES xboxcore xboxio
					LIBRARIES ${ROOT_LIBRARIES} -lROOTWebDisplay -lROOTGpadv7 -lROOTHistDraw ${Boost_LIBRARIES}
				   )

# copy all selected headers to the binary path 
XBOX_INSTALL_HEADERS()

#include(CTest)
#ctest
if(testing)
  add_subdirectory(test)
endif()






