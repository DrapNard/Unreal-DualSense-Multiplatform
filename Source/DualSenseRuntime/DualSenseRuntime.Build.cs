// SPDX-License-Identifier: MPL-2.0
using UnrealBuildTool;

public class DualSenseRuntime : ModuleRules
{
    public DualSenseRuntime(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        CppStandard = CppStandardVersion.Cpp20;

        PublicDependencyModuleNames.AddRange(new[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "DualSenseCore"
        });
    }
}
