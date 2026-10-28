#include "logger.hpp"
#include <string>
#include <string_view>

using namespace Iwa;

FmtString::FmtString(std::string_view str, const std::source_location& loc) : str(str), loc(loc) {}
FmtString::FmtString(const char* str, const std::source_location& loc) : str(str), loc(loc) {}
