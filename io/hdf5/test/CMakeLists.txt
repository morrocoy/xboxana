############################################################################
# CMakeLists.txt file for building XBOX tdms io sub package
############################################################################

set(target test_XboxHdf5Export)

XBOX_EXECUTABLE(${target} 
                ${target}.cpp 
                LIBRARIES ${ROOT_LIBRARIES} xboxcore xboxio)

XBOX_ADD_TEST(${target} COMMAND ${target})

