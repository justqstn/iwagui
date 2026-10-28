cmake_minimum_required(VERSION 3.31)

project(example)

set (CMAKE_CXX_STANDARD 26)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
set (ROOT ${CMAKE_SOURCE_DIR}/../..)

find_library(D3DCompiler D3DCompiler)
find_library(D3D11 D3D11)
find_library(dwmapi dwmapi)

add_subdirectory(${ROOT}/iwa ${CMAKE_CURRENT_BINARY_DIR}/iwagui)
add_subdirectory(${ROOT}/iwa/backends/windows
	${CMAKE_CURRENT_BINARY_DIR}/iwagui/ossdk)

add_subdirectory(${ROOT}/iwa/backends/directx11
	${CMAKE_CURRENT_BINARY_DIR}/iwagui/rendersdk)

add_executable(main main.cpp)

target_compile_options(iwagui PUBLIC -std=c++26)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(iwagui PUBLIC
    -Wno-c23-extensions)
endif()

target_link_libraries(main PUBLIC
    iwaguiRenderSDK_lib
    iwaguiOSSDK_lib
    iwagui
    D3DCompiler
    D3D11
    dwmapi
    stdc++exp
)


if(${CMAKE_BUILD_TYPE} STREQUAL release)
    target_compile_options(main PUBLIC -flto -O3 -DNDEBUG
    -ffunction-sections -fdata-sections)
    target_link_options(main PUBLIC -s -fuse-ld=lld -Wl,--gc-sections)
else()
    target_link_options(main PUBLIC -v)
    target_compile_options(main PUBLIC -g)
endif()
