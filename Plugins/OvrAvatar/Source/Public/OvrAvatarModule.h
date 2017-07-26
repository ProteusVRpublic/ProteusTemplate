#pragma once
 
#include "ModuleManager.h"
 

class OvrAvatarModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	void StartupModule();
	void ShutdownModule();
};