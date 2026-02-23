// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class WwiseGameplayCues : ModuleRules
{
	public WwiseGameplayCues(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange( new []
		{
			"CoreUObject",
			"Engine",
			"GameplayTags",
		});

		PublicDependencyModuleNames.AddRange(new []
		{
			"Core",
			"GameplayAbilities",
			"Wwise",
			"AkAudio",
		});
	}
}
