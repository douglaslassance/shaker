// Copyright (c) 2017 Douglas Lassance. All rights reserved.

#include "Shaker.h"
#include "ShakerShake.h"

UShakerShake::UShakerShake()
{
	AnimationPlayRate = 1.0f;
	AnimationScale = 1.0f;
	AnimationBlendInTime = 0.2f;
	AnimationBlendOutTime = 0.2f;
	OscillationBlendInTime = 0.1f;
	OscillationBlendOutTime = 0.2f;
}

void UShakerShake::Stop(bool bImmediately)
{
	if (bImmediately)
	{
		// Stop cam animation if playing.
		if (AnimationInstance && !AnimationInstance->bFinished)
		{
			if (Owner)
			{
				Owner->StopAnimationInstance(AnimationInstance, true);
			}
			else
			{
				AnimationInstance->Stop(true);
			}
		}

		AnimationInstance = nullptr;

		// Stop oscillation.
		OscillatorTimeRemaining = 0.f;
	}
	else
	{
		// advance to the blend out time
		OscillatorTimeRemaining = FMath::Min(OscillatorTimeRemaining, OscillationBlendOutTime);

		if (AnimationInstance && !AnimationInstance->bFinished)
		{
			if (Owner)
			{
				Owner->StopAnimationInstance(AnimationInstance, false);
			}
			else
			{
				// Stop it ourselves if no owner.
				AnimationInstance->Stop(false);
			}
		}
	}
	ReceiveStop(bImmediately);
}

void UShakerShake::Play(UShakerComponent* Shaker, float Scale)
{
	CurrentScale = Scale;

	UE_LOG(LogTemp, Warning, TEXT("Playing shake."));

	// Initializing oscillations.
	if (OscillationDuration != 0.f)
	{

		if (OscillatorTimeRemaining > 0.f)
		{
			// This shake was already playing.
			OscillatorTimeRemaining = OscillationDuration;

			if (bBlendingOut)
			{
				bBlendingOut = false;
				CurrentBlendOutTime = 0.f;

				// Stop any blend out and reverse it to a blend in.
				if (OscillationBlendInTime > 0.f)
				{
					bBlendingIn = true;
					CurrentBlendInTime = OscillationBlendInTime * (1.f - CurrentBlendOutTime / OscillationBlendOutTime);
				}
				else
				{
					bBlendingIn = false;
					CurrentBlendInTime = 0.f;
				}
			}
		}
		else
		{
			RotSinOffset.X = FFOscillator::GetInitialOffset(RotationalOscillation.Pitch);
			RotSinOffset.Y = FFOscillator::GetInitialOffset(RotationalOscillation.Yaw);
			RotSinOffset.Z = FFOscillator::GetInitialOffset(RotationalOscillation.Roll);

			LocSinOffset.X = FFOscillator::GetInitialOffset(PositionalOscillation.X);
			LocSinOffset.Y = FFOscillator::GetInitialOffset(PositionalOscillation.Y);
			LocSinOffset.Z = FFOscillator::GetInitialOffset(PositionalOscillation.Z);

			InitialLocSinOffset = LocSinOffset;
			InitialRotSinOffset = RotSinOffset;

			OscillatorTimeRemaining = OscillationDuration;

			if (OscillationBlendInTime > 0.f)
			{
				bBlendingIn = true;
				CurrentBlendInTime = 0.f;
			}
		}
	}

	// Initializing animation shakes.
	if (Animation != nullptr)
	{
		if (AnimationInstance)
		{
			float const Duration = bRandomAnimationSegment ? RandomAnimationSegmentDuration : 0.f;
			float const FinalAnimScale = Scale * AnimationScale;
			AnimationInstance->Update(AnimationPlayRate, FinalAnimScale, AnimationBlendInTime, AnimationBlendOutTime, Duration);
		}
		else
		{
			bool bLoop = false;
			bool bRandomStart = false;
			float Duration = 0.f;
			if (bRandomAnimationSegment)
			{
				bLoop = true;
				bRandomStart = true;
				Duration = RandomAnimationSegmentDuration;
			}

			float const FinalAnimScale = Scale * AnimationScale;
			if (FinalAnimScale > 0.f)
			{
				if (Shaker)
				{
					AnimationInstance = Shaker->PlayAnimation(Animation, AnimationPlayRate, FinalAnimScale, AnimationBlendInTime, AnimationBlendOutTime, bLoop, bRandomStart, Duration);
				}
				else
				{
					// Allocate our own instance and start it.
					AnimationInstance = NewObject<UShakerAnimationInstance>(this);
					if (AnimationInstance)
					{
						// NOTE: we don't have a temp camera actor necessary for evaluating a camera animation.
						// Caller is responsible in this case for providing one by calling SetTempCameraAnimActor() on the shake instance before playing the shake.
						AnimationInstance->Play(Animation, TempActorForAnimations, AnimationPlayRate, FinalAnimScale, AnimationBlendInTime, AnimationBlendOutTime, bLoop, bRandomStart, Duration);
					}
				}
			}
		}
	}
	ReceivePlay(Scale);
}

void UShakerShake::Tick(float DeltaTime, float Alpha, FTransform& ShakeTransform)
{
	UE_LOG(LogTemp, Warning, TEXT("Ticking shake."));

	// this is the base scale for the whole shake, animation and oscillation alike
	float const BaseShakeScale = FMath::Max<float>(Alpha * CurrentScale, 0.0f);

	// update animations with any desired scaling
	if (AnimationInstance)
	{
		AnimationInstance->TransientScaleModifier *= BaseShakeScale;
	}

	// update oscillation times
	if (OscillatorTimeRemaining > 0.f)
	{
		OscillatorTimeRemaining -= DeltaTime;
		OscillatorTimeRemaining = FMath::Max(0.f, OscillatorTimeRemaining);
	}
	if (bBlendingIn)
	{
		CurrentBlendInTime += DeltaTime;
	}
	if (bBlendingOut)
	{
		CurrentBlendOutTime += DeltaTime;
	}

	// see if we've crossed any important time thresholds and deal appropriately
	bool bOscillationFinished = false;

	if (OscillatorTimeRemaining == 0.f)
	{
		// finished!
		bOscillationFinished = true;
	}
	else if (OscillatorTimeRemaining < 0.0f)
	{
		// indefinite shaking
	}
	else if (OscillatorTimeRemaining < OscillationBlendOutTime)
	{
		// start blending out
		bBlendingOut = true;
		CurrentBlendOutTime = OscillationBlendOutTime - OscillatorTimeRemaining;
	}

	if (bBlendingIn)
	{
		if (CurrentBlendInTime > OscillationBlendInTime)
		{
			// done blending in!
			bBlendingIn = false;
		}
	}
	if (bBlendingOut)
	{
		if (CurrentBlendOutTime > OscillationBlendOutTime)
		{
			// done!!
			CurrentBlendOutTime = OscillationBlendOutTime;
			bOscillationFinished = true;
		}
	}

	// Do not update oscillation further if finished
	if (bOscillationFinished == false)
	{
		// calculate blend weight. calculating separately and taking the minimum handles overlapping blends nicely.
		float const BlendInWeight = (bBlendingIn) ? (CurrentBlendInTime / OscillationBlendInTime) : 1.f;
		float const BlendOutWeight = (bBlendingOut) ? (1.f - CurrentBlendOutTime / OscillationBlendOutTime) : 1.f;
		float const CurrentBlendWeight = FMath::Min(BlendInWeight, BlendOutWeight);

		// this is the oscillation scale, which includes oscillation fading
		float const OscillationScale = BaseShakeScale * CurrentBlendWeight;

		if (OscillationScale > 0.f)
		{
			// View location offset, compute sin wave value for each component
			FVector	LocOffset = FVector(0);
			LocOffset.X = FFOscillator::UpdateOffset(PositionalOscillation.X, LocSinOffset.X, DeltaTime);
			LocOffset.Y = FFOscillator::UpdateOffset(PositionalOscillation.Y, LocSinOffset.Y, DeltaTime);
			LocOffset.Z = FFOscillator::UpdateOffset(PositionalOscillation.Z, LocSinOffset.Z, DeltaTime);
			LocOffset *= OscillationScale;

			// View rotation offset, compute sin wave value for each component
			FRotator RotOffset;
			RotOffset.Pitch = FFOscillator::UpdateOffset(RotationalOscillation.Pitch, RotSinOffset.X, DeltaTime) * OscillationScale;
			RotOffset.Yaw = FFOscillator::UpdateOffset(RotationalOscillation.Yaw, RotSinOffset.Y, DeltaTime) * OscillationScale;
			RotOffset.Roll = FFOscillator::UpdateOffset(RotationalOscillation.Roll, RotSinOffset.Z, DeltaTime) * OscillationScale;

			// Applying the transforms to input transform struct.
			// We are only using the max value so the shakes are never cumulative.
			ShakeTransform.AddToTranslation(LocOffset);
			ShakeTransform.ConcatenateRotation(RotOffset.Quaternion());
		}
	}

	ReceiveTick(DeltaTime, Alpha, ShakeTransform);
}

bool UShakerShake::IsFinished() const
{
	return (((OscillatorTimeRemaining <= 0.f) && (IsLooping() == false)) &&
		/*((AnimInst == nullptr) || AnimInst->bFinished) &&*/
		ReceiveIsFinished()
	);
}

bool UShakerShake::ReceiveIsFinished_Implementation() const
{
	return true;
}

bool UShakerShake::IsLooping() const
{
	return OscillationDuration < 0.0f;
}