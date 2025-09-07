// Copyright 2025 guigui17f. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * 测试辅助宏定义
 */

// 简化测试声明的宏
#define ELEMENTAL_TEST(CategoryName, TestName) \
	IMPLEMENT_SIMPLE_AUTOMATION_TEST(F##TestName##Test, \
	"ElementalCombat."#CategoryName"."#TestName, \
	EAutomationTestFlags::EditorContext | \
	EAutomationTestFlags::ProductFilter)

// 带延迟的测试声明宏
#define ELEMENTAL_TEST_LATENT(CategoryName, TestName) \
	IMPLEMENT_COMPLEX_AUTOMATION_TEST(F##TestName##Test, \
	"ElementalCombat."#CategoryName"."#TestName, \
	EAutomationTestFlags::EditorContext | \
	EAutomationTestFlags::ProductFilter)

// 近似相等测试宏
#define TEST_NEARLY_EQUAL(Actual, Expected, Tolerance, Description) \
	TestNearlyEqual(Description, Actual, Expected, Tolerance)

// 测试Actor是否有效
#define TEST_ACTOR_VALID(Actor, Description) \
	TestTrue(Description, IsValid(Actor))

// 测试组件是否有效
#define TEST_COMPONENT_VALID(Component, Description) \
	TestTrue(Description, IsValid(Component))

/**
 * 测试辅助工具类
 */
class ELEMENTALCOMBATTESTS_API FTestHelpers
{
public:
	/** 生成随机位置向量 */
	static FVector GetRandomLocation(float Range = 1000.0f);
	
	/** 生成随机旋转 */
	static FRotator GetRandomRotation();
	
	/** 创建测试伤害数据 */
	struct FDamageTestData
	{
		float Damage;
		FVector ImpactLocation;
		FVector ImpulseDirection;
		float ImpulseStrength;
		
		FDamageTestData();
	};
	
	static FDamageTestData CreateTestDamageData(float InDamage = 10.0f);
	
	/** 等待指定帧数 */
	static void WaitForFrames(UWorld* World, int32 NumFrames);
	
	/** 等待指定时间（秒） */
	static void WaitForSeconds(UWorld* World, float Seconds);
	
	/** 验证浮点数组是否在误差范围内相等 */
	static bool AreFloatsNearlyEqual(const TArray<float>& Array1, const TArray<float>& Array2, float Tolerance = KINDA_SMALL_NUMBER);
	
	/** 清理测试产生的所有Actor */
	static void CleanupTestActors(UWorld* World);
	
	/** 获取性能计时器 */
	class FScopedTimer
	{
	public:
		FScopedTimer(const FString& InName);
		~FScopedTimer();
		
		double GetElapsedTime() const;
		
	private:
		FString Name;
		double StartTime;
	};
};

/**
 * 测试数据生成器
 */
class ELEMENTALCOMBATTESTS_API FTestDataGenerator
{
public:
	/** 生成随机HP值 */
	static float GenerateRandomHP(float Min = 1.0f, float Max = 100.0f);
	
	/** 生成随机伤害值 */
	static float GenerateRandomDamage(float Min = 1.0f, float Max = 50.0f);
	
	/** 生成随机元素类型（为未来五行系统准备） */
	static int32 GenerateRandomElementType();
	
	/** 生成测试用角色名称 */
	static FString GenerateTestCharacterName();
	
	/** 生成测试用敌人名称 */
	static FString GenerateTestEnemyName();
};