## Build the irc lib
PROJECT(IRCBOT_LIBS_LIBBOTMODULE)

set (HEADER_FILES
    ./module.h
    ./cppmodule.h
    
    ./cpp/IrcModuleConnection.h
    ./cpp/IrcModule.h
    ./cpp/IrcModuleManager.h
)

set (SOURCE_FILES
	./cppmodule.cpp
	
	./cpp/IrcModuleConnection.cpp
	./cpp/IrcModule.cpp
	./cpp/IrcModuleManager.cpp
)


add_library(botmodule STATIC ${SOURCE_FILES} ${HEADER_FILES})
set_property(TARGET botmodule PROPERTY DEBUG_POSTFIX _d)
