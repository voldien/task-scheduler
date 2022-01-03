# Source files for the hpmassert program.
FILE(GLOB unitsrc  ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

# Find target for the check library.
INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/Modules/FindCheck.cmake)
MESSAGE(STATUS "${CHECK_LIBRARIES}")

# Executable hpm assert program.
ADD_EXECUTABLE(sch-task-cunit ${unitsrc})
TARGET_LINK_LIBRARIES(sch-task-cunit ${CHECK_LIBRARIES} taskSch)
TARGET_INCLUDE_DIRECTORIES(sch-task-cunit PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include ${CHECK_INCLUDE_DIR})

ADD_TEST(
        NAME assertPerformance
        COMMAND sch-task-cunit )
SET_TESTS_PROPERTIES(assertPerformance PROPERTIES DEPENDS taskSch)
