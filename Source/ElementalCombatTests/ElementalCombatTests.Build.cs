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
				"UnrealEd",
				"ElementalCombat"  // 依赖主游戏模块
			});
			
			// 自动化测试框架依赖
			if (Target.bBuildDeveloperTools || 
			    (Target.Configuration != UnrealTargetConfiguration.Shipping && 
			     Target.Configuration != UnrealTargetConfiguration.Test))
			{
				PrivateDependencyModuleNames.AddRange(new string[] {
					"AutomationController",
					"FunctionalTesting"
				});
			}
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