// Fill out your copyright notice in the Description page of Project Settings.

#include "Proteus.h"
#include "ProteusRemote.h"
#include "OvrAvatarManager.h"
#include "Object.h"

uint32_t gIDIndex = 0;
//uint64_t OnlineID[] = { 10149999027226798, 10149999027225493, 10149999027224935, 10149999027209743, 1001577267395 };

// Sets default values
AProteusRemote::AProteusRemote()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	AutoPossessPlayer = EAutoReceiveInput::Disabled;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RemoteAvatarRoot"));
	AvatarComponent = CreateDefaultSubobject<UOvrAvatar>(TEXT("RemoteAvatar"));

	AvatarComponent->SetVisibilityType(ovrAvatarVisibilityFlag_ThirdPerson);
	AvatarComponent->SetPlayerType(UOvrAvatar::ePlayerType::Remote);

	BaseEyeHeight = 0.f;
	//AvatarComponent->SetPlayerHeightOffset(BaseEyeHeight / 0.f);
	PacketKey = GetName();
}

// Called when the game starts or when spawned
void AProteusRemote::BeginPlay()
{
	Super::BeginPlay();

	/// CORRECTLY ASSESS
	//AvatarComponent->RequestAvatar(finalRemoteAvatar);

	//FOvrAvatarManager::Get().RegisterRemoteAvatar(PacketKey);

	//SetActorHiddenInGame(true);
}

void AProteusRemote::BeginDestroy()
{
	Super::BeginDestroy();

	FOvrAvatarManager::Get().UnregisterRemoteAvatar(PacketKey);
}

// Called every frame
void AProteusRemote::Tick(float DeltaSeconds)
{
	LatencyTick += DeltaSeconds;

	if (!CurrentPacket && LatencyTick > FakeLatency)
	{
		CurrentPacket = FOvrAvatarManager::Get().RequestAvatarPacket(PacketKey);
		SetActorHiddenInGame(false);
	}

	if (CurrentPacket)
	{
		const float PacketLength = ovrAvatarPacket_GetDurationSeconds(CurrentPacket);
		AvatarComponent->UpdateFromPacket(CurrentPacket, FMath::Min(PacketLength, CurrentPacketTime));
		CurrentPacketTime += DeltaSeconds;

		if (CurrentPacketTime > PacketLength)
		{
			ovrAvatarPacket_Free(CurrentPacket);
			CurrentPacket = nullptr;
			CurrentPacketTime = CurrentPacketTime - PacketLength;
		}
	}

}

void AProteusRemote::FetchRemoteAvatarID(int32 PartD, int32 PartE, int32 PartF, int32 PartG, int32 PartH, int32 PartI)
{
	int32 rough4 = PartD;
	int32 rough5 = PartE;
	int32 rough6 = PartF;
	int32 rough7 = PartG;
	int32 rough8 = PartH;
	int32 rough9 = PartI;

	UE_LOG(LogTemp, Warning, TEXT("RemoteFetchAvatar has fired!"));
	UE_LOG(LogTemp, Warning, TEXT("PartD is %d, PartE is %d, PartF is %d"), PartD, PartE, PartF);
	UE_LOG(LogTemp, Warning, TEXT("PartG is %d, PartH is %d, PartI is %d"), PartG, PartH, PartI);

	AProteusRemote::ConvertRemoteAvatarID(rough4, rough5, rough6);
	//AProteusRemote::ConvertLocalAvatarID(rough7, rough8, rough9);
	
	AvatarComponent->RequestAvatar(finalRemoteAvatar);

	FOvrAvatarManager::Get().RegisterRemoteAvatar(PacketKey);

	SetActorHiddenInGame(false);
	SetActorTickEnabled(true);
	return;
}

int64 AProteusRemote::ConvertRemoteAvatarID(int32 rough4, int32 rough5, int32 rough6)
{
	UE_LOG(LogTemp, Warning, TEXT("ConvertRemoteAvatar has fired!"));
	UE_LOG(LogTemp, Warning, TEXT("PartD(rough) is %d, PartE(rough) is %d, PartF(rough) is %d"), rough4, rough5, rough6);

	int64 final4 = (int64)rough4;
	int64 final5 = (int64)rough5;
	int64 final6 = (int64)rough6;

	finalRemoteAvatar = final4 * 10000000000 + final5 * 100000 + final6;

	UE_LOG(LogTemp, Warning, TEXT("PartD (Remotefinal) is %d, PartE (Remotefinal) is %d, PartF (Remotefinal) is %d"), final4, final5, final6);
	return finalRemoteAvatar;
}

int64 AProteusRemote::ConvertLocalAvatarID(int32 rough7, int32 rough8, int32 rough9)
{
	UE_LOG(LogTemp, Warning, TEXT("ConvertLocalAvatar has fired!"));
	UE_LOG(LogTemp, Warning, TEXT("PartG(rough) is %d, PartH(rough) is %d, PartI(rough) is %d"), rough7, rough8, rough9);

	int64 final7 = (int64)rough7;
	int64 final8 = (int64)rough8;
	int64 final9 = (int64)rough9;

	finalLocalAvatar = final7 * 10000000000 + final8 * 100000 + final9;

	UE_LOG(LogTemp, Warning, TEXT("PartG (Remotelocal) is %d, PartH (Remotelocal) is %d, PartI (Remotelocal) is %d"), final7, final8, final9);
	return finalLocalAvatar;
}




