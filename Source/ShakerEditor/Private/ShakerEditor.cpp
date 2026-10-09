// Copyright (c) 2026 Douglas Lassance. All rights reserved.

#include "ShakerEditor.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"

#define LOCTEXT_NAMESPACE "FShakerEditorModule"

uint32 FShakerEditorModule::AssetCategory = 0;

void FShakerEditorModule::StartupModule()
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	AssetCategory = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("Shaker")), LOCTEXT("ShakerAssetCategory", "Shaker"));
}

void FShakerEditorModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FShakerEditorModule, ShakerEditor)
