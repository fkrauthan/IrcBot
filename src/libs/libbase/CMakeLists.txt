## Build the base lib
PROJECT(IRCBOT_LIBS_SCBase)

set (HEADER_FILES
    ./StringUtils/StringUtils.h
	./StringUtils/StringUtils.inl
)

set (SOURCE_FILES
	./StringUtils/StringUtils.cpp
)


add_library(base STATIC ${SOURCE_FILES} ${HEADER_FILES})

set_property(TARGET base PROPERTY DEBUG_POSTFIX _d)
set_property(TARGET base PROPERTY COMPILE_DEFINITIONS_DEBUG DEBUG)
