PROJECT(IRCBOT_BOT_TEST)


set (SOURCE_FILES
    ./main.cpp
)


#Find dependencys
find_package(Boost COMPONENTS system thread REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})


#Build the source
find_package(SigC++ REQUIRED)
include_directories(${SigC++_INCLUDE_DIRS})


#Build the source
if(CMAKE_BUILD_TYPE EQUAL "Debug")
    SET_SOURCE_FILE_PROPERTIES(${SOURCE_FILES} COMPILE_FLAGS -DDEBUG)
    SET_SOURCE_FILE_PROPERTIES(${HEADER_FILES} COMPILE_FLAGS -DDEBUG)
endif (CMAKE_BUILD_TYPE EQUAL "Debug")


add_executable(IrcBotTest ${SOURCE_FILES} ${HEADER_FILES})
target_link_libraries(IrcBotTest ${Boost_LIBRARIES} ${SigC++_LIBRARIES})
target_link_libraries(IrcBotTest debug "irc2_d" "ini_d" "base_d" "json_d")
target_link_libraries(IrcBotTest optimized "irc2" "ini" "base" "json")

if(UNIX)
    target_link_libraries(IrcBotTest dl)
endif (UNIX)


set_property(TARGET IrcBotTest PROPERTY DEBUG_POSTFIX _d)
set_property(TARGET IrcBotTest PROPERTY COMPILE_DEFINITIONS_DEBUG DEBUG)
