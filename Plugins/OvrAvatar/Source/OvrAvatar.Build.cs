using UnrealBuildTool;
using System;
using System.IO;

public class OvrAvatar : ModuleRules
{
    public OvrAvatar(ReadOnlyTargetRules Target) : base (Target)
	{
        string BaseDirectory = Path.GetFullPath(Path.Combine(ModuleDirectory, ".."));
        string thirdParty = Path.Combine(BaseDirectory, "ThirdParty");

        PublicIncludePaths.AddRange(
            new string[] {
                "OvrAvatar/Private",
                "OvrAvatar/Classes",
                thirdParty + "/include",
            }
        );

		PrivateDependencyModuleNames.AddRange(
			new string[] {
				"Core",
				"CoreUObject",
				"Engine",
				"Sockets",
				"Projects",
                "RenderCore",
                "RHI",
                "Voice"
            }   
        );

        PrivateIncludePathModuleNames.AddRange(
            new string[]
            {
                "OculusRift",			// For IOculusRiftPlugin.h
                "OculusInput",			// For IOculusInputPlugin.h
                "InputDevice",			// For IInputDevice.h
				"HeadMountedDisplay",   // For IMotionController.h
                "OnlineSubsystem"
            }
        );

        PublicDependencyModuleNames.AddRange(new string[] { "LibOVR", "LibOVRPlatform", "OnlineSubsystem" });

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicAdditionalLibraries.Add(thirdParty + "/lib/libovravatar.lib");
            PublicDelayLoadDLLs.Add("libovravatar.dll");
        }
        else
        {
            PrecompileForTargets = PrecompileTargetsType.None;
        }
    }
}