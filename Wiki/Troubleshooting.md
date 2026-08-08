# Troubleshooting

## Controller is visible to the OS but not the plugin

- Confirm the device is a supported Sony VID/PID.
- Call `RequestImmediateDetection`.
- On Linux, verify hidraw permissions.
- Try USB first to separate Bluetooth-stack problems from protocol problems.

## Inputs work but outputs do not

Direct HID write permissions can differ from read permissions. This is especially common on Linux. Check the udev rule and reconnect the device.

## Adaptive triggers return false

Call `GetCapabilities`. DualShock 4 does not implement adaptive triggers.

## Motion values stay zero

Call `EnableMotion(DeviceId, true)` before reading motion fields.

## Touch values stay unchanged

Call `EnableTouch` or `EnableGesture` before expecting touch processing.

## Unreal CI is queued forever

The full Unreal workflow requires matching self-hosted runner labels. If no runner matches all requested labels, GitHub queues the job until one becomes available or the workflow times out.
