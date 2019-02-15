// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "ProteusAvatars.h"
#include "Core.h"
#include "Modules/ModuleManager.h"
#include "Features/IModularFeatures.h"
#include "Interfaces/IPluginManager.h"
#include "ProteusOvrAvatarManager.h"

#define LOCTEXT_NAMESPACE "FProteusAvatarsModule"

void FProteusAvatarsModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	// Get the base directory of this plugin
	FString BaseDir = IPluginManager::Get().FindPlugin("ProteusAvatars")->GetBaseDir();

	
	// Add on the relative location of the third party dll and load it
	FString LibraryPath;
	FProteusOvrAvatarManager::Get().InitializeSDK();
}

void FProteusAvatarsModule::ShutdownModule()
{
	FProteusOvrAvatarManager::Get().ShutdownSDK();
	FProteusOvrAvatarManager::Destroy();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FProteusAvatarsModule, ProteusAvatars)
