// Copyright 2025 guigui17f. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "AI/ElementalCombatEnemy.h"
#include "AI/ElementalCombatStateTreeTasks.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Character.h"
#include "AIController.h"
#include "Components/ActorComponent.h"

/**
 * 测试专用的Enemy类
 * 继承自ElementalCombatEnemy，仅用于测试
 */
class ATestableElementalCombatEnemy : public AElementalCombatEnemy
{
public:
	// 暴露攻击状态用于测试
	bool IsInAttackState() const { return bIsAttacking; }
	
	// 提供快速重置攻击状态的方法（仅测试使用）
	void ForceResetAttackState()
	{
		bIsAttacking = false;
		CurrentAttackType = EAIAttackType::None;
		
		// 清理可能存在的定时器
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearAllTimersForObject(this);
		}
	}
	
	// 模拟攻击完成（快速完成攻击序列）
	void SimulateAttackCompletion()
	{
		bIsAttacking = false;
		CurrentAttackType = EAIAttackType::None;
		
		// 如果有攻击完成委托，执行它
		if (OnAttackCompleted.IsBound())
		{
			OnAttackCompleted.Execute();
		}
	}
};

/**
 * 测试辅助函数
 */
namespace StateTreeTaskTestHelpers
{
	/**
	 * 创建测试用的ElementalCombatEnemy实例
	 * 使用测试专用子类以便更好地控制状态
	 */
	ATestableElementalCombatEnemy* CreateTestEnemy(UWorld* World)
	{
		if (!World)
		{
			return nullptr;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.Name = MakeUniqueObjectName(World, ATestableElementalCombatEnemy::StaticClass(), TEXT("TestEnemy"));
		
		ATestableElementalCombatEnemy* TestEnemy = World->SpawnActor<ATestableElementalCombatEnemy>(
			ATestableElementalCombatEnemy::StaticClass(),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnParams
		);

		return TestEnemy;
	}

	/**
	 * 创建测试用的目标角色
	 */
	ACharacter* CreateTestTarget(UWorld* World, const FVector& Location = FVector(1000, 0, 0))
	{
		if (!World)
		{
			return nullptr;
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.Name = MakeUniqueObjectName(World, ACharacter::StaticClass(), TEXT("TestTarget"));
		
		ACharacter* TestTarget = World->SpawnActor<ACharacter>(
			ACharacter::StaticClass(),
			Location,
			FRotator::ZeroRotator,
			SpawnParams
		);

		return TestTarget;
	}

	/**
	 * 验证攻击类型决策逻辑
	 */
	bool VerifyAttackDecision(ATestableElementalCombatEnemy* Enemy, float Distance, EAIAttackType Expected)
	{
		if (!Enemy)
		{
			return false;
		}

		EAIAttackType Result = Enemy->DecideAttackType(Distance);
		return Result == Expected;
	}

	/**
	 * 验证偏好范围判断逻辑
	 */
	bool VerifyRangeLogic(ATestableElementalCombatEnemy* Enemy, float Distance)
	{
		if (!Enemy)
		{
			return false;
		}

		return Enemy->IsInPreferredRange(Distance);
	}
}

/**
 * 测试攻击类型选择逻辑
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAttackTypeSelectionTest, 
	"ElementalCombat.AI.StateTreeTasks.AttackTypeSelection",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAttackTypeSelectionTest::RunTest(const FString& Parameters)
{
	// 创建测试世界
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	TestNotNull(TEXT("测试世界应该被创建"), World);
	
	if (!World)
	{
		return false;
	}

	// 初始化世界
	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
	WorldContext.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	World->BeginPlay();

	// 创建测试Enemy
	ATestableElementalCombatEnemy* TestEnemy = StateTreeTaskTestHelpers::CreateTestEnemy(World);
	TestNotNull(TEXT("测试Enemy应该被创建"), TestEnemy);

	if (!TestEnemy)
	{
		World->DestroyWorld(false);
		GEngine->DestroyWorldContext(World);
		return false;
	}

	// 测试近战距离 - 150cm应该选择近战
	bool bMeleeResult = StateTreeTaskTestHelpers::VerifyAttackDecision(
		TestEnemy, 150.0f, EAIAttackType::Melee);
	TestTrue(TEXT("150cm距离应该选择近战攻击"), bMeleeResult);

	// 测试远程距离 - 500cm应该选择远程
	bool bRangedResult = StateTreeTaskTestHelpers::VerifyAttackDecision(
		TestEnemy, 500.0f, EAIAttackType::Ranged);
	TestTrue(TEXT("500cm距离应该选择远程攻击"), bRangedResult);

	// 测试远程距离 - 800cm应该选择远程
	bool bRangedResult2 = StateTreeTaskTestHelpers::VerifyAttackDecision(
		TestEnemy, 800.0f, EAIAttackType::Ranged);
	TestTrue(TEXT("800cm距离应该选择远程攻击"), bRangedResult2);

	// 测试超出范围 - 1500cm应该无攻击
	bool bNoneResult = StateTreeTaskTestHelpers::VerifyAttackDecision(
		TestEnemy, 1500.0f, EAIAttackType::None);
	TestTrue(TEXT("1500cm距离应该无可用攻击"), bNoneResult);

	// 测试无效距离 - 负数应该无攻击
	bool bInvalidResult = StateTreeTaskTestHelpers::VerifyAttackDecision(
		TestEnemy, -100.0f, EAIAttackType::None);
	TestTrue(TEXT("负距离应该无可用攻击"), bInvalidResult);

	// 测试零距离 - 0应该无攻击
	bool bZeroResult = StateTreeTaskTestHelpers::VerifyAttackDecision(
		TestEnemy, 0.0f, EAIAttackType::None);
	TestTrue(TEXT("零距离应该无可用攻击"), bZeroResult);

	// 测试边界值 - 200cm刚好是近战边界
	EAIAttackType BoundaryResult = TestEnemy->DecideAttackType(200.0f);
	TestEqual(TEXT("200cm边界距离应该选择近战"), static_cast<int32>(BoundaryResult), static_cast<int32>(EAIAttackType::Melee));

	// 测试边界值 - 201cm刚好超过近战边界
	EAIAttackType BoundaryResult2 = TestEnemy->DecideAttackType(201.0f);
	TestEqual(TEXT("201cm距离应该选择远程"), static_cast<int32>(BoundaryResult2), static_cast<int32>(EAIAttackType::Ranged));

	// 清理
	TestEnemy->Destroy();
	World->DestroyWorld(false);
	GEngine->DestroyWorldContext(World);

	return true;
}

/**
 * 测试偏好范围判断逻辑
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPreferredRangeCheckTest, 
	"ElementalCombat.AI.StateTreeTasks.PreferredRangeCheck",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FPreferredRangeCheckTest::RunTest(const FString& Parameters)
{
	// 创建测试世界
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	TestNotNull(TEXT("测试世界应该被创建"), World);
	
	if (!World)
	{
		return false;
	}

	// 初始化世界
	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
	WorldContext.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	World->BeginPlay();

	// 创建测试Enemy
	ATestableElementalCombatEnemy* TestEnemy = StateTreeTaskTestHelpers::CreateTestEnemy(World);
	TestNotNull(TEXT("测试Enemy应该被创建"), TestEnemy);

	if (!TestEnemy)
	{
		World->DestroyWorld(false);
		GEngine->DestroyWorldContext(World);
		return false;
	}

	// 测试偏好距离(500cm)
	bool bPreferredDistance = StateTreeTaskTestHelpers::VerifyRangeLogic(TestEnemy, 500.0f);
	TestTrue(TEXT("500cm应该在偏好范围内"), bPreferredDistance);

	// 测试偏好范围内 - 450cm (500-50)
	bool bWithinRange1 = StateTreeTaskTestHelpers::VerifyRangeLogic(TestEnemy, 450.0f);
	TestTrue(TEXT("450cm应该在偏好范围内"), bWithinRange1);

	// 测试偏好范围内 - 550cm (500+50)
	bool bWithinRange2 = StateTreeTaskTestHelpers::VerifyRangeLogic(TestEnemy, 550.0f);
	TestTrue(TEXT("550cm应该在偏好范围内"), bWithinRange2);

	// 测试偏好范围外 - 200cm
	bool bOutOfRange1 = StateTreeTaskTestHelpers::VerifyRangeLogic(TestEnemy, 200.0f);
	TestFalse(TEXT("200cm应该在偏好范围外"), bOutOfRange1);

	// 测试偏好范围外 - 800cm
	bool bOutOfRange2 = StateTreeTaskTestHelpers::VerifyRangeLogic(TestEnemy, 800.0f);
	TestFalse(TEXT("800cm应该在偏好范围外"), bOutOfRange2);

	// 测试边界值 - 401cm (500-99，边界内)
	bool bBoundaryIn = StateTreeTaskTestHelpers::VerifyRangeLogic(TestEnemy, 401.0f);
	TestTrue(TEXT("401cm应该在偏好范围内"), bBoundaryIn);

	// 测试边界值 - 399cm (500-101，边界外)
	bool bBoundaryOut = StateTreeTaskTestHelpers::VerifyRangeLogic(TestEnemy, 399.0f);
	TestFalse(TEXT("399cm应该在偏好范围外"), bBoundaryOut);

	// 测试边界值 - 599cm (500+99，边界内)
	bool bBoundaryIn2 = StateTreeTaskTestHelpers::VerifyRangeLogic(TestEnemy, 599.0f);
	TestTrue(TEXT("599cm应该在偏好范围内"), bBoundaryIn2);

	// 测试边界值 - 601cm (500+101，边界外)
	bool bBoundaryOut2 = StateTreeTaskTestHelpers::VerifyRangeLogic(TestEnemy, 601.0f);
	TestFalse(TEXT("601cm应该在偏好范围外"), bBoundaryOut2);

	// 清理
	TestEnemy->Destroy();
	World->DestroyWorld(false);
	GEngine->DestroyWorldContext(World);

	return true;
}

/**
 * 测试远程攻击执行逻辑
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRangedAttackExecutionTest, 
	"ElementalCombat.AI.StateTreeTasks.RangedAttackExecution",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FRangedAttackExecutionTest::RunTest(const FString& Parameters)
{
	// 创建测试世界
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	TestNotNull(TEXT("测试世界应该被创建"), World);
	
	if (!World)
	{
		return false;
	}

	// 初始化世界
	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
	WorldContext.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	World->BeginPlay();

	// 创建测试Enemy
	ATestableElementalCombatEnemy* TestEnemy = StateTreeTaskTestHelpers::CreateTestEnemy(World);
	TestNotNull(TEXT("测试Enemy应该被创建"), TestEnemy);

	if (!TestEnemy)
	{
		World->DestroyWorld(false);
		GEngine->DestroyWorldContext(World);
		return false;
	}

	// 验证初始状态
	TestFalse(TEXT("初始不应该在攻击状态"), TestEnemy->IsInAttackState());
	TestEqual(TEXT("初始攻击类型应该是None"), 
		static_cast<int32>(TestEnemy->CurrentAttackType), static_cast<int32>(EAIAttackType::None));

	// 执行远程攻击
	TestEnemy->DoAIRangedAttack();

	// 验证攻击状态已设置
	TestTrue(TEXT("执行攻击后应该在攻击状态"), TestEnemy->IsInAttackState());
	TestEqual(TEXT("执行攻击后类型应该是Ranged"), 
		static_cast<int32>(TestEnemy->CurrentAttackType), static_cast<int32>(EAIAttackType::Ranged));

	// 模拟攻击完成
	TestEnemy->SimulateAttackCompletion();
	TestFalse(TEXT("攻击完成后不应该在攻击状态"), TestEnemy->IsInAttackState());

	// 清理
	TestEnemy->Destroy();
	World->DestroyWorld(false);
	GEngine->DestroyWorldContext(World);

	return true;
}

/**
 * 测试实例数据结构
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FInstanceDataStructureTest, 
	"ElementalCombat.AI.StateTreeTasks.InstanceDataStructure",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FInstanceDataStructureTest::RunTest(const FString& Parameters)
{
	// 测试基础实例数据
	FStateTreeElementalCombatInstanceData BaseData;
	TestNull(TEXT("基础实例数据Enemy应该默认为空"), BaseData.EnemyCharacter);

	// 测试攻击选择实例数据
	FStateTreeSelectAttackTypeInstanceData AttackData;
	TestNull(TEXT("攻击选择数据Enemy应该默认为空"), AttackData.EnemyCharacter);
	TestNull(TEXT("攻击选择数据目标应该默认为空"), AttackData.TargetPlayerCharacter);
	TestEqual(TEXT("攻击选择数据距离应该默认为0"), AttackData.DistanceToTarget, 0.0f);
	// 测试UE官方Output category字段 - 直接测试输出字段
	TestEqual(TEXT("攻击选择输出应该默认为None"), 
		static_cast<int32>(AttackData.SelectedAttackType), static_cast<int32>(EAIAttackType::None));

	// 测试范围维持实例数据
	FStateTreeMaintainPreferredRangeInstanceData RangeData;
	TestNull(TEXT("范围维持数据Enemy应该默认为空"), RangeData.EnemyCharacter);
	TestNull(TEXT("范围维持数据AI控制器应该默认为空"), RangeData.AIController);
	TestNull(TEXT("范围维持数据目标应该默认为空"), RangeData.TargetPlayerCharacter);
	TestTrue(TEXT("范围维持数据目标位置应该默认为零向量"), 
		RangeData.TargetPlayerLocation.IsZero());
	TestEqual(TEXT("范围维持数据距离应该默认为0"), RangeData.DistanceToTarget, 0.0f);
	// 测试UE官方Output category字段 - 直接测试输出字段
	TestFalse(TEXT("范围状态输出应该默认为false"), RangeData.InPreferredRange);

	return true;
}


/**
 * 测试边界条件和错误处理
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FErrorHandlingAndBoundariesTest, 
	"ElementalCombat.AI.StateTreeTasks.ErrorHandlingAndBoundaries",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FErrorHandlingAndBoundariesTest::RunTest(const FString& Parameters)
{
	// 创建测试世界
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	TestNotNull(TEXT("测试世界应该被创建"), World);
	
	if (!World)
	{
		return false;
	}

	// 初始化世界
	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
	WorldContext.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	World->BeginPlay();

	// 创建测试Enemy
	ATestableElementalCombatEnemy* TestEnemy = StateTreeTaskTestHelpers::CreateTestEnemy(World);
	TestNotNull(TEXT("测试Enemy应该被创建"), TestEnemy);

	if (!TestEnemy)
	{
		World->DestroyWorld(false);
		GEngine->DestroyWorldContext(World);
		return false;
	}

	// 测试极值情况
	
	// 1. 测试极小距离
	EAIAttackType VerySmallDistance = TestEnemy->DecideAttackType(0.001f);
	TestEqual(TEXT("极小距离应该选择近战"), 
		static_cast<int32>(VerySmallDistance), static_cast<int32>(EAIAttackType::Melee));

	// 2. 测试极大距离
	EAIAttackType VeryLargeDistance = TestEnemy->DecideAttackType(999999.0f);
	TestEqual(TEXT("极大距离应该无可用攻击"), 
		static_cast<int32>(VeryLargeDistance), static_cast<int32>(EAIAttackType::None));

	// 3. 测试范围判断的边界（偏好范围500±100，但使用<而非<=）
	bool bBoundaryInside = TestEnemy->IsInPreferredRange(401.0f);
	TestTrue(TEXT("401cm应该在偏好范围内"), bBoundaryInside);

	bool bBoundaryOutside = TestEnemy->IsInPreferredRange(399.0f);
	TestFalse(TEXT("399cm应该在偏好范围外"), bBoundaryOutside);

	// 4. 测试多次连续攻击调用和状态管理
	TestEnemy->ForceResetAttackState(); // 确保初始状态干净
	
	// 第一次攻击应该成功
	TestEnemy->DoAIRangedAttack();
	TestTrue(TEXT("第一次攻击应该成功"), TestEnemy->IsInAttackState());
	TestEqual(TEXT("第一次攻击类型应该是Ranged"),
		static_cast<int32>(TestEnemy->CurrentAttackType), 
		static_cast<int32>(EAIAttackType::Ranged));
	
	// 第二次攻击应该被阻挡（因为还在攻击状态）
	TestEnemy->DoAIRangedAttack();
	TestTrue(TEXT("第二次攻击时仍应该在攻击状态"), TestEnemy->IsInAttackState());
	
	// 强制重置后再次攻击应该成功
	TestEnemy->ForceResetAttackState();
	TestFalse(TEXT("重置后不应该在攻击状态"), TestEnemy->IsInAttackState());
	
	TestEnemy->DoAIRangedAttack();
	TestTrue(TEXT("重置后的攻击应该成功"), TestEnemy->IsInAttackState());

	// 清理
	TestEnemy->Destroy();
	World->DestroyWorld(false);
	GEngine->DestroyWorldContext(World);

	return true;
}

/**
 * 测试攻击状态管理
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FAttackStateManagementTest, 
	"ElementalCombat.AI.StateTreeTasks.AttackStateManagement",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAttackStateManagementTest::RunTest(const FString& Parameters)
{
	// 创建测试世界
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
	TestNotNull(TEXT("测试世界应该被创建"), World);
	
	if (!World)
	{
		return false;
	}

	// 初始化世界
	FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
	WorldContext.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	World->BeginPlay();

	// 创建测试Enemy
	ATestableElementalCombatEnemy* TestEnemy = StateTreeTaskTestHelpers::CreateTestEnemy(World);
	TestNotNull(TEXT("测试Enemy应该被创建"), TestEnemy);

	if (!TestEnemy)
	{
		World->DestroyWorld(false);
		GEngine->DestroyWorldContext(World);
		return false;
	}
	
	// 测试1：攻击状态锁定
	TestFalse(TEXT("初始不应该被锁定"), TestEnemy->IsInAttackState());
	
	TestEnemy->DoAIRangedAttack();
	TestTrue(TEXT("攻击后应该被锁定"), TestEnemy->IsInAttackState());
	
	// 测试2：攻击状态防重入
	EAIAttackType FirstAttackType = TestEnemy->CurrentAttackType;
	TestEnemy->DoAIRangedAttack(); // 应该被忽略
	TestTrue(TEXT("重复攻击仍应该被锁定"), TestEnemy->IsInAttackState());
	TestEqual(TEXT("攻击类型不应该改变"), 
		static_cast<int32>(TestEnemy->CurrentAttackType), 
		static_cast<int32>(FirstAttackType));
	
	// 测试3：模拟攻击完成
	TestEnemy->SimulateAttackCompletion();
	TestFalse(TEXT("模拟完成后不应该被锁定"), TestEnemy->IsInAttackState());
	TestEqual(TEXT("完成后类型应该是None"),
		static_cast<int32>(TestEnemy->CurrentAttackType), 
		static_cast<int32>(EAIAttackType::None));
	
	// 测试4：强制重置
	TestEnemy->DoAIRangedAttack();
	TestTrue(TEXT("再次攻击应该成功"), TestEnemy->IsInAttackState());
	
	TestEnemy->ForceResetAttackState();
	TestFalse(TEXT("强制重置后不应该被锁定"), TestEnemy->IsInAttackState());
	TestEqual(TEXT("强制重置后类型应该是None"),
		static_cast<int32>(TestEnemy->CurrentAttackType), 
		static_cast<int32>(EAIAttackType::None));
	
	// 测试5：重置后可以再次攻击
	TestEnemy->DoAIRangedAttack();
	TestTrue(TEXT("重置后再次攻击应该成功"), TestEnemy->IsInAttackState());
	TestEqual(TEXT("重置后攻击类型应该是Ranged"),
		static_cast<int32>(TestEnemy->CurrentAttackType), 
		static_cast<int32>(EAIAttackType::Ranged));
	
	// 清理
	TestEnemy->Destroy();
	World->DestroyWorld(false);
	GEngine->DestroyWorldContext(World);
	
	return true;
}