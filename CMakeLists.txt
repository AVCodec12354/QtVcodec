cmake_minimum_required(VERSION 3.16)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(PROJECT_NAME QtVCodec)
project(${PROJECT_NAME} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Cấu hình thư mục xuất file thực thi
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/outputs)

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

# --- Fetch Google Test ---
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        main
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGLWidgets)

add_executable(${PROJECT_NAME}
        src/main.cpp

        inc/Qv2Core/Qv2ComponentFactory.cpp

        readers/implements/YUVSource.cpp
        readers/implements/Y4MSource.cpp

        renderers/implements/VideoRenderer.cpp
        renderers/interfaces/VideoGLWidget.h

        codecs/apv/enc/Qv2ApvEncoder.cpp
        codecs/apv/dec/Qv2ApvDecoder.cpp
        codecs/hevc/enc/Qv2HevcEncoder.cpp
        codecs/hevc/dec/Qv2HevcDecoder.cpp

        ui/helpers/QTLogger.cpp
        ui/view/mainwindow.cpp
        ui/view/mainwindow.ui
        ui/viewmodels/EncoderTabViewModel.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${openapv_lib_SOURCE_DIR}/inc
        ${CMAKE_CURRENT_SOURCE_DIR}/inc/Qv2Core

        ${CMAKE_CURRENT_SOURCE_DIR}/ui
        ${CMAKE_CURRENT_SOURCE_DIR}/ui/helpers
        ${CMAKE_CURRENT_SOURCE_DIR}/ui/view
        ${CMAKE_CURRENT_SOURCE_DIR}/ui/viewmodels

        ${CMAKE_CURRENT_SOURCE_DIR}/readers/interfaces
        ${CMAKE_CURRENT_SOURCE_DIR}/readers/implements

        ${CMAKE_CURRENT_SOURCE_DIR}/renderers/interfaces
        ${CMAKE_CURRENT_SOURCE_DIR}/renderers/implements

        ${CMAKE_CURRENT_SOURCE_DIR}/codecs
        ${CMAKE_CURRENT_SOURCE_DIR}/codecs/apv/enc
        ${CMAKE_CURRENT_SOURCE_DIR}/codecs/apv/dec
        ${CMAKE_CURRENT_SOURCE_DIR}/codecs/hevc/enc
        ${CMAKE_CURRENT_SOURCE_DIR}/codecs/hevc/dec
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::Widgets
        Qt6::OpenGLWidgets
        oapv
)

# Thêm thư mục tests
add_subdirectory(codecs/tests)
add_subdirectory(readers/tests)
add_subdirectory(renderers/tests)