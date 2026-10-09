// Copyright (c) 2026 Douglas Lassance. All rights reserved.

using UnrealBuildTool;

public class ShakerEditor : ModuleRules
{
	public ShakerEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
			}
			);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"UnrealEd",
				"AssetTools",
				"Shaker",
			}
			);
	}
}
