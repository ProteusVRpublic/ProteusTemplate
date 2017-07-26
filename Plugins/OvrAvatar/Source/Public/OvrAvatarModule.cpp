#include "OvrAvatarPCH.h"
#include "IPluginManager.h"
#include "OvrAvatarModule.h"
#include "OvrAvatarManager.h"


void OvrAvatarModule::StartupModule()
{
	FOvrAvatarManager::Get().InitializeSDK();
}
 
void OvrAvatarModule::ShutdownModule()	
{
	FOvrAvatarManager::Get().ShutdownSDK();
	FOvrAvatarManager::Destroy();
}
 
IMPLEMENT_MODULE(OvrAvatarModule, OvrAvatar)