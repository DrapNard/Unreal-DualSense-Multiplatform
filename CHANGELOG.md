# Changelog

All notable changes to this project are documented here.

The project follows [Semantic Versioning](https://semver.org/) and uses Conventional Commits to prepare automated releases.

## [1.1.0](https://github.com/DrapNard/Unreal-DualSense-Multiplatform/compare/Unreal-DualSense-Multiplatform-v1.0.0...Unreal-DualSense-Multiplatform-v1.1.0) (2026-08-14)


### Features

* **accessibility:** add smooth lighting and sensory controls ([2da9104](https://github.com/DrapNard/Unreal-DualSense-Multiplatform/commit/2da910455cfb1860f16886875dcfcbbffac31e0b))
* **blueprint:** expose controller features through game instance subsystem ([0088727](https://github.com/DrapNard/Unreal-DualSense-Multiplatform/commit/008872734387f71a7fe38f6afc2a8dee39bc836e))
* **core:** add cross-platform HID manager and native tests ([0c989c1](https://github.com/DrapNard/Unreal-DualSense-Multiplatform/commit/0c989c1d48be8a8a07e41465cbdf9854869413ba))
* **deps:** track Dualsense-Multiplatform as submodule ([b35b91e](https://github.com/DrapNard/Unreal-DualSense-Multiplatform/commit/b35b91e8783a4bf86690e9650387828fde73238d))


### Bug Fixes

* **core:** preserve complete input state and HID lifetime ([f44cf84](https://github.com/DrapNard/Unreal-DualSense-Multiplatform/commit/f44cf84cc434885ba81671dc34bc9979980d5423))

## [Unreleased]

### Added
- Accessibility profiles and per-controller sensory output limits for lights, flashing, rumble, adaptive triggers, and audio haptics.
- Smooth-step lightbar/player-LED transitions through a `Transition Duration` parameter directly on the existing light nodes.
- `Set Trigger Preset (Simple)` with gameplay-friendly adaptive-trigger presets and intensity scaling.
- In-editor Blueprint tooltips for every public subsystem node and usable defaults for common/advanced trigger nodes.

### Changed
- Replaced the copied GamepadCore vendor snapshot with a pinned `Dualsense-Multiplatform` git submodule.
- Added automatic Dependabot submodule update PRs and self-contained release packaging.
- Moved the duplicate HID handle safeguard into the plugin platform wrapper so the upstream submodule stays unmodified.

## [1.0.0] - 2026-08-08

### Added
- Unreal Engine 5.0 through 5.8 plugin scaffold.
- Blueprint and C++ controller subsystem.
- Windows, Linux, and macOS HID transport backends.
- DualSense, DualSense Edge, and DualShock 4 device support inherited from GamepadCore.
- Adaptive trigger, rumble, LED, touch, sensor, settings, and audio-haptics APIs.
- Native cross-platform smoke tests and Unreal self-hosted build matrix.
- Wiki source, community health files, and release automation.
