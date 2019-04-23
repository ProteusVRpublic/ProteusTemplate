#pragma once

#include "OvrAvatar.h"
// \Plugins\Runtime\Oculus\OculusAvatar\Source\Public\OvrAvatar.h
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
// \Source\Runtime\Engine\Classes\GameFramework\Pawn.h
#include "Online.h"
// \Plugins\Online\OnlineSubsystem\Source\Public\Online.h
#include "OVR_Avatar.h"
// \Source\ThirdParty\Oculus\LibOVRAvatar\LibOVRAvatar\include\OVR_Avatar.h
#include"Runtime/Engine/Public/Net/UnrealNetwork.h"
// \Source\Runtime\Engine\Public\Net\UnrealNetwork.h
#include "Components/ActorComponent.h"
//#include <stdio.h>
// \Source\Runtime\Engine\Classes\Components\ActorComponent.h
#include "ProteusLocalAvatar.generated.h"
// D:\Projects\Proteus_421\Intermediate\Build\Win64\UE4Editor\Inc\Proteus

class UProteusOvrAvatar;

/*USTRUCT()
struct FAvatarPacket3
{
	GENERATED_BODY()

	UPROPERTY()
		TArray<uint8> AvatarPacketData;
	UPROPERTY()
		uint32 packetSequenceNumber;
};*/

USTRUCT()
struct FOculusAvatarPacket
{
	GENERATED_BODY()

		UPROPERTY()
		TArray<uint8> AvatarPacketData;
};


UENUM(BlueprintType)
enum class EAvatarLevelOfDetail : uint8
{
	AvatarLevelOfDetail_One, // low LOD, which conserves even more resources
	AvatarLevelOfDetail_Three, // medium LOD for mobile; his improves performance on Oculus Go and Samsung Gear VR by using lower resolution meshes and textures
	AvatarLevelOfDetail_Five // Oculus Rift
};

UENUM()
enum class AvatarMaterial : uint8 {
	Opaque,
	Translucent,
	Masked
};

UCLASS()
class PROTEUSAVATARS_API AProteusLocalAvatar : public APawn
{
	GENERATED_BODY()
public:

	AProteusLocalAvatar();

	UPROPERTY()
		APawn* OwningPawn = nullptr;
	//Server side property that when updated will call OnRep_ReceivedPacket() function on all clients.
	//UPROPERTY(ReplicatedUsing = OnRep_ReceivedPacket)
	//	FAvatarPacket3 R_AvatarPacket;
	UPROPERTY(ReplicatedUsing = OnRep_PacketData)
		FOculusAvatarPacket ReplicatedPacketData;
	UPROPERTY(BlueprintReadWrite, Replicated, Category = "OculusAvatar")
	FString OculusID;
	UPROPERTY(BlueprintReadWrite, Replicated, Category = "OculusAvatar")
		bool IsUsingAvatars = true;
	UPROPERTY(BlueprintReadOnly, Category = "OculusAvatar")
		bool bRemoteAvatarIsLoaded = false;
	/*Reducing Draw Calls with the Combine Meshes Option
	Each avatar in your scene requires 11 draw calls per eye per frame (22 total). The Combine Meshes option
	reduces this to 3 draw calls per eye (6 total) by combining all the mesh parts into a single mesh. This is an
	important performance gain for Gear VR as most apps typically need to stay within a draw call budget of 50 to
	100 draw calls per frame. Without this option, just having 4 avatars in your scene would use most or all of that
	budget.
	You should almost always select this option when using avatars. The only drawback to using this option is that
	you are no longer able to access mesh parts individually, but that is a rare use case
	*/
	UPROPERTY(EditAnywhere, Category = "OculusAvatar")
		bool UseCombinedMesh = true;

	UPROPERTY(EditAnywhere, Category = "Avatar|Materials")
		AvatarMaterial BodyMaterial = AvatarMaterial::Masked;

	UPROPERTY(EditAnywhere, Category = "Avatar|Materials")
		AvatarMaterial HandsMaterial = AvatarMaterial::Translucent;

	UPROPERTY(EditAnywhere, Category = "Avatar|Capabilities")
		bool EnableExpressive = true;

	UPROPERTY(EditAnywhere, Category = "Avatar|Capabilities")
		bool EnableBody = true;

	UPROPERTY(EditAnywhere, Category = "Avatar|Capabilities")
		bool EnableHands = true;

	UPROPERTY(EditAnywhere, Category = "Avatar|Capabilities")
		bool EnableBase = true;

	//UPROPERTY(EditAnywhere, Category
	//UFUNCTION(Server, WithValidation, Unreliable, Category = "OculusAvatar")
	//	void ServerHandleAvatarPacket(FAvatarPacket3 ThePacket);
	//	void ServerHandleAvatarPacket_Implementation(FAvatarPacket3 ThePacket);
	//	bool ServerHandleAvatarPacket_Validate(FAvatarPacket3 ThePacket);

	UFUNCTION(Server, WithValidation, Unreliable)
	void ServerHandleAvatarPacket(const FOculusAvatarPacket& PacketData);
	void ServerHandleAvatarPacket_Implementation(const FOculusAvatarPacket& PacketData);
	bool ServerHandleAvatarPacket_Validate(const FOculusAvatarPacket& PacketData);

	//UFUNCTION()
	//	void OnRep_ReceivedPacket();
	UFUNCTION(BlueprintCallable, Category = "OculusAvatar")
		void SetRightHandTransform(TArray<float> RightHandTransformsArray);
	UFUNCTION(BlueprintCallable, Category = "OculusAvatar")
		void ResetRightHandTransform();
	UFUNCTION(BlueprintCallable, Category = "OculusAvatar")
		void SetLeftHandTransform(TArray<float> RightHandTransformsArray);
	UFUNCTION(BlueprintCallable, Category = "OculusAvatar")
		void ResetLeftHandTransform();
	UFUNCTION(BlueprintCallable, Category = "OculusAvatar")
		void InitializeLocalAvatar(FString OculusUserID);
	UFUNCTION(BlueprintCallable, Category = "OculusAvatar")
		void InitializeRemoteAvatar();
	UFUNCTION(BlueprintCallable, Category = "OculusAvatar")
		void SetLeftHandVisibility(bool LeftHandVisible);
	UFUNCTION(BlueprintCallable, Category = "OculusAvatar")
		void SetRightHandVisibility(bool RightHandVisible);

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginDestroy() override;
	virtual void PreInitializeComponents() override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;
	static UOvrAvatar::MaterialType GetOvrAvatarMaterialFromType(AvatarMaterial material);

private:
	UFUNCTION()
		void OnRep_PacketData();
	UFUNCTION()
		void LipSyncVismesReady();

	void UpdatePacketRecording(float DeltaTime);

	//virtual void Destroy() override;
	//void RemoveComponents();

	bool bAvatarIsLocal = true;
	//float BufferLatency = 1.0f; //delay a bit to allow queuing up of movement data to prevent the stream from starving
	//float LatencyTick = 0.f;
	float CurrentPacketTime = 0.f;
	ovrAvatarPacket* CurrentPacket = nullptr;
	//uint32 NextAvatarSequenceNumber = 0;
	//FAvatarPacket3 CurrentPacketStruct;

	//ovrAvatarTransform ConvertedTransform;

	//FDelegateHandle OnLoginCompleteDelegateHandle;

	UOvrAvatar* AvatarComponent = nullptr;
	class UOVRLipSyncPlaybackActorComponent* PlayBackLipSyncComponent = nullptr;
	class UOVRLipSyncActorComponent* LipSyncComponent = nullptr;
	class UAudioComponent* AudioComponent = nullptr;
	
	enum class eHandPoseState
	{
		Default,
		Sphere,
		Cube,
		Custom,
		Controller,
		Detached,
	};

	eHandPoseState LeftHandPoseIndex = eHandPoseState::Default;
	eHandPoseState RightHandPoseIndex = eHandPoseState::Default;

	/*struct FPacketRecordSettings
	{
		bool Initialized = false;
		bool RecordingFrames = false;
		float UpdateRate = 1.0f / 5.f;  // Lower rate = bigger packets. Try to optimize against fidelity vs Network Overhead.
		float AccumulatedTime = 0.f;
	};*/

	TWeakObjectPtr<USceneComponent> AvatarHands[UOvrAvatar::HandType_Count];
	//FPacketRecordSettings PacketSettings;

	float CurrentPacketLength = 0.f;
	bool UseCannedLipSyncPlayback = false;
};