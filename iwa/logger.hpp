/**
 * @todo not a good colution lmao
 */

#pragma once

#include "singleton.hpp"
#include "config.hpp"

#include <unordered_set>
#include <unordered_map>
#include <sstream>

namespace iwa
{
    class logger final : public singleton<logger>
    {
    public:
        logger();
        struct channel
        {
            char color = 15;     
            char bgcolor = 0;
        };  
        
        void listen(const std::string& channel);
        void ignore(const std::string& channel);
        void log(const std::string& channel, char color, const char* format, ...);

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


#define LOG(channel, color, format, ...)   iwa::logger::get_instance()->log(channel, color, "{%s:%i} " format, __FILE_NAME__, __LINE__ __VA_OPT__(, ) __VA_ARGS__);

#ifdef IWA_LOG_VERBOSE
#define LOGV(format, ...)   iwa::logger::get_instance()->log("VERBOSE", iwa::logger::COLOR_DARK_GRAY, "{%s:%i} " format, __FILE_NAME__, __LINE__ __VA_OPT__(, ) __VA_ARGS__);
#else
#define LOGV(format, ...);
#endif

#ifdef IWA_LOG_DEBUG
#define LOGD(format, ...) iwa::logger::get_instance()->log("DEBUG", iwa::logger::COLOR_LIGHT_GRAY, "{%s:%i} " format, __FILE_NAME__, __LINE__ __VA_OPT__(, ) __VA_ARGS__);
#else
#define LOGD(format, ...);
#endif

#ifdef IWA_LOG_WARN
#define LOGW(format, ...) iwa::logger::get_instance()->log("WARN", iwa::logger::COLOR_LIGHT_YELLOW, "{%s:%i} " format, __FILE_NAME__, __LINE__ __VA_OPT__(, ) __VA_ARGS__);
#else
#define LOGW(format, ...);
#endif

#ifdef IWA_LOG_ERROR
#define LOGE(format, ...)  iwa::logger::get_instance()->log("ERROR", iwa::logger::COLOR_LIGHT_RED,  "{%s:%i} " format, __FILE_NAME__, __LINE__ __VA_OPT__(, ) __VA_ARGS__);
#else
#define LOGE(format, ...);
#endif

#ifdef IWA_LOG_INFO
#define LOGI(format, ...) iwa::logger::get_instance()->log("INFO", iwa::logger::COLOR_LIGHT_CYAN,  "{%s:%i} " format, __FILE_NAME__, __LINE__ __VA_OPT__(, ) __VA_ARGS__);
#else
#define LOGI(format, ...);
#endif
