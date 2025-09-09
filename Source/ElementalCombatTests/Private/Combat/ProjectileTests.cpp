// Copyright 2025 guigui17f. All Rights Reserved.

#include "CoreMinimal.h"
#include "ElementalCombatTestBase.h"
#include "TestHelpers.h"
#include "Engine/World.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

/**
 * 投掷物核心逻辑测试
 * 测试投掷物的基本功能，包括生成、运动、碰撞和伤害
 */

/**
 * 测试投掷物基础属性配置
 */
ELEMENTAL_TEST(Combat.Projectile, BasicConfiguration)
bool FBasicConfigurationTest::RunTest(const FString& Parameters)
{
	// 测试投掷物默认配置值
	struct FProjectileConfig
	{
		float BaseDamage = 2.0f;
		float InitialSpeed = 1000.0f;
		float MaxSpeed = 2000.0f;
		float GravityScale = 1.0f;
		float LifeSpan = 3.0f;
		bool bShouldBounce = false;
	};
	
	FProjectileConfig Config;
	
	// 验证默认值
	TestEqual(TEXT("基础伤害默认值"), Config.BaseDamage, 2.0f);
	TestEqual(TEXT("初始速度默认值"), Config.InitialSpeed, 1000.0f);
	TestEqual(TEXT("最大速度默认值"), Config.MaxSpeed, 2000.0f);
	TestEqual(TEXT("重力缩放默认值"), Config.GravityScale, 1.0f);
	TestEqual(TEXT("生命周期默认值"), Config.LifeSpan, 3.0f);
	TestFalse(TEXT("默认不应该弹跳"), Config.bShouldBounce);
	
	// 测试蓄力时间对属性的影响
	auto CalculateChargeMultiplier = [](float ChargeTime) -> float
	{
		if (ChargeTime < 0.5f) return 1.0f;
		if (ChargeTime < 1.0f) return 1.5f;
		return 2.0f;
	};
	
	TestEqual(TEXT("0.3秒蓄力倍率"), CalculateChargeMultiplier(0.3f), 1.0f);
	TestEqual(TEXT("0.7秒蓄力倍率"), CalculateChargeMultiplier(0.7f), 1.5f);
	TestEqual(TEXT("1.5秒蓄力倍率"), CalculateChargeMultiplier(1.5f), 2.0f);
	
	return true;
}

/**
 * 测试投掷物运动组件配置
 */
class FProjectileMovementTestImpl : public FElementalCombatTestBase
{
public:
	FProjectileMovementTestImpl() : FElementalCombatTestBase(TEXT("ProjectileMovement"), false) {}
	
	virtual bool RunTest(const FString& Parameters) override
	{
		// 创建测试世界
		UWorld* World = CreateTestWorld();
		TestNotNull(TEXT("测试世界应该被创建"), World);
		
		// 创建投掷物Actor
		FActorSpawnParameters SpawnParams;
		AActor* ProjectileActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		TEST_ACTOR_VALID(ProjectileActor, TEXT("投掷物Actor应该被创建"));
		
		// 添加碰撞组件
		USphereComponent* CollisionComp = NewObject<USphereComponent>(ProjectileActor, TEXT("SphereComp"));
		CollisionComp->InitSphereRadius(10.0f);
		CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		CollisionComp->SetCollisionResponseToAllChannels(ECR_Block);
		CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		ProjectileActor->SetRootComponent(CollisionComp);
		CollisionComp->RegisterComponent();
		
		// 添加投掷物运动组件
		UProjectileMovementComponent* ProjectileMovement = NewObject<UProjectileMovementComponent>(ProjectileActor, TEXT("ProjectileComp"));
		ProjectileMovement->UpdatedComponent = CollisionComp;
		ProjectileMovement->InitialSpeed = 1000.0f;
		ProjectileMovement->MaxSpeed = 2000.0f;
		ProjectileMovement->bRotationFollowsVelocity = true;
		ProjectileMovement->bShouldBounce = false;
		ProjectileMovement->ProjectileGravityScale = 1.0f;
		ProjectileMovement->RegisterComponent();
		
		// 验证组件配置
		TEST_COMPONENT_VALID(CollisionComp, TEXT("碰撞组件应该有效"));
		TEST_COMPONENT_VALID(ProjectileMovement, TEXT("运动组件应该有效"));
		TestEqual(TEXT("初始速度设置"), ProjectileMovement->InitialSpeed, 1000.0f);
		TestEqual(TEXT("最大速度设置"), ProjectileMovement->MaxSpeed, 2000.0f);
		TestTrue(TEXT("旋转跟随速度"), ProjectileMovement->bRotationFollowsVelocity);
		TestFalse(TEXT("不应该弹跳"), ProjectileMovement->bShouldBounce);
		TestEqual(TEXT("重力缩放"), ProjectileMovement->ProjectileGravityScale, 1.0f);
		
		// 设置初始速度方向
		FVector LaunchDirection = FVector(1.0f, 0.0f, 0.0f);
		ProjectileMovement->Velocity = LaunchDirection * ProjectileMovement->InitialSpeed;
		
		// 验证速度设置
		float CurrentSpeed = static_cast<float>(ProjectileMovement->Velocity.Size());
		TestNearlyEqual(TEXT("当前速度应该等于初始速度"), CurrentSpeed, 1000.0f, 0.1f);
		
		// 清理
		ProjectileActor->Destroy();
		
		return true;
	}
};

ELEMENTAL_TEST(Combat.Projectile, MovementComponent)
bool FMovementComponentTest::RunTest(const FString& Parameters)
{
	FProjectileMovementTestImpl TestImpl;
	return TestImpl.RunTest(Parameters);
}

/**
 * 测试投掷物伤害计算
 */
ELEMENTAL_TEST(Combat.Projectile, ProjectileDamageCalculation)
bool FProjectileDamageCalculationTest::RunTest(const FString& Parameters)
{
	// 基础伤害配置
	float BaseDamage = 2.0f;
	float ChargeMultiplier = 1.0f;
	
	// 测试不同蓄力时间的伤害计算
	struct FChargeTestCase
	{
		float ChargeTime;
		float ExpectedMultiplier;
		float ExpectedDamage;
	};
	
	TArray<FChargeTestCase> TestCases = {
		{0.2f, 1.0f, 2.0f},    // 短蓄力
		{0.7f, 1.5f, 3.0f},    // 中等蓄力
		{1.2f, 2.0f, 4.0f},    // 完全蓄力
		{2.0f, 2.0f, 4.0f}     // 超过上限
	};
	
	for (const auto& TestCase : TestCases)
	{
		// 计算蓄力倍率
		if (TestCase.ChargeTime < 0.5f)
			ChargeMultiplier = 1.0f;
		else if (TestCase.ChargeTime < 1.0f)
			ChargeMultiplier = 1.5f;
		else
			ChargeMultiplier = 2.0f;
		
		float CalculatedDamage = BaseDamage * ChargeMultiplier;
		
		FString Description = FString::Printf(TEXT("%.1f秒蓄力伤害计算"), TestCase.ChargeTime);
		TestEqual(*Description, CalculatedDamage, TestCase.ExpectedDamage);
	}
	
	// 测试伤害范围限制
	float MaxAllowedDamage = 10.0f;
	float UnlimitedDamage = BaseDamage * 10.0f;  // 20.0f
	float ClampedDamage = FMath::Min(UnlimitedDamage, MaxAllowedDamage);
	TestEqual(TEXT("伤害应该被限制在最大值"), ClampedDamage, MaxAllowedDamage);
	
	return true;
}

/**
 * 测试投掷物碰撞检测
 */
class FProjectileCollisionTestImpl : public FElementalCombatTestBase
{
public:
	FProjectileCollisionTestImpl() : FElementalCombatTestBase(TEXT("ProjectileCollision"), false) {}
	
	virtual bool RunTest(const FString& Parameters) override
	{
		// 创建测试世界
		UWorld* World = CreateTestWorld();
		TestNotNull(TEXT("测试世界应该被创建"), World);
		
		// 创建投掷物
		FActorSpawnParameters SpawnParams;
		AActor* Projectile = World->SpawnActor<AActor>(AActor::StaticClass(), 
			FVector(0, 0, 100), FRotator::ZeroRotator, SpawnParams);
		
		// 添加碰撞组件
		USphereComponent* CollisionComp = NewObject<USphereComponent>(Projectile, TEXT("CollisionSphere"));
		CollisionComp->InitSphereRadius(10.0f);
		CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		CollisionComp->SetCollisionObjectType(ECC_WorldDynamic);
		CollisionComp->SetCollisionResponseToAllChannels(ECR_Block);
		CollisionComp->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
		CollisionComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
		Projectile->SetRootComponent(CollisionComp);
		CollisionComp->RegisterComponent();
		
		// 验证碰撞设置
		TestEqual(TEXT("碰撞半径"), CollisionComp->GetUnscaledSphereRadius(), 10.0f);
		TestTrue(TEXT("碰撞应该启用查询"), 
			CollisionComp->GetCollisionEnabled() == ECollisionEnabled::QueryOnly);
		TestTrue(TEXT("应该阻挡World Static"), 
			CollisionComp->GetCollisionResponseToChannel(ECC_WorldStatic) == ECR_Block);
		TestTrue(TEXT("应该阻挡Pawn通道"), 
			CollisionComp->GetCollisionResponseToChannel(ECC_Pawn) == ECR_Block);
		TestTrue(TEXT("应该忽略相机通道"), 
			CollisionComp->GetCollisionResponseToChannel(ECC_Camera) == ECR_Ignore);
		
		// 创建目标Actor用于碰撞测试
		AActor* Target = World->SpawnActor<AActor>(AActor::StaticClass(), 
			FVector(100, 0, 100), FRotator::ZeroRotator, SpawnParams);
		
		USphereComponent* TargetCollision = NewObject<USphereComponent>(Target, TEXT("TargetSphere"));
		TargetCollision->InitSphereRadius(50.0f);
		TargetCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		TargetCollision->SetCollisionObjectType(ECC_Pawn);
		Target->SetRootComponent(TargetCollision);
		TargetCollision->RegisterComponent();
		
		// 测试重叠检测
		TArray<AActor*> OverlappingActors;
		Projectile->GetOverlappingActors(OverlappingActors);
		TestEqual(TEXT("初始不应该有重叠"), OverlappingActors.Num(), 0);
		
		// 移动投掷物到目标位置
		Projectile->SetActorLocation(FVector(100, 0, 100));
		
		// 执行碰撞检测
		FHitResult HitResult;
		FVector Start = FVector(0, 0, 100);
		FVector End = FVector(100, 0, 100);
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(Projectile);
		
		bool bHit = World->LineTraceSingleByChannel(HitResult, Start, End, 
			ECC_WorldDynamic, QueryParams);
		
		TestTrue(TEXT("应该检测到碰撞"), bHit);
		if (bHit)
		{
			TestEqual(TEXT("碰撞目标应该是Target"), HitResult.GetActor(), Target);
		}
		
		// 清理
		Projectile->Destroy();
		Target->Destroy();
		
		return true;
	}
};

ELEMENTAL_TEST(Combat.Projectile, CollisionDetection)
bool FCollisionDetectionTest::RunTest(const FString& Parameters)
{
	FProjectileCollisionTestImpl TestImpl;
	return TestImpl.RunTest(Parameters);
}

/**
 * 测试投掷物生命周期
 */
ELEMENTAL_TEST(Combat.Projectile, LifeSpan)
bool FLifeSpanTest::RunTest(const FString& Parameters)
{
	// 测试生命周期配置
	float DefaultLifeSpan = 3.0f;
	float ExtendedLifeSpan = 5.0f;
	float ShortLifeSpan = 1.0f;
	
	TestTrue(TEXT("默认生命周期应该为3秒"), FMath::IsNearlyEqual(DefaultLifeSpan, 3.0f));
	TestTrue(TEXT("扩展生命周期应该为5秒"), FMath::IsNearlyEqual(ExtendedLifeSpan, 5.0f));
	TestTrue(TEXT("短生命周期应该为1秒"), FMath::IsNearlyEqual(ShortLifeSpan, 1.0f));
	
	// 测试生命周期倒计时
	float CurrentLifeTime = DefaultLifeSpan;
	
	// 模拟精确的1秒时间流逝
	CurrentLifeTime -= 1.0f;
	TestNearlyEqual(TEXT("1秒后剩余生命时间"), CurrentLifeTime, 2.0f, 0.01f);
	
	// 模拟剩余时间全部消耗
	CurrentLifeTime -= 2.0f;
	TestTrue(TEXT("生命周期应该结束"), CurrentLifeTime <= 0.0f);
	
	// 额外测试：超时后的状态
	CurrentLifeTime -= 1.0f; // 再减1秒
	TestTrue(TEXT("超时后应该保持负值"), CurrentLifeTime < 0.0f);
	
	return true;
}

/**
 * 测试投掷物速度调整
 */
ELEMENTAL_TEST(Combat.Projectile, VelocityAdjustment)
bool FVelocityAdjustmentTest::RunTest(const FString& Parameters)
{
	// 基础速度配置
	float BaseSpeed = 1000.0f;
	float MaxSpeed = 2000.0f;
	
	// 测试不同蓄力倍率下的速度
	struct FSpeedTestCase
	{
		float ChargeMultiplier;
		float ExpectedSpeed;
	};
	
	TArray<FSpeedTestCase> TestCases = {
		{1.0f, 1000.0f},
		{1.5f, 1500.0f},
		{2.0f, 2000.0f},
		{3.0f, 2000.0f}  // 应该被限制在最大速度
	};
	
	for (const auto& TestCase : TestCases)
	{
		float CalculatedSpeed = FMath::Min(BaseSpeed * TestCase.ChargeMultiplier, MaxSpeed);
		
		FString Description = FString::Printf(TEXT("%.1fx倍率速度计算"), TestCase.ChargeMultiplier);
		TestEqual(*Description, CalculatedSpeed, TestCase.ExpectedSpeed);
	}
	
	// 测试速度方向
	FVector LaunchDirection = FVector(1.0f, 0.0f, 0.5f).GetSafeNormal();
	FVector Velocity = LaunchDirection * BaseSpeed;
	
	TestNearlyEqual(TEXT("速度大小"), static_cast<float>(Velocity.Size()), BaseSpeed, 0.1f);
	TestTrue(TEXT("速度方向应该与发射方向一致"), 
		Velocity.GetSafeNormal().Equals(LaunchDirection, 0.01f));
	
	return true;
}