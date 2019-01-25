#pragma once
#include "Containers/Ticker.h"
#include "OVR_Avatar.h"
#include "Containers/Map.h"
#include "Containers/Queue.h"
#include "UObject/WeakObjectPtr.h"
#include "UObject/WeakObjectPtrTemplates.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAvatars, Log, All);

class UTexture2D;
class UTexture;

class PROTEUSAVATARS_API FProteusOvrAvatarManager : public FTickerObjectBase
{
public:
	static FProteusOvrAvatarManager& Get();

	static void Destroy();

	bool Tick(float DeltaTime) override;

	void InitializeSDK();
	void ShutdownSDK();

	void LoadTexture(const uint64_t id, const ovrAvatarTextureAssetData* data);
	UTexture* FindTexture(uint64_t id) const; 
	void CacheNormalMapID(uint64_t id);

	//These both call from the main game thread so should be thread safe.
	//CUSTOM
	UFUNCTION(Server, Reliable)
		void QueueAvatarPacket(uint8_t* inBuffer, uint32_t inBufferSize, const FString& key, uint32 packetSequenceNumber);
	//INSTEAD OF void QueueAvatarPacket(ovrAvatarPacket* packet);
	ovrAvatarPacket* RequestAvatarPacket(const FString& key);

	void RegisterRemoteAvatar(const FString& key);
	void UnregisterRemoteAvatar(const FString& key);

	//NEW
	float GetSDKPacketDuration(ovrAvatarPacket* packet);
	void FreeSDKPacket(ovrAvatarPacket* packet);

	bool IsOVRPluginValid() const;

	void SetSDKLoggingLevel(ovrAvatarLogLevel level) { ovrAvatar_SetLoggingLevel(level); }
	//END OF NEW

private:
	static void SDKLogger(const char * str);

	FProteusOvrAvatarManager() {};
	~FProteusOvrAvatarManager();

	void HandleAvatarSpecification(const ovrAvatarMessage_AvatarSpecification* message);
	void HandleAssetLoaded(const ovrAvatarMessage_AssetLoaded* message);

	UTexture2D* LoadTexture(const ovrAvatarTextureAssetData* data, bool isNormalMap);

	bool IsInitialized = false;

	TMap<uint64_t, TWeakObjectPtr<UTexture>> Textures;
	TSet<uint64_t> NormalMapIDs;

	static FProteusOvrAvatarManager* sAvatarManager;
	const char* AVATAR_APP_ID = nullptr;

	struct SerializedPacketBuffer
	{
		uint32_t Size;
		uint8_t* Buffer;
	};

	struct AvatarPacketQueue
	{
		TQueue<SerializedPacketBuffer> PacketQueue;
		uint32_t PacketQueueSize = 0;
	};

	TMap<FString, AvatarPacketQueue*> AvatarPacketQueues;
	
	//NEW
	void* OVRPluginHandle = nullptr;

	ovrAvatarLogLevel LogLevel = ovrAvatarLogLevel::ovrAvatarLogLevel_Silent;

	static FSoftObjectPath AssetList[];
	static TArray<UObject*> AssetObjects;
	//END OF NEW
};
