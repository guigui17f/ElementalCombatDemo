// Copyright 2025 guigui17f. All Rights Reserved.

#include "StateTreeUtilityTasks.h"
#include "StateTreeExecutionContext.h"
#include "ElementalCombatAIController.h"
#include "ElementalCombatEnemy.h"

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
        LogDebug(TEXT("UniversalUtilityTask: Starting utility evaluation"));
    }

    // 初始化评分计算
    if (InstanceData.bRecalculateOnEnter || !InstanceData.bScoreValid)
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
                                InstanceData.FinalScore));
    }
}

bool FStateTreeUniversalUtilityTask::UpdateScore(FStateTreeExecutionContext& Context) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

    // 从AIController获取配置
    AElementalCombatAIController* AIController = Cast<AElementalCombatAIController>(InstanceData.EnemyCharacter->GetController());
    if (!AIController)
    {
        UE_LOG(LogTemp, Error, TEXT("UniversalUtilityTask: AIController is null or not ElementalCombatAIController"));
        return false;
    }
    
    const FUtilityProfile& AIProfile = AIController->GetCurrentAIProfile();

    // 创建评分上下文
    FUtilityContext UtilityContext = CreateUtilityContext(Context);

    // 使用从AIController获取的配置计算评分
    float NewScore = CalculateUtilityScoreWithCache(AIProfile, UtilityContext);

    // 更新实例数据
    InstanceData.FinalScore = NewScore;
    InstanceData.bTaskCompleted = NewScore > 0.01f;

    if (bEnableDebugOutput)
    {
        LogDebug(FString::Printf(TEXT("UniversalUtilityTask: Updated score to %.3f (Valid: %s)"), 
                                NewScore, NewScore > 0.01f ? TEXT("Yes") : TEXT("No")));

        // 显示上下文信息
        LogDebug(FString::Printf(TEXT("  Health: %.3f, Distance: %.3f, Element: %.3f"), 
                                UtilityContext.HealthPercent, UtilityContext.DistanceToTarget, UtilityContext.ElementAdvantage));
    }

    return NewScore > 0.01f;
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
    return NSLOCTEXT("StateTreeEditor", "UniversalUtility", "Universal Utility Scorer (使用AIController配置)");
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

    // 获取AIController配置
    AElementalCombatAIController* AIController = Cast<AElementalCombatAIController>(InstanceData.AIController);
    if (!AIController)
    {
        InstanceData.ErrorMessage = TEXT("No ElementalCombatAIController found");
        return EStateTreeRunStatus::Failed;
    }

    // 创建评分上下文
    FUtilityContext UtilityContext = CreateUtilityContext(Context);

    if (InstanceData.bUseWeightVariations)
    {
        // 使用权重变化模式：基于AIController配置进行两种不同的权重调整
        FUtilityProfile ProfileA = AIController->GetCurrentAIProfile();
        FUtilityProfile ProfileB = AIController->GetCurrentAIProfile();

        // 应用权重变化A
        for (const auto& WeightPair : InstanceData.WeightVariationA)
        {
            ProfileA.SetWeight(WeightPair.Key, ProfileA.GetWeight(WeightPair.Key) * WeightPair.Value);
        }

        // 应用权重变化B
        for (const auto& WeightPair : InstanceData.WeightVariationB)
        {
            ProfileB.SetWeight(WeightPair.Key, ProfileB.GetWeight(WeightPair.Key) * WeightPair.Value);
        }

        // 计算两个配置的评分
        InstanceData.ScoreA = CalculateUtilityScoreWithCache(ProfileA, UtilityContext);
        InstanceData.ScoreB = CalculateUtilityScoreWithCache(ProfileB, UtilityContext);

        // 比较评分
        InstanceData.bIsABetter = InstanceData.ScoreA > InstanceData.ScoreB;
        InstanceData.FinalScore = InstanceData.bIsABetter ? InstanceData.ScoreA : InstanceData.ScoreB;
        InstanceData.BetterProfileName = InstanceData.bIsABetter ? TEXT("VariationA") : TEXT("VariationB");
        InstanceData.ScoreDifference = FMath::Abs(InstanceData.ScoreA - InstanceData.ScoreB);

        InstanceData.bTaskCompleted = InstanceData.FinalScore > 0.01f;

        if (bEnableDebugOutput)
        {
            LogDebug(FString::Printf(TEXT("UtilityComparison: VariationA(%.3f) vs VariationB(%.3f) -> %s wins (diff: %.3f)"),
                                    InstanceData.ScoreA, InstanceData.ScoreB,
                                    *InstanceData.BetterProfileName, InstanceData.ScoreDifference));
        }
    }
    else
    {
        // 降级模式：只使用基础配置
        const FUtilityProfile& BaseProfile = AIController->GetCurrentAIProfile();
        InstanceData.ScoreA = CalculateUtilityScoreWithCache(BaseProfile, UtilityContext);
        InstanceData.ScoreB = InstanceData.ScoreA; // 相同配置
        InstanceData.bIsABetter = true;
        InstanceData.FinalScore = InstanceData.ScoreA;
        InstanceData.BetterProfileName = BaseProfile.ProfileName;
        InstanceData.ScoreDifference = 0.0f;
        InstanceData.bTaskCompleted = InstanceData.FinalScore > 0.01f;

        if (bEnableDebugOutput)
        {
            LogDebug(FString::Printf(TEXT("UtilityComparison: Using base profile %s (%.3f)"),
                                    *BaseProfile.ProfileName, InstanceData.ScoreA));
        }
    }

    return InstanceData.FinalScore > 0.01f ? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Failed;
}

#if WITH_EDITOR
FText FStateTreeUtilityComparisonTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
    const FInstanceDataType* InstanceData = InstanceDataView.GetPtr<FInstanceDataType>();
    if (InstanceData && InstanceData->bUseWeightVariations)
    {
        return FText::FromString(TEXT("Compare: Weight Variations A vs B"));
    }
    return NSLOCTEXT("StateTreeEditor", "UtilityComparison", "Utility Score Comparison (AIController Based)");
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

    // 获取AIController配置
    AElementalCombatAIController* AIController = Cast<AElementalCombatAIController>(InstanceData.AIController);
    if (!AIController)
    {
        InstanceData.ErrorMessage = TEXT("No ElementalCombatAIController found");
        return EStateTreeRunStatus::Failed;
    }

    // 计算动态权重调整
    CalculateDynamicWeights(UtilityContext, InstanceData);

    // 应用调整后的权重计算评分
    FUtilityProfile AdjustedProfile = AIController->GetCurrentAIProfile();
    for (const auto& Pair : InstanceData.CurrentWeights)
    {
        AdjustedProfile.SetWeight(Pair.Key, Pair.Value);
    }

    InstanceData.FinalScore = CalculateUtilityScoreWithCache(AdjustedProfile, UtilityContext);
    InstanceData.bTaskCompleted = InstanceData.FinalScore > 0.01f;

    if (bEnableDebugOutput)
    {
        LogDebug(FString::Printf(TEXT("DynamicUtility[%s]: Adjusted score %.3f"),
                                *AdjustedProfile.ProfileName, InstanceData.FinalScore));
        
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

    // 获取AIController配置
    AElementalCombatAIController* AIController = Cast<AElementalCombatAIController>(InstanceData.AIController);
    if (!AIController)
    {
        return EStateTreeRunStatus::Failed;
    }

    // 持续更新动态权重
    FUtilityContext UtilityContext = CreateUtilityContext(Context);
    CalculateDynamicWeights(UtilityContext, InstanceData);

    // 重新计算评分
    FUtilityProfile AdjustedProfile = AIController->GetCurrentAIProfile();
    for (const auto& Pair : InstanceData.CurrentWeights)
    {
        AdjustedProfile.SetWeight(Pair.Key, Pair.Value);
    }

    InstanceData.FinalScore = CalculateUtilityScoreWithCache(AdjustedProfile, UtilityContext);

    return EStateTreeRunStatus::Running;
}

void FStateTreeDynamicUtilityTask::CalculateDynamicWeights(const FUtilityContext& UtilityContext, FInstanceDataType& InstanceData) const
{
    // 获取AIController配置作为基础权重
    AElementalCombatAIController* AIController = Cast<AElementalCombatAIController>(InstanceData.AIController);
    if (!AIController)
    {
        return;
    }
    const FUtilityProfile& AIProfile = AIController->GetCurrentAIProfile();
    InstanceData.CurrentWeights = AIProfile.Weights;

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
        return FText::FromString(TEXT("Dynamic Utility: AIController Config"));
    }
    return NSLOCTEXT("StateTreeEditor", "DynamicUtility", "Dynamic Utility Weights");
}
#endif