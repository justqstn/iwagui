cmake_minimum_required(VERSION 3.31)

project(example)

set (CMAKE_CXX_STANDARD 20)
set (ROOT ${CMAKE_SOURCE_DIR}/../..)

find_library(D3DCompiler D3DCompiler)
find_library(D3D11 D3D11)
find_library(dwmapi dwmapi)

add_subdirectory(${ROOT}/iwa ${CMAKE_CURRENT_BINARY_DIR}/iwagui)

aux_source_directory(. srcs)  
add_executable(main ${srcs})

target_link_libraries(main PUBLIC 
    iwagui 
    D3DCompiler
    D3D11
    dwmapi
)

target_compile_options(main PUBLIC -Oz -flto=auto)