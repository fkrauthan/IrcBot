## Build the invite me module
PROJECT(IRCBOT_BOT_MODULE_INVITEME)

set (HEADER_FILES
    ./inviteme.h
)

set (SOURCE_FILES
    ./main.cpp

    ./inviteme.cpp
)


if(CMAKE_BUILD_TYPE EQUAL "Debug")
    SET_SOURCE_FILE_PROPERTIES(${SOURCE_FILES} COMPILE_FLAGS -DDEBUG)
    SET_SOURCE_FILE_PROPERTIES(${HEADER_FILES} COMPILE_FLAGS -DDEBUG)
endif (CMAKE_BUILD_TYPE EQUAL "Debug")

add_library(inviteme SHARED ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(inviteme debug "json_d" "botmodule_d")
target_link_libraries(inviteme optimized "json" "botmodule")


set_property(TARGET inviteme PROPERTY DEBUG_POSTFIX _d)
set_property(TARGET inviteme PROPERTY COMPILE_DEFINITIONS_DEBUG DEBUG)