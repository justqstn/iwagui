cmake_minimum_required(VERSION 3.31)

project(iwagui)

set (CMAKE_CXX_STANDARD 20)

set (ROOT ${CMAKE_SOURCE_DIR}/../..)

add_subdirectory(${ROOT}/deps/ImGui ${CMAKE_CURRENT_BINARY_DIR}/imgui)

aux_source_directory(./ui ui_srcs)  
aux_source_directory(./ui/widgets widgets_srcs)  
aux_source_directory(./ui/utils utils_srcs)  
aux_source_directory(./ui/core core_srcs)  
aux_source_directory(./ui/animations animations_srcs)  
aux_source_directory(. srcs)  

add_library(iwagui STATIC ${srcs} ${ui_srcs} ${widgets_srcs} ${animations_srcs} ${utils_srcs} ${core_srcs}  $<TARGET_OBJECTS:ImGui_lib>)

target_include_directories(iwagui PUBLIC 
    ${ROOT}/deps/ImGui
    .
    ./ui
    ./ui/widgets
    ./ui/utils
    ./ui/core
    ./ui/animations
)

# -0z doesn't work due to internal compiler error
target_compile_options(iwagui PRIVATE -Ofast -flto)
