############################################################################
# CMakeLists.txt file for building XBOX core sub package
############################################################################

set(libname xboxbase)

XBOX_GLOB_SOURCES(sources ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cxx)
XBOX_GLOB_HEADERS(headers ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hxx *h)
XBOX_GLOB_FILES(linkdefs ${CMAKE_CURRENT_SOURCE_DIR}/include/*LinkDef.h)

# remove items from list if required if not wished to install
#list(REMOVE_ITEM headers ${CMAKE_CURRENT_SOURCE_DIR}/include/dummy.h)

# forward headers to the parent scope
# set(core_dict_headers ${headers} PARENT_SCOPE)

# add include directory. This is necessary for headers excluded from the 
# installation, that is copying to the binary path.
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# generate ROOT dictionary 
XBOX_GENERATE_DICTIONARY(${CMAKE_CURRENT_BINARY_DIR}/G__${libname} ${headers}
                      MODULE ${libname}
                      LINKDEF ${linkdefs}
 #                     OPTIONS "-writeEmptyRootPCM"
                      DEPENDENCIES 
                      )

# generate object library
XBOX_OBJECT_LIBRARY(${libname} 
					${sources} ${CMAKE_CURRENT_BINARY_DIR}/G__${libname}.cxx
					COMPILEDEF XBOX_SHARED_EXPORTS
					)

# generate linker library (this is done later when combining all core libraries)
#set(ARG_LIBRARIES ${ARG_LIBRARIES} ${ROOT_LIBRARIES})
#XBOX_LINKER_LIBRARY(${libname} ${sources} ${CMAKE_CURRENT_BINARY_DIR}/G__${libname}.cxx
#                    LIBRARIES ${ARG_LIBRARIES}
#                   )

# alternatively use ROOT_STANDARD_LIBRARY_PACKAGE command to build library
#ROOT_STANDARD_LIBRARY_PACKAGE(${libname}
#                              HEADERS ${headers}
#                              SOURCES ${sources} 
#                              LINKDEF  ${CMAKE_CURRENT_SOURCE_DIR}/include/XboxCoreLinkDef.h
#                              OBJECT_LIBRARY
#                              DICTIONARY_OPTIONS "-writeEmptyRootPCM"
#)

# copy all selected headers to the binary path 
XBOX_INSTALL_HEADERS()

# ctest
if(testing)
  add_subdirectory(test)
endif()









