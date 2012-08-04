## Build the ini parser lib
PROJECT(TRINIDAD_BACKEND_LIBS_LIBINI)

set (HEADER_FILES
    ./Exceptions.h

	./INI.h
	./INI.inl
)

set (SOURCE_FILES
	./INI.cpp
)


add_library(ini STATIC ${SOURCE_FILES} ${HEADER_FILES})
set_property(TARGET ini PROPERTY DEBUG_POSTFIX _d)
