// Made by ProteusVR

#include "ProteusVOIP.h"



// Sets default values for this component's properties
UProteusVOIP::UProteusVOIP()
	: bTalkerRegistered(false),
	RegisteredTalkerOculusId(FString())
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UProteusVOIP::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	UnregisterRemoteTalker();

	Super::EndPlay(EndPlayReason);
}

bool UProteusVOIP::RegisterRemoteTalker(FString OculusID)
{
	UE_LOG(LogTemp, Warning, TEXT("RegisterRemoteTalker"));
	if (bTalkerRegistered)
	{
		UE_LOG(LogTemp, Warning, TEXT("RegisterRemoteTalker_01"));
		if (OculusID != RegisteredTalkerOculusId)
		{
			// this talker is already registered
			return true;
		}
		else {
			UnregisterRemoteTalker();
		}
	}

	if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get())
	{
		auto IdentityInterface = OnlineSub->GetIdentityInterface();
		UE_LOG(LogTemp, Warning, TEXT("RegisterRemoteTalker_02"));
		if (IdentityInterface.IsValid())
		{
			auto uniqueNetId = IdentityInterface->CreateUniquePlayerId(OculusID);

			if (uniqueNetId.IsValid())
			{
				auto VoiceInterface = OnlineSub->GetVoiceInterface();

				if (VoiceInterface.IsValid())
				{
					bTalkerRegistered = VoiceInterface->RegisterRemoteTalker(*uniqueNetId);
				}
			}
		}
	}

	if (bTalkerRegistered)
	{
		RegisteredTalkerOculusId = OculusID;
		UE_LOG(LogTemp, Warning, TEXT("RegisterRemoteTalker_03 is %s"), *OculusID);
	}

	return bTalkerRegistered;
	UE_LOG(LogTemp, Warning, TEXT("RegisterRemoteTalker_04"));
}

void UProteusVOIP::UnregisterRemoteTalker()
{
	if (bTalkerRegistered)
	{
		if (IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get())
		{
			auto IdentityInterface = OnlineSub->GetIdentityInterface();

			if (IdentityInterface.IsValid())
			{
				auto uniqueNetId = IdentityInterface->CreateUniquePlayerId(RegisteredTalkerOculusId);

				if (uniqueNetId.IsValid())
				{
					auto VoiceInterface = OnlineSub->GetVoiceInterface();

					if (VoiceInterface.IsValid())
					{
						VoiceInterface->UnregisterRemoteTalker(*uniqueNetId);
					}
				}
			}
		}

		bTalkerRegistered = false;
	}
}



