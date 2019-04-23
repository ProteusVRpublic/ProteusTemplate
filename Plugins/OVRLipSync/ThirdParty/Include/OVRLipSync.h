/*******************************************************************************
 * Filename    :   OVRLipSync.h
 * Content     :   Low-level interface to OVRLipSync library
 * Created     :   Oct 19th, 2015
 * Copyright   :   Copyright Facebook Technologies, LLC and its affiliates.
 *                 All rights reserved.
 *
 * Licensed under the Oculus Audio SDK License Version 3.3 (the "License");
 * you may not use the Oculus Audio SDK except in compliance with the License,
 * which is provided at the time of installation or download, or which
 * otherwise accompanies this software in either electronic or hard copy form.
 *
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
#ifndef OVR_LipSync_H
#define OVR_LipSync_H

#include <stdint.h>

// Version
#define OVR_LIPSYNC_MAJOR_VERSION 1
#define OVR_LIPSYNC_MINOR_VERSION 30
#define OVR_LIPSYNC_PATCH_VERSION 0

/// Result type used by the OVRLipSync API
/// Success is zero, while all error types are non-zero values.

typedef enum {
  ovrLipSyncSuccess = 0,
  // ERRORS
  /// An unknown error has occurred
  ovrLipSyncError_Unknown = -2200,
  /// Unable to create a context
  ovrLipSyncError_CannotCreateContext = -2201,
  /// An invalid parameter, e.g. NULL pointer or out of range
  ovrLipSyncError_InvalidParam = -2202,
  /// An unsupported sample rate was declared
  ovrLipSyncError_BadSampleRate = -2203,
  /// The DLL or shared library could not be found
  ovrLipSyncError_MissingDLL = -2204,
  /// Mismatched versions between header and libs
  ovrLipSyncError_BadVersion = -2205,
  /// An undefined function
  ovrLipSyncError_UndefinedFunction = -2206
} ovrLipSyncResult;

/// Audio buffer data type
typedef enum {
  // Signed 16-bit integer mono audio stream
  ovrLipSyncAudioDataType_S16_Mono,
  // Signed 16-bit integer stereo audio stream
  ovrLipSyncAudioDataType_S16_Stereo,
  // Signed 32-bit float mono data type
  ovrLipSyncAudioDataType_F32_Mono,
  // Signed 32-bit float stereo data type
  ovrLipSyncAudioDataType_F32_Stereo,
} ovrLipSyncAudioDataType;

/// Visemes
///
/// \see struct ovrLipSyncFrame
typedef enum {
  ovrLipSyncViseme_sil,
  ovrLipSyncViseme_PP,
  ovrLipSyncViseme_FF,
  ovrLipSyncViseme_TH,
  ovrLipSyncViseme_DD,
  ovrLipSyncViseme_kk,
  ovrLipSyncViseme_CH,
  ovrLipSyncViseme_SS,
  ovrLipSyncViseme_nn,
  ovrLipSyncViseme_RR,
  ovrLipSyncViseme_aa,
  ovrLipSyncViseme_E,
  ovrLipSyncViseme_ih,
  ovrLipSyncViseme_oh,
  ovrLipSyncViseme_ou,

  ovrLipSyncViseme_Count
} ovrLipSyncViseme;

/// Laughter types
///
/// \see struct ovrLipSyncFrame
typedef enum { ovrLipSyncLaughterCategory_Count } ovrLipSyncLaughterCategory;

/// Supported signals to send to LipSync API
///
/// \see ovrLipSync_SendSignal
typedef enum {
  ovrLipSyncSignals_VisemeOn, ///< fully on  (1.0f)
  ovrLipSyncSignals_VisemeOff, ///< fully off (0.0f)
  ovrLipSyncSignals_VisemeAmount, ///< Set to a given amount (0 - 100)
  ovrLipSyncSignals_VisemeSmoothing, ///< Amount to set per frame to target (0 - 100)
  ovrLipSyncSignals_LaughterAmount, ///< Set to a given amount (0.0-1.0)
  ovrLipSyncSignals_Count
} ovrLipSyncSignals;

/// Context provider
///
/// \see ovrLipSync_CreateContext
typedef enum {
  ovrLipSyncContextProvider_Original,
  ovrLipSyncContextProvider_Enhanced,
  ovrLipSyncContextProvider_EnhancedWithLaughter
} ovrLipSyncContextProvider;

/// Current LipSync frame results
///
/// \see ovrLipSync_ProcessFrame
/// \see ovrLipSync_ProcessFrameInterleaved
typedef struct {
  int frameNumber; ///< count from start of recognition
  int frameDelay; ///< in ms
  float* visemes; ///< ptr to Viseme array, sizeof ovrLipSyncViseme_Count
  unsigned int visemesLength; ///< number of visemes in array (must match internally)

  float laughterScore; ///< laughter score for the current audio frame
  float* laughterCategories; ///< per-laughter category score, sizeof
                             ///< ovrLipSyncLaughterCategory_Count
  unsigned int laughterCategoriesLength; ///< number of laughter categories
} ovrLipSyncFrame;

/// Opaque type def for LipSync context
typedef unsigned int ovrLipSyncContext;

typedef void (
    *ovrLipSyncCallback)(void* opaque, const ovrLipSyncFrame* pFrame, ovrLipSyncResult result);

/// Initialize OVRLipSync
///
/// Load the OVR LipSync library.  Call this first before any other ovrLipSync
/// functions!
///
/// \param[in] Default sample rate for all created context
/// \param[in] Default buffer size of all context
///
/// \return Returns an ovrLipSyncResult indicating success or failure.
///
/// \see ovrLipSync_Shutdown
ovrLipSyncResult ovrLipSync_Initialize(int sampleRate, int bufferSize);

/// Initialize OVRLipSyncEx
///
/// Load the OVR LipSync library.  Call this first before any other ovrLipSync
/// functions!
///
/// \param[in] Default sample rate for all created context
/// \param[in] Default buffer size of all context
/// \param[in] Path to the folder where OVR LipSync library is located.
///
/// \return Returns an ovrLipSyncResult indicating success or failure.
///
/// \see ovrLipSync_Shutdown
ovrLipSyncResult ovrLipSync_InitializeEx(int sampleRate, int bufferSize, const char* path);

/// Shutdown OVRLipSync
///
///  Shut down all of ovrLipSync.  Be sure to destroy any existing contexts
///  first!
///
///  \see ovrLipSync_Initialize
///
ovrLipSyncResult ovrLipSync_Shutdown(void);

/// Return library's built version information.
///
/// Can be called any time.
/// \param[out] Major pointer to integer that accepts major version number
/// \param[out] Minor pointer to integer that accepts minor version number
/// \param[out] Patch pointer to integer that accepts patch version number
///
/// \return Returns a string with human readable build information
///
const char* ovrLipSync_GetVersion(int* major, int* minor, int* patch);

/// Create a lip-sync context for transforming incoming audio
/// into a stream of visemes.
///
/// \param[out] pContext pointer to store address of context.
///             NOTE: pointer must be pointing to NULL!
/// \param[in] Provider sets the desired provider to use
/// \return Returns an ovrLipSyncResult indicating success or failure
/// \see ovrLipSync_DeleteContext
///
ovrLipSyncResult ovrLipSync_CreateContext(
    ovrLipSyncContext* pContext,
    const ovrLipSyncContextProvider provider);

/// Create a lip-sync context for transforming incoming audio
/// into a stream of visemes.
///
/// \param[out] pContext pointer to store address of context.
///             NOTE: pointer must be pointing to NULL!
/// \param[in] Provider sets the desired provider to use
/// \param[in] SampleRate sample rate of the audio stream
///            NOTE: Pass zero to use default sample rate
/// \param[in] EnableAcceleration sets the flag to allow DSP use on supported platforms
/// \return Returns an ovrLipSyncResult indicating success or failure
/// \see ovrLipSync_DeleteContext
///
ovrLipSyncResult ovrLipSync_CreateContextEx(
    ovrLipSyncContext* pContext,
    const ovrLipSyncContextProvider provider,
    const int sampleRate,
    const bool enableAcceleration);

/// Destroy a previously created lip-sync context.
///
/// \param[in] Context a valid lip-sync context
/// \see ovrLipSync_CreateContext
///
ovrLipSyncResult ovrLipSync_DestroyContext(ovrLipSyncContext context);

/// Reset the internal state of a lip-sync context.
///
/// \param[in] Context a valid lip-sync context
///
ovrLipSyncResult ovrLipSync_ResetContext(ovrLipSyncContext context);

/// Send context various signals to drive its output state.
///
/// \param[in] Context a valid lip-sync context
/// \param[in] Signal signal type to send to context
/// \param[in] Arg1 first arguement based on signal type
/// \param[in] Arg2 second arguement based on signal type
///
ovrLipSyncResult ovrLipSync_SendSignal(
    ovrLipSyncContext context,
    const ovrLipSyncSignals signal,
    const int arg1,
    const int arg2);

/// Send context a mono audio buffer and receive a frame of viseme data
///
/// \param[in] Context a valid lip-sync context
/// \param[in] A pointer to an audio buffer (mono)
/// \param[out] A reference to the current viseme frame
///
ovrLipSyncResult ovrLipSync_ProcessFrame(
    ovrLipSyncContext context,
    const float* audioBuffer,
    ovrLipSyncFrame* pFrame);

/// Send context an interleaved (stereo) audio buffer and receive a frame of viseme data
///
/// \param[in] Context a valid lip-sync context
/// \param[in] A pointer to an audio buffer (stereo)
/// \param[out] A reference to the current viseme frame
///
ovrLipSyncResult ovrLipSync_ProcessFrameInterleaved(
    ovrLipSyncContext context,
    const float* audioBuffer,
    ovrLipSyncFrame* pFrame);

/// Send context an audio buffer(mono or stereo) and receive a frame of viseme data
///
/// \param[in] Context a valid lip-sync context
/// \param[in] A pointer to an audio buffer
/// \param[in] Samples count
/// \param[in] Audio buffer data type: 32-bit float or 16-bit integer mono or stereo stream
/// \param[out] A reference to the current viseme frame
///
ovrLipSyncResult ovrLipSync_ProcessFrameEx(
    ovrLipSyncContext context,
    const void* audioBuffer,
    int sampleCount,
    ovrLipSyncAudioDataType dataType,
    ovrLipSyncFrame* pFrame);

/// Send context an audio buffer(mono or stereo) and receive a notification when processing is done
///
/// \param[in] Context a valid lip-sync context
/// \param[in] A pointer to an audio buffer
/// \param[in] Samples count
/// \param[in] Audio buffer data type: 32-bit float or 16-bit integer mono or stereo stream
/// \param[in] Pointer to a callback function
/// \param[in] Pointer to a context to be passed to a callback function
///
ovrLipSyncResult ovrLipSync_ProcessFrameAsync(
    ovrLipSyncContext context,
    const void* audioBuffer,
    int sampleCount,
    ovrLipSyncAudioDataType dataType,
    ovrLipSyncCallback callback,
    void* opaque);

#endif // OVR_LipSync_H
