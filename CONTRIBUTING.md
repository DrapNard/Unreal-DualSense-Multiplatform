# Contributing

Thanks for helping improve DualSense Multiplatform for Unreal Engine.

## Ground rules

- Keep repository content, code comments, commit messages, issues, and documentation in English.
- Keep platform code out of the Blueprint layer. HID code belongs in `DualSenseCore/Private/Platform`.
- Do not expose vendored GamepadCore types from the public Unreal API.
- Prefer small, reviewable changes with tests.
- Preserve the MIT notice for vendored GamepadCore code.
- New integration code is MPL-2.0 and should carry `SPDX-License-Identifier: MPL-2.0`.

## Development flow

1. Fork the repository and branch from `main`.
2. Use a descriptive branch name such as `feature/linux-hotplug` or `fix/ds4-bluetooth-output`.
3. Use Conventional Commits (`feat:`, `fix:`, `docs:`, `test:`, `ci:`, `refactor:`, `chore:`).
4. Run native tests on your platform.
5. If you have an Unreal source/binary installation, run `BuildPlugin` for the Unreal versions you changed.
6. Open a pull request and complete the checklist.

## Native tests

```bash
cmake -S Tests/Native -B build/native -DCMAKE_BUILD_TYPE=Release
cmake --build build/native --config Release
ctest --test-dir build/native -C Release --output-on-failure
```

These tests do not require a controller. Hardware validation is documented in the Wiki.

## Unreal build test

```bash
/path/to/UnrealEngine/Engine/Build/BatchFiles/RunUAT.sh BuildPlugin \
  -Plugin="$PWD/DualSenseMultiplatform.uplugin" \
  -Package="$PWD/build/package" \
  -TargetPlatforms=Linux \
  -Rocket
```

Use `RunUAT.bat` on Windows.

## Pull requests

A PR should explain what changed, why it changed, affected controller models/transports, tested OS/UE versions, and any hardware used. Changes to HID report layouts should include references or captures that can be legally redistributed.

## Vendored GamepadCore updates

Do not casually edit `Source/DualSenseCore/Private/Vendor/GamepadCore`. Prefer fixing integration behavior outside the vendor tree. When an upstream patch is unavoidable, document it in `Docs/VENDOR_PATCHES.md` and link the upstream issue or PR.
