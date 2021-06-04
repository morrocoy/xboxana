# - Find ROOT instalation
# This module tries to find the ROOT installation on your system.
# It tries to find the root-config script which gives you all the needed information.
# If the system variable ROOTSYS is set this is straight forward.
# If not the module uses the pathes given in ROOT_CONFIG_SEARCHPATH.
# If you need an other path you should add this path to this variable.
# The root-config script is then used to detect basically everything else.
# This module defines a number of key variables and macros.
# F.Uhlig@gsi.de (fairroot.gsi.de)
#
# Variables defined by this module:
#
#   ROOT_FOUND               System has ROOT, this means the root-config 
#                            executable was found.
#
#   ROOT_INCLUDE_DIR         ROOT include directories: not cached
#
#   ROOT_INCLUDES            Same as above,
#
#   ROOT_LIBRARIES           Link to these to use the ROOT libraries, not cached
#
#   ROOT_LIBRARY_DIR         The path to where the ROOT library files are.
#
#   ROOT_VERSION_STRING      The version string of the ROOT libraries which
#                            is reported by root-config
#
#   ROOT_VERSION_MAJOR       Major version number of ROOT
#   ROOT_VERSION_MINOR       Minor version number of ROOT
#   ROOT_VERSION_PATCH       Patch version number of ROOT
#
#   ROOT_VERSION_NUMBER      A unique version number which is calculated from 
#                            major, minor and patch version found
#
#   ROOT_CINT_EXECUTABLE     The rootcint executable.
#
#   RLIBMAP_EXECUTABLE       The rlibmap executable.

SET(ROOT_CONFIG_SEARCHPATH
  ${SIMPATH}/tools/root/bin
  $ENV{ROOTSYS}/bin
  /opt/local/bin
  /root/bin
  /usr/local/root/bin/
  /usr/local/bin
)

SET(ROOT_DEFINITIONS "")
SET(ROOT_INSTALLED_VERSION_TOO_OLD FALSE)
SET(ROOT_CONFIG_EXECUTABLE ROOT_CONFIG_EXECUTABLE-NOTFOUND)

FIND_PROGRAM(ROOT_CONFIG_EXECUTABLE NAMES root-config PATHS
   ${ROOT_CONFIG_SEARCHPATH}
   NO_DEFAULT_PATH)

IF (${ROOT_CONFIG_EXECUTABLE} MATCHES "ROOT_CONFIG_EXECUTABLE-NOTFOUND")
  MESSAGE( FATAL_ERROR "ROOT not installed in the searchpath and ROOTSYS is not set. Please
 set ROOTSYS or add the path to your ROOT installation in the Macro FindROOT.cmake in the
 subdirectory cmake/modules.")
ELSE (${ROOT_CONFIG_EXECUTABLE} MATCHES "ROOT_CONFIG_EXECUTABLE-NOTFOUND")
  STRING(REGEX REPLACE "(^.*)/bin/root-config" "\\1" test ${ROOT_CONFIG_EXECUTABLE})
  SET( ENV{ROOTSYS} ${test})
  set( ROOTSYS ${test})
ENDIF (${ROOT_CONFIG_EXECUTABLE} MATCHES "ROOT_CONFIG_EXECUTABLE-NOTFOUND")

# root config is a bash script and not commonly executable under Windows
# make some static assumptions instead
IF (WIN32)
  SET(ROOT_FOUND FALSE)
  IF (ROOT_CONFIG_EXECUTABLE)
    SET(ROOT_FOUND TRUE)
    set(ROOT_INCLUDE_DIR ${ROOTSYS}/include)
    set(ROOT_LIBRARY_DIR ${ROOTSYS}/lib)
    SET(ROOT_BINARY_DIR ${ROOTSYS}/bin)
    set(ROOT_LIBRARIES -LIBPATH:${ROOT_LIBRARY_DIR} libCore.lib libImt.lib libRIO.lib libNet.lib libHist.lib libGraf.lib libGraf3d.lib libGpad.lib libROOTDataFrame.lib libROOTVecOps.lib libTree.lib libTreePlayer.lib libRint.lib libPostscript.lib libMatrix.lib libPhysics.lib libMathCore.lib libThread.lib)
	#set(ROOT_LIBRARIES -LIBPATH:${ROOT_LIBRARY_DIR} libGpad.lib libHist.lib libGraf.lib libGraf3d.lib libTree.lib libRint.lib libPostscript.lib libMathCore.lib libRIO.lib libNet.lib libThread.lib libCore.lib libCint.lib libGui.lib libGuiBld.lib)
    FIND_PROGRAM(ROOT_CINT_EXECUTABLE
      NAMES rootcling
      PATHS ${ROOT_BINARY_DIR}
      NO_DEFAULT_PATH
      )
    Find_Program(RLIBMAP_EXECUTABLE
      NAMES rlibmap
      PATHS ${ROOT_BINARY_DIR}
      NO_DEFAULT_PATH
    )
    MESSAGE(STATUS "Found ROOT: $ENV{ROOTSYS}/bin/root (WIN32/version not identified)")
  ENDIF (ROOT_CONFIG_EXECUTABLE)

ELSE(WIN32)

  IF (ROOT_CONFIG_EXECUTABLE)

    SET(ROOT_FOUND FALSE)
	String(REGEX REPLACE "(^.*)/bin/root-config" "\\1" test ${ROOT_CONFIG_EXECUTABLE}) 
	Set(ENV{ROOTSYS} ${test})
	Set(ROOTSYS ${test})

	Execute_Process(COMMAND ${ROOT_CONFIG_EXECUTABLE} --version 
		          OUTPUT_VARIABLE ROOT_VERSION_STRING
		         )
	Execute_Process(COMMAND ${ROOT_CONFIG_EXECUTABLE} --prefix
		          OUTPUT_VARIABLE ROOT_INSTALL_DIR
		         )
	String(STRIP ${ROOT_VERSION_STRING} ROOT_VERSION_STRING)
	String(STRIP ${ROOT_INSTALL_DIR} ROOT_INSTALL_DIR)

	# extract major, minor, and patch versions from
	# the version string given by root-config
	String(REGEX REPLACE "^([0-9]+)\\.[0-9][0-9]+\\/[0-9][0-9]+.*" "\\1" ROOT_VERSION_MAJOR "${ROOT_VERSION_STRING}")
	String(REGEX REPLACE "^[0-9]+\\.([0-9][0-9])+\\/[0-9][0-9]+.*" "\\1" ROOT_VERSION_MINOR "${ROOT_VERSION_STRING}")
	String(REGEX REPLACE "^[0-9]+\\.[0-9][0-9]+\\/([0-9][0-9]+).*" "\\1" ROOT_VERSION_PATCH "${ROOT_VERSION_STRING}")

	# compute overall version numbers which can be compared at once
	Math(EXPR req_vers "${ROOT_FIND_VERSION_MAJOR}*10000 + ${ROOT_FIND_VERSION_MINOR}*100 + ${ROOT_FIND_VERSION_PATCH}")
	Math(EXPR found_vers "${ROOT_VERSION_MAJOR}*10000 + ${ROOT_VERSION_MINOR}*100 + ${ROOT_VERSION_PATCH}")
	Math(EXPR ROOT_FOUND_VERSION "${ROOT_VERSION_MAJOR}*10000 + ${ROOT_VERSION_MINOR}*100 + ${ROOT_VERSION_PATCH}")

	Set(ROOT_Version ${found_vers})
	Set(ROOT_VERSION_NUMBER ${found_vers})

	If(found_vers LESS req_vers)
	Set(ROOT_FOUND FALSE)
	Set(ROOT_INSTALLED_VERSION_TOO_OLD TRUE)
	Else(found_vers LESS req_vers)
	Set(ROOT_FOUND TRUE)
	EndIf(found_vers LESS req_vers)

	MESSAGE(STATUS "Found ROOT: ${ROOT_INSTALL_DIR} (found version \"" ${ROOT_VERSION_MAJOR} "." ${ROOT_VERSION_MINOR} "." ${ROOT_VERSION_PATCH} "\", minimum required is " \"${ROOT_FIND_VERSION}\" ")")  

  Else(ROOT_CONFIG_EXECUTABLE)
    Message(STATUS "ROOT not found.")
    Message(FATAL_ERROR "ROOT not installed in the searchpath and ROOTSYS is not set. Please set ROOTSYS or add the path to your ROOT installation in the Macro FindROOT.cmake in the subdirectory cmake/modules.")
  Endif(ROOT_CONFIG_EXECUTABLE)


  IF (ROOT_FOUND)

    # ask root-config for the library dir
    # Set ROOT_LIBRARY_DIR

    EXEC_PROGRAM( ${ROOT_CONFIG_EXECUTABLE}
      ARGS "--libdir"
      OUTPUT_VARIABLE ROOT_LIBRARY_DIR_TMP )

    IF(EXISTS "${ROOT_LIBRARY_DIR_TMP}")
      SET(ROOT_LIBRARY_DIR ${ROOT_LIBRARY_DIR_TMP} )
    ELSE(EXISTS "${ROOT_LIBRARY_DIR_TMP}")
      MESSAGE("Warning: ROOT_CONFIG_EXECUTABLE reported ${ROOT_LIBRARY_DIR_TMP} as library path,")
      MESSAGE("Warning: but ${ROOT_LIBRARY_DIR_TMP} does NOT exist, ROOT must NOT be installed correctly.")
    ENDIF(EXISTS "${ROOT_LIBRARY_DIR_TMP}")

    # ask root-config for the binary dir
    EXEC_PROGRAM(${ROOT_CONFIG_EXECUTABLE}
      ARGS "--bindir"
      OUTPUT_VARIABLE root_bins )
    SET(ROOT_BINARY_DIR ${root_bins})

    # ask root-config for the include dir
    EXEC_PROGRAM( ${ROOT_CONFIG_EXECUTABLE}
      ARGS "--incdir"
      OUTPUT_VARIABLE root_headers )
    SET(ROOT_INCLUDE_DIR ${root_headers})
    # CACHE INTERNAL "")

    # ask root-config for the library varaibles
    EXEC_PROGRAM( ${ROOT_CONFIG_EXECUTABLE}
      #    ARGS "--noldflags --noauxlibs --libs"
      ARGS "--glibs"
      OUTPUT_VARIABLE root_flags )

    #  STRING(REGEX MATCHALL "([^ ])+"  root_libs_all ${root_flags})
    #  STRING(REGEX MATCHALL "-L([^ ])+"  root_library ${root_flags})
    #  REMOVE_FROM_LIST(root_flags "${root_libs_all}" "${root_library}")

    SET(ROOT_LIBRARIES "${root_flags} -lMinuit")

    # Make variables changeble to the advanced user
    MARK_AS_ADVANCED( ROOT_LIBRARY_DIR ROOT_INCLUDE_DIR ROOT_DEFINITIONS)

    # Set ROOT_INCLUDES
    SET( ROOT_INCLUDES ${ROOT_INCLUDE_DIR})

    SET(LD_LIBRARY_PATH ${LD_LIBRARY_PATH} ${ROOT_LIBRARY_DIR})

    FIND_PROGRAM(ROOT_CINT_EXECUTABLE
      NAMES rootcint
      PATHS ${ROOT_BINARY_DIR}
      NO_DEFAULT_PATH
      )

    Find_Program(RLIBMAP_EXECUTABLE
      NAMES rlibmap
      PATHS ${ROOT_BINARY_DIR}
      NO_DEFAULT_PATH
      )

  ENDIF (ROOT_FOUND)
ENDIF(WIN32)

