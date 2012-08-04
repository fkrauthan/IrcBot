## The Main Buildfile for mountainfort tools
cmake_minimum_required(VERSION 2.6)

project(IRC_BOT)


#Set cmake custom scripts folder
set(CMAKE_MODULE_PATH "${IRC_BOT_SOURCE_DIR}/../cmake")

#Set default build type to debug if no build type is entered
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug)
endif (NOT CMAKE_BUILD_TYPE)


#Specify build paths
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${IRC_BOT_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${IRC_BOT_BINARY_DIR}/bin/modules")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${IRC_BOT_BINARY_DIR}/bin")
if (WIN32 OR APPLE)
	if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
		#We don't want to install in default system location, install is really for the SDK, so call it that
		set(CMAKE_INSTALL_PREFIX "${IRC_BOT_BINARY_DIR}/sdk" CACHE PATH "ircbot install prefix" FORCE)
	endif (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
endif(WIN32 OR APPLE)


#Add Build options for 64bit builds on linux
if(UNIX AND "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
	add_definitions(-fPIC) 
endif (UNIX AND "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "x86_64")


# Set C++ 0x support
if(UNIX OR MINGW)
	add_definitions(-std=gnu++0x)
endif (UNIX OR MINGW)


#Add binary path zu include dir
include_directories(${CMAKE_CURRENT_BINARY_DIR})


#Set include dir to lib and vendor root dir
include_directories("${IRC_BOT_SOURCE_DIR}/libs" "${IRC_BOT_SOURCE_DIR}/vendor" "${IRC_BOT_SOURCE_DIR}/bot")


#Vendor libs
add_subdirectory(vendor)


#Include libs
add_subdirectory(libs)


#Set link directorys
link_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})


#Build bot
add_subdirectory(bot)


#Build test bot
add_subdirectory(test)


#Display
message(STATUS			"Build Type:                   ${CMAKE_BUILD_TYPE}")
