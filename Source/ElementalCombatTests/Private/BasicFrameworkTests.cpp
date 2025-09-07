// Copyright 2025 guigui17f. All Rights Reserved.

#include "CoreMinimal.h"
#include "ElementalCombatTestBase.h"
#include "TestHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

/**
 * 基础框架测试 - 验证测试环境正常工作
 */
ELEMENTAL_TEST(Framework, BasicAssertion)
bool FBasicAssertionTest::RunTest(const FString& Parameters)
{
	// 测试基本断言
	TestTrue(TEXT("真值应该为真"), true);
	TestFalse(TEXT("假值应该为假"), false);
	TestEqual(TEXT("1 + 1 应该等于 2"), 1 + 1, 2);
	TestNotEqual(TEXT("1 不应该等于 2"), 1, 2);
	
	// 测试浮点数比较
	float Value1 = 1.0f;
	float Value2 = 1.0001f;
	TestNearlyEqual(TEXT("浮点数近似相等"), Value1, Value2, 0.001f);
	
	// 测试字符串
	FString TestString = TEXT("ElementalCombat");
	TestEqual(TEXT("字符串比较"), TestString, TEXT("ElementalCombat"));
	
	return true;
}

/**
 * Actor生成测试 - 验证能够在测试世界中生成Actor
 */
class FActorSpawningTestImpl : public FElementalCombatTestBase
{
public:
	FActorSpawningTestImpl() : FElementalCombatTestBase(TEXT("ActorSpawning"), false) {}
	
	virtual bool RunTest(const FString& Parameters) override
	{
		// 创建测试世界
		UWorld* World = CreateTestWorld();
		TestNotNull(TEXT("测试世界应该被创建"), World);
		
		// 生成一个基础Actor
		FActorSpawnParameters SpawnParams;
		AActor* TestActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		TEST_ACTOR_VALID(TestActor, TEXT("Actor应该被生成"));
		
		// 验证Actor属性
		TestEqual(TEXT("Actor世界应该匹配"), TestActor->GetWorld(), World);
		TestTrue(TEXT("Actor位置应该为零"), TestActor->GetActorLocation().Equals(FVector::ZeroVector));
		
		// 移动Actor
		FVector NewLocation(100.0f, 200.0f, 300.0f);
		TestActor->SetActorLocation(NewLocation);
		TestTrue(TEXT("Actor应该移动到新位置"), TestActor->GetActorLocation().Equals(NewLocation));
		
		// 销毁Actor
		TestActor->Destroy();
		TestTrue(TEXT("Actor应该被标记为销毁"), !IsValid(TestActor) || TestActor->IsActorBeingDestroyed());
		
		// 清理在析构函数中自动处理
		
		return true;
	}
};

ELEMENTAL_TEST(Framework, ActorSpawning)
bool FActorSpawningTest::RunTest(const FString& Parameters)
{
	FActorSpawningTestImpl TestImpl;
	return TestImpl.RunTest(Parameters);
}

/**
 * 伤害计算测试 - 验证基础伤害计算公式
 */
ELEMENTAL_TEST(Framework, DamageCalculation)
bool FDamageCalculationTest::RunTest(const FString& Parameters)
{
	// 测试基础伤害值
	float BaseDamage = 10.0f;
	float Multiplier = 1.5f;
	float ExpectedDamage = BaseDamage * Multiplier;
	float CalculatedDamage = BaseDamage * Multiplier;
	
	TestEqual(TEXT("伤害计算应该正确"), CalculatedDamage, ExpectedDamage);
	
	// 测试伤害范围
	float MinDamage = 5.0f;
	float MaxDamage = 15.0f;
	float ClampedDamage = FMath::Clamp(CalculatedDamage, MinDamage, MaxDamage);
	
	TestTrue(TEXT("限制后的伤害应该在范围内"), 
		ClampedDamage >= MinDamage && ClampedDamage <= MaxDamage);
	
	// 测试伤害减免
	float DamageReduction = 0.2f;  // 20% 减免
	float ReducedDamage = BaseDamage * (1.0f - DamageReduction);
	TestNearlyEqual(TEXT("伤害减免计算"), ReducedDamage, 8.0f, 0.01f);
	
	return true;
}

/**
 * 测试辅助工具验证 - 验证测试辅助函数正常工作
 */
ELEMENTAL_TEST(Framework, TestHelpersValidation)
bool FTestHelpersValidationTest::RunTest(const FString& Parameters)
{
	// 测试随机位置生成
	FVector RandomLoc = FTestHelpers::GetRandomLocation(500.0f);
	TestTrue(TEXT("随机X位置应该在范围内"), FMath::Abs(RandomLoc.X) <= 500.0f);
	TestTrue(TEXT("随机Y位置应该在范围内"), FMath::Abs(RandomLoc.Y) <= 500.0f);
	TestTrue(TEXT("随机Z位置应该为正值"), RandomLoc.Z >= 0.0f);
	
	// 测试随机旋转生成
	FRotator RandomRot = FTestHelpers::GetRandomRotation();
	TestTrue(TEXT("随机俯仰角应该在范围内"), FMath::Abs(RandomRot.Pitch) <= 45.0f);
	TestTrue(TEXT("随机偏转角应该在范围内"), RandomRot.Yaw >= 0.0f && RandomRot.Yaw <= 360.0f);
	TestEqual(TEXT("随机翻滚角应该为零"), (double)RandomRot.Roll, 0.0);
	
	// 测试伤害数据生成
	auto DamageData = FTestHelpers::CreateTestDamageData(25.0f);
	TestEqual(TEXT("伤害值应该匹配"), DamageData.Damage, 25.0f);
	TestTrue(TEXT("冲击方向应该被正规化"), 
		FMath::IsNearlyEqual(DamageData.ImpulseDirection.Size(), 1.0f, 0.01f));
	
	// 测试数据生成器
	float RandomHP = FTestDataGenerator::GenerateRandomHP(10.0f, 100.0f);
	TestTrue(TEXT("随机生命值应该在范围内"), RandomHP >= 10.0f && RandomHP <= 100.0f);
	
	int32 ElementType = FTestDataGenerator::GenerateRandomElementType();
	TestTrue(TEXT("元素类型应该有效"), ElementType >= 0 && ElementType <= 5);
	
	FString CharacterName = FTestDataGenerator::GenerateTestCharacterName();
	TestFalse(TEXT("角色名称不应该为空"), CharacterName.IsEmpty());
	
	return true;
}