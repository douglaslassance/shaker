// Copyright (c) 2017 Douglas Lassance. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Camera/CameraShake.h"
#include "ShakerShake.generated.h"

class UShakerComponent;

UCLASS(Blueprintable, editinlinenew, meta = (DisplayName = "Shake"))
class SHAKER_API UShakerShake : public UObject
{
	GENERATED_BODY()

public:

	UShakerShake();

	/**
	*  If true to only allow a single instance of this shake class to play at any given time.
	*  Subsequent attempts to play this shake will simply restart the timer.
	*/
	UPROPERTY(EditAnywhere, Category = "Shake")
	uint32 bSingleInstance : 1;

	/************************************************************
	* Oscillation stuff.
	************************************************************/

	/** Duration in seconds of current screen shake. Less than 0 means indefinite, 0 means no oscillation. */
	UPROPERTY(EditAnywhere, Category = "Oscillation")
	float OscillationDuration;

	/** Duration of the blend-in, where the oscillation scales from 0 to 1. */
	UPROPERTY(EditAnywhere, Category = "Oscillation", meta = (ClampMin = "0.0"))
	float OscillationBlendInTime;

	/** Duration of the blend-out, where the oscillation scales from 1 to 0. */
	UPROPERTY(EditAnywhere, Category = "Oscillation", meta = (ClampMin = "0.0"))
	float OscillationBlendOutTime;

	/** Rotational oscillation. */
	UPROPERTY(EditAnywhere, Category = "Oscillation")
	struct FROscillator RotationalOscillation;

	/** Positional oscillation. */
	UPROPERTY(EditAnywhere, Category = "Oscillation")
	struct FVOscillator PositionalOscillation;

	/************************************************************
	* Animation stuff.
	************************************************************/

	/** Scalar defining how fast to play the animation. */
	UPROPERTY(EditAnywhere, Category = "Animation", meta = (ClampMin = "0.001"))
	float AnimationPlayRate;

	/** Scalar defining how "intense" to play the animation. */
	UPROPERTY(EditAnywhere, Category = "Animation", meta = (ClampMin = "0.0"))
	float AnimationScale;

	/** Linear blend-in time. */
	UPROPERTY(EditAnywhere, Category = "Animation", meta = (ClampMin = "0.0"))
	float AnimationBlendInTime;

	/** Linear blend-out time. */
	UPROPERTY(EditAnywhere, Category = "Animation", meta = (ClampMin = "0.0"))
	float AnimationBlendOutTime;

	/** When bRandomAnimSegment is true, this defines how long the animation should play. */
	UPROPERTY(EditAnywhere, Category = "Animation", meta = (ClampMin = "0.0", editcondition = "bRandomAnimSegment"))
	float RandomAnimationSegmentDuration;

	/** Source camera animation to play. Can be null. */
	UPROPERTY(EditAnywhere, Category = "Animation")
	class UShakerAnimation* Animation;

	/** The playing instance of the CameraAnim-based shake, if any. */
	UPROPERTY(transient, BlueprintReadOnly, Category = "Shake")
	class UShakerAnimationInstance* AnimationInstance;

	/**
	* If true, play a random snippet of the animation of length Duration.  Implies bLoop and bRandomStartTime = true for the CameraAnim.
	* If false, play the full animation once, non-looped. Useful for getting variety out of a single looped CameraAnim asset.
	*/
	UPROPERTY(EditAnywhere, Category = "Animation")
	uint32 bRandomAnimationSegment : 1;

	/************************************************************
	* Transient stuff.
	************************************************************/

	/** Overall intensity scale for this shake instance. */
	UPROPERTY(transient, BlueprintReadOnly, Category = "Shake")
	float CurrentScale;

	/** Time remaining for oscillation shakes. Less than 0.f means shake infinitely. */
	UPROPERTY(transient, BlueprintReadOnly, Category = "Oscillation")
	float OscillatorTimeRemaining;

	/************************************************************
	* Blueprint functions.
	************************************************************/

	/** Called every tick to let the shake modify the point of view */
	UFUNCTION(BlueprintImplementableEvent, Category = "Shake")
	void ReceiveTick(float DeltaTime, float Alpha, FTransform& ShakeTransform);

	/** Called when the shake starts playing */
	UFUNCTION(BlueprintImplementableEvent, Category = "Shake")
	void ReceivePlay(float Scale);

	/** Called to allow a shake to decide when it's finished playing. */
	UFUNCTION(BlueprintNativeEvent, Category="Shake")
	bool ReceiveIsFinished() const;

	/**
	* Called when the shake is explicitly stopped.
	* @param Immediately		If true, shake stops right away regardless of blend out settings. If false, shake may blend out according to its settings.
	*/
	UFUNCTION(BlueprintImplementableEvent, Category = "Shake")
	void ReceiveStop(bool bImmediately);

	/************************************************************
	* Functions.
	************************************************************/

	virtual void Play(UShakerComponent* ShakerComponent, float Scale);

	virtual void Tick(float DeltaTime, float Alpha, FTransform& ShakeTransform);

	virtual bool IsFinished() const;

	virtual void Stop(bool Immediately = true);

	bool IsLooping() const;

protected:

	UPROPERTY(transient, BlueprintReadOnly, Category = "Shake")
	UShakerComponent* Owner;

	/** True if this shake is currently blending in. */
	uint16 bBlendingIn : 1;

	/** True if this shake is currently blending out. */
	uint16 bBlendingOut : 1;

	/** How long this instance has been blending in. */
	float CurrentBlendInTime;

	/** How long this instance has been blending out. */
	float CurrentBlendOutTime;

	/** Current location sinusoidal offset. */
	FVector LocSinOffset;

	/** Current rotational sinusoidal offset. */
	FVector RotSinOffset;

	/** Initial offset (could have been assigned at random). */
	FVector InitialLocSinOffset;

	/** Initial offset (could have been assigned at random). */
	FVector InitialRotSinOffset;

	/** Temp actor to use for playing camera animations. Used when playing an animation in non-gameplay context, e.g. in the editor */
	AActor* TempActorForAnimations;
};
