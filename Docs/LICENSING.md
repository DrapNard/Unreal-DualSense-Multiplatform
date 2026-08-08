# Licensing model

## Integration code: MPL-2.0

Files authored for this Unreal integration are licensed under Mozilla Public License 2.0 unless another notice says otherwise.

MPL is file-level copyleft. In practical terms, distributing a modified MPL-covered source file requires making that modified file available under MPL-2.0. It does not automatically relicense separate game modules or assets that merely use the plugin API.

This makes the project meaningfully copyleft while still allowing proprietary Unreal games to use it.

## Dualsense-Multiplatform submodule: MIT

`ThirdParty/Dualsense-Multiplatform` is a git submodule pointing to the upstream Dualsense-Multiplatform / GamepadCore repository. Its source remains under the upstream MIT license.

The parent repository pins an exact upstream commit for reproducible builds. `LICENSES/MIT-GamepadCore.txt` is retained as a convenience copy of the upstream license, and an initialized submodule also contains its own `LICENSE` file.

## Engine and platform SDKs

Unreal Engine, Windows SDK components, Apple frameworks, and operating-system headers are not redistributed under this repository's license. Users are responsible for complying with the licenses of their engine and SDK installations.

This document is an engineering overview, not legal advice.
