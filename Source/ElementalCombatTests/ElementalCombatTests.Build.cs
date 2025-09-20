// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ElementalCombatTests : ModuleRules
{
	public ElementalCombatTests(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		// 仅在非Shipping构建中编译
		if (Target.Configuration != UnrealTargetConfiguration.Shipping)
		{
			PublicDependencyModuleNames.AddRange(new string[] {
				"Core",
				"CoreUObject",
				"Engine",
				"AIModule",
				"StateTreeModule",
				"ElementalCombat"  // 依赖主游戏模块
			});

			// 测试框架和编辑器功能私有依赖
			PrivateDependencyModuleNames.AddRange(new string[] {
				"UnrealEd"
			});
		}
		
		// 包含路径
		PublicIncludePaths.AddRange(new string[] {
			"ElementalCombatTests/Public"
		});
		
		PrivateIncludePaths.AddRange(new string[] {
			"ElementalCombatTests/Private"
		});
	}
}