cmake_minimum_required (VERSION 3.25)

project (tests)

set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
set (CMAKE_CXX_STANDARD 20)

include (FetchContent)
FetchContent_Declare (
    catch2
    GIT_REPOSITORY https://github.com/catchorg/catch2.git
    GIT_TAG v3.11.0
    SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/catch2
)

FetchContent_MakeAvailable (catch2)

target_compile_definitions (Catch2WithMain PRIVATE DO_NOT_USE_WMAIN)

file (GLOB_RECURSE UNITTEST_SRC RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "../Source/*_unittest.cpp")

message ("== Found unittest files")
message ("${UNITTEST_SRC}")

add_executable (tests ${UNITTEST_SRC})

target_link_libraries (tests PRIVATE Catch2WithMain)
