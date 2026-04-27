cmake_minimum_required(VERSION 3.16)

project(Qv2ComponentTests LANGUAGES CXX)

find_package(GTest REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/testWorkSpace)

include_directories(
    ${CMAKE_SOURCE_DIR}/codecs/Qv2Core
    ${CMAKE_SOURCE_DIR}/codecs/apv
    ${CMAKE_SOURCE_DIR}/codecs/apv/enc
    ${CMAKE_SOURCE_DIR}/codecs/apv/dec
    ${CMAKE_SOURCE_DIR}/codecs/hevc/enc
    ${CMAKE_SOURCE_DIR}/codecs/hevc/dec
    ${CMAKE_SOURCE_DIR}/inc/Qv2Core
    ${CMAKE_SOURCE_DIR}/inc
)

set(CORE_SOURCES
    ${CMAKE_SOURCE_DIR}/inc/Qv2Core/Qv2ComponentFactory.cpp
    ${CMAKE_SOURCE_DIR}/codecs/apv/enc/Qv2ApvEncoder.cpp
    ${CMAKE_SOURCE_DIR}/codecs/apv/dec/Qv2ApvDecoder.cpp
    ${CMAKE_SOURCE_DIR}/codecs/hevc/enc/Qv2HevcEncoder.cpp
    ${CMAKE_SOURCE_DIR}/codecs/hevc/dec/Qv2HevcDecoder.cpp
)

add_executable(Qv2ComponentTests 
    testMain.cpp
    Qv2EncoderTests.cpp
    Qv2ComponentBaseTests.cpp
    ${CORE_SOURCES}
)

target_link_libraries(Qv2ComponentTests 
    PRIVATE 
    oapv
    GTest::gtest
    GTest::gtest_main
)