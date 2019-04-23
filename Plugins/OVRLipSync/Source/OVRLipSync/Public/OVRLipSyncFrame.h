/*******************************************************************************
 * Filename    :   OVRLipSyncFrame.h
 * Content     :   Prototype for OVRLipSync Frame
 * Created     :   Sep 14th, 2018
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

#include "CoreMinimal.h"
#include "OVRLipSyncFrame.generated.h"

USTRUCT()
struct OVRLIPSYNC_API FOVRLipSyncFrame
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<float> VisemeScores;

	UPROPERTY()
	float LaughterScore;

	FOVRLipSyncFrame(const TArray<float> &visemes = {}, float laughterScore = 0.0f)
		: VisemeScores(visemes), LaughterScore(laughterScore)
	{
	}
};

UCLASS(BlueprintType)
class OVRLIPSYNC_API UOVRLipSyncFrameSequence : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY()
	TArray<FOVRLipSyncFrame> FrameSequence;
	unsigned Num() const { return FrameSequence.Num(); }
	void Add(const TArray<float> &Visemes, float LaughterScore) { FrameSequence.Emplace(Visemes, LaughterScore); }
	const FOVRLipSyncFrame &operator[](unsigned idx) const { return FrameSequence[idx]; }
};
