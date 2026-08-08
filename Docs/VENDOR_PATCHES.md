# Vendored GamepadCore patches

The initial vendor snapshot is kept source-compatible with upstream. Integration-specific behavior lives outside the vendor tree whenever possible.

If an upstream defect must be patched locally, document the file, reason, upstream issue/PR, and exact semantic change here.

## Duplicate HID handle fix

`TBasicDeviceRegistry::PlugAndPlay` is patched so a device path that is already registered is not opened again during the one-second detection pass. The upstream implementation creates a fresh OS handle before checking whether the device already has a library instance, which can leak one handle per scan. The patch does not change the public API or controller protocol logic.
