#pragma once

#include "abstractions/inputs/enums.hpp"
#include "utils/math.hpp"
#include "utils/signal.hpp"

#include <cstdint>

namespace Iwa
{
    VKey toIwaKey(int Value);
    bool isVKeyPressed(VKey key);
    bool isVkeyClicked(VKey key);
    bool isVkeyReleased(VKey key);

    // Client-space mouse cursor position (pixels), updated by the OS backend.
    extern Vec2_t mouseCursorPos;

    extern Signal<VKey> onKeyClickedSignal;
    extern Signal<VKey> onKeyPressedSignal;
    extern Signal<VKey> onKeyReleasedSignal;
} // namespace Iwa
