// Fill out your copyright notice in the Description page of Project Settings.

#include "OvrAvatarPCH.h"
#include "OvrAvatar.h"
#include "OvrAvatarManager.h"
#include "Private/OvrAvatarHelpers.h"
#include "Components/PoseableMeshComponent.h"
#include "IOculusRiftPlugin.h"
#include "IOculusInputPlugin.h"
#include "OVR_Voip_LowLevel.h"

float DebugLineScale = 100.f;
bool DrawDebug = false;

FString UOvrAvatar::HandNames[ovrHand_Count] = { FString("hand_left"), FString("hand_right") };

static const FString sEmptyString = "";

static const FString sMatBlendModeStrings[ovrAvatarMaterialLayerBlendMode_Count] =
{
	FString("Add"),
	FString("Multiply")
};

static const FString sMatSampleModeStrings[ovrAvatarMaterialLayerSampleMode_Count] =
{
	FString("Color"),
	FString("Texture"),
	FString("TextureSingleChannel"),
	FString("Parallax")
};

static const FString sMatMaskTypeStrings[ovrAvatarMaterialMaskType_Count] =
{
	FString("None"),
	FString("Positional"),
	FString("ViewReflection"),
	FString("Fresnel"),
	FString("Pulse")
};

static const FString MaskTypeToString(ovrAvatarMaterialMaskType mode)
{
	return mode < ovrAvatarMaterialMaskType_Count ? sMatMaskTypeStrings[mode] : sEmptyString;
}

static const FString BlendModeToString(ovrAvatarMaterialLayerBlendMode mode)
{
	return mode < ovrAvatarMaterialLayerBlendMode_Count ? sMatBlendModeStrings[mode] : sEmptyString;
}

static const FString SampleModeToString(ovrAvatarMaterialLayerSampleMode mode)
{
	return mode < ovrAvatarMaterialLayerSampleMode_Count ? sMatSampleModeStrings[mode] : sEmptyString;
}

UOvrAvatar::UOvrAvatar()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UOvrAvatar::BeginPlay()
{
	Super::BeginPlay();

	OvrAvatarHelpers::OvrAvatarHandISZero(HandInputState[ovrHand_Left]);
	OvrAvatarHelpers::OvrAvatarHandISZero(HandInputState[ovrHand_Right]);
	HandInputState[ovrHand_Left].isActive = true;
	HandInputState[ovrHand_Right].isActive = true;
	AvatarHands[ovrHand_Left] = nullptr;
	AvatarHands[ovrHand_Right] = nullptr;
}

void UOvrAvatar::BeginDestroy()
{
	Super::BeginDestroy();

	UE_LOG(LogAvatars, Display, TEXT("[Avatars] AOvrAvatar::BeginDestroy()"));

	if (Avatar)
	{
		ovrAvatar_Destroy(Avatar);
		Avatar = nullptr;
	}

}

void UOvrAvatar::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!Avatar || AssetIds.Num() != 0)
		return;

	UpdateVoiceData(DeltaTime);
	UpdateSDK(DeltaTime);
	UpdatePostSDK();
}

void UOvrAvatar::AddMeshComponent(ovrAvatarAssetID id, UPoseableMeshComponent* mesh)
{
	if (!GetMeshComponent(id))
	{
		MeshComponents.Add(id, mesh);
	}
}

void UOvrAvatar::AddDepthMeshComponent(ovrAvatarAssetID id, UPoseableMeshComponent* mesh)
{
	if (!GetDepthMeshComponent(id))
	{
		DepthMeshComponents.Add(id, mesh);
	}
}

void UOvrAvatar::HandleAvatarSpecification(const ovrAvatarMessage_AvatarSpecification* message)
{
	if (Avatar || OnlineUserID != message->oculusUserID)
		return;

	Avatar = ovrAvatar_Create(message->avatarSpec, ovrAvatarCapability_All);

	DebugLogAvatarSDKTransforms(TEXT("HandleAvatarSpecification"));

	ovrAvatar_SetLeftControllerVisibility(Avatar, LeftControllerVisible);
	ovrAvatar_SetRightControllerVisibility(Avatar, RightControllerVisible);

	const uint32_t ComponentCount = ovrAvatarComponent_Count(Avatar);
	RootAvatarComponents.Reserve(ComponentCount);

	for (uint32_t CompIndex = 0; CompIndex < ComponentCount; ++CompIndex)
	{
		const ovrAvatarComponent* AvatarComponent = ovrAvatarComponent_Get(Avatar, CompIndex);

		FString name = AvatarComponent->name;
		USceneComponent* BaseComponent = NewObject<USceneComponent>(this, *name);
		BaseComponent->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
		BaseComponent->RegisterComponent();
		RootAvatarComponents.Add(name, BaseComponent);

		for (uint32_t RenderIndex = 0; RenderIndex < AvatarComponent->renderPartCount; ++RenderIndex)
		{
			const ovrAvatarRenderPart* RenderPart = AvatarComponent->renderParts[RenderIndex];

			switch (ovrAvatarRenderPart_GetType(RenderPart))
			{
			case ovrAvatarRenderPartType_SkinnedMeshRender:
			{
				const ovrAvatarRenderPart_SkinnedMeshRender* RenderData = ovrAvatarRenderPart_GetSkinnedMeshRender(RenderPart);
				FString MeshName = name + FString::Printf(TEXT("_%u"), RenderIndex);
				UPoseableMeshComponent* MeshComponent = CreateMeshComponent(BaseComponent, RenderData->meshAssetID, MeshName);

				UPoseableMeshComponent* DepthMesh = CreateDepthMeshComponent(BaseComponent, RenderData->meshAssetID, MeshName + TEXT("_Depth"));
				DepthMesh->SetMasterPoseComponent(MeshComponent);

				const auto& material = RenderData->materialState;
				const bool UseNormalMap = material.normalMapTextureID > 0;
				bool UseParallax = material.parallaxMapTextureID > 0;

				for (uint32_t l = 0; l < material.layerCount && !UseParallax; ++l)
				{
					UseParallax = material.layers[l].sampleMode == ovrAvatarMaterialLayerSampleMode_Parallax;
				}

				FString MaterialFolder = TEXT("");
				FString AlphaFolder = material.alphaMaskTextureID > 0 ? TEXT("AlphaOn/") : TEXT("AlphaOff/");
				
				if (UseNormalMap && UseParallax)
				{
					MaterialFolder = TEXT("N_ON_P_ON/");
				}
				else if (UseNormalMap && !UseParallax)
				{
					MaterialFolder = TEXT("N_ON_P_OFF/");
				}
				else if (!UseNormalMap && UseParallax)
				{
					MaterialFolder = TEXT("N_OFF_P_ON/");
				}
				else
				{
					MaterialFolder = TEXT("N_OFF_P_OFF/");
				}
				
				FString sMaterialName = TEXT("OculusAvatar8Layers_Inst_") + FString::FromInt(material.layerCount) + TEXT("Layers");
				FString sMaterialPath = TEXT("/Game/Avatars/Materials/Oculus/OculusAvatar8Layers/Instances/") + AlphaFolder + MaterialFolder + sMaterialName + TEXT(".") + sMaterialName;

				auto Material = LoadObject<UMaterialInstance>(nullptr, *sMaterialPath, nullptr, LOAD_None, nullptr);
				MeshComponent->SetMaterial(0, UMaterialInstanceDynamic::Create(Material, GetTransientPackage()));

				DebugLogMaterialData(material, MeshComponent->GetName());
			}
			break;
			case ovrAvatarRenderPartType_SkinnedMeshRenderPBS:
			{
				const ovrAvatarRenderPart_SkinnedMeshRenderPBS* RenderData = ovrAvatarRenderPart_GetSkinnedMeshRenderPBS(RenderPart);
				FString MeshName = name + FString::Printf(TEXT("_%u"), RenderIndex);
				auto MeshComponent = CreateMeshComponent(BaseComponent, RenderData->meshAssetID, MeshName);

				auto Material = (UMaterialInterface*)StaticLoadObject(UMaterial::StaticClass(), NULL, TEXT("/Game/Avatars/Materials/Oculus/OculusAvatarsPBR.OculusAvatarsPBR"), NULL, LOAD_None, NULL);
				MeshComponent->SetMaterial(0, UMaterialInstanceDynamic::Create(Material, GetTransientPackage()));
			}
			break;
			case ovrAvatarRenderPartType_ProjectorRender:
			{
				const ovrAvatarRenderPart_ProjectorRender* RenderData = ovrAvatarRenderPart_GetProjectorRender(RenderPart);
				UE_LOG(LogAvatars, Display, TEXT("[Avatars] Projector Found - %u - %u"), RenderData->componentIndex, RenderData->renderPartIndex);

				const ovrAvatarComponent* MappedComponent = ovrAvatarComponent_Get(Avatar, RenderData->componentIndex);
				const ovrAvatarRenderPart* MappedPart = MappedComponent->renderParts[RenderData->renderPartIndex];

				switch (ovrAvatarRenderPart_GetType(MappedPart))
				{
				case ovrAvatarRenderPartType_SkinnedMeshRender:
					ProjectorMeshID = ovrAvatarRenderPart_GetSkinnedMeshRender(MappedPart)->meshAssetID;
					break;
				default:
					break;
				}

				FString MeshName = name + FString::Printf(TEXT("_%u"), RenderIndex) + TEXT("_Projector");

				UPoseableMeshComponent* MeshComponent = NewObject<UPoseableMeshComponent>(BaseComponent->GetOwner(), *MeshName);
				MeshComponent->AttachToComponent(BaseComponent, FAttachmentTransformRules::SnapToTargetIncludingScale);
				MeshComponent->RegisterComponent();
				MeshComponent->bCastDynamicShadow = false;
				MeshComponent->CastShadow = false;
				MeshComponent->TranslucencySortPriority = 1;

				if (UPoseableMeshComponent* RootMesh = GetMeshComponent(ProjectorMeshID))
				{
					MeshComponent->SetMasterPoseComponent(RootMesh);
				}

				const auto& material = RenderData->materialState;

				FString sMaterialName = TEXT("Projector");
				FString sMaterialPath = TEXT("/Game/Avatars/Materials/Oculus/OculusAvatar8Layers/Instances/") + sMaterialName + TEXT(".") + sMaterialName;

				auto Material = LoadObject<UMaterialInstance>(nullptr, *sMaterialPath, nullptr, LOAD_None, nullptr);
				MeshComponent->SetMaterial(0, UMaterialInstanceDynamic::Create(Material, GetTransientPackage()));

				DebugLogMaterialData(material, MeshComponent->GetName());

				ProjectorMeshComponent = MeshComponent;
			}
			break;
			default:
				break;
			}
		}
	}

	const auto AssetsWaitingToLoad = ovrAvatar_GetReferencedAssetCount(Avatar);

	for (uint32_t AssetIndex = 0; AssetIndex < AssetsWaitingToLoad; ++AssetIndex)
	{
		const ovrAvatarAssetID Asset = ovrAvatar_GetReferencedAsset(Avatar, AssetIndex);
		AssetIds.Add(Asset);
		ovrAvatarAsset_BeginLoading(Asset);
	}
}

void UOvrAvatar::HandleAssetLoaded(const ovrAvatarMessage_AssetLoaded* message)
{
	if (auto Asset = AssetIds.Find(message->assetID))
	{
		AssetIds.Remove(*Asset);

		const ovrAvatarAssetType assetType = ovrAvatarAsset_GetType(message->asset);

		switch (assetType)
		{
		case ovrAvatarAssetType_Mesh:
		{
			if (UPoseableMeshComponent* MeshComp = GetMeshComponent(message->assetID))
			{
				USkeletalMesh* mesh = NewObject<USkeletalMesh>(GetTransientPackage(), NAME_None, RF_Transient);
				LoadMesh(mesh, ovrAvatarAsset_GetMeshData(message->asset));
				MeshComp->SetSkeletalMesh(mesh);
				MeshComp->RecreateRenderState_Concurrent();

				if (UPoseableMeshComponent* DepthMesh = GetDepthMeshComponent(message->assetID))
				{
					DepthMesh->SetSkeletalMesh(mesh);
					DepthMesh->RecreateRenderState_Concurrent();
				}

				if (message->assetID == ProjectorMeshID && ProjectorMeshComponent.IsValid())
				{
					ProjectorMeshComponent->SetSkeletalMesh(mesh);
				}
			}
		}
		break;
		case ovrAvatarAssetType_Texture:
			if (!FOvrAvatarManager::Get().FindTexture(message->assetID))
			{
				FOvrAvatarManager::Get().LoadTexture(message->assetID, ovrAvatarAsset_GetTextureData(message->asset));
			}
			break;
		default:
			UE_LOG(LogAvatars, Warning, TEXT("[Avatars] Unknown Asset Type"));
			break;
		}
	}
}

UPoseableMeshComponent* UOvrAvatar::GetMeshComponent(ovrAvatarAssetID id) const
{
	UPoseableMeshComponent* Return = nullptr;

	auto MeshComponent = MeshComponents.Find(id);
	if (MeshComponent && MeshComponent->IsValid())
	{
		Return = MeshComponent->Get();
	}

	return Return;
}

UPoseableMeshComponent* UOvrAvatar::GetDepthMeshComponent(ovrAvatarAssetID id) const
{
	UPoseableMeshComponent* Return = nullptr;

	auto MeshComponent = DepthMeshComponents.Find(id);
	if (MeshComponent && MeshComponent->IsValid())
	{
		Return = MeshComponent->Get();
	}

	return Return;
}

void UOvrAvatar::DebugDrawSceneComponents()
{
	DebugLineScale = 200.f;
	FTransform world_trans = GetOwner()->GetRootComponent()->GetComponentTransform();
	OvrAvatarHelpers::DebugDrawCoords(GetWorld(), world_trans);

	DebugLineScale = 100.f;
	for (auto comp : RootAvatarComponents)
	{
		if (comp.Value.IsValid())
		{
			world_trans = comp.Value->GetComponentTransform();
			OvrAvatarHelpers::DebugDrawCoords(GetWorld(), world_trans);
		}
	}

	DebugLineScale = 50.f;
	for (auto mesh : MeshComponents)
	{
		if (mesh.Value.IsValid())
		{
			world_trans = mesh.Value->GetComponentTransform();
			OvrAvatarHelpers::DebugDrawCoords(GetWorld(), world_trans);
		}
	}
}

void UOvrAvatar::UpdateSDK(float DeltaTime)
{
	UpdateTransforms(DeltaTime);
	ovrAvatarPose_Finalize(Avatar, DeltaTime);
}

void UOvrAvatar::UpdatePostSDK()
{
	DebugLogAvatarSDKTransforms(TEXT("UpdatePostSDK"));

	//Copy SDK Transforms into UE4 Components
	const uint32_t ComponentCount = ovrAvatarComponent_Count(Avatar);
	for (uint32_t ComponentIndex = 0; ComponentIndex < ComponentCount; ComponentIndex++)
	{
		const ovrAvatarComponent* OvrComponent = ovrAvatarComponent_Get(Avatar, ComponentIndex);
		USceneComponent* OvrSceneComponent = nullptr;

		if (auto ScenePtr = RootAvatarComponents.Find(FString(OvrComponent->name)))
		{
			OvrSceneComponent = ScenePtr->Get();
			if (OvrSceneComponent)
			{
				OvrAvatarHelpers::OvrAvatarTransformToSceneComponent(*OvrSceneComponent, OvrComponent->transform);
			}
		}

		for (uint32_t RenderIndex = 0; RenderIndex < OvrComponent->renderPartCount; ++RenderIndex)
		{
			const ovrAvatarRenderPart* RenderPart = OvrComponent->renderParts[RenderIndex];

			switch (ovrAvatarRenderPart_GetType(RenderPart))
			{
			case ovrAvatarRenderPartType_SkinnedMeshRender:
			{
				const ovrAvatarRenderPart_SkinnedMeshRender* RenderData = ovrAvatarRenderPart_GetSkinnedMeshRender(RenderPart);
				const bool MeshVisible = (VisibilityMask & RenderData->visibilityMask) != 0;

				if (UPoseableMeshComponent* mesh = GetMeshComponent(RenderData->meshAssetID))
				{
					if (MeshVisible)
					{
						UpdateMeshComponent(*mesh, RenderData->localTransform);
						UpdateSkeleton(*mesh, RenderData->skinnedPose);
						UpdateMaterial(*mesh, RenderData->materialState);
					}

					mesh->SetVisibility(MeshVisible, true);
				}

				if (UPoseableMeshComponent* depthMesh = GetDepthMeshComponent(RenderData->meshAssetID))
				{
					const bool IsSelfOccluding = (RenderData->visibilityMask & ovrAvatarVisibilityFlag_SelfOccluding) > 0;

					if (MeshVisible && IsSelfOccluding)
					{
						UpdateMeshComponent(*depthMesh, RenderData->localTransform);
					}

					depthMesh->SetVisibility(MeshVisible && IsSelfOccluding, true);
				}
			}
			break;
			case ovrAvatarRenderPartType_SkinnedMeshRenderPBS:
			{
				const ovrAvatarRenderPart_SkinnedMeshRenderPBS* RenderData = ovrAvatarRenderPart_GetSkinnedMeshRenderPBS(RenderPart);
				if (UPoseableMeshComponent* mesh = GetMeshComponent(RenderData->meshAssetID))
				{
					const bool MeshVisible = (VisibilityMask & RenderData->visibilityMask) != 0;
					if (MeshVisible)
					{
						UpdateMeshComponent(*mesh, RenderData->localTransform);
						UpdateSkeleton(*mesh, RenderData->skinnedPose);
						UpdateMaterialPBR(*mesh, *RenderData);
					}

					mesh->SetVisibility(MeshVisible, true);
				}
			}
			break;
			case ovrAvatarRenderPartType_ProjectorRender:
			{
				const ovrAvatarRenderPart_ProjectorRender* RenderData = ovrAvatarRenderPart_GetProjectorRender(RenderPart);
				if (UPoseableMeshComponent* mesh = GetMeshComponent(ProjectorMeshID))
				{
					if (mesh->bVisible && ProjectorMeshComponent.IsValid())
					{
						UpdateMaterial(*ProjectorMeshComponent, RenderData->materialState);

						if (OvrSceneComponent)
						{
							UpdateMaterialProjector(*ProjectorMeshComponent, *RenderData, *OvrSceneComponent);
						}
					}
				}
			}
			break;
			default:
				break;
			}
		}
	}
}

void UOvrAvatar::UpdateTransforms(float DeltaTime)
{
	if (PlayerType != ePlayerType::Local)
		return;

	OvrAvatarHelpers::OvrAvatarIdentity(BodyTransform);

	if (IOculusRiftPlugin::IsAvailable())
	{
		ovrTrackingState TrackingState;
		OvrAvatarHelpers::OvrTrackingStateIdentity(TrackingState);

		if (IOculusRiftPlugin::Get().GetCurrentTrackingState(&TrackingState))
		{
			OvrAvatarHelpers::OvrPoseToAvatarTransform(BodyTransform, TrackingState.HeadPose.ThePose);
			BodyTransform.position.y += PlayerHeightOffset;
		}

		OvrAvatarHelpers::OvrPoseToAvatarTransform(HandInputState[ovrHand_Left].transform, TrackingState.HandPoses[ovrHand_Left].ThePose);
		OvrAvatarHelpers::OvrPoseToAvatarTransform(HandInputState[ovrHand_Right].transform, TrackingState.HandPoses[ovrHand_Right].ThePose);

		HandInputState[ovrHand_Right].transform.position.y += PlayerHeightOffset;
		HandInputState[ovrHand_Left].transform.position.y += PlayerHeightOffset;

		ovrAvatarPose_UpdateBody(Avatar, BodyTransform);
		ovrAvatarPose_UpdateHands(Avatar, HandInputState[ovrHand_Left], HandInputState[ovrHand_Right]);
	}
}

void UOvrAvatar::UpdateVoiceData(float DeltaTime)
{
	if (Microphone)
	{
		const uint32_t NUM_SAMPLES = 48000;
		float Samples[NUM_SAMPLES];

		auto SampleCount = ovr_Microphone_ReadData(Microphone, Samples, NUM_SAMPLES);
		if (SampleCount > 0)
		{
			ovrAvatarPose_UpdateVoiceVisualization(Avatar, (uint32_t)SampleCount, Samples);
		}
	}
}

void UOvrAvatar::RequestAvatar(uint64_t userId)
{
	OnlineUserID = userId;

	ovrAvatar_RequestAvatarSpecification(userId);

	if (PlayerType == ePlayerType::Local)
	{
		Microphone = ovr_Microphone_Create();
		if (Microphone)
		{
			ovr_Microphone_Start(Microphone);
		}
	}
}

void UOvrAvatar::UpdateSkeleton(UPoseableMeshComponent& mesh, const ovrAvatarSkinnedMeshPose& pose)
{
	FTransform LocalBone = FTransform::Identity;
	for (int BoneIndex = 0; BoneIndex < pose.jointCount; BoneIndex++)
	{
		OvrAvatarHelpers::ConvertTransform(pose.jointTransform[BoneIndex], LocalBone);
		mesh.BoneSpaceTransforms[BoneIndex] = LocalBone;
	}
}

USceneComponent* UOvrAvatar::DetachHand(ovrHandType hand)
{
	USceneComponent* handComponent = nullptr;

	if (hand >= ovrHand_Count || AvatarHands[hand].IsValid())
		return handComponent;

	if (auto ScenePtr = RootAvatarComponents.Find(HandNames[hand]))
	{
		if (auto Hand = ScenePtr->Get())
		{
			Hand->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
			RootAvatarComponents.Remove(HandNames[hand]);
			AvatarHands[hand] = Hand;
			handComponent = Hand;
		}
	}

	return handComponent;
}

void UOvrAvatar::ReAttachHand(ovrHandType hand)
{
	if (hand < ovrHand_Count && AvatarHands[hand].IsValid() && !RootAvatarComponents.Find(HandNames[hand]))
	{
		AvatarHands[hand]->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
		AvatarHands[hand]->RegisterComponent();
		RootAvatarComponents.Add(HandNames[hand], AvatarHands[hand]);
		AvatarHands[hand] = nullptr;
	}
}

void UOvrAvatar::SetRightHandPose(ovrAvatarHandGesture pose)
{
	if (!Avatar || pose == ovrAvatarHandGesture_Count)
		return;

	ovrAvatar_SetRightHandGesture(Avatar, pose);
}

void UOvrAvatar::SetLeftHandPose(ovrAvatarHandGesture pose)
{
	if (!Avatar || pose == ovrAvatarHandGesture_Count)
		return;

	ovrAvatar_SetLeftHandGesture(Avatar, pose);
}

void UOvrAvatar::SetCustomGesture(ovrHandType hand, ovrAvatarTransform* joints, uint32_t numJoints)
{
	if (!Avatar)
		return;

	switch (hand)
	{
	case ovrHandType::ovrHand_Left:
		ovrAvatar_SetLeftHandCustomGesture(Avatar, numJoints, joints);
		break;
	case ovrHandType::ovrHand_Right:
		ovrAvatar_SetRightHandCustomGesture(Avatar, numJoints, joints);
		break;
	default:
		break;
	}
}

void UOvrAvatar::SetControllerVisibility(ovrHandType hand, bool visible)
{
	if (!Avatar)
		return;

	switch (hand)
	{
	case ovrHandType::ovrHand_Left:
		ovrAvatar_SetLeftControllerVisibility(Avatar, visible);

		break;
	case ovrHandType::ovrHand_Right:
		ovrAvatar_SetRightControllerVisibility(Avatar, visible);
		break;
	default:
		break;
	}
}

void UOvrAvatar::StartPacketRecording()
{
	if (!Avatar)
		return;

	ovrAvatarPacket_BeginRecording(Avatar);
}

ovrAvatarPacket* UOvrAvatar::EndPacketRecording()
{
	if (!Avatar)
		return nullptr;

	return ovrAvatarPacket_EndRecording(Avatar);
}

void UOvrAvatar::UpdateFromPacket(ovrAvatarPacket* packet, const float time)
{
	if (Avatar && packet && time > 0.f)
	{
		ovrAvatar_UpdatePoseFromPacket(Avatar, packet, time);
	}
}

void UOvrAvatar::UpdateMeshComponent(USceneComponent& mesh, const ovrAvatarTransform& transform)
{
	OvrAvatarHelpers::OvrAvatarTransformToSceneComponent(mesh, transform);
	mesh.SetVisibility(true, true);
}

void UOvrAvatar::UpdateMaterial(UMeshComponent& mesh, const ovrAvatarMaterialState& material)
{
	UMaterialInstanceDynamic* MaterialInstance = Cast<UMaterialInstanceDynamic>(mesh.GetMaterial(0));

	check(MaterialInstance);

	if (auto AlphaTexture = FOvrAvatarManager::Get().FindTexture(material.alphaMaskTextureID))
	{
		MaterialInstance->SetVectorParameterValue(FName("alphaMaskScaleOffset"), OvrAvatarHelpers::OvrAvatarVec4ToLinearColor(material.alphaMaskScaleOffset));
		MaterialInstance->SetTextureParameterValue(FName("alphaMask"), AlphaTexture);
	}

	if (auto NormalTexture = FOvrAvatarManager::Get().FindTexture(material.normalMapTextureID))
	{
		MaterialInstance->SetVectorParameterValue(FName("normalMapScaleOffset"), OvrAvatarHelpers::OvrAvatarVec4ToLinearColor(material.normalMapScaleOffset));
		MaterialInstance->SetTextureParameterValue(FName("normalMap"), NormalTexture);
	}

	if (auto RoughnessTexture = FOvrAvatarManager::Get().FindTexture(material.roughnessMapTextureID))
	{
		MaterialInstance->SetScalarParameterValue(FName("useRoughnessMap"), 1.0f);
		MaterialInstance->SetTextureParameterValue(FName("roughnessMap"), RoughnessTexture);
		MaterialInstance->SetVectorParameterValue(FName("roughnessMapScaleOffset"), OvrAvatarHelpers::OvrAvatarVec4ToLinearColor(material.roughnessMapScaleOffset));
	}
	else
	{
		MaterialInstance->SetScalarParameterValue(FName("useRoughnessMap"), 0.0f);
	}

	MaterialInstance->SetVectorParameterValue(FName("parallaxMapScaleOffset"), OvrAvatarHelpers::OvrAvatarVec4ToLinearColor(material.parallaxMapScaleOffset));
	if (auto ParallaxTexture = FOvrAvatarManager::Get().FindTexture(material.parallaxMapTextureID))
	{
		MaterialInstance->SetTextureParameterValue(FName("parallaxMap"), ParallaxTexture);
	}

	MaterialInstance->SetVectorParameterValue(FName("baseColor"), OvrAvatarHelpers::OvrAvatarVec4ToLinearColor(material.baseColor));
	MaterialInstance->SetScalarParameterValue(FName("baseMaskType"), material.baseMaskType);
	MaterialInstance->SetVectorParameterValue(FName("baseMaskParameters"), OvrAvatarHelpers::OvrAvatarVec4ToLinearColor(material.baseMaskParameters));

	// Converts vector from Oculus to Unreal because of coordinate system difference
	ovrAvatarVector4f baseMaskAxis;
	baseMaskAxis.x = -material.baseMaskAxis.z;
	baseMaskAxis.y = material.baseMaskAxis.x;
	baseMaskAxis.z = material.baseMaskAxis.y;
	baseMaskAxis.w = material.baseMaskAxis.w;
	MaterialInstance->SetVectorParameterValue(FName("baseMaskAxis"), OvrAvatarHelpers::OvrAvatarVec4ToLinearColor(baseMaskAxis));

	for (uint32_t l = 0; l < material.layerCount; ++l)
	{
		FString ParamName;

		ParamName = FString::Printf(TEXT("Layer%u_SamplerMode"), l);
		MaterialInstance->SetScalarParameterValue(FName(*ParamName), material.layers[l].sampleMode);
		ParamName = FString::Printf(TEXT("Layer%u_MaskType"), l);
		MaterialInstance->SetScalarParameterValue(FName(*ParamName), material.layers[l].maskType);
		ParamName = FString::Printf(TEXT("Layer%u_BlendMode"), l);
		MaterialInstance->SetScalarParameterValue(FName(*ParamName), material.layers[l].blendMode);

		ParamName = FString::Printf(TEXT("Layer%u_Color"), l);
		MaterialInstance->SetVectorParameterValue(FName(*ParamName), OvrAvatarHelpers::OvrAvatarVec4ToLinearColor(material.layers[l].layerColor));
		ParamName = FString::Printf(TEXT("Layer%u_SurfaceScaleOffset"), l);
		MaterialInstance->SetVectorParameterValue(FName(*ParamName), OvrAvatarHelpers::OvrAvatarVec4ToLinearColor(material.layers[l].sampleScaleOffset));
		ParamName = FString::Printf(TEXT("Layer%u_SampleParameters"), l);
		MaterialInstance->SetVectorParameterValue(FName(*ParamName), OvrAvatarHelpers::OvrAvatarVec4ToLinearColor(material.layers[l].sampleParameters));

		ParamName = FString::Printf(TEXT("Layer%u_MaskParameters"), l);
		MaterialInstance->SetVectorParameterValue(FName(*ParamName), OvrAvatarHelpers::OvrAvatarVec4ToLinearColor(material.layers[l].maskParameters));
		ParamName = FString::Printf(TEXT("Layer%u_MaskAxis"), l);

		ovrAvatarVector4f layerMaskAxis;
		layerMaskAxis.x = -material.layers[l].maskAxis.z;
		layerMaskAxis.y = material.layers[l].maskAxis.x;
		layerMaskAxis.z = material.layers[l].maskAxis.y;
		layerMaskAxis.w = material.layers[l].maskAxis.w;
		MaterialInstance->SetVectorParameterValue(FName(*ParamName), OvrAvatarHelpers::OvrAvatarVec4ToLinearColor(layerMaskAxis));

		if (auto SampleTexture = FOvrAvatarManager::Get().FindTexture(material.layers[l].sampleTexture))
		{
			ParamName = FString::Printf(TEXT("Layer%u_Surface"), l);
			MaterialInstance->SetTextureParameterValue(FName(*ParamName), SampleTexture);
		}
	}
}

void UOvrAvatar::UpdateMaterialPBR(UPoseableMeshComponent& mesh, const ovrAvatarRenderPart_SkinnedMeshRenderPBS& data)
{
	UMaterialInstanceDynamic* MaterialInstance = Cast<UMaterialInstanceDynamic>(mesh.GetMaterial(0));

	if (auto AlbedoTexture = FOvrAvatarManager::Get().FindTexture(data.albedoTextureAssetID))
	{	
		MaterialInstance->SetTextureParameterValue(FName("AlbedoMap"), AlbedoTexture);
	}

	if (auto SurfaceTexture = FOvrAvatarManager::Get().FindTexture(data.surfaceTextureAssetID))
	{
		MaterialInstance->SetTextureParameterValue(FName("SurfaceMap"), SurfaceTexture);
	}
}

void UOvrAvatar::UpdateMaterialProjector(UPoseableMeshComponent& mesh, const ovrAvatarRenderPart_ProjectorRender& data, const USceneComponent& OvrComponent)
{
	UMaterialInstanceDynamic* MaterialInstance = Cast<UMaterialInstanceDynamic>(mesh.GetMaterial(0));

	FTransform ProjectorLocalTransform;
	FTransform OvrComponentWorld = OvrComponent.GetComponentToWorld();

	OvrAvatarHelpers::ConvertTransform(data.localTransform, ProjectorLocalTransform);
	ProjectorLocalTransform.SetScale3D(100.0f * FVector(data.localTransform.scale.z, data.localTransform.scale.x, data.localTransform.scale.y));

	FTransform ProjWorld;
	FTransform::Multiply(&ProjWorld, &ProjectorLocalTransform, &OvrComponentWorld);

	FMatrix ProjectorBasis = ProjWorld.ToInverseMatrixWithScale();
	FLinearColor row0(ProjectorBasis.M[0][0], ProjectorBasis.M[1][0], ProjectorBasis.M[2][0], ProjectorBasis.M[3][0]);
	FLinearColor row1(ProjectorBasis.M[0][1], ProjectorBasis.M[1][1], ProjectorBasis.M[2][1], ProjectorBasis.M[3][1]);
	FLinearColor row2(ProjectorBasis.M[0][2], ProjectorBasis.M[1][2], ProjectorBasis.M[2][2], ProjectorBasis.M[3][2]);

	MaterialInstance->SetVectorParameterValue(FName("proj_row0"), row0);
	MaterialInstance->SetVectorParameterValue(FName("proj_row1"), row1);
	MaterialInstance->SetVectorParameterValue(FName("proj_row2"), row2);
}

UPoseableMeshComponent* UOvrAvatar::CreateMeshComponent(USceneComponent* parent, ovrAvatarAssetID assetID, const FString& name)
{
	UPoseableMeshComponent* MeshComponent = NewObject<UPoseableMeshComponent>(parent->GetOwner(), *name);
	MeshComponent->AttachToComponent(parent, FAttachmentTransformRules::SnapToTargetIncludingScale);
	MeshComponent->RegisterComponent();

	MeshComponent->bCastDynamicShadow = false;
	MeshComponent->CastShadow = false;
	MeshComponent->bRenderCustomDepth = false;
	MeshComponent->bRenderInMainPass = true;

	AddMeshComponent(assetID, MeshComponent);

	return MeshComponent;
}

UPoseableMeshComponent* UOvrAvatar::CreateDepthMeshComponent(USceneComponent* parent, ovrAvatarAssetID assetID, const FString& name)
{
	UPoseableMeshComponent* MeshComponent = NewObject<UPoseableMeshComponent>(parent->GetOwner(), *name);
	MeshComponent->AttachToComponent(parent, FAttachmentTransformRules::SnapToTargetIncludingScale);
	MeshComponent->RegisterComponent();

	MeshComponent->bCastDynamicShadow = false;
	MeshComponent->CastShadow = false;
	MeshComponent->bRenderCustomDepth = true;
	MeshComponent->bRenderInMainPass = false;

	AddDepthMeshComponent(assetID, MeshComponent);

	return MeshComponent;
}

void UOvrAvatar::LoadMesh(USkeletalMesh* SkeletalMesh, const ovrAvatarMeshAssetData* data)
{
	UE_LOG(LogAvatars, Warning, TEXT("[Avatars] Loaded Mesh."));

	FStaticLODModel* LodModel = new FStaticLODModel();
	SkeletalMesh->GetImportedResource()->LODModels.Add(LodModel);

	new(LodModel->Sections) FSkelMeshSection();
	LodModel->Sections[0].MaterialIndex = 0;
	LodModel->Sections[0].BaseIndex = 0;
	LodModel->Sections[0].NumTriangles = 0;

	SkeletalMesh->LODInfo.Add(FSkeletalMeshLODInfo());
	FSkeletalMeshLODInfo& LodInfo = SkeletalMesh->LODInfo[0];

	LodInfo.ScreenSize = 0.3f;
	LodInfo.LODHysteresis = 0.2f;
	LodInfo.TriangleSortSettings.Add(FTriangleSortSettings());
	LodInfo.LODMaterialMap.Add(0);

	SkeletalMesh->Materials.Add(UMaterial::GetDefaultMaterial(MD_Surface));
	//SkeletalMesh->RefSkeleton.Allocate(data->skinnedBindPose.jointCount);

	SkeletalMesh->bUseFullPrecisionUVs = true;
	SkeletalMesh->bHasBeenSimplified = false;
	SkeletalMesh->bHasVertexColors = false;

	for (int BoneIndex = 0; BoneIndex < data->skinnedBindPose.jointCount; BoneIndex++)
	{
		LodModel->RequiredBones.Add(BoneIndex);
		LodModel->ActiveBoneIndices.Add(BoneIndex);
		LodModel->Sections[0].BoneMap.Add(BoneIndex);

		FString BoneString = data->skinnedBindPose.jointNames[BoneIndex];
		FName BoneName = FName(*BoneString);

		FTransform Transform = FTransform::Identity;
		OvrAvatarHelpers::ConvertTransform(data->skinnedBindPose.jointTransform[BoneIndex], Transform);

		FReferenceSkeletonModifier Modifier = FReferenceSkeletonModifier(SkeletalMesh->RefSkeleton, nullptr);
		Modifier.Add(FMeshBoneInfo(BoneName, BoneString, data->skinnedBindPose.jointParents[BoneIndex]), Transform);
	}

	check(data->indexCount % 3 == 0);
	check(data->vertexCount > 0);

	auto& MeshSection = LodModel->Sections[0];
	MeshSection.BaseIndex = 0;
	MeshSection.NumTriangles = data->indexCount / 3;
	MeshSection.BaseVertexIndex = 0;
	MeshSection.NumVertices = data->vertexCount;
	MeshSection.MaxBoneInfluences = 4;

	MeshSection.SoftVertices.SetNumUninitialized(data->vertexCount);

	const ovrAvatarMeshVertex* SourceVertex = data->vertexBuffer;
	const uint32_t NumBlendWeights = 4;

	FSoftSkinVertex* DestVertex = MeshSection.SoftVertices.GetData();
	FBox BoundBox = FBox();
	BoundBox.Init();

	for (uint32_t VertIndex = 0; VertIndex < data->vertexCount; VertIndex++, SourceVertex++, DestVertex++)
	{
		DestVertex->Position = 100.0f * FVector(-SourceVertex->z, SourceVertex->x, SourceVertex->y);

		BoundBox += DestVertex->Position;

		FVector n = FVector(-SourceVertex->nz, SourceVertex->nx, SourceVertex->ny);
		FVector t = FVector(-SourceVertex->tz, SourceVertex->tx, SourceVertex->ty);
		FVector bt = FVector::CrossProduct(t, n) * FMath::Sign(SourceVertex->tw);
		DestVertex->TangentX = FPackedNormal(t);
		DestVertex->TangentY = FPackedNormal(bt);
		DestVertex->TangentZ = FPackedNormal(n);
		DestVertex->UVs[0] = FVector2D(SourceVertex->u, SourceVertex->v);

		for (uint32_t BlendIndex = 0; BlendIndex < MAX_TOTAL_INFLUENCES; BlendIndex++)
		{
			DestVertex->InfluenceWeights[BlendIndex] = BlendIndex < NumBlendWeights ? (uint8_t)(255.0f*SourceVertex->blendWeights[BlendIndex]) : 0;
			DestVertex->InfluenceBones[BlendIndex] = BlendIndex < NumBlendWeights ? SourceVertex->blendIndices[BlendIndex] : 0;
		}
	}

	LodModel->MultiSizeIndexContainer.CreateIndexBuffer(sizeof(uint16_t));
	LodModel->NumVertices = data->vertexCount;
	LodModel->NumTexCoords = 1;

	for (uint32_t index = 0; index < data->indexCount; index++)
	{
		LodModel->MultiSizeIndexContainer.GetIndexBuffer()->AddItem(data->indexBuffer[index]);
	}

	FBoxSphereBounds Bounds(BoundBox);
	Bounds = Bounds.ExpandBy(100000.0f);
	SkeletalMesh->SetImportedBounds(Bounds);

	const uint32 vert_flags = FStaticLODModel::EVertexFlags::None | FStaticLODModel::EVertexFlags::UseFullPrecisionUVs;
	LodModel->BuildVertexBuffers(vert_flags);

	SkeletalMesh->Skeleton = NewObject<USkeleton>();
	SkeletalMesh->Skeleton->MergeAllBonesToBoneTree(SkeletalMesh);
	SkeletalMesh->PostLoad();
}

bool gLogSDKTransforms = false;
void UOvrAvatar::DebugLogAvatarSDKTransforms(const FString& wrapper)
{
	if (!Avatar || !gLogSDKTransforms)
		return;

	UE_LOG(LogAvatars, Warning, TEXT("\n[Avatars] -------------------------- %s ----------------------------"), *wrapper);

	const uint32_t ComponentCount = ovrAvatarComponent_Count(Avatar);

	FTransform Logger = FTransform::Identity;

	for (uint32_t CompIndex = 0; CompIndex < ComponentCount; ++CompIndex)
	{
		const ovrAvatarComponent* AvatarComponent = ovrAvatarComponent_Get(Avatar, CompIndex);

		OvrAvatarHelpers::ConvertTransform(AvatarComponent->transform, Logger);
		Logger.DebugPrint();

		for (uint32_t RenderIndex = 0; RenderIndex < AvatarComponent->renderPartCount; ++RenderIndex)
		{
			const ovrAvatarRenderPart* RenderPart = AvatarComponent->renderParts[RenderIndex];

			switch (ovrAvatarRenderPart_GetType(RenderPart))
			{
			case ovrAvatarRenderPartType_SkinnedMeshRender:
			{
				const ovrAvatarRenderPart_SkinnedMeshRender* RenderData = ovrAvatarRenderPart_GetSkinnedMeshRender(RenderPart);
				OvrAvatarHelpers::ConvertTransform(RenderData->localTransform, Logger);
				Logger.DebugPrint();
			}
			break;
			case ovrAvatarRenderPartType_SkinnedMeshRenderPBS:
			{
				const ovrAvatarRenderPart_SkinnedMeshRenderPBS* RenderData = ovrAvatarRenderPart_GetSkinnedMeshRenderPBS(RenderPart);
				OvrAvatarHelpers::ConvertTransform(RenderData->localTransform, Logger);
				Logger.DebugPrint();
			}
			break;
			case ovrAvatarRenderPartType_ProjectorRender:
			default:
				break;
			}
		}
	}

	UE_LOG(LogAvatars, Display, TEXT("\n[Avatars] -----------------------------------------------------------------------------"));
}

void UOvrAvatar::DebugLogMaterialData(const ovrAvatarMaterialState& material, const FString& name)
{
	UE_LOG(LogAvatars, Display, TEXT("[Avatars] --------------------------Material For - %s ----------------------------"), *name);
	UE_LOG(LogAvatars, Display, TEXT("[Avatars] Alpha Texture %llu"), material.alphaMaskTextureID);
	UE_LOG(LogAvatars, Display, TEXT("[Avatars] Normal Map %llu"), material.normalMapTextureID);
	UE_LOG(LogAvatars, Display, TEXT("[Avatars] Roughenss Map %llu"), material.roughnessMapTextureID);
	UE_LOG(LogAvatars, Display, TEXT("[Avatars] Parallax Map %llu"), material.parallaxMapTextureID);
	UE_LOG(LogAvatars, Display, TEXT("[Avatars] baseMaskType %s"), *MaskTypeToString(material.baseMaskType));

	for (uint32_t l = 0; l < material.layerCount; ++l)
	{
		UE_LOG(LogAvatars, Display, TEXT("Layer %u - SampleMode - %s"), l, *SampleModeToString(material.layers[l].sampleMode));
		UE_LOG(LogAvatars, Display, TEXT("Layer %u - MaskType - %s"), l, *MaskTypeToString(material.layers[l].maskType));
		UE_LOG(LogAvatars, Display, TEXT("Layer %u - BlendMode - %s"), l, *BlendModeToString(material.layers[l].blendMode));
		UE_LOG(LogAvatars, Display, TEXT("Layer %u - Texture - %llu"), l, material.layers[l].sampleTexture);
	}

	UE_LOG(LogAvatars, Display, TEXT("\n[Avatars] -----------------------------------------------------------------------------"));
}

TArray<TWeakObjectPtr<UPoseableMeshComponent>> UOvrAvatar::ConvertMeshComponents()
{
	//Map Size
	int32 Count = MeshComponents.Num();
	UE_LOG(LogTemp, Warning, TEXT("Number of mesh components is %d"), Count);
	//Convert Map
	TArray<ovrAvatarAssetID> MeshComponentsKeys;
	TArray<TWeakObjectPtr<UPoseableMeshComponent>> MeshComponentsValues;
	MeshComponentsKeys.Add(999);
	MeshComponentsKeys.Add(123);
	MeshComponents.GenerateKeyArray(MeshComponentsKeys);
	MeshComponents.GenerateValueArray(MeshComponentsValues);
	return MeshComponentsValues;
}

#define CUSTOM_ENTRY(entry, hand, field, invert) \
		void UOvrAvatar::##entry##_Value(float value)  { HandInputState[hand].field =  invert ? -value : value; }
CUSTOM_AXIS_TUPLE
#undef CUSTOM_ENTRY


#define INPUT_ENTRY(entry, hand, flag) \
	void UOvrAvatar::##entry##_Pressed()  { HandInputState[hand].buttonMask |= flag; }\
	void UOvrAvatar::##entry##_Released() { HandInputState[hand].buttonMask &= ~flag; }
INPUT_COMMAND_TUPLE
#undef INPUT_ENTRY

#define AXIS_ENTRY(entry, hand, flag) \
	void UOvrAvatar::##entry##_Value( float value) {( fabs( value ) > 0.f ) ? HandInputState[hand].touchMask |= flag :  HandInputState[hand].touchMask &= ~flag;}
AXIS_INPUT_TUPLE
#undef AXIS_ENTRY


