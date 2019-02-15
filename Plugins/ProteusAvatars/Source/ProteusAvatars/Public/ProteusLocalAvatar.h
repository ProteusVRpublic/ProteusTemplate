#pragma once

#include "ProteusOvrAvatar.h"
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

USTRUCT()
struct FAvatarPacket3
{
	GENERATED_BODY()

	UPROPERTY()
		TArray<uint8> AvatarPacketData;
	UPROPERTY()
		uint32 packetSequenceNumber;
};

UENUM(BlueprintType)
enum class EAvatarLevelOfDetail : uint8
{
	AvatarLevelOfDetail_One, // low LOD, which conserves even more resources
	AvatarLevelOfDetail_Three, // medium LOD for mobile; his improves performance on Oculus Go and Samsung Gear VR by using lower resolution meshes and textures
	AvatarLevelOfDetail_Five // Oculus Rift
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
	UPROPERTY(ReplicatedUsing = OnRep_ReceivedPacket)
		FAvatarPacket3 R_AvatarPacket;
	UPROPERTY(BlueprintReadWrite, Replicated, Category = "OculusAvatar")
		int32 PartA;
	UPROPERTY(BlueprintReadWrite, Replicated, Category = "OculusAvatar")
		int32 PartB;
	UPROPERTY(BlueprintReadWrite, Replicated, Category = "OculusAvatar")
		int32 PartC;
	UPROPERTY(BlueprintReadOnly, Category = "OculusAvatar")
		bool bRemoteAvatarIsLoaded;

	/*Reducing Draw Calls with the Combine Meshes Option
	Each avatar in your scene requires 11 draw calls per eye per frame (22 total). The Combine Meshes option
	reduces this to 3 draw calls per eye (6 total) by combining all the mesh parts into a single mesh. This is an
	important performance gain for Gear VR as most apps typically need to stay within a draw call budget of 50 to
	100 draw calls per frame. Without this option, just having 4 avatars in your scene would use most or all of that
	budget.
	You should almost always select this option when using avatars. The only drawback to using this option is that
	you are no longer able to access mesh parts individually, but that is a rare use case
	*/
	UPROPERTY(BlueprintReadWrite, Replicated, Category = "OculusAvatar")
		bool bUseCombinedMeshes = true;
	UPROPERTY(BlueprintReadWrite, Replicated, Category = "OculusAvatar")
		EAvatarLevelOfDetail AvatarLevelofDetail = EAvatarLevelOfDetail::AvatarLevelOfDetail_Five;

	UFUNCTION(BlueprintCallable, Category = "OculusAvatar")
		void FetchAvatarID();
	UFUNCTION(Server, WithValidation, Unreliable, Category = "OculusAvatar")
		void ServerHandleAvatarPacket(FAvatarPacket3 ThePacket);
	void ServerHandleAvatarPacket_Implementation(FAvatarPacket3 ThePacket);
	bool ServerHandleAvatarPacket_Validate(FAvatarPacket3 ThePacket);
	UFUNCTION()
		void OnRep_ReceivedPacket();
	UFUNCTION(BlueprintCallable, Category = "OculusAvatar")
		void FetchRemoteAvatar();
	UFUNCTION(BlueprintCallable, Category = "OculusAvatar")
		void SetRightHandTransform(TArray<float> RightHandTransformsArray);
	UFUNCTION(BlueprintCallable, Category = "OculusAvatar")
		void ResetRightHandTransform();
	UFUNCTION(BlueprintCallable, Category = "OculusAvatar")
		void SetLeftHandTransform(TArray<float> RightHandTransformsArray);
	UFUNCTION(BlueprintCallable, Category = "OculusAvatar")
		void ResetLeftHandTransform();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginDestroy() override;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

private:
	void OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);
	void UpdatePacketRecording(float DeltaTime);

	bool bAvatarIsLocal;
	bool bLocalAvatarIsLoaded;
	float BufferLatency = 1.0f; //delay a bit to allow queuing up of movement data to prevent the stream from starving
	float LatencyTick = 0.f;
	float CurrentPacketTime = 0.f;
	ovrAvatarPacket* CurrentPacket = nullptr;
	uint32 NextAvatarSequenceNumber = 0;
	FAvatarPacket3 CurrentPacketStruct;
	FString PacketKey;

	const int64 ConvertAvatarID(int32 PartG, int32 PartH, int32 PartI);
	const FString StringAvatarID(const int64 AvatarIDPre);

	int64 finalAvatar = 0;
	ovrAvatarTransform ConvertedTransform;

	FDelegateHandle OnLoginCompleteDelegateHandle;

	UProteusOvrAvatar* AvatarComponent = nullptr;
	
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

	struct FPacketRecordSettings
	{
		bool Initialized = false;
		bool RecordingFrames = false;
		float UpdateRate = 1.0 / 5.f;  // Lower rate = bigger packets. Try to optimize against fidelity vs Network Overhead.
		float AccumulatedTime = 0.f;
	};

	TWeakObjectPtr<USceneComponent> AvatarHands[UProteusOvrAvatar::HandType_Count];
	FPacketRecordSettings PacketSettings;

	FVector2D StickPosition[UProteusOvrAvatar::HandType_Count];
};