// SPDX-License-Identifier: MPL-2.0
#include "DualSenseCoreManager.h"
#include "GCore/Types/Structs/Context/DeviceContext.h"
#include "GImplementations/Utils/GamepadTrigger.h"

#include <array>
#include <cassert>
#include <iostream>

int main()
{
    FDeviceContext Context;
    FDualSenseTriggerComposer::Resistance(&Context, 3, 7, EDSGamepadHand::Right);
    assert(Context.Output.RightTrigger.Mode == 0x01);
    assert(Context.Output.RightTrigger.Strengths.Compose[0] == 3);
    assert(Context.Output.RightTrigger.Strengths.Compose[1] == 7);

    FDualSenseTriggerComposer::GameCube(&Context, EDSGamepadHand::Left);
    assert(Context.Output.LeftTrigger.Mode == 0x02);

    const std::vector<std::uint8_t> Custom = {0x25, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    FDualSenseTriggerComposer::CustomTrigger(&Context, EDSGamepadHand::Right, Custom);
    assert(Context.Output.RightTrigger.Mode == 0xFF);

    DualSense::Manager Manager;
    assert(Manager.Start());
    Manager.RequestImmediateDetection();
    Manager.Tick(0.0f);
    Manager.Stop();

    std::cout << "DualSense native smoke tests passed\n";
    return 0;
}
