# C++ API

There are two supported C++ entry points.

## Unreal-facing API

Use `UDualSenseSubsystem` when your code already has a `UGameInstance`. Every Blueprint-callable method is also directly callable from C++.

```cpp
UDualSenseSubsystem* Subsystem = GameInstance->GetSubsystem<UDualSenseSubsystem>();
FDualSenseState State;
if (Subsystem && Subsystem->GetState(DeviceId, State))
{
    // Use State.LeftStick, State.Gyroscope, State.PressedButtons, ...

    // Smooth light change: the fourth argument is TransitionDuration.
    Subsystem->SetLightbar(DeviceId, FLinearColor(0.1f, 0.3f, 1.0f), true, 0.25f);

    // Gameplay-friendly adaptive trigger with accessibility-aware intensity.
    Subsystem->SetTriggerPreset(DeviceId, EDualSenseTriggerPreset::MediumResistance);
}
```

## Native manager API

Lower-level modules can access the standard-C++ manager:

```cpp
DualSense::Manager& Manager = IDualSenseCoreModule::Get().GetManager();
Manager.RequestImmediateDetection();
```

The manager API uses `std::vector`, `std::array`, `std::string`, and POD-like structs. It deliberately hides all upstream GamepadCore implementation types.

## Threading

The current runtime subsystem ticks the manager from the game thread. The upstream context protects its input/output buffers with mutexes, but gameplay code should treat the public manager as a game-thread service unless you add explicit synchronization around your own cross-thread calls.
