// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class ElementalCombatEditorTarget : TargetRules
{
	public ElementalCombatEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("ElementalCombat");

		// 添加编辑器模块
		ExtraModuleNames.Add("ElementalCombatEditor");

		// 添加测试模块（仅在编辑器目标中）
		ExtraModuleNames.Add("ElementalCombatTests");
	}
}
