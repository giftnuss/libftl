IF(UNIX)
	SET(PONA_PLATFORM "POSIX")
	ADD_DEFINITIONS(
		-DPONA_POSIX
		-D_REENTRANT
		-D_FILE_OFFSET_BITS=64
	)
	IF(APPLE)
		ADD_DEFINITIONS(-DPONA_APPLE)
	ELSE(APPLE)
		ADD_DEFINITIONS(-D_XOPEN_SOURCE=600)
	ENDIF(APPLE)
	SET(CMAKE_CXX_FLAGS_DEBUG "-Wall ${CMAKE_CXX_FLAGS_DEBUG}")
ELSEIF(WIN32)
	SET(PONA_PLATFORM "WINDOWS")
	ADD_DEFINITIONS(
		-DPONA_WINDOWS
		-DWIN32_LEAN_AND_MEAN
		-DUNICODE
		-D_UNICODE
	)
	SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
	SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
ELSE(UNIX)
	MESSAGE("Your platform is not supported by PONA.")
ENDIF(UNIX)

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/build CACHE PATH "")
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/build CACHE PATH "")

