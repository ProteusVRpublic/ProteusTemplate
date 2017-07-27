// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "Proteus.h"
#include "OculusLogin.h"
#include "OvrAvatarManager.h"
#include "Object.h"

DEFINE_LOG_CATEGORY(RetrieveOculusLog);

UOculusLogin::UOculusLogin(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, FriendListReadCompleteDelegate(FOnReadFriendsListComplete::CreateUObject(this, &ThisClass::OnReadFriendsListCompleted))
{
}

UOculusLogin* UOculusLogin::RetrieveOculusSessions(UObject* WorldContextObject, class APlayerController* PlayerController)
{
	UOculusLogin* Proxy = NewObject<UOculusLogin>();
	Proxy->PlayerControllerWeakPtr = PlayerController;
	Proxy->WorldContextObject = WorldContextObject;
	return Proxy;
}

void UOculusLogin::Activate()
{
	if (!PlayerControllerWeakPtr.IsValid())
	{
		// Fail immediately
		TArray<FFriendInfo> EmptyArray;
		OnFailure.Broadcast(EmptyArray);
		return;
	}

	IOnlineFriendsPtr Friends = Online::GetFriendsInterface();
	if (Friends.IsValid())
	{
		ULocalPlayer* Player = Cast<ULocalPlayer>(PlayerControllerWeakPtr->Player);

		Friends->ReadFriendsList(Player->GetControllerId(), EFriendsLists::ToString((EFriendsLists::Default)), FriendListReadCompleteDelegate);
		return;
	}

	// Fail immediately
	TArray<FFriendInfo> EmptyArray;

	OnFailure.Broadcast(EmptyArray);
}

void UOculusLogin::OnReadFriendsListCompleted(int32 LocalUserNum, bool bWasSuccessful, const FString& ListName, const FString& ErrorString)
{
	if (bWasSuccessful)
	{
		IOnlineFriendsPtr Friends = Online::GetFriendsInterface();
		if (Friends.IsValid())
		{
			TArray<FFriendInfo> FriendsListOut;
			TArray< TSharedRef<FOnlineFriend> > FriendList;
			Friends->GetFriendsList(LocalUserNum, ListName, FriendList);

			for (int32 i = 0; i < FriendList.Num(); i++)
			{
				TSharedRef<FOnlineFriend> Friend = FriendList[i];
				FFriendInfo BPF;
				FOnlineUserPresence pres = Friend->GetPresence();
				BPF.OnlineState = ((EOculusOnlinePresenceState)((int32)pres.Status.State));
				BPF.OculusUsername = Friend->GetDisplayName();
				BPF.OculusID.SetUniqueNetId(Friend->GetUserId());
				BPF.bIsPlayingSameGame = pres.bIsPlayingThisGame;

				BPF.PresenceInfo.bIsOnline = pres.bIsOnline;
				BPF.PresenceInfo.bHasVoiceSupport = pres.bHasVoiceSupport;
				BPF.PresenceInfo.bIsPlaying = pres.bIsPlaying;
				BPF.PresenceInfo.PresenceState = ((EOculusOnlinePresenceState)((int32)pres.Status.State));
				BPF.PresenceInfo.StatusString = pres.Status.StatusStr;
				BPF.PresenceInfo.bIsJoinable = pres.bIsJoinable;
				BPF.PresenceInfo.bIsPlayingThisGame = pres.bIsPlayingThisGame;

				FriendsListOut.Add(BPF);
			}

			OnSuccess.Broadcast(FriendsListOut);
		}
	}
	else
	{
		TArray<FFriendInfo> EmptyArray;
		OnFailure.Broadcast(EmptyArray);
	}
}

void UOculusLogin::OculusIDToString(const FBPUniqueNetId& UniqueNetId, FString &String)
{
	const FUniqueNetId * ID = UniqueNetId.GetUniqueNetId();

	if (!ID)
	{
		UE_LOG(RetrieveOculusLog, Warning, TEXT("UniqueNetIdToString received a bad UniqueNetId!"));
		String = "ERROR, BAD UNIQUE NET ID";
	}
	else
		String = ID->ToString();
}
