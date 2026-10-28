#pragma once
#include "config.hpp"

#include <cassert>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <format>
#include <iostream>
#include <print>
#include <source_location>
#include <string>
#include <string_view>


namespace Iwa
{
    namespace Internal
    {
        struct FmtString
        {
          std::string_view str;
          std::source_location loc;

          FmtString(std::string_view str, const std::source_location& loc = std::source_location::current());
          FmtString(const char* str, const std::source_location& loc = std::source_location::current());
        };

        extern uint8_t TabCount;
    }

    void LogPushTab();
    void LogPopTab();

    #ifdef IWA_LOGGING
    template <typename... TArgs>
    void Log(std::string_view channel, std::string_view color, const Internal::FmtString& fmt, TArgs... args)
    {
        auto filename = static_cast<std::string_view>(fmt.loc.file_name());

        char* tabs = nullptr;
        if (Internal::TabCount > 0)
        {
            tabs = new char[Internal::TabCount+1];
            std::memset(tabs, '\t', Internal::TabCount);
            tabs[Internal::TabCount] = '\0';
        }
        else
            tabs = const_cast<char*>("");


        std::println(std::dynamic_format(std::format(
                         "\033[{}m({:%T}) [{}] {}{}:{}{} {}{}",
                         color,
                         std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now()),
                         channel,
                         "{{",
                         filename.substr(filename.find_last_of("/") + 1),
                         fmt.loc.line(),
                         "}}",
                         tabs,
                         fmt.str
        )),
                     args...);

        if (Internal::TabCount > 0)
            delete[] tabs;
    }
    #else
    template <typename... TArgs>
    inline void Log(std::string_view channel, std::string_view color, const FmtString& fmt, TArgs... args) {}
    #endif

    template <typename... TArgs>
    inline void LogFatal(const Internal::FmtString& fmt, TArgs... args)
    {
        Log("FATAL", "38;2;190;20;20", fmt, args...);
        return IWA_ASSERT(0 && "Fatal error!" == std::format(std::dynamic_format(fmt.str), args...));
    }

    template <typename... TArgs>
    inline void LogError(const Internal::FmtString& fmt, TArgs... args)
    {
        return Log("ERROR", "38;2;255;20;20", fmt, args...);
    }

    template <typename... TArgs>
    inline void LogWarn(const Internal::FmtString& fmt, TArgs... args)
    {
        return Log("WARN", "38;2;255;196;60", fmt, args...);
    }

    template <typename... TArgs>
    inline void LogInfo(const Internal::FmtString& fmt, TArgs... args)
    {
        return Log("INFO", "38;2;128;128;255", fmt, args...);
    }

    template <typename... TArgs>
    inline void LogDebug(const Internal::FmtString& fmt, TArgs... args)
    {
        return Log("DEBUG", "38;2;200;200;200", fmt, args...);
    }

    template <typename... TArgs>
    inline void LogVerbose(const Internal::FmtString& fmt, TArgs... args)
    {
        return Log("VERBOSE", "38;2;100;100;100", fmt, args...);
    }
}
