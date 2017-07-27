// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System;
using System.IO;

public class Proteus : ModuleRules
{
	public Proteus(ReadOnlyTargetRules Target) : base (Target)
    {
        string BaseDirectory = Path.GetFullPath(Path.Combine(ModuleDirectory, "..", ".."));
        string thirdParty = Path.Combine(BaseDirectory, "Plugins", "OvrAvatar", "ThirdParty");

        PublicDependencyModuleNames.Add("Proteus");

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "OnlineSubsystem", "OnlineSubsystemUtils", "Networking", "Sockets" });
        PrivateDependencyModuleNames.AddRange(new string[] { "OvrAvatar", "OnlineSubsystem", "Sockets", "Networking", "OnlineSubsystemUtils" });

        PublicIncludePaths.AddRange(new string[] { });
        PrivateIncludePaths.AddRange(new string[] { thirdParty + "/include" });

        PublicIncludePathModuleNames.AddRange(new string[] { });
        PrivateIncludePathModuleNames.AddRange(new string[] { });

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
