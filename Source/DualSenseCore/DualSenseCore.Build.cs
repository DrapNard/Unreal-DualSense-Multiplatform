// SPDX-License-Identifier: MPL-2.0
using UnrealBuildTool;
using System.IO;

public class DualSenseCore : ModuleRules
{
    public DualSenseCore(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        CppStandard = CppStandardVersion.Cpp20;
        bEnableExceptions = false;
        bUseRTTI = false;

        PublicDependencyModuleNames.AddRange(new[] { "Core" });

        string VendorRoot = Path.Combine(ModuleDirectory, "Private", "Vendor", "GamepadCore", "Source");
        PrivateIncludePaths.Add(Path.Combine(VendorRoot, "Public"));
        PrivateIncludePaths.Add(Path.Combine(VendorRoot, "Private"));

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicSystemLibraries.AddRange(new[] { "setupapi.lib", "hid.lib" });
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            PublicFrameworks.AddRange(new[] { "IOKit", "CoreFoundation" });
        }

        PublicDefinitions.Add("DUALSENSE_MULTIPLATFORM_WITH_GAMEPADCORE=1");
    }
}
