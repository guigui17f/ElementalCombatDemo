// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ElementalCombat : ModuleRules
{
	public ElementalCombat(ReadOnlyTargetRules Target) : base(Target)
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
			"NavigationSystem",
			"Slate",
			"Niagara",
			"NiagaraCore",
			"RenderCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
			"SlateCore"
		});

		PublicIncludePaths.AddRange(new string[] {
			"ElementalCombat/UI",
			"ElementalCombat",
			"ElementalCombat/AI",
			"ElementalCombat/Animation",
			"ElementalCombat/Characters",
			"ElementalCombat/Combat",
			"ElementalCombat/Combat/Projectiles",
			"ElementalCombat/Controllers",
			"ElementalCombat/Variant_Combat",
			"ElementalCombat/Variant_Combat/AI",
			"ElementalCombat/Variant_Combat/Animation",
			"ElementalCombat/Variant_Combat/Gameplay",
			"ElementalCombat/Variant_Combat/Interfaces",
			"ElementalCombat/Variant_Combat/UI"
		});

		// Slate UI is now enabled
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
