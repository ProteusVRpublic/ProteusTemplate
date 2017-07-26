// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Online.h"
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "OvrAvatar.h"
#include "ProteusRemote.generated.h"

UCLASS()
class PROTEUS_API AProteusRemote : public APawn
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "ProteusAvatar")
		void FetchRemoteAvatarID(int PartD, int PartE, int PartF, int PartG, int PartH, int PartI);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProteusAvatar")
		FString OculusKey;

	// Sets default values for this pawn's properties
	AProteusRemote();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void BeginDestroy() override;
	
	// Called every frame
	virtual void Tick(float DeltaSeconds) override;

private:

	UOvrAvatar* AvatarComponent = nullptr;
	ovrAvatarPacket* CurrentPacket = nullptr;

	float CurrentPacketTime = 0.f;
	float FakeLatency = 0.0f;
	float LatencyTick = 0.f;

	FString PacketKey;

	//VARIABLES
	int32 Part4;
	int32 Part5;
	int32 Part6;
	int64 finalRemoteAvatar = 0;
	int64 ConvertRemoteAvatarID(int32 Part4, int32 Part5, int32 Part6);
	int32 Part7;
	int32 Part8;
	int32 Part9;
	int64 finalLocalAvatar = 0;
	int64 ConvertLocalAvatarID(int32 Part7, int32 Part8, int32 Part9);
};