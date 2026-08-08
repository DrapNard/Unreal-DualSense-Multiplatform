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
        // Compile bridges include one upstream .cpp each; keep them as separate translation units.
        bUseUnity = false;

        PublicDependencyModuleNames.AddRange(new[] { "Core" });

        string PluginRoot = Path.GetFullPath(Path.Combine(ModuleDirectory, "..", ".."));
        string GamepadCoreRoot = Path.Combine(PluginRoot, "ThirdParty", "Dualsense-Multiplatform", "Source");
        string GamepadCorePublic = Path.Combine(GamepadCoreRoot, "Public");
        string GamepadCorePrivate = Path.Combine(GamepadCoreRoot, "Private");

        if (!Directory.Exists(GamepadCorePublic) || !Directory.Exists(GamepadCorePrivate))
        {
            throw new BuildException(
                "Dualsense-Multiplatform submodule is missing. Run: " +
                "git submodule update --init ThirdParty/Dualsense-Multiplatform");
        }

        PrivateIncludePaths.Add(GamepadCorePublic);
        PrivateIncludePaths.Add(GamepadCorePrivate);
        ExternalDependencies.Add(Path.Combine(PluginRoot, ".gitmodules"));

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
