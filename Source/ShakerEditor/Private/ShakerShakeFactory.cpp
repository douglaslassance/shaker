// Copyright (c) 2026 Douglas Lassance. All rights reserved.

#include "ShakerShakeFactory.h"
#include "ShakerEditor.h"
#include "ShakerShake.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Kismet2/KismetEditorUtilities.h"

#define LOCTEXT_NAMESPACE "ShakerShakeFactory"

UShakerShakeFactory::UShakerShakeFactory()
{
	SupportedClass = UBlueprint::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UShakerShakeFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UBlueprint* Blueprint = FKismetEditorUtilities::CreateBlueprint(
		UShakerShake::StaticClass(),
		InParent,
		Name,
		BPTYPE_Normal,
		UBlueprint::StaticClass(),
		UBlueprintGeneratedClass::StaticClass(),
		NAME_None);

	// Shakes are tuned through their class defaults. Clearing the newly created flag
	// lets the editor open them in the data-only (Class Defaults) view instead of the
	// full graph editor. Users who need scripting can still opt into the full editor.
	if (Blueprint)
	{
		Blueprint->bIsNewlyCreated = false;
	}

	return Blueprint;
}

FText UShakerShakeFactory::GetDisplayName() const
{
	return LOCTEXT("DisplayName", "Shake");
}

FText UShakerShakeFactory::GetToolTip() const
{
	return LOCTEXT("ToolTip", "A Shake Blueprint, played on a Shaker component with PlayShake.");
}

uint32 UShakerShakeFactory::GetMenuCategories() const
{
	return FShakerEditorModule::GetAssetCategory();
}

FString UShakerShakeFactory::GetDefaultNewAssetName() const
{
	return TEXT("NewShake");
}

#undef LOCTEXT_NAMESPACE
