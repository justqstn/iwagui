cmake_minimum_required(VERSION 3.31)

project(iwagui)

set (CMAKE_CXX_STANDARD 26)

set (ROOT ${CMAKE_SOURCE_DIR}/../..)

add_subdirectory(${ROOT}/deps/ImGui ${CMAKE_CURRENT_BINARY_DIR}/imgui)

aux_source_directory(./ui ui_srcs)  
aux_source_directory(./ui/widgets widgets_srcs)  
aux_source_directory(./ui/utils utils_srcs)  
aux_source_directory(./ui/core core_srcs)  
aux_source_directory(./ui/animations animations_srcs)  
aux_source_directory(./ui/controllers controllers_srcs)  
aux_source_directory(. srcs)  

add_library(iwagui STATIC ${srcs} ${ui_srcs} ${widgets_srcs} ${animations_srcs} ${utils_srcs} ${controllers_srcs} ${core_srcs}  $<TARGET_OBJECTS:ImGui_lib>)

target_include_directories(iwagui PUBLIC 
    ${ROOT}/deps/ImGui
    .
    ./ui
    ./ui/widgets
    ./ui/utils
    ./ui/core
    ./ui/animations
    ./ui/controllers
    ./ui/internal
)

target_precompile_headers(iwagui PUBLIC
     ui/core/widget.hpp
     ui/animations/tween.hpp    
     ui/widgets/frame.hpp
     ui/core/context.hpp
 )

message("iwagui build type: " ${CMAKE_BUILD_TYPE})
if(${CMAKE_BUILD_TYPE} STREQUAL release)
    target_link_options(iwagui PRIVATE -s)
    target_compile_options(iwagui PRIVATE -O3 -DNDEBUG -flto -ftemplate-backtrace-limit=0)
else()
    target_compile_options(iwagui PRIVATE -fconcepts-diagnostics-depth=10 -g)
endif()

