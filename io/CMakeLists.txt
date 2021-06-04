############################################################################
# CMakeLists.txt file for building XBOX (global) io package
############################################################################

set(libname xboxio)

# create object libraries for each sub module
#---------------------------------------------------------------------------
add_subdirectory(tdms)
if(hdf5) # add HDF5 functionality if available
  add_subdirectory(hdf5)
endif()
add_subdirectory(converter)


# add object libraries to the target linker library
#---------------------------------------------------------------------------
set(objectlibs $<TARGET_OBJECTS:tdms>)
if(hdf5)
  set(objectlibs ${objectlibs} $<TARGET_OBJECTS:hdf5>)
endif(hdf5)
set(objectlibs ${objectlibs} $<TARGET_OBJECTS:converter>)

# external libraries
#---------------------------------------------------------------------------
if(hdf5) # add HDF5 functionality if available
  set(ARG_LIBRARIES ${ROOT_LIBRARIES} ${HDF5_LIBRARIES})
else()
  set(ARG_LIBRARIES ${ROOT_LIBRARIES})
endif()


# generate linker library from all xbox core object libraries 
#---------------------------------------------------------------------------
XBOX_LINKER_LIBRARY(${libname} 
                    ${objectlibs}
                    LIBRARIES ${ARG_LIBRARIES}
                    DEPENDENCIES xboxcore 
                   )
