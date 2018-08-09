// Copyright (c) 2018 Douglas Lassance. All rights reserved.

#pragma once

#include "Shaker.h"
#include "Components/SceneComponent.h"
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
};
