// Copyright (c) 2026 Douglas Lassance. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "ShakerShakeFactory.generated.h"

/** Creates a new Shake Blueprint from the Content Browser's Add menu. */
UCLASS(hidecategories = Object)
class UShakerShakeFactory : public UFactory
{
	GENERATED_BODY()

public:

	UShakerShakeFactory();

	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual FText GetDisplayName() const override;
	virtual FText GetToolTip() const override;
	virtual uint32 GetMenuCategories() const override;
	virtual FString GetDefaultNewAssetName() const override;
};
