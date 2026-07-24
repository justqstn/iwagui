#pragma once
/**
 * @brief Basic bindings for load and get fonts
 */

#ifdef _WIN32
#define FONTS_DIR "C:\\Windows\\Fonts\\"
#endif

#include <string>
#include "imgui.h"

namespace iwa
{
    ImFont* load_font(std::string_view id, std::string_view path_to_font);
    ImFont* load_font(std::string_view id, void* data, size_t data_size);
    ImFont* get_font(std::string_view id);
}