/*******************************************************************************
 * Filename    :   OVRLipSyncContextWrapper.cpp
 * Content     :   Wrapper around OVRLipSync API
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

#include "OVRLipSyncContextWrapper.h"

#include <Core.h>
#include <algorithm>

#define DEFAULT_SAMPLE_RATE 44100
#define DEFAULT_BUFFER_SIZE 4096

UOVRLipSyncContextWrapper::UOVRLipSyncContextWrapper(ovrLipSyncContextProvider ProviderKind, int SampleRate,
													 bool EnableAcceleration)
{
#if !PLATFORM_ANDROID
	auto pluginsDir = FPaths::ProjectPluginsDir();
	auto libDir = FPaths::Combine(pluginsDir, TEXT("OVRLipSync"), TEXT("ThirdParty"), TEXT("Lib"),
								  FPlatformProcess::GetBinariesSubdirectory());
	TArray<char> libDirChar(libDir.GetCharArray());
	auto rc = ovrLipSync_InitializeEx(DEFAULT_SAMPLE_RATE, DEFAULT_BUFFER_SIZE, libDirChar.GetData());
#else
	auto rc = ovrLipSync_Initialize(DEFAULT_SAMPLE_RATE, DEFAULT_BUFFER_SIZE);
#endif
	if (rc != ovrLipSyncSuccess)
	{
		UE_LOG(LogTemp, Error, TEXT("Can't initialize ovrLipSync: %d"), rc);
		return;
	}
	rc = ovrLipSync_CreateContextEx(&LipSyncContext, ProviderKind, SampleRate, EnableAcceleration);
	if (rc != ovrLipSyncSuccess)
	{
		UE_LOG(LogTemp, Error, TEXT("Can't create ovrLipSync context: %d"), rc);
	}
}

UOVRLipSyncContextWrapper::~UOVRLipSyncContextWrapper() { ovrLipSync_DestroyContext(LipSyncContext); }

void UOVRLipSyncContextWrapper::ProcessFrame(const int16_t *AudioBuffer, int AudioBufferSize, TArray<float> &Visemes,
											 float &LaughterScore, bool Stereo)
{
	if (Visemes.Num() != ovrLipSyncViseme_Count)
	{
		Visemes.SetNumZeroed(ovrLipSyncViseme_Count);
	}
	ovrLipSyncFrame frame = {};
	frame.visemes = Visemes.GetData();
	frame.visemesLength = Visemes.Num();
	auto rc = ovrLipSync_ProcessFrameEx(LipSyncContext, AudioBuffer, AudioBufferSize,
										Stereo ? ovrLipSyncAudioDataType_S16_Stereo : ovrLipSyncAudioDataType_S16_Mono,
										&frame);
	if (rc != ovrLipSyncSuccess)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to process frame: %d"), rc);
		return;
	}
	LaughterScore = frame.laughterScore;
}
