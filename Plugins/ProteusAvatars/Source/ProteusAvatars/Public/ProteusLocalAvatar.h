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
#include <unordered_map>
//
#include "ProteusLocalAvatar.generated.h"
// D:\Projects\Proteus_421\Intermediate\Build\Win64\UE4Editor\Inc\Proteus

class UProteusOvrAvatar;

USTRUCT()
struct FOculusAvatarPacket
{
	GENERATED_BODY()

		UPROPERTY()
		TArray<uint8> AvatarPacketData;
};

UENUM()
enum class AvatarVisibility : uint8 {
	FirstPerson = 1 << 0, ///< Visible in the first person view
	ThirdPerson = 1 << 1, ///< Visible in the third person view
};

UENUM()
enum class AvatarMaterial : uint8 {
	Opaque,
	Translucent,
	Masked
};

UENUM()
enum class AvatarLevelOfDetail : uint8 {
	Low,
	Mid,
	High
};

struct AvatarLevelOfDetailHash
{
	template <typename T>
	std::size_t operator()(T t) const
	{
		return static_cast<std::size_t>(t);
	}
};

UCLASS()
class PROTEUSAVATARS_API AProteusLocalAvatar : public APawn
{
	GENERATED_BODY()
public:

	AProteusLocalAvatar();

	UPROPERTY()
		APawn* OwningPawn = nullptr;
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
	UPROPERTY(BlueprintReadWrite, Category = "OculusAvatar")
		bool UseCombinedMesh = false;

	UPROPERTY(BlueprintReadWrite, Category = "Avatar|Materials")
		AvatarMaterial BodyMaterial = AvatarMaterial::Masked;

	UPROPERTY(BlueprintReadWrite, Category = "Avatar|Materials")
		AvatarMaterial HandsMaterial = AvatarMaterial::Translucent;

	UPROPERTY(EditAnywhere, Category = "Avatar|Capabilities")
		bool EnableExpressive = true;

	UPROPERTY(EditAnywhere, Category = "Avatar|Capabilities")
		bool EnableBody = true;

	UPROPERTY(EditAnywhere, Category = "Avatar|Capabilities")
		bool EnableHands = true;

	UPROPERTY(EditAnywhere, Category = "Avatar|Capabilities")
		bool EnableBase = true;

	UPROPERTY(BlueprintReadWrite, Category = "Avatar")
		bool UseLocalMicrophone = false;

	//Maps First/Third Person Visibility of Avatar
	UPROPERTY(BlueprintReadWrite, Category = "Avatar")
		AvatarVisibility AvatarVisibilityType = AvatarVisibility::FirstPerson;

	UPROPERTY(BlueprintReadWrite, Category = "Avatar")
		AvatarLevelOfDetail LevelOfDetail = AvatarLevelOfDetail::High;

	UFUNCTION(Server, WithValidation, Unreliable)
	void ServerHandleAvatarPacket(const FOculusAvatarPacket& PacketData);
	void ServerHandleAvatarPacket_Implementation(const FOculusAvatarPacket& PacketData);
	bool ServerHandleAvatarPacket_Validate(const FOculusAvatarPacket& PacketData);

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
	UFUNCTION(BlueprintCallable, Category = "OculusAvatar")
		void SetControllersVisibility(bool ControllersVisible);
	UFUNCTION(BlueprintCallable, Category = "OculusAvatar")
		void AvatarVisibility();

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

	bool bAvatarIsLocal = true;
	float CurrentPacketTime = 0.f;
	ovrAvatarPacket* CurrentPacket = nullptr;

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

	TWeakObjectPtr<USceneComponent> AvatarHands[UOvrAvatar::HandType_Count];

	float CurrentPacketLength = 0.f;

	static std::unordered_map<AvatarLevelOfDetail, ovrAvatarAssetLevelOfDetail, AvatarLevelOfDetailHash> LODMap;
};