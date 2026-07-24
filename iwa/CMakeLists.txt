cmake_minimum_required(VERSION 3.31)

project(iwagui)

set (CMAKE_CXX_STANDARD 20)

set (ROOT ${CMAKE_SOURCE_DIR}/../..)

add_subdirectory(${ROOT}/deps/ImGui ${CMAKE_CURRENT_BINARY_DIR}/imgui)

aux_source_directory(./ui ui_srcs)  
aux_source_directory(./ui/widgets widgets_srcs)  
aux_source_directory(./ui/utils utils_srcs)  
aux_source_directory(./ui/bases bases_srcs)  
aux_source_directory(./ui/animations animations_srcs)  
aux_source_directory(. srcs)  

add_library(iwagui STATIC ${srcs} ${ui_srcs} ${widgets_srcs} ${animations_srcs} ${utils_srcs} ${bases_srcs}  $<TARGET_OBJECTS:ImGui_lib>)

target_include_directories(iwagui PUBLIC 
    ${ROOT}/deps/ImGui
    .
    ./ui
    ./ui/widgets
    ./ui/utils
    ./ui/bases
    ./ui/animations
)

# "-0z -flto=auto" doesn't work due to internal compiler error
target_compile_options(iwagui PRIVATE -O3 -flto)
