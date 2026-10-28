// --- Standalone settings ---
// [SR]     Standalone rendering.
// ||       If enabled, render handling will be more simplifed.
// ||       E.g in DX11 you won't needed to call DXGISwapChain::Present()
// ||       or handle other things.
#define IWA_SR
#ifdef IWA_SR
    // --- Automatically swap buffers at the end of render call.
    #define IWA_SR_AUTO_SWAP

    // --- Handles resize (automatically resizes viewport to window's size)
    #define  IWA_SR_AUTO_RESIZE
#endif

// ---- Logging ----
// --- If not enabled, there still will be assert expressions in fatal logs
#define IWA_LOGGING

// --- If not enabled, you will use Iwa::Vec2_t instead of Vec2_t... terrible i think
#define IWA_USING_MATH_NAMESPACE

// ---- Custom assert ----
#include <cassert>
#define IWA_ASSERT(x) assert(x)

// ---- Debug configuration ----
// #define IWA_DEBUG // Is debugging enabled
// #ifdef IWA_DEBUG
//     // ...
// #endif
