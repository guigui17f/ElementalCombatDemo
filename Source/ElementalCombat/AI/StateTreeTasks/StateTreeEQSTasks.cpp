// Copyright 2025 guigui17f. All Rights Reserved.

#include "StateTreeEQSTasks.h"
#include "StateTreeExecutionContext.h"
#include "AI/ElementalCombatEnemy.h"
#include "Engine/World.h"

// === FStateTreeEQSQueryTask 实现 ===

EStateTreeRunStatus FStateTreeEQSQueryTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

    if (!InstanceData.EnemyCharacter || !InstanceData.QueryTemplate)
    {
        InstanceData.ErrorMessage = TEXT("Missing EnemyCharacter or QueryTemplate");
        return EStateTreeRunStatus::Failed;
    }

    // 执行EQS查询
    TArray<FVector> QueryResults;
    FVector BestResult;
    if (!ExecuteEQSQueryWithCache(InstanceData.QueryTemplate, Context, QueryResults, BestResult))
    {
        InstanceData.ErrorMessage = TEXT("EQS query failed");
        return EStateTreeRunStatus::Failed;
    }
    
    InstanceData.ResultLocations = QueryResults;
    InstanceData.BestLocation = BestResult;
    InstanceData.bQuerySucceeded = true;

    return EStateTreeRunStatus::Succeeded;
}

EStateTreeRunStatus FStateTreeEQSQueryTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
    // Query already completed in EnterState
    return EStateTreeRunStatus::Succeeded;
}

void FStateTreeEQSQueryTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    // Cleanup if needed
}

#if WITH_EDITOR
FText FStateTreeEQSQueryTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
    const FInstanceDataType* InstanceData = InstanceDataView.GetPtr<FInstanceDataType>();
    if (InstanceData && InstanceData->QueryTemplate)
    {
        return FText::FromString(FString::Printf(TEXT("EQS Query: %s"), *InstanceData->QueryTemplate->GetName()));
    }
    return NSLOCTEXT("StateTreeEditor", "EQSQuery", "EQS Query");
}
#endif

// === FStateTreeEQSTacticalQueryTask 实现 ===

EStateTreeRunStatus FStateTreeEQSTacticalQueryTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

    if (!InstanceData.EnemyCharacter)
    {
        InstanceData.ErrorMessage = TEXT("Missing EnemyCharacter");
        return EStateTreeRunStatus::Failed;
    }

    // 选择查询类型
    FUtilityContext UtilityContext = CreateUtilityContext(Context);
    EAIAttackType QueryType = InstanceData.bAutoSelectQueryType ? 
        SelectQueryType(UtilityContext, InstanceData) : InstanceData.ForcedQueryType;
    
    UEnvQuery* SelectedQuery = GetQueryForType(QueryType, InstanceData);
    if (!SelectedQuery)
    {
        InstanceData.ErrorMessage = TEXT("No query available for selected type");
        return EStateTreeRunStatus::Failed;
    }

    // 执行EQS查询
    TArray<FVector> QueryResults;
    FVector BestResult;
    if (!ExecuteEQSQueryWithCache(SelectedQuery, Context, QueryResults, BestResult))
    {
        InstanceData.ErrorMessage = TEXT("EQS tactical query failed");
        return EStateTreeRunStatus::Failed;
    }
    
    // 更新结果
    InstanceData.RecommendedPosition = BestResult;
    InstanceData.UsedQueryType = QueryType;
    InstanceData.AlternativePositions = QueryResults;
    InstanceData.PositionQuality = QueryResults.Num() > 0 ? 1.0f : 0.0f;
    InstanceData.bTaskCompleted = true;

    return EStateTreeRunStatus::Succeeded;
}

EStateTreeRunStatus FStateTreeEQSTacticalQueryTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
    // Query already completed in EnterState
    return EStateTreeRunStatus::Succeeded;
}

EAIAttackType FStateTreeEQSTacticalQueryTask::SelectQueryType(const FUtilityContext& UtilityContext, const FInstanceDataType& InstanceData) const
{
    // 简化的查询类型选择逻辑
    if (UtilityContext.DistanceToTarget > 500.0f)
    {
        return EAIAttackType::Ranged; // 距离远时选择远程
    }
    else
    {
        return EAIAttackType::Melee; // 默认近战
    }
}

UEnvQuery* FStateTreeEQSTacticalQueryTask::GetQueryForType(EAIAttackType QueryType, const FInstanceDataType& InstanceData) const
{
    switch (QueryType)
    {
    case EAIAttackType::Melee:
        return InstanceData.MeleePositionQuery.Get();
    case EAIAttackType::Ranged:
        return InstanceData.RangedPositionQuery.Get();
    default:
        return InstanceData.CoverPositionQuery.Get();
    }
}

#if WITH_EDITOR
FText FStateTreeEQSTacticalQueryTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
    return NSLOCTEXT("StateTreeEditor", "EQSTactical", "EQS Tactical Query");
}
#endif

// === FStateTreeEQSUtilityTask 实现 ===

EStateTreeRunStatus FStateTreeEQSUtilityTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

    if (!InstanceData.EnemyCharacter)
    {
        InstanceData.ErrorMessage = TEXT("Missing EnemyCharacter");
        return EStateTreeRunStatus::Failed;
    }

    // 简化实现 - 直接返回成功
    InstanceData.bTaskCompleted = true;
    return EStateTreeRunStatus::Succeeded;
}

EStateTreeRunStatus FStateTreeEQSUtilityTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
    return EStateTreeRunStatus::Succeeded;
}

#if WITH_EDITOR
FText FStateTreeEQSUtilityTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
    return NSLOCTEXT("StateTreeEditor", "EQSUtility", "EQS Utility Query");
}
#endif

// === FStateTreeEQSCacheManagerTask 实现 ===

EStateTreeRunStatus FStateTreeEQSCacheManagerTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

    if (!InstanceData.EnemyCharacter)
    {
        InstanceData.ErrorMessage = TEXT("Missing EnemyCharacter");
        return EStateTreeRunStatus::Failed;
    }

    // 简化缓存管理
    InstanceData.bTaskCompleted = true;
    return EStateTreeRunStatus::Succeeded;
}

#if WITH_EDITOR
FText FStateTreeEQSCacheManagerTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
    return NSLOCTEXT("StateTreeEditor", "EQSCacheManager", "EQS Cache Manager");
}
#endif