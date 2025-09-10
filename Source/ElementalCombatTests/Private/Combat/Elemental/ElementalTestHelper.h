// Copyright 2025 guigui17f. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Combat/Elemental/ElementalConfigManager.h"
#include "Combat/Elemental/DefaultElementalDataAsset.h"

/**
 * 元素测试辅助类
 * 提供测试环境中的配置管理器初始化和清理功能
 */
class FElementalTestHelper
{
public:
	/**
	 * 为测试设置默认的元素配置
	 * @param WorldContextObject 世界上下文对象
	 * @return 配置管理器实例
	 */
	static UElementalConfigManager* SetupDefaultConfiguration(const UObject* WorldContextObject = nullptr);

	/**
	 * 清理测试配置
	 * @param WorldContextObject 世界上下文对象
	 */
	static void CleanupConfiguration(const UObject* WorldContextObject = nullptr);

	/**
	 * 创建自定义配置的数据资产
	 * @param AdvantageMultiplier 克制倍率
	 * @param DisadvantageMultiplier 被克制倍率（目前未使用）
	 * @return 配置数据资产
	 */
	static UDefaultElementalDataAsset* CreateCustomConfiguration(float AdvantageMultiplier = 1.5f, float DisadvantageMultiplier = 0.67f);

private:
	// 测试用的数据资产实例
	static UDefaultElementalDataAsset* TestDataAsset;
};