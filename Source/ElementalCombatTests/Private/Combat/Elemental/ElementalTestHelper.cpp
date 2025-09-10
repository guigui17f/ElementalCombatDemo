// Copyright 2025 guigui17f. All Rights Reserved.

#include "ElementalTestHelper.h"

// 静态成员初始化
UDefaultElementalDataAsset* FElementalTestHelper::TestDataAsset = nullptr;

UElementalConfigManager* FElementalTestHelper::SetupDefaultConfiguration(const UObject* WorldContextObject)
{
	// 创建测试用的数据资产
	TestDataAsset = NewObject<UDefaultElementalDataAsset>();
	if (!TestDataAsset)
	{
		return nullptr;
	}
	
	// 确保设置了默认配置
	TestDataAsset->SetupDefaultConfiguration();
	
	// 获取配置管理器并设置数据资产
	// 注意：在测试环境中可能没有真正的GameInstance，这里模拟设置
	// 实际测试时可能需要根据具体测试环境调整
	if (UElementalConfigManager* ConfigManager = UElementalConfigManager::GetInstance(WorldContextObject))
	{
		ConfigManager->SetElementalDataAsset(TestDataAsset);
		return ConfigManager;
	}
	
	return nullptr;
}

void FElementalTestHelper::CleanupConfiguration(const UObject* WorldContextObject)
{
	if (UElementalConfigManager* ConfigManager = UElementalConfigManager::GetInstance(WorldContextObject))
	{
		ConfigManager->SetElementalDataAsset(nullptr);
	}
	
	// 清理测试数据资产
	if (TestDataAsset)
	{
		TestDataAsset = nullptr;
	}
}

UDefaultElementalDataAsset* FElementalTestHelper::CreateCustomConfiguration(float AdvantageMultiplier, float DisadvantageMultiplier)
{
	UDefaultElementalDataAsset* CustomAsset = NewObject<UDefaultElementalDataAsset>();
	if (CustomAsset)
	{
		CustomAsset->SetCustomMultipliers(AdvantageMultiplier, DisadvantageMultiplier);
	}
	return CustomAsset;
}