#pragma once

#include "OVR_Avatar.h"
#include "OVR_CAPI.h"
#include "Vector.h"
#include "Quat.h"

static const float OvrAvatarScaleToUE4 = 100.0f;

extern float DebugLineScale;
extern bool DrawDebug;

namespace OvrAvatarHelpers
{
	inline void DebugDrawCoords(const UWorld* world, FTransform& trans)
	{
		if (!DrawDebug)
			return;

		DrawDebugCoordinateSystem(world, trans.GetLocation(), FRotator(trans.GetRotation()), DebugLineScale);
	}

	inline void ConvertTransform(const ovrAvatarTransform& oTransform, FTransform& outTransform)
	{
		outTransform.SetLocation(FVector(
			float(-oTransform.position.z * OvrAvatarScaleToUE4),
			float(oTransform.position.x  * OvrAvatarScaleToUE4),
			float(oTransform.position.y  * OvrAvatarScaleToUE4)
		));

		outTransform.SetRotation(FQuat(float(-oTransform.orientation.z),
			float(oTransform.orientation.x),
			float(oTransform.orientation.y),
			float(-oTransform.orientation.w)
		));
	}

	inline void OvrVector2fZero(ovrVector2f& vec)
	{
		vec.x = vec.y = 0.f;
	}

	inline void OvrVector3fZero(ovrVector3f& vec)
	{
		vec.x = vec.y = vec.z = 0.f;
	}

	inline void OvrQuatfIdentity(ovrQuatf& quat)
	{
		quat.x = quat.y = quat.z = 0.f;
		quat.w = 1.f;
	}

	inline void OvrPosefIdentity(ovrPosef& pose)
	{
		OvrQuatfIdentity(pose.Orientation);
		OvrVector3fZero(pose.Position);
	}

	inline void OvrPoseStatefIdentity(ovrPoseStatef& poseState)
	{
		OvrPosefIdentity(poseState.ThePose);
		OvrVector3fZero(poseState.AngularVelocity);
		OvrVector3fZero(poseState.LinearVelocity);
		OvrVector3fZero(poseState.AngularAcceleration);
		OvrVector3fZero(poseState.LinearAcceleration);
		poseState.TimeInSeconds = 0.0f;
	}

	inline void OvrAvatarVec3Zero(ovrAvatarVector3f& vec)
	{
		vec.x = vec.y = vec.z = 0.f;
	}

	inline void OvrAvatarQuatIdentity(ovrAvatarQuatf& quat)
	{
		quat.x = quat.y = quat.z = 0.f;
		quat.w = 1.f;
	}

	inline void OvrAvatarIdentity(ovrAvatarTransform& transform)
	{
		OvrAvatarVec3Zero(transform.position);
		OvrAvatarQuatIdentity(transform.orientation);

		transform.scale.x = 1.f;
		transform.scale.y = 1.f;
		transform.scale.z = 1.f;
	}

	inline void OvrAvatarTransformToSceneComponent(USceneComponent& sceneComponent, const ovrAvatarTransform& transform)
	{
		const FVector Location = OvrAvatarScaleToUE4 * FVector(-transform.position.z, transform.position.x, transform.position.y);
		const FVector Scale = FVector(transform.scale.x, transform.scale.y, transform.scale.z);
		FQuat Orientation = FQuat(-transform.orientation.z, transform.orientation.x, transform.orientation.y, -transform.orientation.w);
		Orientation.Normalize();

		sceneComponent.SetRelativeLocationAndRotation(Location, Orientation);
		sceneComponent.SetRelativeScale3D(Scale);
	}

	inline void OvrTrackingStateIdentity(ovrTrackingState& state)
	{
		OvrPosefIdentity(state.CalibratedOrigin);
		OvrPoseStatefIdentity(state.HeadPose);
		OvrPoseStatefIdentity(state.HandPoses[ovrHand_Left]);
		OvrPoseStatefIdentity(state.HandPoses[ovrHand_Right]);

		state.StatusFlags = 0;
		state.HandStatusFlags[ovrHand_Left] = state.HandStatusFlags[ovrHand_Right] = 0;
	}

	inline void OvrVector3fToAvatarVector3f(ovrAvatarVector3f& outVec, const ovrVector3f& inVec)
	{
		outVec.x = inVec.x;
		outVec.y = inVec.y;
		outVec.z = inVec.z;
	}

	inline void OvrQuat3fToAvatarQuat3f(ovrAvatarQuatf& outQuat, const ovrQuatf& inQuat)
	{
		outQuat.x = inQuat.x;
		outQuat.y = inQuat.y;
		outQuat.z = inQuat.z;
		outQuat.w = inQuat.w;
	}

	inline void OvrPoseToAvatarTransform(ovrAvatarTransform& trans, const ovrPosef& pose)
	{
		OvrVector3fToAvatarVector3f(trans.position, pose.Position);
		OvrQuat3fToAvatarQuat3f(trans.orientation, pose.Orientation);

		trans.scale.x = 1.0f;
		trans.scale.y = 1.0f;
		trans.scale.z = 1.0f;
	}

	inline void OvrAvatarHandISZero(ovrAvatarHandInputState& state)
	{
		OvrAvatarIdentity(state.transform);
		state.buttonMask = 0;
		state.touchMask = 0;
		state.joystickX = 0.f;
		state.joystickY = 0.f;
		state.indexTrigger = 0.f;
		state.handTrigger = 0.f;
		state.isActive = false;
	}

	inline void OvrInputStateZero(ovrInputState& state)
	{
		state.TimeInSeconds = 0;
		state.Buttons = 0;
		state.Touches = 0;
		state.IndexTrigger[ovrHand_Left] = 0.f;
		state.IndexTrigger[ovrHand_Right] = 0.f;

		state.HandTrigger[ovrHand_Left] = 0.f;
		state.HandTrigger[ovrHand_Right] = 0.f;

		state.IndexTriggerNoDeadzone[ovrHand_Left] = 0.f;
		state.IndexTriggerNoDeadzone[ovrHand_Right] = 0.f;

		state.HandTriggerNoDeadzone[ovrHand_Left] = 0.f;
		state.HandTriggerNoDeadzone[ovrHand_Right] = 0.f;

		OvrVector2fZero(state.Thumbstick[ovrHand_Left]);
		OvrVector2fZero(state.Thumbstick[ovrHand_Right]);

		OvrVector2fZero(state.ThumbstickNoDeadzone[ovrHand_Left]);
		OvrVector2fZero(state.ThumbstickNoDeadzone[ovrHand_Right]);

		state.ControllerType = ovrControllerType::ovrControllerType_None;
	}

	inline FLinearColor OvrAvatarVec4ToLinearColor(const ovrAvatarVector4f& v)
	{
		return FLinearColor(v.x, v.y, v.z, v.w);
	}
};
