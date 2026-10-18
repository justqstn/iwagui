#pragma once

#include "utils/signal.hpp"
#include "sdk/enums.hpp"

#include <cstdint>
namespace Iwa
{
    namespace SDK
    {
        VKey toIwaKey(int Value);
        bool isVKeyPressed(VKey key);
        bool isVkeyClicked(VKey key);
        bool isVkeyReleased(VKey key);

        extern Iwa::Signal<VKey> onKeyClickedSignal;
        extern Iwa::Signal<VKey> onKeyPressedSignal;
        extern Iwa::Signal<VKey> onKeyReleasedSignal;

    } // namespace SDK
} // namespace Iwa
