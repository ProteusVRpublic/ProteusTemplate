/*******************************************************************************
 * Filename    :   OVRLipSyncActorComponent.cpp
 * Content     :   OVRLipSync component for Actor objects
 * Created     :   Aug 9th, 2018
 * Copyright   :   Copyright Facebook Technologies, LLC and its affiliates.
 *                 All rights reserved.
 *
 * Licensed under the Oculus Audio SDK License Version 3.3 (the "License");
 * you may not use the Oculus Audio SDK except in compliance with the License,
 * which is provided at the time of installation or download, or which
 * otherwise accompanies this software in either electronic or hard copy form.

 * You may obtain a copy of the License at
 *
 * https://developer.oculus.com/licenses/audio-3.3/
 *
 * Unless required by applicable law or agreed to in writing, the Oculus Audio SDK
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ******************************************************************************/

#include "OVRLipSyncLiveActorComponent.h"

#include "OVRLipSyncContextWrapper.h"
#include "Voice/Public/VoiceModule.h"
#include "TimerManager.h"
#include <algorithm>

#define DEFAULT_SAMPLE_RATE 44100
#define DEFAULT_BUFFER_SIZE 4096

// Convert OVRLipSyncProviderKind enum to OVRLipSync
ovrLipSyncContextProvider ContextProviderFromProviderKind(OVRLipSyncProviderKind Kind)
{
	switch (Kind)
	{
	default:
	case OVRLipSyncProviderKind::Original:
		return ovrLipSyncContextProvider_Original;
	case OVRLipSyncProviderKind::Enhanced:
		return ovrLipSyncContextProvider_Enhanced;
	case OVRLipSyncProviderKind::EnhancedWithLaughter:
		return ovrLipSyncContextProvider_EnhancedWithLaughter;
	}
}

// Called when the game starts
void UOVRLipSyncActorComponent::BeginPlay()
{
	Super::BeginPlay();

	LipSyncContext = MakeShared<UOVRLipSyncContextWrapper>(ContextProviderFromProviderKind(ProviderKind), SampleRate,
														   EnableHardwareAcceleration);
}

void UOVRLipSyncActorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Stop();
	LipSyncContext = nullptr;

	Super::EndPlay(EndPlayReason);
}

void UOVRLipSyncActorComponent::Start()
{
	if (VoiceCapture)
	{
		Stop();
	}
	VoiceCapture = FVoiceModule::Get().CreateVoiceCapture(DEFAULT_DEVICE_NAME, SampleRate, 1);
	if (!VoiceCapture)
	{
		UE_LOG(LogTemp, Error, TEXT("Can't create voice capture"));
		return;
	}
	VoiceCapture->Start();
	auto &TimerManager = GetWorld()->GetTimerManager();
	TimerManager.SetTimer(VoiceCaptureTimer, this, &UOVRLipSyncActorComponent::OnVoiceCaptureTimer,
						  VoiceCaptureTimerRate, true);
}

void UOVRLipSyncActorComponent::FeedAudio(const TArray<uint8> &VoiceData)
{
	if (!LipSyncContext)
	{
		return;
	}

	auto *ShortData = reinterpret_cast<const int16 *>(VoiceData.GetData());
	auto ShortDataSize = VoiceData.Num() / 2;
	LipSyncContext->ProcessFrame(ShortData, ShortDataSize, Visemes, LaughterScore);
	OnVisemesReady.Broadcast();
}

void UOVRLipSyncActorComponent::Stop()
{
	if (!VoiceCapture)
	{
		return;
	}

	if (GetWorld())
	{
		auto &TimerManager = GetWorld()->GetTimerManager();
		TimerManager.ClearTimer(VoiceCaptureTimer);
	}

	VoiceCapture->Stop();
	VoiceCapture = nullptr;

	InitNeutralPose();
}

// Called every VoiceCaptureTimerRate seconds (20ms) to process audio data
void UOVRLipSyncActorComponent::OnVoiceCaptureTimer()
{
	auto time = GetWorld()->GetTimeSeconds();
	if (!VoiceCapture)
	{
		return;
	}

	uint32 AvailableVoiceData = 0;
	auto CaptureState = VoiceCapture->GetCaptureState(AvailableVoiceData);
	if (CaptureState != EVoiceCaptureState::Ok && CaptureState != EVoiceCaptureState::NoData)
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid capture state: %s"), EVoiceCaptureState::ToString(CaptureState));
		return;
	}
	if (CaptureState == EVoiceCaptureState::NoData)
	{
		return;
	}

	if (AvailableVoiceData == 0)
	{
		return;
	}
	TArray<uint8> VoiceData;
	uint32 VoiceDataCaptured;
	VoiceData.SetNumUninitialized(AvailableVoiceData);

	CaptureState = VoiceCapture->GetVoiceData(VoiceData.GetData(), VoiceData.Num(), VoiceDataCaptured);
	if (CaptureState != EVoiceCaptureState::Ok || VoiceDataCaptured == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to get voice data: %s DataCaptured=%d"),
			   EVoiceCaptureState::ToString(CaptureState), VoiceDataCaptured);
		return;
	}
	TArray<uint8> AudioBuffer(VoiceData.GetData(), VoiceDataCaptured);
	FeedAudio(AudioBuffer);
}

const float UOVRLipSyncActorComponent::VoiceCaptureTimerRate = .01f;
