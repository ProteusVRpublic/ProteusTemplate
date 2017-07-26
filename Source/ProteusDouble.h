// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Online.h"
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "OvrAvatar.h"
#include "Engine/LocalPlayer.h"
#include "ProteusDouble.generated.h"

UCLASS()
class PROTEUS_API AProteusDouble : public APawn
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "ProteusAvatar")
		void FetchAvatarID(int PartA, int PartB, int Partc);

	// Constructor
	AProteusDouble();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	void SetupPlayerInputComponent(UInputComponent* Input) override;

private:

	//VARIABLES
	int32 Part1;
	int32 Part2;
	int32 Part3;
	int64 finalAvatar = 0;
	//int64 MatID = 1376786192353530;
	int64 ConvertAvatarID(int32 Part1, int32 Part2, int32 Part3);

	bool ControllersVisible = true;

	bool bIsPlayingSameGame;

	FVector2D StickPosition[ovrHand_Count];

	enum class eHandPoseState
	{
		Default,
		Sphere,
		Cube,
		Custom,
		Controller,
		Detached,
	};
	eHandPoseState LeftHandPoseIndex = eHandPoseState::Default;
	eHandPoseState RightHandPoseIndex = eHandPoseState::Default;

	//DELEGATES
	FDelegateHandle OnLoginCompleteDelegateHandle;

	//POINTERS
	UOvrAvatar* AvatarComponent = nullptr;
	//The TWeakObjectPtr assignment operator takes a raw C++ pointer to its template type,
	//so for a TWeakObjectPtr<AActor> the assignable type would be
	//AvatarHands array of 2 elements is declared as 
	TWeakObjectPtr<USceneComponent> AvatarHands[ovrHand_Count];

	//FUNCTIONS
	void OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);
	void DriveHand(ovrHandType hand, float DeltaTime);
	void LeftThumbstickX_Value(float value);
	void LeftThumbstickY_Value(float value);
	void RightThumbstickX_Value(float value);
	void RightThumbstickY_Value(float value);

	//DEFINES
#define CUSTOM_ENTRY(entry, hand, field, invert) \
    void entry##_Value( float value);
	CUSTOM_AXIS_TUPLE
#undef CUSTOM_ENTRY

#define INPUT_ENTRY(entry, hand, flag) \
    void entry##_Pressed();\
    void entry##_Released();
		INPUT_COMMAND_TUPLE
#undef INPUT_ENTRY

#define AXIS_ENTRY(entry, hand, flag) \
    void entry##_Value( float value);
		AXIS_INPUT_TUPLE
#undef AXIS_ENTRY

		//SUPPLEMENTAL
		//void SpawnNewRemoteAvatar();
		//void DestroyRemoteAvatar();
		//void CycleRightHandAttach();
		//void CycleLeftHandAttach();

};