#---------------------------------------------------------------------------------------------------
#  XBOXMacros.cmake
#---------------------------------------------------------------------------------------------------

set(THISDIR ${CMAKE_CURRENT_LIST_DIR})

set(lib lib)
set(bin bin)
if(WIN32)
  set(ssuffix .bat)
  set(scomment rem)
  set(libprefix lib)
  set(ld_library_path PATH)
  set(libsuffix .dll)
  set(localruntimedir ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
  set(runtimedir ${CMAKE_INSTALL_BINDIR})
elseif(APPLE)
  set(ld_library_path DYLD_LIBRARY_PATH)
  set(ssuffix .csh)
  set(scomment \#)
  set(libprefix ${CMAKE_SHARED_LIBRARY_PREFIX})
  if(CMAKE_PROJECT_NAME STREQUAL XBOX)
    set(libsuffix .so)
  else()
    set(libsuffix ${CMAKE_SHARED_LIBRARY_SUFFIX})
  endif()
  set(localruntimedir ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
  set(runtimedir ${CMAKE_INSTALL_LIBDIR})
else()
  set(ld_library_path LD_LIBRARY_PATH)
  set(ssuffix .csh)
  set(scomment \#)
  set(libprefix ${CMAKE_SHARED_LIBRARY_PREFIX})
  set(libsuffix ${CMAKE_SHARED_LIBRARY_SUFFIX})
  set(localruntimedir ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
  set(runtimedir ${CMAKE_INSTALL_LIBDIR})
endif()

if(soversion)
  set(XBOX_LIBRARY_PROPERTIES ${XBOX_LIBRARY_PROPERTIES}
      VERSION ${XBOX_VERSION}
      SOVERSION ${XBOX_MAJOR_VERSION}.${XBOX_MINOR_VERSION}
      SUFFIX ${libsuffix}
      PREFIX ${libprefix} )
else()
  set(XBOX_LIBRARY_PROPERTIES ${XBOX_LIBRARY_PROPERTIES}
      SUFFIX ${libsuffix}
      PREFIX ${libprefix}
      IMPORT_PREFIX ${libprefix} )
endif()

set(CMAKE_VERBOSE_MAKEFILES OFF)
set(CMAKE_INCLUDE_CURRENT_DIR OFF)

include(CMakeParseArguments)

#---------------------------------------------------------------------------------------------------
# XBOX_GLOB_FILES( <variable> [REALTIVE path] [FILTER regexp] <sources> ...)
#---------------------------------------------------------------------------------------------------
function(XBOX_GLOB_FILES variable)
  CMAKE_PARSE_ARGUMENTS(ARG "RECURSE" "RELATIVE;FILTER" "" ${ARGN})
  set(_possibly_recurse "")
  if (ARG_RECURSE)
    set(_possibly_recurse "_RECURSE")
  endif()
  if(ARG_RELATIVE)
    file(GLOB${_possibly_recurse} _sources RELATIVE ${ARG_RELATIVE} ${ARG_UNPARSED_ARGUMENTS})
  else()
    file(GLOB${_possibly_recurse} _sources ${ARG_UNPARSED_ARGUMENTS})
  endif()
  if(ARG_FILTER)
    foreach(s ${_sources})
      if(s MATCHES ${ARG_FILTER})
        list(REMOVE_ITEM _sources ${s})
      endif()
    endforeach()
  endif()
  set(${variable} ${_sources} PARENT_SCOPE)
endfunction()

#---------------------------------------------------------------------------------------------------
# XBOX_GLOB_SOURCES( <variable> [REALTIVE path] [FILTER regexp] <sources> ...)
#---------------------------------------------------------------------------------------------------
function(XBOX_GLOB_SOURCES variable)
  XBOX_GLOB_FILES(_sources FILTER "(^|/)G__" ${ARGN})
  set(${variable} ${_sources} PARENT_SCOPE)
endfunction()

#---------------------------------------------------------------------------------------------------
# XBOX_GLOB_HEADERS( <variable> [REALTIVE path] [FILTER regexp] <sources> ...)
#---------------------------------------------------------------------------------------------------
function(XBOX_GLOB_HEADERS variable)
  XBOX_GLOB_FILES(_sources FILTER "LinkDef" ${ARGN})
  set(${variable} ${_sources} PARENT_SCOPE)
endfunction()

#---------------------------------------------------------------------------------------------------
# XBOX_GET_SOURCES( <variable> cwd <sources> ...)
#---------------------------------------------------------------------------------------------------
function(XBOX_GET_SOURCES variable cwd )
  set(sources)
  foreach( fp ${ARGN})
    if( IS_ABSOLUTE ${fp})
      file(GLOB files ${fp})
    endif()
    if(files)
      foreach(s ${files})
        if(fp MATCHES "[*]" AND s MATCHES "(^|/)G__") # Eliminate G__* files
        elseif(s MATCHES "${cwd}/G__")
          set(sources ${fp} ${sources})
        else()
          set(sources ${sources} ${s})
        endif()
      endforeach()
    else()
      if(fp MATCHES "(^|/)G__")
        set(sources ${fp} ${sources})
      else()
        set(sources ${sources} ${fp})
      endif()
    endif()
  endforeach()
  set(${variable} ${sources} PARENT_SCOPE)
endfunction()

#---------------------------------------------------------------------------------------------------
# REFLEX_GENERATE_DICTIONARY( dictionary headerfiles SELECTION selectionfile OPTIONS opt1 opt2 ...)
#---------------------------------------------------------------------------------------------------
macro(REFLEX_GENERATE_DICTIONARY dictionary)
  CMAKE_PARSE_ARGUMENTS(ARG "" "SELECTION" "OPTIONS" ${ARGN})
  #---Get List of header files---------------
  set(headerfiles)
  foreach(fp ${ARG_UNPARSED_ARGUMENTS})
    file(GLOB files include/${fp})
    if(files)
      foreach(f ${files})
        if(NOT f MATCHES LinkDef)
          set(headerfiles ${headerfiles} ${f})
        endif()
      endforeach()
    elseif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${fp})
      set(headerfiles ${headerfiles} ${CMAKE_CURRENT_SOURCE_DIR}/${fp})
    else()
      set(headerfiles ${headerfiles} ${fp})
    endif()
  endforeach()
  #---Get Selection file------------------------------------
  if(IS_ABSOLUTE ${ARG_SELECTION})
    set(selectionfile ${ARG_SELECTION})
  else()
    set(selectionfile ${CMAKE_CURRENT_SOURCE_DIR}/${ARG_SELECTION})
  endif()

  set(gensrcdict ${dictionary}.cxx)

  #---roottest compability---------------------------------
  if(CMAKE_XBOXTEST_NOXBOXMAP)
    set(rootmapname )
    set(rootmapopts )
  elseif(DEFINED CMAKE_XBOXTEST_NOXBOXMAP)  # Follow the roottest dictionary library naming
    set(rootmapname ${dictionary}.rootmap)
    set(rootmapopts --rootmap=${rootmapname} --rootmap-lib=${libprefix}${dictionary}_dictrflx)
  else()
    set(rootmapname ${dictionary}Dict.rootmap)
    set(rootmapopts --rootmap=${rootmapname} --rootmap-lib=${libprefix}${dictionary}Dict)
  endif()

  set(include_dirs -I${CMAKE_CURRENT_SOURCE_DIR})
  get_directory_property(incdirs INCLUDE_DIRECTORIES)
  foreach( d ${incdirs})
    if(NOT "${d}" MATCHES "AFTER|BEFORE|INTERFACE|PRIVATE|PUBLIC|SYSTEM")
      set(include_dirs ${include_dirs} -I${d})
    endif()
  endforeach()

  get_directory_property(defs COMPILE_DEFINITIONS)
  foreach( d ${defs})
   set(definitions ${definitions} -D${d})
  endforeach()

  add_custom_command(
    OUTPUT ${gensrcdict} ${rootmapname}
    COMMAND ${XBOX_genreflex_CMD}
    ARGS ${headerfiles} -o ${gensrcdict} ${rootmapopts} --select=${selectionfile}
         --gccxmlpath=${GCCXML_home}/bin ${ARG_OPTIONS} ${include_dirs} ${definitions}
    DEPENDS ${headerfiles} ${selectionfile})

  #---roottest compability---------------------------------
  if(CMAKE_XBOXTEST_DICT)
    XBOXTEST_TARGETNAME_FROM_FILE(targetname ${dictionary})

    set(targetname "${targetname}-dictgen")

    add_custom_target(${targetname} DEPENDS ${gensrcdict} ${XBOX_LIBRARIES})
  else()
    set(targetname "${dictionary}-dictgen")
    # Creating this target at ALL level enables the possibility to generate dictionaries (genreflex step)
    # well before the dependent libraries of the dictionary are build
    add_custom_target(${targetname} ALL DEPENDS ${gensrcdict})
  endif()

endmacro()

#---------------------------------------------------------------------------------------------------
# XBOX_GENERATE_DICTIONARY( dictionary headerfiles MODULE module DEPENDENCIES dep1 dep2
#                                                    BUILTINS dep1 dep2
#                                                    STAGE1 LINKDEF linkdef OPTIONS opt1 opt2 ...)
#---------------------------------------------------------------------------------------------------
function(XBOX_GENERATE_DICTIONARY dictionary)
  CMAKE_PARSE_ARGUMENTS(ARG "STAGE1;MULTIDICT;NOINSTALL" "MODULE" "LINKDEF;OPTIONS;DEPENDENCIES;BUILTINS" ${ARGN})

  # Check if OPTIONS start with a dash.
  if (ARG_OPTIONS)
    foreach(ARG_O ${ARG_OPTIONS})
      if (NOT ARG_O MATCHES "^-*")
        message(FATAL_ERROR "Wrong rootcling option: ${ARG_OPTIONS}")
      endif()
    endforeach()
  endif(ARG_OPTIONS)

  #---roottest compability---------------------------------
  if(CMAKE_XBOXTEST_DICT)
    set(CMAKE_INSTALL_LIBDIR ${CMAKE_CURRENT_BINARY_DIR})
    set(libprefix "")
  endif()

  #---Get the list of include directories------------------
  get_directory_property(incdirs INCLUDE_DIRECTORIES)
  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/include)
    set(localinclude ${CMAKE_CURRENT_SOURCE_DIR}/include)
  else()
    set(localinclude ${CMAKE_CURRENT_SOURCE_DIR})
  endif()

  #---Get the list of header files-------------------------
  set(headerfiles)
  set(_list_of_header_dependencies)
  foreach(fp ${ARG_UNPARSED_ARGUMENTS})
    if(${fp} MATCHES "[*?]") # Is this header a globbing expression?
      file(GLOB files include/${fp} ${fp})
      foreach(f ${files})
        if(NOT f MATCHES LinkDef) # skip LinkDefs from globbing result
          list(APPEND headerfiles ${f})
          list(APPEND _list_of_header_dependencies ${f})
        endif()
      endforeach()
    elseif(CMAKE_PROJECT_NAME STREQUAL XBOX AND
           EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${fp}) # only for XBOX project
      list(APPEND headerfiles ${CMAKE_CURRENT_SOURCE_DIR}/${fp})
      list(APPEND _list_of_header_dependencies ${CMAKE_CURRENT_SOURCE_DIR}/${fp})
    elseif(IS_ABSOLUTE ${fp})
      list(APPEND headerfiles ${fp})
      list(APPEND _list_of_header_dependencies ${fp})
    else()
      find_file(headerFile ${fp} HINTS ${localinclude} ${incdirs} NO_DEFAULT_PATH)
      find_file(headerFile ${fp} NO_SYSTEM_ENVIRONMENT_PATH)
      if(headerFile)
        list(APPEND headerfiles ${headerFile})
        list(APPEND _list_of_header_dependencies ${headerFile})
      else()
        list(APPEND headerfiles ${fp})
      endif()
      unset(headerFile CACHE)
    endif()
  endforeach()
  string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/include/" ""  headerfiles "${headerfiles}")
  # Replace the non-standard folder layout of Core.
  if (ARG_STAGE1 AND ARG_MODULE STREQUAL "Core")
    # FIXME: Glob these folders.
    set(core_folders base lz4 zip)
    foreach(core_folder ${core_folders})
      string(REPLACE "${CMAKE_SOURCE_DIR}/core/${core_folder}/include/" ""  headerfiles "${headerfiles}")
    endforeach()
  endif()


  #---Get the list of include directories ------------------
  if(CMAKE_PROJECT_NAME STREQUAL XBOX)
    set(includedirs -I${CMAKE_SOURCE_DIR}
                    -I${CMAKE_BINARY_DIR}/include)
    set(excludepaths ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})
  elseif(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/include)
    set(includedirs -I${CMAKE_CURRENT_SOURCE_DIR}/include)
  endif()
  foreach( d ${incdirs})
   set(includedirs ${includedirs} -I${d})
  endforeach()

  #---Append include directory of ROOT ---------------------
  #set(includedirs ${includedirs} -I${ROOT_INCLUDE_DIR})

  foreach(dep ${ARG_DEPENDENCIES})
    if(TARGET ${dep})
      get_property(dep_include_dirs TARGET ${dep} PROPERTY INCLUDE_DIRECTORIES)
      foreach(d ${dep_include_dirs})
        set(includedirs ${includedirs} -I${d})
      endforeach()
    endif()
  endforeach()
  list(REMOVE_DUPLICATES includedirs)

  #---Get the list of definitions---------------------------
  get_directory_property(defs COMPILE_DEFINITIONS)
  foreach( d ${defs})
   if((NOT d MATCHES "=") AND (NOT d MATCHES "^[$]<.*>$")) # avoid generator expressions
     set(definitions ${definitions} -D${d})
   endif()
  endforeach()

  #---Get LinkDef.h file------------------------------------
  foreach( f ${ARG_LINKDEF})
    if( IS_ABSOLUTE ${f})
      set(_linkdef ${_linkdef} ${f})
    else()
      if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/include/${f})
        set(_linkdef ${_linkdef} ${CMAKE_CURRENT_SOURCE_DIR}/include/${f})
      else()
        set(_linkdef ${_linkdef} ${CMAKE_CURRENT_SOURCE_DIR}/${f})
      endif()
    endif()
  endforeach()


  #---Build the names for library, pcm and rootmap file ----
  get_filename_component(dict_base_name ${dictionary} NAME_WE)
  if(dict_base_name MATCHES "^G__")
    string(SUBSTRING ${dictionary} 3 -1 deduced_arg_module)
  else()
    set(deduced_arg_module ${dict_base_name})
  endif()

  #---Set the library output directory-----------------------
  if(DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY AND NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY STREQUAL "")
    set(library_output_dir ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
  else()
    set(library_output_dir ${CMAKE_CURRENT_BINARY_DIR})
  endif()

  if(ARG_MODULE)
    set(library_name ${libprefix}${ARG_MODULE}${libsuffix})
    if(ARG_MULTIDICT)
      set(newargs -s ${library_output_dir}/${library_name} -multiDict)
      set(pcm_name ${library_output_dir}/${libprefix}${ARG_MODULE}_${dictionary}_rdict.pcm)
      set(rootmap_name ${library_output_dir}/${libprefix}${deduced_arg_module}.rootmap)
    else()
      set(newargs -s ${library_output_dir}/${library_name})
      set(pcm_name ${library_output_dir}/${libprefix}${ARG_MODULE}_rdict.pcm)
      set(rootmap_name ${library_output_dir}/${libprefix}${ARG_MODULE}.rootmap)
      set(cpp_module ${ARG_MODULE})
      if(runtime_cxxmodules)
        set(cpp_module_file ${library_output_dir}/${cpp_module}.pcm)
      endif()
    endif()
  else()
    set(library_name ${libprefix}${deduced_arg_module}${libsuffix})
    set(newargs -s ${library_output_dir}/${library_name})
    set(pcm_name ${library_output_dir}/${libprefix}${deduced_arg_module}_rdict.pcm)
    set(rootmap_name ${library_output_dir}/${libprefix}${deduced_arg_module}.rootmap)
  endif()

  if(CMAKE_XBOXTEST_NOXBOXMAP)
    set(rootmapname )
    set(rootmapargs )
  else()
    set(rootmapargs -rml ${library_name} -rmf ${rootmap_name})
  endif()

  #---Get the library and module dependencies-----------------
  if(ARG_DEPENDENCIES)
    foreach(dep ${ARG_DEPENDENCIES})
      set(newargs ${newargs} -m  ${libprefix}${dep}_rdict.pcm)
    endforeach()
  endif()

  if(runtime_cxxmodules AND ARG_MODULE)
    # FIXME: Once modules work better, we should use some other value like "1"
    # to disable the module-build remarks from clang.
    set(runtime_cxxmodules_env "XBOX_MODULES=DEBUG")
  endif()

  #---what rootcling command to use--------------------------
  if(CMAKE_PROJECT_NAME STREQUAL XBOX)
    set(command ${CMAKE_COMMAND} -E env "LD_LIBRARY_PATH=${CMAKE_BINARY_DIR}/lib:$ENV{LD_LIBRARY_PATH}"
                "XBOXIGNOREPREFIX=1" "${runtime_cxxmodules_env}" ${ROOT_CINT_EXECUTABLE})
    set(XBOXCINTDEP ${ROOT_CINT_EXECUTABLE})
  else()
    set(command ${CMAKE_COMMAND} -E env "${runtime_cxxmodules_env}" ${ROOT_CINT_EXECUTABLE})
  endif()

  #---build the path exclusion switches----------------------
  set(excludepathsargs "")
  foreach(excludepath ${excludepaths})
    set(excludepathsargs ${excludepathsargs} -excludePath ${excludepath})
  endforeach()

  #---build the implicit dependencies arguments
  foreach(_dep ${_linkdef} ${_list_of_header_dependencies})
    list(APPEND _implicitdeps CXX ${_dep})
  endforeach()

  #message(${_list_of_header_dependencies})
  #message(${_linkdef})
  #message(${XBOXCINTDEP})
  #message(${implicitdeps})
  #message(${definitions})
  #message(${includedirs})

  #---call rootcint------------------------------------------
  add_custom_command(OUTPUT ${dictionary}.cxx ${pcm_name} ${rootmap_name} ${cpp_module_file}
                     COMMAND ${command} -v2 -f  ${dictionary}.cxx ${newargs} ${excludepathsargs} ${rootmapargs}
                                        ${ARG_OPTIONS} ${definitions} ${includedirs} ${headerfiles} ${_linkdef}
                     IMPLICIT_DEPENDS ${_implicitdeps}
                     DEPENDS ${_list_of_header_dependencies} ${_linkdef} ${XBOXCINTDEP} ${ARG_DEPENDENCIES})
  get_filename_component(dictname ${dictionary} NAME)

  #---roottest compability
  add_custom_target(${dictname} DEPENDS ${dictionary}.cxx ${pcm_name} ${rootmap_name} ${cpp_module_file})
  if(NOT ARG_NOINSTALL AND NOT CMAKE_XBOXTEST_DICT AND DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    set_property(GLOBAL APPEND PROPERTY XBOX_DICTIONARY_TARGETS ${dictname})
    set_property(GLOBAL APPEND PROPERTY XBOX_DICTIONARY_FILES ${CMAKE_CURRENT_BINARY_DIR}/${dictionary}.cxx)

    if(ARG_STAGE1)
      install(FILES ${rootmap_name}
                    DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libraries)
    else()
      install(FILES ${pcm_name} ${rootmap_name}
                    DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libraries)
    endif()
  endif()

  if(ARG_BUILTINS)
    foreach(arg1 ${ARG_BUILTINS})
      if(${arg1}_TARGET)
        add_dependencies(${dictname} ${${arg1}_TARGET})
      endif()
    endforeach()
  endif()
  # FIXME: Support mulptiple dictionaries. In some cases (libSMatrix and
  # libGenVector) we have to have two or more dictionaries (eg. for math,
  # we need the two for double vs Double32_t template specializations).
  # In some other cases, eg. libTreePlayer.so we add in a separate dictionary
  # files which for some reason (temporarily?) cannot be put in the PCH. Eg.
  # all rest of the first dict is in the PCH but this file is not and it
  # cannot be present in the original dictionary.
  if(cpp_module)
    #XBOX_CXXMODULES_APPEND_TO_MODULEMAP("${cpp_module}" "${headerfiles}")
  endif()
endfunction()

#---------------------------------------------------------------------------------------------------
# XBOX_CXXMODULES_APPEND_TO_MODULEMAP( library library_headers )
#---------------------------------------------------------------------------------------------------
function (XBOX_CXXMODULES_APPEND_TO_MODULEMAP library library_headers)
  XBOX_FIND_DIRS_WITH_HEADERS(dirs)

  # Variable 'dirs' is the return result of XBOX_FIND_DIRS_WITH_HEADERS.
  if(NOT DEFINED dirs)
    message(SEND_ERROR "Error, the variable ${dirs} is not defined!")
  endif()

  set(found_headers "")
  set(dir_headers "")
  foreach(d ${dirs})
    XBOX_GLOB_FILES(dir_headers
                    RECURSE
                    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/${d}
                    FILTER "LinkDef" ${d}/*)
    list(APPEND found_headers "${dir_headers}")
  endforeach()

  if (APPLE)
    # FIXME: Krb5Auth.h triggers "declaration of '__mb_cur_max' has a different language linkage"
    # problem.
    # FIXME: error: declaration of 'NSObject' must be imported from module 'XBOX.libBonjour.so.TBonjourBrowser.h' before it is required
    if (${library} MATCHES "Krb5Auth" OR ${library} MATCHES "(GCocoa|GQuartz)")
      return()
    endif()
  endif(APPLE)

  set(excluded_headers RConfig.h RVersion.h RtypesImp.h TVersionCheck.h
                        Rtypes.h RtypesCore.h TClassEdit.h
                        TIsAProxy.h TVirtualIsAProxy.h
                        DllImport.h TGenericClassInfo.h
                        TSchemaHelper.h ESTLType.h RStringView.h Varargs.h
                        RootMetaSelection.h libcpp_string_view.h
                        RWrap_libcpp_string_view.h
                        ThreadLocalStorage.h
                        TBranchProxyTemplate.h TGLIncludes.h TGLWSIncludes.h
                        snprintf.h strlcpy.h)

   # Deprecated header files.
  set (excluded_headers "${excluded_headers}")

  set(modulemap_entry "module \"${library}\" {")
  # For modules GCocoa and GQuartz we need objc context.
  if (${library} MATCHES "(GCocoa|GQuartz)")
    set (modulemap_entry "${modulemap_entry}\n  requires objc\n")
  else()
    set (modulemap_entry "${modulemap_entry}\n  requires cplusplus\n")
  endif()
  if (library_headers)
    set(found_headers ${library_headers})
  endif()
  foreach(header ${found_headers})
    set(textual_header "")
    if (${header} MATCHES ".*\\.icc$")
      set(textual_header "textual ")
    endif()
    # Check if header is in included header list
    set(is_excluded NO)
    foreach(excluded_header ${excluded_headers})
      if(${header} MATCHES ${excluded_header})
        set(is_excluded YES)
        break()
      endif()
    endforeach()
    if(NOT is_excluded)
      set(modulemap_entry "${modulemap_entry}  module \"${header}\" { ${textual_header}header \"${header}\" export * }\n")
    endif()
  endforeach()
  #set(modulemap_entry "${modulemap_entry}  link \"lib/${library}\"\n")
  set(modulemap_entry "${modulemap_entry}  export *\n}\n\n")
  # Non XBOX projects need a modulemap generated for them in the current
  # directory. The same happens with test dictionaries in XBOX which are not
  # exposed via the main modulemap. This is exposed by setting the
  # XBOX_CXXMODULES_WRITE_TO_CURRENT_DIR.
  if (NOT "${CMAKE_PROJECT_NAME}" STREQUAL XBOX OR XBOX_CXXMODULES_WRITE_TO_CURRENT_DIR)
    set(modulemap_output_file "${CMAKE_CURRENT_BINARY_DIR}/module.modulemap")

    # It's possible that multiple modulemaps are needed in the current
    # directory and we need to merge them. As we don't want to have multiple
    # modules in the same moduluemap when rerunning CMake, we do a quick
    # check if the current module is already in the modulemap (in which case
    # we know we rerun CMake at the moment and start writing a new modulemap
    # instead of appending new modules).

    # The string we use to identify if the current module is already in the
    # modulemap.
    set(modulemap_needle "module \"${library}\"")
    # Check if the needle is in the modulemap. If the file doesn't exist
    # we just pretend we didn't found the string in the modulemap.
    set(match_result -1)
    if (EXISTS "${modulemap_output_file}")
      file(READ "${modulemap_output_file}" existing_contents)
      string(FIND "${existing_contents}" "${modulemap_needle}" match_result)
    endif()
    # Append our new module to the existing modulemap containing other modules.
    if(${match_result} EQUAL -1)
      file(APPEND "${modulemap_output_file}" "${modulemap_entry}")
    else()
      file(WRITE "${modulemap_output_file}" "${modulemap_entry}")
    endif()

    # Sanity check that the string we're looking for is actually in the content
    # we're writing to this file.
    string(FIND "${modulemap_entry}" "${modulemap_needle}" match_result)
    if(${match_result} EQUAL -1)
      message(AUTHOR_WARNING "Couldn't find module declaration in modulemap file."
                             "This would break the modulemap generation when "
                             " rerunning CMake. Module needle was "
                             "'${modulemap_needle}' and the content was '${modulemap_entry}'")
    endif()

  else()
    set_property(GLOBAL APPEND PROPERTY XBOX_CXXMODULES_EXTRA_MODULEMAP_CONTENT ${modulemap_entry})
  endif()
endfunction()

#---------------------------------------------------------------------------------------------------
# XBOX_LINKER_LIBRARY( <name> source1 source2 ...[TYPE STATIC|SHARED] [DLLEXPORT]
#                        [NOINSTALL] LIBRARIES library1 library2 ...
#                        BUILTINS dep1 dep2)
#---------------------------------------------------------------------------------------------------
function(XBOX_LINKER_LIBRARY library)
  CMAKE_PARSE_ARGUMENTS(ARG "DLLEXPORT;CMAKENOEXPORT;TEST;NOINSTALL" "TYPE" "LIBRARIES;DEPENDENCIES;BUILTINS;COMPILEDEF"  ${ARGN})
  XBOX_GET_SOURCES(lib_srcs src ${ARG_UNPARSED_ARGUMENTS})

  #message("Begin")
  #message(${lib_srcs})
  #message(${ARG_UNPARSED_ARGUMENTS})

  if(NOT ARG_TYPE)
    set(ARG_TYPE SHARED)
  endif()
  if(ARG_TEST) # we are building a test, so add EXCLUDE_FROM_ALL
    set(_all EXCLUDE_FROM_ALL)
  endif()
  include_directories(BEFORE ${CMAKE_BINARY_DIR}/include)
  set(library_name ${library})
  if(TARGET ${library})
    message("Target ${library} already exists. Renaming target name to ${library}_new")
    set(library ${library}_new)
  endif()
  if(WIN32 AND ARG_TYPE STREQUAL SHARED AND NOT ARG_DLLEXPORT)
    #---create a list of all the object files-----------------------------
    if(CMAKE_GENERATOR MATCHES "Visual Studio")
      set(library_name ${libprefix}${library})
      #foreach(src1 ${lib_srcs})
      #  if(NOT src1 MATCHES "[.]h$|[.]icc$|[.]hxx$|[.]hpp$")
      #    string (REPLACE ${CMAKE_CURRENT_SOURCE_DIR} "" src2 ${src1})
      #    string (REPLACE ${CMAKE_CURRENT_BINARY_DIR} "" src3 ${src2})
      #    string (REPLACE ".." "__" src ${src3})
      #    get_filename_component(name ${src} NAME_WE)
      #    set(lib_objs ${lib_objs} ${library}.dir/${CMAKE_CFG_INTDIR}/${name}.obj)
      #  endif()
      #endforeach()
      set(lib_objs ${lib_objs} ${library}.dir/${CMAKE_CFG_INTDIR}/*.obj)
    else()
      foreach(src1 ${lib_srcs})
        if(NOT src1 MATCHES "[.]h$|[.]icc$|[.]hxx$|[.]hpp$")
          string (REPLACE ${CMAKE_CURRENT_SOURCE_DIR} "" src2 ${src1})
          string (REPLACE ${CMAKE_CURRENT_BINARY_DIR} "" src3 ${src2})
          string (REPLACE ".." "__" src ${src3})
          get_filename_component(name ${src} NAME)
          get_filename_component(path ${src} PATH)
          set(lib_objs ${lib_objs} ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${library}.dir/${path}/${name}.obj)
        endif()
      endforeach()
    endif()
    #---create a shared library with the .def file------------------------
    add_library(${library} ${_all} SHARED ${lib_srcs})

	if(ARG_COMPILEDEF)
      target_compile_definitions(${library} PRIVATE ${ARG_COMPILEDEF})
    endif()

    target_link_libraries(${library} ${ARG_LIBRARIES} ${ARG_DEPENDENCIES})
    set_target_properties(${library} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
  else()
    #---Need to add a dummy source file if all sources are OBJECT libraries (Xcode, ...)
    if(NOT lib_srcs MATCHES "(^|[;])[^$][^<]")
      add_custom_command(OUTPUT dummy.cxx COMMAND ${CMAKE_COMMAND} -E touch dummy.cxx)
      set(lib_srcs ${lib_srcs} dummy.cxx)
    endif()
    add_library( ${library} ${_all} ${ARG_TYPE} ${lib_srcs})

	if(ARG_COMPILEDEF)
      target_compile_definitions(${library} PRIVATE ${ARG_COMPILEDEF})
    endif()

    if(ARG_TYPE STREQUAL SHARED)
      set_target_properties(${library} PROPERTIES  ${XBOX_LIBRARY_PROPERTIES} )
    endif()
    if(explicitlink OR XBOX_explicitlink_FOUND)
      target_link_libraries(${library} PUBLIC ${ARG_LIBRARIES} ${ARG_DEPENDENCIES})
    else()
      target_link_libraries(${library} PUBLIC ${ARG_LIBRARIES})
    endif()
  endif()
  if(TARGET G__${library})
    add_dependencies(${library} G__${library})
  else()
    # Uncomment to see if we maybe forgot to add a dependency between linking
    # a dictionary and generating the G__*.cxx file. We can't have this by
    # default because right now quite few dictionaries don't have the associated
    # XBOX_GENERATE_DICTIONARY call that prevents this warning.
    #message(AUTHOR_WARNING "Couldn't find target: " ${library} "\n Forgot to call XBOX_GENERATE_DICTIONARY?")
  endif()
  if(CMAKE_PROJECT_NAME STREQUAL XBOX)
    add_dependencies(${library} move_headers)
  endif()
  set_property(GLOBAL APPEND PROPERTY XBOX_EXPORTED_TARGETS ${library})
  set_target_properties(${library} PROPERTIES OUTPUT_NAME ${library_name})
  set_target_properties(${library} PROPERTIES INTERFACE_LINK_LIBRARIES "${ARG_DEPENDENCIES}")
  target_include_directories(${library} INTERFACE $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
  # Do not add -Dname_EXPORTS to the command-line when building files in this
  # target. Doing so is actively harmful for the modules build because it
  # creates extra module variants, and not useful because we don't use these
  # macros.
  set_target_properties(${library} PROPERTIES DEFINE_SYMBOL "")
  if(ARG_BUILTINS)
    foreach(arg1 ${ARG_BUILTINS})
      if(${arg1}_TARGET)
        add_dependencies(${library} ${${arg1}_TARGET})
      endif()
    endforeach()
  endif()

  #----Installation details-------------------------------------------------------
  if(NOT ARG_TEST AND NOT ARG_NOINSTALL AND CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    if(ARG_CMAKENOEXPORT)
      install(TARGETS ${library} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT libraries
                                 LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libraries
                                 ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libraries)
    else()
      install(TARGETS ${library} EXPORT ${CMAKE_PROJECT_NAME}Exports
                                 RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT libraries
                                 LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libraries
                                 ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libraries)
    endif()
    if(WIN32 AND ARG_TYPE STREQUAL SHARED)
      install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/lib${library}.pdb
                    CONFIGURATIONS Debug RelWithDebInfo
                    DESTINATION ${CMAKE_INSTALL_BINDIR}
                    COMPONENT libraries)
    endif()
  endif()
endfunction()

#---------------------------------------------------------------------------------------------------
# XBOX_OBJECT_LIBRARY( <name> source1 source2 ... BUILTINS dep1 dep2 ...)
#---------------------------------------------------------------------------------------------------
function(XBOX_OBJECT_LIBRARY library)
  CMAKE_PARSE_ARGUMENTS(ARG "" "" "DEPENDENCIES;BUILTINS;COMPILEDEF"  ${ARGN})
  XBOX_GET_SOURCES(lib_srcs src ${ARG_UNPARSED_ARGUMENTS})
  include_directories(BEFORE ${CMAKE_BINARY_DIR}/include)
  add_library(${library} OBJECT ${lib_srcs})

  if(ARG_COMPILEDEF)
      target_compile_definitions(${library} PRIVATE ${ARG_COMPILEDEF})
  endif()
  target_link_libraries(${library} PRIVATE ${ARG_DEPENDENCIES})
	
  if(lib_srcs MATCHES "(^|/)(G__[^.]*)[.]cxx.*")
     add_dependencies(${library} ${CMAKE_MATCH_2})
  endif()
  add_dependencies(${library} move_headers)

  #--- Only for building shared libraries
  set_property(TARGET ${library} PROPERTY POSITION_INDEPENDENT_CODE 1)
  # Do not add -Dname_EXPORTS to the command-line when building files in this
  # target. Doing so is actively harmful for the modules build because it
  # creates extra module variants, and not useful because we don't use these
  # macros.
  set_target_properties(${library} PROPERTIES DEFINE_SYMBOL "")
  if(WIN32)
    set_target_properties(${library} PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
  endif()

  if(ARG_BUILTINS)
    foreach(arg1 ${ARG_BUILTINS})
      if(${arg1}_TARGET)
        add_dependencies(${library} ${${arg1}_TARGET})
      endif()
    endforeach()
  endif()

  #--- Fill the property OBJECTS with all the object files
  #    This is needed becuase the generator expression $<TARGET_OBJECTS:target>
  #    does not get expanded when used in custom command dependencies
  get_target_property(sources ${library} SOURCES)
  foreach(s ${sources})
    if(CMAKE_GENERATOR MATCHES Xcode)
      get_filename_component(name ${s} NAME_WE)
      set(obj ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.build/${CMAKE_CFG_INTDIR}/${library}.build/Objects-normal/x86_64/${name}${CMAKE_CXX_OUTPUT_EXTENSION})
    else()
      if(IS_ABSOLUTE ${s})
        string(REGEX REPLACE "([][.?*+|()$^-])" "\\\\\\1" escaped_source_dir "${CMAKE_CURRENT_SOURCE_DIR}")
        string(REGEX REPLACE "([][.?*+|()$^-])" "\\\\\\1" escaped_binary_dir "${CMAKE_CURRENT_BINARY_DIR}")
        if(${s} MATCHES "^${escaped_source_dir}")
          string(REPLACE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${library}.dir src ${s})
        elseif(${s} MATCHES "^${escaped_binary_dir}")
          string(REPLACE ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${library}.dir src ${s})
        else()
          #message(WARNING "Unknown location of source ${s} for object library ${library}")
        endif()
      else()
        set(src ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${library}.dir/${s})
      endif()
      set(obj ${src}${CMAKE_CXX_OUTPUT_EXTENSION})
    endif()
    set_property(TARGET ${library} APPEND PROPERTY OBJECTS ${obj})
  endforeach()
endfunction()

#---------------------------------------------------------------------------------------------------
# XBOX_MODULE_LIBRARY( <name> source1 source2 ... [DLLEXPORT] LIBRARIES library1 library2 ...)
#---------------------------------------------------------------------------------------------------
function(XBOX_MODULE_LIBRARY library)
  CMAKE_PARSE_ARGUMENTS(ARG "" "" "LIBRARIES" ${ARGN})
  XBOX_GET_SOURCES(lib_srcs src ${ARG_UNPARSED_ARGUMENTS})
  include_directories(BEFORE ${CMAKE_BINARY_DIR}/include)
  add_library( ${library} SHARED ${lib_srcs})
  add_dependencies(${library} move_headers)
  set_target_properties(${library}  PROPERTIES ${XBOX_LIBRARY_PROPERTIES})
  # Do not add -Dname_EXPORTS to the command-line when building files in this
  # target. Doing so is actively harmful for the modules build because it
  # creates extra module variants, and not useful because we don't use these
  # macros.
  set_target_properties(${library} PROPERTIES DEFINE_SYMBOL "")

  target_link_libraries(${library} ${ARG_LIBRARIES})
  #----Installation details-------------------------------------------------------
  install(TARGETS ${library} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT libraries
                             LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libraries
                             ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libraries)
endfunction()

#---------------------------------------------------------------------------------------------------
# XBOX_GENERATE_ROOTMAP( library LINKDEF linkdef LIBRRARY lib DEPENDENCIES lib1 lib2 )
#---------------------------------------------------------------------------------------------------
function(XBOX_GENERATE_ROOTMAP library)
  return()   #--- No needed anymore
  CMAKE_PARSE_ARGUMENTS(ARG "" "LIBRARY" "LINKDEF;DEPENDENCIES" ${ARGN})
  get_filename_component(libname ${library} NAME_WE)
  get_filename_component(path ${library} PATH)

  #---Set the library output directory-----------------------
  if(DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    set(library_output_dir ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
  else()
    set(library_output_dir ${CMAKE_CURRENT_BINARY_DIR})
  endif()

  set(outfile ${library_output_dir}/${libprefix}${libname}.rootmap)
  foreach( f ${ARG_LINKDEF})
    if( IS_ABSOLUTE ${f})
      set(_linkdef ${_linkdef} ${f})
    else()
      set(_linkdef ${_linkdef} ${CMAKE_CURRENT_SOURCE_DIR}/include/${f})
    endif()
  endforeach()
  foreach(d ${ARG_DEPENDENCIES})
    get_filename_component(_ext ${d} EXT)
    if(_ext)
      set(_dependencies ${_dependencies} ${d})
    else()
      set(_dependencies ${_dependencies} ${libprefix}${d}${CMAKE_SHARED_LIBRARY_SUFFIX})
    endif()
  endforeach()
  if(ARG_LIBRARY)
    set(_library ${ARG_LIBRARY})
  else()
    set(_library ${libprefix}${library}${CMAKE_SHARED_LIBRARY_SUFFIX})
  endif()
  #---Build the rootmap file--------------------------------------
  #add_custom_command(OUTPUT ${outfile}
  #                   COMMAND ${rlibmap_cmd} -o ${outfile} -l ${_library} -d ${_dependencies} -c ${_linkdef}
  #                   DEPENDS ${_linkdef} ${rlibmap_cmd} )
  add_custom_target( ${libprefix}${library}.rootmap ALL DEPENDS  ${outfile})
  set_target_properties(${libprefix}${library}.rootmap PROPERTIES FOLDER RootMaps )
  #---Install the rootmap file------------------------------------
  install(FILES ${outfile} DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT libraries)
endfunction()

#---------------------------------------------------------------------------------------------------
# XBOX_FIND_DIRS_WITH_HEADERS([dir1 dir2 ...] OPTIONS [options])
#---------------------------------------------------------------------------------------------------
function(XBOX_FIND_DIRS_WITH_HEADERS result_dirs)
  if( ARGN )
    set(dirs ${ARGN})
  else()
    set(dirs include/)
  endif()
  set (${result_dirs} ${dirs} PARENT_SCOPE)
endfunction()

#---------------------------------------------------------------------------------------------------
# XBOX_INSTALL_HEADERS([dir1 dir2 ...] OPTIONS [options])
#---------------------------------------------------------------------------------------------------
function(XBOX_INSTALL_HEADERS)
  CMAKE_PARSE_ARGUMENTS(ARG "OPTIONS" "" "FILTER" ${ARGN})
  if (${ARG_OPTIONS})
    message(FATAL_ERROR "XBOX_INSTALL_HEADERS no longer supports the OPTIONS argument. Rewrite using the FILTER argument.")
  endif()
  XBOX_FIND_DIRS_WITH_HEADERS(dirs ${ARG_UNPARSED_ARGUMENTS})
  set (filter "LinkDef")
  set (options REGEX "LinkDef" EXCLUDE)
  foreach (f ${ARG_FILTER})
    set (filter "${filter}|${f}")
    set (options ${options} REGEX "${f}" EXCLUDE)
  endforeach()
  set (filter "(${filter})")
  string(REPLACE ${CMAKE_SOURCE_DIR} "" tgt ${CMAKE_CURRENT_SOURCE_DIR})
  string(MAKE_C_IDENTIFIER move_header${tgt} tgt)
  set_property(GLOBAL APPEND PROPERTY XBOX_HEADER_TARGETS ${tgt})
  foreach(d ${dirs})
    install(DIRECTORY ${d} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
                           COMPONENT headers
                           ${options})
    string(REGEX REPLACE "(.*)/$" "\\1" d ${d})
    XBOX_GLOB_FILES(include_files
      RECURSE
      RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/${d}
      FILTER ${filter}
      ${d}/*.h ${d}/*.hxx ${d}/*.icc )
    foreach (include_file ${include_files})
      set (src ${CMAKE_CURRENT_SOURCE_DIR}/${d}/${include_file})
      set (dst ${CMAKE_BINARY_DIR}/include/${include_file})
      add_custom_command(
        OUTPUT ${dst}
        COMMAND ${CMAKE_COMMAND} -E copy ${src} ${dst}
        COMMENT "Copying header ${src} to ${CMAKE_BINARY_DIR}/include"
        DEPENDS ${src})
      list(APPEND dst_list ${dst})
    endforeach()
    set_property(GLOBAL APPEND PROPERTY XBOX_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/${d})
  endforeach()
  add_custom_target(${tgt} DEPENDS ${dst_list})
endfunction()

#---------------------------------------------------------------------------------------------------
# XBOX_STANDARD_LIBRARY_PACKAGE(libname
#                                 [NO_INSTALL_HEADERS]         : don't install headers for this package
#                                 [STAGE1]                     : use rootcling_stage1 for generating
#                                 HEADERS header1 header2      : if not specified, globbing for *.h is used)
#                                 [NO_HEADERS]                 : don't glob to fill HEADERS variable
#                                 SOURCES source1 source2      : if not specified, globbing for *.cxx is used)
#                                 [NO_SOURCES]                 : don't glob to fill SOURCES variable
#                                 [OBJECT_LIBRARY]             : use XBOX_OBJECT_LIBRARY to generate object files
#                                                                and then use those for linking.
#                                 LIBRARIES lib1 lib2          : linking flags such as dl, readline
#                                 DEPENDENCIES lib1 lib2       : dependencies such as Core, MathCore
#                                 BUILTINS builtin1 builtin2   : builtins like AFTERIMAGE
#                                 LINKDEF LinkDef.h LinkDef2.h : linkdef files, default value is "LinkDef.h"
#                                 DICTIONARY_OPTIONS option    : options passed to rootcling
#                                 INSTALL_OPTIONS option       : options passed to install headers
#                                )
#---------------------------------------------------------------------------------------------------
function(ROOT_STANDARD_LIBRARY_PACKAGE libname)
  set(options NO_INSTALL_HEADERS STAGE1 NO_HEADERS NO_SOURCES OBJECT_LIBRARY)
  set(oneValueArgs)
  set(multiValueArgs DEPENDENCIES HEADERS SOURCES BUILTINS LIBRARIES DICTIONARY_OPTIONS LINKDEF INSTALL_OPTIONS)
  CMAKE_PARSE_ARGUMENTS(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  # Check if we have any unparsed arguments
  if(ARG_UNPARSED_ARGUMENTS)
    message(AUTHOR_WARNING "Unparsed arguments for XBOX_STANDARD_LIBRARY_PACKAGE: ${ARG_UNPARSED_ARGUMENTS}")
  endif()
  # Check that the user doesn't parse NO_HEADERS to disable globbing and HEADERS at the same time.
  if (ARG_HEADERS AND ARG_NO_HEADERS)
    message(AUTHOR_WARNING "HEADERS and NO_HEADERS arguments are mutually exclusive.")
  endif()
  if (ARG_SOURCES AND ARG_NO_SOURCES)
    message(AUTHOR_WARNING "SOURCES and NO_SOURCES arguments are mutually exclusive.")
  endif()

  # Set default values
  # If HEADERS/SOURCES are not parsed, we glob for those files.
  if (NOT ARG_HEADERS AND NOT ARG_NO_HEADERS)
    set(ARG_HEADERS "*.h")
  endif()
  if (NOT ARG_SOURCES AND NOT ARG_NO_SOURCES)
    set(ARG_SOURCES "*.cxx")
  endif()
  if (NOT ARG_LINKDEF)
    set(ARG_LINKDEF "LinkDef.h")
  endif()
  if (ARG_STAGE1)
    set(STAGE1_FLAG "STAGE1")
  endif()

  # add ROOT libraries to the list (Minimum required: Core library of ROOT to inherent class from TObject!)
  set(ARG_LIBRARIES ${ARG_LIBRARIES} ${ROOT_LIBRARIES})

  #message(${libname})
  #message(${ARG_HEADERS})
  #message(${ARG_LINKDEF})
  #message(${ARG_DICTIONARY_OPTIONS})
  #message(${ARG_BUILTINS})

  # generate ROOT dictionary 
  XBOX_GENERATE_DICTIONARY(${CMAKE_CURRENT_BINARY_DIR}/G__${libname} ${ARG_HEADERS}
                          MODULE ${libname}
                          ${STAGE1_FLAG}
                          LINKDEF ${ARG_LINKDEF}
                          OPTIONS ${ARG_DICTIONARY_OPTIONS}
                          DEPENDENCIES ${ARG_DEPENDENCIES}
                          BUILTINS ${ARG_BUILTINS}
                          )

  # create object or linker library
  if (ARG_OBJECT_LIBRARY)
    XBOX_OBJECT_LIBRARY(${libname} ${ARG_SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/G__${libname}.cxx)
    #XBOX_LINKER_LIBRARY(${libname} $<TARGET_OBJECTS:${libname}Objs>
    #                    LIBRARIES ${ARG_LIBRARIES}
    #                    DEPENDENCIES ${ARG_DEPENDENCIES}
    #                    BUILTINS ${ARG_BUILTINS}
    #                   )
  else(ARG_OBJECT_LIBRARY)
    XBOX_LINKER_LIBRARY(${libname} ${ARG_SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/G__${libname}.cxx
                        LIBRARIES ${ARG_LIBRARIES}
                        DEPENDENCIES ${ARG_DEPENDENCIES}
                        BUILTINS ${ARG_BUILTINS}
                       )
  endif(ARG_OBJECT_LIBRARY)

  # Install headers if we have any headers and if the user didn't explicitly disabled this.
  if (NOT ARG_NO_INSTALL_HEADERS OR ARG_NO_HEADERS)
    XBOX_INSTALL_HEADERS(${ARG_INSTALL_OPTIONS})
  endif()
endfunction()

#---------------------------------------------------------------------------------------------------
# XBOX_EXECUTABLE( <name> source1 source2 ... LIBRARIES library1 library2 ... BUILTINS dep1 dep2 ...)
#---------------------------------------------------------------------------------------------------
function(XBOX_EXECUTABLE executable)
  CMAKE_PARSE_ARGUMENTS(ARG "CMAKENOEXPORT;NOINSTALL;TEST" "" "LIBRARIES;BUILTINS;ADDITIONAL_COMPILE_FLAGS;COMPILEDEF"  ${ARGN})
  XBOX_GET_SOURCES(exe_srcs src ${ARG_UNPARSED_ARGUMENTS})
  set(executable_name ${executable})
  if(TARGET ${executable})
    message("Target ${executable} already exists. Renaming target name to ${executable}_new")
    set(executable ${executable}_new)
  endif()
  if(ARG_TEST) # we are building a test, so add EXCLUDE_FROM_ALL
    set(_all EXCLUDE_FROM_ALL)
  endif()
  include_directories(BEFORE ${CMAKE_BINARY_DIR}/include)
  add_executable( ${executable} ${_all} ${exe_srcs})
  target_link_libraries(${executable} ${ARG_LIBRARIES} )
  if(WIN32 AND ${executable} MATCHES .exe)
    set_target_properties(${executable} PROPERTIES SUFFIX "")
  endif()
  set_property(GLOBAL APPEND PROPERTY XBOX_EXPORTED_TARGETS ${executable})
  set_target_properties(${executable} PROPERTIES OUTPUT_NAME ${executable_name})
  if (ARG_ADDITIONAL_COMPILE_FLAGS)
    set_target_properties(${executable} PROPERTIES COMPILE_FLAGS ${ARG_ADDITIONAL_COMPILE_FLAGS})
  endif()
  add_dependencies(${executable} move_headers)
  if(ARG_BUILTINS)
    foreach(arg1 ${ARG_BUILTINS})
      if(${arg1}_TARGET)
        add_dependencies(${executable} ${${arg1}_TARGET})
      endif()
    endforeach()
  endif()

  if(ARG_COMPILEDEF)
    target_compile_definitions(${executable} PRIVATE ${ARG_COMPILEDEF})
  endif()

  #----Installation details------------------------------------------------------
  if(NOT ARG_NOINSTALL AND CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    if(ARG_CMAKENOEXPORT)
      install(TARGETS ${executable} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT applications)
    else()
      install(TARGETS ${executable} EXPORT ${CMAKE_PROJECT_NAME}Exports RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT applications)
    endif()
    if(WIN32)
      install(FILES ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${executable}.pdb
              CONFIGURATIONS Debug RelWithDebInfo
              DESTINATION ${CMAKE_INSTALL_BINDIR}
              COMPONENT applications)
    endif()
  endif()
endfunction()

#---------------------------------------------------------------------------------------------------
# XBOX_REFLEX_BUILD_DICTIONARY( dictionary headerfiles selectionfile OPTIONS opt1 opt2 ...  LIBRARIES lib1 lib2 ... )
#---------------------------------------------------------------------------------------------------
function(XBOX_REFLEX_BUILD_DICTIONARY dictionary headerfiles selectionfile )
  CMAKE_PARSE_ARGUMENTS(ARG "" "" "LIBRARIES;OPTIONS" ${ARGN})
  REFLEX_GENERATE_DICTIONARY(${dictionary} ${headerfiles} SELECTION ${selectionfile} OPTIONS ${ARG_OPTIONS})
  add_library(${dictionary}Dict MODULE ${gensrcdict})
  target_link_libraries(${dictionary}Dict ${ARG_LIBRARIES} ${XBOX_Reflex_LIBRARY})
  #----Installation details-------------------------------------------------------
  install(TARGETS ${dictionary}Dict LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
  set(mergedRootMap ${CMAKE_INSTALL_PREFIX}/${lib}/${CMAKE_PROJECT_NAME}Dict.rootmap)
  set(srcRootMap ${CMAKE_CURRENT_BINARY_DIR}/${rootmapname})
  install(CODE "EXECUTE_PROCESS(COMMAND ${merge_rootmap_cmd} --do-merge --input-file ${srcRootMap} --merged-file ${mergedRootMap})")
endfunction()

#---------------------------------------------------------------------------------------------------
# XBOX_CHECK_OUT_OF_SOURCE_BUILD( )
#---------------------------------------------------------------------------------------------------
macro(XBOX_CHECK_OUT_OF_SOURCE_BUILD)
  get_filename_component(bindir_parent ${CMAKE_BINARY_DIR} PATH)
  if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
     file(REMOVE_RECURSE ${CMAKE_SOURCE_DIR}/Testing)
     file(REMOVE ${CMAKE_SOURCE_DIR}/DartConfiguration.tcl)
     message(FATAL_ERROR "XBOX should be built as an out of source build, to keep the source directory clean. Please create a extra build directory and run the command 'cmake <path_to_source_dir>' in this newly created directory. You have also to delete the directory CMakeFiles and the file CMakeCache.txt in the source directory. Otherwise cmake will complain even if you run it from an out-of-source directory.")
  elseif(IS_SYMLINK ${CMAKE_BINARY_DIR} AND CMAKE_SOURCE_DIR STREQUAL bindir_parent)
     message(FATAL_ERROR "XBOX cannot be built from a sub-directory of the source tree that is a symlink. This is a current limitation of CMake. Please create a real build directory and run the command 'cmake <path_to_source_dir>' in this newly created directory.")
  endif()
endmacro()

#----------------------------------------------------------------------------
# function XBOX_ADD_TEST( <name> COMMAND cmd [arg1... ]
#                        [PRECMD cmd [arg1...]] [POSTCMD cmd [arg1...]]
#                        [OUTPUT outfile] [ERROR errfile] [INPUT infile]
#                        [ENVIRONMENT var1=val1 var2=val2 ...
#                        [DEPENDS test1 ...]
#                        [RUN_SERIAL]
#                        [TIMEOUT seconds]
#                        [DEBUG]
#                        [SOURCE_DIR dir] [BINARY_DIR dir]
#                        [WORKING_DIR dir]
#                        [BUILD target] [PROJECT project]
#                        [PASSREGEX exp] [FAILREGEX epx]
#                        [PASSRC code])
#
function(XBOX_ADD_TEST test)
  CMAKE_PARSE_ARGUMENTS(ARG "DEBUG;WILLFAIL;CHECKOUT;CHECKERR;RUN_SERIAL"
                            "TIMEOUT;BUILD;INPUT;OUTPUT;ERROR;SOURCE_DIR;BINARY_DIR;WORKING_DIR;PROJECT;PASSRC"
                             "COMMAND;COPY_TO_BUILDDIR;DIFFCMD;OUTCNV;OUTCNVCMD;PRECMD;POSTCMD;ENVIRONMENT;COMPILEMACROS;DEPENDS;PASSREGEX;OUTREF;ERRREF;FAILREGEX;LABELS"
                            ${ARGN})

  #- Handle COMMAND argument
  list(LENGTH ARG_COMMAND _len)
  if(_len LESS 1)
    if(NOT ARG_BUILD)
      message(FATAL_ERROR "XBOX_ADD_TEST: command is mandatory (without build)")
    endif()
  else()
    list(GET ARG_COMMAND 0 _prg)
    list(REMOVE_AT ARG_COMMAND 0)

    if(TARGET ${_prg})                                 # if command is a target, get the actual executable
      set(_prg "$<TARGET_FILE:${_prg}>")
      set(_cmd ${_prg} ${ARG_COMMAND})
    else()
      find_program(_exe ${_prg})
      if(_exe)                                         # if the command is found in the system, use it
        set(_cmd ${_exe} ${ARG_COMMAND})
      elseif(NOT IS_ABSOLUTE ${_prg})                  # if not absolute, assume is found in current binary dir
        set(_prg ${CMAKE_CURRENT_BINARY_DIR}/${_prg})
        set(_cmd ${_prg} ${ARG_COMMAND})
      else()                                           # take as it is
        set(_cmd ${_prg} ${ARG_COMMAND})
      endif()
      unset(_exe CACHE)
    endif()

    string(REPLACE ";" "^" _cmd "${_cmd}")
  endif()

  set(_command ${CMAKE_COMMAND} -DCMD=${_cmd})

  #- Handle PRE and POST commands
  if(ARG_PRECMD)
    string(REPLACE ";" "^" _pre "${ARG_PRECMD}")
    set(_command ${_command} -DPRE=${_pre})
  endif()

  if(ARG_POSTCMD)
    string(REPLACE ";" "^" _post "${ARG_POSTCMD}")
    set(_command ${_command} -DPOST=${_post})
  endif()

  #- Handle INPUT, OUTPUT, ERROR, DEBUG arguments
  if(ARG_INPUT)
    set(_command ${_command} -DIN=${ARG_INPUT})
  endif()

  if(ARG_OUTPUT)
    set(_command ${_command} -DOUT=${ARG_OUTPUT})
  endif()

  if(ARG_OUTREF)
    set(_command ${_command} -DOUTREF=${ARG_OUTREF})
  endif()

  if(ARG_ERRREF)
    set(_command ${_command} -DERRREF=${ARG_ERRREF})
  endif()

  if(ARG_ERROR)
    set(_command ${_command} -DERR=${ARG_ERROR})
  endif()

  if(ARG_WORKING_DIR)
    set(_command ${_command} -DCWD=${ARG_WORKING_DIR})
  endif()

  if(ARG_DEBUG)
    set(_command ${_command} -DDBG=ON)
  endif()

  if(ARG_PASSRC)
    set(_command ${_command} -DRC=${ARG_PASSRC})
  endif()

  if(ARG_OUTCNVCMD)
    string(REPLACE ";" "^" _outcnvcmd "${ARG_OUTCNVCMD}")
    string(REPLACE "=" "@" _outcnvcmd "${_outcnvcmd}")
    set(_command ${_command} -DCNVCMD=${_outcnvcmd})
  endif()

  if(ARG_OUTCNV)
    string(REPLACE ";" "^" _outcnv "${ARG_OUTCNV}")
    set(_command ${_command} -DCNV=${_outcnv})
  endif()

  if(ARG_DIFFCMD)
    string(REPLACE ";" "^" _diff_cmd "${ARG_DIFFCMD}")
    set(_command ${_command} -DDIFFCMD=${_diff_cmd})
  endif()

  if(ARG_CHECKOUT)
    set(_command ${_command} -DCHECKOUT=true)
  endif()

  if(ARG_CHECKERR)
    set(_command ${_command} -DCHECKERR=true)
  endif()

  set(_command ${_command} -DSYS=${XBOXSYS})

  #- Handle ENVIRONMENT argument
  if(ARG_ENVIRONMENT)
    string(REPLACE ";" "#" _env "${ARG_ENVIRONMENT}")
    string(REPLACE "=" "@" _env "${_env}")
    set(_command ${_command} -DENV=${_env})
  endif()

  #- Copy files to the build directory.
  if(ARG_COPY_TO_BUILDDIR)
    string(REPLACE ";" "^" _copy_files "${ARG_COPY_TO_BUILDDIR}")
    set(_command ${_command} -DCOPY=${_copy_files})
  endif()

  #- Locate the test driver
  find_file(XBOX_TEST_DRIVER XBOXTestDriver.cmake PATHS ${THISDIR} ${CMAKE_MODULE_PATH} NO_DEFAULT_PATH)
  if(NOT XBOX_TEST_DRIVER)
    message(FATAL_ERROR "XBOX_ADD_TEST: XBOXTestDriver.cmake not found!")
  endif()
  set(_command ${_command} -P ${XBOX_TEST_DRIVER})

  if(ARG_WILLFAIL)
    set(test ${test}_WILL_FAIL)
  endif()

  #- Now we can actually add the test
  if(ARG_BUILD)
    if(NOT ARG_SOURCE_DIR)
      set(ARG_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
    endif()
    if(NOT ARG_BINARY_DIR)
      set(ARG_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
    endif()
    if(NOT ARG_PROJECT)
       if(NOT PROJECT_NAME STREQUAL "XBOX")
         set(ARG_PROJECT ${PROJECT_NAME})
       else()
         set(ARG_PROJECT ${ARG_BUILD})
       endif()
    endif()
    add_test(NAME ${test} COMMAND ${CMAKE_CTEST_COMMAND}
      --build-and-test  ${ARG_SOURCE_DIR} ${ARG_BINARY_DIR}
      --build-generator ${CMAKE_GENERATOR}
      --build-makeprogram ${CMAKE_MAKE_PROGRAM}
      --build-target ${ARG_BUILD}
      --build-project ${ARG_PROJECT}
      --build-config $<CONFIGURATION>
      --build-noclean
      --test-command ${_command} )
    set_property(TEST ${test} PROPERTY ENVIRONMENT XBOX_DIR=${CMAKE_BINARY_DIR})
  else()
    add_test(NAME ${test} COMMAND ${_command})
    if (gnuinstall)
      set_property(TEST ${test} PROPERTY ENVIRONMENT XBOXIGNOREPREFIX=1)
    endif()
  endif()

  #- Handle TIMOUT and DEPENDS arguments
  if(ARG_TIMEOUT)
    set_property(TEST ${test} PROPERTY TIMEOUT ${ARG_TIMEOUT})
  endif()

  if(ARG_DEPENDS)
    set_property(TEST ${test} PROPERTY DEPENDS ${ARG_DEPENDS})
  endif()

  if(ARG_PASSREGEX)
    set_property(TEST ${test} PROPERTY PASS_REGULAR_EXPRESSION ${ARG_PASSREGEX})
  endif()

  if(ARG_FAILREGEX)
    set_property(TEST ${test} PROPERTY FAIL_REGULAR_EXPRESSION ${ARG_FAILREGEX})
  endif()

  if(ARG_WILLFAIL)
    set_property(TEST ${test} PROPERTY WILL_FAIL true)
  endif()

  if(ARG_LABELS)
    set_tests_properties(${test} PROPERTIES LABELS "${ARG_LABELS}")
  endif()

  if(ARG_RUN_SERIAL)
    set_property(TEST ${test} PROPERTY RUN_SERIAL true)
  endif()

endfunction()

#----------------------------------------------------------------------------
# XBOX_PATH_TO_STRING( <variable> path PATH_SEPARATOR_REPLACEMENT replacement )
#
# Mangle the path to a string.
#----------------------------------------------------------------------------
function(XBOX_PATH_TO_STRING resultvar path)
  # FIXME: Copied and modified from XBOXTEST_TARGETNAME_FROM_FILE. We should find a common place for that code.
  # FIXME: XBOXTEST_TARGETNAME_FROM_FILE could be replaced by just a call to string(MAKE_C_IDENTIFIER)...
  CMAKE_PARSE_ARGUMENTS(ARG "" "" "PATH_SEPARATOR_REPLACEMENT" ${ARGN})

  set(sep_replacement "")
  if (ARG_PATH_SEPARATOR_REPLACEMENT)
    set(sep_replacement ${ARG_PATH_SEPARATOR_REPLACEMENT})
  endif()

  get_filename_component(realfp ${path} ABSOLUTE)
  get_filename_component(filename_we ${path} NAME_WE)

  string(REPLACE "${CMAKE_SOURCE_DIR}" "" relativepath ${realfp})
  string(REPLACE "${path}" "" relativepath ${relativepath})

  string(MAKE_C_IDENTIFIER ${relativepath}${filename_we} mangledname)
  string(REPLACE "_" "${sep_replacement}" mangledname ${mangledname})

  set(${resultvar} "${mangledname}" PARENT_SCOPE)
endfunction(XBOX_PATH_TO_STRING)

#----------------------------------------------------------------------------
# XBOX_ADD_UNITTEST_DIR(<libraries ...>)
#----------------------------------------------------------------------------
function(XBOX_ADD_UNITTEST_DIR)
  XBOX_GLOB_FILES(test_files ${CMAKE_CURRENT_SOURCE_DIR}/*.cxx)
  # Get the component from the path. Eg. core to form coreTests test suite name.
  XBOX_PATH_TO_STRING(test_name ${CMAKE_CURRENT_SOURCE_DIR}/)
  XBOX_ADD_GTEST(${test_name}Unit ${test_files} LIBRARIES ${ARGN})
endfunction()

#----------------------------------------------------------------------------
# function XBOX_ADD_GTEST(<testsuite> source1 source2... LIBRARIES)
#
function(XBOX_ADD_GTEST test_suite)
  CMAKE_PARSE_ARGUMENTS(ARG "" "" "LIBRARIES" ${ARGN})
  include_directories(${CMAKE_CURRENT_BINARY_DIR} ${GTEST_INCLUDE_DIR} ${GMOCK_INCLUDE_DIR})

  XBOX_GET_SOURCES(source_files . ${ARG_UNPARSED_ARGUMENTS})
  # Note we cannot use XBOX_EXECUTABLE without user-specified set of LIBRARIES to link with.
  # The test suites should choose this in their specific CMakeLists.txt file.
  # FIXME: For better coherence we could restrict the libraries the test suite could link
  # against. For example, tests in Core should link only against libCore. This could be tricky
  # to implement because some XBOX components create more than one library.
  XBOX_EXECUTABLE(${test_suite} ${source_files} LIBRARIES ${ARG_LIBRARIES})
  set_property(TARGET ${test_suite} PROPERTY RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
  target_link_libraries(${test_suite} gtest gtest_main gmock gmock_main)

  XBOX_PATH_TO_STRING(mangled_name ${test_suite} PATH_SEPARATOR_REPLACEMENT "-")
  XBOX_ADD_TEST(gtest${mangled_name} COMMAND ${test_suite} WORKING_DIR ${CMAKE_CURRENT_BINARY_DIR})
endfunction()


#----------------------------------------------------------------------------
# XBOX_ADD_TEST_SUBDIRECTORY( <name> )
#----------------------------------------------------------------------------
function(XBOX_ADD_TEST_SUBDIRECTORY subdir)
  file(RELATIVE_PATH subdir ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/${subdir})
  set_property(GLOBAL APPEND PROPERTY XBOX_TEST_SUBDIRS ${subdir})
endfunction()

#----------------------------------------------------------------------------
# XBOX_ADD_CXX_FLAG(var flag)
#----------------------------------------------------------------------------
function(XBOX_ADD_CXX_FLAG var flag)
  string(REGEX REPLACE "[-.+/:= ]" "_" flag_esc "${flag}")
  CHECK_CXX_COMPILER_FLAG("-Werror ${flag}" CXX_HAS${flag_esc})
  if(CXX_HAS${flag_esc})
    set(${var} "${${var}} ${flag}" PARENT_SCOPE)
  endif()
endfunction()

#----------------------------------------------------------------------------
# XBOX_ADD_C_FLAG(var flag)
#----------------------------------------------------------------------------
function(XBOX_ADD_C_FLAG var flag)
  string(REGEX REPLACE "[-.+/:= ]" "_" flag_esc "${flag}")
  CHECK_C_COMPILER_FLAG("-Werror ${flag}" C_HAS${flag_esc})
  if(C_HAS${flag_esc})
    set(${var} "${${var}} ${flag}" PARENT_SCOPE)
  endif()
endfunction()



