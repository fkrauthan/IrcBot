## The libs Buildfile for mountainfort tools
project(MOUNTAINFORT_TOOLS_LIBS)


#Build libini
add_subdirectory(libini)


#Build libirc
add_subdirectory(libirc)


#Build libirc2 (just for compiling at the moment)
add_subdirectory(libirc2)


#Build libbase
add_subdirectory(libbase)


#Build libbotmodule
add_subdirectory(libbotmodule)
