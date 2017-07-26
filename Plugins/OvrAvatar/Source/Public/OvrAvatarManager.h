#pragma once
#include "Ticker.h"
#include "OVR_Avatar.h"
#include "OVR_CAPI.h"
#include "Map.h"

DECLARE_LOG_CATEGORY_EXTERN(LogAvatars, Log, All);

class UTexture2D;
class UTexture;

class OVRAVATAR_API FOvrAvatarManager : public FTickerObjectBase
{
public:
	static FOvrAvatarManager& Get();

	static void Destroy()
	{
		if (sAvatarManager)
		{
			delete sAvatarManager;
			sAvatarManager = nullptr;
		}
	}

	bool Tick(float DeltaTime) override;

	void InitializeSDK();
	void ShutdownSDK();

	void LoadTexture(const uint64_t id, const ovrAvatarTextureAssetData* data);
	UTexture* FindTexture(uint64_t id) const; 

	//These both call from the main game thread so should be thread safe.
	void QueueAvatarPacket(ovrAvatarPacket* packet);
	ovrAvatarPacket* RequestAvatarPacket(const FString& key);

	void RegisterRemoteAvatar(const FString& key);
	void UnregisterRemoteAvatar(const FString& key);
private:
	FOvrAvatarManager() {};

	void HandleAvatarSpecification(const ovrAvatarMessage_AvatarSpecification* message);
	void HandleAssetLoaded(const ovrAvatarMessage_AssetLoaded* message);

	UTexture2D* LoadTexture(const ovrAvatarTextureAssetData* data);

	bool IsInitialized = false;

	TMap<uint64_t, TWeakObjectPtr<UTexture>> Textures;

	static FOvrAvatarManager* sAvatarManager;
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
};
