// Copyright (c) 2026 Douglas Lassance. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FShakerEditorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	/** Content Browser category that Shaker assets are listed under. */
	static uint32 GetAssetCategory() { return AssetCategory; }

private:

	static uint32 AssetCategory;
};
