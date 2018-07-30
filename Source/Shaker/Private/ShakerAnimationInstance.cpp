// Fill out your copyright notice in the Description page of Project Settings.

#include "Shaker.h"
#include "ShakerAnimationInstance.h"
#include "Camera/CameraAnim.h"
#include "Matinee/InterpTrackMove.h"
#include "Matinee/InterpGroup.h"
#include "Matinee/InterpGroupInst.h"
#include "Matinee/InterpTrackInstMove.h"
#include "Matinee/InterpTrackFloatProp.h"

UShakerAnimationInstance::UShakerAnimationInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bFinished = true;
	bStopAutomatically = true;
	PlayRate = 1.0f;
	TransientScaleModifier = 1.0f;
	InterpGroupInst = CreateDefaultSubobject<UInterpGroupInst>(TEXT("InterpGroupInst0"));
}

void UShakerAnimationInstance::AdvanceAnim(float DeltaTime, bool bJump)
{
	// check to see if our animation has been deleted.  not a fan of 
	// polling for this, but we want to stop this immediately, not when
	// GC gets around to cleaning up.
	if ((AnimationClass != NULL) && !bFinished)
	{
		// will set to true if animation finishes this frame
		bool bAnimJustFinished = false;

		float const ScaledDeltaTime = DeltaTime * PlayRate;

		// find new times
		CurrentTime += ScaledDeltaTime;
		if (bBlendingIn)
		{
			CurBlendInTime += DeltaTime;
		}
		if (bBlendingOut)
		{
			CurBlendOutTime += DeltaTime;
		}

		// see if we've crossed any important time thresholds and deal appropriately
		if (bLooping)
		{
			if (CurrentTime > AnimationClass->AnimationLength)
			{
				// loop back to the beginning
				CurrentTime -= AnimationClass->AnimationLength;
			}
		}
		else
		{
			if (CurrentTime > AnimationClass->AnimationLength)
			{
				// done!!
				bAnimJustFinished = true;
			}
			else if (CurrentTime > (AnimationClass->AnimationLength - BlendOutTime))
			{
				// start blending out
				bBlendingOut = true;
				CurBlendOutTime = CurrentTime - (AnimationClass->AnimationLength - BlendOutTime);
			}
		}

		if (bBlendingIn)
		{
			if ((CurBlendInTime > BlendInTime) || (BlendInTime == 0.f))
			{
				// done blending in!
				bBlendingIn = false;
			}
		}
		if (bBlendingOut)
		{
			if (CurBlendOutTime > BlendOutTime)
			{
				// done!!
				CurBlendOutTime = BlendOutTime;
				bAnimJustFinished = true;
			}
		}

		// calculate blend weight. calculating separately and taking the minimum handles overlapping blends nicely.
		{
			float const BlendInWeight = (bBlendingIn) ? (CurBlendInTime / BlendInTime) : 1.f;
			float const BlendOutWeight = (bBlendingOut) ? (1.f - CurBlendOutTime / BlendOutTime) : 1.f;
			CurrentBlendWeight = FMath::Min(BlendInWeight, BlendOutWeight) * BasePlayScale * TransientScaleModifier;

			// this is intended to be applied only once
			TransientScaleModifier = 1.f;
		}

		// this will update tracks and apply the effects to the group actor (except move tracks)
		if (InterpGroupInst->Group)
		{
			InterpGroupInst->Group->UpdateGroup(CurrentTime, InterpGroupInst, false, bJump);
		}

		if (bStopAutomatically)
		{
			if (bAnimJustFinished)
			{
				// completely finished
				Stop(true);
			}
			else if (RemainingTime > 0.f)
			{
				// handle any specified duration
				RemainingTime -= DeltaTime;
				if (RemainingTime <= 0.f)
				{
					// stop with blend out
					Stop();
				}
			}
		}
	}
}

void UShakerAnimationInstance::SetCurrentTime(float NewTime)
{
	float const TimeDelta = NewTime - (CurrentTime / PlayRate);
	AdvanceAnim(TimeDelta, true);
}


void UShakerAnimationInstance::Update(float NewRate, float NewScale, float NewBlendInTime, float NewBlendOutTime, float NewDuration)
{
	if (bFinished == false)
	{
		if (bBlendingOut)
		{
			bBlendingOut = false;
			CurBlendOutTime = 0.f;

			// stop any blend out and reverse it to a blend in.
			bBlendingIn = true;
			CurBlendInTime = NewBlendInTime * (1.f - CurBlendOutTime / BlendOutTime);
		}

		PlayRate = NewRate;
		BasePlayScale = NewScale;
		BlendInTime = NewBlendInTime;
		BlendOutTime = NewBlendOutTime;
		RemainingTime = (NewDuration > 0.f) ? (NewDuration - BlendOutTime) : 0.f;
	}
}

void UShakerAnimationInstance::SetDuration(float NewDuration)
{
	if (bFinished == false)
	{
		// setting a new duration will reset the play timer back to 0 but maintain current playback position
		// if blending out, stop it and blend back in to reverse it so it's smooth
		if (bBlendingOut)
		{
			bBlendingOut = false;
			CurBlendOutTime = 0.f;

			// stop any blend out and reverse it to a blend in.
			bBlendingIn = true;
			CurBlendInTime = BlendInTime * (1.f - CurBlendOutTime / BlendOutTime);
		}

		RemainingTime = (NewDuration > 0.f) ? (NewDuration - BlendOutTime) : 0.f;
	}
	else
	{
		// UE_LOG(LogCameraAnim, Warning, TEXT("SetDuration called for CameraAnim %s after it finished. Ignored."), *GetNameSafe(Animation));
	}
}

void UShakerAnimationInstance::SetScale(float NewScale)
{
	BasePlayScale = NewScale;
}
static const FName NAME_CameraComponentFieldOfViewPropertyName(TEXT("CameraComponent.FieldOfView"));

void UShakerAnimationInstance::Play(UShakerAnimation* Animation, class AActor* Actor, float InRate, float InScale, float InBlendInTime, float InBlendOutTime, bool bInLooping, bool bRandomStartTime, float Duration)
{
	check(IsInGameThread());

	if (Animation && Animation->InterpolationGroup)
	{
		// make sure any previous animation has been terminated correctly
		Stop(true);

		CurrentTime = bRandomStartTime ? (FMath::FRand() * Animation->AnimationLength) : 0.f;
		CurBlendInTime = 0.f;
		CurBlendOutTime = 0.f;
		bBlendingIn = true;
		bBlendingOut = false;
		bFinished = false;

		// copy properties
		AnimationClass = Animation;
		PlayRate = InRate;
		BasePlayScale = InScale;
		BlendInTime = InBlendInTime;
		BlendOutTime = InBlendOutTime;
		bLooping = bInLooping;
		RemainingTime = (Duration > 0.f) ? (Duration - BlendOutTime) : 0.f;

		// init the interpolation group.
		if (Actor && Actor->IsA(AActor::StaticClass()))
		{
			// #fixme jf: I don't think this is necessary anymore
			// ensure CameraActor is zeroed, so RelativeToInitial anims get proper InitialTM
			Actor->SetActorLocation(FVector::ZeroVector, false);
			Actor->SetActorRotation(FRotator::ZeroRotator);
		}

		InterpGroupInst->InitGroupInst(AnimationClass->InterpolationGroup, Actor);

		// cache move track refs
		for (int32 Idx = 0; Idx < InterpGroupInst->TrackInst.Num(); ++Idx)
		{
			MoveTrack = Cast<UInterpTrackMove>(AnimationClass->InterpolationGroup->InterpTracks[Idx]);
			if (MoveTrack != NULL)
			{
				MoveInstance = CastChecked<UInterpTrackInstMove>(InterpGroupInst->TrackInst[Idx]);
				// only 1 move track per group, so we can bail here
				break;
			}
		}

		// store initial transform so we can treat camera movements as offsets relative to the initial anim key
		if (MoveTrack && MoveInstance)
		{
			FRotator OutRot;
			FVector OutLoc;
			MoveTrack->GetKeyTransformAtTime(MoveInstance, 0.f, OutLoc, OutRot);

			// @todo, store inverted since that's how we use it?
			InitialCamToWorld = FTransform(OutRot, OutLoc);		
		}
	}
}

void UShakerAnimationInstance::Stop(bool bImmediate)
{
	check(IsInGameThread());

	if (bImmediate || (BlendOutTime <= 0.f))
	{
		if (InterpGroupInst->Group != nullptr)
		{
			InterpGroupInst->TermGroupInst(true);
			InterpGroupInst->Group = nullptr;
		}
		MoveTrack = nullptr;
		MoveInstance = nullptr;
		bFinished = true;
	}
	else if (bBlendingOut == false)
	{
		// start blend out if not already blending out
		bBlendingOut = true;
		CurBlendOutTime = 0.f;
	}
}

void UShakerAnimationInstance::ApplyTransientScaling(float Scalar)
{
	TransientScaleModifier *= Scalar;
}

void UShakerAnimationInstance::SetPlaySpace(ECameraAnimPlaySpace::Type NewSpace, FRotator UserPlaySpace)
{
	PlaySpace = NewSpace;
	UserPlaySpaceMatrix = (PlaySpace == ECameraAnimPlaySpace::UserDefined) ? FRotationMatrix(UserPlaySpace) : FMatrix::Identity;
}


/** Returns InterpGroupInst sub-object **/
UInterpGroupInst* UShakerAnimationInstance::GetInterpGroupInst() const
{
	return InterpGroupInst;
}

void UShakerAnimationInstance::ApplyToView(FMinimalViewInfo& InOutPOV) const
{
	if (CurrentBlendWeight > 0.f)
	{
		AActor const* AnimatedActor = dynamic_cast<AActor*>(InterpGroupInst->GetGroupActor());
		if (AnimatedActor)
		{

			if (AnimationClass->bRelativeToInitialTransform)
			{
				// move animated cam actor to initial-relative position
				FTransform const AnimatedCamToWorld = AnimatedActor->GetTransform();
				FTransform const AnimatedCamToInitialCam = AnimatedCamToWorld * InitialCamToWorld.Inverse();
				AActor* const MutableActor = const_cast<AActor*>(AnimatedActor);
				MutableActor->SetActorTransform(AnimatedCamToInitialCam);
			}

			float const Scale = CurrentBlendWeight;
			FRotationMatrix const CameraToWorld(InOutPOV.Rotation);

			if (PlaySpace == ECameraAnimPlaySpace::CameraLocal)
			{
				// the code in the else block will handle this just fine, but this path provides efficiency and simplicity for the most common case

				// loc
				FVector const LocalOffset = CameraToWorld.TransformVector(AnimatedActor->GetActorLocation()*Scale);
				InOutPOV.Location += LocalOffset;

				// rot
				FRotationMatrix const AnimRotMat(AnimatedActor->GetActorRotation()*Scale);
				InOutPOV.Rotation = (AnimRotMat * CameraToWorld).Rotator();
			}
			else
			{
				// handle playing the animation in an arbitrary space relative to the camera

				// find desired space
				FMatrix const PlaySpaceToWorld = (PlaySpace == ECameraAnimPlaySpace::UserDefined) ? UserPlaySpaceMatrix : FMatrix::Identity;

				// loc
				FVector const LocalOffset = PlaySpaceToWorld.TransformVector(AnimatedActor->GetActorLocation()*Scale);
				InOutPOV.Location += LocalOffset;

				// rot
				// find transform from camera to the "play space"
				FMatrix const CameraToPlaySpace = CameraToWorld * PlaySpaceToWorld.Inverse();	// CameraToWorld * WorldToPlaySpace

																								// find transform from anim (applied in playspace) back to camera
				FRotationMatrix const AnimToPlaySpace(AnimatedActor->GetActorRotation()*Scale);
				FMatrix const AnimToCamera = AnimToPlaySpace * CameraToPlaySpace.Inverse();			// AnimToPlaySpace * PlaySpaceToCamera

																									// RCS = rotated camera space, meaning camera space after it's been animated
																									// this is what we're looking for, the diff between rotated cam space and regular cam space.
																									// apply the transform back to camera space from the post-animated transform to get the RCS
				FMatrix const RCSToCamera = CameraToPlaySpace * AnimToCamera;

				// now apply to real camera
				FRotationMatrix const RealCamToWorld(InOutPOV.Rotation);
				InOutPOV.Rotation = (RCSToCamera * RealCamToWorld).Rotator();
			}
		}
	}
}

