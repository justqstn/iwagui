
#include <cstdarg>
#include <cstdio>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include "logger.hpp"

#if defined(_WIN32) || defined(WIN32)
    #include <windows.h>
#endif


using namespace Iwa;



Logger::Logger()
{
    __ignored_channels = {};
}

void Logger::Ignore(const std::string& channel)
{
    __ignored_channels.emplace(channel);
}

void Logger::Listen(const std::string& channel)
{
    __ignored_channels.erase(channel);
}


void Logger::Log(const std::string& channel, char color, const char* format, ...)
{
    if (__ignored_channels.contains(channel)) return;

    #if defined(_WIN32) || defined(WIN32)
        HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
        if (console == NULL) return;
        SetConsoleTextAttribute(console, color);
    #endif

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char buffer[100];
    strftime(buffer, sizeof(buffer), "%H:%M:%S", t);

    va_list args;
    va_start(args, format);

    printf("(%s) [%s] ", buffer, channel.data());
    vprintf(format, args);
    printf("\n");

    va_end(args);

    #if defined(_WIN32) || defined(WIN32)
        SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN);
    #endif
}
