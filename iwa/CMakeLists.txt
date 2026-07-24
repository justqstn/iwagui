cmake_minimum_required(VERSION 3.31)

project(iwagui)

set (CMAKE_CXX_STANDARD 20)

set (ROOT ${CMAKE_SOURCE_DIR}/../..)

add_subdirectory(${ROOT}/deps/ImGui ${CMAKE_CURRENT_BINARY_DIR}/imgui)

aux_source_directory(./ui ui_srcs)  
aux_source_directory(. srcs)  

add_library(iwagui STATIC ${srcs} ${ui_srcs} $<TARGET_OBJECTS:ImGui_lib>)

target_include_directories(iwagui PUBLIC 
    ${ROOT}/deps/ImGui
    .
    ./ui
)

target_compile_options(iwagui PRIVATE -O3 -flto=auto)
