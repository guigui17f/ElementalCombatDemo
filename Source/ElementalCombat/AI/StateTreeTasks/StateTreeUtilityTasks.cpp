// Copyright 2025 guigui17f. All Rights Reserved.

#include "StateTreeUtilityTasks.h"
#include "StateTreeExecutionContext.h"

// === FStateTreeUniversalUtilityTask 实现 ===

EStateTreeRunStatus FStateTreeUniversalUtilityTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

    if (!InstanceData.EnemyCharacter)
    {
        LogDebug(TEXT("UniversalUtilityTask: EnemyCharacter is null"));
        InstanceData.ErrorMessage = TEXT("No EnemyCharacter found");
        return EStateTreeRunStatus::Failed;
    }

    if (bEnableDebugOutput)
    {
        LogDebug(FString::Printf(TEXT("UniversalUtilityTask: Starting evaluation with profile '%s'"), 
                                *InstanceData.ScoringProfile.ProfileName));
    }

    // 初始化评分计算
    if (InstanceData.bRecalculateOnEnter || !InstanceData.CalculatedScore.bIsValid)
    {
        if (!UpdateScore(Context))
        {
            return EStateTreeRunStatus::Failed;
        }
    }

    // 如果不需要持续更新，直接成功
    if (!InstanceData.bContinuousUpdate)
    {
        return EStateTreeRunStatus::Succeeded;
    }

    // 需要持续更新，保持运行状态
    return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FStateTreeUniversalUtilityTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

    if (!InstanceData.bContinuousUpdate)
    {
        // 不需要持续更新，直接成功
        return EStateTreeRunStatus::Succeeded;
    }

    float CurrentTime = GetCurrentWorldTime(Context);

    // 检查是否需要更新
    if (ShouldUpdate(InstanceData, CurrentTime))
    {
        if (!UpdateScore(Context))
        {
            return EStateTreeRunStatus::Failed;
        }

        InstanceData.LastUpdateTime = CurrentTime;
    }

    return EStateTreeRunStatus::Running;
}

void FStateTreeUniversalUtilityTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    if (bEnableDebugOutput)
    {
        const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
        LogDebug(FString::Printf(TEXT("UniversalUtilityTask: Exiting with final score %.3f"), 
                                InstanceData.CalculatedScore.FinalScore));
    }
}

bool FStateTreeUniversalUtilityTask::UpdateScore(FStateTreeExecutionContext& Context) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

    // 创建评分上下文
    FUtilityContext UtilityContext = CreateUtilityContext(Context);

    // 计算评分（使用缓存）
    FUtilityScore NewScore = CalculateUtilityScoreWithCache(InstanceData.ScoringProfile, UtilityContext);

    // 更新实例数据
    InstanceData.CalculatedScore = NewScore;
    InstanceData.bTaskCompleted = NewScore.bIsValid;

    if (bEnableDebugOutput)
    {
        LogDebug(FString::Printf(TEXT("UniversalUtilityTask: Updated score to %.3f (Valid: %s)"), 
                                NewScore.FinalScore, NewScore.bIsValid ? TEXT("Yes") : TEXT("No")));

        // 显示详细评分信息
        for (const auto& Pair : NewScore.ConsiderationScores)
        {
            FString TypeName = UEnum::GetValueAsString(Pair.Key);
            LogDebug(FString::Printf(TEXT("  %s: %.3f"), *TypeName, Pair.Value));
        }
    }

    return NewScore.bIsValid;
}

bool FStateTreeUniversalUtilityTask::ShouldUpdate(const FInstanceDataType& InstanceData, float CurrentTime) const
{
    if (InstanceData.LastUpdateTime < 0.0f)
    {
        return true; // 首次更新
    }

    return (CurrentTime - InstanceData.LastUpdateTime) >= InstanceData.UpdateInterval;
}

#if WITH_EDITOR
FText FStateTreeUniversalUtilityTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
    const FInstanceDataType* InstanceData = InstanceDataView.GetPtr<FInstanceDataType>();
    if (InstanceData)
    {
        return FText::FromString(FString::Printf(TEXT("Universal Utility: %s"), *InstanceData->ScoringProfile.ProfileName));
    }
    return NSLOCTEXT("StateTreeEditor", "UniversalUtility", "Universal Utility Scorer");
}
#endif

// === FStateTreeUtilityConsiderationTask 实现 ===

EStateTreeRunStatus FStateTreeUtilityConsiderationTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

    if (!InstanceData.EnemyCharacter)
    {
        LogDebug(TEXT("UtilityConsiderationTask: EnemyCharacter is null"));
        InstanceData.ErrorMessage = TEXT("No EnemyCharacter found");
        return EStateTreeRunStatus::Failed;
    }

    // 创建评分上下文
    FUtilityContext UtilityContext = CreateUtilityContext(Context);

    // 获取输入值
    InstanceData.InputValue = UtilityContext.GetInputValue(InstanceData.Consideration.ConsiderationType);

    // 计算单项评分
    InstanceData.ConsiderationScore = InstanceData.Consideration.CalculateScore(UtilityContext);

    // 检查评分有效性
    InstanceData.bScoreValid = InstanceData.ConsiderationScore >= InstanceData.ValidScoreThreshold;
    InstanceData.bTaskCompleted = InstanceData.bScoreValid;

    if (bEnableDebugOutput)
    {
        FString TypeName = UEnum::GetValueAsString(InstanceData.Consideration.ConsiderationType);
        LogDebug(FString::Printf(TEXT("UtilityConsideration[%s]: Input=%.3f, Score=%.3f, Valid=%s"), 
                                *TypeName, InstanceData.InputValue, InstanceData.ConsiderationScore,
                                InstanceData.bScoreValid ? TEXT("Yes") : TEXT("No")));
    }

    return InstanceData.bScoreValid ? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Failed;
}

#if WITH_EDITOR
FText FStateTreeUtilityConsiderationTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
    const FInstanceDataType* InstanceData = InstanceDataView.GetPtr<FInstanceDataType>();
    if (InstanceData)
    {
        FString TypeName = UEnum::GetValueAsString(InstanceData->Consideration.ConsiderationType);
        return FText::FromString(FString::Printf(TEXT("Consideration: %s"), *TypeName));
    }
    return NSLOCTEXT("StateTreeEditor", "UtilityConsideration", "Utility Consideration");
}
#endif

// === FStateTreeUtilityComparisonTask 实现 ===

EStateTreeRunStatus FStateTreeUtilityComparisonTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

    if (!InstanceData.EnemyCharacter)
    {
        LogDebug(TEXT("UtilityComparisonTask: EnemyCharacter is null"));
        InstanceData.ErrorMessage = TEXT("No EnemyCharacter found");
        return EStateTreeRunStatus::Failed;
    }

    // 创建评分上下文
    FUtilityContext UtilityContext = CreateUtilityContext(Context);

    // 计算两个配置的评分
    InstanceData.ScoreA = CalculateUtilityScoreWithCache(InstanceData.ProfileA, UtilityContext);
    InstanceData.ScoreB = CalculateUtilityScoreWithCache(InstanceData.ProfileB, UtilityContext);

    // 比较评分
    InstanceData.bIsABetter = InstanceData.ScoreA.IsBetterThan(InstanceData.ScoreB);
    InstanceData.BetterScore = InstanceData.bIsABetter ? InstanceData.ScoreA : InstanceData.ScoreB;
    InstanceData.BetterProfileName = InstanceData.bIsABetter ? InstanceData.ProfileA.ProfileName : InstanceData.ProfileB.ProfileName;
    InstanceData.ScoreDifference = FMath::Abs(InstanceData.ScoreA.FinalScore - InstanceData.ScoreB.FinalScore);

    InstanceData.bTaskCompleted = InstanceData.BetterScore.bIsValid;

    if (bEnableDebugOutput)
    {
        LogDebug(FString::Printf(TEXT("UtilityComparison: %s(%.3f) vs %s(%.3f) -> %s wins (diff: %.3f)"), 
                                *InstanceData.ProfileA.ProfileName, InstanceData.ScoreA.FinalScore,
                                *InstanceData.ProfileB.ProfileName, InstanceData.ScoreB.FinalScore,
                                *InstanceData.BetterProfileName, InstanceData.ScoreDifference));
    }

    return InstanceData.BetterScore.bIsValid ? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Failed;
}

#if WITH_EDITOR
FText FStateTreeUtilityComparisonTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
    const FInstanceDataType* InstanceData = InstanceDataView.GetPtr<FInstanceDataType>();
    if (InstanceData)
    {
        return FText::FromString(FString::Printf(TEXT("Compare: %s vs %s"), 
                                                *InstanceData->ProfileA.ProfileName,
                                                *InstanceData->ProfileB.ProfileName));
    }
    return NSLOCTEXT("StateTreeEditor", "UtilityComparison", "Utility Score Comparison");
}
#endif

// === FStateTreeDynamicUtilityTask 实现 ===

EStateTreeRunStatus FStateTreeDynamicUtilityTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

    if (!InstanceData.EnemyCharacter)
    {
        LogDebug(TEXT("DynamicUtilityTask: EnemyCharacter is null"));
        InstanceData.ErrorMessage = TEXT("No EnemyCharacter found");
        return EStateTreeRunStatus::Failed;
    }

    // 创建评分上下文
    FUtilityContext UtilityContext = CreateUtilityContext(Context);

    // 计算动态权重调整
    CalculateDynamicWeights(UtilityContext, InstanceData);

    // 应用调整后的权重计算评分
    FUtilityProfile AdjustedProfile = InstanceData.BaseProfile;
    for (const auto& Pair : InstanceData.CurrentWeights)
    {
        AdjustedProfile.SetWeight(Pair.Key, Pair.Value);
    }

    InstanceData.AdjustedScore = CalculateUtilityScoreWithCache(AdjustedProfile, UtilityContext);
    InstanceData.bTaskCompleted = InstanceData.AdjustedScore.bIsValid;

    if (bEnableDebugOutput)
    {
        LogDebug(FString::Printf(TEXT("DynamicUtility[%s]: Adjusted score %.3f"), 
                                *InstanceData.BaseProfile.ProfileName, InstanceData.AdjustedScore.FinalScore));
        
        for (const auto& Pair : InstanceData.CurrentWeights)
        {
            FString TypeName = UEnum::GetValueAsString(Pair.Key);
            LogDebug(FString::Printf(TEXT("  Weight[%s]: %.3f"), *TypeName, Pair.Value));
        }
    }

    return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FStateTreeDynamicUtilityTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

    if (!InstanceData.bUseDynamicAdjustment)
    {
        return EStateTreeRunStatus::Succeeded;
    }

    // 持续更新动态权重
    FUtilityContext UtilityContext = CreateUtilityContext(Context);
    CalculateDynamicWeights(UtilityContext, InstanceData);

    // 重新计算评分
    FUtilityProfile AdjustedProfile = InstanceData.BaseProfile;
    for (const auto& Pair : InstanceData.CurrentWeights)
    {
        AdjustedProfile.SetWeight(Pair.Key, Pair.Value);
    }

    InstanceData.AdjustedScore = CalculateUtilityScoreWithCache(AdjustedProfile, UtilityContext);

    return EStateTreeRunStatus::Running;
}

void FStateTreeDynamicUtilityTask::CalculateDynamicWeights(const FUtilityContext& UtilityContext, FInstanceDataType& InstanceData) const
{
    // 获取基础权重
    InstanceData.CurrentWeights = InstanceData.BaseProfile.Weights;

    if (!InstanceData.bUseDynamicAdjustment)
    {
        return;
    }

    // 应用动态调整
    for (auto& WeightPair : InstanceData.CurrentWeights)
    {
        EConsiderationType Type = WeightPair.Key;
        float& CurrentWeight = WeightPair.Value;

        // 查找是否有预设的调整
        if (const float* Adjustment = InstanceData.WeightAdjustments.Find(Type))
        {
            CurrentWeight *= *Adjustment;
        }

        // 基于当前情况的动态调整
        switch (Type)
        {
        case EConsiderationType::Health:
            // 健康度越低，权重越高
            CurrentWeight *= (1.0f + (1.0f - UtilityContext.HealthPercent));
            break;

        case EConsiderationType::Distance:
            // 根据距离调整权重
            if (UtilityContext.DistanceToTarget < 300.0f) // 近距离
            {
                CurrentWeight *= 1.5f; // 增加距离考虑的权重
            }
            break;

        case EConsiderationType::ElementAdvantage:
            // 如果有元素优势，增加权重
            if (UtilityContext.ElementAdvantage > 0.0f)
            {
                CurrentWeight *= (1.0f + UtilityContext.ElementAdvantage);
            }
            break;

        case EConsiderationType::ThreatLevel:
            // 高威胁时增加权重
            CurrentWeight *= (1.0f + UtilityContext.ThreatLevel);
            break;
        }

        // 确保权重不为负数
        CurrentWeight = FMath::Max(0.0f, CurrentWeight);
    }
}

#if WITH_EDITOR
FText FStateTreeDynamicUtilityTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
    const FInstanceDataType* InstanceData = InstanceDataView.GetPtr<FInstanceDataType>();
    if (InstanceData)
    {
        return FText::FromString(FString::Printf(TEXT("Dynamic Utility: %s"), *InstanceData->BaseProfile.ProfileName));
    }
    return NSLOCTEXT("StateTreeEditor", "DynamicUtility", "Dynamic Utility Weights");
}
#endif