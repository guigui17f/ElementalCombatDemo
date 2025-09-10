// Copyright 2025 guigui17f. All Rights Reserved.

#include "CoreMinimal.h"
#include "TestHelpers.h"
#include "Combat/Elemental/ElementalTypes.h"

/**
 * 元素类型枚举测试
 */
ELEMENTAL_TEST(Combat.Elemental, ElementTypeEnum)
bool FElementTypeEnumTest::RunTest(const FString& Parameters)
{
	// 验证元素类型值
	TestEqual(TEXT("None元素值为0"), static_cast<int32>(EElementalType::None), 0);
	TestEqual(TEXT("金元素值为1"), static_cast<int32>(EElementalType::Metal), 1);
	TestEqual(TEXT("木元素值为2"), static_cast<int32>(EElementalType::Wood), 2);
	TestEqual(TEXT("水元素值为3"), static_cast<int32>(EElementalType::Water), 3);
	TestEqual(TEXT("火元素值为4"), static_cast<int32>(EElementalType::Fire), 4);
	TestEqual(TEXT("土元素值为5"), static_cast<int32>(EElementalType::Earth), 5);
	
	// 验证元素数量
	TestEqual(TEXT("元素总数为6（包含None）"), 6, 6);
	
	return true;
}

/**
 * 元素效果数据结构测试
 */
ELEMENTAL_TEST(Combat.Elemental, ElementalEffectDataStruct)
bool FElementalEffectDataStructTest::RunTest(const FString& Parameters)
{
	FElementalEffectData TestData;
	
	// 测试默认值
	TestNearlyEqual(TEXT("默认伤害倍率为1.0"), TestData.DamageMultiplier, 1.0f, 0.01f);
	TestNearlyEqual(TEXT("默认吸血为0"), TestData.LifeStealPercentage, 0.0f, 0.01f);
	TestNearlyEqual(TEXT("默认减速为0"), TestData.SlowPercentage, 0.0f, 0.01f);
	TestNearlyEqual(TEXT("默认减速时间为0"), TestData.SlowDuration, 0.0f, 0.01f);
	TestNearlyEqual(TEXT("默认DOT伤害为0"), TestData.DotDamage, 0.0f, 0.01f);
	TestNearlyEqual(TEXT("默认DOT间隔为1秒"), TestData.DotTickInterval, 1.0f, 0.01f);
	TestNearlyEqual(TEXT("默认DOT持续时间为0"), TestData.DotDuration, 0.0f, 0.01f);
	TestNearlyEqual(TEXT("默认减伤为0"), TestData.DamageReduction, 0.0f, 0.01f);
	TestNull(TEXT("默认投掷物类为空"), TestData.ProjectileClass);
	
	// 测试赋值
	TestData.DamageMultiplier = 1.5f;
	TestData.LifeStealPercentage = 0.3f;
	TestData.SlowPercentage = 0.5f;
	TestData.SlowDuration = 3.0f;
	TestData.DotDamage = 10.0f;
	TestData.DotTickInterval = 0.5f;
	TestData.DotDuration = 5.0f;
	TestData.DamageReduction = 0.3f;
	
	TestNearlyEqual(TEXT("伤害倍率赋值"), TestData.DamageMultiplier, 1.5f, 0.01f);
	TestNearlyEqual(TEXT("吸血赋值"), TestData.LifeStealPercentage, 0.3f, 0.01f);
	TestNearlyEqual(TEXT("减速赋值"), TestData.SlowPercentage, 0.5f, 0.01f);
	TestNearlyEqual(TEXT("减速时间赋值"), TestData.SlowDuration, 3.0f, 0.01f);
	TestNearlyEqual(TEXT("DOT伤害赋值"), TestData.DotDamage, 10.0f, 0.01f);
	TestNearlyEqual(TEXT("DOT间隔赋值"), TestData.DotTickInterval, 0.5f, 0.01f);
	TestNearlyEqual(TEXT("DOT持续时间赋值"), TestData.DotDuration, 5.0f, 0.01f);
	TestNearlyEqual(TEXT("减伤赋值"), TestData.DamageReduction, 0.3f, 0.01f);
	
	return true;
}

/**
 * 元素相克数据结构测试
 */
ELEMENTAL_TEST(Combat.Elemental, ElementalCounterDataStruct)
bool FElementalCounterDataStructTest::RunTest(const FString& Parameters)
{
	FElementalCounterData TestCounter;
	
	// 测试默认值
	TestEqual(TEXT("默认被克制元素为None"), static_cast<int32>(TestCounter.CounteredElement), static_cast<int32>(EElementalType::None));
	TestNearlyEqual(TEXT("默认效果倍率为1.0"), TestCounter.EffectMultiplier, 1.0f, 0.01f);
	
	// 测试赋值
	TestCounter.CounteredElement = EElementalType::Fire;
	TestCounter.EffectMultiplier = 1.5f;
	
	TestEqual(TEXT("被克制元素赋值"), static_cast<int32>(TestCounter.CounteredElement), static_cast<int32>(EElementalType::Fire));
	TestNearlyEqual(TEXT("效果倍率赋值"), TestCounter.EffectMultiplier, 1.5f, 0.01f);
	
	return true;
}

/**
 * 元素关系数据结构测试
 */
ELEMENTAL_TEST(Combat.Elemental, ElementalRelationshipStruct)
bool FElementalRelationshipStructTest::RunTest(const FString& Parameters)
{
	FElementalRelationship TestRelation;
	
	// 测试默认值
	TestEqual(TEXT("默认元素为None"), static_cast<int32>(TestRelation.Element), static_cast<int32>(EElementalType::None));
	TestEqual(TEXT("默认克制列表为空"), TestRelation.Counters.Num(), 0);
	
	// 测试添加克制关系
	TestRelation.Element = EElementalType::Water;
	
	FElementalCounterData Counter1;
	Counter1.CounteredElement = EElementalType::Fire;
	Counter1.EffectMultiplier = 1.5f;
	TestRelation.Counters.Add(Counter1);
	
	TestEqual(TEXT("元素设置为水"), static_cast<int32>(TestRelation.Element), static_cast<int32>(EElementalType::Water));
	TestEqual(TEXT("克制列表有1个元素"), TestRelation.Counters.Num(), 1);
	TestEqual(TEXT("克制火元素"), static_cast<int32>(TestRelation.Counters[0].CounteredElement), static_cast<int32>(EElementalType::Fire));
	TestNearlyEqual(TEXT("克制倍率1.5"), TestRelation.Counters[0].EffectMultiplier, 1.5f, 0.01f);
	
	// 测试多个克制关系
	FElementalCounterData Counter2;
	Counter2.CounteredElement = EElementalType::Metal;
	Counter2.EffectMultiplier = 1.2f;
	TestRelation.Counters.Add(Counter2);
	
	TestEqual(TEXT("克制列表有2个元素"), TestRelation.Counters.Num(), 2);
	
	return true;
}