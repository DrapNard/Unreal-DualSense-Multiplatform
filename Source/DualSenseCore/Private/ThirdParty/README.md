# GamepadCore compile bridges

`Dualsense-Multiplatform` lives at the plugin root as a git submodule instead of
inside an Unreal module. UnrealBuildTool does not compile arbitrary `.cpp` files
from that external directory, so each file here is a tiny translation unit that
includes one upstream implementation file.

Keeping one bridge per upstream `.cpp` preserves separate translation units and
keeps the upstream symbols inside `DualSenseCore` rather than a second Unreal
DLL. `Scripts/validate_repo.py` verifies that every upstream implementation
`.cpp` has exactly this integration coverage after a submodule update.
