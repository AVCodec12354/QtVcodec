cmake_minimum_required(VERSION 3.16)

set(PROJECT_NAME Qv2RendererTest)
project(${PROJECT_NAME} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/outputs/tests)

add_executable(${PROJECT_NAME}
        ${CMAKE_SOURCE_DIR}/readers/implements/Y4MSource.cpp
        ${CMAKE_SOURCE_DIR}/readers/implements/YUVSource.cpp
        ${CMAKE_SOURCE_DIR}/renderers/implements/VideoRenderer.cpp
        ${CMAKE_SOURCE_DIR}/renderers/interfaces/VideoGLWidget.h
        Qv2RendererTest.cpp
        main.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/inc/Qv2Core
        ${CMAKE_SOURCE_DIR}/readers/implements
        ${CMAKE_SOURCE_DIR}/readers/interfaces
        ${CMAKE_SOURCE_DIR}/renderers/implements
        ${CMAKE_SOURCE_DIR}/renderers/interfaces
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
        TEST_DATA_PATH="${CMAKE_SOURCE_DIR}/readers/tests/raw/"
)

find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGLWidgets)

# Link với GTest
target_link_libraries(${PROJECT_NAME}
        PRIVATE
#        GTest::gtest_main
        GTest::gtest
        Qt6::Widgets
        Qt6::OpenGLWidgets
)

# Tự động phát hiện các hàm TEST() trong code
#include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})