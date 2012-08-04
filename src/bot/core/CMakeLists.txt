PROJECT(IRCBOT_BOT_CORE)


set (HEADER_FILES
    ./Bot/bot.h
    
    ./ModuleManager/modulemanager.h
    ./ModuleManager/ModuleManagerC.h
)

set (SOURCE_FILES
    ./main.cpp
    
    ./Bot/bot.cpp
    
    ./ModuleManager/modulemanager.cpp
    ./ModuleManager/ModuleManagerC.cpp
)


#Find dependencys
find_package(Boost COMPONENTS system program_options thread REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})


#Build the source
if(CMAKE_BUILD_TYPE EQUAL "DEBUG" OR CMAKE_BUILD_TYPE EQUAL "RELWITHDEBINFO")
    SET_SOURCE_FILE_PROPERTIES(${SOURCE_FILES} COMPILE_FLAGS -DDEBUG)
    SET_SOURCE_FILE_PROPERTIES(${HEADER_FILES} COMPILE_FLAGS -DDEBUG)
endif ()


add_executable(IrcBot ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(IrcBot ${Boost_LIBRARIES})
target_link_libraries(IrcBot debug "irc_d" "ini_d" "base_d" "json_d")
target_link_libraries(IrcBot optimized "irc" "ini" "base" "json")

if(UNIX)
    target_link_libraries(IrcBot dl)
endif (UNIX)


set_property(TARGET IrcBot PROPERTY DEBUG_POSTFIX _d)
set_property(TARGET IrcBot PROPERTY COMPILE_DEFINITIONS_DEBUG DEBUG)
