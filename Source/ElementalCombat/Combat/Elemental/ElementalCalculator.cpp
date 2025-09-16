// Copyright 2025 guigui17f. All Rights Reserved.

#include "Combat/Elemental/ElementalCalculator.h"
#include "ElementalConfigManager.h"

bool UElementalCalculator::IsElementAdvantage(EElementalType AttackerElement, EElementalType DefenderElement, const UObject* WorldContextObject)
{
	// 尝试从配置管理器获取
	if (UElementalConfigManager* ConfigManager = UElementalConfigManager::GetInstance(WorldContextObject))
	{
		return ConfigManager->IsElementAdvantage(AttackerElement, DefenderElement);
	}

	// 如果无法获取配置管理器，使用默认硬编码逻辑
	if (AttackerElement == EElementalType::None || DefenderElement == EElementalType::None)
	{
		return false;
	}

	// 五行相克：金克木、木克土、土克水、水克火、火克金
	switch (AttackerElement)
	{
	case EElementalType::Metal:
		return DefenderElement == EElementalType::Wood;
	case EElementalType::Wood:
		return DefenderElement == EElementalType::Earth;
	case EElementalType::Water:
		return DefenderElement == EElementalType::Fire;
	case EElementalType::Fire:
		return DefenderElement == EElementalType::Metal;
	case EElementalType::Earth:
		return DefenderElement == EElementalType::Water;
	default:
		return false;
	}
}

float UElementalCalculator::CalculateCounterMultiplier(EElementalType AttackerElement, EElementalType DefenderElement, const UObject* WorldContextObject)
{
	// 尝试从配置管理器获取
	if (UElementalConfigManager* ConfigManager = UElementalConfigManager::GetInstance(WorldContextObject))
	{
		return ConfigManager->GetCounterMultiplier(AttackerElement, DefenderElement);
	}

	// 如果无法获取配置管理器，使用默认逻辑
	// 检查克制关系
	if (IsElementAdvantage(AttackerElement, DefenderElement, WorldContextObject))
	{
		return ADVANTAGE_MULTIPLIER; // 1.5倍伤害
	}
	
	// 检查被克制关系
	if (IsElementAdvantage(DefenderElement, AttackerElement, WorldContextObject))
	{
		return DISADVANTAGE_MULTIPLIER; // 0.5倍伤害
	}
	
	// 无关系或相同元素
	return NEUTRAL_MULTIPLIER; // 1.0倍伤害
}

float UElementalCalculator::CalculateElementalDamageModifier(EElementalType AttackElement, EElementalType DefenseElement, const UObject* WorldContextObject)
{
	// 与CalculateCounterMultiplier功能相同，提供不同命名以便理解
	return CalculateCounterMultiplier(AttackElement, DefenseElement, WorldContextObject);
}

float UElementalCalculator::CalculateFinalDamage(float BaseDamage, EElementalType AttackerElement, EElementalType DefenderElement, const UObject* WorldContextObject)
{
	// 防止负数伤害
	if (BaseDamage < 0.0f)
	{
		return 0.0f;
	}

	float Multiplier = CalculateCounterMultiplier(AttackerElement, DefenderElement, WorldContextObject);
	float FinalDamage = BaseDamage * Multiplier;

	// 确保最终伤害不为负数
	return FMath::Max(FinalDamage, 0.0f);
}

EElementalType UElementalCalculator::GetElementThatCounters(EElementalType Element)
{
	// 五行相克：金克木、木克土、土克水、水克火、火克金
	// 反向查找：谁克制这个元素？
	switch (Element)
	{
	case EElementalType::Metal:
		return EElementalType::Fire;  // 火克金
	case EElementalType::Wood:
		return EElementalType::Metal; // 金克木
	case EElementalType::Water:
		return EElementalType::Earth; // 土克水
	case EElementalType::Fire:
		return EElementalType::Water; // 水克火
	case EElementalType::Earth:
		return EElementalType::Wood;  // 木克土
	default:
		return EElementalType::None;
	}
}

EElementalType UElementalCalculator::GetElementCounteredBy(EElementalType Element)
{
	// 五行相克：金克木、木克土、土克水、水克火、火克金
	// 正向查找：这个元素克制谁？
	switch (Element)
	{
	case EElementalType::Metal:
		return EElementalType::Wood;  // 金克木
	case EElementalType::Wood:
		return EElementalType::Earth; // 木克土
	case EElementalType::Water:
		return EElementalType::Fire;  // 水克火
	case EElementalType::Fire:
		return EElementalType::Metal; // 火克金
	case EElementalType::Earth:
		return EElementalType::Water; // 土克水
	default:
		return EElementalType::None;
	}
}