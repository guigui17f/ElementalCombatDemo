// Copyright 2025 guigui17f. All Rights Reserved.

#include "StateTreeEQSTasks.h"
#include "StateTreeExecutionContext.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "AIController.h"
#include "AI/ElementalCombatAIController.h"
#include "Engine/World.h"

// === FStateTreeEQSQueryTask 实现 ===

EStateTreeRunStatus FStateTreeEQSQueryTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
    
    // 验证查询模板
    if (!InstanceData.QueryTemplate)
    {
        InstanceData.bQuerySucceeded = false;
        InstanceData.QueryError = TEXT("未指定查询模板");
        return EStateTreeRunStatus::Failed;
    }

    // 开始EQS查询
    if (!StartEQSQuery(Context))
    {
        InstanceData.bQuerySucceeded = false;
        InstanceData.QueryError = TEXT("启动EQS查询失败");
        return EStateTreeRunStatus::Failed;
    }

    LogDebug(TEXT("EQS查询：开始查询"));
    return InstanceData.bRunAsync ? EStateTreeRunStatus::Running : EStateTreeRunStatus::Succeeded;
}

EStateTreeRunStatus FStateTreeEQSQueryTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
    
    if (!InstanceData.bIsQueryRunning)
    {
        return EStateTreeRunStatus::Succeeded;
    }

    // 使用简化方法检查超时
    if (InstanceData.QueryStartTime > 0 && 
        FPlatformTime::Seconds() - InstanceData.QueryStartTime > InstanceData.QueryTimeout)
    {
        InstanceData.bQuerySucceeded = false;
        InstanceData.QueryError = TEXT("查询超时");
        InstanceData.bIsQueryRunning = false;
        return EStateTreeRunStatus::Failed;
    }

    // 检查查询完成
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
        LogDebug(TEXT("开始EQS查询：未指定查询模板"));
        return false;
    }

    UWorld* World = AIController->GetWorld();
    if (!World)
    {
        LogDebug(TEXT("开始EQS查询：未找到世界"));
        return false;
    }

    UEnvQueryManager* EQSManager = UEnvQueryManager::GetCurrent(World);
    if (!EQSManager)
    {
        LogDebug(TEXT("开始EQS查询：未找到EQS管理器"));
        return false;
    }

    APawn* QueryPawn = AIController->GetPawn();
    if (!QueryPawn)
    {
        LogDebug(TEXT("开始EQS查询：未找到Pawn"));
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
            LogDebug(FString::Printf(TEXT("EQS异步查询启动，ID: %d"), InstanceData.QueryRequestID));
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
            LogDebug(FString::Printf(TEXT("EQS同步查询完成：%d 个结果"), QueryResults.Num()));
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
        InstanceData.QueryError = TEXT("未找到有效位置");
    }
}

void FStateTreeEQSQueryTask::OnEQSQueryComplete(TSharedPtr<FEnvQueryResult> QueryResult) const
{
    if (!QueryResult.IsValid())
    {
        if (bEnableDebugOutput)
        {
            LogDebug(TEXT("EQS查询完成：查询结果无效"));
        }
        return;
    }

    TArray<FVector> QueryResults;

    if (QueryResult->IsSuccessful())
    {
        QueryResult->GetAllAsLocations(QueryResults);

        if (bEnableDebugOutput)
        {
            LogDebug(FString::Printf(TEXT("EQS查询成功完成：%d 个结果"), QueryResults.Num()));
        }
    }
    else
    {
        if (bEnableDebugOutput)
        {
            LogDebug(TEXT("EQS查询失败"));
        }
    }

    // 注意：这里需要访问任务实例数据，在实际实现中可能需要额外的同步机制
    // ProcessQueryResults(QueryResults, InstanceData);
}

#if WITH_EDITOR
FText FStateTreeEQSQueryTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
    return FText::FromString(TEXT("执行EQS查询"));
}
#endif

// === FStateTreeEQSUtilityTask 实现 ===

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
            LogDebug(FString::Printf(TEXT("EQS效用：处理 %d 个位置（共 %d 个）"),
                                    MaxPositions, QueryResults.Num()));
        }

        // 处理查询结果并进行Utility评分
        ProcessEQSResults(QueryResults, Context);
    }
    else
    {
        if (bEnableDebugOutput)
        {
            LogDebug(TEXT("EQS效用：EQS查询失败或无结果"));
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
        float PositionScore = EvaluatePosition(Location, BaseContext, Context);
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

float FStateTreeEQSUtilityTask::EvaluatePosition(const FVector& Position, const FUtilityContext& BaseContext, FStateTreeExecutionContext& Context) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

    // 获取AIController配置
    AElementalCombatAIController* AIController = Cast<AElementalCombatAIController>(InstanceData.AIController);
    if (!AIController)
    {
        return 0.0f;
    }
    const FUtilityProfile& AIProfile = AIController->GetCurrentAIProfile();

    // Create context for this specific position
    FUtilityContext PositionContext = BaseContext;
    PositionContext.SetCustomValue(TEXT("PositionDistance"), FVector::Dist(Position, BaseContext.SelfActor.IsValid() ? BaseContext.SelfActor->GetActorLocation() : FVector::ZeroVector));

    return AIProfile.CalculateScore(PositionContext);
}

#if WITH_EDITOR
FText FStateTreeEQSUtilityTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
    return FText::FromString(TEXT("EQS + 效用位置评分"));
}
#endif

// === FStateTreeEQSCacheManagerTask 实现 ===

EStateTreeRunStatus FStateTreeEQSCacheManagerTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
    
    if (InstanceData.bClearAllCaches)
    {
        // 清理所有EQS和效用缓存
        ClearAllCaches();
        InstanceData.CacheStatsInfo = TEXT("所有缓存已清理");
    }
    
    if (InstanceData.bClearExpiredCaches)
    {
        // 清理过期缓存
        ClearExpiredCaches(FPlatformTime::Seconds());
        InstanceData.CacheStatsInfo += TEXT(" | 过期缓存已清理");
    }
    
    if (InstanceData.bShowCacheStats)
    {
        // 显示缓存统计
        InstanceData.CacheStatsInfo += TEXT(" | 缓存统计可用");
    }
    
    return EStateTreeRunStatus::Succeeded;
}

#if WITH_EDITOR
FText FStateTreeEQSCacheManagerTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
    return FText::FromString(TEXT("管理EQS缓存"));
}
#endif