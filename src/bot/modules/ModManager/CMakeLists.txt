## Build the modmanager module
PROJECT(IRCBOT_BOT_MODULE_MODMANAGER)

set (HEADER_FILES
    ./modmanager.h
)

set (SOURCE_FILES
    ./main.cpp

    ./modmanager.cpp
)


if(CMAKE_BUILD_TYPE EQUAL "Debug")
    SET_SOURCE_FILE_PROPERTIES(${SOURCE_FILES} COMPILE_FLAGS -DDEBUG)
    SET_SOURCE_FILE_PROPERTIES(${HEADER_FILES} COMPILE_FLAGS -DDEBUG)
endif (CMAKE_BUILD_TYPE EQUAL "Debug")

add_library(modmanager SHARED ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(modmanager debug "json_d" "botmodule_d" "base_d")
target_link_libraries(modmanager optimized "json" "botmodule" "base")


set_property(TARGET modmanager PROPERTY DEBUG_POSTFIX _d)
set_property(TARGET modmanager PROPERTY COMPILE_DEFINITIONS_DEBUG DEBUG)
