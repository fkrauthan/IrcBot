## Build the irc lib v2
PROJECT(IRCBOT_LIBS_LIBIRC2)

set (HEADER_FILES
    ./IRC.h
    ./IRCConnection.h
    ./IRCConnectionSendThread.h
    
    ./IRCMessage.h
    ./IRCExceptions.h
)

set (SOURCE_FILES
	./IRC.cpp
	./IRCConnection.cpp
	./IRCConnectionSendThread.cpp
)


#Find dependencys
find_package(Boost COMPONENTS thread REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})


#Build the source
find_package(SigC++ REQUIRED)
include_directories(${SigC++_INCLUDE_DIRS})


add_library(irc2 STATIC ${SOURCE_FILES} ${HEADER_FILES})
set_property(TARGET irc2 PROPERTY DEBUG_POSTFIX _d)
