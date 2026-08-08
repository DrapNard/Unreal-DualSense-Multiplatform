# DualSense Multiplatform Wiki

DualSense Multiplatform is an Unreal Engine 5 plugin that exposes advanced Sony controller features to Blueprint and C++ while keeping the underlying HID implementation modular.

## Start here

- [Getting Started](Getting-Started)
- [Blueprint API](Blueprint-API)
- [C++ API](Cpp-API)
- [Hardware Support](Hardware-Support)
- [Linux Permissions](Linux-Permissions)
- [Architecture](Architecture)
- [CI and Releases](CI-and-Releases)
- [Troubleshooting](Troubleshooting)

## Design goals

The project aims to be readable by people who are new to Unreal plugin development. Unreal-facing code, controller protocol code, and operating-system HID code live in separate areas so contributors can understand one layer at a time.
