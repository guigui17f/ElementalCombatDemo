// Copyright 2025 guigui17f. All Rights Reserved.

#include "Combat/Elemental/ElementalEffectProcessor.h"

#include "ElementalCalculator.h"

// ===========================================
// 通用伤害倍率应用
// ===========================================

float UElementalEffectProcessor::ApplyDamageMultiplier(float BaseDamage, const FElementalEffectData& EffectData)
{
	if (BaseDamage < 0.0f || EffectData.DamageMultiplier < 0.0f)
	{
		return MIN_DAMAGE;
	}

	return BaseDamage * EffectData.DamageMultiplier;
}

// ===========================================
// 木元素 - 吸血效果
// ===========================================

float UElementalEffectProcessor::CalculateLifeSteal(float DamageDealt, const FElementalEffectData& WoodData)
{
	if (DamageDealt < 0.0f)
	{
		return MIN_DAMAGE;
	}

	// 限制吸血比例不超过100%
	float ClampedPercentage = FMath::Clamp(WoodData.LifeStealPercentage, 0.0f, MAX_LIFE_STEAL_PERCENTAGE);
	return DamageDealt * ClampedPercentage;
}

float UElementalEffectProcessor::ApplyLifeSteal(float CurrentHealth, float MaxHealth, float LifeStealAmount)
{
	if (LifeStealAmount < 0.0f || CurrentHealth < 0.0f || MaxHealth <= 0.0f)
	{
		return CurrentHealth;
	}

	// 限制生命值不超过最大值
	return FMath::Min(CurrentHealth + LifeStealAmount, MaxHealth);
}

// ===========================================
// 水元素 - 减速效果
// ===========================================

float UElementalEffectProcessor::CalculateSlowedSpeed(float BaseSpeed, const FElementalEffectData& WaterData)
{
	if (BaseSpeed < 0.0f)
	{
		return 0.0f;
	}

	// 限制减速比例不超过100%
	float ClampedSlowPercentage = FMath::Clamp(WaterData.SlowPercentage, 0.0f, MAX_SLOW_PERCENTAGE);
	float SlowedSpeed = BaseSpeed * (1.0f - ClampedSlowPercentage);
	
	// 确保速度不为负
	return FMath::Max(SlowedSpeed, 0.0f);
}

float UElementalEffectProcessor::CalculateSlowedAttackSpeed(float BaseAttackSpeed, const FElementalEffectData& WaterData)
{
	if (BaseAttackSpeed < 0.0f)
	{
		return 0.0f;
	}

	// 限制减速比例不超过100%
	float ClampedSlowPercentage = FMath::Clamp(WaterData.SlowPercentage, 0.0f, MAX_SLOW_PERCENTAGE);
	float SlowedAttackSpeed = BaseAttackSpeed * (1.0f - ClampedSlowPercentage);
	
	// 确保攻击速度不为负
	return FMath::Max(SlowedAttackSpeed, 0.0f);
}

// ===========================================
// 火元素 - DOT效果
// ===========================================

int32 UElementalEffectProcessor::CalculateDotTicks(const FElementalEffectData& FireData)
{
	if (FireData.DotDuration <= 0.0f || FireData.DotTickInterval <= 0.0f)
	{
		return 0;
	}

	return FMath::FloorToInt(FireData.DotDuration / FireData.DotTickInterval);
}

float UElementalEffectProcessor::CalculateTotalDotDamage(const FElementalEffectData& FireData)
{
	int32 TickCount = CalculateDotTicks(FireData);
	if (TickCount <= 0 || FireData.DotDamage < 0.0f)
	{
		return MIN_DAMAGE;
	}

	return FireData.DotDamage * TickCount;
}

float UElementalEffectProcessor::GetDotTickDamage(const FElementalEffectData& FireData)
{
	return FMath::Max(FireData.DotDamage, 0.0f);
}

float UElementalEffectProcessor::CalculateDotTickDamage(const FElementalEffectData& FireData, EElementalType AttackerElement, EElementalType DefenderElement)
{
	float BaseDotDamage = GetDotTickDamage(FireData);
	if (BaseDotDamage <= 0.0f)
	{
		return MIN_DAMAGE;
	}

	// 应用元素相克修正
	float ElementMultiplier = UElementalCalculator::CalculateCounterMultiplier(AttackerElement, DefenderElement);
	return BaseDotDamage * ElementMultiplier;
}

// ===========================================
// 土元素 - 减伤效果
// ===========================================

float UElementalEffectProcessor::ApplyDamageReduction(float IncomingDamage, const FElementalEffectData& EarthData)
{
	if (IncomingDamage < 0.0f)
	{
		return MIN_DAMAGE;
	}

	// 限制减伤比例不超过100%
	float ClampedReduction = FMath::Clamp(EarthData.DamageReduction, 0.0f, MAX_DAMAGE_REDUCTION);
	float ReducedDamage = IncomingDamage * (1.0f - ClampedReduction);

	// 确保减伤后伤害不为负
	return FMath::Max(ReducedDamage, 0.0f);
}

// ===========================================
// 综合处理函数
// ===========================================

float UElementalEffectProcessor::ProcessDamage(float BaseDamage, EElementalType AttackerElement, EElementalType DefenderElement, const FElementalEffectData& AttackerData)
{
	if (BaseDamage < 0.0f)
	{
		return MIN_DAMAGE;
	}

	// 首先应用攻击者的元素效果（如金元素的伤害倍率）
	float ProcessedDamage = BaseDamage;
	
	if (AttackerElement == EElementalType::Metal)
	{
		ProcessedDamage = ApplyDamageMultiplier(BaseDamage, AttackerData);
	}

	// 然后应用元素相克修正
	float CounterMultiplier = UElementalCalculator::CalculateCounterMultiplier(AttackerElement, DefenderElement);
	ProcessedDamage *= CounterMultiplier;

	// 确保最终伤害不为负
	return FMath::Max(ProcessedDamage, 0.0f);
}