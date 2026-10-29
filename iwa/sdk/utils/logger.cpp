#include "logger.hpp"
#include <cstdint>
#include <string>
#include <string_view>

using namespace Iwa;
using namespace Iwa::__private__;

uint8_t __private__::TabCount = 0;

FmtString::FmtString(std::string_view str, const std::source_location& loc) : str(str), loc(loc) {}
FmtString::FmtString(const char* str, const std::source_location& loc) : str(str), loc(loc) {}


// todo: implement error handling
void Iwa::LogPushTab()
{
    __private__::TabCount += 1;
}

void Iwa::LogPopTab()
{
    if (__private__::TabCount == 0) LogWarn("Missing LogPushTab(), ignoring pop");
    else __private__::TabCount -= 1;
}
