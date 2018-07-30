// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "Engine/Scene.h"
#include "ShakerAnimation.generated.h"

UCLASS(BlueprintType, notplaceable)
class SHAKER_API UShakerAnimation : public UObject
{
	GENERATED_UCLASS_BODY()

	/** The UInterpGroup that holds our actual interpolation data. */
	UPROPERTY()
	class UInterpGroup* InterpolationGroup;

#if WITH_EDITORONLY_DATA
	/** This is to preview and they only exists in editor */
	UPROPERTY(transient)
	class UInterpGroup* PreviewInterpolationGroup;
#endif // WITH_EDITORONLY_DATA

	/** Length, in seconds. */
	UPROPERTY()
	float AnimationLength;

	/** AABB in local space. */
	UPROPERTY()
	FBox BoundingBox;

	/**
	* If true, assume all transform keys are intended be offsets from the start of the animation. This allows the animation to be authored at any world location and be applied as a delta to the camera.
	* If false, assume all transform keys are authored relative to the world origin. Positions will be directly applied as deltas to the camera.
	*/
	UPROPERTY(EditDefaultsOnly, Category = "Shake Animation")
	uint8 bRelativeToInitialTransform : 1;

	/** Default PP settings to put on the animated camera. For modifying PP without keyframes. */
	UPROPERTY()
	FPostProcessSettings BasePostProcessSettings;

	/** Default PP blend weight to put on the animated camera. For modifying PP without keyframes. */
	UPROPERTY()
	float BasePostProcessBlendWeight;

	//~ Begin UObject Interface
	virtual void PreSave(const class ITargetPlatform* TargetPlatform) override;
	virtual void PostLoad() override;
	virtual void GetResourceSizeEx(FResourceSizeEx& CumulativeResourceSize) override;
	//~ End UObject Interface

	/**
	* Construct a camera animation from an InterpGroup.  The InterpGroup must control a CameraActor.
	* Used by the editor to "export" a camera animation from a normal Matinee scene.
	*/
	bool CreateFromInterpolationGroup(class UInterpGroup* SrcGroup, class AMatineeActor* InMatineeActor);

	/**
	* Gets AABB of the camera's path. Useful for rough testing if you can play an animation at a certain
	* location in the world without penetrating geometry.
	* @return Returns the local-space axis-aligned bounding box of the entire motion of this animation.
	*/
	FBox GetAABB(FVector const& BaseLoc, FRotator const& BaseRot, float Scale) const;

protected:

	/** Internal. Computes and stores the local AABB of the camera's motion. */
	void CalculateLocalAABB();
	
};
