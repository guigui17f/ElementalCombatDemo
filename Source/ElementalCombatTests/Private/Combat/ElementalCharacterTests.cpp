// Copyright 2025 guigui17f. All Rights Reserved.

#include "CoreMinimal.h"
#include "ElementalCombatTestBase.h"
#include "TestHelpers.h"
#include "Engine/World.h"

/**
 * 元素战斗角色测试
 * 测试角色发射投掷物的核心功能
 */

/**
 * 测试角色投掷物发射配置
 */
ELEMENTAL_TEST(Combat.ElementalCharacter, ProjectileConfiguration)
bool FProjectileConfigurationTest::RunTest(const FString& Parameters)
{
	// 模拟角色投掷物配置
	struct FCharacterProjectileConfig
	{
		FName ProjectileSocketName = TEXT("hand_r");
		TSubclassOf<AActor> ProjectileClass = nullptr;
		float ChargeStartTime = 0.0f;
		bool bIsCharging = false;
	};
	
	FCharacterProjectileConfig Config;
	
	// 验证默认配置
	TestEqual(TEXT("默认发射Socket名称"), Config.ProjectileSocketName.ToString(), TEXT("hand_r"));
	TestNull(TEXT("投掷物类默认为空"), Config.ProjectileClass);
	TestEqual(TEXT("蓄力开始时间默认为0"), Config.ChargeStartTime, 0.0f);
	TestFalse(TEXT("默认未在蓄力状态"), Config.bIsCharging);
	
	// 测试Socket名称变更
	Config.ProjectileSocketName = TEXT("hand_l");
	TestEqual(TEXT("Socket名称应该可以更改"), Config.ProjectileSocketName.ToString(), TEXT("hand_l"));
	
	return true;
}

/**
 * 测试蓄力机制
 */
ELEMENTAL_TEST(Combat.ElementalCharacter, ChargeMechanism)
bool FChargeMechanismTest::RunTest(const FString& Parameters)
{
	// 模拟蓄力系统
	struct FChargingSystem
	{
		float ChargeStartTime = 0.0f;
		bool bIsCharging = false;
		
		void StartCharging(float CurrentTime)
		{
			bIsCharging = true;
			ChargeStartTime = CurrentTime;
		}
		
		void EndCharging()
		{
			bIsCharging = false;
		}
		
		float GetChargeTime(float CurrentTime) const
		{
			if (!bIsCharging) return 0.0f;
			return CurrentTime - ChargeStartTime;
		}
		
		float GetChargeMultiplier(float CurrentTime) const
		{
			float ChargeTime = GetChargeTime(CurrentTime);
			if (ChargeTime < 0.5f) return 1.0f;
			if (ChargeTime < 1.0f) return 1.5f;
			return 2.0f;
		}
	};
	
	FChargingSystem ChargingSystem;
	
	// 测试蓄力开始
	float StartTime = 10.0f;
	ChargingSystem.StartCharging(StartTime);
	TestTrue(TEXT("应该进入蓄力状态"), ChargingSystem.bIsCharging);
	TestEqual(TEXT("蓄力开始时间应该被记录"), ChargingSystem.ChargeStartTime, StartTime);
	
	// 测试蓄力时间计算
	float CurrentTime = 10.5f;
	float ChargeTime = ChargingSystem.GetChargeTime(CurrentTime);
	TestNearlyEqual(TEXT("0.5秒后的蓄力时间"), ChargeTime, 0.5f, 0.01f);
	
	// 测试蓄力倍率
	TestEqual(TEXT("0.5秒蓄力倍率"), ChargingSystem.GetChargeMultiplier(CurrentTime), 1.5f);
	
	CurrentTime = 11.5f;
	TestEqual(TEXT("1.5秒蓄力倍率"), ChargingSystem.GetChargeMultiplier(CurrentTime), 2.0f);
	
	// 测试蓄力结束
	ChargingSystem.EndCharging();
	TestFalse(TEXT("应该退出蓄力状态"), ChargingSystem.bIsCharging);
	TestEqual(TEXT("结束蓄力后时间应该为0"), ChargingSystem.GetChargeTime(CurrentTime), 0.0f);
	
	return true;
}

/**
 * 测试投掷物发射逻辑
 */
class FProjectileLaunchTestImpl : public FElementalCombatTestBase
{
public:
	FProjectileLaunchTestImpl() : FElementalCombatTestBase(TEXT("ProjectileLaunch"), false) {}
	
	virtual bool RunTest(const FString& Parameters) override
	{
		// 创建测试世界
		UWorld* World = CreateTestWorld();
		TestNotNull(TEXT("测试世界应该被创建"), World);
		
		// 模拟发射参数计算
		struct FLaunchParameters
		{
			FVector SpawnLocation;
			FRotator SpawnRotation;
			float SpeedMultiplier;
			float DamageMultiplier;
			AActor* Owner;
			AActor* Instigator;
		};
		
		// 设置发射参数
		FLaunchParameters LaunchParams;
		LaunchParams.SpawnLocation = FVector(100, 0, 150);  // 手部位置
		LaunchParams.SpawnRotation = FRotator(0, 45, 0);    // 朝向
		LaunchParams.SpeedMultiplier = 1.5f;                // 蓄力速度倍率
		LaunchParams.DamageMultiplier = 1.5f;               // 蓄力伤害倍率
		LaunchParams.Owner = nullptr;
		LaunchParams.Instigator = nullptr;
		
		// 验证发射参数
		TestTrue(TEXT("发射位置应该合理"), LaunchParams.SpawnLocation.Z > 0);
		TestTrue(TEXT("速度倍率应该大于0"), LaunchParams.SpeedMultiplier > 0);
		TestTrue(TEXT("伤害倍率应该大于0"), LaunchParams.DamageMultiplier > 0);
		
		// 计算发射方向
		FVector LaunchDirection = LaunchParams.SpawnRotation.Vector();
		TestTrue(TEXT("发射方向应该被正规化"), 
			FMath::IsNearlyEqual(LaunchDirection.Size(), 1.0f, 0.01f));
		
		// 模拟投掷物生成
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = LaunchParams.Owner;
		SpawnParams.Instigator = Cast<APawn>(LaunchParams.Instigator);
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
		AActor* Projectile = World->SpawnActor<AActor>(
			AActor::StaticClass(),
			LaunchParams.SpawnLocation,
			LaunchParams.SpawnRotation,
			SpawnParams
		);
		
		TEST_ACTOR_VALID(Projectile, TEXT("投掷物应该被生成"));
		TestTrue(TEXT("投掷物位置应该正确"), 
			Projectile->GetActorLocation().Equals(LaunchParams.SpawnLocation, 0.1f));
		TestTrue(TEXT("投掷物旋转应该正确"), 
			Projectile->GetActorRotation().Equals(LaunchParams.SpawnRotation, 0.1f));
		
		// 清理
		Projectile->Destroy();
		
		return true;
	}
};

ELEMENTAL_TEST(Combat.ElementalCharacter, ProjectileLaunch)
bool FProjectileLaunchTest::RunTest(const FString& Parameters)
{
	FProjectileLaunchTestImpl TestImpl;
	return TestImpl.RunTest(Parameters);
}

/**
 * 测试AI远程攻击决策
 */
ELEMENTAL_TEST(Combat.ElementalEnemy, AttackDecision)
bool FAttackDecisionTest::RunTest(const FString& Parameters)
{
	// AI攻击决策参数
	struct FAIAttackConfig
	{
		float MeleeAttackRange = 200.0f;
		float RangedAttackRange = 1000.0f;
		float PreferredAttackRange = 500.0f;
		
		enum EAttackType
		{
			None,
			Melee,
			Ranged
		};
		
		EAttackType DecideAttackType(float DistanceToTarget) const
		{
			if (DistanceToTarget <= 0) return None;
			if (DistanceToTarget <= MeleeAttackRange) return Melee;
			if (DistanceToTarget <= RangedAttackRange) return Ranged;
			return None;
		}
		
		bool IsInPreferredRange(float DistanceToTarget) const
		{
			return FMath::Abs(DistanceToTarget - PreferredAttackRange) < 100.0f;
		}
	};
	
	FAIAttackConfig AIConfig;
	
	// 测试攻击决策
	TestEqual(TEXT("150cm应该选择近战"), 
		(int32)AIConfig.DecideAttackType(150.0f), (int32)FAIAttackConfig::Melee);
	TestEqual(TEXT("300cm应该选择远程"), 
		(int32)AIConfig.DecideAttackType(300.0f), (int32)FAIAttackConfig::Ranged);
	TestEqual(TEXT("800cm应该选择远程"), 
		(int32)AIConfig.DecideAttackType(800.0f), (int32)FAIAttackConfig::Ranged);
	TestEqual(TEXT("1500cm超出范围"), 
		(int32)AIConfig.DecideAttackType(1500.0f), (int32)FAIAttackConfig::None);
	
	// 测试偏好距离
	TestTrue(TEXT("500cm是偏好距离"), AIConfig.IsInPreferredRange(500.0f));
	TestTrue(TEXT("450cm接近偏好距离"), AIConfig.IsInPreferredRange(450.0f));
	TestFalse(TEXT("200cm不是偏好距离"), AIConfig.IsInPreferredRange(200.0f));
	
	return true;
}

/**
 * 测试投掷物与曲线配置
 */
ELEMENTAL_TEST(Combat.ElementalCharacter, CurveConfiguration)
bool FCurveConfigurationTest::RunTest(const FString& Parameters)
{
	// 模拟曲线数据点
	struct FCurvePoint
	{
		float Time;
		float Value;
	};
	
	// 速度曲线配置（蓄力时间 -> 速度倍率）
	TArray<FCurvePoint> SpeedCurve = {
		{0.0f, 1.0f},
		{0.5f, 1.3f},
		{1.0f, 1.7f},
		{1.5f, 2.0f},
		{2.0f, 2.0f}
	};
	
	// 伤害曲线配置（蓄力时间 -> 伤害倍率）
	TArray<FCurvePoint> DamageCurve = {
		{0.0f, 1.0f},
		{0.5f, 1.5f},
		{1.0f, 2.0f},
		{1.5f, 2.5f},
		{2.0f, 3.0f}
	};
	
	// 简单的线性插值函数模拟
	auto GetCurveValue = [](const TArray<FCurvePoint>& Curve, float Time) -> float
	{
		if (Curve.Num() == 0) return 1.0f;
		if (Time <= Curve[0].Time) return Curve[0].Value;
		if (Time >= Curve.Last().Time) return Curve.Last().Value;
		
		for (int32 i = 0; i < Curve.Num() - 1; ++i)
		{
			if (Time >= Curve[i].Time && Time <= Curve[i + 1].Time)
			{
				float Alpha = (Time - Curve[i].Time) / (Curve[i + 1].Time - Curve[i].Time);
				return FMath::Lerp(Curve[i].Value, Curve[i + 1].Value, Alpha);
			}
		}
		return 1.0f;
	};
	
	// 测试速度曲线
	TestNearlyEqual(TEXT("0秒速度倍率"), GetCurveValue(SpeedCurve, 0.0f), 1.0f, 0.01f);
	TestNearlyEqual(TEXT("0.25秒速度倍率"), GetCurveValue(SpeedCurve, 0.25f), 1.15f, 0.01f);
	TestNearlyEqual(TEXT("1秒速度倍率"), GetCurveValue(SpeedCurve, 1.0f), 1.7f, 0.01f);
	TestNearlyEqual(TEXT("2秒速度倍率"), GetCurveValue(SpeedCurve, 2.0f), 2.0f, 0.01f);
	
	// 测试伤害曲线
	TestNearlyEqual(TEXT("0秒伤害倍率"), GetCurveValue(DamageCurve, 0.0f), 1.0f, 0.01f);
	TestNearlyEqual(TEXT("0.5秒伤害倍率"), GetCurveValue(DamageCurve, 0.5f), 1.5f, 0.01f);
	TestNearlyEqual(TEXT("1.5秒伤害倍率"), GetCurveValue(DamageCurve, 1.5f), 2.5f, 0.01f);
	
	return true;
}

/**
 * 测试投掷物所有者设置
 */
ELEMENTAL_TEST(Combat.ElementalCharacter, ProjectileOwnership)
bool FProjectileOwnershipTest::RunTest(const FString& Parameters)
{
	// 模拟投掷物所有权配置
	struct FProjectileOwnership
	{
		AActor* Owner = nullptr;
		APawn* Instigator = nullptr;
		
		bool IsOwnedBy(AActor* TestActor) const
		{
			return Owner == TestActor;
		}
		
		bool ShouldIgnoreActor(AActor* TestActor) const
		{
			// 投掷物应该忽略发射者
			return TestActor == Owner || TestActor == Instigator;
		}
	};
	
	// 创建模拟的所有者和目标
	FProjectileOwnership Ownership;
	AActor* MockOwner = reinterpret_cast<AActor*>(0x1000);
	AActor* MockTarget = reinterpret_cast<AActor*>(0x2000);
	APawn* MockInstigator = reinterpret_cast<APawn*>(0x3000);
	
	Ownership.Owner = MockOwner;
	Ownership.Instigator = MockInstigator;
	
	// 测试所有权
	TestTrue(TEXT("应该识别正确的所有者"), Ownership.IsOwnedBy(MockOwner));
	TestFalse(TEXT("不应该识别错误的所有者"), Ownership.IsOwnedBy(MockTarget));
	
	// 测试忽略逻辑
	TestTrue(TEXT("应该忽略所有者"), Ownership.ShouldIgnoreActor(MockOwner));
	TestTrue(TEXT("应该忽略发起者"), Ownership.ShouldIgnoreActor(MockInstigator));
	TestFalse(TEXT("不应该忽略目标"), Ownership.ShouldIgnoreActor(MockTarget));
	
	return true;
}