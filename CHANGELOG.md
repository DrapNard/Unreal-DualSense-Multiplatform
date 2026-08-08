# Changelog

All notable changes to this project are documented here.

The project follows [Semantic Versioning](https://semver.org/) and uses Conventional Commits to prepare automated releases.

## [Unreleased]

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
