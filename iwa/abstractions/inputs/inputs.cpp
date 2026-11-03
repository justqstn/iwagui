#include "abstractions/inputs/inputs.hpp"

namespace Iwa
{
    Vec2_t mouseCursorPos = {0.0f, 0.0f};

    Signal<VKey> onKeyClickedSignal = {};
    Signal<VKey> onKeyPressedSignal = {};
    Signal<VKey> onKeyReleasedSignal = {};
} // namespace Iwa
