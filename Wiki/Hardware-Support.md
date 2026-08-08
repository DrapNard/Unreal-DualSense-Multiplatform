# Hardware Support

## Known Sony USB product IDs

| Device | VID | PID |
| --- | --- | --- |
| DualSense | `054C` | `0CE6` |
| DualSense Edge | `054C` | `0DF2` |
| DualShock 4 v1 | `054C` | `05C4` |
| DualShock 4 v2 | `054C` | `09CC` |

The platform backends filter HID gamepad usages and these IDs.

## USB vs Bluetooth

GamepadCore uses different report layouts and CRC handling depending on transport. The platform layer detects transport and leaves report composition/parsing to the pinned upstream submodule.

## Feature notes

DualShock 4 does not expose DualSense adaptive triggers, microphone LED, or player LED functionality. Query `GetCapabilities` instead of assuming a feature exists.

Audio-driven haptics are DualSense-specific and transport behavior can vary with OS controller stacks.
