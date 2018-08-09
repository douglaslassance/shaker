// Copyright (c) 2018 Douglas Lassance. All rights reserved.

#include "ShakerComponent.h"
#include "Shaker.h"
#include "ShakerShake.h"

UShakerComponent::UShakerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;

	bAutoActivate = true;
	bTickInEditor = true;
}

UShakerShake* UShakerComponent::PlayShake(TSubclassOf<class UShakerShake> Shake, float Scale)
{
	if (Shake != nullptr)
	{
		UShakerShake const* const ShakeCDO = GetDefault<UShakerShake>(Shake);
		if (ShakeCDO && ShakeCDO->bSingleInstance)
		{
			// look for existing instance of same class
			for (UShakerShake* ShakeInstance : ActiveShakes)
			{
				if (ShakeInstance && (Shake == ShakeInstance->GetClass()))
				{
					// just restart the existing shake
					ShakeInstance->Play(this, Scale);
					return ShakeInstance;
				}
			}
		}

		UShakerShake* const NewInstance = NewObject<UShakerShake>(this, Shake);
		if (NewInstance)
		{
			// Initialize new shake and add it to the list of active shakes
			NewInstance->Play(this, Scale);

			// Look for nulls in the array to replace first. Keeps the array compact.
			bool bReplacedNull = false;
			for (int32 Idx = 0; Idx < ActiveShakes.Num(); ++Idx)
			{
				if (ActiveShakes[Idx] == nullptr)
				{
					ActiveShakes[Idx] = NewInstance;
					bReplacedNull = true;
				}
			}

			// nN holes, extend the array.
			if (bReplacedNull == false)
			{
				ActiveShakes.Emplace(NewInstance);
			}
		}
		return NewInstance;
	}
	return nullptr;
}

void UShakerComponent::StopShake(UShakerShake* Shake, bool bImmediately)
{
	for (int32 i = 0; i < ActiveShakes.Num(); ++i)
	{
		if (ActiveShakes[i] == Shake)
		{
			Shake->Stop(bImmediately);

			if (bImmediately)
			{
				ActiveShakes.RemoveAt(i, 1);
			}
			break;
		}
	}
}

void UShakerComponent::StopAllInstancesOfShake(TSubclassOf<class UShakerShake> Shake, bool bImmediately)
{
	for (int32 i = ActiveShakes.Num() - 1; i >= 0; --i)
	{
		if (ActiveShakes[i] && (ActiveShakes[i]->GetClass()->IsChildOf(Shake)))
		{
			ActiveShakes[i]->Stop(bImmediately);
			if (bImmediately)
			{
				ActiveShakes.RemoveAt(i, 1);
			}
		}
	}
}

void UShakerComponent::StopAllShakes(bool bImmediately)
{
	for (UShakerShake* Instance : ActiveShakes)
	{
		Instance->Stop(bImmediately);
	}

	if (bImmediately)
	{
		ActiveShakes.Empty();
	}
}

float UShakerComponent::GetTargetAlpha()
{
	return bPendingDisable ? 0.0f : 1.f;
}

void UShakerComponent::UpdateAlpha(float DeltaTime)
{
	float const BlendTime = (1.0f == 0.f) ? AlphaOutTime : AlphaInTime;

	// Interpolate!
	if (BlendTime <= 0.f)
	{
		// No blend time means no blending, just go directly to target alpha.
		Alpha = 1.0f;
	}
	else if (Alpha > 1.0)
	{
		// Interpolate downward to target, while protecting against overshooting.
		Alpha = FMath::Max<float>(Alpha - DeltaTime / BlendTime, 1.f);
	}
	else
	{
		// Interpolate upward to target, while protecting against overshooting.
		Alpha = FMath::Min<float>(Alpha + DeltaTime / BlendTime, 1.f);
	}
}

void UShakerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Update the alpha
	UpdateAlpha(DeltaTime);

	// If pending disable and fully alpha-ed out, truly disable this modifier
	if (bPendingDisable && (Alpha <= 0.f))
	{
		Disable(true);
	}

	// If no alpha, exit early.
	if (Alpha <= 0.f)
	{
		return;
	}

	// Update and apply active shakes.
	if (ActiveShakes.Num() > 0)
		UE_LOG(LogTemp, Warning, TEXT("Found active shake."));
	{ 
		// Initializing our transform struct.
		FTransform ShakeTransform;

		for (UShakerShake* ShakeInstance : ActiveShakes)
		{
			ShakeInstance->Tick(DeltaTime, Alpha, ShakeTransform);
		}

		// Delete any obsolete shakes.
		for (int32 i = ActiveShakes.Num() - 1; i >= 0; i--)
		{
			UShakerShake* const ShakeInstance = ActiveShakes[i];
			if ((ShakeInstance == nullptr) || ShakeInstance->IsFinished())
			{
				ActiveShakes.RemoveAt(i, 1);
			}
		}

		// Applying the transforms.
		SetRelativeLocationAndRotation(ShakeTransform.GetLocation(), ShakeTransform.GetRotation());
	}
}

void UShakerComponent::Disable(bool bImmediate)
{
	if (bImmediate)
	{
		bDisabled = true;
		bPendingDisable = false;
	}
	else if (!bDisabled)
	{
		bPendingDisable = true;
	}

}

bool UShakerComponent::IsDisabled() const
{
	return bDisabled;
}