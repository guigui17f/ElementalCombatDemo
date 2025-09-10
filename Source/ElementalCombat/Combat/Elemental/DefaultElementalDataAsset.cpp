// Copyright 2025 guigui17f. All Rights Reserved.

#include "DefaultElementalDataAsset.h"

UDefaultElementalDataAsset::UDefaultElementalDataAsset()
{
	// 自动设置默认配置
	SetupDefaultConfiguration();
}

void UDefaultElementalDataAsset::SetupDefaultConfiguration()
{
	// 清空现有配置
	ElementRelationships.Empty();
	
	// 设置五行相克关系：金克木、木克土、土克水、水克火、火克金
	AddCounterRelationship(EElementalType::Metal, EElementalType::Wood, DefaultAdvantageMultiplier);
	AddCounterRelationship(EElementalType::Wood, EElementalType::Earth, DefaultAdvantageMultiplier);
	AddCounterRelationship(EElementalType::Earth, EElementalType::Water, DefaultAdvantageMultiplier);
	AddCounterRelationship(EElementalType::Water, EElementalType::Fire, DefaultAdvantageMultiplier);
	AddCounterRelationship(EElementalType::Fire, EElementalType::Metal, DefaultAdvantageMultiplier);
	
	// 重建缓存
	BuildElementMaps();
}

void UDefaultElementalDataAsset::SetCustomMultipliers(float AdvantageMultiplier, float DisadvantageMultiplier)
{
	DefaultAdvantageMultiplier = AdvantageMultiplier;
	
	// 重新设置配置使用新的倍率
	SetupDefaultConfiguration();
}

void UDefaultElementalDataAsset::AddCounterRelationship(EElementalType AttackerElement, EElementalType DefenderElement, float Multiplier)
{
	// 查找现有的关系配置
	FElementalRelationship* ExistingRelationship = nullptr;
	for (FElementalRelationship& Relationship : ElementRelationships)
	{
		if (Relationship.Element == AttackerElement)
		{
			ExistingRelationship = &Relationship;
			break;
		}
	}
	
	// 如果不存在，创建新的关系配置
	if (!ExistingRelationship)
	{
		FElementalRelationship NewRelationship;
		NewRelationship.Element = AttackerElement;
		ElementRelationships.Add(NewRelationship);
		ExistingRelationship = &ElementRelationships.Last();
	}
	
	// 添加克制数据
	FElementalCounterData CounterData;
	CounterData.CounteredElement = DefenderElement;
	CounterData.EffectMultiplier = Multiplier;
	ExistingRelationship->Counters.Add(CounterData);
}