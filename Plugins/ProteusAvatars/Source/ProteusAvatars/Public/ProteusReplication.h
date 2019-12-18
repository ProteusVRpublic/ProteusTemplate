
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/NetSerialization.h"
#include "ProteusReplication.generated.h"

//To gain speed, we want to serialize the transforms of the pawn (head & controllers)
USTRUCT(BlueprintType)
struct FTransform_NetQuantize100
{
	GENERATED_BODY()

		UPROPERTY(BlueprintReadWrite)
		FVector Location;

	UPROPERTY(BlueprintReadWrite)
		FRotator Rotation;

	//Initialize Transform
	FTransform_NetQuantize100() : Location(0.f), Rotation(0.f) {};

	//Get vectors values
	FTransform_NetQuantize100(const FVector& InLocation, const FRotator& InRotation)
		: Location(InLocation), Rotation(InRotation) {};

	FTransform_NetQuantize100(const FTransform& InTransform)
		: Location(InTransform.GetLocation()), Rotation(InTransform.Rotator()) {};

	bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
	{
		//Serialize vectors
		//2 decimal place of precision; up to 30 bits per component
		bOutSuccess = SerializePackedVector<100, 30>(Location, Ar);

		//Serialize rotators
		// Rotation compressed to short per component
		Rotation.SerializeCompressedShort(Ar);

		return true;
	};
};

//Tell the engine that struct FTransform_NetQuantize100 defines a custom NetSerializer function
//you have to set to true the type trait WithNetSerializer for the struct
//If you don’t add that piece of code, the NetSerialize method will never be called
template<>
struct TStructOpsTypeTraits<FTransform_NetQuantize100> : public TStructOpsTypeTraitsBase2<FTransform_NetQuantize100>
{
	enum
	{
		WithNetSerializer = true
	};
};

USTRUCT()
struct FProteusReplicationData
{
	GENERATED_BODY()

		UPROPERTY()
		FTransform_NetQuantize100 HeadTransform;

	UPROPERTY()
		FTransform_NetQuantize100 LeftHandTransform;

	UPROPERTY()
		FTransform_NetQuantize100 RightHandTransform;
};

UCLASS(BlueprintType, ClassGroup = (Network), meta = (BlueprintSpawnableComponent), HideCategories = (Activation, Cooking, ComponentReplication, Collision))
class PROTEUSAVATARS_API UProteusReplication : public UActorComponent
{
	GENERATED_BODY()
public:

	UProteusReplication();

	UFUNCTION(BlueprintCallable, Category = "Network")
		bool IsAuthoritative() const;

	UFUNCTION(BlueprintCallable, Category = "Network")
		bool RegisterProteusForReplication(USceneComponent* Head, USceneComponent* LeftHand, USceneComponent* RightHand);

	virtual void SetComponentTickEnabled(bool bTickEnabled) override;
	virtual void Activate(bool bReset = false) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:

	UPROPERTY()
		APawn* OwningPawn = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Replication", Meta = (AllowPrivateAcess = "true"))
		USceneComponent* HeadComponent = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Replication", Meta = (AllowPrivateAcess = "true"))
		USceneComponent* LeftHandComponent = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Replication", Meta = (AllowPrivateAcess = "true"))
		USceneComponent* RightHandComponent = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedTransforms)
		FProteusReplicationData ReplicatedVRBodyTransforms;

	UFUNCTION(Server, Unreliable, WithValidation, Category = "Network")
		void Server_UpdateReplicationData(const FProteusReplicationData& UpdatedData);

	UFUNCTION()
		void OnRep_ReplicatedTransforms();

	bool bIsRemote = false;

	bool bHasRegisteredBody = false;

	virtual void BeginPlay() override;

	void SendTransformsToServer();

	void LerpReplicatedComponents(float DeltaTime);

	void LerpReplicatedComponent(USceneComponent* Component, const FTransform_NetQuantize100& TargetTransform, float DeltaTime, float LerpAlpha);
	FTransform_NetQuantize100 LerpTransform(const FTransform_NetQuantize100& Start, const FTransform_NetQuantize100& End, float DeltaTime, float LerpAlpha) const;
};