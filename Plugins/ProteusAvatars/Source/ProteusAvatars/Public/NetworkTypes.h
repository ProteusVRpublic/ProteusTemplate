
#pragma once

#include "CoreMinimal.h"
#include "Engine/NetSerialization.h"
#include "NetworkTypes.generated.h"

/**
* FTransform_NetQuantize10 compresses vectors to one decimal place
* accuracy and rotations to byte per component
*/
USTRUCT(BlueprintType)
struct FTransform_NetQuantize10
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FVector Location;

    UPROPERTY(BlueprintReadWrite)
    FRotator Rotation;

    FTransform_NetQuantize10() : Location(0.f), Rotation(0.f) {};

    FTransform_NetQuantize10(const FVector& InLocation, const FRotator& InRotation)
        : Location(InLocation), Rotation(InRotation) {};

    FTransform_NetQuantize10(const FTransform& InTransform)
        : Location(InTransform.GetLocation()), Rotation(InTransform.Rotator()) {};

    bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
    {
        bOutSuccess = SerializePackedVector<10, 27>(Location, Ar);

        // Rotation compressed to byte per component
        Rotation.SerializeCompressed(Ar);

        return true;
    };
};

// Required for our custom NetSerialize function to be used by Unreal's network replication system
template<>
struct TStructOpsTypeTraits<FTransform_NetQuantize10> : public TStructOpsTypeTraitsBase2<FTransform_NetQuantize10>
{
    enum
    {
        WithNetSerializer = true
    };
};



/**
* FTransform_NetQuantize100 compresses vectors to two decimal places
* accuracy and rotations to short per component
*/
USTRUCT(BlueprintType)
struct FTransform_NetQuantize100
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FVector Location;

    UPROPERTY(BlueprintReadWrite)
    FRotator Rotation;

    FTransform_NetQuantize100() : Location(0.f), Rotation(0.f) {};

    FTransform_NetQuantize100(const FVector& InLocation, const FRotator& InRotation)
        : Location(InLocation), Rotation(InRotation) {};

    FTransform_NetQuantize100(const FTransform& InTransform)
        : Location(InTransform.GetLocation()), Rotation(InTransform.Rotator()) {};

    bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess)
    {
        bOutSuccess = SerializePackedVector<100, 30>(Location, Ar);

        // Rotation compressed to short per component
        Rotation.SerializeCompressedShort(Ar);

        return true;
    };
};

template<>
struct TStructOpsTypeTraits<FTransform_NetQuantize100> : public TStructOpsTypeTraitsBase2<FTransform_NetQuantize100>
{
    enum
    {
        WithNetSerializer = true
    };
};

/**
*
*/
UCLASS(abstract)
class PROTEUSAVATARS_API UNetworkTypes : public UObject
{
    GENERATED_BODY()
};