// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class ElementalCombatEditorTarget : TargetRules
{
	public ElementalCombatEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_6;
		ExtraModuleNames.Add("ElementalCombat");

		// 添加编辑器模块
		ExtraModuleNames.Add("ElementalCombatEditor");

		// 添加测试模块（仅在编辑器目标中）
		ExtraModuleNames.Add("ElementalCombatTests");
	}
}
