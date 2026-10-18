cmake_minimum_required(VERSION 3.31)

project(iwaguiRenderCore)

set(SOURCES
    sdk/directx11.cpp
)

set (ROOT ${CMAKE_SOURCE_DIR}/../../iwa)

add_library(iwaguiRenderSDK_lib ${SOURCES})
target_include_directories(iwaguiRenderSDK_lib PUBLIC
    ${ROOT}
    .
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(iwaguiRenderSDK_lib PRIVATE -Wno-c23-extensions)
endif()

if(${CMAKE_BUILD_TYPE} STREQUAL release)
    target_compile_options(iwaguiRenderSDK_lib PRIVATE -O3)
    target_link_options(iwaguiRenderSDK_lib PRIVATE -s)
else()
    target_compile_options(iwaguiRenderSDK_lib PRIVATE -g)
endif()
