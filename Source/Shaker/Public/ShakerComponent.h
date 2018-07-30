// Copyright (c) 2017 Douglas Lassance. All rights reserved.

#pragma once

#include "Components/SceneComponent.h"
#include "ShakerAnimation.h"
#include "ShakerAnimationInstance.h"
#include "ShakerComponent.generated.h"

class UShakerShake;

UCLASS(ClassGroup=(Transform), meta=(BlueprintSpawnableComponent))
class SHAKER_API UShakerComponent : public USceneComponent
{
	GENERATED_BODY()

public:

	UShakerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UPROPERTY()
	TArray<UShakerShake*> ActiveShakes;

	/** Array of camera animation instances that are currently playing and in-use */
	UPROPERTY(transient)
	TArray<class UShakerAnimationInstance*> ActiveAnimations;

	UFUNCTION(BlueprintCallable, Category = "Actor|Component|Shaker")
	virtual UShakerShake* PlayShake(TSubclassOf<class UShakerShake> Shake, float Scale = 1.f);

	UFUNCTION(BlueprintCallable, Category = "Actor|Component|Shaker")
	virtual void StopShake(UShakerShake* Shake, bool bImmediately = true);

	UFUNCTION(BlueprintCallable, Category = "Actor|Component|Shaker")
	virtual void StopAllShakes(bool Immediately = true);

	UFUNCTION(BlueprintCallable, Category = "Actor|Component|Shaker")
	virtual void StopAllInstancesOfShake(TSubclassOf<UShakerShake> Shake, bool bImmediately = true);

	virtual void UpdateAlpha(float DeltaTime);

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;	

	//
	//  Animation support.
	//

	/**
	* Play the indicated animation on this component owner.
	*
	* @param Anim				The animation that should play on this instance.
	* @param Rate				How fast to play the animation. 1.0 is normal.
	* @param Scale				How "intense" to play the animation. 1.0 is normal.
	* @param BlendInTime		Time to linearly ramp in.
	* @param BlendOutTime		Time to linearly ramp out.
	* @param bLoop				True to loop the animation if it hits the end.
	* @param bRandomStartTime	Whether or not to choose a random time to start playing. Useful with bLoop=true and a duration to randomize things like shakes.
	* @param Duration			Optional total playtime for this animation, including blends. 0 means to use animations natural duration, or infinite if looping.
	* @param PlaySpace			Which space to play the animation in.
	* @param UserPlaySpaceRot   Custom play space, used when PlaySpace is UserDefined.
	* @return The CameraAnim instance, which can be stored to manipulate/stop the animation after the fact.
	*/
	UFUNCTION(BlueprintCallable, Category = "Actor|Component|Shaker")
	virtual class UShakerAnimationInstance* PlayAnimation(class UShakerAnimation* Animation, float Rate = 1.f, float Scale = 1.f, float BlendInTime = 0.f, float BlendOutTime = 0.f, bool bLoop = false, bool bRandomStartTime = false, float Duration = 0.f);

	/**
	* Stop playing all instances of the indicated ShakerAnim.
	* @param bImmediate	True to stop it right now and ignore blend out, false to let it blend out as indicated.
	*/
	UFUNCTION(BlueprintCallable, Category = "Actor|Component|Shaker")
	virtual void StopAllAnimationInstances(UShakerAnimation* Animation, bool bImmediate = false);

	/**
	* Stops the given ShakerAnimInst from playing.  The given pointer should be considered invalid after this.
	* @param bImmediate	True to stop it right now and ignore blend out, false to let it blend out as indicated.
	*/
	UFUNCTION(BlueprintCallable, Category = "Actor|Component|Shaker")
	virtual void StopAnimationInstance(UShakerAnimationInstance* Animation, bool bImmediate = false);

	/**
	* Stop playing all ShakerAnims on this component owner.
	* @param bImmediate	True to stop it right now and ignore blend out, false to let it blend out as indicated.
	*/
	UFUNCTION(BlueprintCallable, Category = "Actor|Component|Shaker")
	virtual void StopAllAnimations(bool bImmediate = false);

	/** @return Returns first existing instance of the specified shaker animation, ReleaseAnimationInstanceor NULL if none exists. */
	UShakerAnimationInstance* FindAnimationInstance(UShakerAnimation const* Animation) const;

	/** Returns active post process info. */
	void GetCachedPostProcessBlends(TArray<struct FPostProcessSettings> const*& OutPPSettings, TArray<float> const*& OutBlendWeigthts) const;

	/**
	*  Disables the component.
	*  @param  bImmediate  - true to disable with no blend out, false (default) to allow blend out
	*/
	UFUNCTION(BlueprintCallable, Category = "Actor|Component|Shaker")
	virtual void Disable(bool bImmediate = false);

	/** @return Returns true if modifier is disabled, false otherwise. */
	UFUNCTION(BlueprintCallable, Category = "Actor|Component|Shaker")
	virtual bool IsDisabled() const;

protected:

	/** If true, do not apply this modifier to the camera. */
	uint32 bDisabled : 1;

	/** If true, this modifier will disable itself when finished interpolating out. */
	uint32 bPendingDisable : 1;

	/** Current blend alpha. */
	UPROPERTY(transient, BlueprintReadOnly, Category = "Actor|Component|Shaker")
	float Alpha;

	/** When blending in, alpha proceeds from 0 to 1 over this time */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Actor|Component|Shaker")
	float AlphaInTime;

	/** When blending out, alpha proceeds from 1 to 0 over this time */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Actor|Component|Shaker")
	float AlphaOutTime;

	/** @return Returns the ideal blend alpha for this modifier. Interpolation will seek this value. */
	virtual float GetTargetAlpha();

	/** @return Returns an available ShakerAnimInst, or NULL if no more are available. */
	UShakerAnimationInstance* AllocateAnimationInstance();

	/**
	* Frees an allocated CameraAnimInst for future use.
	* @param Inst - Instance to free.
	*/
	void ReleaseAnimationInstance(UShakerAnimationInstance* Animation);

	/** Array of animation instances that are not playing and available to be used. */
	UPROPERTY(transient)
	TArray<class UShakerAnimationInstance*> FreeAnimations;

	/** Internal list of active post process effects. Parallel array to PostProcessBlendCacheWeights. */
	UPROPERTY(transient)
	TArray<struct FPostProcessSettings> PostProcessBlendCache;

	/** Internal list of weights for active post process effects. Parallel array to PostProcessBlendCache. */
	TArray<float> PostProcessBlendCacheWeights;
};
