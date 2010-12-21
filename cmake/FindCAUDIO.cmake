# - Locate CAUDIO library
# This module defines
#  CAUDIO_LIBRARY, the library to link against
#  CAUDIO_FOUND, if false, do not try to link to CAUDIO
#  CAUDIO_INCLUDE_DIR, where to find headers.

IF(CAUDIO_LIBRARY AND CAUDIO_INCLUDE_DIR)
	# in cache already
	SET(CAUDIO_FIND_QUIETLY TRUE)
ENDIF(CAUDIO_LIBRARY AND CAUDIO_INCLUDE_DIR)


FIND_PATH(CAUDIO_INCLUDE_DIR
	cAudio/cAudio.h
	PATHS
	$ENV{CAUDIO_DIR}/include
	/usr/local/include
	/usr/include
	/sw/include
	/opt/local/include
	/opt/csw/include
	/opt/include
	PATH_SUFFIXES
)

set( CAUDIO_NAMES_RELEASE cAudio cAudio.2.1.0 )
FIND_LIBRARY( CAUDIO_LIBRARY_RELEASE
	NAMES ${CAUDIO_NAMES_RELEASE}
	PATHS $ENV{CAUDIO_DIR}/lib /usr/local/lib /usr/lib
	)

set( CAUDIO_NAMES_DEBUG cAudio cAudio.2.1.0 )
FIND_LIBRARY( CAUDIO_LIBRARY_DEBUG
	NAMES ${CAUDIO_NAMES_DEBUG}
	PATHS $ENV{CAUDIO_DIR}/lib /usr/local/lib /usr/lib
	)

if( CAUDIO_LIBRARY_DEBUG AND NOT CAUDIO_LIBRARY_RELEASE )
	set( CAUDIO_LIBRARY_RELEASE ${CAUDIO_LIBRARY_DEBUG} )
endif()
set( CAUDIO_LIBRARY optimized ${CAUDIO_LIBRARY_RELEASE}
	debug ${CAUDIO_LIBRARY_DEBUG} )

IF(CAUDIO_LIBRARY AND CAUDIO_INCLUDE_DIR)
	SET(CAUDIO_FOUND "YES")
	SET(CAUDIO_INCLUDE_DIR "${CAUDIO_INCLUDE_DIR};${CAUDIO_INCLUDE_DIR}/cAudio")
	IF(NOT CAUDIO_FIND_QUIETLY)
		MESSAGE(STATUS "Found CAUDIO: ${CAUDIO_LIBRARY}")
	ENDIF(NOT CAUDIO_FIND_QUIETLY)
ELSE(CAUDIO_LIBRARY AND CAUDIO_INCLUDE_DIR)
	IF(NOT CAUDIO_FIND_QUIETLY)
		MESSAGE(STATUS "Warning: Unable to find CAUDIO!")
	ENDIF(NOT CAUDIO_FIND_QUIETLY)
ENDIF(CAUDIO_LIBRARY AND CAUDIO_INCLUDE_DIR)
