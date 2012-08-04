## Build the irc lib
PROJECT(IRCBOT_LIBS_LIBIRC)

set (HEADER_FILES
    ./irc.h
    ./ircconnection.h
    ./irceventhandler.h
)

set (SOURCE_FILES
	./irc.cpp
    ./ircconnection.cpp
    ./irceventhandler.cpp
)


add_library(irc STATIC ${SOURCE_FILES} ${HEADER_FILES})
set_property(TARGET irc PROPERTY DEBUG_POSTFIX _d)
