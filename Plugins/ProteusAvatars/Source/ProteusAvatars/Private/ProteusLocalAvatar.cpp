
#include "ProteusLocalAvatar.h"
#include "ProteusOvrAvatarManager.h"
// \Plugins\Runtime\Oculus\OculusAvatar\Source\Public\OvrAvatarManager.h
#include "Object.h"
// D:\Source\Runtime\CoreUObject\Public\UObject\Object.h

static const uint32_t HAND_JOINTS = 25;

ovrAvatarTransform gAvatarLeftHandTrans[] =
{
	{ { 0.0000f,0.0000f,0.0000f },{ 0.0000f, 0.0000f, 0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },
	{ { -0.0400f,-0.0250f,0.1091f },{ -0.6825f, 0.1749f, 0.7085f, 0.0411f },{ 1.0000f,1.0000f,1.0000f } },
	{ { 0.0735f,-0.0140f,-0.0240f },{ -0.3899f, 0.7092f, -0.1519f, 0.5674f },{ 1.0000f,1.0000f,1.0000f } },
	{ { 0.1361f,0.0000f,-0.0000f },{ -0.2270f, 0.0000f, -0.0000f, 0.9739f },{ 1.0000f,1.0000f,1.0000f } },

	{ { 0.0871f,-0.0351f,0.0068f },{ -0.3804f, 0.6000f, -0.5778f, -0.4017f },{ 1.0000f,1.0000f,1.0000f } }, //Index
	{ { -0.0416f,-0.0000f,-0.0000f },{ -0.0000f, 0.0000f, -0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },
	{ { -0.0222f,0.0000f,0.0000f },{ -0.0000f, 0.0000f, -0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },
	{ { -0.0291f,0.0000f,0.0000f },{ 0.0000f, 0.0000f, 0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },

	{ { 0.0914f,-0.0095f,0.0108f },{ 0.4631f, -0.4423f, 0.5945f, 0.4863f },{ 1.0000f,1.0000f,1.0000f } }, //Middle
	{ { -0.0460f,-0.0000f,-0.0000f },{ 0.0000f, -0.0000f, -0.8362f, 0.5484f },{ 1.0000f,1.0000f,1.0000f } },
	{ { -0.0296f,0.0000f,0.0000f },{ -0.0000f, -0.0000f, -0.7300f, 0.6834f },{ 1.0000f,1.0000f,1.0000f } },
	{ { -0.0265f,0.0000f,-0.0000f },{ -0.0000f, 0.0000f, -0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },

	{ { 0.0313f,0.0191f,-0.0115f },{ 0.4713f, 0.0618f, 0.0753f, 0.8766f },{ 1.0000f,1.0000f,1.0000f } }, //Pinky
	{ { 0.0536f,0.0024f,0.0015f },{ 0.1300f, 0.0348f, 0.6327f, 0.7626f },{ 1.0000f,1.0000f,1.0000f } },
	{ { -0.0334f,0.0000f,-0.0000f },{ 0.0000f, 0.0000f, 0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },
	{ { -0.0174f,-0.0000f,0.0000f },{ 0.0000f, 0.0000f, 0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },
	{ { -0.0194f,0.0000f,0.0000f },{ 0.0000f, 0.0000f, 0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },

	{ { 0.0895f,0.0127f,0.0019f },{ 0.4589f, -0.3678f, 0.6193f, 0.5203f },{ 1.0000f,1.0000f,1.0000f } }, //Ring
	{ { -0.0386f,0.0000f,-0.0000f },{ -0.0000f, -0.0000f, -0.8446f, 0.5354f },{ 1.0000f,1.0000f,1.0000f } },
	{ { -0.0258f,-0.0000f,0.0000f },{ 0.0000f, -0.0000f, -0.7372f, 0.6757f },{ 1.0000f,1.0000f,1.0000f } },
	{ { -0.0242f,-0.0000f,0.0000f },{ -0.0000f, -0.0000f, -0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },

	{ { 0.0309f,-0.0415f,-0.0206f },{ 0.1999f, 0.9526f, 0.0626f, -0.2205f },{ 1.0000f,1.0000f,1.0000f } }, //Thumb
	{ { -0.0326f,0.0000f,-0.0000f },{ -0.0087f, 0.0964f, -0.2674f, 0.9587f },{ 1.0000f,1.0000f,1.0000f } },
	{ { -0.0264f,0.0000f,-0.0000f },{ -0.0000f, 0.0000f, -0.5985f, 0.8011f },{ 1.0000f,1.0000f,1.0000f } },
	{ { -0.0341f,0.0000f,0.0000f },{ -0.0000f, -0.0000f, 0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },
};

/*ovrAvatarTransform gAvatarRightHandTrans[] =
{
	{ { 0.0000f,0.0000f,0.0000f },{ 0.0000f, 0.0000f, 0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },
	{ { 0.0400f,-0.0250f,0.1091f },{ 0.0411f, -0.7085f, 0.1749f, 0.6825f },{ 1.0000f,1.0000f,1.0000f } },
	{ { -0.0735f,0.0140f,0.0240f },{ -0.5702f, -0.0164f, 0.8065f, -0.1554f },{ 1.0000f,1.0000f,1.0000f } },
	{ { -0.1361f,0.0000f,-0.0000f },{ -0.2270f, -0.0000f, 0.0000f, 0.9739f },{ 1.0000f,1.0000f,1.0000f } },

	{ { -0.0871f,0.0351f,-0.0068f },{ -0.3804f, 0.6000f, -0.5778f, -0.4017f },{ 1.0000f,1.0000f,1.0000f } },
	{ { 0.0416f,-0.0000f,0.0000f },{ 0.0000f, 0.0000f, 0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },
	{ { 0.0222f,-0.0000f,0.0000f },{ 0.0000f, 0.0000f, 0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },
	{ { 0.0291f,0.0000f,-0.0000f },{ -0.0000f, -0.0000f, -0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },

	{ { -0.0914f,0.0095f,-0.0108f },{ 0.4631f, -0.4423f, 0.5945f, 0.4863f },{ 1.0000f,1.0000f,1.0000f } },
	{ { 0.0460f,0.0000f,0.0000f },{ 0.0000f, -0.0000f, -0.8362f, 0.5484f },{ 1.0000f,1.0000f,1.0000f } },
	{ { 0.0296f,-0.0000f,0.0000f },{ 0.0000f, -0.0000f, -0.7300f, 0.6834f },{ 1.0000f,1.0000f,1.0000f } },
	{ { 0.0265f,0.0000f,-0.0000f },{ 0.0000f, -0.0000f, 0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },

	{ { -0.0313f,-0.0191f,0.0115f },{ 0.4713f, 0.0618f, 0.0753f, 0.8766f },{ 1.0000f,1.0000f,1.0000f } },
	{ { -0.0536f,-0.0024f,-0.0015f },{ 0.1300f, 0.0348f, 0.6327f, 0.7626f },{ 1.0000f,1.0000f,1.0000f } },
	{ { 0.0334f,0.0000f,-0.0000f },{ 0.0000f, 0.0000f, 0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },
	{ { 0.0174f,-0.0000f,0.0000f },{ 0.0000f, 0.0000f, 0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },
	{ { 0.0194f,0.0000f,-0.0000f },{ 0.0000f, 0.0000f, 0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },

	{ { -0.0895f,-0.0127f,-0.0019f },{ 0.4589f, -0.3678f, 0.6193f, 0.5203f },{ 1.0000f,1.0000f,1.0000f } },
	{ { 0.0386f,0.0000f,0.0000f },{ -0.0000f, -0.0000f, -0.8446f, 0.5354f },{ 1.0000f,1.0000f,1.0000f } },
	{ { 0.0258f,0.0000f,-0.0000f },{ -0.0000f, 0.0000f, -0.7372f, 0.6757f },{ 1.0000f,1.0000f,1.0000f } },
	{ { 0.0242f,-0.0000f,-0.0000f },{ 0.0000f, 0.0000f, -0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },

	{ { -0.0309f,0.0415f,0.0206f },{ 0.1999f, 0.9526f, 0.0626f, -0.2205f },{ 1.0000f,1.0000f,1.0000f } },
	{ { 0.0326f,0.0000f,0.0000f },{ -0.0087f, 0.0964f, -0.2674f, 0.9587f },{ 1.0000f,1.0000f,1.0000f } },
	{ { 0.0264f,-0.0000f,-0.0000f },{ 0.0000f, -0.0000f, -0.5985f, 0.8011f },{ 1.0000f,1.0000f,1.0000f } },
	{ { 0.0341f,0.0000f,-0.0000f },{ 0.0000f, -0.0000f, -0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },
};*/


//TArray <ovrAvatarTransform> gAvatarRightHandTrans;

//ovrAvatarTransform gAvatarRightHandTrans2[HAND_JOINTS];

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
	//OwningPawn = Cast<APawn>(GetOwner());
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

	/*IOnlineIdentityPtr IdentityInterface = Online::GetIdentityInterface();
	if (IdentityInterface.IsValid())
	{
		OnLoginCompleteDelegateHandle = IdentityInterface->AddOnLoginCompleteDelegate_Handle(0, FOnLoginCompleteDelegate::CreateUObject(this, &AProteusLocalAvatar::OnLoginComplete));
		IdentityInterface->AutoLogin(0);
		UE_LOG(LogTemp, Warning, TEXT("LOCALAVATAR_AutoLogin Identity Interface has fired!"));
	}*/

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
	
	/*
		bool UseCombinedMesh = true;

	#if PLATFORM_ANDROID
		ovrAvatarAssetLevelOfDetail lod = ovrAvatarAssetLevelOfDetail_Three;
		#else
		ovrAvatarAssetLevelOfDetail lod = ovrAvatarAssetLevelOfDetail_Five;
		#endif
	*/

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

/*bool AProteusLocalAvatar::IsAuthoritative() const
{
	return OwningPawn->IsLocallyControlled();
}*/

void AProteusLocalAvatar::FetchRemoteAvatar()
{
	UE_LOG(LogTemp, Warning, TEXT("REMOTEAVATAR_FetchRemoteAvatar has fired!"));
	UE_LOG(LogTemp, Warning, TEXT("REMOTEVATAR_FetchRemoteAvatar PartA is %d, PartB is %d, PartC is %d"), PartA, PartB, PartC);

	ConvertAvatarID(PartA, PartB, PartC);

	//UE_LOG(LogTemp, Warning, TEXT("REMOTEAVATAR_finalAvatar is %llu"), finalAvatar);

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
	//DoRep_PacketReceived = !DoRep_PacketReceived; //trigger replication
	R_AvatarPacket = ThePacket;
	//ClientRPCReceivePackets(ThePacket);//make all connected machines call this to do something with the packet
	//if (HasAuthority())
	//ServerReceivePackets(ThePacket);
	OnRep_ReceivedPacket(); //force server to call the replication callback since it wont fire for server automatically
}

bool AProteusLocalAvatar::ServerHandleAvatarPacket_Validate(FAvatarPacket3 ThePacket)
{
	if ((ThePacket.AvatarPacketData.Num() > 0) /*&& (!ThePacket.PacketKey.IsEmpty())*/)
	{
		//UE_LOG_ONLINE(VeryVerbose, TEXT("LOCALAVATAR::HandleAvatarPacket_Validate - VALIDATED for %s"), *PacketKey);
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

		/*CurrentPacket = ovrAvatarPacket_Read(BufferSize, Buffer);
		const float PacketLength = ovrAvatarPacket_GetDurationSeconds(CurrentPacket);

		if (AvatarComponent)
		{
		AvatarComponent->UpdateFromPacket(CurrentPacket, PacketLength);
		}*/
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

/*ovrAvatarTransform AProteusLocalAvatar::ConvertTransformsOculus(TArray<float> LocalTransforms, int32 Index)
{
	ovrAvatarVector3f position;
	position.x = LocalTransforms[0];
	position.y = LocalTransforms[1];
	position.z = LocalTransforms[2];

	ovrAvatarQuatf orientation;
	orientation.x = LocalTransforms[3];
	orientation.y = LocalTransforms[4];
	orientation.z = LocalTransforms[5];
	orientation.w = LocalTransforms[6];

	ovrAvatarVector3f scale;
	scale.x = LocalTransforms[7];
	scale.y = LocalTransforms[8];
	scale.z = LocalTransforms[9];

	//ovrAvatarTransform ConvertedTransform;

	ConvertedTransform.position = position;
	ConvertedTransform.orientation = orientation;
	ConvertedTransform.scale = scale;

	//gAvatarRightHandTrans[25].orientation.w

	//ovrAvatarTransform gAvatarRightHandTrans[25]

	ovrAvatarTransform gAvatarRightHandTrans4[] =
	{
		{ { LocalTransforms[0],LocalTransforms[1],LocalTransforms[2] },{ LocalTransforms[3],LocalTransforms[4],LocalTransforms[5],LocalTransforms[6] },{ LocalTransforms[7],LocalTransforms[8],LocalTransforms[9] } },
	};

	return gAvatarRightHandTrans4[];
}*/

void AProteusLocalAvatar::SetRightHandTransform(TArray<float> LocalTransforms)
{
	ovrAvatarTransform gAvatarRightHandTrans[] =
	
	{
		{ { LocalTransforms[0],LocalTransforms[1],LocalTransforms[2] },{ LocalTransforms[3],LocalTransforms[4],LocalTransforms[5],LocalTransforms[6] },{ LocalTransforms[7],LocalTransforms[8],LocalTransforms[9] } },
		{ { LocalTransforms[10],LocalTransforms[11],LocalTransforms[12] },{ LocalTransforms[13],LocalTransforms[14],LocalTransforms[15],LocalTransforms[16] },{ LocalTransforms[17],LocalTransforms[18],LocalTransforms[19] } },
		{ { LocalTransforms[20],LocalTransforms[21],LocalTransforms[22] },{ LocalTransforms[23],LocalTransforms[24],LocalTransforms[25],LocalTransforms[26] },{ LocalTransforms[27],LocalTransforms[28],LocalTransforms[29] } },
		{ { LocalTransforms[30],LocalTransforms[31],LocalTransforms[32] },{ LocalTransforms[33],LocalTransforms[34],LocalTransforms[35],LocalTransforms[36] },{ LocalTransforms[37],LocalTransforms[38],LocalTransforms[39] } },

		{ { LocalTransforms[40],LocalTransforms[41],LocalTransforms[42] },{ LocalTransforms[43],LocalTransforms[44],LocalTransforms[45],LocalTransforms[46] },{ LocalTransforms[47],LocalTransforms[48],LocalTransforms[49] } }, //Index
		{ { LocalTransforms[50],LocalTransforms[51],LocalTransforms[52] },{ LocalTransforms[53],LocalTransforms[54],LocalTransforms[55],LocalTransforms[56] },{ LocalTransforms[57],LocalTransforms[58],LocalTransforms[59] } },
		{ { LocalTransforms[60],LocalTransforms[61],LocalTransforms[62] },{ LocalTransforms[63],LocalTransforms[64],LocalTransforms[65],LocalTransforms[66] },{ LocalTransforms[67],LocalTransforms[68],LocalTransforms[69] } },
		{ { LocalTransforms[70],LocalTransforms[71],LocalTransforms[72] },{ LocalTransforms[73],LocalTransforms[74],LocalTransforms[75],LocalTransforms[76] },{ LocalTransforms[77],LocalTransforms[78],LocalTransforms[79] } },

		{ { LocalTransforms[80],LocalTransforms[81],LocalTransforms[82] },{ LocalTransforms[83],LocalTransforms[84],LocalTransforms[85],LocalTransforms[86] },{ LocalTransforms[87],LocalTransforms[88],LocalTransforms[89] } }, //Middle
		{ { LocalTransforms[90],LocalTransforms[91],LocalTransforms[92] },{ LocalTransforms[93],LocalTransforms[94],LocalTransforms[95],LocalTransforms[96] },{ LocalTransforms[97],LocalTransforms[98],LocalTransforms[99] } },
		{ { LocalTransforms[100],LocalTransforms[101],LocalTransforms[102] },{ LocalTransforms[103],LocalTransforms[104],LocalTransforms[105],LocalTransforms[106] },{ LocalTransforms[107],LocalTransforms[108],LocalTransforms[109] } },
		{ { LocalTransforms[110],LocalTransforms[111],LocalTransforms[112] },{ LocalTransforms[113],LocalTransforms[114],LocalTransforms[115],LocalTransforms[116] },{ LocalTransforms[117],LocalTransforms[118],LocalTransforms[119] } },

		{ { LocalTransforms[120],LocalTransforms[121],LocalTransforms[122] },{ LocalTransforms[123],LocalTransforms[124],LocalTransforms[125],LocalTransforms[126] },{ LocalTransforms[127],LocalTransforms[128],LocalTransforms[129] } }, //Pinky
		{ { LocalTransforms[130],LocalTransforms[131],LocalTransforms[132] },{ LocalTransforms[133],LocalTransforms[134],LocalTransforms[135],LocalTransforms[136] },{ LocalTransforms[137],LocalTransforms[138],LocalTransforms[139] } },
		{ { LocalTransforms[140],LocalTransforms[141],LocalTransforms[142] },{ LocalTransforms[143],LocalTransforms[144],LocalTransforms[145],LocalTransforms[146] },{ LocalTransforms[147],LocalTransforms[148],LocalTransforms[149] } },
		{ { LocalTransforms[150],LocalTransforms[151],LocalTransforms[152] },{ LocalTransforms[153],LocalTransforms[154],LocalTransforms[155],LocalTransforms[156] },{ LocalTransforms[157],LocalTransforms[158],LocalTransforms[159] } },
		{ { LocalTransforms[160],LocalTransforms[161],LocalTransforms[162] },{ LocalTransforms[163],LocalTransforms[164],LocalTransforms[165],LocalTransforms[166] },{ LocalTransforms[167],LocalTransforms[168],LocalTransforms[169] } },

		{ { LocalTransforms[170],LocalTransforms[171],LocalTransforms[172] },{ LocalTransforms[173],LocalTransforms[174],LocalTransforms[175],LocalTransforms[176] },{ LocalTransforms[177],LocalTransforms[178],LocalTransforms[179] } }, //Ring
		{ { LocalTransforms[180],LocalTransforms[181],LocalTransforms[182] },{ LocalTransforms[183],LocalTransforms[184],LocalTransforms[185],LocalTransforms[186] },{ LocalTransforms[187],LocalTransforms[188],LocalTransforms[189] } },
		{ { LocalTransforms[190],LocalTransforms[191],LocalTransforms[192] },{ LocalTransforms[193],LocalTransforms[194],LocalTransforms[195],LocalTransforms[196] },{ LocalTransforms[197],LocalTransforms[198],LocalTransforms[199] } },
		{ { LocalTransforms[200],LocalTransforms[201],LocalTransforms[202] },{ LocalTransforms[203],LocalTransforms[204],LocalTransforms[205],LocalTransforms[206] },{ LocalTransforms[207],LocalTransforms[208],LocalTransforms[209] } },

		{ { LocalTransforms[210],LocalTransforms[211],LocalTransforms[212] },{ LocalTransforms[213],LocalTransforms[214],LocalTransforms[215],LocalTransforms[216] },{ LocalTransforms[217],LocalTransforms[218],LocalTransforms[219] } }, //Thumb
		{ { LocalTransforms[220],LocalTransforms[221],LocalTransforms[222] },{ LocalTransforms[223],LocalTransforms[224],LocalTransforms[225],LocalTransforms[226] },{ LocalTransforms[227],LocalTransforms[228],LocalTransforms[229] } },
		{ { LocalTransforms[230],LocalTransforms[231],LocalTransforms[232] },{ LocalTransforms[233],LocalTransforms[234],LocalTransforms[235],LocalTransforms[236] },{ LocalTransforms[237],LocalTransforms[238],LocalTransforms[239] } },
		{ { LocalTransforms[240],LocalTransforms[241],LocalTransforms[242] },{ LocalTransforms[243],LocalTransforms[244],LocalTransforms[245],LocalTransforms[246] },{ LocalTransforms[247],LocalTransforms[248],LocalTransforms[249] } },
	};

	/*{
		{ { 0.0000f, 0.0000f, 0.0000f }, { 0.0000f, 0.0000f, 0.0000f, 1.0000f }, { 1.0000f,1.0000f,1.0000f } },
		{ { 0.0400f,-0.0250f,0.1091f },{ 0.0411f, -0.7085f, 0.1749f, 0.6825f },{ 1.0000f,1.0000f,1.0000f } },
		{ { -0.0735f,0.0140f,0.0240f },{ -0.5702f, -0.0164f, 0.8065f, -0.1554f },{ 1.0000f,1.0000f,1.0000f } },
		{ { -0.1361f,0.0000f,-0.0000f },{ -0.2270f, -0.0000f, 0.0000f, 0.9739f },{ 1.0000f,1.0000f,1.0000f } },

		{ { -0.0871f,0.0351f,-0.0068f },{ -0.3804f, 0.6000f, -0.5778f, -0.4017f },{ 1.0000f,1.0000f,1.0000f } },
		{ { 0.0416f,-0.0000f,0.0000f },{ 0.0000f, 0.0000f, 0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },
		{ { 0.0222f,-0.0000f,0.0000f },{ 0.0000f, 0.0000f, 0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },
		{ { 0.0291f,0.0000f,-0.0000f },{ -0.0000f, -0.0000f, -0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },

		{ { -0.0914f,0.0095f,-0.0108f },{ 0.4631f, -0.4423f, 0.5945f, 0.4863f },{ 1.0000f,1.0000f,1.0000f } },
		{ { 0.0460f,0.0000f,0.0000f },{ 0.0000f, -0.0000f, -0.8362f, 0.5484f },{ 1.0000f,1.0000f,1.0000f } },
		{ { 0.0296f,-0.0000f,0.0000f },{ 0.0000f, -0.0000f, -0.7300f, 0.6834f },{ 1.0000f,1.0000f,1.0000f } },
		{ { 0.0265f,0.0000f,-0.0000f },{ 0.0000f, -0.0000f, 0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },

		{ { -0.0313f,-0.0191f,0.0115f },{ 0.4713f, 0.0618f, 0.0753f, 0.8766f },{ 1.0000f,1.0000f,1.0000f } },
		{ { -0.0536f,-0.0024f,-0.0015f },{ 0.1300f, 0.0348f, 0.6327f, 0.7626f },{ 1.0000f,1.0000f,1.0000f } },
		{ { 0.0334f,0.0000f,-0.0000f },{ 0.0000f, 0.0000f, 0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },
		{ { 0.0174f,-0.0000f,0.0000f },{ 0.0000f, 0.0000f, 0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },
		{ { 0.0194f,0.0000f,-0.0000f },{ 0.0000f, 0.0000f, 0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },

		{ { -0.0895f,-0.0127f,-0.0019f },{ 0.4589f, -0.3678f, 0.6193f, 0.5203f },{ 1.0000f,1.0000f,1.0000f } },
		{ { 0.0386f,0.0000f,0.0000f },{ -0.0000f, -0.0000f, -0.8446f, 0.5354f },{ 1.0000f,1.0000f,1.0000f } },
		{ { 0.0258f,0.0000f,-0.0000f },{ -0.0000f, 0.0000f, -0.7372f, 0.6757f },{ 1.0000f,1.0000f,1.0000f } },
		{ { 0.0242f,-0.0000f,-0.0000f },{ 0.0000f, 0.0000f, -0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },

		{ { -0.0309f,0.0415f,0.0206f },{ 0.1999f, 0.9526f, 0.0626f, -0.2205f },{ 1.0000f,1.0000f,1.0000f } },
		{ { 0.0326f,0.0000f,0.0000f },{ -0.0087f, 0.0964f, -0.2674f, 0.9587f },{ 1.0000f,1.0000f,1.0000f } },
		{ { 0.0264f,-0.0000f,-0.0000f },{ 0.0000f, -0.0000f, -0.5985f, 0.8011f },{ 1.0000f,1.0000f,1.0000f } },
		{ { 0.0341f,0.0000f,-0.0000f },{ 0.0000f, -0.0000f, -0.0000f, 1.0000f },{ 1.0000f,1.0000f,1.0000f } },
	};*/

	RightHandPoseIndex = eHandPoseState::Custom;
	AvatarComponent->SetCustomGesture(UProteusOvrAvatar::HandType_Right, gAvatarRightHandTrans, HAND_JOINTS);
}

//gAvatarRightHandTrans[] = ConvertedTransform[];
//AProteusLocalAvatar::SetHandTransform(int32 Index);
//ovrAvatarTransform gAvatarLeftHandTrans2[HAND_JOINTS];

//TArray <ovrAvatarTransform> gAvatarRightHandTrans2

void AProteusLocalAvatar::ChangeLeftHandPose()
{
	//LeftHandPoseIndex = eHandPoseState::Sphere;
	//AvatarComponent->SetLeftHandPose(ovrAvatarHandGesture_GripSphere);
	//LeftHandPoseIndex = eHandPoseState::Cube;
	//AvatarComponent->SetLeftHandPose(ovrAvatarHandGesture_GripCube);
	LeftHandPoseIndex = eHandPoseState::Custom;
	AvatarComponent->SetCustomGesture(UProteusOvrAvatar::HandType_Left, gAvatarLeftHandTrans, HAND_JOINTS);
}

/*void AProteusLocalAvatar::ChangeRightHandPose()
{
	//RighttHandPoseIndex = eHandPoseState::Sphere;
	//AvatarComponent->SetRightHandPose(ovrAvatarHandGesture_GripSphere);
	//RightHandPoseIndex = eHandPoseState::Cube;
	//AvatarComponent->SetRightHandPose(ovrAvatarHandGesture_GripCube);
	RightHandPoseIndex = eHandPoseState::Custom;
	AvatarComponent->SetCustomGesture(UProteusOvrAvatar::HandType_Right, gAvatarRightHandTrans, HAND_JOINTS);
}*/