# - Locate Equalizer library
# This module defines
#  Equalizer_LIBRARY, the library to link against
#  Equalizer_FOUND, if false, do not try to link to Equalizer
#  Equalizer_INCLUDE_DIRS, where to find headers.

IF(Equalizer_LIBRARY AND Equalizer_INCLUDE_DIRS)
	# in cache already
	SET(Equalizer_FIND_QUIETLY TRUE)
ENDIF(Equalizer_LIBRARY AND Equalizer_INCLUDE_DIRS)


FIND_PATH(Equalizer_INCLUDE_DIRS
	eq/eq.h
	PATHS
	$ENV{Equalizer_DIR}/include
	/usr/local/include
	/usr/include
	/sw/include
	/opt/local/include
	/opt/csw/include
	/opt/include
	PATH_SUFFIXES
)

FIND_LIBRARY(Equalizer_LIBRARY
	NAMES Equalizer
	PATHS
	$ENV{Equalizer_DIR}/lib
	/usr/local/lib
	/usr/lib
	/usr/local/X11R6/lib
	/usr/X11R6/lib
	/sw/lib
	/opt/local/lib
	/opt/csw/lib
	/opt/lib
	/usr/freeware/lib64
)
   
IF(Equalizer_LIBRARY AND Equalizer_INCLUDE_DIRS)
	SET(Equalizer_FOUND "YES")
	IF(NOT Equalizer_FIND_QUIETLY)
		MESSAGE(STATUS "Found Equalizer: ${Equalizer_LIBRARY}")
	ENDIF(NOT Equalizer_FIND_QUIETLY)
ELSE(Equalizer_LIBRARY AND Equalizer_INCLUDE_DIRS)
	IF(NOT Equalizer_FIND_QUIETLY)
		MESSAGE(STATUS "Warning: Unable to find Equalizer!")
	ENDIF(NOT Equalizer_FIND_QUIETLY)
ENDIF(Equalizer_LIBRARY AND Equalizer_INCLUDE_DIRS)
