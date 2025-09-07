// Copyright 2025 guigui17f. All Rights Reserved.

#include "Modules/ModuleManager.h"

/**
 * ElementalCombatTests模块
 * 仅在编辑器中可用的测试模块，用于运行单元测试
 */
class FElementalCombatTestsModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override
	{
		// 测试模块启动时的初始化
		UE_LOG(LogTemp, Log, TEXT("ElementalCombatTests 模块已启动"));
	}

	virtual void ShutdownModule() override
	{
		// 测试模块关闭时的清理
		UE_LOG(LogTemp, Log, TEXT("ElementalCombatTests 模块已关闭"));
	}
};

IMPLEMENT_MODULE(FElementalCombatTestsModule, ElementalCombatTests)