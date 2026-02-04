cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(TINYP LANGUAGES CXX)

set(TINYP_OUT_DIR ${TINYP_BINARY_DIR}/out)

set(TINYP_INCLUDE_DIR "${TINYP_SOURCE_DIR}/include")
include_directories(${TINYP_INCLUDE_DIR})

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${TINYP_OUT_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${TINYP_OUT_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${TINYP_OUT_DIR}/lib)

add_subdirectory(tinyp)
add_subdirectory(tinyp_tool)

# Tests
option(TINYP_BUILD_TESTS "Build tests" ON)
if(TINYP_BUILD_TESTS)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    # For Windows: Prevent overriding the parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()
    add_subdirectory(tests)
endif()
