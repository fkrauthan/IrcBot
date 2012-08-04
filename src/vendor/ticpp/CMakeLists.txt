## Build the ticpp lib
PROJECT(VENDOR_TICPP)


set (HEADER_FILES
	./ticpp.h
	./ticpprc.h
	./tinystr.h
	./tinyxml.h
)

set (SOURCE_FILES
	./ticpp.cpp
	./tinystr.cpp
	./tinyxml.cpp
	./tinyxmlerror.cpp
	./tinyxmlparser.cpp
)


add_definitions( -DTIXML_USE_TICPP )


add_library(ticpp STATIC ${SOURCE_FILES} ${HEADER_FILES})
set_property(TARGET ticpp PROPERTY DEBUG_POSTFIX _d)
