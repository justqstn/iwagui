/**
 * @todo not a good colution lmao
 */

#pragma once

#include "singleton.hpp"
#include "config.hpp"

#include <unordered_set>
#include <unordered_map>
#include <string>

namespace Iwa
{
    class Logger final : public Singleton<Logger>
    {
    public:
        Logger();
        struct channel
        {
            char color = 15;
            char bgcolor = 0;
        };

        void Listen(const std::string& channel);
        void Ignore(const std::string& channel);
        void Log(const std::string& channel, char color, const char* format, ...);

        static const int COLOR_BLACK = 0;
        static const int COLOR_DARK_BLUE = 1;
        static const int COLOR_DARK_GREEN = 2;
        static const int COLOR_DARK_CYAN = 3;
        static const int COLOR_DARK_RED = 4;
        static const int COLOR_DARK_MAGENTA = 5;
        static const int COLOR_DARK_YELLOW = 6;
        static const int COLOR_LIGHT_GRAY = 7;
        static const int COLOR_DARK_GRAY = 8;
        static const int COLOR_LIGHT_BLUE = 9;
        static const int COLOR_LIGHT_GREEN = 10;
        static const int COLOR_LIGHT_CYAN = 11;
        static const int COLOR_LIGHT_RED = 12;
        static const int COLOR_LIGHT_MAGENTA = 13;
        static const int COLOR_LIGHT_YELLOW = 14;
        static const int COLOR_WHITE = 15;

    private:
        std::unordered_set<std::string> __ignored_channels;
        std::unordered_map<std::string, channel*> __channels_colors;
    };
}


#define LOG(channel, color, format, ...)   Iwa::Logger::getInstance()->Log(channel, color, "{%s:%i} " format, __FILE_NAME__, __LINE__ __VA_OPT__(, ) __VA_ARGS__);

#ifdef IWA_LOG_VERBOSE
#define LOGV(format, ...)   Iwa::Logger::getInstance()->Log("VERBOSE", Iwa::Logger::COLOR_DARK_GRAY, "{%s:%i} " format, __FILE_NAME__, __LINE__ __VA_OPT__(, ) __VA_ARGS__);
#else
#define LOGV(format, ...);
#endif

#ifdef IWA_LOG_DEBUG
#define LOGD(format, ...) Iwa::Logger::getInstance()->Log("DEBUG", Iwa::Logger::COLOR_LIGHT_GRAY, "{%s:%i} " format, __FILE_NAME__, __LINE__ __VA_OPT__(, ) __VA_ARGS__);
#else
#define LOGD(format, ...);
#endif

#ifdef IWA_LOG_WARN
#define LOGW(format, ...) Iwa::Logger::getInstance()->Log("WARN", Iwa::Logger::COLOR_LIGHT_YELLOW, "{%s:%i} " format, __FILE_NAME__, __LINE__ __VA_OPT__(, ) __VA_ARGS__);
#else
#define LOGW(format, ...);
#endif

#ifdef IWA_LOG_ERROR
#define LOGE(format, ...)  Iwa::Logger::getInstance()->Log("ERROR", Iwa::Logger::COLOR_LIGHT_RED,  "{%s:%i} " format, __FILE_NAME__, __LINE__ __VA_OPT__(, ) __VA_ARGS__);
#else
#define LOGE(format, ...);
#endif

#ifdef IWA_LOG_INFO
#define LOGI(format, ...) Iwa::Logger::getInstance()->Log("INFO", Iwa::Logger::COLOR_LIGHT_CYAN,  "{%s:%i} " format, __FILE_NAME__, __LINE__ __VA_OPT__(, ) __VA_ARGS__);
#else
#define LOGI(format, ...);
#endif
