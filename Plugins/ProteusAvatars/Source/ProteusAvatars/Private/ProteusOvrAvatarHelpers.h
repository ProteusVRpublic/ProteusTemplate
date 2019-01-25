#pragma once

#include "OVR_Avatar.h"
#include "Math/Vector.h"
#include "Math/Quat.h"
#include "OVR_Plugin.h"
#include "OVR_Plugin_Types.h"
#include "DrawDebugHelpers.h"

static const float OvrAvatarScaleToUE4 = 100.0f;

extern float DebugLineScale;
extern bool DrawDebug;

namespace ProteusOvrAvatarHelpers
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

	inline void OvrVector2fZero(ovrpVector2f& vec)
	{
		vec.x = vec.y = 0.f;
	}

	inline void OvrVector3fZero(ovrpVector3f& vec)
	{
		vec.x = vec.y = vec.z = 0.f;
	}

	inline void OvrQuatfIdentity(ovrpQuatf& quat)
	{
		quat.x = quat.y = quat.z = 0.f;
		quat.w = 1.f;
	}

	inline void OvrPosefIdentity(ovrpPosef& pose)
	{
		OvrQuatfIdentity(pose.Orientation);
		OvrVector3fZero(pose.Position);
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

	inline void OvrVector3fToAvatarVector3f(ovrAvatarVector3f& outVec, const ovrpVector3f& inVec)
	{
		outVec.x = inVec.x;
		outVec.y = inVec.y;
		outVec.z = inVec.z;
	}

	inline void OvrQuat3fToAvatarQuat3f(ovrAvatarQuatf& outQuat, const ovrpQuatf& inQuat)
	{
		outQuat.x = inQuat.x;
		outQuat.y = inQuat.y;
		outQuat.z = inQuat.z;
		outQuat.w = inQuat.w;
	}

	inline void OvrPoseToAvatarTransform(ovrAvatarTransform& trans, const ovrpPosef& pose)
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

	inline FLinearColor OvrAvatarVec4ToLinearColor(const ovrAvatarVector4f& v)
	{
		return FLinearColor(v.x, v.y, v.z, v.w);
	}

	inline void OvrAvatarParseButtonsAndTouches(
		const ovrpControllerState4& inputState, 
		const ovrpHand hand, 
		ovrAvatarHandInputState& state)
	{
		state.buttonMask = 0;
		state.touchMask = 0;

		if (hand == ovrpHand_Left)
		{
			if (inputState.Buttons & ovrpButton_X) state.buttonMask |= ovrAvatarButton_One;
			if (inputState.Buttons & ovrpButton_Y) state.buttonMask |= ovrAvatarButton_Two;
			if (inputState.Buttons & ovrpButton_Start) state.buttonMask |= ovrAvatarButton_Three;
			if (inputState.Buttons & ovrpButton_LThumb) state.buttonMask |= ovrAvatarButton_Joystick;

			if (inputState.Touches & ovrpTouch_X) state.touchMask |= ovrAvatarTouch_One;
			if (inputState.Touches & ovrpTouch_Y) state.touchMask |= ovrAvatarTouch_Two;
			if (inputState.Touches & ovrpTouch_LThumb) state.touchMask |= ovrAvatarTouch_Joystick;
			if (inputState.Touches & ovrpTouch_LThumbRest) state.touchMask |= ovrAvatarTouch_ThumbRest;
			if (inputState.Touches & ovrpTouch_LIndexTrigger) state.touchMask |= ovrAvatarTouch_Index;

			if (!((inputState.Touches & ovrpTouch_X) 
				|| (inputState.Touches & ovrpTouch_Y) 
				|| (inputState.Touches & ovrpTouch_LThumb) 
				|| (inputState.Touches & ovrpTouch_LThumbRest)))
			{
				state.touchMask |= ovrAvatarTouch_ThumbUp;
			}
			else
			{
				state.touchMask &= ~(ovrAvatarTouch_ThumbUp);
			}

			if (!(inputState.Touches & ovrpTouch_LIndexTrigger))
			{
				state.touchMask |= ovrAvatarTouch_Pointing;
			}
			else
			{
				state.touchMask &= ~(ovrAvatarTouch_Pointing);
			}
		}
		else if (hand == ovrpHand_Right)
		{
			//Oculus Go Back Button, Rift & Quest A Button (Fires Once)
			if (inputState.Buttons & ovrpButton_A) state.buttonMask |= ovrAvatarButton_One;

			//Oculus Go Trigger, Rift & Quest Motion Controller R Trigger (Fires Once)
			//Oculus Go Trigger axis, Rift & Quest Motion Controller R Trigger axis

			//Oculus Go X axis Touchpad, Rift & Quest X axis R Thumbstick
			//Oculus Go Y axis Touchpad, Rift & Quest Y axis R Thumbstick
			
			//Oculus Go Touchpad, Rift & Quest Thumbstick Button (Fires Continuously)
			if (inputState.Buttons & ovrpButton_RThumb) state.buttonMask |= ovrAvatarButton_Joystick;

			//Oculus Rift & Quest B Button (Fires Once)
			if (inputState.Buttons & ovrpButton_B) state.buttonMask |= ovrAvatarButton_Two;
			//Oculus Rift & Quest Home Button (Fires Once)
			if (inputState.Buttons & ovrpButton_Home) state.buttonMask |= ovrAvatarButton_Three;

			if (inputState.Touches & ovrpTouch_A) state.touchMask |= ovrAvatarTouch_One;
			if (inputState.Touches & ovrpTouch_B) state.touchMask |= ovrAvatarTouch_Two;
			if (inputState.Touches & ovrpTouch_RThumb) state.touchMask |= ovrAvatarTouch_Joystick;
			if (inputState.Touches & ovrpTouch_RThumbRest) state.touchMask |= ovrAvatarTouch_ThumbRest;
			if (inputState.Touches & ovrpTouch_RIndexTrigger) state.touchMask |= ovrAvatarTouch_Index;

			if (!((inputState.Touches & ovrpTouch_A)
				|| (inputState.Touches & ovrpTouch_B)
				|| (inputState.Touches & ovrpTouch_RThumb)
				|| (inputState.Touches & ovrpTouch_RThumbRest)))
			{
				state.touchMask |= ovrAvatarTouch_ThumbUp;
			}
			else
			{
				state.touchMask &= ~(ovrAvatarTouch_ThumbUp);
			}

			if (!(inputState.Touches & ovrpTouch_RIndexTrigger))
			{
				state.touchMask |= ovrAvatarTouch_Pointing;
			}
			else
			{
				state.touchMask &= ~(ovrAvatarTouch_Pointing);
			}
		}
	}
};
