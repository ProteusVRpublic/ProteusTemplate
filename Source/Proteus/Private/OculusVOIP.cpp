// Fill out your copyright notice in the Description page of Project Settings.

#include "OculusVOIP.h"


// Sets default values for this component's properties
UOculusVOIP::UOculusVOIP()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UOculusVOIP::BeginPlay()
{
	Super::BeginPlay();

	// ...

}


// Called every frame
void UOculusVOIP::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

bool UOculusVOIP::RegisterRemoteTalker(FString OculusID)
{
	auto uniqueNetId = Online::GetIdentityInterface()->CreateUniquePlayerId(OculusID);
	UE_LOG(LogTemp, Warning, TEXT("Remote Talker is Registered"));
	return Online::GetVoiceInterface()->RegisterRemoteTalker(*uniqueNetId);
}

bool UOculusVOIP::UnregisterRemoteTalker(FString OculusID)
{
	auto uniqueNetId = Online::GetIdentityInterface()->CreateUniquePlayerId(OculusID);
	return Online::GetVoiceInterface()->UnregisterRemoteTalker(*uniqueNetId);
}