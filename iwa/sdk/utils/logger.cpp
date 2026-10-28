#include "logger.hpp"
#include <cstdint>
#include <string>
#include <string_view>

using namespace Iwa;
using namespace Iwa::Internal;

uint8_t Internal::TabCount = 0;

FmtString::FmtString(std::string_view str, const std::source_location& loc) : str(str), loc(loc) {}
FmtString::FmtString(const char* str, const std::source_location& loc) : str(str), loc(loc) {}


// todo: implement error handling
void Iwa::LogPushTab()
{
    Internal::TabCount += 1;
}

void Iwa::LogPopTab()
{
    Internal::TabCount -= 1;
}
