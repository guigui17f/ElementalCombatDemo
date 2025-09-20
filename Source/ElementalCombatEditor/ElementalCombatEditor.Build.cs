// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ElementalCombatEditor : ModuleRules
{
	public ElementalCombatEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"ElementalCombat",
			"UnrealEd",
			"EditorSubsystem",
			"Slate",
			"SlateCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
			"InputCore",
			"AssetTools",
			"ToolMenus",
			"WorkspaceMenuStructure",
			"ContentBrowser",
			"AssetRegistry",
			"LevelEditor"
		});

		PublicIncludePaths.AddRange(new string[] {
			"ElementalCombatEditor/Public",
			"ElementalCombatEditor/Public/ISMMerge",
			"ElementalCombatEditor/Public/Menus"
		});

		PrivateIncludePaths.AddRange(new string[] {
			"ElementalCombatEditor/Private",
			"ElementalCombatEditor/Private/ISMMerge",
			"ElementalCombatEditor/Private/Menus"
		});
	}
}