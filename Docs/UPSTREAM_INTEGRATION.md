# Upstream integration notes

The controller protocol library is tracked as a git submodule at:

`ThirdParty/Dualsense-Multiplatform`

Upstream: https://github.com/DrapNard/Dualsense-Multiplatform
Branch followed for dependency updates: `main`

The parent repository always pins an exact upstream commit. The `branch = main`
entry in `.gitmodules` is used only by update tooling such as `git submodule
update --remote` and Dependabot. Normal builds never float to a new commit.

## No local patches inside the submodule

Do not edit files inside `ThirdParty/Dualsense-Multiplatform` as part of a
plugin change. Fix reusable protocol/library behavior upstream first, then bump
the submodule pointer in this repository.

Integration-specific workarounds belong outside the submodule.

## Duplicate HID handle safeguard

At the upstream commit currently pinned by this repository,
`TBasicDeviceRegistry::PlugAndPlay` can call `CreateHandle` again for a path that
already owns a live gamepad instance during a later detection pass.

The plugin intentionally does **not** patch the upstream header. Instead,
`DualSensePlatformHardware.cpp` wraps the native OS backend and tracks active
device paths. Duplicate `CreateHandle` requests are rejected until the original
context is invalidated. This keeps the submodule byte-for-byte upstream while
preventing duplicate OS handles in the Unreal integration.

If upstream adopts an equivalent fix later, remove this wrapper safeguard after
updating the submodule and extending the native regression test.
