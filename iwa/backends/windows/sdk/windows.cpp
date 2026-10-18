#include "sdk/windows.hpp"
#include "sdk/inputs.hpp"
#include "sdk/utils/logger.hpp"

#include <array>
#include <windows.h>

using namespace Iwa;
using namespace Iwa::SDK;

struct
{
    std::array<bool, static_cast<size_t>(VKey::VKEYS_COUNT)> pressed_keys;
} data;

bool SDK::isVKeyPressed(VKey key)
{
    return data.pressed_keys[static_cast<size_t>(key)];
}

bool Iwa::WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
    {
        VKey key = SDK::toIwaKey(static_cast<int>(wParam));
        bool& key_status = data.pressed_keys[static_cast<size_t>(key)];
        if (!key_status)
        {
            key_status = true;
            SDK::onKeyClickedSignal.call(key);
        }
        SDK::onKeyPressedSignal.call(key);
        break;
    }
    case WM_SYSKEYUP:
    case WM_KEYUP:
    {
        VKey key = SDK::toIwaKey(static_cast<int>(wParam));
        data.pressed_keys[static_cast<size_t>(key)] = false;
        SDK::onKeyReleasedSignal.call(key);
        break;
    }
    default:
        return false;
    }
    return true;
}

VKey SDK::toIwaKey(int Value)
{
    switch (Value)
    {
    case VK_F1:
        return VKey::F1;
    case VK_F2:
        return VKey::F2;
    case VK_F3:
        return VKey::F3;
    case VK_F4:
        return VKey::F4;
    case VK_F5:
        return VKey::F5;
    case VK_F6:
        return VKey::F6;
    case VK_F7:
        return VKey::F7;
    case VK_F8:
        return VKey::F8;
    case VK_F9:
        return VKey::F9;
    case VK_F10:
        return VKey::F10;
    case VK_F11:
        return VKey::F11;
    case VK_F12:
        return VKey::F12;
    case VK_RSHIFT:
        return VKey::RSHIFT;
    case VK_LSHIFT:
        return VKey::LSHIFT;
    default:
        return VKey::NONE;
    }
}
