// Fill out your copyright notice in the Description page of Project Settings.

#include "Shaker.h"
#include "ShakerAnimation.h"
#include "Serialization/ArchiveCountMem.h"
#include "Matinee/MatineeActor.h"
#include "Matinee/InterpData.h"
#include "Matinee/InterpGroupInst.h"
#include "Matinee/InterpGroupCamera.h"
#include "Matinee/InterpTrackMove.h"

//DEFINE_LOG_CATEGORY(LogShaker);

UShakerAnimation::UShakerAnimation(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AnimationLength = 3.0f;
	bRelativeToInitialTransform = true;
}


bool UShakerAnimation::CreateFromInterpolationGroup(class UInterpGroup* SrcGroup, class AMatineeActor* InMatineeActor)
{
	// assert we're controlling a camera actor
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	{
		UInterpGroupInst* GroupInst = InMatineeActor ? InMatineeActor->FindFirstGroupInst(SrcGroup) : NULL;
		if (GroupInst)
		{
			check(GroupInst->GetGroupActor()->IsA(AActor::StaticClass()));
		}
	}
#endif

	// copy length information
	AnimationLength = (InMatineeActor && InMatineeActor->MatineeData) ? InMatineeActor->MatineeData->InterpLength : 0.f;

	UInterpGroup* OldGroup = InterpolationGroup;

	if (InterpolationGroup != SrcGroup)
	{
		// Copy the source interpolation group for use in the animation.
		// TODO: Fixed this potentially creating an object of UInterpGroup and raw casting it to InterpGroupCamera.  No source data in UE4 to test though.
		InterpolationGroup = Cast<UInterpGroupCamera>(StaticDuplicateObject(SrcGroup, this, NAME_None, RF_AllFlags, UInterpGroupCamera::StaticClass()));

		if (InterpolationGroup)
		{
			// Delete the old one, if it exists.
			if (OldGroup)
			{
				OldGroup->MarkPendingKill();
			}

			// Success!
			return true;
		}
		else
		{
			// Creation of new one failed somehow, restore the old one.
			InterpolationGroup = OldGroup;
		}
	}
	else
	{
		// No need to perform work above, but still a "success" case.
		return true;
	}

	// Failed creation.
	return false;
}

FBox UShakerAnimation::GetAABB(FVector const& BaseLoc, FRotator const& BaseRot, float Scale) const
{
	FRotationTranslationMatrix const BaseTM(BaseRot, BaseLoc);

	FBox ScaledLocalBox = BoundingBox;
	ScaledLocalBox.Min *= Scale;
	ScaledLocalBox.Max *= Scale;

	return ScaledLocalBox.TransformBy(BaseTM);
}

void UShakerAnimation::PreSave(const class ITargetPlatform* TargetPlatform)
{
#if WITH_EDITORONLY_DATA
	CalculateLocalAABB();
#endif // WITH_EDITORONLY_DATA
	Super::PreSave(TargetPlatform);
}

void UShakerAnimation::CalculateLocalAABB()
{
	BoundingBox.Init();

	if (InterpolationGroup)
	{
		// Find move track.
		UInterpTrackMove *MoveTrack = NULL;
		for (int32 TrackIdx = 0; TrackIdx < InterpolationGroup->InterpTracks.Num(); ++TrackIdx)
		{
			MoveTrack = Cast<UInterpTrackMove>(InterpolationGroup->InterpTracks[TrackIdx]);
			if (MoveTrack != NULL)
			{
				break;
			}
		}

		if (MoveTrack != NULL)
		{
			FVector Zero(0.0f);
			FVector MinBounds(0.0f);
			FVector MaxBounds(0.0f);
			if (bRelativeToInitialTransform)
			{
				if (MoveTrack->PosTrack.Points.Num() > 0 &&
					MoveTrack->EulerTrack.Points.Num() > 0)
				{
					const FRotator InitialRotation = FRotator::MakeFromEuler(MoveTrack->EulerTrack.Points[0].OutVal);
					const FVector InitialLocation = MoveTrack->PosTrack.Points[0].OutVal;
					const FTransform MoveTrackInitialTransform(InitialRotation, InitialLocation);
					const FTransform MoveTrackInitialTransformInverse = MoveTrackInitialTransform.Inverse();

					// start at Index = 1 as the initial position will be transformed back to (0,0,0)
					const int32 MoveTrackNum = MoveTrack->PosTrack.Points.Num();
					for (int32 Index = 1; Index < MoveTrackNum; Index++)
					{
						const FVector CurrentPositionRelativeToInitial = MoveTrackInitialTransformInverse.TransformPosition(MoveTrack->PosTrack.Points[Index].OutVal);

						MinBounds = CurrentPositionRelativeToInitial.ComponentMin(MinBounds);
						MaxBounds = CurrentPositionRelativeToInitial.ComponentMax(MaxBounds);
					}
				}
			}
			else
			{
				MoveTrack->PosTrack.CalcBounds(MinBounds, MaxBounds, Zero);
			}
			BoundingBox = FBox(MinBounds, MaxBounds);
		}
	}
}

void UShakerAnimation::PostLoad()
{
	if (GIsEditor)
	{
		// Update existing shaker animations bounding boxes on load, so editor knows they need to be re-saved.
		if (!BoundingBox.IsValid)
		{
			CalculateLocalAABB();
			if (BoundingBox.IsValid)
			{
				MarkPackageDirty();
			}
		}
	}

	Super::PostLoad();
}

void UShakerAnimation::GetResourceSizeEx(FResourceSizeEx& CumulativeResourceSize)
{
	Super::GetResourceSizeEx(CumulativeResourceSize);

	if (CumulativeResourceSize.GetResourceSizeMode() == EResourceSizeMode::Inclusive && InterpolationGroup)
	{
		// find move track
		UInterpTrackMove *MoveTrack = NULL;
		for (int32 TrackIdx = 0; TrackIdx < InterpolationGroup->InterpTracks.Num(); ++TrackIdx)
		{
			// somehow movement track's not calculated when you just used serialize, so I'm adding it here. 
			MoveTrack = Cast<UInterpTrackMove>(InterpolationGroup->InterpTracks[TrackIdx]);
			if (MoveTrack)
			{
				FArchiveCountMem CountBytesSize(MoveTrack);
				CumulativeResourceSize.AddDedicatedSystemMemoryBytes(CountBytesSize.GetNum());
			}
		}
	}
}