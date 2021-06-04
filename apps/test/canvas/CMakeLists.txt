############################################################################
# CMakeLists.txt file for building XBOX converter gui package
############################################################################

cmake_minimum_required(VERSION 3.1.0)

# process ``GENERATED`` source files in ``AUTOMOC`` and ``AUTOUIC`` just like regular source files.
cmake_policy(SET CMP0071 NEW) 

# Tell CMake which c++ standard to use
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Tell CMake to run moc when necessary:
set(CMAKE_AUTOMOC ON)

# As moc files are generated in the binary dir, tell CMake
# to always look for includes there:
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# set additional include paths
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/)

# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)

# Widgets finds its own dependencies (QtGui and QtCore).
find_package(Qt5Widgets REQUIRED)

# Tell moc where to find the specific header files (required for class definitions containg Q_OBJECT macro)
qt5_wrap_cpp(moc_sources ${CMAKE_CURRENT_SOURCE_DIR}/include/canvas.hxx)

# Tell moc where to find the specific header files (required for class definitions containg Q_OBJECT macro)
#qt5_wrap_ui(uic_sources ${CMAKE_CURRENT_SOURCE_DIR}/MainWindow.ui)

# Tell CMake to create the executables
XBOX_EXECUTABLE(test_canvas test_canvas.cxx canvas.cxx  ${moc_sources} ${uic_sources} 
				LIBRARIES ${ROOT_LIBRARIES} xboxcore xboxio)

# Tell CMake which c++ standard to use (alternative to above)
#target_compile_features(QXboxTdmsConverterApp PRIVATE cxx_std_14)

# extends include paths and definitions, apply compiler flags and link to libraries all in once for qt5 modules
qt5_use_modules(test_canvas Widgets) 

