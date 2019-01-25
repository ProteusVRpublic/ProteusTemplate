
#include "ProteusOvrAvatarManager.h"
#include "ProteusOvrAvatar.h"
#include "OVR_Avatar.h"
#include "Engine/Texture2D.h"
#include "OculusHMDModule.h"
#include "UObject/UObjectIterator.h"
#include "RenderUtils.h"

#if PLATFORM_ANDROID
#include "Android/AndroidApplication.h"
#endif

DEFINE_LOG_CATEGORY(LogAvatars);

FProteusOvrAvatarManager* FProteusOvrAvatarManager::sAvatarManager = nullptr;

FSoftObjectPath FProteusOvrAvatarManager::AssetList[] =
{
	FString(TEXT("/ProteusAvatars/Materials/AvatarsPBR_2/OculusAvatars_PBRV2_Combined")),
	FString(TEXT("/ProteusAvatars/Materials/AvatarsPBR_2/OculusAvatars_PBRV2_Mobile")),
	FString(TEXT("/ProteusAvatars/Materials/AvatarsPBR_2/OculusAvatars_PBRV2_Mobile_Combined")),
	FString(TEXT("/ProteusAvatars/Materials/AvatarsPBR_2/OculusAvatars_PBRV2_2_Depth")),
	FString(TEXT("/ProteusAvatars/Materials/AvatarsPBR_2/OculusAvatars_PBRV2")),
	FString(TEXT("/ProteusAvatars/Materials/OculusAvatarsPBR")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/Off/N_OFF_P_OFF/OculusAvatar8Layers_Inst_0Layers.OculusAvatar8Layers_Inst_0Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/Off/N_OFF_P_OFF/OculusAvatar8Layers_Inst_1Layers.OculusAvatar8Layers_Inst_1Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/Off/N_OFF_P_OFF/OculusAvatar8Layers_Inst_2Layers.OculusAvatar8Layers_Inst_2Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/Off/N_OFF_P_OFF/OculusAvatar8Layers_Inst_3Layers.OculusAvatar8Layers_Inst_3Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/Off/N_OFF_P_OFF/OculusAvatar8Layers_Inst_4Layers.OculusAvatar8Layers_Inst_4Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/Off/N_OFF_P_OFF/OculusAvatar8Layers_Inst_5Layers.OculusAvatar8Layers_Inst_5Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/Off/N_OFF_P_OFF/OculusAvatar8Layers_Inst_6Layers.OculusAvatar8Layers_Inst_6Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/Off/N_OFF_P_OFF/OculusAvatar8Layers_Inst_7Layers.OculusAvatar8Layers_Inst_7Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/Off/N_OFF_P_OFF/OculusAvatar8Layers_Inst_8Layers.OculusAvatar8Layers_Inst_8Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/Off/N_OFF_P_ON/OculusAvatar8Layers_Inst_0Layers.OculusAvatar8Layers_Inst_0Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/Off/N_OFF_P_ON/OculusAvatar8Layers_Inst_1Layers.OculusAvatar8Layers_Inst_1Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/Off/N_OFF_P_ON/OculusAvatar8Layers_Inst_2Layers.OculusAvatar8Layers_Inst_2Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/Off/N_OFF_P_ON/OculusAvatar8Layers_Inst_3Layers.OculusAvatar8Layers_Inst_3Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/Off/N_OFF_P_ON/OculusAvatar8Layers_Inst_4Layers.OculusAvatar8Layers_Inst_4Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/Off/N_OFF_P_ON/OculusAvatar8Layers_Inst_5Layers.OculusAvatar8Layers_Inst_5Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/Off/N_OFF_P_ON/OculusAvatar8Layers_Inst_6Layers.OculusAvatar8Layers_Inst_6Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/Off/N_OFF_P_ON/OculusAvatar8Layers_Inst_7Layers.OculusAvatar8Layers_Inst_7Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/Off/N_OFF_P_ON/OculusAvatar8Layers_Inst_8Layers.OculusAvatar8Layers_Inst_8Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/Off/N_ON_P_OFF/OculusAvatar8Layers_Inst_0Layers.OculusAvatar8Layers_Inst_0Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/Off/N_ON_P_OFF/OculusAvatar8Layers_Inst_1Layers.OculusAvatar8Layers_Inst_1Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/Off/N_ON_P_OFF/OculusAvatar8Layers_Inst_2Layers.OculusAvatar8Layers_Inst_2Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/Off/N_ON_P_OFF/OculusAvatar8Layers_Inst_3Layers.OculusAvatar8Layers_Inst_3Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/Off/N_ON_P_OFF/OculusAvatar8Layers_Inst_4Layers.OculusAvatar8Layers_Inst_4Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/Off/N_ON_P_OFF/OculusAvatar8Layers_Inst_5Layers.OculusAvatar8Layers_Inst_5Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/Off/N_ON_P_OFF/OculusAvatar8Layers_Inst_6Layers.OculusAvatar8Layers_Inst_6Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/Off/N_ON_P_OFF/OculusAvatar8Layers_Inst_7Layers.OculusAvatar8Layers_Inst_7Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/Off/N_ON_P_OFF/OculusAvatar8Layers_Inst_8Layers.OculusAvatar8Layers_Inst_8Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/Off/N_ON_P_ON/OculusAvatar8Layers_Inst_0Layers.OculusAvatar8Layers_Inst_0Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/Off/N_ON_P_ON/OculusAvatar8Layers_Inst_1Layers.OculusAvatar8Layers_Inst_1Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/Off/N_ON_P_ON/OculusAvatar8Layers_Inst_2Layers.OculusAvatar8Layers_Inst_2Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/Off/N_ON_P_ON/OculusAvatar8Layers_Inst_3Layers.OculusAvatar8Layers_Inst_3Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/Off/N_ON_P_ON/OculusAvatar8Layers_Inst_4Layers.OculusAvatar8Layers_Inst_4Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/Off/N_ON_P_ON/OculusAvatar8Layers_Inst_5Layers.OculusAvatar8Layers_Inst_5Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/Off/N_ON_P_ON/OculusAvatar8Layers_Inst_6Layers.OculusAvatar8Layers_Inst_6Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/Off/N_ON_P_ON/OculusAvatar8Layers_Inst_7Layers.OculusAvatar8Layers_Inst_7Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/Off/N_ON_P_ON/OculusAvatar8Layers_Inst_8Layers.OculusAvatar8Layers_Inst_8Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/On/N_OFF_P_OFF/OculusAvatar8Layers_Inst_0Layers.OculusAvatar8Layers_Inst_0Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/On/N_OFF_P_OFF/OculusAvatar8Layers_Inst_1Layers.OculusAvatar8Layers_Inst_1Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/On/N_OFF_P_OFF/OculusAvatar8Layers_Inst_2Layers.OculusAvatar8Layers_Inst_2Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/On/N_OFF_P_OFF/OculusAvatar8Layers_Inst_3Layers.OculusAvatar8Layers_Inst_3Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/On/N_OFF_P_OFF/OculusAvatar8Layers_Inst_4Layers.OculusAvatar8Layers_Inst_4Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/On/N_OFF_P_OFF/OculusAvatar8Layers_Inst_5Layers.OculusAvatar8Layers_Inst_5Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/On/N_OFF_P_OFF/OculusAvatar8Layers_Inst_6Layers.OculusAvatar8Layers_Inst_6Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/On/N_OFF_P_OFF/OculusAvatar8Layers_Inst_7Layers.OculusAvatar8Layers_Inst_7Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/On/N_OFF_P_OFF/OculusAvatar8Layers_Inst_8Layers.OculusAvatar8Layers_Inst_8Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/On/N_OFF_P_ON/OculusAvatar8Layers_Inst_0Layers.OculusAvatar8Layers_Inst_0Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/On/N_OFF_P_ON/OculusAvatar8Layers_Inst_1Layers.OculusAvatar8Layers_Inst_1Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/On/N_OFF_P_ON/OculusAvatar8Layers_Inst_2Layers.OculusAvatar8Layers_Inst_2Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/On/N_OFF_P_ON/OculusAvatar8Layers_Inst_3Layers.OculusAvatar8Layers_Inst_3Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/On/N_OFF_P_ON/OculusAvatar8Layers_Inst_4Layers.OculusAvatar8Layers_Inst_4Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/On/N_OFF_P_ON/OculusAvatar8Layers_Inst_5Layers.OculusAvatar8Layers_Inst_5Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/On/N_OFF_P_ON/OculusAvatar8Layers_Inst_6Layers.OculusAvatar8Layers_Inst_6Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/On/N_OFF_P_ON/OculusAvatar8Layers_Inst_7Layers.OculusAvatar8Layers_Inst_7Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/On/N_OFF_P_ON/OculusAvatar8Layers_Inst_8Layers.OculusAvatar8Layers_Inst_8Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/On/N_ON_P_OFF/OculusAvatar8Layers_Inst_0Layers.OculusAvatar8Layers_Inst_0Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/On/N_ON_P_OFF/OculusAvatar8Layers_Inst_1Layers.OculusAvatar8Layers_Inst_1Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/On/N_ON_P_OFF/OculusAvatar8Layers_Inst_2Layers.OculusAvatar8Layers_Inst_2Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/On/N_ON_P_OFF/OculusAvatar8Layers_Inst_3Layers.OculusAvatar8Layers_Inst_3Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/On/N_ON_P_OFF/OculusAvatar8Layers_Inst_4Layers.OculusAvatar8Layers_Inst_4Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/On/N_ON_P_OFF/OculusAvatar8Layers_Inst_5Layers.OculusAvatar8Layers_Inst_5Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/On/N_ON_P_OFF/OculusAvatar8Layers_Inst_6Layers.OculusAvatar8Layers_Inst_6Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/On/N_ON_P_OFF/OculusAvatar8Layers_Inst_7Layers.OculusAvatar8Layers_Inst_7Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/On/N_ON_P_OFF/OculusAvatar8Layers_Inst_8Layers.OculusAvatar8Layers_Inst_8Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/On/N_ON_P_ON/OculusAvatar8Layers_Inst_0Layers.OculusAvatar8Layers_Inst_0Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/On/N_ON_P_ON/OculusAvatar8Layers_Inst_1Layers.OculusAvatar8Layers_Inst_1Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/On/N_ON_P_ON/OculusAvatar8Layers_Inst_2Layers.OculusAvatar8Layers_Inst_2Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/On/N_ON_P_ON/OculusAvatar8Layers_Inst_3Layers.OculusAvatar8Layers_Inst_3Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/On/N_ON_P_ON/OculusAvatar8Layers_Inst_4Layers.OculusAvatar8Layers_Inst_4Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/On/N_ON_P_ON/OculusAvatar8Layers_Inst_5Layers.OculusAvatar8Layers_Inst_5Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/On/N_ON_P_ON/OculusAvatar8Layers_Inst_6Layers.OculusAvatar8Layers_Inst_6Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/On/N_ON_P_ON/OculusAvatar8Layers_Inst_7Layers.OculusAvatar8Layers_Inst_7Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/On/N_ON_P_ON/OculusAvatar8Layers_Inst_8Layers.OculusAvatar8Layers_Inst_8Layers")),
	FString(TEXT("/ProteusAvatars/Materials/v1/Inst/Projector.Projector"))
};

TArray<UObject*> FProteusOvrAvatarManager::AssetObjects;

FProteusOvrAvatarManager& FProteusOvrAvatarManager::Get()
{
	return sAvatarManager != nullptr ? *sAvatarManager : *(sAvatarManager = new FProteusOvrAvatarManager());
}

void FProteusOvrAvatarManager::Destroy()
{
	if (sAvatarManager)
	{
		delete sAvatarManager;
		sAvatarManager = nullptr;
	}
}

FProteusOvrAvatarManager::~FProteusOvrAvatarManager()
{
	if (OVRPluginHandle)
	{
		FPlatformProcess::FreeDllHandle(OVRPluginHandle);
		OVRPluginHandle = nullptr;
	}
}

static const FString sTextureFormatStrings[ovrAvatarTextureFormat_Count] =
{
	FString("ovrAvatarTextureFormat_RGB24"),
	FString("ovrAvatarTextureFormat_DXT1"),
	FString("ovrAvatarTextureFormat_DXT5"),
	FString("ovrAvatarTextureFormat_ASTC_RGB_6x6_DEPRECATED"),
	FString("ovrAvatarTextureFormat_ASTC_RGB_6x6_MIPMAPS")
};

static const FString sOvrEmptyString = "";

static const FString TextureFormatToString(ovrAvatarTextureFormat format)
{
	return format < ovrAvatarTextureFormat_Count ? sTextureFormatStrings[format] : sOvrEmptyString;
}

bool FProteusOvrAvatarManager::Tick(float DeltaTime)
{
	if (!IsInitialized)
		return false;

	while (ovrAvatarMessage* message = ovrAvatarMessage_Pop())
	{
		switch (ovrAvatarMessage_GetType(message))
		{
		case ovrAvatarMessageType_AvatarSpecification:
			HandleAvatarSpecification(ovrAvatarMessage_GetAvatarSpecification(message));
			break;
		case ovrAvatarMessageType_AssetLoaded:
			HandleAssetLoaded(ovrAvatarMessage_GetAssetLoaded(message));
			break;
		default:
			break;
		}

		ovrAvatarMessage_Free(message);
	}

	return true;
}

void FProteusOvrAvatarManager::SDKLogger(const char * str)
{	
	UE_LOG(LogAvatars, Display, TEXT("[AVATAR SDK]: %s"), *FString(str));
}

void FProteusOvrAvatarManager::InitializeSDK()
{
	UE_LOG(LogAvatars, Display, TEXT("FProteusOvrAvatarManager::InitializeSDK()"));

	if (!IsInitialized)
	{
#if WITH_EDITORONLY_DATA
		for (int32 AssetIndex = 0; AssetIndex < ARRAY_COUNT(AssetList); ++AssetIndex)
		{
			UObject* AssetObj = AssetList[AssetIndex].TryLoad();
			if (AssetObj != nullptr)
			{
				AssetObjects.AddUnique(AssetObj);
			}
		}
#endif
		if (IOculusHMDModule::IsAvailable())
		{
			OVRPluginHandle = FOculusHMDModule::GetOVRPluginHandle();
		}

		IsInitialized = true;

#if PLATFORM_ANDROID
		AVATAR_APP_ID = TCHAR_TO_ANSI(*GConfig->GetStr(TEXT("OnlineSubsystemOculus"), TEXT("GearVRAppId"), GEngineIni));
		UE_LOG(LogAvatars, Display, TEXT("ovrAvatar_InitializeAndroid"));
		ovrAvatar_InitializeAndroid(AVATAR_APP_ID, FAndroidApplication::GetGameActivityThis(), FAndroidApplication::GetJavaEnv());
#else
		AVATAR_APP_ID = TCHAR_TO_ANSI(*GConfig->GetStr(TEXT("OnlineSubsystemOculus"), TEXT("RiftAppId"), GEngineIni));
		UE_LOG(LogAvatars, Display, TEXT("ovrAvatar_Initialize"));
		ovrAvatar_Initialize(AVATAR_APP_ID);
#endif

		ovrAvatar_SetLoggingLevel(LogLevel);

		// Clear avatar message queue in case there are leftover/invalid messages from other sessions/apps
		while (ovrAvatarMessage* Message = ovrAvatarMessage_Pop())
		{
			ovrAvatarMessage_Free(Message);
		}

		ovrAvatar_RegisterLoggingCallback(&FProteusOvrAvatarManager::SDKLogger);
	}
}

void FProteusOvrAvatarManager::ShutdownSDK()
{
	if (IsInitialized)
	{
#if WITH_EDITORONLY_DATA
		AssetObjects.Empty();
#endif
		ovrAvatar_RegisterLoggingCallback(nullptr);

		IsInitialized = false;
		ovrAvatar_Shutdown();
	}
}

void FProteusOvrAvatarManager::HandleAvatarSpecification(const ovrAvatarMessage_AvatarSpecification* message)
{
	UE_LOG(LogAvatars, Display, TEXT("[Avatars] Request Spec Arrived [%llu]"), message->oculusUserID);

	for (TObjectIterator<UProteusOvrAvatar> Itr; Itr; ++Itr)
	{
		(*Itr)->HandleAvatarSpecification(message);
	}
}

void FProteusOvrAvatarManager::HandleAssetLoaded(const ovrAvatarMessage_AssetLoaded* message)
{
	for (TObjectIterator<UProteusOvrAvatar> Itr; Itr; ++Itr)
	{
		(*Itr)->HandleAssetLoaded(message);
	}
}

void FProteusOvrAvatarManager::LoadTexture(const uint64_t id, const ovrAvatarTextureAssetData* data)
{ 
	const bool isNormalMap = NormalMapIDs.Find(id) != nullptr;
	Textures.Add(id, LoadTexture(data, isNormalMap));

	UE_LOG(LogAvatars, Display, TEXT("[Avatars] Loaded Texture: [%llu] - [%s]"), id, *TextureFormatToString(data->format));
	UE_LOG(LogAvatars, Display, TEXT("[Avatars]        Res:     [%d]x[%d]"), data->sizeX, data->sizeY);
	UE_LOG(LogAvatars, Display, TEXT("[Avatars]        Size:    [%llu]"), data->textureDataSize);
	UE_LOG(LogAvatars, Display, TEXT("[Avatars]        Mips:    [%d]"), data->mipCount);
	UE_LOG(LogAvatars, Display, TEXT("[Avatars]        Normal:  [%d]"), isNormalMap ? 1u : 0u);
}


UTexture2D* FProteusOvrAvatarManager::LoadTexture(const ovrAvatarTextureAssetData* data, bool isNormalMap)
{
	const uint8_t* TextureData = data->textureData;
	uint8_t* NewTextureData = nullptr;
	EPixelFormat PixelFormat = PF_Unknown;
	uint64_t TextureSize = data->textureDataSize;

	switch (data->format)
	{
	case ovrAvatarTextureFormat_RGB24:
	{
		check(data->textureDataSize % 3 == 0);

		PixelFormat = EPixelFormat::PF_R8G8B8A8;
		const auto PixelCount = data->textureDataSize / 3;
		const auto NewTextureSize = PixelCount * 4;
		NewTextureData = (uint8_t*)FMemory::Malloc(NewTextureSize);
		uint8_t* TextureDataIter = NewTextureData;

		for (int pixel_index = 0; pixel_index < PixelCount; ++pixel_index)
		{
			// rgba -> bgra
			TextureDataIter[0] = TextureData[2];
			TextureDataIter[1] = TextureData[1];
			TextureDataIter[2] = TextureData[0];
			TextureDataIter[3] = 255;
			TextureData += 3;
			TextureDataIter += 4;
		}

		TextureSize = NewTextureSize;
		TextureData = NewTextureData;
		break;
	}

	case ovrAvatarTextureFormat_DXT1:
		PixelFormat = EPixelFormat::PF_DXT1;
		break;

	case ovrAvatarTextureFormat_DXT5:
		PixelFormat = EPixelFormat::PF_DXT5;
		break;
	case ovrAvatarTextureFormat_ASTC_RGB_6x6_MIPMAPS:
	case ovrAvatarTextureFormat_ASTC_RGB_6x6_DEPRECATED:
		PixelFormat = EPixelFormat::PF_ASTC_6x6;
		break;
	default:
		UE_LOG(LogAvatars, Warning, TEXT("[Avatars] Unknown pixel format [%u]."), (int)data->format);
		break;
	}

	// TODO SW: Return Default Texture
	if (PixelFormat == PF_Unknown)
		return nullptr;

	UTexture2D* UnrealTexture = NULL;
	uint32_t Width = data->sizeX;
	uint32_t Height = data->sizeY;

	const uint32_t BlockSizeX = GPixelFormats[PixelFormat].BlockSizeX;
	const uint32_t BlockSizeY = GPixelFormats[PixelFormat].BlockSizeY;
	const uint32_t BlockBytes = GPixelFormats[PixelFormat].BlockBytes;

	if (Width > 0 && Height > 0)
	{
		UnrealTexture = NewObject<UTexture2D>(GetTransientPackage(), NAME_None, RF_Transient);

		UnrealTexture->PlatformData = new FTexturePlatformData();
		UnrealTexture->PlatformData->SizeX = Width;
		UnrealTexture->PlatformData->SizeY = Height;
		UnrealTexture->PlatformData->PixelFormat = PixelFormat;
		UnrealTexture->SRGB = !isNormalMap;

		uint32_t DataOffset = 0;

		// The old deprecated format reads in as zero mips
		const uint32_t MipCount = FMath::Max(data->mipCount, 1u);

		for (uint32_t MipIndex = 0; MipIndex < MipCount; MipIndex++)
		{
			const uint32_t BlocksX = PixelFormat == EPixelFormat::PF_ASTC_6x6 ? (Width + 5) / 6 : Width / BlockSizeX;
			const uint32_t BlocksY = PixelFormat == EPixelFormat::PF_ASTC_6x6 ? (Height + 5) / 6 : Height / BlockSizeY;
			const uint32_t MipSize = PixelFormat == EPixelFormat::PF_ASTC_6x6 ? BlocksX * BlocksY * 16 : BlocksX * BlocksY * BlockBytes;

			if (MipSize == 0)
				break;

			check(DataOffset + MipSize <= TextureSize)

			FTexture2DMipMap* MipMap = new FTexture2DMipMap();

			UnrealTexture->PlatformData->Mips.Add(MipMap);
			MipMap->SizeX = Width;
			MipMap->SizeY = Height;
			MipMap->BulkData.Lock(LOCK_READ_WRITE);

			void* MipMemory = MipMap->BulkData.Realloc(MipSize);
			FMemory::Memcpy(MipMemory, TextureData + DataOffset, MipSize);
			DataOffset += MipSize;

			MipMap->BulkData.Unlock();

			Width /= 2;
			Height /= 2;

			Width = FMath::Max(Width, 1u);
			Height = FMath::Max(Height, 1u);
		}
	}

	if (NewTextureData)
	{
		delete NewTextureData;
	}

	// TODO SW: Offload to background thread to avoid hiccups on load.
	UnrealTexture->UpdateResource();

	return UnrealTexture;
}

UTexture* FProteusOvrAvatarManager::FindTexture(uint64_t id) const
{
	if (auto Tex = Textures.Find(id))
	{
		return Tex->IsValid() ? Tex->Get() : nullptr;
	}

	return nullptr;
}

void FProteusOvrAvatarManager::CacheNormalMapID(uint64_t id)
{
	if (!NormalMapIDs.Contains(id))
	{
		NormalMapIDs.Emplace(id);
	}
}

// Setting a max in case there is no consumer and recording turned on.
static const uint32_t SANITY_SIZE = 500;

/* CUSTOM
void FProteusOvrAvatarManager::QueueAvatarPacket(ProteusOvrAvatarPacket* packet)
{
	if (packet == nullptr)
	{
		return;
	}

	SerializedPacketBuffer Buffer;

	for (auto& QueuePair : AvatarPacketQueues)
	{
		auto Queue = QueuePair.Value;
		if (Queue->PacketQueueSize >= SANITY_SIZE)
		{
			UE_LOG(LogAvatars, Warning, TEXT("[Avatars] Unexpectedly large amount of packets recorded, losing data"));
			Queue->PacketQueue.Dequeue(Buffer);
			Queue->PacketQueueSize--;
			delete[] Buffer.Buffer;
		}

		Queue->PacketQueueSize++;

		// TODO SW: We should use a fast slot allocator for these packets.
		Buffer.Size = ProteusOvrAvatarPacket_GetSize(packet);
		Buffer.Buffer = new uint8_t[Buffer.Size];
		ProteusOvrAvatarPacket_Write(packet, Buffer.Size, Buffer.Buffer);
		Queue->PacketQueue.Enqueue(Buffer);
	}

	ProteusOvrAvatarPacket_Free(packet);
}
*/

void FProteusOvrAvatarManager::QueueAvatarPacket(uint8_t* inBuffer, uint32_t inBufferSize, const FString& key, uint32 packetSequenceNumber)
{
	if (inBufferSize < 1)//(packet == nullptr)
	{
		UE_LOG(LogAvatars, Warning, TEXT("FProteusOvrAvatarManager::QueueAvatarPacket() packet is empty"));
		return;
	}

	SerializedPacketBuffer Buffer;

	//find the PacketQueue that belongs to the key
	if (auto Queue = AvatarPacketQueues.Find(key))
	{
		UE_LOG(LogAvatars, Warning, TEXT("[Avatars] FProteusOvrAvatarManager::QueueAvatarPacket Found a queue with Key: %s "), *key);//pw
		if ((*Queue)->PacketQueueSize >= SANITY_SIZE)
		{
			UE_LOG(LogAvatars, Warning, TEXT("[Avatars] Unexpectedly large amount of packets recorded, losing data.  Queue Size: %d, Key: %s "), (*Queue)->PacketQueueSize, *key);
			(*Queue)->PacketQueue.Dequeue(Buffer);
			(*Queue)->PacketQueueSize--;
			delete[] Buffer.Buffer;
		}
		(*Queue)->PacketQueueSize++;
		UE_LOG(LogAvatars, Warning, TEXT("[Avatars] FProteusOvrAvatarManager::QueueAvatarPacket PacketQueueSize: %d "), (*Queue)->PacketQueueSize);//pw

																																			// TODO SW: We should use a fast slot allocator for these packets.
		Buffer.Buffer = new uint8_t[inBufferSize];
		for (uint32_t elementIdx = 0; elementIdx < inBufferSize; elementIdx++)
		{
			Buffer.Buffer[elementIdx] = inBuffer[elementIdx];
		}
		Buffer.Size = inBufferSize;

		if (!(*Queue)->PacketQueue.Enqueue(Buffer))
		{
			UE_LOG(LogAvatars, Warning, TEXT("[Avatars] FProteusOvrAvatarManager::QueueAvatarPacket() - (*Queue)->PacketQueue.Enqueue(Buffer) FAILED.  Key: %s  Buffer: %llu"), *key, Buffer.Buffer);
		}
		else
		{
			UE_LOG(LogAvatars, Warning, TEXT("[Avatars] FProteusOvrAvatarManager::QueueAvatarPacket() - (*Queue)->PacketQueue.Enqueue(Buffer).  Key: %s  SequenceNum: %d"), *key, packetSequenceNumber);
		}
	}
	else
	{
		UE_LOG(LogAvatars, Warning, TEXT("[Avatars] FProteusOvrAvatarManager::QueueAvatarPacket() AvatarPacketQueues.Find(%s) failed to find a queue"), *key);
	}
}

ovrAvatarPacket* FProteusOvrAvatarManager::RequestAvatarPacket(const FString& key)
{
	ovrAvatarPacket* ReturnPacket = nullptr;

	if (auto Queue = AvatarPacketQueues.Find(key))
	{
		SerializedPacketBuffer Buffer;

		if ((*Queue)->PacketQueue.Peek(Buffer))
		{
			(*Queue)->PacketQueueSize--;
			(*Queue)->PacketQueue.Dequeue(Buffer);
			ReturnPacket = ovrAvatarPacket_Read(Buffer.Size, Buffer.Buffer);
			delete[] Buffer.Buffer;
		}
	}

	return ReturnPacket;
}

void FProteusOvrAvatarManager::RegisterRemoteAvatar(const FString& key)
{
	check(!AvatarPacketQueues.Find(key));

	AvatarPacketQueue* NewQueue = new AvatarPacketQueue();
	AvatarPacketQueues.Add(key, NewQueue);
}

void FProteusOvrAvatarManager::UnregisterRemoteAvatar(const FString& key)
{
	if (auto Queue = AvatarPacketQueues.Find(key))
	{
		SerializedPacketBuffer Buffer;
		while ((*Queue)->PacketQueue.Peek(Buffer))
		{
			(*Queue)->PacketQueue.Dequeue(Buffer);
			delete[] Buffer.Buffer;
		}

		AvatarPacketQueues.Remove(key);

		delete (*Queue);
	}
}

float FProteusOvrAvatarManager::GetSDKPacketDuration(ovrAvatarPacket* packet)
{
	return packet != nullptr ? ovrAvatarPacket_GetDurationSeconds(packet) : 0.f;
}

void FProteusOvrAvatarManager::FreeSDKPacket(ovrAvatarPacket* packet)
{
	if (packet != nullptr)
	{
		ovrAvatarPacket_Free(packet);
	}
}

bool FProteusOvrAvatarManager::IsOVRPluginValid() const
{
#if PLATFORM_ANDROID
	return true;
#else
	return OVRPluginHandle != nullptr;
#endif
}

