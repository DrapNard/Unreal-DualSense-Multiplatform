# Getting Started

## Requirements

- Unreal Engine 5.0–5.8
- A C++ toolchain supported by your Unreal version
- Windows, Linux, or macOS
- DualSense, DualSense Edge, or DualShock 4

## Install as a project plugin

Place the repository in `YourProject/Plugins/DualSenseMultiplatform` and build the project. The `.uplugin` descriptor contains both runtime modules.

## First Blueprint

1. In Blueprint, get the Game Instance Subsystem named **DualSense Subsystem**.
2. Bind `On Device Connected`.
3. Store the `Device Id` from `FDualSenseDeviceInfo`.
4. Call `Get State` to read analog, buttons, touch, battery, and enabled motion fields.
5. Call output functions using the same Device Id. Try `Set Lightbar` with `Transition Duration = 0.25` for a visible smooth transition.
6. For adaptive triggers, start with `Set Trigger Preset (Simple)` before using the raw advanced nodes.
7. If needed, apply an accessibility preset before gameplay output so sensory limits are enforced automatically.

The subsystem requests an immediate device scan on initialization and continues hot-plug scans at the upstream registry interval.

## Output batching

Most output functions have `Apply Immediately` enabled by default. For multiple non-animated changes in one frame, disable it on each setter and call `Apply Output` once at the end. Smooth light transitions are tick-driven and flush their animation frames automatically.

## Sensors and touch

Motion, touch, and gesture processing are opt-in to avoid unnecessary work. Enable them before reading those fields.

## Linux

Install the supplied udev rule before expecting write access to hidraw devices. See [[Linux-Permissions]].
