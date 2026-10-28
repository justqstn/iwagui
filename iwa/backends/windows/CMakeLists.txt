cmake_minimum_required(VERSION 3.31)

project(iwaguiOSSDK)

set(SOURCES
    sdk/windows.cpp
)

set (ROOT ${CMAKE_SOURCE_DIR}/../../iwa)

add_library(iwaguiOSSDK_lib ${SOURCES})
target_include_directories(iwaguiOSSDK_lib PUBLIC
    .
)


if(${CMAKE_BUILD_TYPE} STREQUAL release)
    target_compile_options(iwaguiOSSDK_lib PRIVATE -O3)
    target_link_options(iwaguiOSSDK_lib PRIVATE -s)
else()
    target_compile_options(iwaguiOSSDK_lib PRIVATE -g)
endif()
