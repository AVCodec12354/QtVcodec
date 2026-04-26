cmake_minimum_required(VERSION 3.16)

project(Qv2ComponentTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/outputs/tests)

add_executable(Qv2SourceTest
        ${CMAKE_SOURCE_DIR}/readers/implements/Y4MSource.cpp
        ${CMAKE_SOURCE_DIR}/readers/implements/YUVSource.cpp
        Qv2SourceTest.cpp
)

target_include_directories(Qv2SourceTest PRIVATE
        ${CMAKE_SOURCE_DIR}/inc/Qv2Core
        ${CMAKE_SOURCE_DIR}/readers/implements
        ${CMAKE_SOURCE_DIR}/readers/interfaces
        ${CMAKE_SOURCE_DIR}/readers/tests
)

target_compile_definitions(Qv2SourceTest PRIVATE
        TEST_DATA_PATH="${CMAKE_CURRENT_SOURCE_DIR}/raw/"
)

# Link với GTest
target_link_libraries(Qv2SourceTest
        PRIVATE
        GTest::gtest_main
        GTest::gtest
)

# Tự động phát hiện các hàm TEST() trong code
include(GoogleTest)
gtest_discover_tests(Qv2SourceTest)