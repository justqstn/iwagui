cmake_minimum_required(VERSION 3.31)

project(iwagui)

set (CMAKE_CXX_STANDARD 20)

add_subdirectory(deps/ImGui)

aux_source_directory(src/ui ui_srcs)  
aux_source_directory(src srcs)  

add_library(iwagui STATIC ${srcs} ${ui_srcs} $<TARGET_OBJECTS:ImGui_lib>)

target_include_directories(iwagui PUBLIC 
    deps/ImGui
    src
    src/ui
)

target_compile_options(iwagui PRIVATE -O3 -flto)
