// Copyright 2025 guigui17f. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ElementalDataAsset.h"
#include "DefaultElementalDataAsset.generated.h"

/**
 * 默认元素配置数据资产
 * 提供标准五行相克关系的配置
 * 可以在编辑器中创建实例并修改配置
 */
UCLASS(BlueprintType, Blueprintable)
class ELEMENTALCOMBAT_API UDefaultElementalDataAsset : public UElementalDataAsset
{
	GENERATED_BODY()

public:
	UDefaultElementalDataAsset();

	/**
	 * 创建标准的五行相克配置
	 * 金克木、木克土、土克水、水克火、火克金
	 * 克制倍率：1.5倍，被克制倍率：通过反向计算得出0.67倍
	 */
	UFUNCTION(BlueprintCallable, Category = "Elemental Config")
	void SetupDefaultConfiguration();

	/**
	 * 设置自定义克制倍率
	 * @param AdvantageMultiplier 克制时的伤害倍率
	 * @param DisadvantageMultiplier 被克制时的伤害倍率
	 */
	UFUNCTION(BlueprintCallable, Category = "Elemental Config") 
	void SetCustomMultipliers(float AdvantageMultiplier = 1.5f, float DisadvantageMultiplier = 0.67f);

private:
	// 添加单个元素的克制关系
	void AddCounterRelationship(EElementalType AttackerElement, EElementalType DefenderElement, float Multiplier = 1.5f);
	
	// 默认克制倍率
	float DefaultAdvantageMultiplier = 1.5f;
};