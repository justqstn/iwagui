#include "font.hpp"
#include "logger.hpp"
#include <unordered_map>

std::unordered_map<std::string, ImFont*> fonts;


ImFont* iwa::load_font(const std::string& id, std::string_view path_to_font)
{
    auto& io = ImGui::GetIO();
    auto font = io.Fonts->AddFontFromFileTTF(path_to_font.data());
    fonts.emplace(id, font);
    return font;
}

ImFont* iwa::load_font(const std::string& id, void* data, size_t data_size)
{
    auto& io = ImGui::GetIO();
    auto font = io.Fonts->AddFontFromMemoryTTF(data, data_size);
    fonts.emplace(id, font);
    return font;
}

ImFont* iwa::get_font(const std::string& id)
{
    if (fonts.contains(id)) return fonts[id];
    else
    {
        LOGE("Couldn't get font '%s': there is no loaded font with given id.", id);
        return nullptr;
    }
}