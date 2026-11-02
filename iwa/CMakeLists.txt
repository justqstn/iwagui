cmake_minimum_required(VERSION 3.31)

project(iwagui)

set (CMAKE_CXX_STANDARD 26)

set (ROOT ${CMAKE_SOURCE_DIR}/../..)

aux_source_directory(./utils utils_srcs)
aux_source_directory(./abstractions/inputs inputs_srcs)

add_library(iwagui STATIC
    ${utils_srcs}
    ${inputs_srcs})

target_include_directories(iwagui PUBLIC
    .
)


message("iwagui build type: " ${CMAKE_BUILD_TYPE})

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(iwagui PRIVATE -Wno-c23-extensions)
endif()
if(${CMAKE_BUILD_TYPE} STREQUAL release)
    target_link_options(iwagui PRIVATE -s -Wl,--gc-sections)
    target_compile_options(iwagui PRIVATE
        -O3
        -DNDEBUG
        -fno-exceptions
        -ffunction-sections
        -fdata-sections
        )
else()
    target_compile_options(iwagui PRIVATE -g)
endif()

target_precompile_headers(iwagui PRIVATE
    ./abstractions/abstractions.hpp
    ./abstractions/rendering/types.hpp
    ./abstractions/rendering/fonts.hpp
    ./abstractions/rendering/renderer.hpp
    ./utils/signal.hpp
    ./utils/math.hpp
    ./utils/logger.hpp
)
