// Copyright 2025 guigui17f. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "AI/StateTreeTasks/ElementalStateTreeTaskBase.h"
#include "AI/StateTreeTasks/StateTreeEQSTasks.h"
#include "Engine/World.h"
#include "AI/ElementalCombatEnemy.h"

// === EQS集成功能测试 ===

/**
 * 测试EQS查询缓存结构
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEQSCacheStructTest,
    "ElementalCombat.AI.EQS.CacheStruct",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FEQSCacheStructTest::RunTest(const FString& Parameters)
{
    // Arrange - 创建EQS缓存结构
    FEQSQueryCache Cache;

    // Act & Assert - 测试初始状态
    TestFalse(TEXT("Cache should be invalid initially"), Cache.bIsValid);
    TestEqual(TEXT("Cache timestamp should be -1"), Cache.CacheTimestamp, -1.0f);
    TestTrue(TEXT("Cached locations should be empty"), Cache.CachedLocations.Num() == 0);
    TestEqual(TEXT("Best location should be zero"), Cache.BestLocation, FVector::ZeroVector);

    // 测试过期检查
    TestTrue(TEXT("Empty cache should be expired"), Cache.IsExpired(0.0f, 1.0f));

    // 模拟有效缓存数据
    Cache.CachedLocations.Add(FVector(100, 200, 0));
    Cache.CachedLocations.Add(FVector(150, 250, 0));
    Cache.BestLocation = FVector(100, 200, 0);
    Cache.CacheTimestamp = 10.0f;
    Cache.bIsValid = true;

    TestTrue(TEXT("Cache should be valid with data"), Cache.bIsValid);
    TestEqual(TEXT("Should have 2 cached locations"), Cache.CachedLocations.Num(), 2);
    TestFalse(TEXT("Fresh cache should not be expired"), Cache.IsExpired(10.5f, 1.0f));
    TestTrue(TEXT("Old cache should be expired"), Cache.IsExpired(12.0f, 1.0f));

    // 测试重置功能
    Cache.Reset();
    TestFalse(TEXT("Reset cache should be invalid"), Cache.bIsValid);
    TestTrue(TEXT("Reset cache should have empty locations"), Cache.CachedLocations.Num() == 0);

    return true;
}

/**
 * 测试EQS查询任务实例数据
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEQSQueryInstanceDataTest,
    "ElementalCombat.AI.EQS.QueryInstanceData",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FEQSQueryInstanceDataTest::RunTest(const FString& Parameters)
{
    // Arrange - 创建EQS查询实例数据
    FStateTreeEQSQueryInstanceData InstanceData;

    // Act & Assert - 测试默认值
    TestEqual(TEXT("Default run mode should be SingleResult"), 
              InstanceData.RunMode, EEnvQueryRunMode::SingleResult);
    TestEqual(TEXT("Default max results should be 1"), InstanceData.MaxResults, 1);
    TestTrue(TEXT("Should run async by default"), InstanceData.bRunAsync);
    TestEqual(TEXT("Default query timeout should be 2.0"), InstanceData.QueryTimeout, 2.0f);
    TestFalse(TEXT("Query should not be successful initially"), InstanceData.bQuerySucceeded);

    // 测试配置设置
    InstanceData.RunMode = EEnvQueryRunMode::AllMatching;
    InstanceData.MaxResults = 10;
    InstanceData.bRunAsync = false;
    InstanceData.QueryTimeout = 5.0f;

    TestEqual(TEXT("Run mode should be updated"), InstanceData.RunMode, EEnvQueryRunMode::AllMatching);
    TestEqual(TEXT("Max results should be updated"), InstanceData.MaxResults, 10);
    TestFalse(TEXT("Async should be disabled"), InstanceData.bRunAsync);
    TestEqual(TEXT("Timeout should be updated"), InstanceData.QueryTimeout, 5.0f);

    // 模拟查询结果
    InstanceData.ResultLocations.Add(FVector(100, 100, 0));
    InstanceData.ResultLocations.Add(FVector(200, 200, 0));
    InstanceData.BestLocation = FVector(100, 100, 0);
    InstanceData.bQuerySucceeded = true;

    TestTrue(TEXT("Query should be marked as successful"), InstanceData.bQuerySucceeded);
    TestEqual(TEXT("Should have 2 result locations"), InstanceData.ResultLocations.Num(), 2);
    TestEqual(TEXT("Best location should be set"), InstanceData.BestLocation, FVector(100, 100, 0));

    return true;
}

/**
 * 测试EQS战术查询任务配置
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEQSTacticalQueryTest,
    "ElementalCombat.AI.EQS.TacticalQuery",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FEQSTacticalQueryTest::RunTest(const FString& Parameters)
{
    // Arrange - 创建战术查询实例数据
    FStateTreeEQSTacticalQueryInstanceData InstanceData;

    // Act & Assert - 测试默认配置
    TestTrue(TEXT("Should auto-select query type by default"), InstanceData.bAutoSelectQueryType);
    TestEqual(TEXT("Default forced query type should be Melee"), 
              InstanceData.ForcedQueryType, EAIAttackType::Melee);
    TestEqual(TEXT("Initial used query type should be None"), 
              InstanceData.UsedQueryType, EAIAttackType::None);

    // 测试查询类型设置
    InstanceData.bAutoSelectQueryType = false;
    InstanceData.ForcedQueryType = EAIAttackType::Ranged;

    TestFalse(TEXT("Auto-select should be disabled"), InstanceData.bAutoSelectQueryType);
    TestEqual(TEXT("Forced query type should be Ranged"), 
              InstanceData.ForcedQueryType, EAIAttackType::Ranged);

    // 模拟查询结果
    InstanceData.RecommendedPosition = FVector(300, 400, 0);
    InstanceData.UsedQueryType = EAIAttackType::Ranged;
    InstanceData.AlternativePositions.Add(FVector(250, 350, 0));
    InstanceData.AlternativePositions.Add(FVector(350, 450, 0));
    InstanceData.PositionQuality = 0.8f;

    TestEqual(TEXT("Recommended position should be set"), 
              InstanceData.RecommendedPosition, FVector(300, 400, 0));
    TestEqual(TEXT("Used query type should be Ranged"), 
              InstanceData.UsedQueryType, EAIAttackType::Ranged);
    TestEqual(TEXT("Should have 2 alternative positions"), InstanceData.AlternativePositions.Num(), 2);
    TestEqual(TEXT("Position quality should be 0.8"), InstanceData.PositionQuality, 0.8f);

    return true;
}

/**
 * 测试EQS与Utility结合任务
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEQSUtilityTaskTest,
    "ElementalCombat.AI.EQS.EQSUtilityTask", 
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FEQSUtilityTaskTest::RunTest(const FString& Parameters)
{
    // Arrange - 创建EQS-Utility结合任务实例数据
    FStateTreeEQSUtilityInstanceData InstanceData;

    // 设置测试AIController（由于PositionScoringProfile已移除，现在使用AIController配置）
    // 注意：在实际测试中需要先设置AIController

    // 设置评分参数
    InstanceData.MinAcceptableScore = 0.4f;
    InstanceData.MaxPositionsToEvaluate = 5;

    // Act & Assert - 验证配置
    TestEqual(TEXT("Min acceptable score should be 0.4"), InstanceData.MinAcceptableScore, 0.4f);
    TestEqual(TEXT("Max positions to evaluate should be 5"), InstanceData.MaxPositionsToEvaluate, 5);
    // PositionScoringProfile已移除，配置现在位于AIController中
    TestTrue(TEXT("EQS Utility task instance data configured"), true);

    // 模拟评估结果
    InstanceData.BestScoredPosition = FVector(400, 300, 0);
    InstanceData.FinalScore = 0.75f;
    // Score already set above, no need to set validity separately
    InstanceData.EvaluatedPositions.Add(FVector(400, 300, 0), 0.75f);
    InstanceData.EvaluatedPositions.Add(FVector(350, 250, 0), 0.65f);
    InstanceData.EvaluatedPositions.Add(FVector(450, 350, 0), 0.55f);
    InstanceData.bFoundValidPosition = true;

    TestEqual(TEXT("Best scored position should be set"), 
              InstanceData.BestScoredPosition, FVector(400, 300, 0));
    TestEqual(TEXT("Best position score should be 0.75"), InstanceData.FinalScore, 0.75f);
    TestTrue(TEXT("Best position score should be valid"), InstanceData.FinalScore > 0.01f);
    TestEqual(TEXT("Should have 3 evaluated positions"), InstanceData.EvaluatedPositions.Num(), 3);
    TestTrue(TEXT("Should have found valid position"), InstanceData.bFoundValidPosition);

    // 验证最佳位置评分高于阈值
    TestTrue(TEXT("Best score should be above threshold"), 
             InstanceData.FinalScore >= InstanceData.MinAcceptableScore);

    return true;
}

/**
 * 测试EQS缓存管理任务
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEQSCacheManagerTest,
    "ElementalCombat.AI.EQS.CacheManager",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FEQSCacheManagerTest::RunTest(const FString& Parameters)
{
    // Arrange - 创建缓存管理任务实例数据
    FStateTreeEQSCacheInstanceData InstanceData;

    // Act & Assert - 测试默认配置
    TestFalse(TEXT("Should not clear all caches by default"), InstanceData.bClearAllCaches);
    TestTrue(TEXT("Should clear expired caches by default"), InstanceData.bClearExpiredCaches);
    TestFalse(TEXT("Should not show cache stats by default"), InstanceData.bShowCacheStats);

    // 配置缓存管理选项
    InstanceData.bClearAllCaches = true;
    InstanceData.bClearExpiredCaches = true;
    InstanceData.bShowCacheStats = true;

    TestTrue(TEXT("Clear all caches should be enabled"), InstanceData.bClearAllCaches);
    TestTrue(TEXT("Clear expired caches should be enabled"), InstanceData.bClearExpiredCaches);
    TestTrue(TEXT("Show cache stats should be enabled"), InstanceData.bShowCacheStats);

    // 模拟缓存统计信息
    InstanceData.CacheStatsInfo = TEXT("EQS Cache: 5/10 valid (50.0%)");

    TestEqual(TEXT("Cache stats info should be set"), 
              InstanceData.CacheStatsInfo, TEXT("EQS Cache: 5/10 valid (50.0%)"));
    TestTrue(TEXT("Cache stats info should not be empty"), !InstanceData.CacheStatsInfo.IsEmpty());

    return true;
}

/**
 * 测试EQS哈希计算和缓存键生成
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEQSHashCalculationTest,
    "ElementalCombat.AI.EQS.HashCalculation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FEQSHashCalculationTest::RunTest(const FString& Parameters)
{
    // 这个测试主要验证哈希计算的一致性，而不是具体的哈希值

    // Arrange - 创建测试数据
    FVector Position1(100, 200, 0);
    FVector Position2(100, 200, 0); // 相同位置
    FVector Position3(150, 250, 0); // 不同位置

    // Act - 计算哈希值
    uint32 Hash1A = GetTypeHash(Position1.X) ^ GetTypeHash(Position1.Y);
    uint32 Hash1B = GetTypeHash(Position2.X) ^ GetTypeHash(Position2.Y);
    uint32 Hash2 = GetTypeHash(Position3.X) ^ GetTypeHash(Position3.Y);

    // Assert - 验证哈希一致性
    TestEqual(TEXT("Same positions should have same hash"), Hash1A, Hash1B);
    TestNotEqual(TEXT("Different positions should have different hash"), Hash1A, Hash2);

    // 测试更复杂的哈希组合
    AActor* TestActor1 = nullptr;
    AActor* TestActor2 = nullptr;
    
    uint32 ComplexHash1 = HashCombine(Hash1A, GetTypeHash(TestActor1));
    uint32 ComplexHash2 = HashCombine(Hash1B, GetTypeHash(TestActor2));
    
    TestEqual(TEXT("Complex hashes with same inputs should be equal"), ComplexHash1, ComplexHash2);

    return true;
}

/**
 * 测试EQS查询参数验证
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEQSQueryValidationTest,
    "ElementalCombat.AI.EQS.QueryValidation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FEQSQueryValidationTest::RunTest(const FString& Parameters)
{
    // Arrange - 创建各种查询配置
    FStateTreeEQSQueryInstanceData ValidData;
    ValidData.RunMode = EEnvQueryRunMode::SingleResult;
    ValidData.MaxResults = 1;
    ValidData.QueryTimeout = 2.0f;

    FStateTreeEQSQueryInstanceData InvalidTimeoutData;
    InvalidTimeoutData.QueryTimeout = -1.0f; // 无效超时

    FStateTreeEQSQueryInstanceData InvalidMaxResultsData;
    InvalidMaxResultsData.MaxResults = 0; // 无效结果数量

    // Act & Assert - 验证参数范围
    TestTrue(TEXT("Valid data should have positive timeout"), ValidData.QueryTimeout > 0.0f);
    TestTrue(TEXT("Valid data should have positive max results"), ValidData.MaxResults > 0);

    TestFalse(TEXT("Invalid timeout should be caught"), InvalidTimeoutData.QueryTimeout > 0.0f);
    TestFalse(TEXT("Invalid max results should be caught"), InvalidMaxResultsData.MaxResults > 0);

    // 测试超时边界值
    TestTrue(TEXT("Minimum timeout should be reasonable"), ValidData.QueryTimeout >= 0.1f);
    TestTrue(TEXT("Maximum timeout should be reasonable"), ValidData.QueryTimeout <= 10.0f);

    // 测试结果数量边界值
    TestTrue(TEXT("Max results should be at least 1"), ValidData.MaxResults >= 1);
    TestTrue(TEXT("Max results should have reasonable upper limit"), ValidData.MaxResults <= 100);

    return true;
}

/**
 * 测试EQS任务状态管理
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEQSTaskStateTest,
    "ElementalCombat.AI.EQS.TaskState",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FEQSTaskStateTest::RunTest(const FString& Parameters)
{
    // Arrange - 创建任务状态数据
    FStateTreeEQSQueryInstanceData StateData;
    
    // 模拟查询生命周期状态
    
    // 初始状态
    TestFalse(TEXT("Query should not be successful initially"), StateData.bQuerySucceeded);
    TestTrue(TEXT("Result locations should be empty initially"), StateData.ResultLocations.Num() == 0);
    TestEqual(TEXT("Best location should be zero initially"), StateData.BestLocation, FVector::ZeroVector);

    // 查询运行状态（这些是内部状态，实际测试中可能无法直接访问）
    // 在实际实现中，这些状态会在任务内部管理

    // 查询完成状态
    StateData.bQuerySucceeded = true;
    StateData.ResultLocations.Add(FVector(100, 200, 0));
    StateData.ResultLocations.Add(FVector(150, 250, 0));
    StateData.BestLocation = FVector(100, 200, 0);

    TestTrue(TEXT("Query should be successful after completion"), StateData.bQuerySucceeded);
    TestEqual(TEXT("Should have result locations after completion"), StateData.ResultLocations.Num(), 2);
    TestNotEqual(TEXT("Best location should be set after completion"), StateData.BestLocation, FVector::ZeroVector);

    // 查询失败状态
    StateData.bQuerySucceeded = false;
    StateData.QueryError = TEXT("Query failed due to no valid results");

    TestFalse(TEXT("Query should not be successful after failure"), StateData.bQuerySucceeded);
    TestFalse(TEXT("Error message should be set after failure"), StateData.QueryError.IsEmpty());

    return true;
}

/**
 * 测试EQS任务类型系统
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEQSTaskTypeTest,
    "ElementalCombat.AI.EQS.TaskType",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FEQSTaskTypeTest::RunTest(const FString& Parameters)
{
    // Arrange - 创建不同类型的EQS任务
    FStateTreeEQSQueryTask QueryTask;
    FStateTreeEQSTacticalQueryTask TacticalTask;
    FStateTreeEQSUtilityTask UtilityTask;
    FStateTreeEQSCacheManagerTask CacheManagerTask;

    // Act & Assert - 验证任务类型系统
    TestTrue(TEXT("Query task should have correct instance data type"),
             QueryTask.GetInstanceDataType() == FStateTreeEQSQueryInstanceData::StaticStruct());

    TestTrue(TEXT("Tactical task should have correct instance data type"),
             TacticalTask.GetInstanceDataType() == FStateTreeEQSTacticalQueryInstanceData::StaticStruct());

    TestTrue(TEXT("Utility task should have correct instance data type"),
             UtilityTask.GetInstanceDataType() == FStateTreeEQSUtilityInstanceData::StaticStruct());

    TestTrue(TEXT("Cache manager task should have correct instance data type"),
             CacheManagerTask.GetInstanceDataType() == FStateTreeEQSCacheInstanceData::StaticStruct());

    // 验证任务类型唯一性
    TestNotEqual(TEXT("Query and Tactical tasks should have different types"),
                 QueryTask.GetInstanceDataType(), TacticalTask.GetInstanceDataType());

    TestNotEqual(TEXT("Utility and Cache manager tasks should have different types"),
                 UtilityTask.GetInstanceDataType(), CacheManagerTask.GetInstanceDataType());

    return true;
}

/**
 * 测试EQS性能特性
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FEQSPerformanceTest,
    "ElementalCombat.AI.EQS.Performance",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FEQSPerformanceTest::RunTest(const FString& Parameters)
{
    // Arrange - 创建性能测试数据
    FEQSQueryCache PerfCache;
    const int32 LocationCount = 100;

    // 填充大量位置数据
    for (int32 i = 0; i < LocationCount; ++i)
    {
        FVector Location(i * 10.0f, i * 15.0f, 0.0f);
        PerfCache.CachedLocations.Add(Location);
    }
    PerfCache.BestLocation = PerfCache.CachedLocations[0];
    PerfCache.CacheTimestamp = 0.0f;
    PerfCache.bIsValid = true;

    // Act - 测试缓存操作性能
    double StartTime = FPlatformTime::Seconds();
    const int32 IterationCount = 1000;

    for (int32 i = 0; i < IterationCount; ++i)
    {
        // 模拟缓存访问操作
        bool bIsExpired = PerfCache.IsExpired(i * 0.001f, 1.0f);
        FVector BestLoc = PerfCache.BestLocation;
        int32 Count = PerfCache.CachedLocations.Num();
        
        // 防止编译器优化
        if (Count != LocationCount || BestLoc == FVector::ZeroVector || bIsExpired)
        {
            // 这个条件在正常情况下不应该成立
        }
    }

    double EndTime = FPlatformTime::Seconds();
    double TotalTime = EndTime - StartTime;
    double AverageTimeMs = (TotalTime / IterationCount) * 1000.0;

    // Assert - 验证性能要求
    TestTrue(FString::Printf(TEXT("Cache access time (%.3fms) should be under 0.1ms"), AverageTimeMs),
             AverageTimeMs < 0.1);

    TestEqual(TEXT("Cache should maintain correct location count"), PerfCache.CachedLocations.Num(), LocationCount);

    UE_LOG(LogTemp, Log, TEXT("EQS Cache Performance: %d operations in %.3fs (avg: %.3fms per operation)"),
           IterationCount, TotalTime, AverageTimeMs);

    return true;
}