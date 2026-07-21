cmake_minimum_required(VERSION 3.31)

project(ImGui)

set(SOURCES 
    imgui_demo.cpp
    imgui_impl_win32.cpp
    imgui_impl_dx11.cpp
    imgui.cpp
    imgui_widgets.cpp
    imgui_tables.cpp
    imgui_draw.cpp
)

add_library(ImGui_lib STATIC ${SOURCES})
target_compile_options(ImGui_lib PRIVATE -fpermissive -Oz)