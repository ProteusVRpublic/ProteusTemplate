// Fill out your copyright notice in the Description page of Project Settings.

#include "ProteusOvrAvatar.h"
#include "ProteusOvrAvatarManager.h"
#include "ProteusOvrAvatarHelpers.h"
#include "Components/PoseableMeshComponent.h"
#include "Rendering/SkeletalMeshLODRenderData.h"
#include "Rendering/SkeletalMeshRenderData.h"
#include "Rendering/SkeletalMeshLODModel.h"
#include "Rendering/SkeletalMeshModel.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Animation/Skeleton.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/Skeleton.h"
#include "Model.h"
#include "Animation/MorphTarget.h"

float DebugLineScale = 100.f;
bool DrawDebug = false;

FString UProteusOvrAvatar::HandNames[HandType_Count] = { FString("hand_left"), FString("hand_right") };
FString UProteusOvrAvatar::BodyName = FString("body");

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

FColor UProteusOvrAvatar::GetColorFromVertex(const ovrAvatarMeshVertex& vertex)
{
	return FColor::Black;
}

FColor UProteusOvrAvatar::GetColorFromVertex(const ovrAvatarMeshVertexV2& vertex)
{
	FLinearColor LinearColor(vertex.r, vertex.g, vertex.b, vertex.a);
	return LinearColor.ToFColor(false);
}

UProteusOvrAvatar::UProteusOvrAvatar()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UProteusOvrAvatar::BeginPlay()
{
	Super::BeginPlay();

	ProteusOvrAvatarHelpers::OvrAvatarHandISZero(HandInputState[HandType_Left]);
	ProteusOvrAvatarHelpers::OvrAvatarHandISZero(HandInputState[HandType_Right]);
	HandInputState[HandType_Left].isActive = true;
	HandInputState[HandType_Right].isActive = true;
	AvatarHands[HandType_Left] = nullptr;
	AvatarHands[HandType_Right] = nullptr;

	ControllerType = GetControllerTypeByHardware();
}

void UProteusOvrAvatar::BeginDestroy()
{
	Super::BeginDestroy();

	UE_LOG(LogAvatars, Display, TEXT("[Avatars] AOvrAvatar::BeginDestroy()"));

	if (Avatar)
	{
		ovrAvatar_Destroy(Avatar);
		Avatar = nullptr;
	}

}

void UProteusOvrAvatar::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!Avatar || AssetIds.Num() != 0)
		return;

	UpdateSDK(DeltaTime);
	UpdatePostSDK();
	UpdateV2VoiceOffsetParams();
}

void UProteusOvrAvatar::AddMeshComponent(ovrAvatarAssetID id, UPoseableMeshComponent* mesh)
{
	if (!GetMeshComponent(id))
	{
		MeshComponents.Add(id, mesh);
	}
}

void UProteusOvrAvatar::AddDepthMeshComponent(ovrAvatarAssetID id, UPoseableMeshComponent* mesh)
{
	if (!GetDepthMeshComponent(id))
	{
		DepthMeshComponents.Add(id, mesh);
	}
}

void UProteusOvrAvatar::HandleAvatarSpecification(const ovrAvatarMessage_AvatarSpecification* message)
{
	if (Avatar || !message->oculusUserID || OnlineUserID != message->oculusUserID)
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

		const bool IsBodyComponent = name.Equals(BodyName);

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

				if (RenderIndex == 0 && IsBodyComponent)
				{
					BodyMeshID = RenderData->meshAssetID;
				}

				if (UseDepthMeshes)
				{
					UPoseableMeshComponent* DepthMesh = CreateDepthMeshComponent(BaseComponent, RenderData->meshAssetID, MeshName + TEXT("_Depth"));
					DepthMesh->SetMasterPoseComponent(MeshComponent);
				}

				const auto& material = RenderData->materialState;
				const bool UseNormalMap = material.normalMapTextureID > 0;
				bool UseParallax = material.parallaxMapTextureID > 0;

				for (uint32_t l = 0; l < material.layerCount && !UseParallax; ++l)
				{
					UseParallax = material.layers[l].sampleMode == ovrAvatarMaterialLayerSampleMode_Parallax;
				}

				FString MaterialFolder = TEXT("");
				FString AlphaFolder = material.alphaMaskTextureID > 0 ? TEXT("On/") : TEXT("Off/");
				
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
				FString sMaterialPath = TEXT("/ProteusAvatars/Materials/v1/Inst/") + AlphaFolder + MaterialFolder + sMaterialName + TEXT(".") + sMaterialName;

				auto Material = LoadObject<UMaterialInstance>(nullptr, *sMaterialPath, nullptr, LOAD_None, nullptr);
				MeshComponent->SetMaterial(0, UMaterialInstanceDynamic::Create(Material, GetTransientPackage()));
			}
			break;
			case ovrAvatarRenderPartType_SkinnedMeshRenderPBS:
			{
				const ovrAvatarRenderPart_SkinnedMeshRenderPBS* RenderData = ovrAvatarRenderPart_GetSkinnedMeshRenderPBS(RenderPart);
				FString MeshName = name + FString::Printf(TEXT("_%u"), RenderIndex);
				auto MeshComponent = CreateMeshComponent(BaseComponent, RenderData->meshAssetID, MeshName);

				auto Material = (UMaterialInterface*)StaticLoadObject(UMaterial::StaticClass(), NULL, TEXT("/ProteusAvatars/Materials/OculusAvatarsPBR.OculusAvatarsPBR"), NULL, LOAD_None, NULL);
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
				FString sMaterialPath = TEXT("/ProteusAvatars/Materials/OculusAvatar8Layers/Instances/") + sMaterialName + TEXT(".") + sMaterialName;

				auto Material = LoadObject<UMaterialInstance>(nullptr, *sMaterialPath, nullptr, LOAD_None, nullptr);
				MeshComponent->SetMaterial(0, UMaterialInstanceDynamic::Create(Material, GetTransientPackage()));

				ProjectorMeshComponent = MeshComponent;
			}
			break;
			case ovrAvatarRenderPartType_SkinnedMeshRenderPBS_V2:
			{
				const ovrAvatarRenderPart_SkinnedMeshRenderPBS_V2* RenderData = ovrAvatarRenderPart_GetSkinnedMeshRenderPBSV2(RenderPart);

				if (RenderIndex == 0 && IsBodyComponent)
				{
					BodyMeshID = RenderData->meshAssetID;
				}

				FString MeshName = name + FString::Printf(TEXT("_%u"), RenderIndex);

				UPoseableMeshComponent* MeshComponent = CreateMeshComponent(BaseComponent, RenderData->meshAssetID, MeshName);
				const auto MaterialString = GetPBRV2MainMaterialString(false);
				auto Material = (UMaterialInterface*)StaticLoadObject(UMaterial::StaticClass(), NULL, *MaterialString);
				MeshComponent->SetMaterial(0, UMaterialInstanceDynamic::Create(Material, GetTransientPackage()));
				MeshComponent->SetVisibility(false, true);

				if (UseDepthMeshes)
				{
					UPoseableMeshComponent* DepthMesh = CreateDepthMeshComponent(BaseComponent, RenderData->meshAssetID, MeshName + TEXT("_Depth"));
					DepthMesh->SetMasterPoseComponent(MeshComponent);
					auto DepthMaterial = (UMaterialInterface*)StaticLoadObject(UMaterial::StaticClass(), NULL, TEXT("/ProteusAvatars/Materials/AvatarsPBR_2/OculusAvatars_PBRV2_2_Depth"));
					DepthMesh->SetMaterial(0, UMaterialInstanceDynamic::Create(DepthMaterial, GetTransientPackage()));
					DepthMesh->SetVisibility(false, true);
				}

				// Cache the Normal Map ID for appropriate tagging on Load.
				FProteusOvrAvatarManager::Get().CacheNormalMapID(RenderData->materialState.normalTextureID);
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
		UE_LOG(LogAvatars, Display, TEXT("[Avatars] - Loading Asset ID: %llu"), Asset);

		ovrAvatarAsset_BeginLoading(Asset);
	}
}

void UProteusOvrAvatar::HandleAssetLoaded(const ovrAvatarMessage_AssetLoaded* message)
{
	const ovrAvatarAssetType assetType = ovrAvatarAsset_GetType(message->asset);
	const bool OurCombinedMeshLoaded = assetType == ovrAvatarAssetType_CombinedMesh && ovrAvatarAsset_GetAvatar(message->asset) == Avatar && Avatar != nullptr;

	if (OurCombinedMeshLoaded)
	{
		UE_LOG(LogAvatars, Display, TEXT("[Avatars] ovrAvatarAssetType_CombinedMesh Loaded"));

		auto BaseComponent = RootAvatarComponents.Find(BodyName);
		if (BaseComponent && BaseComponent->Get())
		{
			BodyMeshID = message->assetID;

			USkeletalMesh * mesh = NewObject<USkeletalMesh>(GetTransientPackage(), NAME_None, RF_Transient);

			UE_LOG(LogAvatars, Display, TEXT("Loading Combined Mesh"));
			LoadMesh<ovrAvatarMeshAssetDataV2, ovrAvatarMeshVertexV2>(mesh, ovrAvatarAsset_GetCombinedMeshData(message->asset));

			FString MeshName = BodyName + FString::Printf(TEXT("_Combined"));
			UPoseableMeshComponent* MeshComponent = CreateMeshComponent(BaseComponent->Get(), message->assetID, MeshName);
			const auto CombinedMaterialString = GetPBRV2MainMaterialString(true);
			auto Material = (UMaterialInterface*)StaticLoadObject(UMaterial::StaticClass(), NULL, *CombinedMaterialString);
			MeshComponent->SetMaterial(0, UMaterialInstanceDynamic::Create(Material, GetTransientPackage()));
			MeshComponent->SetSkeletalMesh(mesh);
			MeshComponent->RecreateRenderState_Concurrent();
			MeshComponent->SetVisibility(false, true);

			if (UseDepthMeshes)
			{
				UPoseableMeshComponent* DepthMesh = CreateDepthMeshComponent(BaseComponent->Get(), message->assetID, MeshName + TEXT("_Depth"));
				auto DepthMaterial = (UMaterialInterface*)StaticLoadObject(UMaterial::StaticClass(), NULL, TEXT("/ProteusAvatars/Materials/AvatarsPBR_2/OculusAvatars_PBRV2_2_Depth"));
				DepthMesh->SetMaterial(0, UMaterialInstanceDynamic::Create(DepthMaterial, GetTransientPackage()));
				DepthMesh->SetSkeletalMesh(mesh);
				DepthMesh->RecreateRenderState_Concurrent();
				DepthMesh->SetMasterPoseComponent(MeshComponent);
				DepthMesh->SetVisibility(false, true);
			}

			uint32_t MeshCount = 0;
			const auto MeshIds = ovrAvatarAsset_GetCombinedMeshIDs(message->asset, &MeshCount);
			for (uint32_t meshIndex = 0; meshIndex < MeshCount; meshIndex++)
			{
				// TODO: Check if they have a mesh assigned and destroy it
				// Can happen if other avatars load the same mesh asset we are waiting on (unlikely as all will be mesh combining, but who knows)
				RemoveMeshComponent(MeshIds[meshIndex]);
				RemoveDepthMeshComponent(MeshIds[meshIndex]);

				AssetIds.Remove(MeshIds[meshIndex]);
			}
		}
	}
	else if (auto Asset = AssetIds.Find(message->assetID))
	{
		AssetIds.Remove(*Asset);

		switch (assetType)
		{
		case ovrAvatarAssetType_Mesh:
		{
			if (UPoseableMeshComponent* MeshComp = GetMeshComponent(message->assetID))
			{
				USkeletalMesh* mesh = NewObject<USkeletalMesh>(GetTransientPackage(), NAME_None, RF_Transient);
				LoadMesh<ovrAvatarMeshAssetData, ovrAvatarMeshVertex>(mesh, ovrAvatarAsset_GetMeshData(message->asset));
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
			if (!FProteusOvrAvatarManager::Get().FindTexture(message->assetID))
			{
				FProteusOvrAvatarManager::Get().LoadTexture(message->assetID, ovrAvatarAsset_GetTextureData(message->asset));
			}
			break;
		default:
			UE_LOG(LogAvatars, Warning, TEXT("[Avatars] Unknown Asset Type"));
			break;
		}
	}

	if (Avatar && AssetIds.Num() == 0 && !AreMaterialsInitialized)
	{
		AreMaterialsInitialized = true;
		InitializeMaterials();
	}
}

UPoseableMeshComponent* UProteusOvrAvatar::GetMeshComponent(ovrAvatarAssetID id) const
{
	UPoseableMeshComponent* Return = nullptr;

	auto MeshComponent = MeshComponents.Find(id);
	if (MeshComponent && MeshComponent->IsValid())
	{
		Return = MeshComponent->Get();
	}

	return Return;
}

void UProteusOvrAvatar::RemoveMeshComponent(ovrAvatarAssetID id)
{
	auto MeshComponent = MeshComponents.Find(id);
	if (MeshComponent && MeshComponent->IsValid())
	{
		MeshComponents.Remove(id);
		MeshComponent->Get()->DestroyComponent(true);
	}
}

UPoseableMeshComponent* UProteusOvrAvatar::GetDepthMeshComponent(ovrAvatarAssetID id) const
{
	UPoseableMeshComponent* Return = nullptr;

	auto MeshComponent = DepthMeshComponents.Find(id);
	if (MeshComponent && MeshComponent->IsValid())
	{
		Return = MeshComponent->Get();
	}

	return Return;
}

void UProteusOvrAvatar::RemoveDepthMeshComponent(ovrAvatarAssetID id)
{
	auto MeshComponent = DepthMeshComponents.Find(id);
	if (MeshComponent && MeshComponent->IsValid())
	{
		DepthMeshComponents.Remove(id);
		MeshComponent->Get()->DestroyComponent(true);
	}
}


void UProteusOvrAvatar::DebugDrawBoneTransforms()
{
	for (auto mesh : MeshComponents)
	{
		if (mesh.Value.IsValid())
		{
			auto skeletalMesh = mesh.Value.Get();
			const auto BoneCount = skeletalMesh->GetNumBones();
			auto BoneTransform = FTransform::Identity;
			for (auto index = 0; index < BoneCount; index++)
			{
				BoneTransform = skeletalMesh->GetBoneTransform(index);
				ProteusOvrAvatarHelpers::DebugDrawCoords(GetWorld(), BoneTransform);
			}
		}
	}
}

void UProteusOvrAvatar::DebugDrawSceneComponents()
{
	DebugLineScale = 200.f;
	FTransform world_trans = GetOwner()->GetRootComponent()->GetComponentTransform();
	ProteusOvrAvatarHelpers::DebugDrawCoords(GetWorld(), world_trans);

	DebugLineScale = 100.f;
	for (auto comp : RootAvatarComponents)
	{
		if (comp.Value.IsValid())
		{
			world_trans = comp.Value->GetComponentTransform();
			ProteusOvrAvatarHelpers::DebugDrawCoords(GetWorld(), world_trans);
		}
	}

	DebugLineScale = 50.f;
	for (auto mesh : MeshComponents)
	{
		if (mesh.Value.IsValid())
		{
			world_trans = mesh.Value->GetComponentTransform();
			ProteusOvrAvatarHelpers::DebugDrawCoords(GetWorld(), world_trans);
		}
	}
}

void UProteusOvrAvatar::UpdateSDK(float DeltaTime)
{
	UpdateTransforms(DeltaTime);
	ovrAvatarPose_Finalize(Avatar, DeltaTime);
}

void UProteusOvrAvatar::UpdatePostSDK()
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
				ProteusOvrAvatarHelpers::OvrAvatarTransformToSceneComponent(*OvrSceneComponent, OvrComponent->transform);
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
					}

					mesh->SetVisibility(MeshVisible, true);
				}

				if (UPoseableMeshComponent* depthMesh = GetDepthMeshComponent(RenderData->meshAssetID))
				{
					const bool IsSelfOccluding = (RenderData->visibilityMask & ovrAvatarVisibilityFlag_SelfOccluding) > 0;

					if (MeshVisible && IsSelfOccluding)
					{
						UpdateMeshComponent(*depthMesh, RenderData->localTransform);
						///////////////////////////// ONLY FOR UNREAL 4.21 depthMesh->MarkRefreshTransformDirty();
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
			case ovrAvatarRenderPartType_SkinnedMeshRenderPBS_V2:
			{
				const ovrAvatarRenderPart_SkinnedMeshRenderPBS_V2* RenderData = ovrAvatarRenderPart_GetSkinnedMeshRenderPBSV2(RenderPart);
				const bool MeshVisible = (VisibilityMask & RenderData->visibilityMask) != 0;

				if (UPoseableMeshComponent* mesh = GetMeshComponent(RenderData->meshAssetID))
				{
					if (MeshVisible)
					{
						UpdateMeshComponent(*mesh, RenderData->localTransform);
						UpdateSkeleton(*mesh, RenderData->skinnedPose);
					}

					mesh->SetVisibility(MeshVisible, true);
					//mesh->bOwnerNoSee = true;
				}

				if (UPoseableMeshComponent* depthMesh = GetDepthMeshComponent(RenderData->meshAssetID))
				{
					const bool IsSelfOccluding = (RenderData->visibilityMask & ovrAvatarVisibilityFlag_SelfOccluding) > 0;

					if (MeshVisible && IsSelfOccluding)
					{
						UpdateMeshComponent(*depthMesh, RenderData->localTransform);
						///////////////////////////// ONLY FOR UNREAL 4.21 depthMesh->MarkRefreshTransformDirty();
					}

					depthMesh->SetVisibility(MeshVisible && IsSelfOccluding, true);
				}
			}
			break;
			default:
				break;
			}
		}
	}
}

void UProteusOvrAvatar::UpdateTransforms(float DeltaTime)
{
	if (PlayerType != ePlayerType::Local)
		return;

	if (!FProteusOvrAvatarManager::Get().IsOVRPluginValid())
		return;

	ProteusOvrAvatarHelpers::OvrAvatarIdentity(BodyTransform);

	// Head
	{
		ovrpPoseStatef ovrPose;
		ovrp_GetNodePoseState3(ovrpStep_Render, OVRP_CURRENT_FRAMEINDEX, ovrpNode_Head, &ovrPose);


		ProteusOvrAvatarHelpers::OvrPoseToAvatarTransform(BodyTransform, ovrPose.Pose);
		BodyTransform.position.y += PlayerHeightOffset;
	}

	ovrpResult result = ovrpFailure_NotInitialized;
	ovrpController ControllerMask = ovrpController_None;
	ovrpController ActiveController = ovrpController_None;
	result = ovrp_GetConnectedControllers2(&ControllerMask);

	if (result != ovrpSuccess)
	{
		UE_LOG(LogAvatars, Display, TEXT("ovrp_GetConnectedControllers2 failed %d"), result);
	}

	result = ovrp_GetActiveController2(&ActiveController);

	if (result != ovrpSuccess)
	{
		UE_LOG(LogAvatars, Display, TEXT("ovrp_GetActiveController2 failed %d"), result);
	}

	// Left hand
	{
		ovrpControllerState4 controllerState;
		ovrpController LeftControllerType = ovrpController_None;

		if (ControllerMask & ovrpController_LTouch)
		{
			LeftControllerType = ovrpController_LTouch;
		}
		else if (ControllerMask & ovrpController_LTrackedRemote)
		{
			LeftControllerType = ovrpController_LTrackedRemote;
		}

		ovrAvatarHandInputState& handInputState = HandInputState[HandType_Left];
		handInputState.isActive = false;

		if (LeftControllerType != ovrpController_None)
		{
			ovrp_GetControllerState4(LeftControllerType, &controllerState);

			ovrpPoseStatef ovrPose;
			ovrp_GetNodePoseState3(ovrpStep_Render, OVRP_CURRENT_FRAMEINDEX, ovrpNode_HandLeft, &ovrPose);

			ProteusOvrAvatarHelpers::OvrPoseToAvatarTransform(handInputState.transform, ovrPose.Pose);

			handInputState.isActive = true;
			handInputState.indexTrigger = controllerState.IndexTrigger[ovrpHand_Left];
			handInputState.handTrigger = controllerState.HandTrigger[ovrpHand_Left];
			handInputState.joystickX = controllerState.Thumbstick[ovrpHand_Left].x;
			handInputState.joystickY = controllerState.Thumbstick[ovrpHand_Left].y;

			ProteusOvrAvatarHelpers::OvrAvatarParseButtonsAndTouches(controllerState, ovrpHand_Left, handInputState);
		}
	}
	// Right hand
	{
		ovrpControllerState4 controllerState;
		ovrpController RightControllerType = ovrpController_None;

		if (ControllerMask & ovrpController_RTouch)
		{
			RightControllerType = ovrpController_RTouch;
		}
		else if (ControllerMask & ovrpController_RTrackedRemote)
		{
			RightControllerType = ovrpController_RTrackedRemote;
		}

		ovrAvatarHandInputState& handInputState = HandInputState[ovrpHand_Right];
		handInputState.isActive = false;

		if (RightControllerType != ovrpController_None)
		{
			ovrp_GetControllerState4(RightControllerType, &controllerState);

			ovrpPoseStatef ovrPose;
			ovrp_GetNodePoseState3(ovrpStep_Render, OVRP_CURRENT_FRAMEINDEX, ovrpNode_HandRight, &ovrPose);

			ProteusOvrAvatarHelpers::OvrPoseToAvatarTransform(handInputState.transform, ovrPose.Pose);

			handInputState.isActive = true;
			handInputState.indexTrigger = controllerState.IndexTrigger[ovrpHand_Right];
			handInputState.handTrigger = controllerState.HandTrigger[ovrpHand_Right];
			handInputState.joystickX = controllerState.Thumbstick[ovrpHand_Right].x;
			handInputState.joystickY = controllerState.Thumbstick[ovrpHand_Right].y;

			ProteusOvrAvatarHelpers::OvrAvatarParseButtonsAndTouches(controllerState, ovrpHand_Right, handInputState);
		}
	}

	HandInputState[HandType_Right].transform.position.y += PlayerHeightOffset;
	HandInputState[HandType_Left].transform.position.y += PlayerHeightOffset;

	ovrAvatarPose_UpdateBody(Avatar, BodyTransform);

	switch (ControllerType)
	{
	case ovrAvatarControllerType_Malibu:
	case ovrAvatarControllerType_Go:
		ovrAvatarPose_Update3DofHands(Avatar, &HandInputState[HandType_Left], &HandInputState[HandType_Right], ControllerType);
		break;
	case ovrAvatarControllerType_Touch:
		ovrAvatarPose_UpdateHands(Avatar, HandInputState[HandType_Left], HandInputState[HandType_Right]);
		break;
	default:
		break;
	}
}

void UProteusOvrAvatar::RequestAvatar(uint64_t userId, ovrAvatarAssetLevelOfDetail lod, bool useCombinedBodyMesh)
{
	UE_LOG(LogAvatars, Display, TEXT("[Avatars] RequestAvatar %llu, %d, %d"), userId, lod, useCombinedBodyMesh);

	OnlineUserID = userId;
	LevelOfDetail = lod;

	/////////////////////////////REMOVED IN 1.31
	if (LookAndFeel != ovrAvatarLookAndFeelVersion_Two && useCombinedBodyMesh)
	{
		UE_LOG(LogAvatars, Display, TEXT("[Avatars] RequestAvatar: Combined Body Mesh only compatible with ovrAvatarLookAndFeelVersion_Two"));
	}

	UseCombinedBodyMesh = LookAndFeel == ovrAvatarLookAndFeelVersion_Two && useCombinedBodyMesh;
	////////////////////////////////////////////
	auto requestSpec = ovrAvatarSpecificationRequest_Create(userId);
	ovrAvatarSpecificationRequest_SetLookAndFeelVersion(requestSpec, LookAndFeel);
	/////////////////////////////REMOVED IN 1.31
	ovrAvatarSpecificationRequest_SetLevelOfDetail(requestSpec, LevelOfDetail);
	///////////////////////////////////////////

	ovrAvatarSpecificationRequest_SetCombineMeshes(requestSpec, UseCombinedBodyMesh);

	ovrAvatar_RequestAvatarSpecificationFromSpecRequest(requestSpec);
	ovrAvatarSpecificationRequest_Destroy(requestSpec);
}

void UProteusOvrAvatar::UpdateSkeleton(UPoseableMeshComponent& mesh, const ovrAvatarSkinnedMeshPose& pose)
{
	FTransform LocalBone = FTransform::Identity;
	for (uint32 BoneIndex = 0; BoneIndex < pose.jointCount; BoneIndex++)
	{
		ProteusOvrAvatarHelpers::ConvertTransform(pose.jointTransform[BoneIndex], LocalBone);
		mesh.BoneSpaceTransforms[BoneIndex] = LocalBone;
	}
	///////////////////////////// ONLY FOR UNREAL 4.21 mesh.MarkRefreshTransformDirty();
}

USceneComponent* UProteusOvrAvatar::DetachHand(HandType hand)
{
	USceneComponent* handComponent = nullptr;

	if (hand >= HandType_Count || AvatarHands[hand].IsValid())
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

void UProteusOvrAvatar::ReAttachHand(HandType hand)
{
	if (hand < HandType_Count && AvatarHands[hand].IsValid() && !RootAvatarComponents.Find(HandNames[hand]))
	{
		AvatarHands[hand]->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::SnapToTargetIncludingScale);
		AvatarHands[hand]->RegisterComponent();
		RootAvatarComponents.Add(HandNames[hand], AvatarHands[hand]);
		AvatarHands[hand] = nullptr;
	}
}

void UProteusOvrAvatar::SetRightHandPose(ovrAvatarHandGesture pose)
{
	if (!Avatar || pose == ovrAvatarHandGesture_Count)
		return;

	ovrAvatar_SetRightHandGesture(Avatar, pose);
}

void UProteusOvrAvatar::SetLeftHandPose(ovrAvatarHandGesture pose)
{
	if (!Avatar || pose == ovrAvatarHandGesture_Count)
		return;

	ovrAvatar_SetLeftHandGesture(Avatar, pose);
}

void UProteusOvrAvatar::SetCustomGesture(HandType hand, ovrAvatarTransform* joints, uint32_t numJoints)
{
	if (!Avatar)
		return;

	switch (hand)
	{
	case HandType::HandType_Left:
		ovrAvatar_SetLeftHandCustomGesture(Avatar, numJoints, joints);
		break;
	case HandType::HandType_Right:
		ovrAvatar_SetRightHandCustomGesture(Avatar, numJoints, joints);
		break;
	default:
		break;
	}
}

void UProteusOvrAvatar::SetControllerVisibility(HandType hand, bool visible)
{
	if (!Avatar)
		return;

	switch (hand)
	{
	case HandType::HandType_Left:
		ovrAvatar_SetLeftControllerVisibility(Avatar, visible);

		break;
	case HandType::HandType_Right:
		ovrAvatar_SetRightControllerVisibility(Avatar, visible);
		break;
	default:
		break;
	}
}

void UProteusOvrAvatar::StartPacketRecording()
{
	if (!Avatar)
		return;

	ovrAvatarPacket_BeginRecording(Avatar);
}

ovrAvatarPacket* UProteusOvrAvatar::EndPacketRecording()
{
	if (!Avatar)
		return nullptr;

	return ovrAvatarPacket_EndRecording(Avatar);
}

void UProteusOvrAvatar::UpdateFromPacket(ovrAvatarPacket* packet, const float time)
{
	if (Avatar && packet && time > 0.f)
	{
		ovrAvatar_UpdatePoseFromPacket(Avatar, packet, time);
	}
}

void UProteusOvrAvatar::UpdateMeshComponent(USceneComponent& mesh, const ovrAvatarTransform& transform)
{
	ProteusOvrAvatarHelpers::OvrAvatarTransformToSceneComponent(mesh, transform);
	mesh.SetVisibility(true, true);
}

void UProteusOvrAvatar::UpdateMaterial(UMeshComponent& mesh, const ovrAvatarMaterialState& material)
{
	UMaterialInstanceDynamic* MaterialInstance = Cast<UMaterialInstanceDynamic>(mesh.GetMaterial(0));

	check(MaterialInstance);

	if (auto AlphaTexture = FProteusOvrAvatarManager::Get().FindTexture(material.alphaMaskTextureID))
	{
		MaterialInstance->SetVectorParameterValue(FName("alphaMaskScaleOffset"), ProteusOvrAvatarHelpers::OvrAvatarVec4ToLinearColor(material.alphaMaskScaleOffset));
		MaterialInstance->SetTextureParameterValue(FName("alphaMask"), AlphaTexture);
	}

	if (auto NormalTexture = FProteusOvrAvatarManager::Get().FindTexture(material.normalMapTextureID))
	{
		MaterialInstance->SetVectorParameterValue(FName("normalMapScaleOffset"), ProteusOvrAvatarHelpers::OvrAvatarVec4ToLinearColor(material.normalMapScaleOffset));
		MaterialInstance->SetTextureParameterValue(FName("normalMap"), NormalTexture);
	}

	if (auto RoughnessTexture = FProteusOvrAvatarManager::Get().FindTexture(material.roughnessMapTextureID))
	{
		MaterialInstance->SetScalarParameterValue(FName("useRoughnessMap"), 1.0f);
		MaterialInstance->SetTextureParameterValue(FName("roughnessMap"), RoughnessTexture);
		MaterialInstance->SetVectorParameterValue(FName("roughnessMapScaleOffset"), ProteusOvrAvatarHelpers::OvrAvatarVec4ToLinearColor(material.roughnessMapScaleOffset));
	}
	else
	{
		MaterialInstance->SetScalarParameterValue(FName("useRoughnessMap"), 0.0f);
	}

	MaterialInstance->SetVectorParameterValue(FName("parallaxMapScaleOffset"), ProteusOvrAvatarHelpers::OvrAvatarVec4ToLinearColor(material.parallaxMapScaleOffset));
	if (auto ParallaxTexture = FProteusOvrAvatarManager::Get().FindTexture(material.parallaxMapTextureID))
	{
		MaterialInstance->SetTextureParameterValue(FName("parallaxMap"), ParallaxTexture);
	}

	MaterialInstance->SetVectorParameterValue(FName("baseColor"), ProteusOvrAvatarHelpers::OvrAvatarVec4ToLinearColor(material.baseColor));
	MaterialInstance->SetScalarParameterValue(FName("baseMaskType"), material.baseMaskType);
	MaterialInstance->SetVectorParameterValue(FName("baseMaskParameters"), ProteusOvrAvatarHelpers::OvrAvatarVec4ToLinearColor(material.baseMaskParameters));

	// Converts vector from Oculus to Unreal because of coordinate system difference
	ovrAvatarVector4f baseMaskAxis;
	baseMaskAxis.x = -material.baseMaskAxis.z;
	baseMaskAxis.y = material.baseMaskAxis.x;
	baseMaskAxis.z = material.baseMaskAxis.y;
	baseMaskAxis.w = material.baseMaskAxis.w;
	MaterialInstance->SetVectorParameterValue(FName("baseMaskAxis"), ProteusOvrAvatarHelpers::OvrAvatarVec4ToLinearColor(baseMaskAxis));

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
		MaterialInstance->SetVectorParameterValue(FName(*ParamName), ProteusOvrAvatarHelpers::OvrAvatarVec4ToLinearColor(material.layers[l].layerColor));
		ParamName = FString::Printf(TEXT("Layer%u_SurfaceScaleOffset"), l);
		MaterialInstance->SetVectorParameterValue(FName(*ParamName), ProteusOvrAvatarHelpers::OvrAvatarVec4ToLinearColor(material.layers[l].sampleScaleOffset));
		ParamName = FString::Printf(TEXT("Layer%u_SampleParameters"), l);
		MaterialInstance->SetVectorParameterValue(FName(*ParamName), ProteusOvrAvatarHelpers::OvrAvatarVec4ToLinearColor(material.layers[l].sampleParameters));

		ParamName = FString::Printf(TEXT("Layer%u_MaskParameters"), l);
		MaterialInstance->SetVectorParameterValue(FName(*ParamName), ProteusOvrAvatarHelpers::OvrAvatarVec4ToLinearColor(material.layers[l].maskParameters));
		ParamName = FString::Printf(TEXT("Layer%u_MaskAxis"), l);

		ovrAvatarVector4f layerMaskAxis;
		layerMaskAxis.x = -material.layers[l].maskAxis.z;
		layerMaskAxis.y = material.layers[l].maskAxis.x;
		layerMaskAxis.z = material.layers[l].maskAxis.y;
		layerMaskAxis.w = material.layers[l].maskAxis.w;
		MaterialInstance->SetVectorParameterValue(FName(*ParamName), ProteusOvrAvatarHelpers::OvrAvatarVec4ToLinearColor(layerMaskAxis));

		if (auto SampleTexture = FProteusOvrAvatarManager::Get().FindTexture(material.layers[l].sampleTexture))
		{
			ParamName = FString::Printf(TEXT("Layer%u_Surface"), l);
			MaterialInstance->SetTextureParameterValue(FName(*ParamName), SampleTexture);
		}
	}
}

void UProteusOvrAvatar::UpdateMaterialPBR(UPoseableMeshComponent& mesh, const ovrAvatarRenderPart_SkinnedMeshRenderPBS& data)
{
	UMaterialInstanceDynamic* MaterialInstance = Cast<UMaterialInstanceDynamic>(mesh.GetMaterial(0));

	if (auto AlbedoTexture = FProteusOvrAvatarManager::Get().FindTexture(data.albedoTextureAssetID))
	{	
		MaterialInstance->SetTextureParameterValue(FName("AlbedoMap"), AlbedoTexture);
	}

	if (auto SurfaceTexture = FProteusOvrAvatarManager::Get().FindTexture(data.surfaceTextureAssetID))
	{
		MaterialInstance->SetTextureParameterValue(FName("SurfaceMap"), SurfaceTexture);
	}
}

void UProteusOvrAvatar::UpdateMaterialProjector(UPoseableMeshComponent& mesh, const ovrAvatarRenderPart_ProjectorRender& data, const USceneComponent& OvrComponent)
{
	UMaterialInstanceDynamic* MaterialInstance = Cast<UMaterialInstanceDynamic>(mesh.GetMaterial(0));

	FTransform ProjectorLocalTransform;
	FTransform OvrComponentWorld = OvrComponent.GetComponentToWorld();

	ProteusOvrAvatarHelpers::ConvertTransform(data.localTransform, ProjectorLocalTransform);
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

void UProteusOvrAvatar::UpdateMaterialPBRV2(UPoseableMeshComponent& mesh, const ovrAvatarRenderPart_SkinnedMeshRenderPBS_V2& data)
{	
	UMaterialInstanceDynamic* MaterialInstance = Cast<UMaterialInstanceDynamic>(mesh.GetMaterial(0));

	if (auto AlbedoTexture = FProteusOvrAvatarManager::Get().FindTexture(data.materialState.albedoTextureID))
	{
		static FName AlbedoParamName(TEXT("AlbedoTexture"));
		MaterialInstance->SetTextureParameterValue(AlbedoParamName, AlbedoTexture);
	}

	static FName AlbedoMultiplierParamName(TEXT("AlbedoMultiplier"));
	MaterialInstance->SetVectorParameterValue(AlbedoMultiplierParamName, ProteusOvrAvatarHelpers::OvrAvatarVec4ToLinearColor(data.materialState.albedoMultiplier));

	if (auto MetallicnessTexture = FProteusOvrAvatarManager::Get().FindTexture(data.materialState.metallicnessTextureID))
	{
		static FName MetalicnessParamName(TEXT("Roughness"));
		MaterialInstance->SetTextureParameterValue(MetalicnessParamName, MetallicnessTexture);
	}

	if (auto NormalTexture = FProteusOvrAvatarManager::Get().FindTexture(data.materialState.normalTextureID))
	{
		static FName MetalicnessParamName(TEXT("NormalMap"));
		MaterialInstance->SetTextureParameterValue(MetalicnessParamName, NormalTexture);
	}
}

UPoseableMeshComponent* UProteusOvrAvatar::CreateMeshComponent(USceneComponent* parent, ovrAvatarAssetID assetID, const FString& name)
{
	UPoseableMeshComponent* MeshComponent = NewObject<UPoseableMeshComponent>(parent->GetOwner(), *name);
	MeshComponent->AttachToComponent(parent, FAttachmentTransformRules::SnapToTargetIncludingScale);
	MeshComponent->RegisterComponent();

	MeshComponent->bCastDynamicShadow = false;
	MeshComponent->CastShadow = false;
	MeshComponent->bRenderCustomDepth = false;
	MeshComponent->bRenderInMainPass = true;
	MeshComponent->SetVisibility(false, true);
	///////////////////////////////////////////////////////////////
	//MeshComponent->bOwnerNoSee = true;

	AddMeshComponent(assetID, MeshComponent);

	return MeshComponent;
}

UPoseableMeshComponent* UProteusOvrAvatar::CreateDepthMeshComponent(USceneComponent* parent, ovrAvatarAssetID assetID, const FString& name)
{
	UPoseableMeshComponent* MeshComponent = NewObject<UPoseableMeshComponent>(parent->GetOwner(), *name);
	MeshComponent->AttachToComponent(parent, FAttachmentTransformRules::SnapToTargetIncludingScale);
	MeshComponent->RegisterComponent();

	MeshComponent->bCastDynamicShadow = false;
	MeshComponent->CastShadow = false;
	MeshComponent->bRenderCustomDepth = true;
	MeshComponent->bRenderInMainPass = false;
	MeshComponent->SetVisibility(false, true);
	MeshComponent->SetVisibility(false, true);
	///////////////////////////////////////////////////////////////
	//MeshComponent->bOwnerNoSee = true;

	AddDepthMeshComponent(assetID, MeshComponent);

	return MeshComponent;
}

template<typename MeshAssetData, typename VertexType>
void UProteusOvrAvatar::LoadMesh(USkeletalMesh* SkeletalMesh, const MeshAssetData* data)
{
#if WITH_EDITOR
	UE_LOG(LogAvatars, Display, TEXT("[Avatars] Loaded Mesh WITH_EDITOR."));

	FSkeletalMeshLODModel* LodModel = new FSkeletalMeshLODModel();
	SkeletalMesh->GetImportedModel()->LODModels.Add(LodModel);

	new(LodModel->Sections) FSkelMeshSection();
	LodModel->Sections[0].MaterialIndex = 0;
	LodModel->Sections[0].BaseIndex = 0;
	LodModel->Sections[0].NumTriangles = 0;

	FSkeletalMeshLODInfo& LodInfo = SkeletalMesh->AddLODInfo();

	LodInfo.ScreenSize = 0.3f;
	LodInfo.LODHysteresis = 0.2f;

	LodInfo.LODMaterialMap.Add(0);

	SkeletalMesh->Materials.Add(UMaterial::GetDefaultMaterial(MD_Surface));
	SkeletalMesh->Materials[0].UVChannelData.bInitialized = true;
	SkeletalMesh->RefSkeleton.Empty(data->skinnedBindPose.jointCount);

	SkeletalMesh->bUseFullPrecisionUVs = true;
	SkeletalMesh->bHasBeenSimplified = false;
	SkeletalMesh->bHasVertexColors = true;

	for (uint32 BoneIndex = 0; BoneIndex < data->skinnedBindPose.jointCount; BoneIndex++)
	{
		LodModel->RequiredBones.Add(BoneIndex);
		LodModel->ActiveBoneIndices.Add(BoneIndex);
		LodModel->Sections[0].BoneMap.Add(BoneIndex);

		FString BoneString = data->skinnedBindPose.jointNames[BoneIndex];

		// Not allowed to duplicate bone names...
		static FString RootBoneName = FString(TEXT("root"));
		if (BoneString == RootBoneName)
		{
			BoneString += FString::Printf(TEXT("_%u"), BoneIndex);
		}

		FName BoneName = FName(*BoneString);

		FTransform Transform = FTransform::Identity;
		ProteusOvrAvatarHelpers::ConvertTransform(data->skinnedBindPose.jointTransform[BoneIndex], Transform);

		FReferenceSkeletonModifier Modifier = FReferenceSkeletonModifier(SkeletalMesh->RefSkeleton, nullptr);
		int32 ParentIndex = BoneIndex > 0 && data->skinnedBindPose.jointParents[BoneIndex] < 0 ? 0 : data->skinnedBindPose.jointParents[BoneIndex];
		Modifier.Add(FMeshBoneInfo(BoneName, BoneString, ParentIndex), Transform);
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

	const VertexType* SourceVertex = data->vertexBuffer;
	const uint32_t NumBlendWeights = 4;

	FSoftSkinVertex* DestVertex = MeshSection.SoftVertices.GetData();
	FBox BoundBox = FBox();
	BoundBox.Init();

	for (uint32_t VertIndex = 0; VertIndex < data->vertexCount; VertIndex++, SourceVertex++, DestVertex++)
	{
		DestVertex->Position = 100.0f * FVector(-SourceVertex->z, SourceVertex->x, SourceVertex->y);
		DestVertex->Color = UProteusOvrAvatar::GetColorFromVertex(*SourceVertex);

		BoundBox += DestVertex->Position;

		FVector n = FVector(-SourceVertex->nz, SourceVertex->nx, SourceVertex->ny);
		FVector t = FVector(-SourceVertex->tz, SourceVertex->tx, SourceVertex->ty);
		FVector bt = FVector::CrossProduct(t, n) * FMath::Sign(SourceVertex->tw);
		DestVertex->TangentX = t;
		DestVertex->TangentY = bt;
		DestVertex->TangentZ = n;
		DestVertex->UVs[0] = FVector2D(SourceVertex->u, SourceVertex->v);

		uint32 RecomputeIndex = -1;
		uint32 RecomputeIndexWeight = 0;

		for (uint32_t BlendIndex = 0; BlendIndex < MAX_TOTAL_INFLUENCES; BlendIndex++)
		{
			DestVertex->InfluenceWeights[BlendIndex] = BlendIndex < NumBlendWeights ? (uint8_t)(255.9999f*SourceVertex->blendWeights[BlendIndex]) : 0;
			DestVertex->InfluenceBones[BlendIndex] = BlendIndex < NumBlendWeights ? SourceVertex->blendIndices[BlendIndex] : 0;

			uint32 Weight = DestVertex->InfluenceWeights[BlendIndex];
			if (Weight > RecomputeIndexWeight)
			{
				RecomputeIndexWeight = Weight;
				RecomputeIndex = BlendIndex;
			}
		}

		uint32 SumExceptRecompute = 0;
		for (uint32_t BlendIndex = 0; BlendIndex < NumBlendWeights; BlendIndex++)
		{
			if (BlendIndex != RecomputeIndex)
			{
				SumExceptRecompute += DestVertex->InfluenceWeights[BlendIndex];
			}
		}

		ensure(SumExceptRecompute >= 0 && SumExceptRecompute <= 255);
		DestVertex->InfluenceWeights[RecomputeIndex] = 255 - SumExceptRecompute;
	}

	LodModel->NumVertices = data->vertexCount;
	LodModel->NumTexCoords = 1;

	for (uint32_t index = 0; index < data->indexCount; index++)
	{
		LodModel->IndexBuffer.Add(data->indexBuffer[index]);
	}

	FBoxSphereBounds Bounds(BoundBox);
	Bounds = Bounds.ExpandBy(100000.0f);
	SkeletalMesh->SetImportedBounds(Bounds);
	SkeletalMesh->PostEditChange();

	SkeletalMesh->Skeleton = NewObject<USkeleton>();
	SkeletalMesh->Skeleton->MergeAllBonesToBoneTree(SkeletalMesh);
	SkeletalMesh->PostLoad();
#else
	UE_LOG(LogAvatars, Display, TEXT("[Avatars] Loaded Mesh."));

	FSkeletalMeshLODRenderData* LodRenderData = new FSkeletalMeshLODRenderData();
	SkeletalMesh->AllocateResourceForRendering();
	SkeletalMesh->GetResourceForRendering()->LODRenderData.Add(LodRenderData);

	new(LodRenderData->RenderSections) FSkelMeshRenderSection();
	LodRenderData->RenderSections[0].MaterialIndex = 0;
	LodRenderData->RenderSections[0].BaseIndex = 0;
	LodRenderData->RenderSections[0].NumTriangles = 0;

	FSkeletalMeshLODInfo& LodInfo = SkeletalMesh->AddLODInfo();

	LodInfo.ScreenSize = 0.3f;
	LodInfo.LODHysteresis = 0.2f;

	LodInfo.LODMaterialMap.Add(0);

	SkeletalMesh->Materials.Add(UMaterial::GetDefaultMaterial(MD_Surface));
	SkeletalMesh->Materials[0].UVChannelData.bInitialized = true;
	SkeletalMesh->RefSkeleton.Empty(data->skinnedBindPose.jointCount);

	SkeletalMesh->bUseFullPrecisionUVs = true;
	SkeletalMesh->bHasBeenSimplified = false;
	SkeletalMesh->bHasVertexColors = true;

	for (uint32 BoneIndex = 0; BoneIndex < data->skinnedBindPose.jointCount; BoneIndex++)
	{
		LodRenderData->RequiredBones.Add(BoneIndex);
		LodRenderData->ActiveBoneIndices.Add(BoneIndex);
		LodRenderData->RenderSections[0].BoneMap.Add(BoneIndex);

		FString BoneString = data->skinnedBindPose.jointNames[BoneIndex];

		// Not allowed to duplicate bone names...
		static FString RootBoneName = FString(TEXT("root"));
		if (BoneString == RootBoneName)
		{
			BoneString += FString::Printf(TEXT("_%u"), BoneIndex);
		}

		FName BoneName = FName(*BoneString);

		FTransform Transform = FTransform::Identity;
		ProteusOvrAvatarHelpers::ConvertTransform(data->skinnedBindPose.jointTransform[BoneIndex], Transform);

		FReferenceSkeletonModifier Modifier = FReferenceSkeletonModifier(SkeletalMesh->RefSkeleton, nullptr);
		int32 ParentIndex = BoneIndex > 0 && data->skinnedBindPose.jointParents[BoneIndex] < 0 ? 0 : data->skinnedBindPose.jointParents[BoneIndex];
		Modifier.Add(FMeshBoneInfo(BoneName, BoneString, ParentIndex), Transform);
	}

	check(data->indexCount % 3 == 0);
	check(data->vertexCount > 0);

	auto& MeshSection = LodRenderData->RenderSections[0];
	MeshSection.BaseIndex = 0;
	MeshSection.NumTriangles = data->indexCount / 3;
	MeshSection.BaseVertexIndex = 0;
	MeshSection.NumVertices = data->vertexCount;
	MeshSection.MaxBoneInfluences = 4;

	const VertexType* SourceVertex = data->vertexBuffer;
	const uint32_t NumBlendWeights = 4;

	FBox BoundBox = FBox();
	BoundBox.Init();

	LodRenderData->StaticVertexBuffers.PositionVertexBuffer.Init(data->vertexCount);
	LodRenderData->StaticVertexBuffers.ColorVertexBuffer.Init(data->vertexCount);
	LodRenderData->StaticVertexBuffers.StaticMeshVertexBuffer.Init(data->vertexCount, 1);

	TArray<FColor> ColorArray;
	TArray<TSkinWeightInfo<true>> InWeights;
	InWeights.AddUninitialized(data->vertexCount);
	TMap<int32, TArray<int32>> OverlappingVertices;

	for (uint32_t VertIndex = 0; VertIndex < data->vertexCount; VertIndex++, SourceVertex++)
	{
		FModelVertex ModelVertex;
		ModelVertex.Position = 100.0f * FVector(-SourceVertex->z, SourceVertex->x, SourceVertex->y);
		BoundBox += ModelVertex.Position;

		ColorArray.Add(UProteusOvrAvatar::GetColorFromVertex(*SourceVertex));

		FVector n = FVector(-SourceVertex->nz, SourceVertex->nx, SourceVertex->ny);
		FVector t = FVector(-SourceVertex->tz, SourceVertex->tx, SourceVertex->ty);
		ModelVertex.TangentX = t;
		ModelVertex.TangentZ = n;
		ModelVertex.TexCoord = FVector2D(SourceVertex->u, SourceVertex->v);

		LodRenderData->StaticVertexBuffers.PositionVertexBuffer.VertexPosition(VertIndex) = ModelVertex.Position;
		LodRenderData->StaticVertexBuffers.StaticMeshVertexBuffer.SetVertexTangents(VertIndex, ModelVertex.TangentX, ModelVertex.GetTangentY(), ModelVertex.TangentZ);
		LodRenderData->StaticVertexBuffers.StaticMeshVertexBuffer.SetVertexUV(VertIndex, 0, ModelVertex.TexCoord);

		uint32 RecomputeIndex = -1;
		uint32 RecomputeIndexWeight = 0;

		TArray<int32> Vertices;
		for (uint32_t BlendIndex = 0; BlendIndex < MAX_TOTAL_INFLUENCES; BlendIndex++)
		{
			InWeights[VertIndex].InfluenceWeights[BlendIndex] = BlendIndex < NumBlendWeights ? (uint8_t)(255.9999f*SourceVertex->blendWeights[BlendIndex]) : 0;
			InWeights[VertIndex].InfluenceBones[BlendIndex] = BlendIndex < NumBlendWeights ? SourceVertex->blendIndices[BlendIndex] : 0;

			uint32 Weight = InWeights[VertIndex].InfluenceWeights[BlendIndex];
			if (Weight > RecomputeIndexWeight)
			{
				RecomputeIndexWeight = Weight;
				RecomputeIndex = BlendIndex;
			}

			Vertices.Add(BlendIndex < NumBlendWeights ? SourceVertex->blendIndices[BlendIndex] : 0);
		}

		uint32 SumExceptRecompute = 0;
		for (uint32_t BlendIndex = 0; BlendIndex < NumBlendWeights; BlendIndex++)
		{
			if (BlendIndex != RecomputeIndex)
			{
				SumExceptRecompute += InWeights[VertIndex].InfluenceWeights[BlendIndex];
			}
		}

		ensure(SumExceptRecompute >= 0 && SumExceptRecompute <= 255);
		InWeights[VertIndex].InfluenceWeights[RecomputeIndex] = 255 - SumExceptRecompute;

		OverlappingVertices.Add(VertIndex, Vertices);
	}

	LodRenderData->StaticVertexBuffers.ColorVertexBuffer.InitFromColorArray(ColorArray);
	LodRenderData->SkinWeightVertexBuffer.SetHasExtraBoneInfluences(true);
	LodRenderData->SkinWeightVertexBuffer = InWeights;
	MeshSection.DuplicatedVerticesBuffer.Init(data->vertexCount, OverlappingVertices);
	LodRenderData->MultiSizeIndexContainer.CreateIndexBuffer(sizeof(uint16_t));

	for (uint32_t index = 0; index < data->indexCount; index++)
	{
		LodRenderData->MultiSizeIndexContainer.GetIndexBuffer()->AddItem(data->indexBuffer[index]);
	}

	FBoxSphereBounds Bounds(BoundBox);
	Bounds = Bounds.ExpandBy(100000.0f);
	SkeletalMesh->SetImportedBounds(Bounds);

	SkeletalMesh->Skeleton = NewObject<USkeleton>();
	SkeletalMesh->Skeleton->MergeAllBonesToBoneTree(SkeletalMesh);
	SkeletalMesh->PostLoad();
#endif
}

void UProteusOvrAvatar::InitializeMaterials()
{
	const uint32_t ComponentCount = ovrAvatarComponent_Count(Avatar);
	for (uint32_t ComponentIndex = 0; ComponentIndex < ComponentCount; ComponentIndex++)
	{
		const ovrAvatarComponent* OvrComponent = ovrAvatarComponent_Get(Avatar, ComponentIndex);
		const bool IsBodyComponent = FString(OvrComponent->name).Contains(BodyName);

		for (uint32_t RenderIndex = 0; RenderIndex < OvrComponent->renderPartCount; ++RenderIndex)
		{
			const ovrAvatarRenderPart* RenderPart = OvrComponent->renderParts[RenderIndex];

			switch (ovrAvatarRenderPart_GetType(RenderPart))
			{
			case ovrAvatarRenderPartType_SkinnedMeshRender:
			{
				const ovrAvatarRenderPart_SkinnedMeshRender* RenderData = ovrAvatarRenderPart_GetSkinnedMeshRender(RenderPart);
				if (UPoseableMeshComponent* mesh = GetMeshComponent(RenderData->meshAssetID))
				{
					UpdateMaterial(*mesh, RenderData->materialState);
				}

			}
			break;
			case ovrAvatarRenderPartType_SkinnedMeshRenderPBS:
			{
				const ovrAvatarRenderPart_SkinnedMeshRenderPBS* RenderData = ovrAvatarRenderPart_GetSkinnedMeshRenderPBS(RenderPart);
				if (UPoseableMeshComponent* mesh = GetMeshComponent(RenderData->meshAssetID))
				{
					UpdateMaterialPBR(*mesh, *RenderData);
				}
			}
			break;
			case ovrAvatarRenderPartType_ProjectorRender:
				break;
			case ovrAvatarRenderPartType_SkinnedMeshRenderPBS_V2:
			{
				if (IsBodyComponent && UseCombinedBodyMesh && RenderIndex == 0)
				{
					const ovrAvatarRenderPart_SkinnedMeshRenderPBS_V2* RenderData = ovrAvatarRenderPart_GetSkinnedMeshRenderPBSV2(RenderPart);
					if (UPoseableMeshComponent* mesh = GetMeshComponent(RenderData->meshAssetID))
					{
						static FName AlbedoFields[] =
						{
							"Albedo_Body",
							"Albedo_Clothing",
							"Albedo_Visor",
							"Albedo_Hair",
							"Albedo_Beard"
						};

						static FName AlbedoMultiplierFields[] =
						{
							"AlbedoMultiplier_Body",
							"AlbedoMultiplier_Clothing",
							"AlbedoMultiplier_Visor",
							"AlbedoMultiplier_Hair",
							"AlbedoMultiplier_Beard"
						};

						static FName RoughnessFields[] =
						{
							"BodyRoughness",
							"ClothingRoughness",
							"VisorRoughness",
							"HairRoughness",
							"BeardRoughness"
						};

						static FName NormalFields[] =
						{
							"BodyNormal",
							"ClothingNormal",
							"VisorNormal",
							"HairNormal",
							"BeardNormal"
						};

						static FName RedThresholds[] =
						{
							"BodyRedThreshold",
							"ClothingRedThreshold",
							"VisorRedThreshold",
							"HairRedThreshold",
							"BeardRedThreshold"
						};

						UMaterialInstanceDynamic* MaterialInstance = Cast<UMaterialInstanceDynamic>(mesh->GetMaterial(0));

						uint32_t Count = 0;
						const ovrAvatarPBSMaterialState* MaterialStates = ovrAvatar_GetBodyPBSMaterialStates(RenderPart, &Count);

						for (uint32_t MatIndex = 0; MatIndex < Count; MatIndex++)
						{
							auto matState = MaterialStates[MatIndex];

							const float ThresholdValue = (float)MatIndex * 0.25f + 0.05f;
							MaterialInstance->SetScalarParameterValue(RedThresholds[MatIndex], ThresholdValue);

							if (auto AlbedoTexture = FProteusOvrAvatarManager::Get().FindTexture(matState.albedoTextureID))
							{
								MaterialInstance->SetTextureParameterValue(AlbedoFields[MatIndex], AlbedoTexture);
							}

							MaterialInstance->SetVectorParameterValue(
								AlbedoMultiplierFields[MatIndex],
								ProteusOvrAvatarHelpers::OvrAvatarVec4ToLinearColor(matState.albedoMultiplier));

							if (auto MetallicnessTexture = FProteusOvrAvatarManager::Get().FindTexture(matState.metallicnessTextureID))
							{
								MaterialInstance->SetTextureParameterValue(RoughnessFields[MatIndex], MetallicnessTexture);
							}

							if (auto NormalTexture = FProteusOvrAvatarManager::Get().FindTexture(matState.normalTextureID))
							{
								MaterialInstance->SetTextureParameterValue(NormalFields[MatIndex], NormalTexture);
							}
						}
					}
				}
				else
				{
					const ovrAvatarRenderPart_SkinnedMeshRenderPBS_V2* RenderData = ovrAvatarRenderPart_GetSkinnedMeshRenderPBSV2(RenderPart);
					if (UPoseableMeshComponent* mesh = GetMeshComponent(RenderData->meshAssetID))
					{
						UpdateMaterialPBRV2(*mesh, *RenderData);
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

void UProteusOvrAvatar::UpdateV2VoiceOffsetParams()
{
	if (!UseV2VoiceVisualization)
	{
		return;
	}

	if (auto BodyMesh = GetMeshComponent(BodyMeshID))
	{
		UpdateVoiceVizOnMesh(BodyMesh);
	}

	if (auto DepthMesh = GetDepthMeshComponent(BodyMeshID))
	{
		UpdateVoiceVizOnMesh(DepthMesh);
	}

}

void UProteusOvrAvatar::UpdateVoiceVizOnMesh(UPoseableMeshComponent* Mesh)
{
	static const FName VoiceScaleParam(TEXT("VoiceScale"));
	static const FName VoiceDirectionParam(TEXT("VoiceDirection"));
	static const FName VoicePositionParam(TEXT("VoicePosition"));
	static const FName VoiceComponentScaleParam(TEXT("VoiceComponentScale"));

	static const FVector4 MOUTH_POSITION_OFFSET = FVector4(10.51, 0.f, -1.4f, 0.f);
	static const float MOUTH_SCALE = 0.7f;
	static const float MOUTH_MAX = 0.7f;
	static const int32 NECK_JOINT = 4;
	static const FVector4 UP(0.f, 0.f, 1.f, 0.f);

	auto parentTransform = Mesh->GetAttachParent();
	auto scale = parentTransform->GetComponentScale();
	Mesh->GetBoneTransform(NECK_JOINT).TransformFVector4(FVector::UpVector);
	if (UMaterialInstanceDynamic* MaterialInstance = Cast<UMaterialInstanceDynamic>(Mesh->GetMaterial(0)))
	{
		auto NeckJoint = Mesh->GetBoneTransform(NECK_JOINT);
		FVector transUp = NeckJoint.TransformFVector4(UP);
		transUp.Normalize();

		auto direction = FLinearColor(transUp);
		auto position = NeckJoint.TransformFVector4(MOUTH_POSITION_OFFSET);
		auto neckPosition = NeckJoint.GetTranslation();

		MaterialInstance->SetVectorParameterValue(
			VoicePositionParam,
			FLinearColor(neckPosition + position));


		MaterialInstance->SetVectorParameterValue(
			VoiceDirectionParam,
			FLinearColor(direction));

		FTransform mouthPos;
		mouthPos.SetRotation(NeckJoint.GetRotation());
		mouthPos.SetTranslation(neckPosition + position);

		ProteusOvrAvatarHelpers::DebugDrawCoords(GetWorld(), mouthPos);

		const float appliedValue = FMath::Min(scale.Z * MOUTH_MAX, scale.Z * VoiceVisualValue * MOUTH_SCALE);
		MaterialInstance->SetScalarParameterValue(VoiceScaleParam, appliedValue);

		// Assume Uniform Scale, it's going to be messed up anyway if not
		MaterialInstance->SetScalarParameterValue(VoiceComponentScaleParam, scale.Z);
	}
}

void UProteusOvrAvatar::DebugDriveVoiceValue(float DeltaTime)
{
	static float TimeAccum = 0.f;
	TimeAccum += DeltaTime;

	static float Dampen = 0.25f;
	float VoiceValue = (FMath::Sin(TimeAccum * 2.f * PI * Dampen) + 1.f) * 0.5f;

	SetVoiceVisualValue(VoiceValue);
}

bool gLogSDKTransforms = false;
void UProteusOvrAvatar::DebugLogAvatarSDKTransforms(const FString& wrapper)
{
	if (!Avatar || !gLogSDKTransforms)
		return;

	UE_LOG(LogAvatars, Warning, TEXT("\n[Avatars] -------------------------- %s ----------------------------"), *wrapper);

	const uint32_t ComponentCount = ovrAvatarComponent_Count(Avatar);

	FTransform Logger = FTransform::Identity;

	for (uint32_t CompIndex = 0; CompIndex < ComponentCount; ++CompIndex)
	{
		const ovrAvatarComponent* AvatarComponent = ovrAvatarComponent_Get(Avatar, CompIndex);

		ProteusOvrAvatarHelpers::ConvertTransform(AvatarComponent->transform, Logger);
		Logger.DebugPrint();

		for (uint32_t RenderIndex = 0; RenderIndex < AvatarComponent->renderPartCount; ++RenderIndex)
		{
			const ovrAvatarRenderPart* RenderPart = AvatarComponent->renderParts[RenderIndex];

			switch (ovrAvatarRenderPart_GetType(RenderPart))
			{
			case ovrAvatarRenderPartType_SkinnedMeshRender:
			{
				const ovrAvatarRenderPart_SkinnedMeshRender* RenderData = ovrAvatarRenderPart_GetSkinnedMeshRender(RenderPart);
				ProteusOvrAvatarHelpers::ConvertTransform(RenderData->localTransform, Logger);
				Logger.DebugPrint();
			}
			break;
			case ovrAvatarRenderPartType_SkinnedMeshRenderPBS:
			{
				const ovrAvatarRenderPart_SkinnedMeshRenderPBS* RenderData = ovrAvatarRenderPart_GetSkinnedMeshRenderPBS(RenderPart);
				ProteusOvrAvatarHelpers::ConvertTransform(RenderData->localTransform, Logger);
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

void UProteusOvrAvatar::DebugLogMaterialData(const ovrAvatarMaterialState& material, const FString& name)
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

static FString* gOverrideMaterial = nullptr;
const FString& UProteusOvrAvatar::GetPBRV2MainMaterialString(bool useCombined)
{
	static FString Mobile = FString(TEXT("/ProteusAvatars/Materials/AvatarsPBR_2/OculusAvatars_PBRV2_Mobile"));
	static FString CombinedMobile = FString(TEXT("/ProteusAvatars/Materials/AvatarsPBR_2/OculusAvatars_PBRV2_Mobile_Combined"));
	static FString PCSingle = FString(TEXT("/ProteusAvatars/Materials/AvatarsPBR_2/OculusAvatars_PBRV2.OculusAvatars_PBRV2"));
	static FString PCCombined = FString(TEXT("/ProteusAvatars/Materials/AvatarsPBR_2/OculusAvatars_PBRV2_Combined"));

	if (gOverrideMaterial)
	{
		return *gOverrideMaterial;
	}

#if PLATFORM_ANDROID
	return useCombined ? CombinedMobile : Mobile;
#else
	return useCombined ? PCCombined : PCSingle;
#endif
}

ovrAvatarControllerType UProteusOvrAvatar::GetControllerTypeByHardware()
{
	if (!FProteusOvrAvatarManager::Get().IsOVRPluginValid())
		return ovrAvatarControllerType_Touch;

	ovrAvatarControllerType controllerType = ovrAvatarControllerType_Touch;

	ovrpSystemHeadset hmdType = ovrpSystemHeadset_None;
	ovrpResult result = ovrp_GetSystemHeadsetType2(&hmdType);

	if (result != ovrpSuccess)
	{
		UE_LOG(LogAvatars, Warning, TEXT("GetControllerTypeByHardware: ovrp_GetSystemHeadsetType2 failed %d"), result);
	}

	switch (hmdType)
	{
	case ovrpSystemHeadset_GearVR_R320:
	case ovrpSystemHeadset_GearVR_R321:
	case ovrpSystemHeadset_GearVR_R322:
	case ovrpSystemHeadset_GearVR_R323:
	case ovrpSystemHeadset_GearVR_R324:
	case ovrpSystemHeadset_GearVR_R325:
		controllerType = ovrAvatarControllerType_Malibu;
		break;
	case ovrpSystemHeadset_Oculus_Go:  
		controllerType = ovrAvatarControllerType_Go;
		break;
	case ovrpSystemHeadset_Oculus_Quest: 
	case ovrpSystemHeadset_Rift_DK1:
	case ovrpSystemHeadset_Rift_DK2:
	case ovrpSystemHeadset_Rift_CV1:
	case ovrpSystemHeadset_Rift_CB:
	default:
		controllerType = ovrAvatarControllerType_Touch;
		break;
	}

	return controllerType;
}
	