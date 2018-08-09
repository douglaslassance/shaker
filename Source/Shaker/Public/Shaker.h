// Copyright (c) 2018 Douglas Lassance. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "ModuleManager.h"

class FShakerModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};