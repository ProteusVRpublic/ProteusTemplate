using UnrealBuildTool;
using System;
using System.IO;

public class ProteusAvatars : ModuleRules
{
	public ProteusAvatars(ReadOnlyTargetRules Target) : base(Target)
	{
        // Precompiled header usage for this module
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        string BaseDirectory = Path.GetFullPath(Path.Combine(ModuleDirectory, ".."));

        //(This setting is currently not need as we discover all files from the 'Public' folder) List of all paths to include files that are exposed to other modules
        PublicIncludePaths.AddRange
            (
			new string[]
            {
                BaseDirectory + "/ProteusAvatars/Public",
                // Access to ProteusOvrAvatarHelpers.h
                BaseDirectory + "/ProteusAvatars/Private",
                //Access to OVR_Avatar.h and OVR_Avatar_defs.h
                EngineDirectory + "/Source/ThirdParty/Oculus/LibOVRAvatar/LibOVRAvatar/include",
                // Access to OculusHMDModule.h
                EngineDirectory + "/Plugins/Runtime/Oculus/OculusVR/Source/OculusHMD/Private",
                //Access to Avatars Materials
                EngineDirectory + "/Plugins/Runtime/Oculus/OculusAvatar"
            }
			);

        // List of public dependency module names (no path needed) (automatically does the private/public include). These are modules that are required by our public source files.
        PublicIncludePathModuleNames.AddRange
            (
            new string[]
            {
                //Access to the module /Source/ThirdParty/Oculus/OVRPlugin
                "OVRPlugin",
                "OVRLipSync"
            }
            );

        // List of public dependency module names (no path needed) (automatically does the private/public include).
        // These are modules that are required by our public source files
        PublicDependencyModuleNames.AddRange
            (
            new string[]
            {
                "Core",
               // "ProteusAvatarsLibrary",
                "Projects",
                //Access to the module /Source/ThirdParty/Oculus/OVRPlugin and /LibOVRAvatar
                "OVRPlugin",
                "LibOVRAvatar",
                //Access to /Plugins/Runtime/Oculus/OculusVR/Source/OculusHMD
                "OculusHMD",
                "CoreUObject",
                "Engine",
                "InputCore",
                "UMG",
                "Slate",
                "SlateCore",
                "RenderCore",
                "OnlineSubsystem",
                "HeadMountedDisplay",
                "Voice",
                "OculusAvatar",
                "OVRLipSync"
            }
            );


        // List of all paths to this module's internal include files, not exposed to other modules (at least one include to the 'Private' path, more if we want to avoid relative paths)
        PrivateIncludePaths.AddRange
            (
			new string[]
            {
            BaseDirectory + "/ProteusAvatars/Private"
            }
			);

        // List of modules name(no path needed) with header files that our module's private code files needs access to, but we don't need to "import" or link against.
        PrivateIncludePathModuleNames.AddRange
            (
            new string[]
            {
                 //Access to /Plugins/Runtime/Oculus/OculusVR/Source/OculusHMD
                "OculusHMD",
                "OVRLipSync"
            }
            );

        // List of private dependency module names.These are modules that our private code depends on but nothing in our public include files depend on.
       PrivateDependencyModuleNames.AddRange
            (
			new string[]
			{
            "Core", "CoreUObject", "Engine", "InputCore", "UMG", "Slate", "SlateCore", "RenderCore",
            "OnlineSubsystem", "HeadMountedDisplay", "OVRPlugin", "OculusHMD","LibOVRAvatar", "OculusAvatar", "OVRLipSync"
            }
			);

        // Addition modules this module may require at run-time
        DynamicallyLoadedModuleNames.AddRange
            (
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            // List of delay load DLLs - typically used for External (third party) modules
            // Binaries/ThirdParty/Oculus/OVRPlugin/OVRPlugin/Win64/OVRPLugin.dll
            PublicDelayLoadDLLs.Add("libovravatar.dll");
            // Binaries/ThirdParty/Oculus/OVRPlugin/OVRPlugin/Win64/OVRPLugin.dll
            PublicDelayLoadDLLs.Add("OVRPlugin.dll");
            // List of files which this module depends on at runtime. These files will be staged along with the target.
            RuntimeDependencies.Add("$(EngineDir)/Binaries/ThirdParty/Oculus/OVRPlugin/OVRPlugin/" + Target.Platform.ToString() + "/OVRPlugin.dll");
        }
        // Which targets this module should be precompiled for
        else if (Target.Platform == UnrealTargetPlatform.Android)
        {
            PublicDelayLoadDLLs.Add("libovravatarloader.so");
            PublicDelayLoadDLLs.Add("libOVRPlugin.so");
        // List of files which this module depends on at runtime. These files will be staged along with the target.
           // RuntimeDependencies.Add("$(EngineDir)/Binaries/ThirdParty/Oculus/OVRPlugin/OVRPlugin/Lib/armeabi-v7a/libOVRPlugin.so");
        }
        else
        {
            PrecompileForTargets = PrecompileTargetsType.None;
        }
    }
}
