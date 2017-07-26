#pragma once

#include "GameFramework/Pawn.h"
#include "OVR_Avatar.h"
#include "Set.h"
#include "OVR_CAPI.h"
#include "OVR_Microphone.h"
#include "Engine/SkeletalMesh.h"
#include "Components/MeshComponent.h"
#include "Components/ActorComponent.h"

#include "OvrAvatar.generated.h"

#define INPUT_COMMAND_TUPLE\
	INPUT_ENTRY(AvatarLeftThumbstick,		ovrHand_Left,   ovrAvatarButton_Joystick)\
	INPUT_ENTRY(AvatarRightThumbstick,		ovrHand_Right,  ovrAvatarButton_Joystick)\
	INPUT_ENTRY(AvatarLeftFaceButton1,		ovrHand_Left,	ovrAvatarButton_One)\
	INPUT_ENTRY(AvatarRightFaceButton1,		ovrHand_Right,	ovrAvatarButton_One)\
	INPUT_ENTRY(AvatarLeftFaceButton2,		ovrHand_Left,	ovrAvatarButton_Two)\
	INPUT_ENTRY(AvatarRightFaceButton2,		ovrHand_Right,	ovrAvatarButton_Two)

#define AXIS_INPUT_TUPLE\
	AXIS_ENTRY(AvatarLeftThumbstick,		ovrHand_Left,	ovrAvatarTouch_Joystick)\
	AXIS_ENTRY(AvatarRightThumbstick,		ovrHand_Right,	ovrAvatarTouch_Joystick)\
	AXIS_ENTRY(AvatarLeftTrigger,			ovrHand_Left,	ovrAvatarTouch_Index)\
	AXIS_ENTRY(AvatarRightTrigger,			ovrHand_Right,	ovrAvatarTouch_Index)\
	AXIS_ENTRY(AvatarLeftFaceButton1,		ovrHand_Left,	ovrAvatarTouch_One)\
	AXIS_ENTRY(AvatarRightFaceButton1,		ovrHand_Right,	ovrAvatarTouch_One)\
	AXIS_ENTRY(AvatarLeftFaceButton2,		ovrHand_Left,	ovrAvatarTouch_Two)\
	AXIS_ENTRY(AvatarRightFaceButton2,		ovrHand_Right,	ovrAvatarTouch_Two)\
	AXIS_ENTRY(AvatarLeftIndexPointing,		ovrHand_Left,	ovrAvatarTouch_Pointing )\
	AXIS_ENTRY(AvatarRightIndexPointing,	ovrHand_Right,	ovrAvatarTouch_Pointing )\
	AXIS_ENTRY(AvatarLeftThumbUp,			ovrHand_Left,	ovrAvatarTouch_ThumbUp)\
	AXIS_ENTRY(AvatarRightThumbUp,			ovrHand_Right,	ovrAvatarTouch_ThumbUp)

#define CUSTOM_AXIS_TUPLE\
	CUSTOM_ENTRY( AvatarLeftTriggerAxis,	ovrHand_Left,	indexTrigger,	false )\
	CUSTOM_ENTRY( AvatarRightTriggerAxis,	ovrHand_Right,	indexTrigger,	false )\
	CUSTOM_ENTRY( AvatarLeftGrip1Axis,		ovrHand_Left,	handTrigger,	false )\
	CUSTOM_ENTRY( AvatarRightGrip1Axis,		ovrHand_Right,	handTrigger,	false )\
	CUSTOM_ENTRY( AvatarLeftThumbstickX,	ovrHand_Left,	joystickX,		false )\
	CUSTOM_ENTRY( AvatarRightThumbstickX,	ovrHand_Right,	joystickX,		false )\
	CUSTOM_ENTRY( AvatarLeftThumbstickY,	ovrHand_Left,	joystickY,		true )\
	CUSTOM_ENTRY( AvatarRightThumbstickY,	ovrHand_Right,	joystickY,		true )\

class UPoseableMeshComponent;

UCLASS()
class OVRAVATAR_API UOvrAvatar : public UActorComponent
{
	GENERATED_BODY()

public:
	enum class ePlayerType
	{
		Local,
		Remote
	};

	UOvrAvatar();

	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;

	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	void RequestAvatar(uint64_t userID);

	void HandleAvatarSpecification(const ovrAvatarMessage_AvatarSpecification* message);
	void HandleAssetLoaded(const ovrAvatarMessage_AssetLoaded* message);

	void SetVisibilityType(ovrAvatarVisibilityFlags flag) { VisibilityMask = flag; }
	void SetPlayerType(ePlayerType type) { PlayerType = type; }
	void SetPlayerHeightOffset(float offset) { PlayerHeightOffset = offset; }

	USceneComponent* DetachHand(ovrHandType hand);
	void ReAttachHand(ovrHandType hand);

	void SetRightHandPose(ovrAvatarHandGesture pose);
	void SetLeftHandPose(ovrAvatarHandGesture pose);
	void SetCustomGesture(ovrHandType hand, ovrAvatarTransform* joints, uint32_t numJoints);
	void SetControllerVisibility(ovrHandType hand, bool visible);

	void StartPacketRecording();
	ovrAvatarPacket* EndPacketRecording();
	void UpdateFromPacket(ovrAvatarPacket* packet, const float time);

	//uint64_t *OnlineUserIDptr = &OnlineUserID;
	
	//TMap* MatMeshComponents = &MeshComponents<ovrAvatarAssetID, TWeakObjectPtr<UPoseableMeshComponent>>;

	// First create a pointer to MeshComponents
	const TMap<ovrAvatarAssetID, TWeakObjectPtr<UPoseableMeshComponent>> *MeshComponentsPtr = &MeshComponents;

	// Second function that converts MeshComponents to an array of UPoseableMeshComponents
	TArray<TWeakObjectPtr<UPoseableMeshComponent>> ConvertMeshComponents();

#define INPUT_ENTRY(entry, hand, flag) \
	void entry##_Pressed();\
	void entry##_Released();
	INPUT_COMMAND_TUPLE
#undef INPUT_ENTRY

#define AXIS_ENTRY(entry, hand, flag) \
	void entry##_Value( float value);
		AXIS_INPUT_TUPLE
#undef AXIS_ENTRY

#define CUSTOM_ENTRY(entry, hand, field, invert) \
	void entry##_Value( float value);
		CUSTOM_AXIS_TUPLE
#undef CUSTOM_ENTRY

protected:
	
	static FString HandNames[ovrHand_Count];

	void UpdateTransforms(float DeltaTime);

	void DebugDrawSceneComponents();

	void AddMeshComponent(ovrAvatarAssetID id, UPoseableMeshComponent* mesh);
	void AddDepthMeshComponent(ovrAvatarAssetID id, UPoseableMeshComponent* mesh);

	UPoseableMeshComponent* GetMeshComponent(ovrAvatarAssetID id) const;
	UPoseableMeshComponent* GetDepthMeshComponent(ovrAvatarAssetID id) const;

	UPoseableMeshComponent* CreateMeshComponent(USceneComponent* parent, ovrAvatarAssetID assetID, const FString& name);
	UPoseableMeshComponent* CreateDepthMeshComponent(USceneComponent* parent, ovrAvatarAssetID assetID, const FString& name);

	void LoadMesh(USkeletalMesh* SkeletalMesh, const ovrAvatarMeshAssetData* data);

	void UpdateSDK(float DeltaTime);
	void UpdatePostSDK();
	void UpdateVoiceData(float DeltaTime);

	void UpdateMeshComponent(USceneComponent& mesh, const ovrAvatarTransform& transform);
	void UpdateMaterial(UMeshComponent& mesh, const ovrAvatarMaterialState& material);
	void UpdateMaterialPBR(UPoseableMeshComponent& mesh, const ovrAvatarRenderPart_SkinnedMeshRenderPBS& data);
	void UpdateMaterialProjector(UPoseableMeshComponent& mesh, const ovrAvatarRenderPart_ProjectorRender& data, const USceneComponent& OvrComponent);

	void UpdateSkeleton(UPoseableMeshComponent& mesh, const ovrAvatarSkinnedMeshPose& pose);

	void DebugLogAvatarSDKTransforms(const FString& wrapper);
	void DebugLogMaterialData(const ovrAvatarMaterialState& material, const FString& name);

	uint64_t OnlineUserID = 0;

	TSet<ovrAvatarAssetID> AssetIds;
	TMap<ovrAvatarAssetID, TWeakObjectPtr<UPoseableMeshComponent>> MeshComponents;
	TMap<ovrAvatarAssetID, TWeakObjectPtr<UPoseableMeshComponent>> DepthMeshComponents;

	ovrAvatar* Avatar = nullptr;

	TMap<FString, TWeakObjectPtr<USceneComponent>> RootAvatarComponents;

	ovrAvatarHandInputState HandInputState[ovrHand_Count];
	ovrAvatarTransform BodyTransform;

	bool LeftControllerVisible = false;
	bool RightControllerVisible = false;
	uint32_t VisibilityMask = ovrAvatarVisibilityFlag_ThirdPerson;

	ePlayerType PlayerType = ePlayerType::Local;
	float PlayerHeightOffset = 0.f;

	ovrMicrophoneHandle Microphone = nullptr;

	ovrAvatarAssetID ProjectorMeshID = 0;
	TWeakObjectPtr<UPoseableMeshComponent> ProjectorMeshComponent;

	TWeakObjectPtr<USceneComponent> AvatarHands[ovrHand_Count];
};