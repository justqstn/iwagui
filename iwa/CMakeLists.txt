cmake_minimum_required(VERSION 3.31)

project(iwagui)

set (CMAKE_CXX_STANDARD 26)

set (ROOT ${CMAKE_SOURCE_DIR}/../..)

aux_source_directory(./sdk                     sdk_srcs)
aux_source_directory(./sdk/utils               sdk_utils_srcs)

add_library(iwagui STATIC
    ${sdk_utils_srcs}
    ${sdk_srcs})

target_include_directories(iwagui PUBLIC
    ${ROOT}/deps/ImGui
    .
    ./sdk/utils
    ./sdk
)

message("iwagui build type: " ${CMAKE_BUILD_TYPE})

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(iwagui PRIVATE -Wno-c23-extensions)
endif()
if(${CMAKE_BUILD_TYPE} STREQUAL release)
    target_link_options(iwagui PRIVATE -s)
    target_compile_options(iwagui PRIVATE -O3 -DNDEBUG
    	-ftemplate-backtrace-limit=0)
else()
    target_compile_options(iwagui PRIVATE -g)
endif()
