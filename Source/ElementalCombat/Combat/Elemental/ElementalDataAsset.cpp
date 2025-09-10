// Copyright 2025 guigui17f. All Rights Reserved.

#include "Combat/Elemental/ElementalDataAsset.h"
#include "Combat/Elemental/ElementalComponent.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

UElementalDataAsset::UElementalDataAsset()
{
	bCacheBuilt = false;
	
	// 设置默认的元素效果数据
	DefaultEffectData.Element = EElementalType::None;
	DefaultEffectData.DamageMultiplier = 1.0f;
	DefaultEffectData.LifeStealPercentage = 0.0f;
	DefaultEffectData.SlowPercentage = 0.0f;
	DefaultEffectData.SlowDuration = 0.0f;
	DefaultEffectData.DotDamage = 0.0f;
	DefaultEffectData.DotTickInterval = 1.0f;
	DefaultEffectData.DotDuration = 0.0f;
	DefaultEffectData.DamageReduction = 0.0f;
	DefaultEffectData.ProjectileClass = nullptr;
}

bool UElementalDataAsset::GetElementEffectData(EElementalType Element, FElementalEffectData& OutEffectData) const
{
	const FElementalEffectData* FoundData = GetElementEffectDataPtr(Element);
	if (FoundData)
	{
		OutEffectData = *FoundData;
		return true;
	}
	
	OutEffectData = DefaultEffectData;
	return false;
}

bool UElementalDataAsset::GetElementRelationship(EElementalType Element, FElementalRelationship& OutRelationship) const
{
	const FElementalRelationship* FoundRelationship = GetElementRelationshipPtr(Element);
	if (FoundRelationship)
	{
		OutRelationship = *FoundRelationship;
		return true;
	}
	
	return false;
}

const FElementalEffectData* UElementalDataAsset::GetElementEffectDataPtr(EElementalType Element) const
{
	// 确保缓存已构建
	if (!bCacheBuilt)
	{
		const_cast<UElementalDataAsset*>(this)->BuildElementMaps();
	}

	const FElementalEffectData* FoundData = ElementEffectMap.Find(Element);
	return FoundData ? FoundData : &DefaultEffectData;
}

const FElementalRelationship* UElementalDataAsset::GetElementRelationshipPtr(EElementalType Element) const
{
	// 确保缓存已构建
	if (!bCacheBuilt)
	{
		const_cast<UElementalDataAsset*>(this)->BuildElementMaps();
	}

	return ElementRelationshipMap.Find(Element);
}

bool UElementalDataAsset::HasElementConfiguration(EElementalType Element) const
{
	// 确保缓存已构建
	if (!bCacheBuilt)
	{
		const_cast<UElementalDataAsset*>(this)->BuildElementMaps();
	}

	return ElementEffectMap.Contains(Element);
}

bool UElementalDataAsset::ValidateData(FString& OutErrorMessage) const
{
	TArray<FString> ErrorMessages;

	// 检查是否至少配置了一个元素
	if (ElementEffects.Num() == 0)
	{
		ErrorMessages.Add(TEXT("No element effects configured"));
	}

	// 验证元素效果数据
	for (int32 i = 0; i < ElementEffects.Num(); ++i)
	{
		const FElementalEffectData& Effect = ElementEffects[i];
		
		// 检查伤害倍率
		if (Effect.DamageMultiplier < 0.0f)
		{
			ErrorMessages.Add(FString::Printf(TEXT("Element effect %d: DamageMultiplier cannot be negative"), i));
		}
		
		// 检查吸血比例
		if (Effect.LifeStealPercentage < 0.0f || Effect.LifeStealPercentage > 1.0f)
		{
			ErrorMessages.Add(FString::Printf(TEXT("Element effect %d: LifeStealPercentage must be between 0 and 1"), i));
		}
		
		// 检查减速比例
		if (Effect.SlowPercentage < 0.0f || Effect.SlowPercentage > 1.0f)
		{
			ErrorMessages.Add(FString::Printf(TEXT("Element effect %d: SlowPercentage must be between 0 and 1"), i));
		}
		
		// 检查DOT参数
		if (Effect.DotDamage < 0.0f)
		{
			ErrorMessages.Add(FString::Printf(TEXT("Element effect %d: DotDamage cannot be negative"), i));
		}
		
		if (Effect.DotTickInterval <= 0.0f)
		{
			ErrorMessages.Add(FString::Printf(TEXT("Element effect %d: DotTickInterval must be positive"), i));
		}
		
		if (Effect.DotDuration < 0.0f)
		{
			ErrorMessages.Add(FString::Printf(TEXT("Element effect %d: DotDuration cannot be negative"), i));
		}
		
		// 检查减伤比例
		if (Effect.DamageReduction < 0.0f || Effect.DamageReduction > 1.0f)
		{
			ErrorMessages.Add(FString::Printf(TEXT("Element effect %d: DamageReduction must be between 0 and 1"), i));
		}
	}

	// 验证元素相克关系
	for (int32 i = 0; i < ElementRelationships.Num(); ++i)
	{
		const FElementalRelationship& Relationship = ElementRelationships[i];
		
		for (int32 j = 0; j < Relationship.Counters.Num(); ++j)
		{
			const FElementalCounterData& Counter = Relationship.Counters[j];
			
			if (Counter.EffectMultiplier < 0.0f)
			{
				ErrorMessages.Add(FString::Printf(TEXT("Element relationship %d, counter %d: EffectMultiplier cannot be negative"), i, j));
			}
		}
	}

	// 组合错误信息
	if (ErrorMessages.Num() > 0)
	{
		OutErrorMessage = FString::Join(ErrorMessages, TEXT("; "));
		return false;
	}

	OutErrorMessage = TEXT("Data validation passed");
	return true;
}

TArray<EElementalType> UElementalDataAsset::GetConfiguredElements() const
{
	// 确保缓存已构建
	if (!bCacheBuilt)
	{
		const_cast<UElementalDataAsset*>(this)->BuildElementMaps();
	}

	TArray<EElementalType> ConfiguredElements;
	ElementEffectMap.GetKeys(ConfiguredElements);
	return ConfiguredElements;
}

void UElementalDataAsset::CopyDataToComponent(UElementalComponent* ElementComponent) const
{
	if (!ElementComponent)
	{
		return;
	}

	// 确保缓存已构建
	if (!bCacheBuilt)
	{
		const_cast<UElementalDataAsset*>(this)->BuildElementMaps();
	}

	// 复制所有元素效果数据到组件
	for (const auto& ElementPair : ElementEffectMap)
	{
		ElementComponent->SetElementEffectData(ElementPair.Key, ElementPair.Value);
	}
}

void UElementalDataAsset::BuildElementMaps()
{
	ElementEffectMap.Empty();
	ElementRelationshipMap.Empty();

	// 构建元素效果映射 - 使用Element字段作为键
	for (const FElementalEffectData& Effect : ElementEffects)
	{
		if (Effect.Element != EElementalType::None)
		{
			ElementEffectMap.Add(Effect.Element, Effect);
		}
	}

	// 构建元素关系映射
	for (const FElementalRelationship& Relationship : ElementRelationships)
	{
		if (Relationship.Element != EElementalType::None)
		{
			ElementRelationshipMap.Add(Relationship.Element, Relationship);
		}
	}

	bCacheBuilt = true;
}

#if WITH_EDITOR
EDataValidationResult UElementalDataAsset::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	FString ErrorMessage;
	if (!const_cast<UElementalDataAsset*>(this)->ValidateData(ErrorMessage))
	{
		Context.AddError(FText::FromString(ErrorMessage));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}
#endif