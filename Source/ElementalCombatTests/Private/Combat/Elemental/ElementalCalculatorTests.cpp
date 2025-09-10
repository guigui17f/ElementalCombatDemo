// Copyright 2025 guigui17f. All Rights Reserved.

#include "CoreMinimal.h"
#include "TestHelpers.h"
#include "Combat/Elemental/ElementalCalculator.h"

/**
 * 五行相克关系验证测试
 */
ELEMENTAL_TEST(Combat.Elemental, ElementalCounterRelations)
bool FElementalCounterRelationsTest::RunTest(const FString& Parameters)
{
	// 金克木
	TestTrue(TEXT("金克木"), UElementalCalculator::IsElementAdvantage(EElementalType::Metal, EElementalType::Wood));
	TestFalse(TEXT("金不克火"), UElementalCalculator::IsElementAdvantage(EElementalType::Metal, EElementalType::Fire));
	
	// 木克土
	TestTrue(TEXT("木克土"), UElementalCalculator::IsElementAdvantage(EElementalType::Wood, EElementalType::Earth));
	TestFalse(TEXT("木不克水"), UElementalCalculator::IsElementAdvantage(EElementalType::Wood, EElementalType::Water));
	
	// 土克水
	TestTrue(TEXT("土克水"), UElementalCalculator::IsElementAdvantage(EElementalType::Earth, EElementalType::Water));
	TestFalse(TEXT("土不克金"), UElementalCalculator::IsElementAdvantage(EElementalType::Earth, EElementalType::Metal));
	
	// 水克火
	TestTrue(TEXT("水克火"), UElementalCalculator::IsElementAdvantage(EElementalType::Water, EElementalType::Fire));
	TestFalse(TEXT("水不克木"), UElementalCalculator::IsElementAdvantage(EElementalType::Water, EElementalType::Wood));
	
	// 火克金
	TestTrue(TEXT("火克金"), UElementalCalculator::IsElementAdvantage(EElementalType::Fire, EElementalType::Metal));
	TestFalse(TEXT("火不克土"), UElementalCalculator::IsElementAdvantage(EElementalType::Fire, EElementalType::Earth));
	
	return true;
}

/**
 * 相克倍率计算测试
 */
ELEMENTAL_TEST(Combat.Elemental, CalculateCounterMultiplier)
bool FCalculateCounterMultiplierTest::RunTest(const FString& Parameters)
{
	// 克制关系 - 返回1.5倍
	TestNearlyEqual(TEXT("金克木1.5倍"), 
		UElementalCalculator::CalculateCounterMultiplier(EElementalType::Metal, EElementalType::Wood), 1.5f, 0.01f);
	TestNearlyEqual(TEXT("木克土1.5倍"), 
		UElementalCalculator::CalculateCounterMultiplier(EElementalType::Wood, EElementalType::Earth), 1.5f, 0.01f);
	TestNearlyEqual(TEXT("土克水1.5倍"), 
		UElementalCalculator::CalculateCounterMultiplier(EElementalType::Earth, EElementalType::Water), 1.5f, 0.01f);
	TestNearlyEqual(TEXT("水克火1.5倍"), 
		UElementalCalculator::CalculateCounterMultiplier(EElementalType::Water, EElementalType::Fire), 1.5f, 0.01f);
	TestNearlyEqual(TEXT("火克金1.5倍"), 
		UElementalCalculator::CalculateCounterMultiplier(EElementalType::Fire, EElementalType::Metal), 1.5f, 0.01f);
	
	// 被克制关系 - 返回0.5倍
	TestNearlyEqual(TEXT("木被金克0.5倍"), 
		UElementalCalculator::CalculateCounterMultiplier(EElementalType::Wood, EElementalType::Metal), 0.5f, 0.01f);
	TestNearlyEqual(TEXT("土被木克0.5倍"), 
		UElementalCalculator::CalculateCounterMultiplier(EElementalType::Earth, EElementalType::Wood), 0.5f, 0.01f);
	TestNearlyEqual(TEXT("水被土克0.5倍"), 
		UElementalCalculator::CalculateCounterMultiplier(EElementalType::Water, EElementalType::Earth), 0.5f, 0.01f);
	TestNearlyEqual(TEXT("火被水克0.5倍"), 
		UElementalCalculator::CalculateCounterMultiplier(EElementalType::Fire, EElementalType::Water), 0.5f, 0.01f);
	TestNearlyEqual(TEXT("金被火克0.5倍"), 
		UElementalCalculator::CalculateCounterMultiplier(EElementalType::Metal, EElementalType::Fire), 0.5f, 0.01f);
	
	// 无关系 - 返回1.0倍
	TestNearlyEqual(TEXT("同元素1.0倍"), 
		UElementalCalculator::CalculateCounterMultiplier(EElementalType::Fire, EElementalType::Fire), 1.0f, 0.01f);
	TestNearlyEqual(TEXT("金水无关系1.0倍"), 
		UElementalCalculator::CalculateCounterMultiplier(EElementalType::Metal, EElementalType::Water), 1.0f, 0.01f);
	TestNearlyEqual(TEXT("木火无关系1.0倍"), 
		UElementalCalculator::CalculateCounterMultiplier(EElementalType::Wood, EElementalType::Fire), 1.0f, 0.01f);
	
	// None元素 - 返回1.0倍
	TestNearlyEqual(TEXT("None元素无效果"), 
		UElementalCalculator::CalculateCounterMultiplier(EElementalType::None, EElementalType::Fire), 1.0f, 0.01f);
	TestNearlyEqual(TEXT("对None元素无效果"), 
		UElementalCalculator::CalculateCounterMultiplier(EElementalType::Water, EElementalType::None), 1.0f, 0.01f);
	
	return true;
}

/**
 * 元素无关系倍率测试
 */
ELEMENTAL_TEST(Combat.Elemental, NeutralElementMultiplier)
bool FNeutralElementMultiplierTest::RunTest(const FString& Parameters)
{
	// 同元素测试
	TestNearlyEqual(TEXT("金对金无克制"), 
		UElementalCalculator::CalculateCounterMultiplier(EElementalType::Metal, EElementalType::Metal), 
		1.0f, 0.01f);
	
	TestNearlyEqual(TEXT("水对水无克制"), 
		UElementalCalculator::CalculateCounterMultiplier(EElementalType::Water, EElementalType::Water), 
		1.0f, 0.01f);
	
	// 无直接关系测试
	TestNearlyEqual(TEXT("金对水无直接克制"), 
		UElementalCalculator::CalculateCounterMultiplier(EElementalType::Metal, EElementalType::Water), 
		1.0f, 0.01f);
	
	TestNearlyEqual(TEXT("木对火无直接克制"), 
		UElementalCalculator::CalculateCounterMultiplier(EElementalType::Wood, EElementalType::Fire), 
		1.0f, 0.01f);
	
	TestNearlyEqual(TEXT("土对金无直接克制"), 
		UElementalCalculator::CalculateCounterMultiplier(EElementalType::Earth, EElementalType::Metal), 
		1.0f, 0.01f);
	
	// None元素测试
	TestNearlyEqual(TEXT("None对任意元素无克制"), 
		UElementalCalculator::CalculateCounterMultiplier(EElementalType::None, EElementalType::Fire), 
		1.0f, 0.01f);
	
	TestNearlyEqual(TEXT("任意元素对None无克制"), 
		UElementalCalculator::CalculateCounterMultiplier(EElementalType::Water, EElementalType::None), 
		1.0f, 0.01f);
	
	return true;
}

/**
 * 元素伤害修正计算测试
 */
ELEMENTAL_TEST(Combat.Elemental, CalculateElementalDamageModifier)
bool FCalculateElementalDamageModifierTest::RunTest(const FString& Parameters)
{
	// 基础测试
	TestNearlyEqual(TEXT("克制增伤50%"), 
		UElementalCalculator::CalculateElementalDamageModifier(EElementalType::Water, EElementalType::Fire), 1.5f, 0.01f);
	TestNearlyEqual(TEXT("被克制减伤50%"), 
		UElementalCalculator::CalculateElementalDamageModifier(EElementalType::Fire, EElementalType::Water), 0.5f, 0.01f);
	TestNearlyEqual(TEXT("无关系不变"), 
		UElementalCalculator::CalculateElementalDamageModifier(EElementalType::Metal, EElementalType::Water), 1.0f, 0.01f);
	
	return true;
}

/**
 * 最终伤害计算测试
 */
ELEMENTAL_TEST(Combat.Elemental, CalculateFinalDamage)
bool FCalculateFinalDamageTest::RunTest(const FString& Parameters)
{
	const float BaseDamage = 100.0f;
	
	// 正常伤害计算
	TestNearlyEqual(TEXT("克制伤害150"), 
		UElementalCalculator::CalculateFinalDamage(BaseDamage, EElementalType::Water, EElementalType::Fire), 150.0f, 0.01f);
	TestNearlyEqual(TEXT("被克制伤害50"), 
		UElementalCalculator::CalculateFinalDamage(BaseDamage, EElementalType::Fire, EElementalType::Water), 50.0f, 0.01f);
	TestNearlyEqual(TEXT("无关系伤害100"), 
		UElementalCalculator::CalculateFinalDamage(BaseDamage, EElementalType::Metal, EElementalType::Water), 100.0f, 0.01f);
	
	// 边界条件
	TestNearlyEqual(TEXT("零伤害保持零"), 
		UElementalCalculator::CalculateFinalDamage(0.0f, EElementalType::Water, EElementalType::Fire), 0.0f, 0.01f);
	
	// 负数伤害处理
	float NegativeDamage = UElementalCalculator::CalculateFinalDamage(-50.0f, EElementalType::Water, EElementalType::Fire);
	TestTrue(TEXT("负伤害返回0"), NegativeDamage == 0.0f);
	
	// 极大值测试
	float LargeDamage = 1000000.0f;
	TestNearlyEqual(TEXT("大数值克制计算"), 
		UElementalCalculator::CalculateFinalDamage(LargeDamage, EElementalType::Metal, EElementalType::Wood), 
		LargeDamage * 1.5f, 1.0f);
	
	return true;
}