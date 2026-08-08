# Blueprint API

The Blueprint surface is provided by `UDualSenseSubsystem`, a Game Instance Subsystem. Every callable node has an in-editor tooltip describing its behavior and important parameters.

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

### Smooth light transitions

Smooth lighting is a parameter of the normal light nodes rather than a separate node. `SetLightbar`, `SetLightbarFlash`, `SetPlayerLed`, and `ResetLights` expose `Transition Duration` in seconds:

- `0.0`: change immediately;
- `0.25`: short smooth transition;
- `1.0`: one-second smooth transition.

Transitions use a smooth-step easing curve and are updated by the subsystem tick. Animated transitions flush their own frames even when the regular output batching API exists, because otherwise the fade could not progress.

For `SetLightbarFlash`, the subsystem first fades to the requested color and then starts the hardware flash effect. `SetPlayerLed` fades brightness; changing the player-LED pattern itself is discrete because the hardware exposes it as a mask rather than independent RGB LEDs. If flashing is disabled by accessibility settings, the same node produces a static color instead.

## Accessibility

- `SetAccessibilitySettings`
- `GetAccessibilitySettings`
- `ApplyAccessibilityPreset`
- `ResetAccessibilitySettings`

`FDualSenseAccessibilitySettings` is stored per connected controller and is automatically applied to later output requests. It can:

- limit RGB/player-LED brightness;
- replace flashing effects with static light;
- enforce a minimum smooth light transition duration;
- scale or disable normal rumble;
- scale or disable configurable adaptive-trigger effects;
- disable audio haptics.

Ready-made presets are `Default`, `Reduced Haptics`, `Reduced Flashing`, `Low Sensory`, and `No Haptics`. Hard-disable options stop active rumble/trigger effects immediately when the profile is applied.

## Adaptive trigger functions

For normal gameplay, prefer `Set Trigger Preset (Simple)`. It provides:

- Off
- Soft Resistance
- Medium Resistance
- Strong Resistance
- GameCube Style
- Bow
- Weapon
- Automatic

The node defaults to `Medium Resistance`, both triggers, and intensity `1.0`. Intensity is additionally limited by the active accessibility profile.

Advanced nodes remain available for exact upstream control and now contain usable default values:

- `StopTrigger`
- `SetGameCubeTrigger`
- `SetResistanceTrigger`
- `SetBowTrigger`
- `SetGallopingTrigger`
- `SetWeaponTrigger`
- `SetMachineGunTrigger`
- `SetMachineTrigger`
- `SetCustomTrigger`

`SetCustomTrigger` requires exactly 10 bytes. Invalid lengths are rejected before entering the upstream composer. Custom raw payload bytes are not rewritten by the accessibility scaler; the global adaptive-trigger disable option is still honored.

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

Audio haptics are a low-level feature. The caller is responsible for supplying data in the format expected by the upstream library. Accessibility settings can block these calls for players who disable audio haptics.
