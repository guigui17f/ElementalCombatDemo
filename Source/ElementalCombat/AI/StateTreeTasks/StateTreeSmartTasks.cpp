// Copyright 2025 guigui17f. All Rights Reserved.

#include "StateTreeSmartTasks.h"
#include "StateTreeExecutionContext.h"
#include "AI/ElementalCombatEnemy.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"

// === FStateTreeSmartAttackTask 实现 ===

EStateTreeRunStatus FStateTreeSmartAttackTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

    if (!InstanceData.EnemyCharacter)
    {
        InstanceData.ErrorMessage = TEXT("Missing EnemyCharacter");
        return EStateTreeRunStatus::Failed;
    }

    float CurrentTime = GetCurrentWorldTime(Context);
    
    // 检查是否需要重新评估
    if (ShouldReevaluate(InstanceData, CurrentTime))
    {
        if (!EvaluateAttackOptions(Context))
        {
            return EStateTreeRunStatus::Failed;
        }
        
        InstanceData.LastDecisionTime = CurrentTime;
    }

    // 执行选择的攻击
    if (InstanceData.bShouldAttack)
    {
        if (!ExecuteSelectedAttack(Context))
        {
            return EStateTreeRunStatus::Failed;
        }
        
        return EStateTreeRunStatus::Succeeded;
    }

    return EStateTreeRunStatus::Failed;
}

EStateTreeRunStatus FStateTreeSmartAttackTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
    // 大多数情况下攻击是瞬间的，不需要持续Tick
    return EStateTreeRunStatus::Succeeded;
}

void FStateTreeSmartAttackTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    if (bEnableDebugOutput)
    {
        const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
        FString AttackTypeName;
        switch (InstanceData.SelectedAttackType)
        {
        case EAIAttackType::Melee:
            AttackTypeName = TEXT("Melee");
            break;
        case EAIAttackType::Ranged:
            AttackTypeName = TEXT("Ranged");
            break;
        default:
            AttackTypeName = TEXT("None");
            break;
        }
        LogDebug(FString::Printf(TEXT("SmartAttackTask: Completed with attack type '%s'"), 
                                *AttackTypeName));
    }
}

bool FStateTreeSmartAttackTask::EvaluateAttackOptions(FStateTreeExecutionContext& Context) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
    
    // 创建评分上下文
    FUtilityContext UtilityContext = CreateUtilityContext(Context);
    
    // 重置结果
    InstanceData.AttackTypeScores.Empty();
    InstanceData.BestAttackScore.Reset();
    InstanceData.SelectedAttackType = EAIAttackType::None;

    FUtilityScore BestScore;
    EAIAttackType BestAttackType = EAIAttackType::None;

    // 评估近战攻击
    FUtilityScore MeleeScore = CalculateUtilityScoreWithCache(InstanceData.MeleeAttackProfile, UtilityContext);
    InstanceData.AttackTypeScores.Add(EAIAttackType::Melee, MeleeScore.FinalScore);
    if (MeleeScore.IsBetterThan(BestScore))
    {
        BestScore = MeleeScore;
        BestAttackType = EAIAttackType::Melee;
    }

    // 评估远程攻击
    FUtilityScore RangedScore = CalculateUtilityScoreWithCache(InstanceData.RangedAttackProfile, UtilityContext);
    InstanceData.AttackTypeScores.Add(EAIAttackType::Ranged, RangedScore.FinalScore);
    if (RangedScore.IsBetterThan(BestScore))
    {
        BestScore = RangedScore;
        BestAttackType = EAIAttackType::Ranged;
    }

    // 技能攻击已移除，因为EAIAttackType中没有Skill类型

    // 更新结果
    InstanceData.BestAttackScore = BestScore;
    InstanceData.SelectedAttackType = BestAttackType;
    InstanceData.bShouldAttack = BestScore.bIsValid && BestAttackType != EAIAttackType::None;
    InstanceData.DecisionReason = GenerateDecisionReason(InstanceData);

    return InstanceData.bShouldAttack;
}

bool FStateTreeSmartAttackTask::ExecuteSelectedAttack(FStateTreeExecutionContext& Context) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
    
    // 这里应该触发实际的攻击行为
    // 具体实现依赖于敌人角色的攻击系统
    if (InstanceData.EnemyCharacter)
    {
        switch (InstanceData.SelectedAttackType)
        {
        case EAIAttackType::Melee:
            // 执行近战攻击逻辑
            break;
        case EAIAttackType::Ranged:
            // 执行远程攻击逻辑
            break;
        // Skill attack type removed
        default:
            return false;
        }
        
        return true;
    }
    
    return false;
}

bool FStateTreeSmartAttackTask::ShouldReevaluate(const FInstanceDataType& InstanceData, float CurrentTime) const
{
    if (InstanceData.LastDecisionTime < 0.0f)
    {
        return true; // 首次决策
    }
    
    return (CurrentTime - InstanceData.LastDecisionTime) >= InstanceData.MinDecisionInterval;
}

FString FStateTreeSmartAttackTask::GenerateDecisionReason(const FInstanceDataType& InstanceData) const
{
    if (InstanceData.SelectedAttackType == EAIAttackType::None)
    {
        return TEXT("No valid attack option");
    }
    
    FString AttackTypeName = TEXT("Unknown");
    switch (InstanceData.SelectedAttackType)
    {
    case EAIAttackType::Melee:
        AttackTypeName = TEXT("Melee");
        break;
    case EAIAttackType::Ranged:
        AttackTypeName = TEXT("Ranged");
        break;
    default:
        AttackTypeName = TEXT("None");
        break;
    }
    float BestScore = InstanceData.BestAttackScore.FinalScore;
    
    return FString::Printf(TEXT("Selected %s attack (Score: %.2f)"), *AttackTypeName, BestScore);
}

#if WITH_EDITOR
FText FStateTreeSmartAttackTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
    return NSLOCTEXT("StateTreeEditor", "SmartAttack", "Smart Attack Selector");
}
#endif

// === FStateTreeTacticalPositionTask 实现 ===

EStateTreeRunStatus FStateTreeTacticalPositionTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

    if (!InstanceData.EnemyCharacter || !InstanceData.PositionQuery)
    {
        InstanceData.MovementFailureReason = TEXT("Missing EnemyCharacter or PositionQuery");
        return EStateTreeRunStatus::Failed;
    }

    // 评估并选择最佳位置
    if (!EvaluateAndSelectPosition(Context))
    {
        return EStateTreeRunStatus::Failed;
    }

    // 开始移动到选择的位置
    if (!MoveToSelectedPosition(Context))
    {
        return EStateTreeRunStatus::Failed;
    }

    return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FStateTreeTacticalPositionTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

    float CurrentTime = GetCurrentWorldTime(Context);

    // 检查是否需要重新评估位置
    if (ShouldReevaluatePosition(InstanceData, CurrentTime))
    {
        EvaluateAndSelectPosition(Context);
        InstanceData.LastEvaluationTime = CurrentTime;
    }

    // 检查移动是否完成
    if (CheckMovementCompletion(Context))
    {
        InstanceData.bIsMoving = false;
        InstanceData.bReachedTarget = true;
        return EStateTreeRunStatus::Succeeded;
    }

    return EStateTreeRunStatus::Running;
}

void FStateTreeTacticalPositionTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
    
    // 停止移动
    if (AAIController* AIController = Cast<AAIController>(InstanceData.EnemyCharacter->GetController()))
    {
        AIController->StopMovement();
    }
    
    InstanceData.bIsMoving = false;
}

bool FStateTreeTacticalPositionTask::EvaluateAndSelectPosition(FStateTreeExecutionContext& Context) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
    
    // 执行EQS查询获取候选位置
    TArray<FVector> CandidatePositions;
    FVector BestResult;
    if (!ExecuteEQSQueryWithCache(InstanceData.PositionQuery, Context, CandidatePositions, BestResult))
    {
        return false;
    }
    
    if (CandidatePositions.Num() > 0)
    {
        // 使用Utility评分选择最佳位置
        float BestScore = -1.0f;
        FVector BestPosition = FVector::ZeroVector;
        
        for (const FVector& Position : CandidatePositions)
        {
            FUtilityContext UtilityContext = CreateUtilityContext(Context);
            // 更新上下文以反映候选位置
            UtilityContext.DistanceToTarget = FVector::Dist(Position, 
                UtilityContext.TargetActor.IsValid() ? UtilityContext.TargetActor->GetActorLocation() : FVector::ZeroVector);
                
            FUtilityScore Score = CalculateUtilityScoreWithCache(InstanceData.PositionScoringProfile, UtilityContext);
            
            if (Score.bIsValid && Score.FinalScore > BestScore)
            {
                BestScore = Score.FinalScore;
                BestPosition = Position;
                InstanceData.PositionScore = Score;
            }
        }
        
        if (BestScore > 0.0f)
        {
            InstanceData.SelectedPosition = BestPosition;
            return true;
        }
    }
    
    return false;
}

bool FStateTreeTacticalPositionTask::MoveToSelectedPosition(FStateTreeExecutionContext& Context) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
    
    AAIController* AIController = Cast<AAIController>(InstanceData.EnemyCharacter->GetController());
    if (!AIController)
    {
        InstanceData.MovementFailureReason = TEXT("No AIController");
        return false;
    }

    // 检查移动距离是否足够
    float DistanceToTarget = FVector::Dist(InstanceData.EnemyCharacter->GetActorLocation(), InstanceData.SelectedPosition);
    if (DistanceToTarget < InstanceData.MinMovementDistance)
    {
        InstanceData.MovementFailureReason = TEXT("Target position too close");
        return false;
    }

    // 开始移动
    EPathFollowingRequestResult::Type MoveResult = AIController->MoveToLocation(InstanceData.SelectedPosition);
    
    if (MoveResult == EPathFollowingRequestResult::RequestSuccessful)
    {
        InstanceData.CurrentMoveTarget = InstanceData.SelectedPosition;
        InstanceData.bIsMoving = true;
        InstanceData.bReachedTarget = false;
        InstanceData.MovementFailureReason = TEXT("");
        return true;
    }
    else
    {
        InstanceData.MovementFailureReason = TEXT("PathFinding failed");
        return false;
    }
}

bool FStateTreeTacticalPositionTask::CheckMovementCompletion(FStateTreeExecutionContext& Context) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
    
    if (!InstanceData.bIsMoving)
    {
        return true;
    }
    
    AAIController* AIController = Cast<AAIController>(InstanceData.EnemyCharacter->GetController());
    if (!AIController)
    {
        return true; // 无法检查，认为完成
    }

    // 检查移动状态
    EPathFollowingStatus::Type MoveStatus = AIController->GetMoveStatus();
    if (MoveStatus == EPathFollowingStatus::Idle)
    {
        // 检查是否到达目标位置
        float DistanceToTarget = FVector::Dist(InstanceData.EnemyCharacter->GetActorLocation(), InstanceData.CurrentMoveTarget);
        return DistanceToTarget <= InstanceData.MovementTolerance;
    }
    
    return false;
}

bool FStateTreeTacticalPositionTask::ShouldReevaluatePosition(const FInstanceDataType& InstanceData, float CurrentTime) const
{
    if (InstanceData.LastEvaluationTime < 0.0f)
    {
        return true;
    }
    
    return (CurrentTime - InstanceData.LastEvaluationTime) >= InstanceData.ReevaluationInterval;
}

#if WITH_EDITOR
FText FStateTreeTacticalPositionTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
    return NSLOCTEXT("StateTreeEditor", "TacticalPosition", "Tactical Position Selector");
}
#endif

// === FStateTreeElementalDecisionTask 实现 ===

EStateTreeRunStatus FStateTreeElementalDecisionTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

    if (!InstanceData.EnemyCharacter)
    {
        InstanceData.ErrorMessage = TEXT("Missing EnemyCharacter");
        return EStateTreeRunStatus::Failed;
    }

    // 评估所有元素选项
    if (!EvaluateElementalOptions(Context))
    {
        return EStateTreeRunStatus::Failed;
    }

    return EStateTreeRunStatus::Succeeded;
}

EStateTreeRunStatus FStateTreeElementalDecisionTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

    // 检查是否需要切换元素
    if (InstanceData.bShouldSwitchElement && InstanceData.bAllowElementSwitching)
    {
        float CurrentTime = GetCurrentWorldTime(Context);
        
        if (CanSwitchElement(InstanceData, CurrentTime))
        {
            SwitchToElement(InstanceData.RecommendedElement, Context);
            InstanceData.LastSwitchTime = CurrentTime;
        }
    }

    return EStateTreeRunStatus::Succeeded;
}

bool FStateTreeElementalDecisionTask::EvaluateElementalOptions(FStateTreeExecutionContext& Context) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
    
    FUtilityContext UtilityContext = CreateUtilityContext(Context);
    
    // 重置结果
    InstanceData.ElementScores.Empty();
    
    EElementalType BestElement = EElementalType::None;
    float BestScore = -1.0f;

    // 评估每种元素类型
    for (const auto& ElementProfilePair : InstanceData.ElementalProfiles)
    {
        EElementalType ElementType = ElementProfilePair.Key;
        const FUtilityProfile& Profile = ElementProfilePair.Value;

        FUtilityScore Score = CalculateUtilityScoreWithCache(Profile, UtilityContext);
        
        InstanceData.ElementScores.Add(ElementType, Score.FinalScore);
        
        if (Score.bIsValid && Score.FinalScore > BestScore)
        {
            BestScore = Score.FinalScore;
            BestElement = ElementType;
        }
    }

    // 更新推荐
    InstanceData.RecommendedElement = BestElement;
    InstanceData.ElementAdvantageValue = BestScore;
    
    // 检查是否应该切换
    if (BestElement != InstanceData.CurrentElement && BestScore > InstanceData.MinElementAdvantageThreshold)
    {
        InstanceData.bShouldSwitchElement = true;
    }
    else
    {
        InstanceData.bShouldSwitchElement = false;
    }

    return BestElement != EElementalType::None;
}

bool FStateTreeElementalDecisionTask::CanSwitchElement(const FInstanceDataType& InstanceData, float CurrentTime) const
{
    if (InstanceData.LastSwitchTime < 0.0f)
    {
        return true; // 首次切换
    }
    
    return (CurrentTime - InstanceData.LastSwitchTime) >= InstanceData.ElementSwitchCooldown;
}

bool FStateTreeElementalDecisionTask::SwitchToElement(EElementalType NewElement, FStateTreeExecutionContext& Context) const
{
    const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
    
    // 这里应该调用敌人角色的元素切换方法
    // 具体实现依赖于元素系统的设计
    if (InstanceData.EnemyCharacter)
    {
        // 假设敌人角色有一个SetCurrentElement方法
        // InstanceData.EnemyCharacter->SetCurrentElement(NewElement);
        // 现在仅返回成功，实际的元素切换逻辑需要在具体应用中实现
        return true;
    }
    
    return false;
}

#if WITH_EDITOR
FText FStateTreeElementalDecisionTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
    return NSLOCTEXT("StateTreeEditor", "ElementalDecision", "Elemental Decision Maker");
}
#endif

// === FStateTreeMasterDecisionTask 实现 ===

EStateTreeRunStatus FStateTreeMasterDecisionTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

    if (!InstanceData.EnemyCharacter)
    {
        InstanceData.ErrorMessage = TEXT("Missing EnemyCharacter");
        return EStateTreeRunStatus::Failed;
    }

    // 更新所有子决策系统
    if (!UpdateAllDecisions(Context))
    {
        return EStateTreeRunStatus::Failed;
    }

    return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FStateTreeMasterDecisionTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

    float CurrentTime = GetCurrentWorldTime(Context);
    
    // 检查是否需要更新决策
    if (ShouldUpdateDecisions(InstanceData, CurrentTime))
    {
        UpdateAllDecisions(Context);
        InstanceData.LastDecisionUpdate = CurrentTime;
    }

    // 执行主要行动
    if (InstanceData.bProactiveDecisions)
    {
        ExecutePrimaryAction(Context);
    }

    return EStateTreeRunStatus::Running;
}

void FStateTreeMasterDecisionTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    if (bEnableDebugOutput)
    {
        const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
        LogDebug(FString::Printf(TEXT("MasterDecisionTask: Exiting with primary action '%s'"), 
                                *InstanceData.CurrentPrimaryAction));
    }
}

bool FStateTreeMasterDecisionTask::UpdateAllDecisions(FStateTreeExecutionContext& Context) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
    
    // 更新攻击决策
    // 这里应该调用攻击决策系统的更新方法
    
    // 更新位置决策
    // 这里应该调用位置决策系统的更新方法
    
    // 更新元素决策
    // 这里应该调用元素决策系统的更新方法
    
    // 计算综合决策优先级
    CalculateDecisionPriorities(InstanceData);
    
    return true;
}

void FStateTreeMasterDecisionTask::CalculateDecisionPriorities(FInstanceDataType& InstanceData) const
{
    InstanceData.DecisionPriorities.Empty();
    
    // 基于权重和当前评分计算优先级
    TArray<TPair<FString, float>> PriorityScores;
    
    // 攻击决策优先级
    if (InstanceData.AttackDecision.bShouldAttack)
    {
        float AttackPriority = InstanceData.AttackDecision.BestAttackScore.FinalScore * InstanceData.AttackDecisionWeight;
        PriorityScores.Add(TPair<FString, float>(TEXT("Attack"), AttackPriority));
    }
    
    // 位置决策优先级
    if (InstanceData.PositionDecision.bIsMoving || !InstanceData.PositionDecision.bReachedTarget)
    {
        float PositionPriority = InstanceData.PositionDecision.PositionScore.FinalScore * InstanceData.PositionDecisionWeight;
        PriorityScores.Add(TPair<FString, float>(TEXT("Position"), PositionPriority));
    }
    
    // 元素决策优先级
    if (InstanceData.ElementDecision.bShouldSwitchElement)
    {
        float ElementPriority = InstanceData.ElementDecision.ElementAdvantageValue * InstanceData.ElementDecisionWeight;
        PriorityScores.Add(TPair<FString, float>(TEXT("Element"), ElementPriority));
    }
    
    // 按优先级排序
    PriorityScores.Sort([](const TPair<FString, float>& A, const TPair<FString, float>& B) {
        return A.Value > B.Value;
    });
    
    // 更新优先级列表
    for (const auto& Priority : PriorityScores)
    {
        InstanceData.DecisionPriorities.Add(FString::Printf(TEXT("%s (%.2f)"), *Priority.Key, Priority.Value));
    }
    
    // 设置主要行动
    if (PriorityScores.Num() > 0)
    {
        InstanceData.CurrentPrimaryAction = PriorityScores[0].Key;
    }
    else
    {
        InstanceData.CurrentPrimaryAction = TEXT("None");
    }
}

bool FStateTreeMasterDecisionTask::ExecutePrimaryAction(FStateTreeExecutionContext& Context) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
    
    if (InstanceData.CurrentPrimaryAction == TEXT("Attack"))
    {
        // 执行攻击行动
        return true;
    }
    else if (InstanceData.CurrentPrimaryAction == TEXT("Position"))
    {
        // 执行位置移动
        return true;
    }
    else if (InstanceData.CurrentPrimaryAction == TEXT("Element"))
    {
        // 执行元素切换
        return true;
    }
    
    return false;
}

bool FStateTreeMasterDecisionTask::ShouldUpdateDecisions(const FInstanceDataType& InstanceData, float CurrentTime) const
{
    if (InstanceData.LastDecisionUpdate < 0.0f)
    {
        return true;
    }
    
    return (CurrentTime - InstanceData.LastDecisionUpdate) >= InstanceData.DecisionUpdateInterval;
}

#if WITH_EDITOR
FText FStateTreeMasterDecisionTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
    return NSLOCTEXT("StateTreeEditor", "MasterDecision", "Master Decision Coordinator");
}
#endif