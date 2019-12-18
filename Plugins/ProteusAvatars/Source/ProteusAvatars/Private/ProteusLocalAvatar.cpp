
#include "ProteusLocalAvatar.h"
#include "Object.h"
#include "OVRLipSyncLiveActorComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"
#include "IConsoleManager.h"

static const uint32_t HAND_JOINTS = 25;

std::unordered_map<AvatarLevelOfDetail, ovrAvatarAssetLevelOfDetail, AvatarLevelOfDetailHash> AProteusLocalAvatar::LODMap
(
	{
		{ AvatarLevelOfDetail::Low, ovrAvatarAssetLevelOfDetail_One},
		{ AvatarLevelOfDetail::Mid, ovrAvatarAssetLevelOfDetail_Three},
		{ AvatarLevelOfDetail::High, ovrAvatarAssetLevelOfDetail_Five}
	}
);

AProteusLocalAvatar::AProteusLocalAvatar()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("LocalAvatarRoot"));
	AvatarComponent = CreateDefaultSubobject<UOvrAvatar>(TEXT("LocalAvatar"));
	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("LocalAvatarAudio"));
	LipSyncComponent = CreateDefaultSubobject<UOVRLipSyncActorComponent>(TEXT("LocalLipSync"));

	PrimaryActorTick.bCanEverTick = true;
	AutoPossessPlayer = EAutoReceiveInput::Disabled;

	bReplicates = true; //pawn is a replicated to clients
	bAlwaysRelevant = true; //pawn is always in scope for replication
	bReplicateMovement = true; //server will automatically pass along movement data to clients  (clients still need to tell server they are moving)
	bNetLoadOnClient = true; //probably not needed
	NetPriority = 5.0f;
}

void AProteusLocalAvatar::PreInitializeComponents()
{
	Super::PreInitializeComponents();
	UE_LOG(LogTemp, Warning, TEXT("FCT: PREINITIALIZE COMPONENTS"));
#if PLATFORM_WINDOWS
	auto SilenceDetectionThresholdCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("voice.SilenceDetectionThreshold"));
	SilenceDetectionThresholdCVar->Set(0.f);
#endif
}

void AProteusLocalAvatar::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("BEGIN PLAY"));
}

void AProteusLocalAvatar::BeginDestroy()
{
	Super::BeginDestroy();
}

void AProteusLocalAvatar::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UseLocalMicrophone)
	{
		LipSyncComponent->OnVisemesReady.RemoveDynamic(this, &AProteusLocalAvatar::LipSyncVismesReady);
	}
}

void AProteusLocalAvatar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (IsUsingAvatars)
	{
		bAvatarIsLocal = (Controller && Controller->IsLocalController());

		if ((bAvatarIsLocal) && AvatarComponent)
		{
			UpdatePacketRecording(DeltaTime);
		}

		else if ((!bAvatarIsLocal) && AvatarComponent)
		{
			CurrentPacketTime += DeltaTime;
			AvatarComponent->UpdateFromPacket(CurrentPacket, FMath::Min(CurrentPacketTime, CurrentPacketLength));
		}
	}
}

void AProteusLocalAvatar::InitializeLocalAvatar(FString OculusUserID)
{
	UE_LOG(LogTemp, Warning, TEXT("InitializeLocalAvatar"));
	UE_LOG(LogTemp, Warning, TEXT("InitializeLocalAvatar: PacketKey is %s"), *OculusUserID);

	if (IsUsingAvatars)
	{
		if (UseLocalMicrophone)
		{
			LipSyncComponent->OnVisemesReady.AddDynamic(this, &AProteusLocalAvatar::LipSyncVismesReady);
			LipSyncComponent->Start();
		}
		uint64 OculusID64 = FCString::Strtoui64(*OculusUserID, NULL, 10);
		AvatarComponent->RequestAvatar(OculusID64, LODMap[LevelOfDetail], UseCombinedMesh);
		UE_LOG(LogTemp, Warning, TEXT("InitializeLocalAvatar: Request Avatar %llu"), OculusID64);
		AvatarComponent->SetPlayerType(UOvrAvatar::ePlayerType::Local);
		AvatarComponent->StartPacketRecording();
		AvatarComponent->SetVisibilityType(AvatarVisibilityType == AvatarVisibility::FirstPerson? ovrAvatarVisibilityFlag_FirstPerson: ovrAvatarVisibilityFlag_ThirdPerson);
		AvatarComponent->SetBodyMaterial(GetOvrAvatarMaterialFromType(BodyMaterial));
		AvatarComponent->SetHandMaterial(GetOvrAvatarMaterialFromType(HandsMaterial));
		}
	UE_LOG(LogTemp, Warning, TEXT("Local Avatar: StartPacketRecording has fired!"));
	if (UseLocalMicrophone)
{
	Online::GetVoiceInterface()->StartNetworkedVoice(0);
}
	UE_LOG(LogTemp, Warning, TEXT("Local Avatar: StartNetworkedVoice has fired!"));
	SetActorHiddenInGame(false);
}

void AProteusLocalAvatar::InitializeRemoteAvatar()
{
	bRemoteAvatarIsLoaded = true;
	UE_LOG(LogTemp, Warning, TEXT("InitializeRemoteAvatar"));
	UE_LOG(LogTemp, Warning, TEXT("InitializeRemoteAvatar: LOADED"));
	UE_LOG(LogTemp, Warning, TEXT("InitializeRemoteAvatar: PacketKey is %s"), *OculusID);
#if PLATFORM_ANDROID
	ovrAvatarAssetLevelOfDetail lod = ovrAvatarAssetLevelOfDetail_Three;
#else
	ovrAvatarAssetLevelOfDetail lod = ovrAvatarAssetLevelOfDetail_Five;
#endif
		uint64 OculusID64 = FCString::Strtoui64(*OculusID, NULL, 10);
		AvatarComponent->RequestAvatar(OculusID64, lod, UseCombinedMesh);
		UE_LOG(LogTemp, Warning, TEXT("InitializeRemoteAvatar: Request Avatar %llu"), OculusID64);
		AvatarComponent->SetPlayerType(UOvrAvatar::ePlayerType::Remote);
		AvatarComponent->SetVisibilityType(ovrAvatarVisibilityFlag_ThirdPerson);
		AvatarComponent->SetBodyMaterial(GetOvrAvatarMaterialFromType(BodyMaterial));
		AvatarComponent->SetHandMaterial(GetOvrAvatarMaterialFromType(HandsMaterial));

	SetActorHiddenInGame(false);
}

void AProteusLocalAvatar::UpdatePacketRecording(float DeltaTime)
{
	ovrAvatarPacket* Packet = AvatarComponent->EndPacketRecording();

	if (Packet == nullptr && AvatarComponent)
	{
		// Nothing to do, so start recording again
		AvatarComponent->StartPacketRecording();
		return;
	}

	const uint32_t PacketSize = ovrAvatarPacket_GetSize(Packet);

	TArray<uint8>& PacketDataBuffer = ReplicatedPacketData.AvatarPacketData;
	PacketDataBuffer.SetNumUninitialized(PacketSize);

	if (ovrAvatarPacket_Write(Packet, PacketSize, PacketDataBuffer.GetData()))
	{
		// Send client packet data to server
		ServerHandleAvatarPacket(ReplicatedPacketData);

		// Packet handled, free it up and start recording again
		ovrAvatarPacket_Free(Packet);
		AvatarComponent->StartPacketRecording();
	}
}

void AProteusLocalAvatar::ServerHandleAvatarPacket_Implementation(const FOculusAvatarPacket& PacketData)
{
	//The packet has already been validated to contain data.  Updated our replicated property to fire the callback on all clients.
	ReplicatedPacketData = PacketData;
	//ClientRPCReceivePackets(ThePacket);//make all connected machines call this to do something with the packet
	OnRep_PacketData(); //force server to call the replication callback since it wont fire for server automatically
}

bool AProteusLocalAvatar::ServerHandleAvatarPacket_Validate(const FOculusAvatarPacket& PacketData)
{
		return true;
}

//this function should be called on all client instances when the server has updated the R_AvatarPacket property meaning there is a new packet to handle.
//we also call this function explicitly on the server to process packet data
void AProteusLocalAvatar::OnRep_PacketData()
{
	bAvatarIsLocal = (Controller && Controller->IsLocalController());
	if ((!bAvatarIsLocal) && AvatarComponent)
	{
		const TArray<uint8>& PacketData = ReplicatedPacketData.AvatarPacketData;

		// Read ovrAvatarPacket from buffer
		if (ovrAvatarPacket* const NewPacket = ovrAvatarPacket_Read(PacketData.Num(), PacketData.GetData()))
		{
			// Clear old ovrAvatarPacket
			ovrAvatarPacket_Free(CurrentPacket);

			CurrentPacket = NewPacket;

			CurrentPacketTime = 0.001f;
			CurrentPacketLength = ovrAvatarPacket_GetDurationSeconds(CurrentPacket);

			AvatarComponent->UpdateFromPacket(CurrentPacket, CurrentPacketTime);
		}
	}
}

//	Replication List
void AProteusLocalAvatar::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicate to everyone
	DOREPLIFETIME_CONDITION(AProteusLocalAvatar, ReplicatedPacketData, COND_SkipOwner);
	DOREPLIFETIME(AProteusLocalAvatar, OculusID);
	DOREPLIFETIME(AProteusLocalAvatar, IsUsingAvatars);
}

void AProteusLocalAvatar::SetRightHandTransform(TArray<float> LocalTransforms)
{
	ovrAvatarTransform gAvatarRightHandTrans[] =

	{
	{ { 0.0000f,0.0000f,0.0000f },{ 0.0000f, 0.0000f, 0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } }, // Hand World
	{ { 0.0400f,-0.0250f,0.1091f },{ 0.0411f, -0.7085f, 0.1749f, 0.6825f },{ 1.0000f,1.0000f,1.0000f } }, // Hand
	{ { -0.0735f,0.0140f,0.0240f },{ -0.5702f, -0.0164f, 0.8065f, -0.1554f },{ 1.0000f,1.0000f,1.0000f } }, // Hand Ignore
	{ { -0.1361f,0.0000f,-0.0000f },{ -0.2270f, -0.0000f, 0.0000f, 0.9739f },{ 1.0000f,1.0000f,1.0000f } }, // Hand Grip

	{ { -0.0871f,0.0351f,-0.0068f },{ LocalTransforms[43],LocalTransforms[44],LocalTransforms[45],LocalTransforms[46] },{ 1.0000f,1.0000f,1.0000f } }, //Index
	{ { 0.0416f,-0.0000f,0.0000f },{ LocalTransforms[53],LocalTransforms[54],LocalTransforms[55],LocalTransforms[56] },{ 1.0000f,1.0000f,1.0000f } },
	{ { 0.0222f,-0.0000f,0.0000f },{ LocalTransforms[63],LocalTransforms[64],LocalTransforms[65],LocalTransforms[66] },{ 1.0000f,1.0000f,1.0000f } },
	{ { 0.0291f,0.0000f,-0.0000f },{ -0.0000f, -0.0000f, -0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },

	{ { -0.0914f,0.0095f,-0.0108f },{ LocalTransforms[83],LocalTransforms[84],LocalTransforms[85],LocalTransforms[86] },{ 1.0000f,1.0000f,1.0000f } }, //Middle
	{ { 0.0460f,0.0000f,0.0000f },{ LocalTransforms[93],LocalTransforms[94],LocalTransforms[95],LocalTransforms[96] },{ 1.0000f,1.0000f,1.0000f } },
	{ { 0.0296f,-0.0000f,0.0000f },{ LocalTransforms[103],LocalTransforms[104],LocalTransforms[105],LocalTransforms[106] },{ 1.0000f,1.0000f,1.0000f } },
	{ { 0.0265f,0.0000f,-0.0000f },{ 0.0000f, -0.0000f, 0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },

	{ { -0.0313f,-0.0191f,0.0115f },{ LocalTransforms[123],LocalTransforms[124],LocalTransforms[125],LocalTransforms[126] },{ 1.0000f,1.0000f,1.0000f } }, //Pinky
	{ { -0.0536f,-0.0024f,-0.0015f },{ LocalTransforms[133],LocalTransforms[134],LocalTransforms[135],LocalTransforms[136] },{ 1.0000f,1.0000f,1.0000f } },
	{ { 0.0334f,0.0000f,-0.0000f },{ LocalTransforms[143],LocalTransforms[144],LocalTransforms[145],LocalTransforms[146] },{ 1.0000f,1.0000f,1.0000f } },
	{ { 0.0174f,-0.0000f,0.0000f },{ LocalTransforms[153],LocalTransforms[154],LocalTransforms[155],LocalTransforms[156] },{ 1.0000f,1.0000f,1.0000f } },
	{ { 0.0194f,0.0000f,-0.0000f },{ 0.0000f, 0.0000f, 0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },

	{ { -0.0895f,-0.0127f,-0.0019f },{ LocalTransforms[173],LocalTransforms[174],LocalTransforms[175],LocalTransforms[176] },{ 1.0000f,1.0000f,1.0000f } }, //Ring
	{ { 0.0386f,0.0000f,0.0000f },{ LocalTransforms[183],LocalTransforms[184],LocalTransforms[185],LocalTransforms[186] },{ 1.0000f,1.0000f,1.0000f } },
	{ { 0.0258f,0.0000f,-0.0000f },{ LocalTransforms[193],LocalTransforms[194],LocalTransforms[195],LocalTransforms[196] },{ 1.0000f,1.0000f,1.0000f } },
	{ { 0.0242f,-0.0000f,-0.0000f },{ 0.0000f, 0.0000f, -0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },

	{ { -0.0309f,0.0415f,0.0206f },{ LocalTransforms[213],LocalTransforms[214],LocalTransforms[215],LocalTransforms[216] },{ 1.0000f,1.0000f,1.0000f } }, //Thumb
	{ { 0.0326f,0.0000f,0.0000f },{ LocalTransforms[223],LocalTransforms[224],LocalTransforms[225],LocalTransforms[226] },{ 1.0000f,1.0000f,1.0000f } },
	{ { 0.0264f,-0.0000f,-0.0000f },{ LocalTransforms[233],LocalTransforms[234],LocalTransforms[235],LocalTransforms[236] },{ 1.0000f,1.0000f,1.0000f } },
	{ { 0.0341f,0.0000f,-0.0000f },{ 0.0000f, -0.0000f, -0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },
	};

	RightHandPoseIndex = eHandPoseState::Custom;
	AvatarComponent->SetCustomGesture(UOvrAvatar::HandType_Right, gAvatarRightHandTrans, HAND_JOINTS);
}

void AProteusLocalAvatar::ResetRightHandTransform()
{
	RightHandPoseIndex = eHandPoseState::Default;
	AvatarComponent->SetRightHandPose(ovrAvatarHandGesture_Default);
}

void AProteusLocalAvatar::SetLeftHandTransform(TArray<float> LocalTransforms)
{
	ovrAvatarTransform gAvatarLeftHandTrans[] =

	{
		{ { 0.0000f,0.0000f,0.0000f },{ 0.0000f, 0.0000f, 0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } }, // Hand World
		{ { -0.0400f,-0.0250f,0.1091f },{ -0.6825f, 0.1749f, 0.7085f, 0.0411f },{ 1.0000f,1.0000f,1.0000f } }, // Hand
		{ { 0.0735f,-0.0140f,-0.0240f },{ -0.3899f, 0.7092f, -0.1519f, 0.5674f },{ 1.0000f,1.0000f,1.0000f } }, // Hand Ignore
		{ { 0.1361f,0.0000f,-0.0000f },{ -0.2270f, 0.0000f, -0.0000f, 0.9739f },{ 1.0000f,1.0000f,1.0000f } }, // Hand Grip

		{ { 0.0871f,-0.0351f,0.0068f },{ LocalTransforms[43],LocalTransforms[44],LocalTransforms[45],LocalTransforms[46] },{ 1.0000f,1.0000f,1.0000f } }, //Index
		{ { -0.0416f,-0.0000f,-0.0000f },{ LocalTransforms[53],LocalTransforms[54],LocalTransforms[55],LocalTransforms[56] },{ 1.0000f,1.0000f,1.0000f } },
		{ {- 0.0222f,0.0000f,0.0000f },{ LocalTransforms[63],LocalTransforms[64],LocalTransforms[65],LocalTransforms[66] },{ 1.0000f,1.0000f,1.0000f } },
		{ { -0.0291f,0.0000f,0.0000f },{ 0.0000f, 0.0000f, 0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },

		{ { 0.0914f,-0.0095f,0.0108f },{ LocalTransforms[83],LocalTransforms[84],LocalTransforms[85],LocalTransforms[86] },{ 1.0000f,1.0000f,1.0000f } }, //Middle
		{ { -0.0460f,-0.0000f,-0.0000f },{ LocalTransforms[93],LocalTransforms[94],LocalTransforms[95],LocalTransforms[96] },{ 1.0000f,1.0000f,1.0000f } },
		{ { -0.0296f,0.0000f,0.0000f },{ LocalTransforms[103],LocalTransforms[104],LocalTransforms[105],LocalTransforms[106] },{ 1.0000f,1.0000f,1.0000f } },
		{ { -0.0265f,0.0000f,-0.0000f },{ -0.0000f, 0.0000f, -0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },

		{ { 0.0313f,0.0191f,-0.0115f },{ LocalTransforms[123],LocalTransforms[124],LocalTransforms[125],LocalTransforms[126] },{ 1.0000f,1.0000f,1.0000f } }, //Pinky
		{ { 0.0536f,0.0024f,0.0015f },{ LocalTransforms[133],LocalTransforms[134],LocalTransforms[135],LocalTransforms[136] },{ 1.0000f,1.0000f,1.0000f } },
		{ {- 0.0334f,0.0000f,-0.0000f },{ LocalTransforms[143],LocalTransforms[144],LocalTransforms[145],LocalTransforms[146] },{ 1.0000f,1.0000f,1.0000f } },
		{ { -0.0174f,-0.0000f,0.0000f },{ LocalTransforms[153],LocalTransforms[154],LocalTransforms[155],LocalTransforms[156] },{ 1.0000f,1.0000f,1.0000f } },
		{ { -0.0194f,0.0000f,0.0000f },{ 0.0000f, 0.0000f, 0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },

		{ { 0.0895f,0.0127f,0.0019f },{ LocalTransforms[173],LocalTransforms[174],LocalTransforms[175],LocalTransforms[176] },{ 1.0000f,1.0000f,1.0000f } }, //Ring
		{ { -0.0386f,0.0000f,-0.0000f },{ LocalTransforms[183],LocalTransforms[184],LocalTransforms[185],LocalTransforms[186] },{ 1.0000f,1.0000f,1.0000f } },
		{ { -0.0258f,-0.0000f,0.0000f },{ LocalTransforms[193],LocalTransforms[194],LocalTransforms[195],LocalTransforms[196] },{ 1.0000f,1.0000f,1.0000f } },
		{ { -0.0242f,-0.0000f,0.0000f },{ -0.0000f, -0.0000f, -0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },

		{ { 0.0309f,-0.0415f,-0.0206f },{ LocalTransforms[213],LocalTransforms[214],LocalTransforms[215],LocalTransforms[216] },{ 1.0000f,1.0000f,1.0000f } }, //Thumb
		{ { -0.0326f,0.0000f,-0.0000f },{ LocalTransforms[223],LocalTransforms[224],LocalTransforms[225],LocalTransforms[226] },{ 1.0000f,1.0000f,1.0000f } },
		{ { -0.0264f,0.0000f,-0.0000f },{ LocalTransforms[233],LocalTransforms[234],LocalTransforms[235],LocalTransforms[236] },{ 1.0000f,1.0000f,1.0000f } },
		{ { -0.0341f,0.0000f,0.0000f },{ -0.0000f, -0.0000f, 0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },
	};

	LeftHandPoseIndex = eHandPoseState::Custom;
	AvatarComponent->SetCustomGesture(UOvrAvatar::HandType_Left, gAvatarLeftHandTrans, HAND_JOINTS);
}

void AProteusLocalAvatar::ResetLeftHandTransform()
{
	LeftHandPoseIndex = eHandPoseState::Default;
	AvatarComponent->SetLeftHandPose(ovrAvatarHandGesture_Default);
}

void AProteusLocalAvatar::SetLeftHandVisibility(bool LeftHandVisible)
{
	AvatarComponent->SetLeftHandVisible(LeftHandVisible);
}

void AProteusLocalAvatar::SetRightHandVisibility(bool RightHandVisible)
{
	AvatarComponent->SetRightHandVisible(RightHandVisible);
}

void AProteusLocalAvatar::SetControllersVisibility(bool ControllersVisible)
{
	AvatarComponent->SetHandsOnControllers(ControllersVisible);
}

void AProteusLocalAvatar::LipSyncVismesReady()
{
		AvatarComponent->UpdateVisemeValues(LipSyncComponent->GetVisemes(), LipSyncComponent->GetLaughterScore());
}

UOvrAvatar::MaterialType AProteusLocalAvatar::GetOvrAvatarMaterialFromType(AvatarMaterial material)
{
	switch (material)
	{
	case AvatarMaterial::Masked:
		return UOvrAvatar::MaterialType::Masked;
	case AvatarMaterial::Translucent:
		return UOvrAvatar::MaterialType::Translucent;
	case AvatarMaterial::Opaque:
		return UOvrAvatar::MaterialType::Opaque;
	}

	return UOvrAvatar::MaterialType::Opaque;
}

void AProteusLocalAvatar::AvatarVisibility()
{
	AvatarComponent->SetVisibilityType(AvatarVisibilityType == AvatarVisibility::FirstPerson ? ovrAvatarVisibilityFlag_FirstPerson : ovrAvatarVisibilityFlag_ThirdPerson);
}