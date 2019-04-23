
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NetworkTypes.h"
#include "ProteusReplication.generated.h"

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

/**
* Efficiently replicates VR Head and Hands positions from Client to Server and other clients.
* Only to be used in VR Pawns.
*/
UCLASS(BlueprintType, ClassGroup = (Network), meta = (BlueprintSpawnableComponent), HideCategories = (Activation, Cooking, ComponentReplication, Collision))
class PROTEUSAVATARS_API UProteusReplication : public UActorComponent
{
    GENERATED_BODY()

public:

    // If true, the component will automatically discover the owning pawn's VR camera and left and right motion controllers to replicate
    // If false, RegisterVRBodyForReplication must be called to setup the replicated components (should be called on BeginPlay)
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Parameters")
    bool bAutoRegisterBodyComponents = true;

    // If false, body components will snap to their latest replicated transform
    // If true, body components will lerp to their latest replicated transform
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Parameters")
    bool bLerpToTransform = true;

    // The constant interpolation rate to the authority location, helps keep small movements exact
    // We const interp from the simulated transforms to the server transforms at this rate per second
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Parameters")
    float NetworkLocationConstInterpRate = 100.f;

    // The interpolation rate to the current network transforms, for smoothing when jumping long distances
    // We lerp from the simulated transforms to the server transforms at this rate per second
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Parameters")
    float NetworkLocationLerpRate = 10.f;

protected:

    bool bIsRemote = false;

    bool bHasRegisteredBody = false;


protected:

    UPROPERTY()
    APawn* OwningPawn = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Replication", Meta=(AllowPrivateAcess="true"))
    USceneComponent* HeadComponent = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Replication", Meta = (AllowPrivateAcess = "true"))
    USceneComponent* LeftHandComponent = nullptr;

    UPROPERTY(BlueprintReadOnly, Category = "Replication", Meta = (AllowPrivateAcess = "true"))
    USceneComponent* RightHandComponent = nullptr;

    UPROPERTY(ReplicatedUsing = OnRep_ReplicatedTransforms)
    FProteusReplicationData ReplicatedVRBodyTransforms;

public:    
    UProteusReplication();
protected:
    virtual void BeginPlay() override;
public:    
    virtual void SetComponentTickEnabled(bool bTickEnabled) override;
    virtual void Activate(bool bReset = false) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
    

public:

    UFUNCTION(BlueprintCallable, Category = "Network")
    bool IsAuthoritative() const;

    /**
    * Set the hand and hands components and starts replicating their transforms
    * across the network
    * Typically this will be the Camera, LeftMotionController, and RightMotionController
    * @return Success
    */
    UFUNCTION(BlueprintCallable, Category = "Network")
    bool RegisterProteusForReplication(USceneComponent* Head, USceneComponent* LeftHand, USceneComponent* RightHand);

protected:

    bool AutoRegisterProteusBody();

    void SendTransformsToServer();

    void LerpReplicatedComponents(float DeltaTime);

    void SetToServerTransforms();

    void LerpReplicatedComponent(USceneComponent* Component, const FTransform_NetQuantize100& TargetTransform, float DeltaTime, float LerpAlpha);
    FTransform_NetQuantize100 LerpTransform(const FTransform_NetQuantize100& Start, const FTransform_NetQuantize100& End, float DeltaTime, float LerpAlpha) const;

    void HandleSetupError(const FString& ErrorMessage);

protected:

    UFUNCTION(Server, Unreliable, WithValidation, Category = "Network")
    void Server_UpdateReplicationData(const FProteusReplicationData& UpdatedData);

    UFUNCTION(Server, Reliable, WithValidation, Category = "Network")
    void Server_JumpToAuthorityTransforms();

    UFUNCTION()
    void OnRep_ReplicatedTransforms();
};
