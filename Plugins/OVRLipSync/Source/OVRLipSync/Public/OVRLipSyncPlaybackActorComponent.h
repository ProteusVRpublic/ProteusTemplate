/*******************************************************************************
 * Filename    :   OVRLipSyncPlaybackActorComponent.h
 * Content     :   Prototypes for OVRLipSync Playback Actor Component
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

#pragma once

#include "Classes/Components/AudioComponent.h"
#include "OVRLipSyncActorComponentBase.h"
#include "OVRLipSyncFrame.h"

#include "OVRLipSyncPlaybackActorComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class OVRLIPSYNC_API UOVRLipSyncPlaybackActorComponent : public UOVRLipSyncActorComponentBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Meta = (Tooltip = "LipSync Sequence to be played"))
	UOVRLipSyncFrameSequence *Sequence;

	UPROPERTY(BlueprintReadonly)
	UAudioComponent *AudioComponent;

	UFUNCTION(BlueprintCallable, Category = "LipSync",
			  Meta = (Tooltip = "Start playback of the canned sequence synchronized with AudioComponent"))
	void Start(UAudioComponent *InAudioComponent);

	UFUNCTION(BlueprintCallable, Category = "LipSync")
	void Stop();

protected:
	// Returns audio Component associated with the same
	UAudioComponent *FindAutoplayAudioComponent() const;
	// Audio Component callbacks
	void OnAudioPlaybackPercent(const UAudioComponent *, const USoundWave *, float Percent);
	void OnAudioPlaybackFinished(UAudioComponent *);
	// Called when the game starts
	virtual void BeginPlay() override;
	// Called when the game ends
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	FDelegateHandle PlaybackPercentHandle;
	FDelegateHandle PlaybackFinishedHandle;
};
