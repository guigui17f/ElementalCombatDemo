// Copyright 2025 guigui17f. All Rights Reserved.

#include "CoreMinimal.h"
#include "Misc/AutomationTest.h"
#include "AI/Utility/UtilityAITypes.h"
#include "AI/Utility/IUtilityScorer.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

// === Utility AI基础功能测试 ===

/**
 * 测试FUtilityContext的基本功能
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUtilityContextTest, 
    "ElementalCombat.AI.Utility.UtilityContext",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FUtilityContextTest::RunTest(const FString& Parameters)
{
    // Arrange - 创建测试上下文
    FUtilityContext Context;
    Context.HealthPercent = 0.75f;
    Context.DistanceToTarget = 500.0f;
    Context.ElementAdvantage = 1.0f; // 优势
    Context.ThreatLevel = 0.6f;

    // Act & Assert - 测试输入值获取
    TestEqual(TEXT("Health input value"), Context.GetInputValue(EConsiderationType::Health), 0.75f);
    TestEqual(TEXT("Distance input value"), Context.GetInputValue(EConsiderationType::Distance), 0.5f); // 500/1000
    TestEqual(TEXT("Element advantage input value"), Context.GetInputValue(EConsiderationType::ElementAdvantage), 1.0f); // (1+1)*0.5
    TestEqual(TEXT("Threat level input value"), Context.GetInputValue(EConsiderationType::ThreatLevel), 0.6f);

    // 测试自定义值
    Context.SetCustomValue(TEXT("TestKey"), 0.8f);
    TestEqual(TEXT("Custom value"), Context.GetCustomValue(TEXT("TestKey")), 0.8f);
    TestEqual(TEXT("Non-existent custom value"), Context.GetCustomValue(TEXT("NonExistent"), 0.5f), 0.5f);

    return true;
}

/**
 * 测试FUtilityConsideration的评分计算
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUtilityConsiderationTest,
    "ElementalCombat.AI.Utility.UtilityConsideration",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FUtilityConsiderationTest::RunTest(const FString& Parameters)
{
    // Arrange - 创建测试配置
    FUtilityConsideration Consideration;
    Consideration.ConsiderationType = EConsiderationType::Health;
    Consideration.bInvertInput = true; // 健康度越低评分越高
    Consideration.InputMultiplier = 1.0f;
    Consideration.OutputOffset = 0.0f;
    
    // 设置线性响应曲线 (y = x)
    Consideration.ResponseCurve.EditorCurveData.Reset();
    Consideration.ResponseCurve.EditorCurveData.AddKey(0.0f, 0.0f);
    Consideration.ResponseCurve.EditorCurveData.AddKey(1.0f, 1.0f);

    FUtilityContext Context;
    Context.HealthPercent = 0.3f; // 低健康度

    // Act - 计算评分
    float Score = Consideration.CalculateScore(Context);

    // Assert - 验证结果
    float ExpectedScore = 1.0f - 0.3f; // 反转后的值
    TestEqual(TEXT("Inverted health consideration score"), Score, ExpectedScore);

    // 测试不反转的情况
    Consideration.bInvertInput = false;
    Score = Consideration.CalculateScore(Context);
    TestEqual(TEXT("Non-inverted health consideration score"), Score, 0.3f);

    // 测试乘数
    Consideration.InputMultiplier = 2.0f;
    Score = Consideration.CalculateScore(Context);
    TestEqual(TEXT("Multiplied input consideration score"), Score, 0.6f); // 0.3 * 2.0 但限制在1.0以内

    return true;
}

/**
 * 测试FUtilityProfile的综合评分
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUtilityProfileTest,
    "ElementalCombat.AI.Utility.UtilityProfile",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FUtilityProfileTest::RunTest(const FString& Parameters)
{
    // Arrange - 创建测试配置文件
    FUtilityProfile Profile;
    Profile.ProfileName = TEXT("TestProfile");
    Profile.bUseMultiplicativeCombination = false; // 使用加权平均
    Profile.MinScoreThreshold = 0.1f;

    // 添加健康度考虑因素
    FUtilityConsideration HealthConsideration;
    HealthConsideration.ConsiderationType = EConsiderationType::Health;
    HealthConsideration.ResponseCurve.EditorCurveData.Reset();
    HealthConsideration.ResponseCurve.EditorCurveData.AddKey(0.0f, 0.0f);
    HealthConsideration.ResponseCurve.EditorCurveData.AddKey(1.0f, 1.0f);
    Profile.Considerations.Add(HealthConsideration);

    // 添加距离考虑因素
    FUtilityConsideration DistanceConsideration;
    DistanceConsideration.ConsiderationType = EConsiderationType::Distance;
    DistanceConsideration.ResponseCurve.EditorCurveData.Reset();
    DistanceConsideration.ResponseCurve.EditorCurveData.AddKey(0.0f, 1.0f); // 距离越近评分越高
    DistanceConsideration.ResponseCurve.EditorCurveData.AddKey(1.0f, 0.0f);
    Profile.Considerations.Add(DistanceConsideration);

    // 设置权重
    Profile.SetWeight(EConsiderationType::Health, 1.0f);
    Profile.SetWeight(EConsiderationType::Distance, 2.0f);

    // 创建测试上下文
    FUtilityContext Context;
    Context.HealthPercent = 0.8f;
    Context.DistanceToTarget = 200.0f; // 距离输入为0.2

    // Act - 计算评分
    FUtilityScore Score = Profile.CalculateScore(Context);

    // Assert - 验证结果
    TestTrue(TEXT("Score is valid"), Score.bIsValid);
    TestTrue(TEXT("Score is above threshold"), Score.FinalScore >= Profile.MinScoreThreshold);

    // 验证加权平均计算
    // 预期: (0.8 * 1.0 + 0.8 * 2.0) / (1.0 + 2.0) = 2.4 / 3.0 = 0.8
    float ExpectedScore = (0.8f * 1.0f + 0.8f * 2.0f) / (1.0f + 2.0f);
    TestNearlyEqual(TEXT("Weighted average score"), Score.FinalScore, ExpectedScore, 0.01f);

    // 测试乘法组合
    Profile.bUseMultiplicativeCombination = true;
    Score = Profile.CalculateScore(Context);
    TestTrue(TEXT("Multiplicative score is valid"), Score.bIsValid);
    TestTrue(TEXT("Multiplicative score is positive"), Score.FinalScore > 0.0f);

    return true;
}

/**
 * 测试响应曲线功能
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUtilityResponseCurveTest,
    "ElementalCombat.AI.Utility.ResponseCurve", 
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FUtilityResponseCurveTest::RunTest(const FString& Parameters)
{
    // Arrange - 创建非线性响应曲线
    FUtilityConsideration Consideration;
    Consideration.ConsiderationType = EConsiderationType::Health;
    
    // 创建二次曲线 (y = x^2)
    Consideration.ResponseCurve.EditorCurveData.Reset();
    Consideration.ResponseCurve.EditorCurveData.AddKey(0.0f, 0.0f);
    Consideration.ResponseCurve.EditorCurveData.AddKey(0.5f, 0.25f);
    Consideration.ResponseCurve.EditorCurveData.AddKey(1.0f, 1.0f);

    FUtilityContext Context;
    
    // Act & Assert - 测试不同输入值的响应
    Context.HealthPercent = 0.5f;
    float Score = Consideration.CalculateScore(Context);
    TestNearlyEqual(TEXT("Quadratic curve at 0.5"), Score, 0.25f, 0.02f);

    Context.HealthPercent = 0.7f;
    Score = Consideration.CalculateScore(Context);
    // 0.7^2 = 0.49，但需要考虑曲线插值
    TestTrue(TEXT("Quadratic curve at 0.7 is reasonable"), Score > 0.4f && Score < 0.6f);

    return true;
}

/**
 * 测试UUtilityScorerComponent
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUtilityScorerComponentTest,
    "ElementalCombat.AI.Utility.ScorerComponent",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FUtilityScorerComponentTest::RunTest(const FString& Parameters)
{
    // Arrange - 创建测试世界和组件
    UWorld* TestWorld = UWorld::CreateWorld(EWorldType::Game, false);
    AddErrorIfFalse(TestWorld != nullptr, TEXT("Failed to create test world"));

    AActor* TestActor = TestWorld->SpawnActor<AActor>();
    AddErrorIfFalse(TestActor != nullptr, TEXT("Failed to spawn test actor"));

    UUtilityScorerComponent* ScorerComponent = NewObject<UUtilityScorerComponent>(TestActor);
    AddErrorIfFalse(ScorerComponent != nullptr, TEXT("Failed to create scorer component"));

    // 配置默认评分配置
    FUtilityProfile TestProfile;
    TestProfile.ProfileName = TEXT("ComponentTestProfile");
    FUtilityConsideration HealthConsideration;
    HealthConsideration.ConsiderationType = EConsiderationType::Health;
    TestProfile.Considerations.Add(HealthConsideration);

    ScorerComponent->SetScoringProfile(TestProfile);

    // 创建测试上下文
    FUtilityContext Context;
    Context.HealthPercent = 0.6f;
    Context.CurrentTime = TestWorld->GetTimeSeconds();

    // Act - 计算评分
    FUtilityScore Score = IUtilityScorer::Execute_CalculateUtilityScore(ScorerComponent, Context);

    // Assert - 验证结果
    TestTrue(TEXT("Component score is valid"), Score.bIsValid);
    TestEqual(TEXT("Component scorer name"), IUtilityScorer::Execute_GetScorerName(ScorerComponent), TEXT("DefaultScorer"));

    // 测试缓存功能
    float FirstCalculationTime = Score.CalculationTime;
    FUtilityScore CachedScore = IUtilityScorer::Execute_CalculateUtilityScore(ScorerComponent, Context);
    
    // 由于缓存，第二次调用应该返回相同的结果
    TestEqual(TEXT("Cached score is same"), CachedScore.FinalScore, Score.FinalScore);

    // 清理
    TestWorld->DestroyWorld(false);

    return true;
}

/**
 * 测试边界条件和错误处理
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUtilityBoundaryTest,
    "ElementalCombat.AI.Utility.BoundaryConditions",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FUtilityBoundaryTest::RunTest(const FString& Parameters)
{
    // 测试空配置文件
    FUtilityProfile EmptyProfile;
    FUtilityContext Context;
    Context.HealthPercent = 0.5f;
    
    FUtilityScore Score = EmptyProfile.CalculateScore(Context);
    TestFalse(TEXT("Empty profile should be invalid"), Score.bIsValid);

    // 测试极值输入
    FUtilityConsideration Consideration;
    Consideration.ConsiderationType = EConsiderationType::Health;
    
    Context.HealthPercent = -0.5f; // 负值
    float NegativeScore = Consideration.CalculateScore(Context);
    TestTrue(TEXT("Negative input should be clamped"), NegativeScore >= 0.0f && NegativeScore <= 1.0f);

    Context.HealthPercent = 2.0f; // 超过1.0的值
    float OverflowScore = Consideration.CalculateScore(Context);
    TestTrue(TEXT("Overflow input should be clamped"), OverflowScore >= 0.0f && OverflowScore <= 1.0f);

    // 测试零权重
    FUtilityProfile ZeroWeightProfile;
    ZeroWeightProfile.Considerations.Add(Consideration);
    ZeroWeightProfile.SetWeight(EConsiderationType::Health, 0.0f);
    
    Context.HealthPercent = 1.0f;
    Score = ZeroWeightProfile.CalculateScore(Context);
    TestEqual(TEXT("Zero weight should result in zero score"), Score.FinalScore, 0.0f);

    return true;
}

/**
 * 测试性能基准
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUtilityPerformanceTest,
    "ElementalCombat.AI.Utility.Performance",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FUtilityPerformanceTest::RunTest(const FString& Parameters)
{
    // Arrange - 创建复杂的评分配置
    FUtilityProfile ComplexProfile;
    ComplexProfile.ProfileName = TEXT("PerformanceTestProfile");

    // 添加多个评分因素
    for (int32 i = 1; i < static_cast<int32>(EConsiderationType::Custom); ++i)
    {
        FUtilityConsideration Consideration;
        Consideration.ConsiderationType = static_cast<EConsiderationType>(i);
        ComplexProfile.Considerations.Add(Consideration);
        ComplexProfile.SetWeight(static_cast<EConsiderationType>(i), 1.0f);
    }

    FUtilityContext Context;
    Context.HealthPercent = 0.7f;
    Context.DistanceToTarget = 300.0f;
    Context.ElementAdvantage = 0.5f;
    Context.ThreatLevel = 0.4f;

    // Act - 执行大量计算并测量时间
    double StartTime = FPlatformTime::Seconds();
    const int32 IterationCount = 1000;

    for (int32 i = 0; i < IterationCount; ++i)
    {
        FUtilityScore Score = ComplexProfile.CalculateScore(Context);
        // 防止编译器优化掉循环
        Context.HealthPercent += 0.00001f;
    }

    double EndTime = FPlatformTime::Seconds();
    double TotalTime = EndTime - StartTime;
    double AverageTimeMs = (TotalTime / IterationCount) * 1000.0;

    // Assert - 验证性能
    TestTrue(FString::Printf(TEXT("Average calculation time (%.3fms) should be under 1ms"), AverageTimeMs), 
             AverageTimeMs < 1.0);

    UE_LOG(LogTemp, Log, TEXT("Utility Performance Test: %d iterations in %.3fs (avg: %.3fms per calculation)"),
           IterationCount, TotalTime, AverageTimeMs);

    return true;
}

/**
 * 测试多线程安全性（基础测试）
 */
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUtilityThreadSafetyTest,
    "ElementalCombat.AI.Utility.ThreadSafety",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FUtilityThreadSafetyTest::RunTest(const FString& Parameters)
{
    // 注意：这是一个基础的线程安全测试
    // 在实际应用中可能需要更复杂的并发测试

    FUtilityProfile Profile;
    FUtilityConsideration HealthConsideration;
    HealthConsideration.ConsiderationType = EConsiderationType::Health;
    Profile.Considerations.Add(HealthConsideration);

    // 模拟多个线程同时计算评分
    constexpr int32 ThreadCount = 4;
    constexpr int32 CalculationsPerThread = 100;
    TArray<FUtilityScore> Results;
    Results.SetNum(ThreadCount * CalculationsPerThread);

    // 使用简单的并行for循环模拟多线程访问
    ParallelFor(ThreadCount * CalculationsPerThread, [&](int32 Index)
    {
        FUtilityContext Context;
        Context.HealthPercent = 0.5f + (Index % 10) * 0.05f; // 变化的健康度值
        Results[Index] = Profile.CalculateScore(Context);
    });

    // 验证所有结果都有效
    int32 ValidResults = 0;
    for (const FUtilityScore& Result : Results)
    {
        if (Result.bIsValid)
        {
            ValidResults++;
        }
    }

    TestEqual(TEXT("All parallel calculations should be valid"), ValidResults, Results.Num());

    return true;
}