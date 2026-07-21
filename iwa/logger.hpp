/**
 * @todo This will need to be redone later. At this moment it's just a temporary solution. At least i want to channels and more detailed settings.
 */

#pragma once

#include <Windows.h>

#define IWA_DEBUG
#define IWA_WARN
#define IWA_ERROR
#define IWA_INFO

namespace iwa
{
    void log(const char* type, char code, const char* format, ...);
    void savelogs();

    #ifdef IWA_DEBUG
        #define LOGD(format, ...) iwa::log("DEBUG", 8, \
            "{%s:%i} " format, __FILE_NAME__, __LINE__ __VA_OPT__(,) __VA_ARGS__);
    #else
        #define LOGD  
    #endif   

    #ifdef IWA_WARN
        #define LOGW(format, ...) iwa::log("WARN", FOREGROUND_RED | FOREGROUND_GREEN,\
             "{%s:%i} " format, __FILE_NAME__, __LINE__ __VA_OPT__(,) __VA_ARGS__);
    #else
        #define LOGW
    #endif   

    #ifdef IWA_ERROR
        #define LOGE(format, ...) iwa::log("ERROR", FOREGROUND_RED,\
             "{%s:%i} " format, __FILE_NAME__, __LINE__ __VA_OPT__(,) __VA_ARGS__);
    #else
        #define LOGE
    #endif   

    #ifdef IWA_INFO
        #define LOGI(format, ...) iwa::log("INFO", 15, "{%s:%i} " format, __FILE_NAME__, __LINE__ __VA_OPT__(,) __VA_ARGS__);
    #else
        #define LOGI
    #endif   
}