#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif
#ifndef WINVER
#define WINVER _WIN32_WINNT
#endif

#include "iwa/windows.hpp"
#include "abstractions/inputs/inputs.hpp"
#include "utils/logger.hpp"

#include <array>
#include <windows.h>

#ifndef VK_NAVIGATION_VIEW
#define VK_NAVIGATION_VIEW 0x88
#define VK_NAVIGATION_MENU 0x89
#define VK_NAVIGATION_UP 0x8A
#define VK_NAVIGATION_DOWN 0x8B
#define VK_NAVIGATION_LEFT 0x8C
#define VK_NAVIGATION_RIGHT 0x8D
#define VK_NAVIGATION_ACCEPT 0x8E
#define VK_NAVIGATION_CANCEL 0x8F
#endif
#ifndef VK_GAMEPAD_A
#define VK_GAMEPAD_A 0xC3
#define VK_GAMEPAD_B 0xC4
#define VK_GAMEPAD_X 0xC5
#define VK_GAMEPAD_Y 0xC6
#define VK_GAMEPAD_RIGHT_SHOULDER 0xC7
#define VK_GAMEPAD_LEFT_SHOULDER 0xC8
#define VK_GAMEPAD_LEFT_TRIGGER 0xC9
#define VK_GAMEPAD_RIGHT_TRIGGER 0xCA
#define VK_GAMEPAD_DPAD_UP 0xCB
#define VK_GAMEPAD_DPAD_DOWN 0xCC
#define VK_GAMEPAD_DPAD_LEFT 0xCD
#define VK_GAMEPAD_DPAD_RIGHT 0xCE
#define VK_GAMEPAD_MENU 0xCF
#define VK_GAMEPAD_VIEW 0xD0
#define VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON 0xD1
#define VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON 0xD2
#define VK_GAMEPAD_LEFT_THUMBSTICK_UP 0xD3
#define VK_GAMEPAD_LEFT_THUMBSTICK_DOWN 0xD4
#define VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT 0xD5
#define VK_GAMEPAD_LEFT_THUMBSTICK_LEFT 0xD6
#define VK_GAMEPAD_RIGHT_THUMBSTICK_UP 0xD7
#define VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN 0xD8
#define VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT 0xD9
#define VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT 0xDA
#endif

using namespace Iwa;

struct
{
    std::array<bool, static_cast<size_t>(VKey::VKEYS_COUNT)> pressed_keys;
} data;

bool Iwa::isVKeyPressed(VKey key)
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
        VKey key = toIwaKey(static_cast<int>(wParam));
        bool& key_status = data.pressed_keys[static_cast<size_t>(key)];
        if (!key_status)
        {
            key_status = true;
            onKeyClickedSignal.call(key);
        }
        onKeyPressedSignal.call(key);
        break;
    }
    case WM_SYSKEYUP:
    case WM_KEYUP:
    {
        VKey key = toIwaKey(static_cast<int>(wParam));
        data.pressed_keys[static_cast<size_t>(key)] = false;
        onKeyReleasedSignal.call(key);
        break;
    }
    default:
        return false;
    }
    return true;
}

VKey Iwa::toIwaKey(int Value)
{
    switch (Value)
    {
    case VK_LBUTTON:
        return VKey::MOUSE_LEFT;
    case VK_RBUTTON:
        return VKey::MOUSE_RIGHT;
    case VK_CANCEL:
        return VKey::CANCEL;
    case VK_MBUTTON:
        return VKey::MOUSE_MIDDLE;
    case VK_XBUTTON1:
        return VKey::MOUSE_X1;
    case VK_XBUTTON2:
        return VKey::MOUSE_X2;

    case VK_BACK:
        return VKey::BACKSPACE;
    case VK_TAB:
        return VKey::TAB;
    case VK_CLEAR:
        return VKey::CLEAR;
    case VK_RETURN:
        return VKey::ENTER;
    case VK_SHIFT:
        return VKey::SHIFT;
    case VK_CONTROL:
        return VKey::CTRL;
    case VK_MENU:
        return VKey::ALT;
    case VK_PAUSE:
        return VKey::PAUSE;
    case VK_CAPITAL:
        return VKey::CAPSLOCK;
    case VK_KANA:
        return VKey::KANA;
    case VK_IME_ON:
        return VKey::IME_ON;
    case VK_JUNJA:
        return VKey::JUNJA;
    case VK_FINAL:
        return VKey::FINAL;
    case VK_KANJI:
        return VKey::KANJI;
    case VK_IME_OFF:
        return VKey::IME_OFF;
    case VK_ESCAPE:
        return VKey::ESCAPE;
    case VK_CONVERT:
        return VKey::CONVERT;
    case VK_NONCONVERT:
        return VKey::NONCONVERT;
    case VK_ACCEPT:
        return VKey::ACCEPT;
    case VK_MODECHANGE:
        return VKey::MODECHANGE;
    case VK_SPACE:
        return VKey::SPACE;
    case VK_PRIOR:
        return VKey::PAGE_UP;
    case VK_NEXT:
        return VKey::PAGE_DOWN;
    case VK_END:
        return VKey::END;
    case VK_HOME:
        return VKey::HOME;
    case VK_LEFT:
        return VKey::LEFT;
    case VK_UP:
        return VKey::UP;
    case VK_RIGHT:
        return VKey::RIGHT;
    case VK_DOWN:
        return VKey::DOWN;
    case VK_SELECT:
        return VKey::SELECT;
    case VK_PRINT:
        return VKey::PRINT;
    case VK_EXECUTE:
        return VKey::EXECUTE;
    case VK_SNAPSHOT:
        return VKey::SNAPSHOT;
    case VK_INSERT:
        return VKey::INSERT;
    case VK_DELETE:
        return VKey::KDELETE;
    case VK_HELP:
        return VKey::HELP;

    case '0':
        return VKey::N0;
    case '1':
        return VKey::N1;
    case '2':
        return VKey::N2;
    case '3':
        return VKey::N3;
    case '4':
        return VKey::N4;
    case '5':
        return VKey::N5;
    case '6':
        return VKey::N6;
    case '7':
        return VKey::N7;
    case '8':
        return VKey::N8;
    case '9':
        return VKey::N9;

    case 'A':
        return VKey::A;
    case 'B':
        return VKey::B;
    case 'C':
        return VKey::C;
    case 'D':
        return VKey::D;
    case 'E':
        return VKey::E;
    case 'F':
        return VKey::F;
    case 'G':
        return VKey::G;
    case 'H':
        return VKey::H;
    case 'I':
        return VKey::I;
    case 'J':
        return VKey::J;
    case 'K':
        return VKey::K;
    case 'L':
        return VKey::L;
    case 'M':
        return VKey::M;
    case 'N':
        return VKey::N;
    case 'O':
        return VKey::O;
    case 'P':
        return VKey::P;
    case 'Q':
        return VKey::Q;
    case 'R':
        return VKey::R;
    case 'S':
        return VKey::S;
    case 'T':
        return VKey::T;
    case 'U':
        return VKey::U;
    case 'V':
        return VKey::V;
    case 'W':
        return VKey::W;
    case 'X':
        return VKey::X;
    case 'Y':
        return VKey::Y;
    case 'Z':
        return VKey::Z;

    case VK_LWIN:
        return VKey::LWIN;
    case VK_RWIN:
        return VKey::RWIN;
    case VK_APPS:
        return VKey::APPS;
    case VK_SLEEP:
        return VKey::SLEEP;

    case VK_NUMPAD0:
        return VKey::NUMPAD0;
    case VK_NUMPAD1:
        return VKey::NUMPAD1;
    case VK_NUMPAD2:
        return VKey::NUMPAD2;
    case VK_NUMPAD3:
        return VKey::NUMPAD3;
    case VK_NUMPAD4:
        return VKey::NUMPAD4;
    case VK_NUMPAD5:
        return VKey::NUMPAD5;
    case VK_NUMPAD6:
        return VKey::NUMPAD6;
    case VK_NUMPAD7:
        return VKey::NUMPAD7;
    case VK_NUMPAD8:
        return VKey::NUMPAD8;
    case VK_NUMPAD9:
        return VKey::NUMPAD9;
    case VK_MULTIPLY:
        return VKey::NUMPAD_MULTIPLY;
    case VK_ADD:
        return VKey::NUMPAD_ADD;
    case VK_SEPARATOR:
        return VKey::NUMPAD_SEPARATOR;
    case VK_SUBTRACT:
        return VKey::NUMPAD_SUBTRACT;
    case VK_DECIMAL:
        return VKey::NUMPAD_DECIMAL;
    case VK_DIVIDE:
        return VKey::NUMPAD_DIVIDE;

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
    case VK_F13:
        return VKey::F13;
    case VK_F14:
        return VKey::F14;
    case VK_F15:
        return VKey::F15;
    case VK_F16:
        return VKey::F16;
    case VK_F17:
        return VKey::F17;
    case VK_F18:
        return VKey::F18;
    case VK_F19:
        return VKey::F19;
    case VK_F20:
        return VKey::F20;
    case VK_F21:
        return VKey::F21;
    case VK_F22:
        return VKey::F22;
    case VK_F23:
        return VKey::F23;
    case VK_F24:
        return VKey::F24;

    case VK_NAVIGATION_VIEW:
        return VKey::NAVIGATION_VIEW;
    case VK_NAVIGATION_MENU:
        return VKey::NAVIGATION_MENU;
    case VK_NAVIGATION_UP:
        return VKey::NAVIGATION_UP;
    case VK_NAVIGATION_DOWN:
        return VKey::NAVIGATION_DOWN;
    case VK_NAVIGATION_LEFT:
        return VKey::NAVIGATION_LEFT;
    case VK_NAVIGATION_RIGHT:
        return VKey::NAVIGATION_RIGHT;
    case VK_NAVIGATION_ACCEPT:
        return VKey::NAVIGATION_ACCEPT;
    case VK_NAVIGATION_CANCEL:
        return VKey::NAVIGATION_CANCEL;

    case VK_NUMLOCK:
        return VKey::NUMLOCK;
    case VK_SCROLL:
        return VKey::SCROLL_LOCK;

    case VK_OEM_FJ_JISHO:
        return VKey::OEM_FJ_JISHO;
    case VK_OEM_FJ_MASSHOU:
        return VKey::OEM_FJ_MASSHOU;
    case VK_OEM_FJ_TOUROKU:
        return VKey::OEM_FJ_TOUROKU;
    case VK_OEM_FJ_LOYA:
        return VKey::OEM_FJ_LOYA;
    case VK_OEM_FJ_ROYA:
        return VKey::OEM_FJ_ROYA;

    case VK_LSHIFT:
        return VKey::LSHIFT;
    case VK_RSHIFT:
        return VKey::RSHIFT;
    case VK_LCONTROL:
        return VKey::LCTRL;
    case VK_RCONTROL:
        return VKey::RCTRL;
    case VK_LMENU:
        return VKey::LALT;
    case VK_RMENU:
        return VKey::RALT;

    case VK_BROWSER_BACK:
        return VKey::BROWSER_BACK;
    case VK_BROWSER_FORWARD:
        return VKey::BROWSER_FORWARD;
    case VK_BROWSER_REFRESH:
        return VKey::BROWSER_REFRESH;
    case VK_BROWSER_STOP:
        return VKey::BROWSER_STOP;
    case VK_BROWSER_SEARCH:
        return VKey::BROWSER_SEARCH;
    case VK_BROWSER_FAVORITES:
        return VKey::BROWSER_FAVORITES;
    case VK_BROWSER_HOME:
        return VKey::BROWSER_HOME;

    case VK_VOLUME_MUTE:
        return VKey::VOLUME_MUTE;
    case VK_VOLUME_DOWN:
        return VKey::VOLUME_DOWN;
    case VK_VOLUME_UP:
        return VKey::VOLUME_UP;
    case VK_MEDIA_NEXT_TRACK:
        return VKey::MEDIA_NEXT_TRACK;
    case VK_MEDIA_PREV_TRACK:
        return VKey::MEDIA_PREV_TRACK;
    case VK_MEDIA_STOP:
        return VKey::MEDIA_STOP;
    case VK_MEDIA_PLAY_PAUSE:
        return VKey::MEDIA_PLAY_PAUSE;
    case VK_LAUNCH_MAIL:
        return VKey::LAUNCH_MAIL;
    case VK_LAUNCH_MEDIA_SELECT:
        return VKey::LAUNCH_MEDIA_SELECT;
    case VK_LAUNCH_APP1:
        return VKey::LAUNCH_APP1;
    case VK_LAUNCH_APP2:
        return VKey::LAUNCH_APP2;

    case VK_OEM_1:
        return VKey::SEMICOLON;
    case VK_OEM_PLUS:
        return VKey::PLUS;
    case VK_OEM_COMMA:
        return VKey::COMMA;
    case VK_OEM_MINUS:
        return VKey::MINUS;
    case VK_OEM_PERIOD:
        return VKey::PERIOD;
    case VK_OEM_2:
        return VKey::FORWARD_SLASH;
    case VK_OEM_3:
        return VKey::TILDE;

    case VK_GAMEPAD_A:
        return VKey::GAMEPAD_A;
    case VK_GAMEPAD_B:
        return VKey::GAMEPAD_B;
    case VK_GAMEPAD_X:
        return VKey::GAMEPAD_X;
    case VK_GAMEPAD_Y:
        return VKey::GAMEPAD_Y;
    case VK_GAMEPAD_RIGHT_SHOULDER:
        return VKey::GAMEPAD_RIGHT_SHOULDER;
    case VK_GAMEPAD_LEFT_SHOULDER:
        return VKey::GAMEPAD_LEFT_SHOULDER;
    case VK_GAMEPAD_LEFT_TRIGGER:
        return VKey::GAMEPAD_LEFT_TRIGGER;
    case VK_GAMEPAD_RIGHT_TRIGGER:
        return VKey::GAMEPAD_RIGHT_TRIGGER;
    case VK_GAMEPAD_DPAD_UP:
        return VKey::GAMEPAD_DPAD_UP;
    case VK_GAMEPAD_DPAD_DOWN:
        return VKey::GAMEPAD_DPAD_DOWN;
    case VK_GAMEPAD_DPAD_LEFT:
        return VKey::GAMEPAD_DPAD_LEFT;
    case VK_GAMEPAD_DPAD_RIGHT:
        return VKey::GAMEPAD_DPAD_RIGHT;
    case VK_GAMEPAD_MENU:
        return VKey::GAMEPAD_MENU;
    case VK_GAMEPAD_VIEW:
        return VKey::GAMEPAD_VIEW;
    case VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON:
        return VKey::GAMEPAD_LEFT_THUMBSTICK_BUTTON;
    case VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON:
        return VKey::GAMEPAD_RIGHT_THUMBSTICK_BUTTON;
    case VK_GAMEPAD_LEFT_THUMBSTICK_UP:
        return VKey::GAMEPAD_LEFT_THUMBSTICK_UP;
    case VK_GAMEPAD_LEFT_THUMBSTICK_DOWN:
        return VKey::GAMEPAD_LEFT_THUMBSTICK_DOWN;
    case VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT:
        return VKey::GAMEPAD_LEFT_THUMBSTICK_RIGHT;
    case VK_GAMEPAD_LEFT_THUMBSTICK_LEFT:
        return VKey::GAMEPAD_LEFT_THUMBSTICK_LEFT;
    case VK_GAMEPAD_RIGHT_THUMBSTICK_UP:
        return VKey::GAMEPAD_RIGHT_THUMBSTICK_UP;
    case VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN:
        return VKey::GAMEPAD_RIGHT_THUMBSTICK_DOWN;
    case VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT:
        return VKey::GAMEPAD_RIGHT_THUMBSTICK_RIGHT;
    case VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT:
        return VKey::GAMEPAD_RIGHT_THUMBSTICK_LEFT;

    case VK_OEM_4:
        return VKey::LEFT_BRACKET;
    case VK_OEM_5:
        return VKey::BACKWARD_SLASH;
    case VK_OEM_6:
        return VKey::RIGHT_BRACKET;
    case VK_OEM_7:
        return VKey::QUOTE;
    case VK_OEM_8:
        return VKey::OEM_8;
    case VK_OEM_AX:
        return VKey::OEM_AX;
    case VK_OEM_102:
        return VKey::OEM_102;
    case VK_ICO_HELP:
        return VKey::ICO_HELP;
    case VK_ICO_00:
        return VKey::ICO_00;
    case VK_PROCESSKEY:
        return VKey::PROCESSKEY;
    case VK_ICO_CLEAR:
        return VKey::ICO_CLEAR;
    case VK_PACKET:
        return VKey::PACKET;

    case VK_OEM_RESET:
        return VKey::OEM_RESET;
    case VK_OEM_JUMP:
        return VKey::OEM_JUMP;
    case VK_OEM_PA1:
        return VKey::OEM_PA1;
    case VK_OEM_PA2:
        return VKey::OEM_PA2;
    case VK_OEM_PA3:
        return VKey::OEM_PA3;
    case VK_OEM_WSCTRL:
        return VKey::OEM_WSCTRL;
    case VK_OEM_CUSEL:
        return VKey::OEM_CUSEL;
    case VK_OEM_ATTN:
        return VKey::OEM_ATTN;
    case VK_OEM_FINISH:
        return VKey::OEM_FINISH;
    case VK_OEM_COPY:
        return VKey::OEM_COPY;
    case VK_OEM_AUTO:
        return VKey::OEM_AUTO;
    case VK_OEM_ENLW:
        return VKey::OEM_ENLW;
    case VK_OEM_BACKTAB:
        return VKey::OEM_BACKTAB;
    case VK_ATTN:
        return VKey::ATTN;
    case VK_CRSEL:
        return VKey::CRSEL;
    case VK_EXSEL:
        return VKey::EXSEL;
    case VK_EREOF:
        return VKey::EREOF;
    case VK_PLAY:
        return VKey::PLAY;
    case VK_ZOOM:
        return VKey::ZOOM;
    case VK_NONAME:
        return VKey::NONAME;
    case VK_PA1:
        return VKey::PA1;
    case VK_OEM_CLEAR:
        return VKey::OEM_CLEAR;

    default:
        return VKey::NONE;
    }
}
