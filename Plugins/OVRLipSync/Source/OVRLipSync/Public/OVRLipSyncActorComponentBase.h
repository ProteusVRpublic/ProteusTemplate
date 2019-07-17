/*******************************************************************************
Filename    :   OVRLipSyncActorComponentBase.h
Content     :   Prototypes for OVRLipSync Actor Component Base
Created     :   Sep 14th, 2018
Copyright   :   Copyright Facebook Technologies, LLC and its affiliates.
				All rights reserved.

Licensed under the Oculus Audio SDK License Version 3.3 (the "License");
you may not use the Oculus Audio SDK except in compliance with the License,
which is provided at the time of installation or download, or which
otherwise accompanies this software in either electronic or hard copy form.

You may obtain a copy of the License at

https://developer.oculus.com/licenses/audio-3.3/

Unless required by applicable law or agreed to in writing, the Oculus Audio SDK
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*******************************************************************************/

#pragma once

#include "Components/ActorComponent.h"
#include "CoreMinimal.h"

#include "OVRLipSyncActorComponentBase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOVRLipSyncVisemesDataReadyDelegate);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class OVRLIPSYNC_API UOVRLipSyncActorComponentBase : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UOVRLipSyncActorComponentBase();

	UFUNCTION(BlueprintPure, Category = "LipSync", Meta = (Tooltip = "Returns last predicted viseme scores"))
	const TArray<float> &GetVisemes() const;

	UFUNCTION(BlueprintPure, Category = "LipSync", Meta = (Tooltip = "Returns list of visme names"))
	const TArray<FString> &GetVisemeNames() const;

	UFUNCTION(BlueprintPure, Category = "LipSync", Meta = (Tooltip = "Returns predicted laughter probability"))
	const float GetLaughterScore() const;

	UFUNCTION(BlueprintCallable, Category = "LipSync",
			  Meta = (Tooltip = "Set skeletal mesh morph targets to the predicted viseme scores"))
	void AssignVisemesToMorphTargets(USkeletalMeshComponent *Mesh, const TArray<FString> &MorphTargetNames);

	UPROPERTY(BlueprintAssignable, Category = "LipSync",
			  Meta = (Tooltip = "Event triggered when new prediction is ready"))
	FOVRLipSyncVisemesDataReadyDelegate OnVisemesReady;

protected:
	// Set component internal state to a neutral psoe
	void InitNeutralPose();
	float LaughterScore = 0;

	TArray<float> Visemes;

	static const TArray<FString> VisemeNames;
};
