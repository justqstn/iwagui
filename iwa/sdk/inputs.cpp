#include "sdk/inputs.hpp"
#include <vector>

using namespace Iwa;
using namespace Iwa::SDK;

Iwa::Signal<VKey> SDK::onKeyClickedSignal = {};
Iwa::Signal<VKey> SDK::onKeyPressedSignal = {};
Iwa::Signal<VKey> SDK::onKeyReleasedSignal = {};
