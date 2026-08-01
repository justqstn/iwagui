/**
 * @todo This will need to be redone later. At this moment it's just a temporary solution. At least i want to channels and more detailed settings.
 */

#pragma once

#include <Windows.h>
#include "singleton.hpp"

//#define IWA_LOG_VERBOSE
#define IWA_LOG_DEBUG
#define IWA_LOG_WARN
#define IWA_LOG_ERROR
#define IWA_LOG_INFO

namespace iwa
{
    class logger
    {
        
    };
}



namespace iwa
{
    void log(const char* type, char code, const char* format, ...);
    void savelogs();

    #ifdef IWA_LOG_VERBOSE
        #define LOGV(format, ...) iwa::log("VERBOSE", 8, \
            "{%s:%i} " format, __FILE_NAME__, __LINE__ __VA_OPT__(,) __VA_ARGS__);
    #else
        #define LOGV(format, ...)
    #endif 

    #ifdef IWA_LOG_DEBUG
        #define LOGD(format, ...) iwa::log("DEBUG", 8, \
            "{%s:%i} " format, __FILE_NAME__, __LINE__ __VA_OPT__(,) __VA_ARGS__);
    #else
        #define LOGD(format, ...)  
    #endif   

    #ifdef IWA_LOG_WARN
        #define LOGW(format, ...) iwa::log("WARN", FOREGROUND_RED | FOREGROUND_GREEN,\
             "{%s:%i} " format, __FILE_NAME__, __LINE__ __VA_OPT__(,) __VA_ARGS__);
    #else
        #define LOGW(format, ...)
    #endif   

    #ifdef IWA_LOG_ERROR
        #define LOGE(format, ...) iwa::log("ERROR", FOREGROUND_RED,\
             "{%s:%i} " format, __FILE_NAME__, __LINE__ __VA_OPT__(,) __VA_ARGS__);
    #else
        #define LOGE(format, ...)
    #endif   

    #ifdef IWA_LOG_INFO
        #define LOGI(format, ...) iwa::log("INFO", 15, "{%s:%i} " format, __FILE_NAME__, __LINE__ __VA_OPT__(,) __VA_ARGS__);
    #else
        #define LOGI(format, ...)
    #endif   
}