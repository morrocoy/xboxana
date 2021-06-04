############################################################################
# CMakeLists.txt file for building XBOX tdms io sub package
############################################################################

#set(target test_XboxFileConverter)

#if(hdf5)
#  XBOX_EXECUTABLE(${target}
#                  ${target}.cpp ../src/XboxFileConverter.cxx
#                 COMPILEDEF HDF5_FOUND
#                  LIBRARIES ${ROOT_LIBRARIES} xboxcore xboxio)
#else(hdf5)
#  XBOX_EXECUTABLE(${target}
#                  ${target}.cpp #../src/XboxFileConverter.cxx
#                  LIBRARIES ${ROOT_LIBRARIES} xboxcore xboxio)
#endif(hdf5)

#XBOX_ADD_TEST(${target} COMMAND ${target})

#XBOX_EXECUTABLE(test_converterpanel test_converterpanel.cpp LIBRARIES ${ROOT_LIBRARIES} -lROOTWebDisplay -lROOTGpadv7 -lROOTHistDraw ${Boost_LIBRARIES} xboxcore xboxio xboxconverter)
#XBOX_ADD_TEST(test_converterpanel COMMAND test_converterpanel)
