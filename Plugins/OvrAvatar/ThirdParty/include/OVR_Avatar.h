#ifndef OVR_Avatar_h
#define OVR_Avatar_h

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#ifdef OVR_AVATAR_DLL
#define OVRN_EXPORT __declspec( dllexport )
#else
#define OVRN_EXPORT
#endif
#else
#define OVRN_EXPORT
#endif

//Opaque types
typedef uint64_t ovrAvatarAssetID;
typedef struct ovrAvatarMessage_ ovrAvatarMessage;
typedef struct ovrAvatarSpecification_ ovrAvatarSpecification;
typedef struct ovrAvatarAsset_ ovrAvatarAsset;
typedef struct ovrAvatar_ ovrAvatar;
typedef struct ovrAvatarRenderPart_ ovrAvatarRenderPart;
typedef struct ovrAvatarPacket_ ovrAvatarPacket;

#define OVR_AVATAR_MAXIMUM_JOINT_COUNT 64

//Basic plugin management
OVRN_EXPORT void ovrAvatar_Initialize(const char* appId);
OVRN_EXPORT void ovrAvatar_Shutdown();

//Message system

typedef enum ovrAvatarMessageType_ {
    ovrAvatarMessageType_AvatarSpecification,
    ovrAvatarMessageType_AssetLoaded,
    ovrAvatarMessageType_Count
} ovrAvatarMessageType;

typedef struct ovrAvatarMessage_AvatarSpecification_ {
	ovrAvatarSpecification* avatarSpec;
	uint64_t                oculusUserID;
} ovrAvatarMessage_AvatarSpecification;

typedef struct ovrAvatarMessage_AssetLoaded_ {
	ovrAvatarAssetID assetID;
	ovrAvatarAsset* asset;
} ovrAvatarMessage_AssetLoaded;


OVRN_EXPORT ovrAvatarMessage* ovrAvatarMessage_Pop();
OVRN_EXPORT ovrAvatarMessageType ovrAvatarMessage_GetType(
	const ovrAvatarMessage* msg);
OVRN_EXPORT const ovrAvatarMessage_AvatarSpecification* ovrAvatarMessage_GetAvatarSpecification(
	const ovrAvatarMessage* msg);
OVRN_EXPORT const ovrAvatarMessage_AssetLoaded* ovrAvatarMessage_GetAssetLoaded(
	const ovrAvatarMessage* msg);
OVRN_EXPORT void ovrAvatarMessage_Free(
	const ovrAvatarMessage* msg);

//Avatar capabilities
typedef enum ovrAvatarCapabilities_ {
	ovrAvatarCapability_Body = 1 << 0,
	ovrAvatarCapability_Hands = 1 << 1,
	ovrAvatarCapability_Base = 1 << 2,
	ovrAvatarCapability_Voice = 1 << 3,
	ovrAvatarCapability_All = -1
}ovrAvatarCapabilities;

//Avatar creation and destruction
OVRN_EXPORT void ovrAvatar_RequestAvatarSpecification(
	uint64_t userID);
OVRN_EXPORT ovrAvatar* ovrAvatar_Create(
	const ovrAvatarSpecification* avatarSpecification,
	ovrAvatarCapabilities capabilities);
OVRN_EXPORT void ovrAvatar_Destroy(ovrAvatar* avatar);

//Avatar updates
typedef struct ovrAvatarVector3f_ {
	float x, y, z;
} ovrAvatarVector3f;

typedef struct ovrAvatarVector4f_ {
	float x, y, z, w;
} ovrAvatarVector4f;

typedef struct ovrAvatarQuatf_ {
	float x, y, z, w;
} ovrAvatarQuatf;

typedef struct ovrAvatarTransform_ {
	ovrAvatarVector3f position;
	ovrAvatarQuatf orientation;
	ovrAvatarVector3f scale;
} ovrAvatarTransform;

typedef enum ovrAvatarButton_ {
	ovrAvatarButton_One = 0x0001,
	ovrAvatarButton_Two = 0x0002,
	ovrAvatarButton_Three = 0x0004,
	ovrAvatarButton_Joystick = 0x0008,
} ovrAvatarButton;

typedef enum ovrAvatarTouch_ {
	ovrAvatarTouch_One = 0x0001,
	ovrAvatarTouch_Two = 0x0002,
	ovrAvatarTouch_Joystick = 0x0004,
	ovrAvatarTouch_ThumbRest = 0x0008,
	ovrAvatarTouch_Index = 0x0010,
	ovrAvatarTouch_Pointing = 0x0040,
	ovrAvatarTouch_ThumbUp = 0x0080,
} ovrAvatarTouch;

typedef struct ovrAvatarHandInputState_ {
	ovrAvatarTransform transform;
	uint32_t buttonMask;
	uint32_t touchMask;
	float joystickX;
	float joystickY;
	float indexTrigger;
	float handTrigger;
	bool isActive;
} ovrAvatarHandInputState;

OVRN_EXPORT void ovrAvatarPose_UpdateBody(ovrAvatar* avatar,
	                                      ovrAvatarTransform headPose);
OVRN_EXPORT void ovrAvatarPose_UpdateVoiceVisualization(ovrAvatar* avatar,
														uint32_t voiceSampleCount,
										    			const float voiceSamples[]);
OVRN_EXPORT void ovrAvatarPose_UpdateHands(ovrAvatar* avatar,
											ovrAvatarHandInputState inputStateLeft,
											ovrAvatarHandInputState inputStateRight);
OVRN_EXPORT void ovrAvatarPose_Finalize(ovrAvatar* avatar, float deltaSeconds);

//Showing controllers
OVRN_EXPORT void ovrAvatar_SetLeftControllerVisibility(ovrAvatar* avatar, bool visible);
OVRN_EXPORT void ovrAvatar_SetRightControllerVisibility(ovrAvatar* avatar, bool visible);

//Hand poses/grips
typedef enum ovrAvatarHandGesture_ {
    ovrAvatarHandGesture_Default,
    ovrAvatarHandGesture_GripSphere,
    ovrAvatarHandGesture_GripCube,
    ovrAvatarHandGesture_Count
} ovrAvatarHandGesture;

OVRN_EXPORT void ovrAvatar_SetLeftHandGesture(ovrAvatar* avatar, ovrAvatarHandGesture);
OVRN_EXPORT void ovrAvatar_SetRightHandGesture(ovrAvatar* avatar, ovrAvatarHandGesture);

OVRN_EXPORT void ovrAvatar_SetLeftHandCustomGesture(ovrAvatar* avatar, uint32_t jointCount, const ovrAvatarTransform *customJointTransforms);
OVRN_EXPORT void ovrAvatar_SetRightHandCustomGesture(ovrAvatar* avatar, uint32_t jointCount, const ovrAvatarTransform *customJointTransforms);

OVRN_EXPORT void ovrAvatar_SetActiveCapabilities(ovrAvatar* avatar, ovrAvatarCapabilities);

//Base manipulation
OVRN_EXPORT void ovrAvatar_ClearCustomBasePosition(ovrAvatar* avatar);
OVRN_EXPORT void ovrAvatar_SetCustomBasePosition(ovrAvatar* avatar, ovrAvatarVector3f position);

// Asset manifest helpers
OVRN_EXPORT uint32_t ovrAvatar_GetReferencedAssetCount(ovrAvatar* avatar);
OVRN_EXPORT ovrAvatarAssetID ovrAvatar_GetReferencedAsset(ovrAvatar* avatar, uint32_t index);

//Avatar rendering
typedef enum ovrAvatarMaterialLayerBlendMode_ {
	ovrAvatarMaterialLayerBlendMode_Add,
	ovrAvatarMaterialLayerBlendMode_Multiply,
	ovrAvatarMaterialLayerBlendMode_Count
} ovrAvatarMaterialLayerBlendMode;

typedef enum ovrAvatarMaterialLayerSampleMode_ {
	ovrAvatarMaterialLayerSampleMode_Color,
	ovrAvatarMaterialLayerSampleMode_Texture,
	ovrAvatarMaterialLayerSampleMode_TextureSingleChannel,
	ovrAvatarMaterialLayerSampleMode_Parallax,
	ovrAvatarMaterialLayerSampleMode_Count
} ovrAvatarMaterialLayerSampleMode;

typedef enum ovrAvatarMaterialMaskType_ {
	ovrAvatarMaterialMaskType_None,
	ovrAvatarMaterialMaskType_Positional,
	ovrAvatarMaterialMaskType_ViewReflection,
	ovrAvatarMaterialMaskType_Fresnel,
	ovrAvatarMaterialMaskType_Pulse,
	ovrAvatarMaterialMaskType_Count
} ovrAvatarMaterialMaskType;

typedef struct ovrAvatarMaterialLayerState_ {
	ovrAvatarMaterialLayerBlendMode     blendMode;
	ovrAvatarMaterialLayerSampleMode    sampleMode;
	ovrAvatarMaterialMaskType			maskType;
	ovrAvatarVector4f					layerColor;
	ovrAvatarVector4f                   sampleParameters;
	ovrAvatarAssetID                    sampleTexture;
	ovrAvatarVector4f					sampleScaleOffset;
	ovrAvatarVector4f                   maskParameters;
	ovrAvatarVector4f                   maskAxis;
} ovrAvatarMaterialLayerState;

#define OVR_AVATAR_MAX_MATERIAL_LAYER_COUNT 8

typedef struct ovrAvatarMaterialState_ {
	ovrAvatarVector4f			baseColor;
	ovrAvatarMaterialMaskType	baseMaskType;
	ovrAvatarVector4f			baseMaskParameters;
	ovrAvatarVector4f			baseMaskAxis;
	ovrAvatarAssetID            alphaMaskTextureID;
	ovrAvatarVector4f			alphaMaskScaleOffset;
	ovrAvatarAssetID			normalMapTextureID;
	ovrAvatarVector4f			normalMapScaleOffset;
	ovrAvatarAssetID            parallaxMapTextureID;
	ovrAvatarVector4f			parallaxMapScaleOffset;
	ovrAvatarAssetID			roughnessMapTextureID;
	ovrAvatarVector4f			roughnessMapScaleOffset;
	uint32_t                    layerCount;
	ovrAvatarMaterialLayerState layers[OVR_AVATAR_MAX_MATERIAL_LAYER_COUNT];
} ovrAvatarMaterialState;

typedef struct ovrAvatarSkinnedMeshPose_ {
    int jointCount;
	ovrAvatarTransform jointTransform[OVR_AVATAR_MAXIMUM_JOINT_COUNT];
	int jointParents[OVR_AVATAR_MAXIMUM_JOINT_COUNT];
	const char * jointNames[OVR_AVATAR_MAXIMUM_JOINT_COUNT];
} ovrAvatarSkinnedMeshPose;

typedef enum ovrAvatarVisibilityFlags_ {
	ovrAvatarVisibilityFlag_FirstPerson = 1 << 0,
	ovrAvatarVisibilityFlag_ThirdPerson = 1 << 1,
	ovrAvatarVisibilityFlag_SelfOccluding = 1 << 2,
} ovrAvatarVisibilityFlags;

typedef struct ovrAvatarRenderPart_SkinnedMeshRender_ {
	ovrAvatarTransform       localTransform;
	uint32_t				 visibilityMask; // ovrAvatarVisibilityFlags
	ovrAvatarAssetID         meshAssetID;
	ovrAvatarMaterialState   materialState;
	ovrAvatarSkinnedMeshPose skinnedPose;
} ovrAvatarRenderPart_SkinnedMeshRender;

typedef struct ovrAvatarRenderPart_SkinnedMeshRenderPBS_ {
	ovrAvatarTransform       localTransform;
	uint32_t				 visibilityMask; // ovrAvatarVisibilityFlags
	ovrAvatarAssetID         meshAssetID;
	ovrAvatarAssetID		 albedoTextureAssetID;
	ovrAvatarAssetID		 surfaceTextureAssetID;
	ovrAvatarSkinnedMeshPose skinnedPose;
} ovrAvatarRenderPart_SkinnedMeshRenderPBS;

typedef struct ovrAvatarRenderPart_ProjectorRender_ {
	ovrAvatarTransform		localTransform;
	uint32_t				componentIndex;
	uint32_t				renderPartIndex;
	ovrAvatarMaterialState	materialState;
} ovrAvatarRenderPart_ProjectorRender;

typedef struct ovrAvatarComponent_ {
	ovrAvatarTransform transform;
	uint32_t renderPartCount;
	const ovrAvatarRenderPart* const* renderParts;
	const char* name;
} ovrAvatarComponent;

typedef enum ovrAvatarRenderPartType_ {
    ovrAvatarRenderPartType_SkinnedMeshRender,
	ovrAvatarRenderPartType_SkinnedMeshRenderPBS,
	ovrAvatarRenderPartType_ProjectorRender,
    ovrAvatarRenderPartType_Count
} ovrAvatarRenderPartType;

OVRN_EXPORT ovrAvatarRenderPartType ovrAvatarRenderPart_GetType(
	const ovrAvatarRenderPart* renderPart);
OVRN_EXPORT const ovrAvatarRenderPart_SkinnedMeshRender* ovrAvatarRenderPart_GetSkinnedMeshRender(
	const ovrAvatarRenderPart* renderPart);
OVRN_EXPORT const ovrAvatarRenderPart_SkinnedMeshRenderPBS* ovrAvatarRenderPart_GetSkinnedMeshRenderPBS(
	const ovrAvatarRenderPart* renderPart);
OVRN_EXPORT const ovrAvatarRenderPart_ProjectorRender* ovrAvatarRenderPart_GetProjectorRender(
	const ovrAvatarRenderPart* renderPart);
OVRN_EXPORT uint32_t ovrAvatarComponent_Count(const ovrAvatar* avatar);
OVRN_EXPORT const ovrAvatarComponent* ovrAvatarComponent_Get(
	const ovrAvatar* avatar, uint32_t index);

//Avatar semantic information
typedef struct ovrAvatarBodyComponent_ {
	ovrAvatarTransform  leftEyeTransform;
	ovrAvatarTransform  rightEyeTransform;
	ovrAvatarTransform  centerEyeTransform;
	const ovrAvatarComponent* renderComponent;
} ovrAvatarBodyComponent;

typedef struct ovrAvatarControllerComponent_ {
	ovrAvatarHandInputState inputState;
	const ovrAvatarComponent* renderComponent;
} ovrAvatarControllerComponent;

typedef struct ovrAvatarBaseComponent_ {
	ovrAvatarVector3f basePosition;
	const ovrAvatarComponent* renderComponent;
} ovrAvatarBaseComponent;

typedef struct ovrAvatarHandComponent_ {
	ovrAvatarHandInputState inputState;
	const ovrAvatarComponent* renderComponent;
} ovrAvatarHandComponent;

OVRN_EXPORT const ovrAvatarBodyComponent* ovrAvatarPose_GetBodyComponent(
	ovrAvatar* avatar);
OVRN_EXPORT const ovrAvatarBaseComponent* ovrAvatarPose_GetBaseComponent(
	ovrAvatar* avatar);
OVRN_EXPORT const ovrAvatarControllerComponent* ovrAvatarPose_GetLeftControllerComponent(
	ovrAvatar* avatar);
OVRN_EXPORT const ovrAvatarControllerComponent* ovrAvatarPose_GetRightControllerComponent(
	ovrAvatar* avatar);
OVRN_EXPORT const ovrAvatarHandComponent* ovrAvatarPose_GetLeftHandComponent(
	ovrAvatar* avatar);
OVRN_EXPORT const ovrAvatarHandComponent* ovrAvatarPose_GetRightHandComponent(
	ovrAvatar* avatar);

//Assets
typedef enum ovrAvatarAssetType_ {
    ovrAvatarAssetType_Mesh,
    ovrAvatarAssetType_Texture,
	ovrAvatarAssetType_Pose,
    ovrAvatarAssetType_Count
} ovrAvatarAssetType;

typedef struct ovrAvatarMeshVertex_ {
    float   x;
    float   y;
    float   z;
    float   nx;
    float   ny;
    float   nz;
	float	tx;
	float	ty;
	float	tz;
	float	tw;
    float   u;
    float   v;
    uint8_t blendIndices[4];
    float   blendWeights[4];
} ovrAvatarMeshVertex;

typedef struct ovrAvatarMeshAssetData_ {
	uint32_t                   vertexCount;
	const ovrAvatarMeshVertex* vertexBuffer;
	uint32_t                   indexCount;
	const uint16_t*            indexBuffer;
	ovrAvatarSkinnedMeshPose   skinnedBindPose;
} ovrAvatarMeshAssetData;

typedef enum ovrAvatarTextureFormat_ {
	ovrAvatarTextureFormat_RGB24,
	ovrAvatarTextureFormat_DXT1,
	ovrAvatarTextureFormat_DXT5,
	ovrAvatarTextureFormat_Count
} ovrAvatarTextureFormat;

typedef struct ovrAvatarTextureAssetData_ {
	ovrAvatarTextureFormat	format;
	uint32_t				sizeX;
	uint32_t				sizeY;
	uint32_t				mipCount;
	uint64_t				textureDataSize;
	const uint8_t*			textureData;
} ovrAvatarTextureAssetData;

OVRN_EXPORT void ovrAvatarAsset_BeginLoading(
	ovrAvatarAssetID assetID);
OVRN_EXPORT ovrAvatarAssetType ovrAvatarAsset_GetType(
	const ovrAvatarAsset* asset);
OVRN_EXPORT const ovrAvatarMeshAssetData* ovrAvatarAsset_GetMeshData(
	const ovrAvatarAsset* asset);
OVRN_EXPORT const ovrAvatarTextureAssetData* ovrAvatarAsset_GetTextureData(
	const ovrAvatarAsset* asset);

//Avatar recording and playback
OVRN_EXPORT void ovrAvatarPacket_BeginRecording(ovrAvatar* avatar);
OVRN_EXPORT ovrAvatarPacket* ovrAvatarPacket_EndRecording(ovrAvatar* avatar);
OVRN_EXPORT uint32_t ovrAvatarPacket_GetSize(const ovrAvatarPacket* packet);
OVRN_EXPORT bool ovrAvatarPacket_Write(const ovrAvatarPacket* packet,
	                                   uint32_t targetSize,
									   uint8_t* targetBuffer);
OVRN_EXPORT ovrAvatarPacket* ovrAvatarPacket_Read(uint32_t bufferSize,
	                                              const uint8_t buffer[]);
OVRN_EXPORT float ovrAvatarPacket_GetDurationSeconds(
	const ovrAvatarPacket* packet);
OVRN_EXPORT void ovrAvatarPacket_Free(ovrAvatarPacket* packet);

OVRN_EXPORT void ovrAvatar_UpdatePoseFromPacket(
	ovrAvatar* avatar,
	const ovrAvatarPacket* packet,
	float secondsFromPacketStart);

#ifdef __cplusplus
}
#endif

#endif // OVR_Avatar_h