

#---Check for OpenGL installation-------------------------------------------------------
if(opengl)
  message(STATUS "Looking for OpenGL")
  if(APPLE AND NOT cocoa)
    find_path(OPENGL_INCLUDE_DIR GL/gl.h  PATHS /usr/X11R6/include /opt/X11/include)
    find_library(OPENGL_gl_LIBRARY NAMES GL PATHS /usr/X11R6/lib /opt/X11/lib)
    find_library(OPENGL_glu_LIBRARY NAMES GLU PATHS /usr/X11R6/lib /opt/X11/lib)
    find_package_handle_standard_args(OpenGL REQUIRED_VARS OPENGL_INCLUDE_DIR OPENGL_gl_LIBRARY OPENGL_glu_LIBRARY)
    set(OPENGL_LIBRARIES ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY})
    mark_as_advanced(OPENGL_INCLUDE_DIR OPENGL_glu_LIBRARY OPENGL_gl_LIBRARY)
  else()
    find_package(OpenGL)
  endif()
  if(NOT OPENGL_LIBRARIES)
    if(fail-on-missing)
      message(FATAL_ERROR "OpenGL package (with GLU) not found and opengl option required")
    else()
      message(STATUS "OpenGL (with GLU) not found. Switching off opengl option")
      set(opengl OFF CACHE BOOL "" FORCE)
    endif()
  endif()
endif()

#---Check for XML Parser Support-----------------------------------------------------------
if(xml)
  message(STATUS "Looking for LibXml2")
  find_package(LibXml2)
  if(NOT LIBXML2_FOUND)
    if(fail-on-missing)
      message(FATAL_ERROR "LibXml2 libraries not found and they are required (xml option enabled)")
    else()
      message(STATUS "LibXml2 not found. Switching off xml option")
      set(xml OFF CACHE BOOL "" FORCE)
    endif()
  endif()
endif()

#---Check for MySQL-------------------------------------------------------------------
if(mysql)
  message(STATUS "Looking for MySQL")
  find_package(MySQL)
  if(NOT MYSQL_FOUND)
    if(fail-on-missing)
      message(FATAL_ERROR "MySQL libraries not found and they are required (mysql option enabled)")
    else()
      message(STATUS "MySQL not found. Switching off mysql option")
      set(mysql OFF CACHE BOOL "" FORCE)
    endif()
  endif()
endif()

#---Check for SQLite-------------------------------------------------------------------
if(sqlite)
  message(STATUS "Looking for SQLite")
  find_package(Sqlite)
  if(NOT SQLITE_FOUND)
    if(fail-on-missing)
      message(FATAL_ERROR "SQLite libraries not found and they are required (sqlite option enabled)")
    else()
      message(STATUS "SQLite not found. Switching off sqlite option")
      set(sqlite OFF CACHE BOOL "" FORCE)
    endif()
  endif()
endif()

#---Check for Oracle-------------------------------------------------------------------
if(oracle)
  message(STATUS "Looking for Oracle")
  find_package(Oracle)
  if(NOT ORACLE_FOUND)
    if(fail-on-missing)
      message(FATAL_ERROR "Oracle libraries not found and they are required (orable option enabled)")
    else()
      message(STATUS "Oracle not found. Switching off oracle option")
      set(oracle OFF CACHE BOOL "" FORCE)
    endif()
  endif()
endif()

#---Check for TCMalloc---------------------------------------------------------------
if (tcmalloc)
  message(STATUS "Looking for tcmalloc")
  find_package(tcmalloc)
  if(NOT TCMALLOC_FOUND)
    message(STATUS "TCMalloc not found.")
    set(tcmalloc OFF CACHE BOOL "" FORCE)
  endif()
endif()

#---Check for JEMalloc---------------------------------------------------------------
if (jemalloc)
  if (tcmalloc)
   message(FATAL_ERROR "Both tcmalloc and jemalloc were selected: this is an inconsistent setup.")
  endif()
  message(STATUS "Looking for jemalloc")
  find_package(jemalloc)
  if(NOT JEMALLOC_FOUND)
    message(STATUS "JEMalloc not found.")
    set(jemalloc OFF CACHE BOOL "" FORCE)
  endif()
endif()