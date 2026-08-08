# Blueprint API

The Blueprint surface is provided by `UDualSenseSubsystem`, a Game Instance Subsystem.

## Events

- `OnDeviceConnected(Device)`
- `OnDeviceDisconnected(DeviceId)`
- `OnButtonPressed(DeviceId, Button)`
- `OnButtonReleased(DeviceId, Button)`

## Device and state functions

- `RequestImmediateDetection`
- `GetConnectedDeviceIds`
- `IsDeviceConnected`
- `GetDeviceInfo`
- `GetState`
- `GetCapabilities`
- `IsButtonDown`

`FDualSenseState` contains sticks, analog triggers, gyroscope, accelerometer, gravity, tilt, touch information, pressed buttons, battery level, and phone/headset-related status exposed by the upstream parser.

## Output functions

- `ApplyOutput`
- `SetVibration`
- `SetLightbar`
- `SetLightbarFlash`
- `SetPlayerLed`
- `SetMicrophoneLed`
- `ResetLights`

## Adaptive trigger functions

- `StopTrigger`
- `SetGameCubeTrigger`
- `SetResistanceTrigger`
- `SetBowTrigger`
- `SetGallopingTrigger`
- `SetWeaponTrigger`
- `SetMachineGunTrigger`
- `SetMachineTrigger`
- `SetCustomTrigger`

`SetCustomTrigger` requires exactly 10 bytes. Invalid lengths are rejected before entering the upstream composer.

## Motion and touch

- `EnableMotion`
- `ResetGyroOrientation`
- `EnableTouch`
- `EnableGesture`

## DualSense settings and audio haptics

- `ConfigureDualSense`
- `SendAudioHapticsBytes`
- `SendAudioHapticsFloats`
- `SendAudioAndHapticsBytes`

Audio haptics are a low-level feature. The caller is responsible for supplying data in the format expected by the upstream library.
