// Copyright 2025 guigui17f. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ElementalTypes.h"
#include "ElementalEffectProcessor.generated.h"

/**
 * 元素效果处理器
 * 提供处理各种元素效果的静态函数库
 */
UCLASS()
class ELEMENTALCOMBAT_API UElementalEffectProcessor : public UObject
{
	GENERATED_BODY()

public:
	// ===========================================
	// 通用伤害倍率应用
	// ===========================================
	
	/**
	 * 应用伤害倍率
	 * @param BaseDamage 基础伤害
	 * @param EffectData 元素效果数据（使用其DamageMultiplier字段）
	 * @return 应用倍率后的伤害
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Elemental|General")
	static float ApplyDamageMultiplier(float BaseDamage, const FElementalEffectData& EffectData);

	// ===========================================
	// 木元素 - 吸血效果
	// ===========================================
	
	/**
	 * 计算吸血数值
	 * @param DamageDealt 造成的伤害
	 * @param WoodData 木元素效果数据
	 * @return 吸血数值
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Elemental|Wood")
	static float CalculateLifeSteal(float DamageDealt, const FElementalEffectData& WoodData);

	/**
	 * 应用吸血效果
	 * @param CurrentHealth 当前生命值
	 * @param MaxHealth 最大生命值
	 * @param LifeStealAmount 吸血数值
	 * @return 恢复后的生命值
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Elemental|Wood")
	static float ApplyLifeSteal(float CurrentHealth, float MaxHealth, float LifeStealAmount);

	// ===========================================
	// 水元素 - 减速效果
	// ===========================================
	
	/**
	 * 计算减速后的移动速度
	 * @param BaseSpeed 基础移动速度
	 * @param WaterData 水元素效果数据
	 * @return 减速后的移动速度
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Elemental|Water")
	static float CalculateSlowedSpeed(float BaseSpeed, const FElementalEffectData& WaterData);

	/**
	 * 计算减速后的攻击速度
	 * @param BaseAttackSpeed 基础攻击速度
	 * @param WaterData 水元素效果数据
	 * @return 减速后的攻击速度
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Elemental|Water")
	static float CalculateSlowedAttackSpeed(float BaseAttackSpeed, const FElementalEffectData& WaterData);

	// ===========================================
	// 火元素 - DOT效果
	// ===========================================
	
	/**
	 * 计算DOT总tick次数
	 * @param FireData 火元素效果数据
	 * @return tick次数
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Elemental|Fire")
	static int32 CalculateDotTicks(const FElementalEffectData& FireData);

	/**
	 * 计算DOT总伤害
	 * @param FireData 火元素效果数据
	 * @return 总DOT伤害
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Elemental|Fire")
	static float CalculateTotalDotDamage(const FElementalEffectData& FireData);

	/**
	 * 获取单次DOT伤害
	 * @param FireData 火元素效果数据
	 * @return 单次DOT伤害
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Elemental|Fire")
	static float GetDotTickDamage(const FElementalEffectData& FireData);

	/**
	 * 计算考虑元素相克的DOT伤害
	 * @param FireData 火元素效果数据
	 * @param AttackerElement 攻击者元素
	 * @param DefenderElement 防御者元素
	 * @return 修正后的DOT伤害
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Elemental|Fire")
	static float CalculateDotTickDamage(const FElementalEffectData& FireData, EElementalType AttackerElement, EElementalType DefenderElement);

	// ===========================================
	// 土元素 - 减伤效果
	// ===========================================
	
	/**
	 * 应用伤害减免
	 * @param IncomingDamage 传入伤害
	 * @param EarthData 土元素效果数据
	 * @return 减免后的伤害
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Elemental|Earth")
	static float ApplyDamageReduction(float IncomingDamage, const FElementalEffectData& EarthData);

	// ===========================================
	// 综合处理函数
	// ===========================================
	
	/**
	 * 综合处理元素伤害（包括元素相克和元素自身效果）
	 * @param BaseDamage 基础伤害
	 * @param AttackerElement 攻击者元素
	 * @param DefenderElement 防御者元素
	 * @param AttackerData 攻击者元素效果数据
	 * @return 最终伤害
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Elemental|General")
	static float ProcessDamage(float BaseDamage, EElementalType AttackerElement, EElementalType DefenderElement, const FElementalEffectData& AttackerData);

private:
	// 常量定义
	static constexpr float MAX_LIFE_STEAL_PERCENTAGE = 1.0f;
	static constexpr float MAX_SLOW_PERCENTAGE = 1.0f;
	static constexpr float MAX_DAMAGE_REDUCTION = 1.0f;
	static constexpr float MIN_DAMAGE = 0.0f;
};