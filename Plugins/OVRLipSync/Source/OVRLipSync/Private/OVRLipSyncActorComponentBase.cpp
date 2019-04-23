/*******************************************************************************
 * Filename    :   OVRLipSyncActorComponentBase.cpp
 * Content     :   Base OVRLipSync Actor Component
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

#include "OVRLipSyncActorComponentBase.h"

#include "Classes/Components/SkeletalMeshComponent.h"

// Sets default values for this component's properties
UOVRLipSyncActorComponentBase::UOVRLipSyncActorComponentBase() { Visemes.Init(0.0f, VisemeNames.Num()); }

const TArray<float> &UOVRLipSyncActorComponentBase::GetVisemes() const { return Visemes; }

const TArray<FString> &UOVRLipSyncActorComponentBase::GetVisemeNames() const { return VisemeNames; }

const float UOVRLipSyncActorComponentBase::GetLaughterScore() const { return LaughterScore; }

void UOVRLipSyncActorComponentBase::AssignVisemesToMorphTargets(USkeletalMeshComponent *Mesh,
																const TArray<FString> &MorphTargetNames)
{
	for (int cnt = 0; cnt < MorphTargetNames.Num(); cnt++)
	{
		Mesh->SetMorphTarget(FName(*MorphTargetNames[cnt]), Visemes[cnt]);
	}
}

void UOVRLipSyncActorComponentBase::InitNeutralPose()
{
	if (LaughterScore == 0.0f && Visemes[0] == 1.0f)
	{
		return;
	}

	LaughterScore = 0.0f;
	Visemes[0] = 1.0f;
	for (int idx = 1; idx < Visemes.Num(); ++idx)
	{
		Visemes[idx] = 0.0f;
	}
	OnVisemesReady.Broadcast();
}
const TArray<FString> UOVRLipSyncActorComponentBase::VisemeNames = {
	FString(TEXT("sil")), FString(TEXT("PP")), FString(TEXT("FF")), FString(TEXT("TH")), FString(TEXT("DD")),
	FString(TEXT("kk")),  FString(TEXT("CH")), FString(TEXT("SS")), FString(TEXT("nn")), FString(TEXT("RR")),
	FString(TEXT("aa")),  FString(TEXT("E")),  FString(TEXT("ih")), FString(TEXT("oh")), FString(TEXT("ou")),
};
