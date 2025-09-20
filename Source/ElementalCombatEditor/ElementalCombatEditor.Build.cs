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
			"SlateCore",
			"ToolMenus",
			"EditorWidgets",
			"EditorStyle",
			"PropertyEditor",
			"WorkspaceMenuStructure",
			"ContentBrowser",
			"AssetRegistry",
			"LevelEditor"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
			"InputCore",
			"DesktopPlatform",
			"AssetTools",
			"BlueprintGraph",
			"KismetCompiler",
			"SequencerWidgets",
			"DeveloperSettings"
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