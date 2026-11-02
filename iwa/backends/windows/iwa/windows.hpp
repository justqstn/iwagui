#pragma once

#include "utils/signal.hpp"

#include "windows.h"

namespace Iwa
{
    bool WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    // Called for every message passed to WndProcHandler.
    extern Signal<HWND, UINT, WPARAM, LPARAM> onWndProcSignal;
}
