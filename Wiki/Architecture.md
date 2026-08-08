# Architecture

## Modules

### DualSenseCore

Responsibilities:

- owns the standard-C++ controller manager;
- owns Windows/Linux/macOS hardware backends;
- initializes the upstream `IPlatformHardware` instance;
- owns the vendored GamepadCore source;
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

All implement the same upstream `IPlatformHardware` contract.

## Vendored dependency

GamepadCore is vendored to keep Unreal builds reproducible and offline. The upstream project remains MIT licensed. Integration patches should be avoided when possible and documented in `Docs/VENDOR_PATCHES.md` when unavoidable.

## Adding another controller

Add protocol support upstream or in a clearly isolated core extension, then update the device mapping and capabilities. Do not add device-specific branching to Blueprint code.
