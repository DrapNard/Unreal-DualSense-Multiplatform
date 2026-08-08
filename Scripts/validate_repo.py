#!/usr/bin/env python3
"""Validate repository invariants without requiring Unreal Engine."""
from __future__ import annotations

import json
import pathlib
import re
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]
errors: list[str] = []


def require(path: str) -> pathlib.Path:
    item = ROOT / path
    if not item.exists():
        errors.append(f"Missing required path: {path}")
    return item


plugin_path = require("DualSenseMultiplatform.uplugin")
if plugin_path.exists():
    plugin = json.loads(plugin_path.read_text(encoding="utf-8"))
    if plugin.get("FileVersion") != 3:
        errors.append("DualSenseMultiplatform.uplugin must use FileVersion 3")
    if plugin.get("VersionName") != "1.0.0":
        # Release Please changes this value; only verify a valid semantic version shape below.
        parts = str(plugin.get("VersionName", "")).split(".")
        if len(parts) != 3 or not all(part.isdigit() for part in parts):
            errors.append("VersionName must be a three-part semantic version")
    modules = {module.get("Name") for module in plugin.get("Modules", [])}
    if modules != {"DualSenseCore", "DualSenseRuntime"}:
        errors.append(f"Unexpected module set: {sorted(modules)}")
    platforms = set(plugin.get("SupportedTargetPlatforms", []))
    expected = {"Win64", "Linux", "Mac"}
    if platforms != expected:
        errors.append(f"SupportedTargetPlatforms must be {sorted(expected)}")

required_paths = [
    "LICENSE",
    ".gitmodules",
    "LICENSES/MIT-GamepadCore.txt",
    "ThirdParty/Dualsense-Multiplatform/Source/Public/GCore/Interfaces/IPlatformHardware.h",
    "THIRD_PARTY_NOTICES.md",
    "README.md",
    "CONTRIBUTING.md",
    "CODE_OF_CONDUCT.md",
    "SECURITY.md",
    ".github/PULL_REQUEST_TEMPLATE.md",
    ".github/ISSUE_TEMPLATE/bug_report.yml",
    ".github/ISSUE_TEMPLATE/feature_request.yml",
    ".github/workflows/native-ci.yml",
    ".github/workflows/unreal-matrix.yml",
    ".github/workflows/release-please.yml",
    "Wiki/Home.md",
    "Wiki/Blueprint-API.md",
    "Wiki/Cpp-API.md",
    "Wiki/Architecture.md",
]
for required in required_paths:
    require(required)

gamepad_private = ROOT / "ThirdParty/Dualsense-Multiplatform/Source/Private"
bridge_root = ROOT / "Source/DualSenseCore/Private/ThirdParty"
if gamepad_private.exists() and bridge_root.exists():
    upstream_sources = {
        path.relative_to(gamepad_private).as_posix()
        for path in gamepad_private.rglob("*.cpp")
    }
    bridged_sources: set[str] = set()
    include_prefix = '#include "../../../../ThirdParty/Dualsense-Multiplatform/Source/Private/'
    for bridge in bridge_root.glob("GamepadCore_*.cpp"):
        for line in bridge.read_text(encoding="utf-8").splitlines():
            if line.startswith(include_prefix) and line.endswith('"'):
                bridged_sources.add(line[len(include_prefix):-1])
    missing_bridges = sorted(upstream_sources - bridged_sources)
    stale_bridges = sorted(bridged_sources - upstream_sources)
    if missing_bridges:
        errors.append(f"Missing GamepadCore compile bridges: {missing_bridges}")
    if stale_bridges:
        errors.append(f"Compile bridges reference missing upstream sources: {stale_bridges}")


# Every Blueprint node must explain itself in-editor. Keep this invariant cheap
# to check without requiring UHT or an Unreal installation.
runtime_header = ROOT / "Source/DualSenseRuntime/Public/DualSenseSubsystem.h"
if runtime_header.exists():
    header_text = runtime_header.read_text(encoding="utf-8")
    function_macros = re.findall(r"UFUNCTION\((.*?)\)\s*\n", header_text, flags=re.DOTALL)
    if not function_macros:
        errors.append("No Blueprint UFUNCTION declarations found in DualSenseSubsystem.h")
    for index, metadata in enumerate(function_macros, start=1):
        if "ToolTip=" not in metadata:
            errors.append(f"Blueprint UFUNCTION #{index} is missing ToolTip metadata")

    for function_name in ("SetLightbar", "SetLightbarFlash", "SetPlayerLed", "ResetLights"):
        match = re.search(rf"bool\s+{function_name}\((.*?)\);", header_text, flags=re.DOTALL)
        if not match or "TransitionDuration" not in match.group(1):
            errors.append(f"{function_name} must expose TransitionDuration directly on the node")

gitmodules_path = ROOT / ".gitmodules"
if gitmodules_path.exists():
    gitmodules = gitmodules_path.read_text(encoding="utf-8")
    if "path = ThirdParty/Dualsense-Multiplatform" not in gitmodules:
        errors.append(".gitmodules must register ThirdParty/Dualsense-Multiplatform")
    if "url = https://github.com/DrapNard/Dualsense-Multiplatform.git" not in gitmodules:
        errors.append("Dualsense-Multiplatform submodule URL is incorrect")
    if "branch = main" not in gitmodules:
        errors.append("Dualsense-Multiplatform submodule must follow main for update tooling")

for path in (ROOT / "Source").rglob("*"):
    if not path.is_file():
        continue
    if path.suffix not in {".h", ".cpp", ".cs"}:
        continue
    first_line = path.read_text(encoding="utf-8", errors="replace").splitlines()[:1]
    if first_line != ["// SPDX-License-Identifier: MPL-2.0"]:
        errors.append(f"Missing MPL SPDX header: {path.relative_to(ROOT)}")

if errors:
    print("Repository validation failed:")
    for error in errors:
        print(f"  - {error}")
    sys.exit(1)
print("Repository validation passed")
