############################################################################
# CMakeLists.txt file for building XBOX tdms io sub package
############################################################################

set(target test_XboxTdmsFileConverter)

XBOX_EXECUTABLE(${target} 
                ${target}.cpp 
                LIBRARIES ${ROOT_LIBRARIES} xboxcore xboxio)
XBOX_ADD_TEST(${target} COMMAND ${target})


set(target test_XboxFileConverter)

XBOX_EXECUTABLE(${target}
                ${target}.cpp 
                LIBRARIES ${ROOT_LIBRARIES} xboxcore xboxio)
XBOX_ADD_TEST(${target} COMMAND ${target})
