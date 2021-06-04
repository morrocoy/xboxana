############################################################################
# CMakeLists.txt file for building tdms utility package
############################################################################

set(libname testclass)

XBOX_GLOB_SOURCES(sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cxx)
XBOX_GLOB_HEADERS(headers ${CMAKE_CURRENT_SOURCE_DIR}/*.hxx)
XBOX_GLOB_FILES(linkdefs ${CMAKE_CURRENT_SOURCE_DIR}/*LinkDef.h)

# remove items from list if required if not wished to install
#list(REMOVE_ITEM headers ${CMAKE_CURRENT_SOURCE_DIR}/include/dummy.h)

# forward headers to the parent scope
# set(core_dict_headers ${headers} PARENT_SCOPE)

# add include directory. This is necessary for headers excluded from the 
# installation, that is copying to the binary path.
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)


# get list of linkdef files (they must match the pattern "*LinkDef.h"
#file(GLOB linkdefs "${CMAKE_CURRENT_SOURCE_DIR}/include/*LinkDef.h")

# generate ROOT dictionary 
XBOX_GENERATE_DICTIONARY(${CMAKE_CURRENT_BINARY_DIR}/G__${libname} ${headers}
                      MODULE ${libname}
                      LINKDEF ${linkdefs}
#                      OPTIONS "-writeEmptyRootPCM"
                      DEPENDENCIES 
                      )
					  
# root libraries
set(ARG_LIBRARIES ${ARG_LIBRARIES} ${ROOT_LIBRARIES})

# generate object library
XBOX_LINKER_LIBRARY(${libname} 
                    ${sources} ${CMAKE_CURRENT_BINARY_DIR}/G__${libname}.cxx
					LIBRARIES ${ARG_LIBRARIES}
#					DEPENDENCIES testbase
					)

# build library and add to the lib folder
#XBOX_LINKER_LIBRARY(${libname} ${sources})

# copy all selected headers to the binary path 
XBOX_INSTALL_HEADERS()


#...........................................................................
set(target test_class)

if(WIN32)
  XBOX_EXECUTABLE(${target} 
                  ${target}.cpp 
                  ${sources} ${CMAKE_CURRENT_BINARY_DIR}/G__${libname}.cxx
                  LIBRARIES ${ROOT_LIBRARIES} testclass)
else(WIN32)
  XBOX_EXECUTABLE(${target} 
                  ${target}.cpp 
                  ${sources} ${CMAKE_CURRENT_BINARY_DIR}/G__${libname}.cxx
                  LIBRARIES ${ROOT_LIBRARIES})
endif(WIN32)
XBOX_ADD_TEST(${target} COMMAND ${target})


