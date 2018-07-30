// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "UObject/ScriptMacros.h"
#include "Camera/CameraTypes.h"
#include "ShakerAnimation.h"
#include "ShakerAnimationInstance.generated.h"

UCLASS(notplaceable, BlueprintType, transient)
class SHAKER_API UShakerAnimationInstance : public UObject
{
	GENERATED_UCLASS_BODY()

	/** Which animation this is an instance of. */
	UPROPERTY()
	class UShakerAnimation* AnimationClass;

	/** Current time for the animation */
	float CurrentTime;

	/** True if the animation has finished, false otherwise. */
	uint32 bFinished : 1;

	/** Multiplier for playback rate.  1.0 = normal. */
	UPROPERTY(BlueprintReadWrite, Category = "Shake Animation")
	float PlayRate;

	UPROPERTY()
	TEnumAsByte<ECameraAnimPlaySpace::Type> PlaySpace;

	/** "Intensity" value used to scale keyframe values. */
	float BasePlayScale;

	/** A supplemental scale factor, allowing external systems to scale this animation as necessary.  This is reset to 1.f each frame. */
	float TransientScaleModifier;

	/* Number in range [0..1], controlling how much this influence this instance should have. */
	float CurrentBlendWeight;

	/** cached movement track from the currently playing animation so we don't have to go find it every frame */
	UPROPERTY(transient)
	class UInterpTrackMove* MoveTrack;

	UPROPERTY(transient)
	class UInterpTrackInstMove* MoveInstance;

	/** The user-defined space for UserDefined PlaySpace */
	FMatrix UserPlaySpaceMatrix;

	/** Camera Anim debug variable to trace back to previous location **/
	FVector LastCameraLoc;

	/** transform of initial animation key, used for treating animation keys as offsets from initial key */
	FTransform InitialCamToWorld;

	/**
	* Starts this instance playing the specified CameraAnim.
	*
	* @param CamAnim			The animation that should play on this instance.
	* @param CamActor			The  AActor  that will be modified by this animation.
	* @param InRate			How fast to play the animation.  1.f is normal.
	* @param InScale			How intense to play the animation.  1.f is normal.
	* @param InBlendInTime		Time over which to linearly ramp in.
	* @param InBlendOutTime	Time over which to linearly ramp out.
	* @param bInLoop			Whether or not to loop the animation.
	* @param bRandomStartTime	Whether or not to choose a random time to start playing.  Only really makes sense for bLoop = true;
	* @param Duration			optional specific playtime for this animation.  This is total time, including blends.
	*/
	void Play(class UShakerAnimation* Animation, class AActor* Actor, float InRate, float InScale, float InBlendInTime, float InBlendOutTime, bool bInLoop, bool bRandomStartTime, float Duration = 0.f);

	/** Updates this active instance with new parameters. */
	void Update(float NewRate, float NewScale, float NewBlendInTime, float NewBlendOutTime, float NewDuration = 0.f);

	/** advances the animation by the specified time - updates any modified interpolation properties, moves the group actor, etc */
	void AdvanceAnim(float DeltaTime, bool bJump);

	/** Jumps he camera animation to the given (unscaled) time. */
	void SetCurrentTime(float NewTime);

	/** Returns the current playback time. */
	float GetCurrentTime() const { return CurrentTime; };

	/** Stops this instance playing whatever animation it is playing. */
	UFUNCTION(BlueprintCallable, Category = CameraAnimInst)
		void Stop(bool bImmediate = false);

	/** Applies given scaling factor to the playing animation for the next update only. */
	void ApplyTransientScaling(float Scalar);

	/** Changes the running duration of this active animation, while maintaining playback position. */
	UFUNCTION(BlueprintCallable, Category = CameraAnimInst)
		void SetDuration(float NewDuration);

	/** Changes the scale of the animation while playing.*/
	UFUNCTION(BlueprintCallable, Category = CameraAnimInst)
		void SetScale(float NewDuration);

	/** Sets this animation to play in an alternate play space */
	void SetPlaySpace(ECameraAnimPlaySpace::Type NewSpace, FRotator UserPlaySpace = FRotator::ZeroRotator);

	/** Takes the given view and applies the camera animation transform and fov changes to it. Does not affect PostProcess. */
	void ApplyToView(FMinimalViewInfo& InOutPOV) const;

	/** Sets whether this animation instance should automatically stop when finished. */
	void SetStopAutomatically(bool bNewStopAutoMatically) { bStopAutomatically = bNewStopAutoMatically; };

protected:

	/** the UInterpGroupInst used to do the interpolation */
	class UInterpGroupInst* InterpGroupInst;

	/** If true, this anim inst will automatically stop itself when it finishes, otherwise, it will wait for an explicit Stop() call. */
	uint32 bStopAutomatically : 1;

	/** True if the animation should loop, false otherwise. */
	uint32 bLooping : 1;

	/** True if currently blending in. */
	uint32 bBlendingIn : 1;

	/** True if currently blending out. */
	uint32 bBlendingOut : 1;

	/** Time to interpolate in from zero, for smooth starts. */
	float BlendInTime;

	/** Time to interpolate out to zero, for smooth finishes. */
	float BlendOutTime;

	/** Current time for the blend-in.  I.e. how long we have been blending. */
	float CurBlendInTime;

	/** Current time for the blend-out.  I.e. how long we have been blending. */
	float CurBlendOutTime;

	/** How much longer to play the animation, if a specific duration is desired.  Has no effect if 0.  */
	float RemainingTime;

	/** Returns InterpGroupInst subobject **/
	class UInterpGroupInst* GetInterpGroupInst() const;
};
