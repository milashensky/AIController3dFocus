// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class BaseGameUtils : ModuleRules
{
    public BaseGameUtils(ReadOnlyTargetRules Target) : base(Target)
    {
        if (
            (Target.Platform == UnrealTargetPlatform.Linux)
            || (Target.Platform == UnrealTargetPlatform.Win64)
        ) {
            PrecompileForTargets = PrecompileTargetsType.Any;
        } else {
            PrecompileForTargets = PrecompileTargetsType.None;
        }
        // PrecompileForTargets = PrecompileTargetsType.Any;
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
            new string[] {
                // ... add public include paths required here ...
            }
            );


        PrivateIncludePaths.AddRange(
            new string[] {
                // ... add other private include paths required here ...
            }
            );


        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "AIModule",
                "RigVM",
                "ControlRig"
                // ... add other public dependencies that you statically link with here ...
            }
            );


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                // ... add private dependencies that you statically link with here ...
            }
            );


        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
                // ... add any modules that your module loads dynamically here ...
            }
            );
    }
}
