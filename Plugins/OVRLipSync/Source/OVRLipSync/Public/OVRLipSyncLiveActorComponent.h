/*******************************************************************************
 * Filename    :   OVRLipSyncLiveActorComponent.h
 * Content     :   Prototypes for OVRLipSync Live Actor Component
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
*******************************************************************************/

#pragma once

#include "OVRLipSyncActorComponentBase.h"
#include "OVRLipSyncLiveActorComponent.generated.h"

class IVoiceCapture;
class UOVRLipSyncContextWrapper;

UENUM()
enum class OVRLipSyncProviderKind : uint8
{
	Original = 0,
	Enhanced = 1,
	EnhancedWithLaughter = 2,
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class OVRLIPSYNC_API UOVRLipSyncActorComponent : public UOVRLipSyncActorComponentBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	int32 SampleRate = 16000;

	UPROPERTY(EditAnywhere)
	OVRLipSyncProviderKind ProviderKind = OVRLipSyncProviderKind::EnhancedWithLaughter;

	UPROPERTY(EditAnywhere, Meta = (ToolTip = "Enable hardware acceleration on supported platforms"))
	bool EnableHardwareAcceleration = true;

	UFUNCTION(BlueprintCallable, Category = "LipSync")
	void Start();

	UFUNCTION(BlueprintCallable, Category = "LipSync")
	void Stop();

	UFUNCTION(BlueprintCallable, Category = "LipSync",
			  Meta = (ToolTip = "Feed AudioBuffer containing packaged mono 16-bit signed integer PCM values"))
	void FeedAudio(const TArray<uint8> &AudioData);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	// Called when the game ends
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void OnVoiceCaptureTimer();

private:
	TSharedPtr<UOVRLipSyncContextWrapper> LipSyncContext;

	TSharedPtr<IVoiceCapture> VoiceCapture;
	FTimerHandle VoiceCaptureTimer;
	static const float VoiceCaptureTimerRate;
};
