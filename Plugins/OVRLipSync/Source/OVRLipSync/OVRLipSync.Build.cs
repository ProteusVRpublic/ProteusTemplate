/*******************************************************************************
 * Filename    :   OVRLipSync.Build.cs
 * Content     :   Unreal build script for OVRLipSync
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

using System.IO;
using UnrealBuildTool;

public class OVRLipSync : ModuleRules
{
    public OVRLipSync(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        string BaseDirectory = Path.GetFullPath(Path.Combine(ModuleDirectory, "..", ".."));
        string ThirdPartyDirectory = Path.Combine(BaseDirectory, "ThirdParty");
        string PlatformString = Target.Platform.ToString();
        string LibraryDirectory = Path.Combine(ThirdPartyDirectory, "Lib", PlatformString);
        string TargetBinariesDirectory = Path.Combine(BaseDirectory, "Binaries", PlatformString);
        PublicIncludePaths.Add(Path.Combine(ThirdPartyDirectory, "Include"));
        PublicDependencyModuleNames.AddRange( new string[] { "Core", "CoreUObject", "Engine", "Voice"});

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            PublicLibraryPaths.Add(LibraryDirectory);
            PublicAdditionalLibraries.Add("OVRLipSyncShim.lib");
            PublicDelayLoadDLLs.Add("OVRLipSync.dll");
            RuntimeDependencies.Add(Path.Combine(LibraryDirectory, "OVRLipSync.dll"), StagedFileType.NonUFS);
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            PublicAdditionalLibraries.Add(Path.Combine(LibraryDirectory, "libOVRLipSyncShim.a"));
            RuntimeDependencies.Add(Path.Combine(LibraryDirectory, "libOVRLipSync.dylib"), StagedFileType.NonUFS);
        }
        else if (Target.Platform == UnrealTargetPlatform.Android)
        {
            string Android64Directory = Path.Combine(ThirdPartyDirectory, "Lib", "Android64");
            PublicLibraryPaths.Add(LibraryDirectory);
            PublicLibraryPaths.Add(Android64Directory);
            AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(ModuleDirectory, "OVRLipSync_APL.xml"));
            PublicAdditionalLibraries.Add("OVRLipSyncShim");
        }

    }
}

