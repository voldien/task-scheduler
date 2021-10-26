# gcc compiler options.
IF((CMAKE_COMPILER_IS_GNUCC) AND NOT CMAKE_COMPILER_IS_MINGOW)

    # Compiling language standard.
    ADD_DEFINITIONS(-std=c11)
    ADD_DEFINITIONS(-w)

    # Check build target.
    IF (CMAKE_BUILD_TYPE STREQUAL "Release")
        MESSAGE(STATUS "Compile for release.")
        ADD_DEFINITIONS(-DNDEBUG -O2)
        SET(CMAKE_RELEASE TRUE)
    ELSEIF( CMAKE_BUILD_TYPE STREQUAL "Debug")
        MESSAGE(STATUS "Compile for debug.")
        ADD_DEFINITIONS(-D_DEBUG)
        ADD_DEFINITIONS(-g3)
        SET(CMAKE_DEBUG TRUE)
    ENDIF()

ELSEIF(MSVC)
    # Enable multi-core complication with MSVC
    ADD_DEFINITIONS(/MP)
ENDIF()

# Source files for the hpmassert program.
FILE(GLOB unitsrc  ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

# Find target for the check library.
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/Modules/FindCheck.cmake)
MESSAGE(STATUS "${CHECK_LIBRARIES}")

# Executable hpm assert program.
ADD_EXECUTABLE(sch-task-cunit ${unitsrc})
TARGET_LINK_LIBRARIES(sch-task-cunit ${CHECK_LIBRARIES} pthread taskSch)
TARGET_INCLUDE_DIRECTORIES(sch-task-cunit PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include ${CHECK_INCLUDE_DIR})

ADD_TEST(
        NAME assertPerformance
        COMMAND sch-task-cunit )
SET_TESTS_PROPERTIES(assertPerformance PROPERTIES DEPENDS taskSch)
