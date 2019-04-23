/*******************************************************************************
 * Filename    :   OVRLipSyncPlaybackActorComponent.cpp
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

#include "OVRLipSyncPlaybackActorComponent.h"

UAudioComponent *UOVRLipSyncPlaybackActorComponent::FindAutoplayAudioComponent() const
{
	TArray<UAudioComponent *> AudioComponents;
	GetOwner()->GetComponents<UAudioComponent>(AudioComponents);

	if (AudioComponents.Num() == 0)
	{
		return nullptr;
	}
	for (auto &AudioComponentCandidate : AudioComponents)
	{
		if (AudioComponentCandidate->bAutoActivate)
			return AudioComponentCandidate;
	}
	return nullptr;
}

// Called when the game starts
void UOVRLipSyncPlaybackActorComponent::BeginPlay()
{
	Super::BeginPlay();
	if (!Sequence)
	{
		return;
	}
	auto AutoplayComponent = FindAutoplayAudioComponent();
	if (AutoplayComponent)
	{
		Start(AutoplayComponent);
	}
}

void UOVRLipSyncPlaybackActorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Stop();

	Super::EndPlay(EndPlayReason);
}

void UOVRLipSyncPlaybackActorComponent::OnAudioPlaybackPercent(const UAudioComponent *, const USoundWave *SoundWave,
															   float Percent)
{
	if (!Sequence)
	{	
		InitNeutralPose();
		return;
	}
	static float tuning = 0.125f;
	auto PlayPos = SoundWave->Duration * Percent + tuning;
	auto IntPos = static_cast<unsigned>(PlayPos * 100);
	if (IntPos >= Sequence->Num())
	{
		InitNeutralPose();
		return;
	}
	const auto &Frame = (*Sequence)[IntPos];
	LaughterScore = Frame.LaughterScore;
	Visemes = Frame.VisemeScores;
	OnVisemesReady.Broadcast();
}

void UOVRLipSyncPlaybackActorComponent::OnAudioPlaybackFinished(UAudioComponent *) { InitNeutralPose(); }

void UOVRLipSyncPlaybackActorComponent::Start(UAudioComponent *InAudioComponent)
{
	AudioComponent = InAudioComponent;
	PlaybackPercentHandle = AudioComponent->OnAudioPlaybackPercentNative.AddUObject(
		this, &UOVRLipSyncPlaybackActorComponent::OnAudioPlaybackPercent);
	PlaybackFinishedHandle = AudioComponent->OnAudioFinishedNative.AddUObject(
		this, &UOVRLipSyncPlaybackActorComponent::OnAudioPlaybackFinished);
	AudioComponent->Play();
}

void UOVRLipSyncPlaybackActorComponent::Stop()
{
	if (!AudioComponent)
	{
		return;
	}
	AudioComponent->OnAudioPlaybackPercentNative.Remove(PlaybackPercentHandle);
	AudioComponent->OnAudioFinishedNative.Remove(PlaybackFinishedHandle);
	AudioComponent = nullptr;
	InitNeutralPose();
}
