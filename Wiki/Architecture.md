# Architecture

## Modules

### DualSenseCore

Responsibilities:

- owns the standard-C++ controller manager;
- owns Windows/Linux/macOS hardware backends;
- initializes the upstream `IPlatformHardware` instance;
- compiles the pinned Dualsense-Multiplatform submodule into the core module;
- translates GamepadCore interfaces into a stable public native API.

It does **not** expose UObjects or Blueprint metadata.

### DualSenseRuntime

Responsibilities:

- `UENUM` / `USTRUCT` Blueprint types;
- `UDualSenseSubsystem` lifecycle and ticking;
- Blueprint events;
- conversion between Unreal types and native core types.

It does **not** parse HID packets or know OS APIs.

## Platform backends

`DualSenseCore/Private/Platform` contains one implementation per operating system:

- Windows: SetupAPI enumeration and Windows HID read/write.
- Linux: `/sys/class/hidraw` discovery plus `/dev/hidraw*` read/write.
- macOS: IOHIDManager discovery and IOHIDDevice reports.

All implement the same upstream `IPlatformHardware` contract. A small shared wrapper tracks active device paths so integration-specific handle-lifetime safeguards stay outside the upstream repository.

## Git submodule dependency

`ThirdParty/Dualsense-Multiplatform` is a normal git submodule. The plugin repository records an exact upstream commit, so every commit of this repository is reproducible. `.gitmodules` also declares `branch = main`, which allows update tools to discover the newest upstream commit without making builds float automatically.

The upstream source lives outside `Source/` to prevent UnrealBuildTool from accidentally compiling upstream tests or nested development submodules. Seven tiny compile-bridge translation units under `DualSenseCore/Private/ThirdParty` include the required upstream `.cpp` files into the `DualSenseCore` module. This is important on Windows because the upstream library is intentionally not treated as a separate Unreal DLL.

Do not patch the submodule locally. Reusable fixes belong upstream; Unreal-specific behavior belongs in this repository. See `Docs/UPSTREAM_INTEGRATION.md`.

## Adding another controller

Add reusable protocol support upstream or in a clearly isolated core extension, then update the device mapping and capabilities. Do not add device-specific branching to Blueprint code.
