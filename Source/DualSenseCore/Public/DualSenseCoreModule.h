// SPDX-License-Identifier: MPL-2.0
#pragma once

#include "Modules/ModuleManager.h"

namespace DualSense { class Manager; }

class DUALSENSECORE_API IDualSenseCoreModule : public IModuleInterface
{
public:
    static IDualSenseCoreModule& Get();
    static bool IsAvailable();
    virtual DualSense::Manager& GetManager() = 0;
};
