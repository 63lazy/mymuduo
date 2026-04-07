cmake_minimum_required(VERSION 3.5)
project(utils)

set(UTILS_SOURCES
    logger.cc
    Timestamp.cc
    CurrentThread.cc
    thread.cc
    Buffer.cc
)

add_library(utils STATIC ${UTILS_SOURCES})

target_include_directories(utils PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(utils PUBLIC cxx_std_11)

if(WIN32)
    target_link_libraries(utils PUBLIC ws2_32)
endif()

set_target_properties(utils PROPERTIES POSITION_INDEPENDENT_CODE ON)

# Place utils outputs into this subdirectory's build area (e.g. build/utils)
set_target_properties(utils PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Test executable for utils (creates build/utils/bin/utils_test)
add_executable(utils_test test_main.cpp)
target_link_libraries(utils_test PRIVATE utils)
target_include_directories(utils_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(utils_test PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
