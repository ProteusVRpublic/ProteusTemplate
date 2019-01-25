// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "ProteusAvatars.h"
#include "Core.h"
#include "Modules/ModuleManager.h"
#include "Features/IModularFeatures.h"
#include "Interfaces/IPluginManager.h"
//#include "ProteusAvatarsLibrary/ExampleLibrary.h"
#include "ProteusOvrAvatarManager.h"

#define LOCTEXT_NAMESPACE "FProteusAvatarsModule"

void FProteusAvatarsModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	// Get the base directory of this plugin
	FString BaseDir = IPluginManager::Get().FindPlugin("ProteusAvatars")->GetBaseDir();

	
	// Add on the relative location of the third party dll and load it
	FString LibraryPath;
	//LibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/ProteusAvatarsLibrary/Win64/ExampleLibrary.dll"));

	//ExampleLibraryHandle = !LibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*LibraryPath) : nullptr;*/

	FProteusOvrAvatarManager::Get().InitializeSDK();

	/*
	if (ExampleLibraryHandle)
	{
		// Call the test function in the third party library that opens a message box
		//ExampleLibraryFunction();
	}
	else
	{
		//FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("ThirdPartyLibraryError", "Failed to load example third party library"));
	}*/
}

void FProteusAvatarsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	// Free the dll handle
	//FPlatformProcess::FreeDllHandle(ExampleLibraryHandle);
	//ExampleLibraryHandle = nullptr;

	FProteusOvrAvatarManager::Get().ShutdownSDK();
	FProteusOvrAvatarManager::Destroy();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FProteusAvatarsModule, ProteusAvatars)
