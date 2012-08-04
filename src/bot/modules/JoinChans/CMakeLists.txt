## Build the invite me module
PROJECT(IRCBOT_BOT_MODULE_JOINCHANS)

set (HEADER_FILES
    ./joinchans.h
)

set (SOURCE_FILES
    ./main.cpp

    ./joinchans.cpp
)


if(CMAKE_BUILD_TYPE EQUAL "Debug")
    SET_SOURCE_FILE_PROPERTIES(${SOURCE_FILES} COMPILE_FLAGS -DDEBUG)
    SET_SOURCE_FILE_PROPERTIES(${HEADER_FILES} COMPILE_FLAGS -DDEBUG)
endif (CMAKE_BUILD_TYPE EQUAL "Debug")

add_library(joinchans SHARED ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(joinchans debug "json_d" "botmodule_d" "base_d")
target_link_libraries(joinchans optimized "json" "botmodule" "base")


set_property(TARGET joinchans PROPERTY DEBUG_POSTFIX _d)
set_property(TARGET joinchans PROPERTY COMPILE_DEFINITIONS_DEBUG DEBUG)