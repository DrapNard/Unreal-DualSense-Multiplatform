# Upstream API coverage

This file documents how the public GamepadCore controller features are surfaced by the Unreal integration.

| GamepadCore capability | Native C++ | Blueprint |
| --- | --- | --- |
| Device discovery / hot plug | `Manager::RequestImmediateDetection`, device queries | Device queries + connection events |
| Connection, model, transport | `IsConnected`, `GetDeviceInfo` | `IsDeviceConnected`, `GetDeviceInfo` |
| Battery | `State::BatteryLevel` | `FDualSenseState::BatteryLevel` |
| Sticks / triggers | `State` | `FDualSenseState` |
| Digital buttons | `State::Buttons` | `PressedButtons` + press/release events |
| Analog-direction threshold flags | `State::Buttons` | `PressedButtons` + press/release events |
| L2/R2 threshold flags | `State::Buttons` | `PressedButtons` + press/release events |
| DualSense Edge Fn / paddles | `State::Buttons` | `PressedButtons` + press/release events |
| Touch / gesture state | `State` + enable calls | `FDualSenseState` + enable nodes |
| Motion sensor state | `State` + enable/reset calls | `FDualSenseState` + enable/reset nodes |
| Standard rumble | `SetVibration` | `Set Vibration` |
| RGB lightbar / flash / reset | lightbar methods | lightbar nodes + per-call smooth transition duration |
| Player LEDs | `SetPlayerLed` | `Set Player Led` |
| Microphone LED | `SetMicrophoneLed` | `Set Microphone Led` |
| Adaptive trigger reset | `StopTrigger` | `Stop Trigger` |
| GameCube trigger mode | `SetGameCubeTrigger` | `Set Game Cube Trigger` |
| Resistance mode | `SetResistanceTrigger` | `Set Resistance Trigger` |
| Bow 0x22 mode | `SetBowTrigger` | `Set Bow Trigger` |
| Galloping 0x23 mode | `SetGallopingTrigger` | `Set Galloping Trigger` |
| Weapon 0x25 mode | `SetWeaponTrigger` | `Set Weapon Trigger` |
| Machine gun 0x26 mode | `SetMachineGunTrigger` | `Set Machine Gun Trigger` |
| Machine 0x27 mode | `SetMachineTrigger` | `Set Machine Trigger` |
| Raw 10-byte trigger effect | `SetCustomTrigger` | `Set Custom Trigger` |
| DualSense audio / reduction settings | `ConfigureDualSense` | `Configure Dual Sense` |
| Byte audio haptics | `SendAudioHaptics(bytes)` | `Send Audio Haptics Bytes` |
| Float audio haptics | `SendAudioHaptics(float)` | `Send Audio Haptics Floats` |
| Separate audio + haptic bytes | two-buffer overload | `Send Audio And Haptics Bytes` |
| Batched output flush | `ApplyOutput` | `Apply Output` |

GamepadCore's lifecycle methods (`Initialize`, `ShutdownLibrary`, `UpdateInput`, `UpdateOutput`) remain intentionally internal because the plugin manager owns device lifetime. `UpdateOutput` is represented by `ApplyOutput`; input updates happen from the Unreal subsystem tick.

## Unreal-layer usability and accessibility features

These features intentionally live above GamepadCore because they are engine/user-experience policy rather than controller protocol primitives.

| Unreal-layer feature | Blueprint / C++ API |
| --- | --- |
| Smooth RGB transitions | `TransitionDuration` on `SetLightbar`, `SetLightbarFlash`, `SetPlayerLed`, and `ResetLights` |
| Sensory/accessibility limits | `FDualSenseAccessibilitySettings` |
| Ready-made accessibility profiles | `ApplyAccessibilityPreset` |
| Simple trigger presets | `SetTriggerPreset` / `EDualSenseTriggerPreset` |
| In-editor node documentation | `ToolTip` metadata on every `UFUNCTION` in `UDualSenseSubsystem` |

Accessibility settings are per controller. They can reduce brightness, suppress flashing, enforce smooth light transitions, scale/disable rumble, scale/disable adaptive triggers, and disable audio haptics without changing gameplay Blueprint graphs.
