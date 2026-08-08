// SPDX-License-Identifier: MPL-2.0
#pragma once

#include <memory>
class IPlatformHardware;

namespace DualSense::Platform
{
    std::unique_ptr<IPlatformHardware> CreateHardware();
}
