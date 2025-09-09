// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class CatchMe : ModuleRules
{
	public CatchMe(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"GameplayTags"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { 
			"Slate",
			"SlateCore"
		});

		PublicIncludePaths.AddRange(new string[] {
			"CatchMe",
			"CatchMe/Variant_Platforming",
			"CatchMe/Variant_Platforming/Animation",
			"CatchMe/Variant_Combat",
			"CatchMe/Variant_Combat/AI",
			"CatchMe/Variant_Combat/Animation",
			"CatchMe/Variant_Combat/Gameplay",
			"CatchMe/Variant_Combat/Interfaces",
			"CatchMe/Variant_Combat/UI",
			"CatchMe/Variant_SideScrolling",
			"CatchMe/Variant_SideScrolling/AI",
			"CatchMe/Variant_SideScrolling/Gameplay",
			"CatchMe/Variant_SideScrolling/Interfaces",
			"CatchMe/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
