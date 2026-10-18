/*
 * Standalone settings
 */
// ---
// |Standalone rendering (SR).
// |     If enabled, render handling will be more simplifed.
// |     E.g in DX11 you won't needed to call DXGISwapChain::Present()
// |     or handle another things.
// __
#define IWA_SR
#ifdef IWA_SR
    // |Auto swap.
    // |     Automatically swap buffers at the end of render call.
    #define IWA_SR_AUTO_SWAP
#endif



// ---
/*
 * Debug configuration
 */
#define IWA_DEBUG // Is debugging enabled
#ifdef IWA_DEBUG
    // ...
#endif


#define IWA_LOGGING
#ifdef IWA_LOGGING
    #define IWA_LOG
    // #define IWA_LOG_VERBOSE
    #define IWA_LOG_DEBUG
    #define IWA_LOG_WARN
    #define IWA_LOG_ERROR
    #define IWA_LOG_INFO
#endif
