// SPDX-License-Identifier: MPL-2.0
#include "DualSenseCoreModule.h"
#include "DualSenseCoreManager.h"
#include "Modules/ModuleManager.h"

class FDualSenseCoreModule final : public IDualSenseCoreModule
{
public:
    void StartupModule() override { ManagerInstance.Start(); }
    void ShutdownModule() override { ManagerInstance.Stop(); }
    DualSense::Manager& GetManager() override { return ManagerInstance; }
private:
    DualSense::Manager ManagerInstance;
};

IDualSenseCoreModule& IDualSenseCoreModule::Get()
{
    return FModuleManager::LoadModuleChecked<IDualSenseCoreModule>(TEXT("DualSenseCore"));
}

bool IDualSenseCoreModule::IsAvailable()
{
    return FModuleManager::Get().IsModuleLoaded(TEXT("DualSenseCore"));
}

IMPLEMENT_MODULE(FDualSenseCoreModule, DualSenseCore)
