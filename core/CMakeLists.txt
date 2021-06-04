############################################################################
# CMakeLists.txt file for building XBOX (global) core package
############################################################################

add_subdirectory(xboxbase)
add_subdirectory(xboxchannel)
#add_subdirectory(xboxpulse)

set(objectlibs $<TARGET_OBJECTS:xboxbase>
               $<TARGET_OBJECTS:xboxchannel>
               #$<TARGET_OBJECTS:xboxpulse>
               )

set(ARG_LIBRARIES ${ARG_LIBRARIES} ${ROOT_LIBRARIES})

# generate linker library from all xbox core object libraries defined in the subdirectories
XBOX_LINKER_LIBRARY(xboxcore 
                    ${objectlibs}
                    LIBRARIES ${ARG_LIBRARIES}
                    DEPENDENCIES ${ARG_DEPENDENCIES}
#                    BUILTINS PCRE LZMA LZ4 ZLIB)
                   )

