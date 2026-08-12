
#define IWA_DEBUG
#define IWA_LOGGING



#ifdef IWA_DEBUG
    #define IWA_render_controller_DEBUG
#endif

#ifdef IWA_LOGGING
    #define IWA_LOG
    // #define IWA_LOG_VERBOSE
    #define IWA_LOG_DEBUG
    #define IWA_LOG_WARN
    #define IWA_LOG_ERROR
    #define IWA_LOG_INFO
#endif