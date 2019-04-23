
#include "ProteusReplication.h"
#include "UnrealNetwork.h"
#include "Components/SceneComponent.h"
#include "Camera/CameraComponent.h"
#include "MotionControllerComponent.h"
#include "XRMotionControllerBase.h"


UProteusReplication::UProteusReplication()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
    PrimaryComponentTick.TickGroup = ETickingGroup::TG_PrePhysics;
    bAutoActivate = false;

    bReplicates = true;
    bIsRemote = false;
}

void UProteusReplication::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION(UProteusReplication, ReplicatedVRBodyTransforms, COND_SkipOwner);
}


void UProteusReplication::BeginPlay()
{
    Super::BeginPlay();

    OwningPawn = Cast<APawn>(GetOwner());

    if (OwningPawn == nullptr)
    {
        HandleSetupError(TEXT("ProteusReplication component does not work with non-Pawn Actors!"));
        return;
    }

    bIsRemote = OwningPawn->Role != ROLE_Authority;

    if (!bHasRegisteredBody && bAutoRegisterBodyComponents)
    {
        const bool bRegisterSuccess = AutoRegisterProteusBody();

        if (!bRegisterSuccess)
        {
            HandleSetupError(
                TEXT("ProteusReplication could not find valid VR components to replicate! \
                        Ensure your Pawn has a camera and left and right motion controllers \
                        You can set them manually by disabling AutoRegisterBodyComponents and calling RegisterProteusForReplication.")
            );
        }
    }
}


void UProteusReplication::SetComponentTickEnabled(bool bTickEnabled)
{
    // Only enable tick if the vr body is registered or a crash may occur
    if (!bTickEnabled || bHasRegisteredBody)
    {
        Super::SetComponentTickEnabled(bTickEnabled);
    }
}

void UProteusReplication::Activate(bool bReset)
{
    if (bHasRegisteredBody)
    {
        Super::Activate(bReset);
    }
}

void UProteusReplication::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (IsAuthoritative())
    {
        SendTransformsToServer();
    }
    else
    {
        if (bLerpToTransform)
        {
            LerpReplicatedComponents(DeltaTime);
        }
        // !bLerpToTransform handled by OnRep_ReplicatedTransforms
    }
}

bool UProteusReplication::IsAuthoritative() const
{
    return OwningPawn->IsLocallyControlled();
}

bool UProteusReplication::RegisterProteusForReplication(USceneComponent* Head, USceneComponent* LeftHand, USceneComponent* RightHand)
{
    if (Head == nullptr || LeftHand == nullptr || RightHand == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid VR Body components! Please check that you properly connected the pins for RegisterVRBody. Body will not replicate."));
        return false;
    }

    HeadComponent = Head;
    LeftHandComponent = LeftHand;
    RightHandComponent = RightHand;

    bHasRegisteredBody = true;
    Activate(true);

    return true;
}

bool UProteusReplication::AutoRegisterProteusBody()
{
    if (const AActor* Owner = GetOwner())
    {
        TArray<UCameraComponent*> Cameras;
        TArray<UMotionControllerComponent*> MotionControllers;

        Owner->GetComponents<UCameraComponent>(Cameras);
        Owner->GetComponents<UMotionControllerComponent>(MotionControllers);

        UCameraComponent* RegisteredCamera = nullptr;
        UMotionControllerComponent* RegisteredLeftHand = nullptr;
        UMotionControllerComponent* RegisteredRightHand = nullptr;

        for (UCameraComponent* Camera : Cameras)
        {
            if (Camera->bLockToHmd)
            {
                RegisteredCamera = Camera;
                break;
            }
        }

        // If there are cameras but none with bLockToHMD enabled by default
        // Then we'll just use one that isn't lock to an HMD and assume it will be later
        if (RegisteredCamera == nullptr && Cameras.Num() > 0)
        {
            RegisteredCamera = Cameras[0];
        }

        for (UMotionControllerComponent* MotionController : MotionControllers)
        {
            const FName& MotionSource = MotionController->MotionSource;
            if (MotionSource == FXRMotionControllerBase::LeftHandSourceId)
            {
                RegisteredLeftHand = MotionController;
            }
            else if (MotionSource == FXRMotionControllerBase::RightHandSourceId)
            {
                RegisteredRightHand = MotionController;
            }
        }

        RegisterProteusForReplication(RegisteredCamera, RegisteredLeftHand, RegisteredRightHand);
    }

    return bHasRegisteredBody;
}

void UProteusReplication::SendTransformsToServer()
{
    ReplicatedVRBodyTransforms.HeadTransform = HeadComponent->GetRelativeTransform();
    ReplicatedVRBodyTransforms.LeftHandTransform = LeftHandComponent->GetRelativeTransform();
    ReplicatedVRBodyTransforms.RightHandTransform = RightHandComponent->GetRelativeTransform();

    if (bIsRemote)
    {
        Server_UpdateReplicationData(ReplicatedVRBodyTransforms);
    }
}

void UProteusReplication::LerpReplicatedComponents(float DeltaTime)
{
    const float LerpAlpha = NetworkLocationLerpRate * DeltaTime;
    LerpReplicatedComponent(HeadComponent, ReplicatedVRBodyTransforms.HeadTransform, DeltaTime, LerpAlpha);
    LerpReplicatedComponent(LeftHandComponent, ReplicatedVRBodyTransforms.LeftHandTransform, DeltaTime, LerpAlpha);
    LerpReplicatedComponent(RightHandComponent, ReplicatedVRBodyTransforms.RightHandTransform, DeltaTime, LerpAlpha);
}

void UProteusReplication::SetToServerTransforms()
{
    const FTransform_NetQuantize100& HeadTransform = ReplicatedVRBodyTransforms.HeadTransform;
    HeadComponent->SetRelativeLocationAndRotation(HeadTransform.Location, HeadTransform.Rotation);

    const FTransform_NetQuantize100& LeftHandTransform = ReplicatedVRBodyTransforms.LeftHandTransform;
    LeftHandComponent->SetRelativeLocationAndRotation(LeftHandTransform.Location, LeftHandTransform.Rotation);

    const FTransform_NetQuantize100& RightHandTransform = ReplicatedVRBodyTransforms.RightHandTransform;
    RightHandComponent->SetRelativeLocationAndRotation(RightHandTransform.Location, RightHandTransform.Rotation);
}

void UProteusReplication::LerpReplicatedComponent(USceneComponent* Component, const FTransform_NetQuantize100& TargetTransform, float DeltaTime, float LerpAlpha)
{
    const FTransform_NetQuantize100& OldTransform = Component->GetRelativeTransform();
    const FTransform_NetQuantize100& NewTransform = LerpTransform(OldTransform, TargetTransform, DeltaTime, LerpAlpha);
    Component->SetRelativeLocationAndRotation(NewTransform.Location, NewTransform.Rotation);
}

FTransform_NetQuantize100 UProteusReplication::LerpTransform(const FTransform_NetQuantize100& Start, const FTransform_NetQuantize100& End, float DeltaTime, float Alpha) const
{
    // Mixes both regular and constant rate interpolation for smooth updates during fast hand movement and accuracy in small movements
    const FVector FirstLerpLocation = FMath::Lerp(Start.Location, End.Location, Alpha);
    const FVector FinalLerpLocation = FMath::VInterpConstantTo(FirstLerpLocation, End.Location, DeltaTime, NetworkLocationConstInterpRate);

    const FQuat StartQuat(Start.Rotation);
    const FQuat EndQuat(End.Rotation);
    const FQuat LerpRotation = FQuat::FastLerp(StartQuat, EndQuat, Alpha + .1f);

    return FTransform_NetQuantize100(FinalLerpLocation, LerpRotation.Rotator());
}

void UProteusReplication::HandleSetupError(const FString& ErrorMessage)
{
    FString FinalErrorMessage(ErrorMessage);
    FinalErrorMessage.Append(TEXT(" Destroying component."));

#if UE_BUILD_DEBUG
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, FinalErrorMessage);
    }
#endif
    UE_LOG(LogTemp, Warning, TEXT("%s"), *FinalErrorMessage);

    DestroyComponent();
}



bool UProteusReplication::Server_UpdateReplicationData_Validate(const FProteusReplicationData& UpdatedData)
{
    return true;
}

void UProteusReplication::Server_UpdateReplicationData_Implementation(const FProteusReplicationData& UpdatedData)
{
    ReplicatedVRBodyTransforms = UpdatedData;

    OnRep_ReplicatedTransforms();
}

bool UProteusReplication::Server_JumpToAuthorityTransforms_Validate()
{
    return true;
}

void UProteusReplication::Server_JumpToAuthorityTransforms_Implementation()
{
    SetToServerTransforms();
}

void UProteusReplication::OnRep_ReplicatedTransforms()
{
    if (!bLerpToTransform)
    {
        SetToServerTransforms();
    }
}