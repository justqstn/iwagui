#pragma once

#include "utils/signal.hpp"
#include "abstractions/inputs/enums.hpp"

#include <cstdint>
namespace Iwa
{
    VKey toIwaKey(int Value);
    bool isVKeyPressed(VKey key);
    bool isVkeyClicked(VKey key);
    bool isVkeyReleased(VKey key);

    extern Signal<VKey> onKeyClickedSignal;
    extern Signal<VKey> onKeyPressedSignal;
    extern Signal<VKey> onKeyReleasedSignal;
} // namespace Iwa
