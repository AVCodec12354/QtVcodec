cmake_minimum_required(VERSION 3.16)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(PROJECT_NAME QtVCodec)
project(${PROJECT_NAME} LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
        openapv_lib
        GIT_REPOSITORY https://github.com/AcademySoftwareFoundation/openapv
        GIT_TAG        main
)

FetchContent_GetProperties(openapv_lib)
if(NOT openapv_lib_POPULATED)
    FetchContent_Populate(openapv_lib)

    # 1. Fix lỗi thiếu file oapv.h ở project gốc
    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/inc/oapv.h")
        file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/inc")
        file(COPY "${openapv_lib_SOURCE_DIR}/inc/oapv.h" DESTINATION "${CMAKE_SOURCE_DIR}/inc")
    endif()

    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/pkgconfig/oapv.pc.in")
        file(MAKE_DIRECTORY "${CMAKE_SOURCE_DIR}/pkgconfig")
        file(COPY "${openapv_lib_SOURCE_DIR}/pkgconfig/oapv.pc.in" DESTINATION "${CMAKE_SOURCE_DIR}/pkgconfig")
    endif()

    # add OpenAPV vào project
    add_subdirectory(${openapv_lib_SOURCE_DIR} ${openapv_lib_BINARY_DIR})
endif()


find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGLWidgets)

add_executable(${PROJECT_NAME}
        src/main.cpp
        renderers/VideoGLWidget.cpp
        codecs/bases/C2Component.cpp
        codecs/apv/enc/C2ApvEnc.cpp
        codecs/apv/dec/C2ApvDec.cpp
        ui/mainwindow.cpp
        ui/mainwindow.h
        ui/mainwindow.ui
)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${openapv_lib_SOURCE_DIR}/inc
        ${CMAKE_CURRENT_SOURCE_DIR}/ui
        ${CMAKE_CURRENT_SOURCE_DIR}/renderers
        ${CMAKE_CURRENT_SOURCE_DIR}/codecs
        ${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::Widgets
        Qt6::OpenGLWidgets
        oapv
)
