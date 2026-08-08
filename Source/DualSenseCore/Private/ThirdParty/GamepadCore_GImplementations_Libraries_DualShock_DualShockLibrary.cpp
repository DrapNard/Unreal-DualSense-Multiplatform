// SPDX-License-Identifier: MPL-2.0
// Compile bridge for the Dualsense-Multiplatform git submodule.
// Keep upstream implementation files outside the Unreal module while compiling
// them into DualSenseCore so their symbols stay in the same Unreal module/DLL.
#include "../../../../ThirdParty/Dualsense-Multiplatform/Source/Private/GImplementations/Libraries/DualShock/DualShockLibrary.cpp"
