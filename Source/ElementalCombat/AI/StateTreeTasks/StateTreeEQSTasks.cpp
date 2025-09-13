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

    InstanceData.QueryStartTime = FPlatformTime::Seconds();
    InstanceData.bIsQueryRunning = true;
    InstanceData.QueryRequestID = 1; // Simplified ID
    
    // In a real implementation, we would use UEnvQueryManager to start the query
    // For now, we'll simulate a successful query result
    TArray<FVector> SimulatedResults;
    if (APawn* Pawn = AIController->GetPawn())
    {
        FVector PawnLocation = Pawn->GetActorLocation();
        SimulatedResults.Add(PawnLocation + FVector(100, 0, 0));
        SimulatedResults.Add(PawnLocation + FVector(-100, 0, 0));
    }
    
    ProcessQueryResults(SimulatedResults, InstanceData);
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

void FStateTreeEQSQueryTask::OnEQSQueryComplete(UEnvQueryInstanceBlueprintWrapper* QueryInstance, EEnvQueryStatus::Type QueryStatus)
{
    // Callback implementation for real EQS queries
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

    // Execute simplified query (in real implementation, would use EQS)
    TArray<FVector> CandidatePositions;
    if (AAIController* AIController = GetAIController(Context))
    {
        if (APawn* Pawn = AIController->GetPawn())
        {
            FVector PawnLocation = Pawn->GetActorLocation();
            CandidatePositions.Add(PawnLocation + FVector(200, 0, 0));
            CandidatePositions.Add(PawnLocation + FVector(-200, 0, 0));
        }
    }

    if (CandidatePositions.Num() > 0)
    {
        InstanceData.RecommendedPosition = CandidatePositions[0];
        InstanceData.AlternativePositions = CandidatePositions;
        InstanceData.PositionQuality = EvaluatePositionQuality(CandidatePositions[0], UtilityContext);
        return EStateTreeRunStatus::Succeeded;
    }

    return EStateTreeRunStatus::Failed;
}

EStateTreeRunStatus FStateTreeEQSTacticalQueryTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
    return EStateTreeRunStatus::Succeeded;
}

EAIAttackType FStateTreeEQSTacticalQueryTask::SelectQueryType(const FUtilityContext& UtilityContext, const FInstanceDataType& InstanceData) const
{
    // Simple heuristic: use distance to determine attack type
    if (UtilityContext.DistanceToTarget < 300.0f)
    {
        return EAIAttackType::Melee;
    }
    else
    {
        return EAIAttackType::Ranged;
    }
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

    // Execute simplified EQS query
    TArray<FVector> QueryResults;
    if (AAIController* AIController = GetAIController(Context))
    {
        if (APawn* Pawn = AIController->GetPawn())
        {
            FVector PawnLocation = Pawn->GetActorLocation();
            for (int32 i = 0; i < InstanceData.MaxPositionsToEvaluate; ++i)
            {
                float Angle = (2.0f * PI * i) / InstanceData.MaxPositionsToEvaluate;
                FVector Position = PawnLocation + FVector(FMath::Cos(Angle) * 300, FMath::Sin(Angle) * 300, 0);
                QueryResults.Add(Position);
            }
        }
    }

    // Process results with Utility scoring
    ProcessEQSResults(QueryResults, Context);
    
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