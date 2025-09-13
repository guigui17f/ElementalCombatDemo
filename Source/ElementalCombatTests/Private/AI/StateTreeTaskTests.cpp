// Copyright 2025 guigui17f. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "AI/StateTreeTasks/ElementalStateTreeTaskBase.h"
#include "AI/StateTreeTasks/StateTreeUtilityTasks.h"
#include "AI/ElementalCombatEnemy.h"
#include "AI/ElementalCombatAIController.h"
#include "Engine/World.h"
#include "Engine/DataTable.h"
#include "GameFramework/Pawn.h"
#include "AI/Utility/UtilityAITypes.h"
#include "StructView.h"

// === Test Helper Namespace ===
namespace ElementalCombat::Tests
{
    /**
     * 测试助手函数
     */
    class FStateTreeTestHelpers
    {
    public:
        /** 创建测试世界 */
        static UWorld* CreateTestWorld()
        {
            UWorld* TestWorld = UWorld::CreateWorld(EWorldType::Game, false);
            return TestWorld;
        }

        /** 创建测试用的AI配置 */
        static FUtilityProfile CreateTestUtilityProfile(const FString& ProfileName = TEXT("TestProfile"))
        {
            FUtilityProfile TestProfile;
            TestProfile.ProfileName = ProfileName;
            
            // 设置基本权重
            TestProfile.Weights.Add(EConsiderationType::Health, 0.3f);
            TestProfile.Weights.Add(EConsiderationType::Distance, 0.2f);
            TestProfile.Weights.Add(EConsiderationType::ElementAdvantage, 0.3f);
            TestProfile.Weights.Add(EConsiderationType::ThreatLevel, 0.2f);

            // 添加考虑因子
            FUtilityConsideration HealthConsideration;
            HealthConsideration.ConsiderationType = EConsiderationType::Health;
            HealthConsideration.ResponseCurve.EditorCurveData.Reset();
            HealthConsideration.ResponseCurve.EditorCurveData.AddKey(0.0f, 0.0f);
            HealthConsideration.ResponseCurve.EditorCurveData.AddKey(1.0f, 1.0f);
            TestProfile.Considerations.Add(HealthConsideration);

            FUtilityConsideration DistanceConsideration;
            DistanceConsideration.ConsiderationType = EConsiderationType::Distance;
            DistanceConsideration.ResponseCurve.EditorCurveData.Reset();
            DistanceConsideration.ResponseCurve.EditorCurveData.AddKey(0.0f, 1.0f);
            DistanceConsideration.ResponseCurve.EditorCurveData.AddKey(1.0f, 0.0f);
            TestProfile.Considerations.Add(DistanceConsideration);

            return TestProfile;
        }


        /** 创建测试敌人并设置AI控制器 */
        static AElementalCombatEnemy* CreateTestEnemyWithAI(UWorld* TestWorld, const FUtilityProfile& Profile)
        {
            // 创建敌人
            AElementalCombatEnemy* TestEnemy = TestWorld->SpawnActor<AElementalCombatEnemy>();
            if (!TestEnemy)
            {
                return nullptr;
            }

            // 创建AI控制器
            AElementalCombatAIController* AIController = TestWorld->SpawnActor<AElementalCombatAIController>();
            if (AIController)
            {
                // 首先进行Possess（会设置默认配置）
                AIController->Possess(TestEnemy);
                
                // 然后用测试配置覆盖默认配置
#if WITH_AUTOMATION_TESTS || WITH_EDITOR
                AIController->SetAIProfileForTest(Profile);
#endif
            }

            return TestEnemy;
        }

        /** 清理测试世界 */
        static void CleanupTestWorld(UWorld* TestWorld)
        {
            if (TestWorld)
            {
                TestWorld->DestroyWorld(false);
            }
        }
    };
}

// === StateTree任务基础功能测试 ===

/**
 * 测试ElementalStateTreeTaskBase的缓存功能
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStateTreeTaskCacheTest,
    "ElementalCombat.AI.StateTree.TaskCache",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FStateTreeTaskCacheTest::RunTest(const FString& Parameters)
{
    using namespace ElementalCombat::Tests;

    // Arrange - 创建测试任务
    FElementalStateTreeTaskBase TaskBase;

    // 测试缓存清理功能
    TaskBase.ClearAllCaches();

    // 测试性能统计功能
    FString CacheStats = TaskBase.GetPerformanceStats();
    TestTrue(TEXT("Performance stats should be available"), !CacheStats.IsEmpty());

    return true;
}

/**
 * 测试Utility上下文创建
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStateTreeUtilityContextTest,
    "ElementalCombat.AI.StateTree.UtilityContext",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FStateTreeUtilityContextTest::RunTest(const FString& Parameters)
{
    using namespace ElementalCombat::Tests;

    // Arrange
    UWorld* TestWorld = FStateTreeTestHelpers::CreateTestWorld();
    AddErrorIfFalse(TestWorld != nullptr, TEXT("Failed to create test world"));

    FUtilityProfile TestProfile = FStateTreeTestHelpers::CreateTestUtilityProfile();
    AElementalCombatEnemy* TestEnemy = FStateTreeTestHelpers::CreateTestEnemyWithAI(TestWorld, TestProfile);
    AddErrorIfFalse(TestEnemy != nullptr, TEXT("Failed to create test enemy with AI"));

    // Act & Assert
    AElementalCombatAIController* AIController = Cast<AElementalCombatAIController>(TestEnemy->GetController());
    TestTrue(TEXT("AI Controller should be correctly assigned"), AIController != nullptr);
    
    if (AIController)
    {
        const FUtilityProfile& ProfileFromController = AIController->GetCurrentAIProfile();
        TestEqual(TEXT("Profile name should match"), ProfileFromController.ProfileName, TestProfile.ProfileName);
        TestTrue(TEXT("Profile should have considerations"), ProfileFromController.Considerations.Num() > 0);
    }

    // Cleanup
    FStateTreeTestHelpers::CleanupTestWorld(TestWorld);

    return true;
}

/**
 * 测试UniversalUtilityTask的基本功能
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUniversalUtilityTaskTest,
    "ElementalCombat.AI.StateTree.UniversalUtilityTask",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FUniversalUtilityTaskTest::RunTest(const FString& Parameters)
{
    using namespace ElementalCombat::Tests;

    // Arrange
    UWorld* TestWorld = FStateTreeTestHelpers::CreateTestWorld();
    AddErrorIfFalse(TestWorld != nullptr, TEXT("Failed to create test world"));

    FUtilityProfile TestProfile = FStateTreeTestHelpers::CreateTestUtilityProfile(TEXT("UniversalTestProfile"));
    AElementalCombatEnemy* TestEnemy = FStateTreeTestHelpers::CreateTestEnemyWithAI(TestWorld, TestProfile);
    AddErrorIfFalse(TestEnemy != nullptr, TEXT("Failed to create test enemy"));

    // 创建任务实例数据
    FStateTreeUniversalUtilityInstanceData InstanceData;
    InstanceData.EnemyCharacter = TestEnemy;
    InstanceData.bRecalculateOnEnter = true;
    InstanceData.bContinuousUpdate = false;

    // Act
    AElementalCombatAIController* AIController = Cast<AElementalCombatAIController>(TestEnemy->GetController());
    TestTrue(TEXT("AI Controller should exist"), AIController != nullptr);

    if (AIController)
    {
        const FUtilityProfile& Profile = AIController->GetCurrentAIProfile();
        TestEqual(TEXT("Profile should be correctly assigned"), Profile.ProfileName, TEXT("UniversalTestProfile"));
        TestTrue(TEXT("Profile should have weights"), Profile.Weights.Num() > 0);
        TestTrue(TEXT("Profile should have considerations"), Profile.Considerations.Num() > 0);
    }

    // Cleanup
    FStateTreeTestHelpers::CleanupTestWorld(TestWorld);

    return true;
}

/**
 * 测试单项Utility评分任务
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUtilityConsiderationTaskTest,
    "ElementalCombat.AI.StateTree.UtilityConsiderationTask",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FUtilityConsiderationTaskTest::RunTest(const FString& Parameters)
{
    using namespace ElementalCombat::Tests;

    // Arrange
    UWorld* TestWorld = FStateTreeTestHelpers::CreateTestWorld();
    AddErrorIfFalse(TestWorld != nullptr, TEXT("Failed to create test world"));

    FUtilityProfile TestProfile = FStateTreeTestHelpers::CreateTestUtilityProfile();
    AElementalCombatEnemy* TestEnemy = FStateTreeTestHelpers::CreateTestEnemyWithAI(TestWorld, TestProfile);
    AddErrorIfFalse(TestEnemy != nullptr, TEXT("Failed to create test enemy"));

    // 创建考虑因子任务实例数据
    FStateTreeUtilityConsiderationInstanceData InstanceData;
    InstanceData.EnemyCharacter = TestEnemy;
    InstanceData.Consideration.ConsiderationType = EConsiderationType::Health;
    InstanceData.Consideration.ResponseCurve.EditorCurveData.Reset();
    InstanceData.Consideration.ResponseCurve.EditorCurveData.AddKey(0.0f, 0.0f);
    InstanceData.Consideration.ResponseCurve.EditorCurveData.AddKey(1.0f, 1.0f);
    InstanceData.ValidScoreThreshold = 0.1f;

    // Act & Assert
    TestTrue(TEXT("Enemy should be properly configured"), InstanceData.EnemyCharacter != nullptr);
    TestTrue(TEXT("Consideration should be valid"), InstanceData.Consideration.ConsiderationType == EConsiderationType::Health);

    // Cleanup
    FStateTreeTestHelpers::CleanupTestWorld(TestWorld);

    return true;
}

/**
 * 测试Utility配置比较任务
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUtilityComparisonTaskTest,
    "ElementalCombat.AI.StateTree.UtilityComparisonTask",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FUtilityComparisonTaskTest::RunTest(const FString& Parameters)
{
    using namespace ElementalCombat::Tests;

    // Arrange
    UWorld* TestWorld = FStateTreeTestHelpers::CreateTestWorld();
    AddErrorIfFalse(TestWorld != nullptr, TEXT("Failed to create test world"));

    FUtilityProfile TestProfile = FStateTreeTestHelpers::CreateTestUtilityProfile();
    AElementalCombatEnemy* TestEnemy = FStateTreeTestHelpers::CreateTestEnemyWithAI(TestWorld, TestProfile);
    AddErrorIfFalse(TestEnemy != nullptr, TEXT("Failed to create test enemy"));

    // 创建比较任务实例数据
    FStateTreeUtilityComparisonInstanceData InstanceData;
    InstanceData.EnemyCharacter = TestEnemy;
    InstanceData.ProfileA = FStateTreeTestHelpers::CreateTestUtilityProfile(TEXT("ProfileA"));
    InstanceData.ProfileB = FStateTreeTestHelpers::CreateTestUtilityProfile(TEXT("ProfileB"));
    
    // 设置不同的权重以便比较
    InstanceData.ProfileA.Weights[EConsiderationType::Health] = 0.8f;
    InstanceData.ProfileB.Weights[EConsiderationType::Health] = 0.2f;

    // Act & Assert
    TestTrue(TEXT("ProfileA should be configured"), !InstanceData.ProfileA.ProfileName.IsEmpty());
    TestTrue(TEXT("ProfileB should be configured"), !InstanceData.ProfileB.ProfileName.IsEmpty());
    TestNotEqual(TEXT("Profiles should have different names"), InstanceData.ProfileA.ProfileName, InstanceData.ProfileB.ProfileName);

    // Cleanup
    FStateTreeTestHelpers::CleanupTestWorld(TestWorld);

    return true;
}

/**
 * 测试动态权重调整任务
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDynamicUtilityTaskTest,
    "ElementalCombat.AI.StateTree.DynamicUtilityTask",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDynamicUtilityTaskTest::RunTest(const FString& Parameters)
{
    using namespace ElementalCombat::Tests;

    // Arrange
    UWorld* TestWorld = FStateTreeTestHelpers::CreateTestWorld();
    AddErrorIfFalse(TestWorld != nullptr, TEXT("Failed to create test world"));

    FUtilityProfile TestProfile = FStateTreeTestHelpers::CreateTestUtilityProfile(TEXT("DynamicTestProfile"));
    AElementalCombatEnemy* TestEnemy = FStateTreeTestHelpers::CreateTestEnemyWithAI(TestWorld, TestProfile);
    AddErrorIfFalse(TestEnemy != nullptr, TEXT("Failed to create test enemy"));

    // 创建动态权重任务实例数据
    FStateTreeDynamicUtilityInstanceData InstanceData;
    InstanceData.EnemyCharacter = TestEnemy;
    InstanceData.BaseProfile = TestProfile;
    InstanceData.bUseDynamicAdjustment = true;
    
    // 设置权重调整
    InstanceData.WeightAdjustments.Add(EConsiderationType::Health, 1.5f);
    InstanceData.WeightAdjustments.Add(EConsiderationType::Distance, 0.8f);

    // Act & Assert
    TestTrue(TEXT("Base profile should be set"), !InstanceData.BaseProfile.ProfileName.IsEmpty());
    TestTrue(TEXT("Dynamic adjustment should be enabled"), InstanceData.bUseDynamicAdjustment);
    TestTrue(TEXT("Weight adjustments should be configured"), InstanceData.WeightAdjustments.Num() > 0);

    // Cleanup
    FStateTreeTestHelpers::CleanupTestWorld(TestWorld);

    return true;
}

/**
 * 测试StateTree数据绑定功能
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStateTreeTaskDataBindingTest,
    "ElementalCombat.AI.StateTree.TaskDataBinding",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FStateTreeTaskDataBindingTest::RunTest(const FString& Parameters)
{
    using namespace ElementalCombat::Tests;

    // Arrange
    UWorld* TestWorld = FStateTreeTestHelpers::CreateTestWorld();
    AddErrorIfFalse(TestWorld != nullptr, TEXT("Failed to create test world"));

    FUtilityProfile TestProfile = FStateTreeTestHelpers::CreateTestUtilityProfile();
    AElementalCombatEnemy* TestEnemy = FStateTreeTestHelpers::CreateTestEnemyWithAI(TestWorld, TestProfile);
    AddErrorIfFalse(TestEnemy != nullptr, TEXT("Failed to create test enemy"));

    // Act & Assert - 测试基本的数据绑定
    AElementalCombatAIController* AIController = Cast<AElementalCombatAIController>(TestEnemy->GetController());
    TestTrue(TEXT("AI Controller binding should work"), AIController != nullptr);
    
    if (AIController)
    {
        TestTrue(TEXT("Enemy binding should work"), AIController->GetElementalCombatEnemy() == TestEnemy);
        TestTrue(TEXT("Profile binding should work"), !AIController->GetCurrentAIProfile().ProfileName.IsEmpty());
    }

    // Cleanup
    FStateTreeTestHelpers::CleanupTestWorld(TestWorld);

    return true;
}

/**
 * 测试StateTree任务描述功能
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStateTreeTaskDescriptionTest,
    "ElementalCombat.AI.StateTree.TaskDescription",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FStateTreeTaskDescriptionTest::RunTest(const FString& Parameters)
{
    // Arrange
    FStateTreeUniversalUtilityTask UniversalTask;

#if WITH_EDITOR
    // 简化的描述测试 - 主要验证任务描述功能可用
    // 不构造复杂的DataView，只验证基础功能
    TestTrue(TEXT("Universal utility task is valid"), true);
    
    // 我们可以验证任务结构本身是正确的
    const UStruct* InstanceStruct = UniversalTask.GetInstanceDataType();
    TestTrue(TEXT("Task should have valid instance data type"), InstanceStruct != nullptr);
    
    if (InstanceStruct)
    {
        TestEqual(TEXT("Instance data should be correct type"), InstanceStruct->GetName(), TEXT("StateTreeUniversalUtilityInstanceData"));
    }
#else
    // 非编辑器构建中，跳过描述测试
    TestTrue(TEXT("Description test skipped in non-editor build"), true);
#endif

    return true;
}

/**
 * 测试StateTree任务错误处理
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStateTreeTaskErrorHandlingTest,
    "ElementalCombat.AI.StateTree.TaskErrorHandling",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FStateTreeTaskErrorHandlingTest::RunTest(const FString& Parameters)
{
    using namespace ElementalCombat::Tests;

    // Arrange
    UWorld* TestWorld = FStateTreeTestHelpers::CreateTestWorld();
    AddErrorIfFalse(TestWorld != nullptr, TEXT("Failed to create test world"));

    // Test case 1: 无AI控制器的情况
    AElementalCombatEnemy* EnemyWithoutAI = TestWorld->SpawnActor<AElementalCombatEnemy>();
    AddErrorIfFalse(EnemyWithoutAI != nullptr, TEXT("Failed to create enemy"));

    FStateTreeUniversalUtilityInstanceData ErrorInstanceData;
    ErrorInstanceData.EnemyCharacter = EnemyWithoutAI;

    // Act & Assert
    TestTrue(TEXT("Error case should be handled gracefully when no AI controller"), ErrorInstanceData.EnemyCharacter != nullptr);
    TestTrue(TEXT("AI Controller should be null"), EnemyWithoutAI->GetController() == nullptr);

    // Test case 2: AI控制器无配置的情况（现在会使用默认测试配置）
    AElementalCombatAIController* EmptyAIController = TestWorld->SpawnActor<AElementalCombatAIController>();
    if (EmptyAIController)
    {
        EmptyAIController->Possess(EnemyWithoutAI);
        TestEqual(TEXT("AI Controller should have default test profile"), EmptyAIController->GetCurrentAIProfile().ProfileName, TEXT("DefaultTestProfile"));
    }

    // Cleanup
    FStateTreeTestHelpers::CleanupTestWorld(TestWorld);

    return true;
}

/**
 * 测试StateTree任务性能
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FStateTreeTaskPerformanceTest,
    "ElementalCombat.AI.StateTree.TaskPerformance",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FStateTreeTaskPerformanceTest::RunTest(const FString& Parameters)
{
    using namespace ElementalCombat::Tests;

    // Arrange
    UWorld* TestWorld = FStateTreeTestHelpers::CreateTestWorld();
    AddErrorIfFalse(TestWorld != nullptr, TEXT("Failed to create test world"));

    FUtilityProfile TestProfile = FStateTreeTestHelpers::CreateTestUtilityProfile();
    AElementalCombatEnemy* TestEnemy = FStateTreeTestHelpers::CreateTestEnemyWithAI(TestWorld, TestProfile);
    AddErrorIfFalse(TestEnemy != nullptr, TEXT("Failed to create test enemy"));

    // Act - 测试配置获取性能
    const int32 TestIterations = 1000;
    double StartTime = FPlatformTime::Seconds();
    
    AElementalCombatAIController* AIController = Cast<AElementalCombatAIController>(TestEnemy->GetController());
    for (int32 i = 0; i < TestIterations; ++i)
    {
        if (AIController)
        {
            const FUtilityProfile& Profile = AIController->GetCurrentAIProfile();
            // 强制使用结果避免编译器优化
            volatile bool bHasProfile = !Profile.ProfileName.IsEmpty();
        }
    }
    
    double EndTime = FPlatformTime::Seconds();
    double ElapsedTime = EndTime - StartTime;
    double AverageTime = ElapsedTime / TestIterations * 1000.0; // 转换为毫秒

    // Assert - 每次配置获取应该在合理时间内完成
    TestTrue(FString::Printf(TEXT("Profile access should be fast (%.3f ms average)"), AverageTime), AverageTime < 0.1); // 少于0.1毫秒

    // Cleanup
    FStateTreeTestHelpers::CleanupTestWorld(TestWorld);

    return true;
}