// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <memory>
class IPlatformHardware;

namespace DualSense::Platform
{
    // Returns the raw OS backend implemented by exactly one platform source file.
    std::unique_ptr<IPlatformHardware> CreateNativeHardware();

    // Returns the backend used by GamepadCore. The wrapper keeps integration-only
    // lifecycle safeguards outside the upstream git submodule.
    std::unique_ptr<IPlatformHardware> CreateHardware();
}
