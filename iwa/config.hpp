// --- Standalone settings ---
// [SR]     Standalone rendering.
// ||       If enabled, render handling will be more simplifed.
// ||       E.g in DX11 you won't needed to call DXGISwapChain::Present()
// ||       and handle other similar things.
#define IWA_SR
#ifdef IWA_SR
    // --- Lowers refresh rate when windows is not active/forground
    #define IWA_SR_LAZY_SWAP_FACTOR 2

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
#define IWA_DEBUGGING // Is debugging enabled
#ifdef IWA_DEBUGGING
    // Saves debug information in method
    #define IWA_FRAME_DEBUG_INFO
#endif
