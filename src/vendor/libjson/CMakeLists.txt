## Build the libjson lib
PROJECT(VENDOR_LIBJSON)

set (HEADER_FILES
    ./JSONOptions.h
    ./libjson.h

	./Source/internalJSONNode.h
	./Source/JSON_Base64.h
	./Source/JSONChildren.h
	./Source/JSONDebug.h
	./Source/JSONDefs.h
	./Source/JSONMemory.h
	./Source/JSONNode.h
	./Source/JSONWorker.h
	./Source/NumberToString.h
)

set (SOURCE_FILES
	./Source/internalJSONNode.cpp
	./Source/JSON_Base64.cpp
	./Source/JSONChildren.cpp
	./Source/JSONDebug.cpp
	./Source/JSONIterators.cpp
	./Source/JSONMemory.cpp
	./Source/JSONNode_Mutex.cpp
	./Source/JSONNode.cpp
	./Source/JSONWorker.cpp
	./Source/JSONWriter.cpp
	./Source/libjson.cpp
)


add_library(json STATIC ${SOURCE_FILES} ${HEADER_FILES})
set_property(TARGET json PROPERTY DEBUG_POSTFIX _d)
