
#include "ProteusLocalAvatar.h"
#include "ProteusOvrAvatarManager.h"
// \Plugins\Runtime\Oculus\OculusAvatar\Source\Public\OvrAvatarManager.h
#include "Object.h"
// D:\Source\Runtime\CoreUObject\Public\UObject\Object.h

static const uint32_t HAND_JOINTS = 25;

AProteusLocalAvatar::AProteusLocalAvatar()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("LocalAvatarRoot"));
	AvatarComponent = CreateDefaultSubobject<UProteusOvrAvatar>(TEXT("LocalAvatar"));

	PrimaryActorTick.bCanEverTick = true;
	AutoPossessPlayer = EAutoReceiveInput::Disabled;

	bReplicates = true; //pawn is a replicated to clients
	bAlwaysRelevant = true; //pawn is always in scope for replication
	bReplicateMovement = true; //server will automatically pass along movement data to clients  (clients still need to tell server they are moving)
	bNetLoadOnClient = true; //probably not needed
	NetPriority = 5.0f;
}

void AProteusLocalAvatar::BeginPlay()
{
	Super::BeginPlay();
}

void AProteusLocalAvatar::BeginDestroy()
{
	Super::BeginDestroy();
	FProteusOvrAvatarManager::Get().UnregisterRemoteAvatar(PacketKey);
}

void AProteusLocalAvatar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	bAvatarIsLocal = (Controller && Controller->IsLocalController());

	if ((bAvatarIsLocal) && AvatarComponent)
	{
		UpdatePacketRecording(DeltaTime);
	}

	if ((!bAvatarIsLocal) && AvatarComponent)
	{
		LatencyTick += DeltaTime;
		//this is a remote avatar so we need to get packets from the local AvatarManager queue
		if (!CurrentPacket && LatencyTick > BufferLatency)
		{
			//we don't have a packet, go get one from the local queue
			CurrentPacket = FProteusOvrAvatarManager::Get().RequestAvatarPacket(PacketKey);
			//if (!CurrentPacket)
				//UE_LOG_ONLINE(Verbose, TEXT("REMOTEAVATAR::Tick Tried to get a packet but got NULL - Remote Avatar - packetKey: %s"), *PacketKey);
		}

		if (CurrentPacket)
		{
			//SetActorHiddenInGame(false);//do we need this?
			//UE_LOG_ONLINE(VeryVerbose, TEXT("REMOTEAVATAR::Tick we have a packet for remote avatar and playing back - packetKey: %s"), *PacketKey);
			const float PacketLength = ovrAvatarPacket_GetDurationSeconds(CurrentPacket);
			AvatarComponent->UpdateFromPacket(CurrentPacket, FMath::Min(PacketLength, CurrentPacketTime));
			CurrentPacketTime += DeltaTime;

			if (CurrentPacketTime > PacketLength)
			{
				ovrAvatarPacket_Free(CurrentPacket);
				CurrentPacket = nullptr;
				CurrentPacketTime = CurrentPacketTime - PacketLength;
			}
		}
	}
}

//FETCH AVATAR ID & LOGIN
void AProteusLocalAvatar::FetchAvatarID()
{
		UE_LOG(LogTemp, Warning, TEXT("LOCALAVATAR_FetchAvatarID has fired!"));
		UE_LOG(LogTemp, Warning, TEXT("LOCALAVATAR_PartA is %d, PartB is %d, PartC is %d"), PartA, PartB, PartC);

	//CONVERT STRING TO INT64, RETURNS INT64 finalAvatar
		ConvertAvatarID(PartA, PartB, PartC);

	ovrAvatarAssetLevelOfDetail LevelofDetail;
	if (AvatarLevelofDetail == EAvatarLevelOfDetail::AvatarLevelOfDetail_One) { LevelofDetail = ovrAvatarAssetLevelOfDetail_One; }
	else if (AvatarLevelofDetail == EAvatarLevelOfDetail::AvatarLevelOfDetail_Three) { LevelofDetail = ovrAvatarAssetLevelOfDetail_Three; }
	else { LevelofDetail = ovrAvatarAssetLevelOfDetail_Five; }

	AvatarComponent->RequestAvatar(finalAvatar, LevelofDetail, bUseCombinedMeshes);
	UE_LOG(LogTemp, Warning, TEXT("LOCALAVATAR_AVATAR REQUESTED is %llu"), finalAvatar);
	StringAvatarID(finalAvatar);
	UE_LOG(LogTemp, Warning, TEXT("LOCALAVATAR_StartPacketRecording has fired!"));
	AvatarComponent->StartPacketRecording();
	UE_LOG(LogTemp, Warning, TEXT("LOCALAVATAR_PacketKey Name: %s"), *PacketKey);

	AvatarComponent->SetVisibilityType(ovrAvatarVisibilityFlag_FirstPerson);
	UE_LOG(LogTemp, Warning, TEXT("LOCALAVATAR_Visibility THIRD PERSON"));
	AvatarComponent->SetPlayerType(UProteusOvrAvatar::ePlayerType::Local);
	SetActorHiddenInGame(false);
	bLocalAvatarIsLoaded = true;
	UE_LOG(LogTemp, Warning, TEXT("LOCALAVATAR_IS FINALLY LOADED"));

		AvatarHands[UProteusOvrAvatar::HandType_Left] = nullptr;
		AvatarHands[UProteusOvrAvatar::HandType_Right] = nullptr;

		Online::GetVoiceInterface()->StartNetworkedVoice(0);
		UE_LOG(LogTemp, Warning, TEXT("LOCALAVATAR_StartNetworkedVoice has fired!"));
}

const int64 AProteusLocalAvatar::ConvertAvatarID(int32 PartG, int32 PartH, int32 PartI)
{
		UE_LOG(LogTemp, Warning, TEXT("ConvertAvatarID has fired!"));
		UE_LOG(LogTemp, Warning, TEXT("ConvertAvatarID PartG(rough) is %d, PartH(rough) is %d, PartI(rough) is %d"), PartG, PartH, PartI);

	int64 final1 = (int64)PartG;
	int64 final2 = (int64)PartH;
	int64 final3 = (int64)PartI;

	//finalAvatar = 2423817077660833;
	//finalAvatar = 1376786192353530;
	finalAvatar = final1 * 10000000000 + final2 * 100000 + final3;

		UE_LOG(LogTemp, Warning, TEXT("ConvertAvatarID PartG (final) is %d, PartH (final) is %d, PartI (final) is %d"), final1, final2, final3);

	return finalAvatar;
}

const FString AProteusLocalAvatar::StringAvatarID(const int64 AvatarIDPre)
{
	bAvatarIsLocal = (Controller && Controller->IsLocalController());
	if ((bAvatarIsLocal))
	{
		UE_LOG(LogTemp, Warning, TEXT("LOCALAVATAR_StringAvatarID has fired!"));
		UE_LOG(LogTemp, Warning, TEXT("LOCALAVATAR_AvatarIDPre to convert to string is %llu"), AvatarIDPre);

		char temp[21];
		//sprintf_s(temp, "%llu", AvatarIDPre);
		sprintf(temp, "%llu", AvatarIDPre);
		PacketKey = temp;
		UE_LOG(LogTemp, Warning, TEXT("LOCALAVATAR_PacketKey is now %s"), *PacketKey);
		return PacketKey;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("REMOTEAVATAR_StringAvatarID has fired!"));
		UE_LOG(LogTemp, Warning, TEXT("REMOTEAVATAR_AvatarIDPre to convert to string is %llu"), AvatarIDPre);

		char temp[21];
		sprintf(temp, "%llu", AvatarIDPre);
		PacketKey = temp;
		UE_LOG(LogTemp, Warning, TEXT("REMOTEAVATAR_PacketKey is now %s"), *PacketKey);
		return PacketKey;
	}
}

void AProteusLocalAvatar::OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
{
	IOnlineIdentityPtr OculusIdentityInterface = Online::GetIdentityInterface();
	OculusIdentityInterface->ClearOnLoginCompleteDelegate_Handle(0, OnLoginCompleteDelegateHandle);
	
		ovrAvatarAssetLevelOfDetail LevelofDetail;
		if (AvatarLevelofDetail == EAvatarLevelOfDetail::AvatarLevelOfDetail_One) { LevelofDetail = ovrAvatarAssetLevelOfDetail_One; }
		else if (AvatarLevelofDetail == EAvatarLevelOfDetail::AvatarLevelOfDetail_Three) { LevelofDetail = ovrAvatarAssetLevelOfDetail_Three; }
		else { LevelofDetail = ovrAvatarAssetLevelOfDetail_Five; }

		AvatarComponent->RequestAvatar(finalAvatar, LevelofDetail, bUseCombinedMeshes);
		UE_LOG(LogTemp, Warning, TEXT("LOCALAVATAR_AVATAR REQUESTED is %llu"), finalAvatar);
		StringAvatarID(finalAvatar);
		UE_LOG(LogTemp, Warning, TEXT("LOCALAVATAR_StartPacketRecording has fired!"));
		AvatarComponent->StartPacketRecording();
		UE_LOG(LogTemp, Warning, TEXT("LOCALAVATAR_PacketKey Name: %s"), *PacketKey);

		AvatarComponent->SetVisibilityType(ovrAvatarVisibilityFlag_FirstPerson);
		UE_LOG(LogTemp, Warning, TEXT("LOCALAVATAR_Visibility THIRD PERSON"));
		AvatarComponent->SetPlayerType(UProteusOvrAvatar::ePlayerType::Local);
		SetActorHiddenInGame(false);
		bLocalAvatarIsLoaded = true;
		UE_LOG(LogTemp, Warning, TEXT("LOCALAVATAR_IS FINALLY LOADED"));
}

void AProteusLocalAvatar::UpdatePacketRecording(float DeltaTime)
{
	if (!PacketSettings.Initialized)
		{
		AvatarComponent->StartPacketRecording();
		PacketSettings.AccumulatedTime = 0.f;
		PacketSettings.RecordingFrames = true;
		PacketSettings.Initialized = true;
		}

	if (PacketSettings.RecordingFrames)
		{
		PacketSettings.AccumulatedTime += DeltaTime;
		ovrAvatarPacket* packet = AvatarComponent->EndPacketRecording();

		if (packet == nullptr)
			{
			//nothing to do, so start recording again
			//UE_LOG_ONLINE(VeryVerbose, TEXT("LOCALAVATAR::UpdatePacketRecording - packet == nullptr"));
			AvatarComponent->StartPacketRecording();
			return;
			}

		//UE_LOG_ONLINE(VeryVerbose, TEXT("LOCALAVATAR::UpdatePacketRecording - created a packet"));
		uint8_t* Buffer;
		uint32_t BufferSize;
		/// Get the size of a recorded packet.
		/// \param packet opaque pointer to the packet
		/// \return size of the packet in bytes
		BufferSize = ovrAvatarPacket_GetSize(packet);
		//UE_LOG_ONLINE(VeryVerbose, TEXT("LOCALAVATAR::UpdatePacketRecording - BufferSize =   ovrAvatarPacket_GetSize(): %d"), BufferSize);
		Buffer = new uint8_t[BufferSize];
		/// Write a packet to a buffer.
		/// \param packet opaque pointer to the packet
		/// \param targetSize size of the buffer to write to
		/// \param targetBuffer the buffer to write to
		/// \return size of the packet in bytes

		if (ovrAvatarPacket_Write(packet, BufferSize, Buffer))
			{
			//UE_LOG_ONLINE(VeryVerbose, TEXT("LOCALAVATAR::UpdatePacketRecording - packing USTRUCT"), BufferSize);
			//pack data into our Struct array
			CurrentPacketStruct.AvatarPacketData.Append(Buffer, BufferSize);
			CurrentPacketStruct.packetSequenceNumber = NextAvatarSequenceNumber++;
			//UE_LOG(LogTemp, Warning, TEXT("LOCALAVATAR packetSequenceNumber is %d and PacketKey is %s"), CurrentPacketStruct.packetSequenceNumber, *PacketKey);
			
			//now send our UStruct to the server in the RPC call:
			//UE_LOG_ONLINE(VeryVerbose, TEXT("LOCALAVATAR::UpdatePacketRecording - Calling HandleAvatarPacket()"));
			ServerHandleAvatarPacket(CurrentPacketStruct); //RPC Call to server with USTRUCT data.
			//clear out the current packet data
			CurrentPacketStruct.AvatarPacketData.Empty();
			//CurrentPacketStruct = {};
			delete[] Buffer;
			//packet handled, free it up and start recording again
			ovrAvatarPacket_Free(packet);
			AvatarComponent->StartPacketRecording();
			}
		//else
		//UE_LOG_ONLINE(VeryVerbose, TEXT("LOCALAVATAR::UpdatePacketRecording - FAILED on ovrAvatarPacket_Write Packet: %llu"), packet);
		}
	
}

void AProteusLocalAvatar::FetchRemoteAvatar()
{
	UE_LOG(LogTemp, Warning, TEXT("REMOTEAVATAR_FetchRemoteAvatar has fired!"));
	UE_LOG(LogTemp, Warning, TEXT("REMOTEVATAR_FetchRemoteAvatar PartA is %d, PartB is %d, PartC is %d"), PartA, PartB, PartC);

	ConvertAvatarID(PartA, PartB, PartC);

	ovrAvatarAssetLevelOfDetail LevelofDetail;
	if (AvatarLevelofDetail == EAvatarLevelOfDetail::AvatarLevelOfDetail_One) {LevelofDetail = ovrAvatarAssetLevelOfDetail_One;}
	else if (AvatarLevelofDetail == EAvatarLevelOfDetail::AvatarLevelOfDetail_Three) {LevelofDetail = ovrAvatarAssetLevelOfDetail_Three;}
	else {LevelofDetail = ovrAvatarAssetLevelOfDetail_Five;}

	AvatarComponent->RequestAvatar(finalAvatar, LevelofDetail, bUseCombinedMeshes);
	//UE_LOG(LogTemp, Warning, TEXT("REMOTEAVATAR_AVATAR REQUESTED is %llu"), finalAvatar);
	StringAvatarID(finalAvatar);
	AvatarComponent->SetVisibilityType(ovrAvatarVisibilityFlag_ThirdPerson);
	AvatarComponent->SetPlayerType(UProteusOvrAvatar::ePlayerType::Remote);
	
	//UE_LOG(LogTemp, Warning, TEXT("REMOTEAVATAR_Visibility REMOTE"));
	SetActorHiddenInGame(false);
	bRemoteAvatarIsLoaded = true;
	//UE_LOG(LogTemp, Warning, TEXT("REMOTEAVATAR_IS FINALLY LOADED"));
	FProteusOvrAvatarManager::Get().RegisterRemoteAvatar(PacketKey);
}

void AProteusLocalAvatar::ServerHandleAvatarPacket_Implementation(FAvatarPacket3 ThePacket)
{
	//The packet has already been validated to contain data.  Updated our replicated property to fire the callback on all clients.
	R_AvatarPacket = ThePacket;
	//ClientRPCReceivePackets(ThePacket);//make all connected machines call this to do something with the packet
	OnRep_ReceivedPacket(); //force server to call the replication callback since it wont fire for server automatically
}

bool AProteusLocalAvatar::ServerHandleAvatarPacket_Validate(FAvatarPacket3 ThePacket)
{
	if ((ThePacket.AvatarPacketData.Num() > 0) /*&& (!ThePacket.PacketKey.IsEmpty())*/)
	{
		return true;
	}
	return false;
}

//this function should be called on all client instances when the server has updated the R_AvatarPacket property meaning there is a new packet to handle.
//we also call this function explicitly on the server to process packet data
void AProteusLocalAvatar::OnRep_ReceivedPacket()
{
	bAvatarIsLocal = (Controller && Controller->IsLocalController());
	if ((!bAvatarIsLocal))
	{
		uint32_t BufferSize;
		uint8_t* Buffer;
		BufferSize = R_AvatarPacket.AvatarPacketData.Num();
		//UE_LOG(LogTemp, Warning, TEXT("REMOTEAVATAR_BufferSize is %d"), BufferSize);

		Buffer = new uint8_t[BufferSize];
		for (uint32_t elementIdx = 0; elementIdx < BufferSize; elementIdx++)
		{
			Buffer[elementIdx] = R_AvatarPacket.AvatarPacketData[elementIdx];
		}

		//UE_LOG(LogTemp, Warning, TEXT("REMOTEAVATAR_OnRepReceivedPackets with %s!"), *PacketKey);
		FProteusOvrAvatarManager::Get().QueueAvatarPacket(Buffer, BufferSize, PacketKey, R_AvatarPacket.packetSequenceNumber);
		delete[] Buffer;
	}
}

//	Replication List
void AProteusLocalAvatar::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Replicate to everyone
	DOREPLIFETIME_CONDITION(AProteusLocalAvatar, R_AvatarPacket, COND_SkipOwner);
	DOREPLIFETIME(AProteusLocalAvatar, PartA);
	DOREPLIFETIME(AProteusLocalAvatar, PartB);
	DOREPLIFETIME(AProteusLocalAvatar, PartC);
	DOREPLIFETIME(AProteusLocalAvatar, bUseCombinedMeshes);
	DOREPLIFETIME(AProteusLocalAvatar, AvatarLevelofDetail);
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
	AvatarComponent->SetCustomGesture(UProteusOvrAvatar::HandType_Right, gAvatarRightHandTrans, HAND_JOINTS);
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
	AvatarComponent->SetCustomGesture(UProteusOvrAvatar::HandType_Left, gAvatarLeftHandTrans, HAND_JOINTS);
}

void AProteusLocalAvatar::ResetLeftHandTransform()
{
	LeftHandPoseIndex = eHandPoseState::Default;
	AvatarComponent->SetLeftHandPose(ovrAvatarHandGesture_Default);
}