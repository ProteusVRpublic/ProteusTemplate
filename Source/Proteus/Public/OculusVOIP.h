// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Online.h"
#include "OculusVOIP.generated.h"


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROTEUS_API UOculusVOIP : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UOculusVOIP();

	UFUNCTION(BlueprintCallable, Category = "OculusVOIP")
		bool RegisterRemoteTalker(FString OculusID);

	UFUNCTION(BlueprintCallable, Category = "OculusVOIP")
		bool UnregisterRemoteTalker(FString OculusID);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;



};
