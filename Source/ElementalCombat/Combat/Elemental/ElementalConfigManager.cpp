// Copyright 2025 guigui17f. All Rights Reserved.

#include "ElementalConfigManager.h"
#include "Engine/World.h"
#include "Engine/GameInstance.h"

UElementalConfigManager* UElementalConfigManager::GetInstance(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	if (const UWorld* World = WorldContextObject->GetWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			return GameInstance->GetSubsystem<UElementalConfigManager>();
		}
	}

	return nullptr;
}

void UElementalConfigManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	CurrentDataAsset = nullptr;
}

void UElementalConfigManager::Deinitialize()
{
	CurrentDataAsset = nullptr;
	Super::Deinitialize();
}

void UElementalConfigManager::SetElementalDataAsset(UElementalDataAsset* DataAsset)
{
	CurrentDataAsset = DataAsset;
}

bool UElementalConfigManager::IsElementAdvantage(EElementalType AttackerElement, EElementalType DefenderElement) const
{
	// 如果任一元素为None，则无克制关系
	if (AttackerElement == EElementalType::None || DefenderElement == EElementalType::None)
	{
		return false;
	}

	// 尝试从配置数据获取
	if (CurrentDataAsset)
	{
		FElementalRelationship Relationship;
		if (CurrentDataAsset->GetElementRelationship(AttackerElement, Relationship))
		{
			for (const FElementalCounterData& Counter : Relationship.Counters)
			{
				if (Counter.CounteredElement == DefenderElement)
				{
					return true;
				}
			}
		}
	}

	// 如果没有配置数据，使用默认硬编码逻辑作为后备
	return IsElementAdvantageDefault(AttackerElement, DefenderElement);
}

float UElementalConfigManager::GetCounterMultiplier(EElementalType AttackerElement, EElementalType DefenderElement) const
{
	// 如果任一元素为None，返回中性倍率
	if (AttackerElement == EElementalType::None || DefenderElement == EElementalType::None)
	{
		return DEFAULT_NEUTRAL_MULTIPLIER;
	}

	// 检查攻击者是否克制防御者
	if (CurrentDataAsset)
	{
		FElementalRelationship Relationship;
		if (CurrentDataAsset->GetElementRelationship(AttackerElement, Relationship))
		{
			for (const FElementalCounterData& Counter : Relationship.Counters)
			{
				if (Counter.CounteredElement == DefenderElement)
				{
					return Counter.EffectMultiplier; // 返回配置的倍率
				}
			}
		}

		// 检查防御者是否克制攻击者（被克制情况）
		if (CurrentDataAsset->GetElementRelationship(DefenderElement, Relationship))
		{
			for (const FElementalCounterData& Counter : Relationship.Counters)
			{
				if (Counter.CounteredElement == AttackerElement)
				{
					// 被克制时使用倒数，但至少为0.1倍
					float InverseMultiplier = 1.0f / FMath::Max(Counter.EffectMultiplier, 1.0f);
					return FMath::Max(InverseMultiplier, 0.1f);
				}
			}
		}
	}
	else
	{
		// 没有配置数据时使用默认逻辑
		if (IsElementAdvantageDefault(AttackerElement, DefenderElement))
		{
			return DEFAULT_ADVANTAGE_MULTIPLIER;
		}
		if (IsElementAdvantageDefault(DefenderElement, AttackerElement))
		{
			return DEFAULT_DISADVANTAGE_MULTIPLIER;
		}
	}

	// 无关系或相同元素
	return DEFAULT_NEUTRAL_MULTIPLIER;
}

bool UElementalConfigManager::GetElementRelationship(EElementalType Element, FElementalRelationship& OutRelationship) const
{
	if (CurrentDataAsset)
	{
		return CurrentDataAsset->GetElementRelationship(Element, OutRelationship);
	}
	return false;
}

bool UElementalConfigManager::HasValidConfiguration() const
{
	return CurrentDataAsset != nullptr;
}

bool UElementalConfigManager::IsElementAdvantageDefault(EElementalType AttackerElement, EElementalType DefenderElement) const
{
	// 硬编码的五行相克：金克木、木克土、土克水、水克火、火克金
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