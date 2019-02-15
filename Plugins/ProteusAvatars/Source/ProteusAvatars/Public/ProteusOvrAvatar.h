#pragma once

#include "GameFramework/Pawn.h"
#include "OVR_Avatar.h"
#include "Containers/Set.h"
#include "Components/ActorComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Components/MeshComponent.h"
#include "OVR_Plugin.h"
#include "OVR_Plugin_Types.h"

#include "ProteusOvrAvatar.generated.h"

class UPoseableMeshComponent;
class USkeletalMesh;
class UMeshComponent;

UCLASS()
class PROTEUSAVATARS_API UProteusOvrAvatar : public UActorComponent
{
	GENERATED_BODY()

public:
	enum class ePlayerType
	{
		Local,
		Remote
	};

	enum HandType
	{
		HandType_Left,
		HandType_Right,
		HandType_Count
	};

	UProteusOvrAvatar();

	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	void RequestAvatar(
		uint64_t userID,
		ovrAvatarAssetLevelOfDetail lod,
		bool useCombinedBodyMesh);


	void HandleAvatarSpecification(const ovrAvatarMessage_AvatarSpecification* message);
	void HandleAssetLoaded(const ovrAvatarMessage_AssetLoaded* message);

	void SetVisibilityType(ovrAvatarVisibilityFlags flag) { VisibilityMask = flag; }
	void SetPlayerType(ePlayerType type) { PlayerType = type; }
	void SetPlayerHeightOffset(float offset) { PlayerHeightOffset = offset; }

	USceneComponent* DetachHand(HandType hand);
	void ReAttachHand(HandType hand);

	void SetRightHandPose(ovrAvatarHandGesture pose);
	void SetLeftHandPose(ovrAvatarHandGesture pose);
	void SetCustomGesture(HandType hand, ovrAvatarTransform* joints, uint32_t numJoints);
	void SetControllerVisibility(HandType hand, bool visible);

	void StartPacketRecording();
	ovrAvatarPacket* EndPacketRecording();
	void UpdateFromPacket(ovrAvatarPacket* packet, const float time);

	void SetVoiceVisualValue(float value) { VoiceVisualValue = FMath::Clamp(value, 0.f, 1.f); }

protected:
	static FString HandNames[HandType_Count];
	static FString BodyName;

	void InitializeMaterials();

	void UpdateV2VoiceOffsetParams();
	void UpdateVoiceVizOnMesh(UPoseableMeshComponent* Mesh);
	void UpdateTransforms(float DeltaTime);

	void DebugDrawSceneComponents();
	void DebugDrawBoneTransforms();
	void DebugDriveVoiceValue(float DeltaTime);

	void AddMeshComponent(ovrAvatarAssetID id, UPoseableMeshComponent* mesh);
	void AddDepthMeshComponent(ovrAvatarAssetID id, UPoseableMeshComponent* mesh);

	UPoseableMeshComponent* GetMeshComponent(ovrAvatarAssetID id) const;
	UPoseableMeshComponent* GetDepthMeshComponent(ovrAvatarAssetID id) const;

	void RemoveMeshComponent(ovrAvatarAssetID id);
	void RemoveDepthMeshComponent(ovrAvatarAssetID id);

	UPoseableMeshComponent* CreateMeshComponent(USceneComponent* parent, ovrAvatarAssetID assetID, const FString& name);
	UPoseableMeshComponent* CreateDepthMeshComponent(USceneComponent* parent, ovrAvatarAssetID assetID, const FString& name);

	template<typename MeshAssetData, typename VertexType>
	void LoadMesh(USkeletalMesh* SkeletalMesh, const MeshAssetData* data);

	void UpdateSDK(float DeltaTime);
	void UpdatePostSDK();

	void UpdateMeshComponent(USceneComponent& mesh, const ovrAvatarTransform& transform);
	void UpdateMaterial(UMeshComponent& mesh, const ovrAvatarMaterialState& material);
	void UpdateMaterialPBR(UPoseableMeshComponent& mesh, const ovrAvatarRenderPart_SkinnedMeshRenderPBS& data);
	void UpdateMaterialProjector(UPoseableMeshComponent& mesh, const ovrAvatarRenderPart_ProjectorRender& data, const USceneComponent& OvrComponent);
	void UpdateMaterialPBRV2(UPoseableMeshComponent& mesh, const ovrAvatarRenderPart_SkinnedMeshRenderPBS_V2& data);
	
	void UpdateSkeleton(UPoseableMeshComponent& mesh, const ovrAvatarSkinnedMeshPose& pose);

	void DebugLogAvatarSDKTransforms(const FString& wrapper);
	void DebugLogMaterialData(const ovrAvatarMaterialState& material, const FString& name);

	static const FString& GetPBRV2MainMaterialString(bool useCombined);
	static ovrAvatarControllerType GetControllerTypeByHardware();

	static FColor GetColorFromVertex(const ovrAvatarMeshVertex& vertex);
	static FColor GetColorFromVertex(const ovrAvatarMeshVertexV2& vertex);

	uint64_t OnlineUserID = 0;

	TSet<ovrAvatarAssetID> AssetIds;
	TMap<ovrAvatarAssetID, TWeakObjectPtr<UPoseableMeshComponent>> MeshComponents;
	TMap<ovrAvatarAssetID, TWeakObjectPtr<UPoseableMeshComponent>> DepthMeshComponents;

	ovrAvatar* Avatar = nullptr;

	TMap<FString, TWeakObjectPtr<USceneComponent>> RootAvatarComponents;

	ovrAvatarHandInputState HandInputState[HandType_Count];
	ovrAvatarTransform BodyTransform;

	bool LeftControllerVisible = false;
	bool RightControllerVisible = false;
	uint32_t VisibilityMask = ovrAvatarVisibilityFlag_ThirdPerson;

	ePlayerType PlayerType = ePlayerType::Local;
	float PlayerHeightOffset = 0.f;

	ovrAvatarAssetID ProjectorMeshID = 0;
	TWeakObjectPtr<UPoseableMeshComponent> ProjectorMeshComponent;

	TWeakObjectPtr<USceneComponent> AvatarHands[HandType_Count];

	ovrAvatarLookAndFeelVersion LookAndFeel = ovrAvatarLookAndFeelVersion_Two;
	ovrAvatarAssetLevelOfDetail LevelOfDetail = ovrAvatarAssetLevelOfDetail_Five;

	bool UseV2VoiceVisualization = true;
	float VoiceVisualValue = 0.f;
	ovrAvatarAssetID BodyMeshID = 0;
	bool UseCombinedBodyMesh = false;
	bool AreMaterialsInitialized = false;
	ovrAvatarControllerType ControllerType = ovrAvatarControllerType_Touch;

#if PLATFORM_ANDROID
	const bool UseDepthMeshes = false; 
#else
	const bool UseDepthMeshes = true;
#endif
};
