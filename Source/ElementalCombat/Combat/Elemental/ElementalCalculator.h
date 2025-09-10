// Copyright 2025 guigui17f. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ElementalTypes.h"
#include "ElementalCalculator.generated.h"

/**
 * 元素计算器
 * 提供五行相克关系判断和伤害修正计算的静态函数库
 * 现在使用数据驱动方式，从ElementalConfigManager获取配置数据
 */
UCLASS()
class ELEMENTALCOMBAT_API UElementalCalculator : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * 判断元素克制关系
	 * 现在从配置数据中读取，如果没有配置则使用默认五行相克
	 * 
	 * @param AttackerElement 攻击者元素
	 * @param DefenderElement 防御者元素
	 * @param WorldContextObject 世界上下文对象（用于获取配置管理器）
	 * @return true如果攻击者克制防御者
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Elemental")
	static bool IsElementAdvantage(EElementalType AttackerElement, EElementalType DefenderElement, const UObject* WorldContextObject = nullptr);

	/**
	 * 计算元素相克倍率
	 * 现在从配置数据中读取倍率，如果没有配置则使用默认值
	 * 
	 * @param AttackerElement 攻击者元素
	 * @param DefenderElement 防御者元素
	 * @param WorldContextObject 世界上下文对象（用于获取配置管理器）
	 * @return 伤害倍率
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Elemental")
	static float CalculateCounterMultiplier(EElementalType AttackerElement, EElementalType DefenderElement, const UObject* WorldContextObject = nullptr);

	/**
	 * 计算元素伤害修正值
	 * 等同于CalculateCounterMultiplier，提供不同的函数名以便理解
	 * 
	 * @param AttackElement 攻击元素
	 * @param DefenseElement 防御元素
	 * @return 伤害修正倍率
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Elemental")
	static float CalculateElementalDamageModifier(EElementalType AttackElement, EElementalType DefenseElement, const UObject* WorldContextObject = nullptr);

	/**
	 * 计算最终伤害
	 * 基础伤害 * 元素修正倍率，确保结果不为负数
	 * 
	 * @param BaseDamage 基础伤害
	 * @param AttackerElement 攻击者元素
	 * @param DefenderElement 防御者元素
	 * @return 最终伤害值
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Elemental")
	static float CalculateFinalDamage(float BaseDamage, EElementalType AttackerElement, EElementalType DefenderElement, const UObject* WorldContextObject = nullptr);

private:
	// 克制关系倍率常量
	static constexpr float ADVANTAGE_MULTIPLIER = 1.5f;
	static constexpr float DISADVANTAGE_MULTIPLIER = 0.5f;
	static constexpr float NEUTRAL_MULTIPLIER = 1.0f;
};