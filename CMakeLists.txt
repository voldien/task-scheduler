CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
PROJECT(schScheduler VERSION 0.1.3)

#	build shared/dynamic library option
OPTION(BUILD_SHARED_LIBS "Build package with shared libraries." ON)
OPTION(BUILD_WITH_COVERAGE "Enable profiling and coverage report analysis." ON)
OPTION(BUILD_WITH_PROFILER "Enable profiling." ON)
IF(NOT BUILD_SHARED_LIBS)
    SET(CMAKE_EXE_LINKER_FLAGS "-static")
    SET(LINK_SEARCH_START_STATIC TRUE)
ELSE()
    MESSAGE(STATUS "Shared libraries enabled")
ENDIF(NOT BUILD_SHARED_LIBS)

#############################################
#  Extract git hash and branch information.
#############################################
OPTION(IGNORE_GIT_HASH "Use Git hash" OFF)
FIND_PACKAGE(Git QUIET)
IF(NOT IGNORE_GIT_HASH)
  IF(GIT_FOUND)
    # Get the current working branch
    EXECUTE_PROCESS(
    COMMAND git rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
    )

    # Get the latest abbreviated commit hash of the working branch
    EXECUTE_PROCESS(
    COMMAND git rev-parse --short=8 HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
    )

    #Extract IF there is a tag associated with current commit
    EXECUTE_PROCESS(
    COMMAND git describe --exact-match --tags 2> /dev/null || git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_TAG
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
    )
  ELSE()
    MESSAGE(FATAL_ERROR "Git must be install on the system in order to extract meta data from GIT version control" )
  ENDIF()
ENDIF()


OPTION(TASKSCH_BUILD_WITH_TEST "Enable Testing." OFF)

IF(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
	SET(SCH_STANDALONE ON CACHE BOOL "")
ELSE()
	SET(SCH_STANDALONE OFF CACHE BOOL "")
ENDIF()

# Version setup.
SET(SCH_VERSION_MAJOR 0)
SET(SCH_VERSION_MINOR 1)
SET(SCH_VERSION_REVISION 3)
SET(SCH_VERSION_STATE a)
SET(SCH_VERSION ${SCH_VERSION_MAJOR}.${SCH_VERSION_MINOR}${SCH_VERSION_STATE}${SCH_VERSION_REVISION} )
ADD_DEFINITIONS(-DSCH_VERSION="${SCH_VERSION}")

##############################
# Target compiler flags.
#############################
IF((CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX) AND NOT CMAKE_COMPILER_IS_MINGW)

    ADD_DEFINITIONS(-Wall -w -Wfatal-errors)

    IF (CMAKE_BUILD_TYPE STREQUAL "Release")
        MESSAGE(STATUS "Compile for release.")
        ADD_DEFINITIONS( -DNDEBUG )
        ADD_DEFINITIONS( -O2 )
        SET(CMAKE_RELEASE TRUE)
    ELSE()
        MESSAGE(STATUS "Compile for debug.")
        ADD_DEFINITIONS( -D_DEBUG )
        ADD_DEFINITIONS( -g3 -O0 )
        SET(CMAKE_DEBUG TRUE)
    ENDIF()

    #
    IF(BUILD_WITH_COVERAGE)
        MESSAGE(STATUS "Building with coverage")
        SET(GCC_COVERAGE_COMPILE_FLAGS "--coverage -fprofile-arcs -ftest-coverage")
        SET(GCC_COVERAGE_LINK_FLAGS    "-lgcov")
        SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
        SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
        SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")
    ENDIF()
    #
    IF(BUILD_WITH_PROFILER)
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
        SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
        SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -pg")
    ENDIF()
ELSEIF(MSVC)
    # Enable multi-core compilation with MSVC
    ADD_COMPILE_OPTIONS(/MP)
ELSEIF ( "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang" )
    SET(CMAKE_CXX_FLAGS "-g -fvisibility=hidden -fno-strict-aliasing -Wall -Wno-long-long ${CMAKE_CXX_FLAGS}" )
    SET(CMAKE_C_FLAGS "-fno-strict-aliasing ${CMAKE_C_FLAGS}")
ELSEIF( CMAKE_COMPILER_IS_MINGW )
    SET(CMAKE_CXX_FLAGS "-fvisibility=hidden -fno-strict-aliasing -Wall -Wno-long-long -Wa,-mbig-obj ${CMAKE_CXX_FLAGS}")
    SET(CMAKE_C_FLAGS "-fno-strict-aliasing ${CMAKE_C_FLAGS}")
    ADD_DEFINITIONS( -U__STRICT_ANSI__ )
ENDIF()

###########################
# Source files.
###########################
FILE(GLOB SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)
FILE(GLOB HEADER_PRIVATE_FILES include/internal/internal/*.h)
FILE(GLOB HEADER_PUBLIC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/include/taskSch.h)
IF( UNIX )
	# Unix specific platform code (Linux,FreeBSD and other UNIX compatible)
    FILE(GLOB PLATFORM_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/unix/*.c)
ELSEIF(WIN32)
	# Window specific platform code.
    FILE(GLOB PLATFORM_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/windows/*.c)
ENDIF()

################################
# TaskSch Library Target.
################################
ADD_LIBRARY(taskSch ${SOURCE_FILES} ${PLATFORM_FILES} ${HEADER_PRIVATE_FILES} ${HEADER_PUBLIC_FILES})
TARGET_INCLUDE_DIRECTORIES(taskSch PUBLIC
								$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
								$<INSTALL_INTERFACE:include>)


FIND_PACKAGE(Threads QUIET)
IF(Threads_FOUND)
	MESSAGE(STATUS "Thread Found: ${CMAKE_THREAD_LIBS_INIT}")
ENDIF()

IF(UNIX)
	TARGET_LINK_LIBRARIES(taskSch Threads::Threads)
ELSEIF(WIN32)
	TARGET_LINK_LIBRARIES(taskSch Threads::Threads)
ENDIF()

IF (BUILD_SHARED_LIBS AND CMAKE_SIZEOF_VOID_P EQUAL 8) # -fPIC is only required
	SET_PROPERTY(TARGET taskSch PROPERTY POSITION_INDEPENDENT_CODE ON)
ENDIF()
# Install the target.
INSTALL(TARGETS taskSch DESTINATION lib)
INSTALL(FILES ${HEADER_PUBLIC_FILES} DESTINATION include)

#############################
# Unit testing
#############################
IF(TASKSCH_BUILD_WITH_TEST)
    ENABLE_TESTING()
    ADD_SUBDIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/test)
ENDIF()

#################################
# Generate Docs
#################################
OPTION(TASKSCH_BUILD_WITH_DOCS "Generate Docs" OFF)
	IF(TASKSCH_BUILD_WITH_DOCS)
	ADD_SUBDIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/docs/)
ENDIF()
