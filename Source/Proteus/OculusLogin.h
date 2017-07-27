#pragma once

#include "Online.h"
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "OvrAvatar.h"
#include "Engine/LocalPlayer.h"
#include "OnlineBlueprintCallProxyBase.h"
#include "OculusLogin.generated.h"

UENUM(BlueprintType)
enum class EOculusOnlinePresenceState : uint8
{
	Online,
	Offline,
	Away,
	ExtendedAway,
	DoNotDisturb,
	Chat
};

USTRUCT(BlueprintType)
struct FFriendPresenceInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProteusAvatar")
		bool bIsOnline;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProteusAvatar")
		bool bIsPlaying;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProteusAvatar")
		bool bIsPlayingThisGame;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProteusAvatar")
		bool bIsJoinable;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProteusAvatar")
		bool bHasVoiceSupport;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProteusAvatar")
		EOculusOnlinePresenceState PresenceState;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProteusAvatar")
		FString StatusString;
};

USTRUCT(BlueprintType)
struct FBPUniqueNetId
{
	GENERATED_USTRUCT_BODY()

		bool bUseDirectPointer;

	TSharedPtr<const FUniqueNetId> UniqueNetId;
	const FUniqueNetId * UniqueNetIdPtr;

	void SetUniqueNetId(const TSharedPtr<const FUniqueNetId> &ID)
	{
		bUseDirectPointer = false;
		UniqueNetIdPtr = nullptr;
		UniqueNetId = ID;
	}

	void SetUniqueNetId(const FUniqueNetId *ID)
	{
		bUseDirectPointer = true;
		UniqueNetIdPtr = ID;
	}

	bool IsValid() const
	{
		if (bUseDirectPointer && UniqueNetIdPtr != nullptr && UniqueNetIdPtr->IsValid())
		{
			return true;
		}
		else if (UniqueNetId.IsValid())
		{
			return true;
		}
		else
			return false;
	}

	const FUniqueNetId* GetUniqueNetId() const
	{
		if (bUseDirectPointer && UniqueNetIdPtr != nullptr)
		{
			return (UniqueNetIdPtr);
		}
		else if (UniqueNetId.IsValid())
		{
			return UniqueNetId.Get();
		}
		else
			return nullptr;
	}

	FBPUniqueNetId()
	{
		bUseDirectPointer = false;
		UniqueNetIdPtr = nullptr;
	}
};

USTRUCT(BlueprintType)
struct FFriendInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProteusAvatar")
		FString OculusUsername;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProteusAvatar")
		EOculusOnlinePresenceState OnlineState;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProteusAvatar")
		FBPUniqueNetId OculusID;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProteusAvatar")
		bool bIsPlayingSameGame;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProteusAvatar")
		FFriendPresenceInfo PresenceInfo;
};

DECLARE_LOG_CATEGORY_EXTERN(RetrieveOculusLog, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBlueprintGetFriendsListDelegate, const TArray<FFriendInfo>&, Results);

UCLASS(MinimalAPI)
class UOculusLogin : public UOnlineBlueprintCallProxyBase
{
	GENERATED_UCLASS_BODY()

	// Gets the players list of friends from the OnlineSubsystem and returns it, can be retrieved later with GetStoredFriendsList
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category = "ProteusAvatar")
		static UOculusLogin* RetrieveOculusSessions(UObject* WorldContextObject, class APlayerController* PlayerController);

	// Called when the friends list successfully was retrieved
	UPROPERTY(BlueprintAssignable)
		FBlueprintGetFriendsListDelegate OnSuccess;

	// Called when there was an error retrieving the friends list
	UPROPERTY(BlueprintAssignable)
		FBlueprintGetFriendsListDelegate OnFailure;

	// Check if a UniqueNetId is a friend
	UFUNCTION(BlueprintPure, Category = "ProteusAvatar")
		static void OculusIDToString(const FBPUniqueNetId &UniqueNetId, FString &String);

	virtual void Activate() override;

private:
	// Internal callback when the friends list is retrieved
	void OnReadFriendsListCompleted(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorString);

	// The player controller triggering things
	TWeakObjectPtr<APlayerController> PlayerControllerWeakPtr;

	// The delegate executed
	FOnReadFriendsListComplete FriendListReadCompleteDelegate;

	// The world context object in which this call is taking place
	UObject* WorldContextObject;
};