// Copyright 2025 guigui17f. All Rights Reserved.

#include "CoreMinimal.h"
#include "TestHelpers.h"
#include "Combat/Elemental/ElementalEffectProcessor.h"

/**
 * 伤害倍率应用测试
 */
ELEMENTAL_TEST(Combat.Elemental, ApplyDamageMultiplier)
bool FApplyDamageMultiplierTest::RunTest(const FString& Parameters)
{
	FElementalEffectData EffectData;
	EffectData.DamageMultiplier = 1.5f;
	
	// 基础伤害处理
	TestNearlyEqual(TEXT("100伤害1.5倍率变150"), 
		UElementalEffectProcessor::ApplyDamageMultiplier(100.0f, EffectData), 150.0f, 0.01f);
	TestNearlyEqual(TEXT("50伤害1.5倍率变75"), 
		UElementalEffectProcessor::ApplyDamageMultiplier(50.0f, EffectData), 75.0f, 0.01f);
	TestNearlyEqual(TEXT("0伤害仍为0"), 
		UElementalEffectProcessor::ApplyDamageMultiplier(0.0f, EffectData), 0.0f, 0.01f);
	
	// 不同倍率测试
	EffectData.DamageMultiplier = 2.0f;
	TestNearlyEqual(TEXT("2倍伤害"), 
		UElementalEffectProcessor::ApplyDamageMultiplier(100.0f, EffectData), 200.0f, 0.01f);
	
	EffectData.DamageMultiplier = 1.0f;
	TestNearlyEqual(TEXT("1倍伤害不变"), 
		UElementalEffectProcessor::ApplyDamageMultiplier(100.0f, EffectData), 100.0f, 0.01f);
	
	EffectData.DamageMultiplier = 0.5f;
	TestNearlyEqual(TEXT("0.5倍伤害减半"), 
		UElementalEffectProcessor::ApplyDamageMultiplier(100.0f, EffectData), 50.0f, 0.01f);
	
	return true;
}

/**
 * 木元素吸血计算测试
 */
ELEMENTAL_TEST(Combat.Elemental, CalculateLifeSteal)
bool FCalculateLifeStealTest::RunTest(const FString& Parameters)
{
	FElementalEffectData WoodData;
	
	// 30%吸血
	WoodData.LifeStealPercentage = 0.3f;
	TestNearlyEqual(TEXT("100伤害30%吸血"), 
		UElementalEffectProcessor::CalculateLifeSteal(100.0f, WoodData), 30.0f, 0.01f);
	TestNearlyEqual(TEXT("50伤害30%吸血"), 
		UElementalEffectProcessor::CalculateLifeSteal(50.0f, WoodData), 15.0f, 0.01f);
	
	// 不同吸血比例
	WoodData.LifeStealPercentage = 0.5f;
	TestNearlyEqual(TEXT("50%吸血"), 
		UElementalEffectProcessor::CalculateLifeSteal(100.0f, WoodData), 50.0f, 0.01f);
	
	WoodData.LifeStealPercentage = 1.0f;
	TestNearlyEqual(TEXT("100%吸血"), 
		UElementalEffectProcessor::CalculateLifeSteal(100.0f, WoodData), 100.0f, 0.01f);
	
	WoodData.LifeStealPercentage = 0.0f;
	TestNearlyEqual(TEXT("0%吸血"), 
		UElementalEffectProcessor::CalculateLifeSteal(100.0f, WoodData), 0.0f, 0.01f);
	
	// 边界条件
	WoodData.LifeStealPercentage = 0.3f;
	TestNearlyEqual(TEXT("0伤害0吸血"), 
		UElementalEffectProcessor::CalculateLifeSteal(0.0f, WoodData), 0.0f, 0.01f);
	
	return true;
}

/**
 * 吸血应用测试
 */
ELEMENTAL_TEST(Combat.Elemental, ApplyLifeSteal)
bool FApplyLifeStealTest::RunTest(const FString& Parameters)
{
	// 正常吸血
	TestNearlyEqual(TEXT("50生命吸30血"), 
		UElementalEffectProcessor::ApplyLifeSteal(50.0f, 100.0f, 30.0f), 80.0f, 0.01f);
	
	// 吸血超过上限
	TestNearlyEqual(TEXT("90生命吸30血限制到100"), 
		UElementalEffectProcessor::ApplyLifeSteal(90.0f, 100.0f, 30.0f), 100.0f, 0.01f);
	
	// 满血吸血
	TestNearlyEqual(TEXT("满血无法吸血"), 
		UElementalEffectProcessor::ApplyLifeSteal(100.0f, 100.0f, 30.0f), 100.0f, 0.01f);
	
	// 0吸血
	TestNearlyEqual(TEXT("0吸血无效果"), 
		UElementalEffectProcessor::ApplyLifeSteal(50.0f, 100.0f, 0.0f), 50.0f, 0.01f);
	
	return true;
}

/**
 * 水元素减速计算测试
 */
ELEMENTAL_TEST(Combat.Elemental, CalculateSlowedSpeed)
bool FCalculateSlowedSpeedTest::RunTest(const FString& Parameters)
{
	FElementalEffectData WaterData;
	
	// 50%减速
	WaterData.SlowPercentage = 0.5f;
	TestNearlyEqual(TEXT("600速度50%减速"), 
		UElementalEffectProcessor::CalculateSlowedSpeed(600.0f, WaterData), 300.0f, 0.01f);
	TestNearlyEqual(TEXT("400速度50%减速"), 
		UElementalEffectProcessor::CalculateSlowedSpeed(400.0f, WaterData), 200.0f, 0.01f);
	
	// 不同减速比例
	WaterData.SlowPercentage = 0.3f;
	TestNearlyEqual(TEXT("30%减速"), 
		UElementalEffectProcessor::CalculateSlowedSpeed(600.0f, WaterData), 420.0f, 0.01f);
	
	WaterData.SlowPercentage = 0.8f;
	TestNearlyEqual(TEXT("80%减速"), 
		UElementalEffectProcessor::CalculateSlowedSpeed(600.0f, WaterData), 120.0f, 0.01f);
	
	WaterData.SlowPercentage = 0.0f;
	TestNearlyEqual(TEXT("0%减速无效果"), 
		UElementalEffectProcessor::CalculateSlowedSpeed(600.0f, WaterData), 600.0f, 0.01f);
	
	// 攻速减缓
	WaterData.SlowPercentage = 0.5f;
	TestNearlyEqual(TEXT("攻速50%减缓"), 
		UElementalEffectProcessor::CalculateSlowedAttackSpeed(1.0f, WaterData), 0.5f, 0.01f);
	
	return true;
}

/**
 * 火元素DOT计算测试
 */
ELEMENTAL_TEST(Combat.Elemental, CalculateDotDamage)
bool FCalculateDotDamageTest::RunTest(const FString& Parameters)
{
	FElementalEffectData FireData;
	FireData.DotDamage = 10.0f;
	FireData.DotTickInterval = 0.5f;
	FireData.DotDuration = 5.0f;
	
	// 计算tick次数
	TestEqual(TEXT("5秒0.5秒间隔=10次"), 
		UElementalEffectProcessor::CalculateDotTicks(FireData), 10);
	
	// 计算总伤害
	TestNearlyEqual(TEXT("10次10伤害=100总伤害"), 
		UElementalEffectProcessor::CalculateTotalDotDamage(FireData), 100.0f, 0.01f);
	
	// 不同配置
	FireData.DotTickInterval = 1.0f;
	TestEqual(TEXT("5秒1秒间隔=5次"), 
		UElementalEffectProcessor::CalculateDotTicks(FireData), 5);
	TestNearlyEqual(TEXT("5次10伤害=50总伤害"), 
		UElementalEffectProcessor::CalculateTotalDotDamage(FireData), 50.0f, 0.01f);
	
	// 短持续时间
	FireData.DotDuration = 0.5f;
	FireData.DotTickInterval = 0.5f;
	TestEqual(TEXT("0.5秒0.5秒间隔=1次"), 
		UElementalEffectProcessor::CalculateDotTicks(FireData), 1);
	
	// 零持续时间
	FireData.DotDuration = 0.0f;
	TestEqual(TEXT("0秒持续=0次"), 
		UElementalEffectProcessor::CalculateDotTicks(FireData), 0);
	
	return true;
}

/**
 * DOT伤害与元素相克测试
 */
ELEMENTAL_TEST(Combat.Elemental, CalculateDotTickDamage)
bool FCalculateDotTickDamageTest::RunTest(const FString& Parameters)
{
	FElementalEffectData FireData;
	FireData.DotDamage = 10.0f;
	
	// 火克金
	TestNearlyEqual(TEXT("火克金DOT增强"), 
		UElementalEffectProcessor::CalculateDotTickDamage(FireData, EElementalType::Fire, EElementalType::Metal), 
		15.0f, 0.01f);
	
	// 火被水克
	TestNearlyEqual(TEXT("火被水克DOT减弱"), 
		UElementalEffectProcessor::CalculateDotTickDamage(FireData, EElementalType::Fire, EElementalType::Water), 
		5.0f, 0.01f);
	
	// 无关系
	TestNearlyEqual(TEXT("无关系DOT正常"), 
		UElementalEffectProcessor::CalculateDotTickDamage(FireData, EElementalType::Fire, EElementalType::Wood), 
		10.0f, 0.01f);
	
	return true;
}

/**
 * 土元素减伤测试
 */
ELEMENTAL_TEST(Combat.Elemental, ApplyDamageReduction)
bool FApplyDamageReductionTest::RunTest(const FString& Parameters)
{
	FElementalEffectData EarthData;
	
	// 30%减伤
	EarthData.DamageReduction = 0.3f;
	TestNearlyEqual(TEXT("100伤害30%减伤"), 
		UElementalEffectProcessor::ApplyDamageReduction(100.0f, EarthData), 70.0f, 0.01f);
	TestNearlyEqual(TEXT("50伤害30%减伤"), 
		UElementalEffectProcessor::ApplyDamageReduction(50.0f, EarthData), 35.0f, 0.01f);
	
	// 不同减伤比例
	EarthData.DamageReduction = 0.5f;
	TestNearlyEqual(TEXT("50%减伤"), 
		UElementalEffectProcessor::ApplyDamageReduction(100.0f, EarthData), 50.0f, 0.01f);
	
	EarthData.DamageReduction = 0.8f;
	TestNearlyEqual(TEXT("80%减伤"), 
		UElementalEffectProcessor::ApplyDamageReduction(100.0f, EarthData), 20.0f, 0.01f);
	
	EarthData.DamageReduction = 0.0f;
	TestNearlyEqual(TEXT("0%减伤无效果"), 
		UElementalEffectProcessor::ApplyDamageReduction(100.0f, EarthData), 100.0f, 0.01f);
	
	// 最大减伤限制
	EarthData.DamageReduction = 0.95f;
	float ReducedDamage = UElementalEffectProcessor::ApplyDamageReduction(100.0f, EarthData);
	TestTrue(TEXT("95%减伤仍有伤害"), ReducedDamage > 0.0f);
	TestNearlyEqual(TEXT("95%减伤值"), ReducedDamage, 5.0f, 0.01f);
	
	return true;
}

/**
 * 综合伤害处理测试
 */
ELEMENTAL_TEST(Combat.Elemental, ProcessDamage)
bool FProcessDamageTest::RunTest(const FString& Parameters)
{
	FElementalEffectData AttackerData;
	AttackerData.DamageMultiplier = 1.5f;
	
	// 金克木：1.5 * 1.5 = 2.25
	TestNearlyEqual(TEXT("金克木综合伤害"), 
		UElementalEffectProcessor::ProcessDamage(100.0f, EElementalType::Metal, EElementalType::Wood, AttackerData), 
		225.0f, 0.01f);
	
	// 金被火克：1.5 * 0.5 = 0.75
	TestNearlyEqual(TEXT("金被火克综合伤害"), 
		UElementalEffectProcessor::ProcessDamage(100.0f, EElementalType::Metal, EElementalType::Fire, AttackerData), 
		75.0f, 0.01f);
	
	// 无关系：1.5 * 1.0 = 1.5
	TestNearlyEqual(TEXT("无关系综合伤害"), 
		UElementalEffectProcessor::ProcessDamage(100.0f, EElementalType::Metal, EElementalType::Water, AttackerData), 
		150.0f, 0.01f);
	
	return true;
}

/**
 * 元素效果边界条件测试
 */
ELEMENTAL_TEST(Combat.Elemental, EffectBoundaryConditions)
bool FEffectBoundaryConditionsTest::RunTest(const FString& Parameters)
{
	FElementalEffectData TestEffect;
	
	// 测试负数伤害倍率
	TestEffect.DamageMultiplier = -1.0f;
	float Result = UElementalEffectProcessor::ApplyDamageMultiplier(100.0f, TestEffect);
	TestTrue(TEXT("负数倍率应该被修正"), Result >= 0.0f);
	
	// 测试超过100%的吸血
	TestEffect.LifeStealPercentage = 1.5f;
	Result = UElementalEffectProcessor::CalculateLifeSteal(100.0f, TestEffect);
	TestTrue(TEXT("吸血不应超过100%"), Result <= 100.0f);
	
	// 测试超过100%的减速
	TestEffect.SlowPercentage = 1.2f;
	Result = UElementalEffectProcessor::CalculateSlowedSpeed(600.0f, TestEffect);
	TestTrue(TEXT("减速不应超过100%"), Result >= 0.0f);
	
	// 测试负数DOT伤害
	TestEffect.DotDamage = -10.0f;
	Result = UElementalEffectProcessor::GetDotTickDamage(TestEffect);
	TestTrue(TEXT("DOT伤害不应为负"), Result >= 0.0f);
	
	// 测试超过100%的减伤
	TestEffect.DamageReduction = 1.5f;
	Result = UElementalEffectProcessor::ApplyDamageReduction(100.0f, TestEffect);
	TestTrue(TEXT("减伤不应超过100%"), Result >= 0.0f);
	
	return true;
}