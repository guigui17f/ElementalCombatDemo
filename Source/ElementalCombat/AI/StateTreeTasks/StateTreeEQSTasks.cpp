// Copyright 2025 guigui17f. All Rights Reserved.

#include "StateTreeEQSTasks.h"
#include "StateTreeExecutionContext.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "AIController.h"
#include "Engine/World.h"

// === FStateTreeEQSQueryTask Implementation ===

EStateTreeRunStatus FStateTreeEQSQueryTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
    
    // Validate query template
    if (!InstanceData.QueryTemplate)
    {
        InstanceData.bQuerySucceeded = false;
        InstanceData.QueryError = TEXT("No query template specified");
        return EStateTreeRunStatus::Failed;
    }

    // Start EQS query
    if (!StartEQSQuery(Context))
    {
        InstanceData.bQuerySucceeded = false;
        InstanceData.QueryError = TEXT("Failed to start EQS query");
        return EStateTreeRunStatus::Failed;
    }

    LogDebug(TEXT("EQSQuery: Started query"));
    return InstanceData.bRunAsync ? EStateTreeRunStatus::Running : EStateTreeRunStatus::Succeeded;
}

EStateTreeRunStatus FStateTreeEQSQueryTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
    
    if (!InstanceData.bIsQueryRunning)
    {
        return EStateTreeRunStatus::Succeeded;
    }

    // Check for timeout using simplified approach  
    if (InstanceData.QueryStartTime > 0 && 
        FPlatformTime::Seconds() - InstanceData.QueryStartTime > InstanceData.QueryTimeout)
    {
        InstanceData.bQuerySucceeded = false;
        InstanceData.QueryError = TEXT("Query timed out");
        InstanceData.bIsQueryRunning = false;
        return EStateTreeRunStatus::Failed;
    }

    // Check query completion
    if (CheckQueryCompletion(Context))
    {
        InstanceData.bIsQueryRunning = false;
        return EStateTreeRunStatus::Succeeded;
    }

    return EStateTreeRunStatus::Running;
}

void FStateTreeEQSQueryTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
    InstanceData.bIsQueryRunning = false;
}

bool FStateTreeEQSQueryTask::StartEQSQuery(FStateTreeExecutionContext& Context) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
    AAIController* AIController = GetAIController(Context);
    
    if (!AIController)
    {
        return false;
    }

    // 检查是否有查询模板
    if (!InstanceData.QueryTemplate)
    {
        LogDebug(TEXT("StartEQSQuery: No QueryTemplate specified"));
        return false;
    }

    UWorld* World = AIController->GetWorld();
    if (!World)
    {
        LogDebug(TEXT("StartEQSQuery: No World found"));
        return false;
    }

    UEnvQueryManager* EQSManager = UEnvQueryManager::GetCurrent(World);
    if (!EQSManager)
    {
        LogDebug(TEXT("StartEQSQuery: No EQS Manager found"));
        return false;
    }

    APawn* QueryPawn = AIController->GetPawn();
    if (!QueryPawn)
    {
        LogDebug(TEXT("StartEQSQuery: No Pawn found"));
        return false;
    }

    InstanceData.QueryStartTime = FPlatformTime::Seconds();
    InstanceData.bIsQueryRunning = true;

    // 创建并执行EQS查询
    FEnvQueryRequest QueryRequest(InstanceData.QueryTemplate, QueryPawn);

    // 根据异步设置选择执行方式
    if (InstanceData.bRunAsync)
    {
        // 异步查询 - 使用Lambda委托
        FQueryFinishedSignature QueryFinishedDelegate;
        QueryFinishedDelegate.BindLambda([this](TSharedPtr<FEnvQueryResult> QueryResult)
        {
            OnEQSQueryComplete(QueryResult);
        });

        InstanceData.QueryRequestID = QueryRequest.Execute(EEnvQueryRunMode::AllMatching, QueryFinishedDelegate);

        if (bEnableDebugOutput)
        {
            LogDebug(FString::Printf(TEXT("EQS Async Query started with ID: %d"), InstanceData.QueryRequestID));
        }
    }
    else
    {
        // 同步查询
        TSharedPtr<FEnvQueryResult> QueryResult = EQSManager->RunInstantQuery(QueryRequest, EEnvQueryRunMode::AllMatching);

        TArray<FVector> QueryResults;
        if (QueryResult.IsValid() && QueryResult->IsSuccessful())
        {
            QueryResult->GetAllAsLocations(QueryResults);
        }

        ProcessQueryResults(QueryResults, InstanceData);
        InstanceData.bIsQueryRunning = false;

        if (bEnableDebugOutput)
        {
            LogDebug(FString::Printf(TEXT("EQS Sync Query completed: %d results"), QueryResults.Num()));
        }
    }

    return true;
}

bool FStateTreeEQSQueryTask::CheckQueryCompletion(FStateTreeExecutionContext& Context) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
    return InstanceData.bQuerySucceeded || !InstanceData.QueryError.IsEmpty();
}

void FStateTreeEQSQueryTask::ProcessQueryResults(const TArray<FVector>& Locations, FInstanceDataType& InstanceData) const
{
    InstanceData.ResultLocations = Locations;
    InstanceData.bQuerySucceeded = Locations.Num() > 0;
    
    if (InstanceData.bQuerySucceeded)
    {
        InstanceData.BestLocation = Locations[0];
        InstanceData.QueryError = TEXT("");
    }
    else
    {
        InstanceData.QueryError = TEXT("No valid locations found");
    }
}

void FStateTreeEQSQueryTask::OnEQSQueryComplete(TSharedPtr<FEnvQueryResult> QueryResult) const
{
    if (!QueryResult.IsValid())
    {
        if (bEnableDebugOutput)
        {
            LogDebug(TEXT("OnEQSQueryComplete: QueryResult is invalid"));
        }
        return;
    }

    TArray<FVector> QueryResults;

    if (QueryResult->IsSuccessful())
    {
        QueryResult->GetAllAsLocations(QueryResults);

        if (bEnableDebugOutput)
        {
            LogDebug(FString::Printf(TEXT("EQS Query completed successfully: %d results"), QueryResults.Num()));
        }
    }
    else
    {
        if (bEnableDebugOutput)
        {
            LogDebug(TEXT("EQS Query failed"));
        }
    }

    // 注意：这里需要访问任务实例数据，在实际实现中可能需要额外的同步机制
    // ProcessQueryResults(QueryResults, InstanceData);
}

#if WITH_EDITOR
FText FStateTreeEQSQueryTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
    return FText::FromString(TEXT("Execute EQS Query"));
}
#endif

// === FStateTreeEQSTacticalQueryTask Implementation ===

EStateTreeRunStatus FStateTreeEQSTacticalQueryTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
    FUtilityContext UtilityContext = CreateUtilityContext(Context);

    // Select appropriate query type
    EAIAttackType QueryType = InstanceData.bAutoSelectQueryType ? 
        SelectQueryType(UtilityContext, InstanceData) : InstanceData.ForcedQueryType;
    
    InstanceData.UsedQueryType = QueryType;
    
    // Get query template for the selected type
    UEnvQuery* SelectedQuery = GetQueryForType(QueryType, InstanceData);
    
    if (!SelectedQuery)
    {
        InstanceData.PositionQuality = 0.0f;
        return EStateTreeRunStatus::Failed;
    }

    // 使用真实的EQS查询执行战术位置查询
    TArray<FVector> CandidatePositions;
    FVector BestPosition;

    if (ExecuteEQSQueryWithCache(SelectedQuery, Context, CandidatePositions, BestPosition))
    {
        // 评估所有候选位置的质量
        float BestQuality = 0.0f;
        FVector BestTacticalPosition = BestPosition;

        for (const FVector& Position : CandidatePositions)
        {
            float PositionQuality = EvaluatePositionQuality(Position, UtilityContext);
            if (PositionQuality > BestQuality)
            {
                BestQuality = PositionQuality;
                BestTacticalPosition = Position;
            }
        }

        InstanceData.RecommendedPosition = BestTacticalPosition;
        InstanceData.AlternativePositions = CandidatePositions;
        InstanceData.PositionQuality = BestQuality;

        if (bEnableDebugOutput)
        {
            LogDebug(FString::Printf(TEXT("TacticalQuery: Found %d positions, best quality: %.3f"),
                                    CandidatePositions.Num(), BestQuality));
        }

        return EStateTreeRunStatus::Succeeded;
    }
    else
    {
        if (bEnableDebugOutput)
        {
            LogDebug(TEXT("TacticalQuery: EQS query failed or returned no results"));
        }

        InstanceData.PositionQuality = 0.0f;
        return EStateTreeRunStatus::Failed;
    }
}

EStateTreeRunStatus FStateTreeEQSTacticalQueryTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
    return EStateTreeRunStatus::Succeeded;
}

EAIAttackType FStateTreeEQSTacticalQueryTask::SelectQueryType(const FUtilityContext& UtilityContext, const FInstanceDataType& InstanceData) const
{
    // 让配置决定查询类型，不再硬编码距离判断
    if (InstanceData.ForcedQueryType != EAIAttackType::None)
    {
        return InstanceData.ForcedQueryType;
    }

    // 默认返回近战类型，让StateTree和EQS配置控制具体行为
    return EAIAttackType::Melee;
}

UEnvQuery* FStateTreeEQSTacticalQueryTask::GetQueryForType(EAIAttackType QueryType, const FInstanceDataType& InstanceData) const
{
    switch (QueryType)
    {
    case EAIAttackType::Melee:
        return InstanceData.MeleePositionQuery;
    case EAIAttackType::Ranged:
        return InstanceData.RangedPositionQuery;
    default:
        return InstanceData.MeleePositionQuery;
    }
}

float FStateTreeEQSTacticalQueryTask::EvaluatePositionQuality(const FVector& Position, const FUtilityContext& Context) const
{
    // Simple quality evaluation based on distance
    float Distance = FVector::Dist(Position, Context.TargetActor.IsValid() ? Context.TargetActor->GetActorLocation() : FVector::ZeroVector);
    return FMath::Clamp(1.0f - (Distance / 1000.0f), 0.0f, 1.0f);
}

#if WITH_EDITOR
FText FStateTreeEQSTacticalQueryTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
    return FText::FromString(TEXT("Execute Tactical EQS Query"));
}
#endif

// === FStateTreeEQSUtilityTask Implementation ===

EStateTreeRunStatus FStateTreeEQSUtilityTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
    
    if (!InstanceData.QueryTemplate)
    {
        InstanceData.bFoundValidPosition = false;
        return EStateTreeRunStatus::Failed;
    }

    // 使用真实的EQS查询
    TArray<FVector> QueryResults;
    FVector BestPosition;

    if (ExecuteEQSQueryWithCache(InstanceData.QueryTemplate, Context, QueryResults, BestPosition))
    {
        // 限制评估的位置数量以提高性能
        int32 MaxPositions = FMath::Min(QueryResults.Num(), InstanceData.MaxPositionsToEvaluate);
        if (MaxPositions < QueryResults.Num())
        {
            // 随机选择位置进行评估，保持第一个（最佳位置）
            for (int32 i = MaxPositions; i < QueryResults.Num(); ++i)
            {
                int32 SwapIndex = FMath::RandRange(1, i);
                if (SwapIndex < MaxPositions)
                {
                    QueryResults.Swap(SwapIndex, i);
                }
            }
            QueryResults.SetNum(MaxPositions);
        }

        if (bEnableDebugOutput)
        {
            LogDebug(FString::Printf(TEXT("EQSUtility: Processing %d positions (from %d total)"),
                                    MaxPositions, QueryResults.Num()));
        }

        // 处理查询结果并进行Utility评分
        ProcessEQSResults(QueryResults, Context);
    }
    else
    {
        if (bEnableDebugOutput)
        {
            LogDebug(TEXT("EQSUtility: EQS query failed or returned no results"));
        }

        InstanceData.bFoundValidPosition = false;
    }

    return InstanceData.bFoundValidPosition ? EStateTreeRunStatus::Succeeded : EStateTreeRunStatus::Failed;
}

EStateTreeRunStatus FStateTreeEQSUtilityTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
    return EStateTreeRunStatus::Succeeded;
}

bool FStateTreeEQSUtilityTask::ProcessEQSResults(const TArray<FVector>& Locations, FStateTreeExecutionContext& Context) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
    FUtilityContext BaseContext = CreateUtilityContext(Context);
    
    InstanceData.EvaluatedPositions.Empty();
    float BestScore = 0.0f;
    FVector BestPosition = FVector::ZeroVector;
    
    for (const FVector& Location : Locations)
    {
        float PositionScore = EvaluatePosition(Location, BaseContext, InstanceData.PositionScoringProfile);
        InstanceData.EvaluatedPositions.Add(Location, PositionScore);
        
        if (PositionScore > BestScore && PositionScore >= InstanceData.MinAcceptableScore)
        {
            BestScore = PositionScore;
            BestPosition = Location;
        }
    }
    
    InstanceData.FinalScore = BestScore;
    InstanceData.BestScoredPosition = BestPosition;
    InstanceData.bFoundValidPosition = BestScore >= InstanceData.MinAcceptableScore;
    
    return InstanceData.bFoundValidPosition;
}

float FStateTreeEQSUtilityTask::EvaluatePosition(const FVector& Position, const FUtilityContext& BaseContext, const FUtilityProfile& ScoringProfile) const
{
    // Create context for this specific position
    FUtilityContext PositionContext = BaseContext;
    PositionContext.SetCustomValue(TEXT("PositionDistance"), FVector::Dist(Position, BaseContext.SelfActor.IsValid() ? BaseContext.SelfActor->GetActorLocation() : FVector::ZeroVector));
    
    return ScoringProfile.CalculateScore(PositionContext);
}

#if WITH_EDITOR
FText FStateTreeEQSUtilityTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
    return FText::FromString(TEXT("EQS + Utility Position Scoring"));
}
#endif

// === FStateTreeEQSCacheManagerTask Implementation ===

EStateTreeRunStatus FStateTreeEQSCacheManagerTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
    
    if (InstanceData.bClearAllCaches)
    {
        // Clear all EQS and Utility caches
        ClearAllCaches();
        InstanceData.CacheStatsInfo = TEXT("All caches cleared");
    }
    
    if (InstanceData.bClearExpiredCaches)
    {
        // Clear expired caches
        ClearExpiredCaches(FPlatformTime::Seconds());
        InstanceData.CacheStatsInfo += TEXT(" | Expired caches cleared");
    }
    
    if (InstanceData.bShowCacheStats)
    {
        // Display cache statistics
        InstanceData.CacheStatsInfo += TEXT(" | Cache statistics available");
    }
    
    return EStateTreeRunStatus::Succeeded;
}

#if WITH_EDITOR
FText FStateTreeEQSCacheManagerTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
    return FText::FromString(TEXT("Manage EQS Cache"));
}
#endif