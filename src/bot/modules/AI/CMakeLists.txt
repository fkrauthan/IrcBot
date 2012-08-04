## Build the ai module
PROJECT(IRCBOT_BOT_MODULE_AI)

set (HEADER_FILES
    ./ai.h
    
    ./AIAnswer.h
    ./AILanugageAnalyser.h
)

set (SOURCE_FILES
    ./main.cpp

    ./ai.cpp
    
    ./AIAnswer.cpp
    ./AILanugageAnalyser.cpp
)


if(CMAKE_BUILD_TYPE EQUAL "Debug")
    SET_SOURCE_FILE_PROPERTIES(${SOURCE_FILES} COMPILE_FLAGS -DDEBUG)
    SET_SOURCE_FILE_PROPERTIES(${HEADER_FILES} COMPILE_FLAGS -DDEBUG)
endif (CMAKE_BUILD_TYPE EQUAL "Debug")

add_library(ai SHARED ${SOURCE_FILES} ${HEADER_FILES})

target_link_libraries(ai debug "json_d" "ticpp_d" "botmodule_d" "base_d")
target_link_libraries(ai optimized "json" "ticpp" "botmodule" "base")


set_property(TARGET ai PROPERTY DEBUG_POSTFIX _d)
set_property(TARGET ai PROPERTY COMPILE_DEFINITIONS_DEBUG DEBUG)
