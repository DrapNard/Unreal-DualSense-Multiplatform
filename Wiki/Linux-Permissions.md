# Linux Permissions

The Linux backend uses `/dev/hidraw*` directly. A normal desktop user often cannot open a hidraw node for writing without a udev rule.

Install the rule included with the plugin:

```bash
sudo cp Config/99-dualsense-unreal.rules /etc/udev/rules.d/99-dualsense-unreal.rules
sudo udevadm control --reload-rules
sudo udevadm trigger
```

Disconnect and reconnect the controller afterward.

## Verify access

```bash
ls -l /dev/hidraw*
udevadm info /dev/hidraw0
```

The rule uses `TAG+="uaccess"`, allowing the active local session to receive access through the distribution's udev/logind policy instead of granting world-write permissions.

## Distribution support

The backend does not depend on hidapi, SDL, or a distribution-specific userspace package. It requires Linux hidraw support, standard Linux HID headers at build time, and an Unreal-compatible toolchain.
