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
        LogDebug(TEXT("SmartAttackTask: EnemyCharacter is null"));
        InstanceData.ErrorMessage = TEXT("No EnemyCharacter found");
        return EStateTreeRunStatus::Failed;
    }

    if (!InstanceData.TargetActor)
    {
        LogDebug(TEXT("SmartAttackTask: TargetActor is null"));
        InstanceData.ErrorMessage = TEXT("No valid target found");
        return EStateTreeRunStatus::Failed;
    }

    // 检查是否应该重新评估
    float CurrentTime = GetCurrentWorldTime(Context);
    if (!ShouldReevaluate(InstanceData, CurrentTime))
    {
        return EStateTreeRunStatus::Succeeded;
    }

    // 评估攻击选项
    if (!EvaluateAttackOptions(Context))
    {
        InstanceData.ErrorMessage = TEXT("Failed to evaluate attack options");
        return EStateTreeRunStatus::Failed;
    }

    // 如果不应该攻击，成功完成但不执行攻击
    if (!InstanceData.bShouldAttack)
    {
        InstanceData.bTaskCompleted = true;
        return EStateTreeRunStatus::Succeeded;
    }

    // 执行选择的攻击
    if (!ExecuteSelectedAttack(Context))
    {
        InstanceData.ErrorMessage = TEXT("Attack execution failed");
        return EStateTreeRunStatus::Failed;
    }

    InstanceData.bTaskCompleted = true;
    return EStateTreeRunStatus::Succeeded;
}

EStateTreeRunStatus FStateTreeSmartAttackTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
    // 大多数情况下在EnterState完成，此处可以添加持续监控逻辑
    return EStateTreeRunStatus::Succeeded;
}

void FStateTreeSmartAttackTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    // 清理逻辑
}

bool FStateTreeSmartAttackTask::EvaluateAttackOptions(FStateTreeExecutionContext& Context) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
    
    // 清除之前的评分
    InstanceData.AttackTypeScores.Empty();
    
    // 创建评分上下文
    FUtilityContext UtilityContext = CreateUtilityContext(Context);
    
    // 评估近战攻击
    FUtilityScore MeleeScore = CalculateUtilityScoreWithCache(InstanceData.MeleeAttackProfile, UtilityContext);
    InstanceData.AttackTypeScores.Add(EAIAttackType::Melee, MeleeScore.FinalScore);
    
    // 评估远程攻击
    FUtilityScore RangedScore = CalculateUtilityScoreWithCache(InstanceData.RangedAttackProfile, UtilityContext);
    InstanceData.AttackTypeScores.Add(EAIAttackType::Ranged, RangedScore.FinalScore);
    
    // 选择最佳攻击类型
    EAIAttackType BestAttackType = EAIAttackType::None;
    FUtilityScore BestScore;
    
    if (MeleeScore.IsBetterThan(RangedScore))
    {
        BestAttackType = EAIAttackType::Melee;
        BestScore = MeleeScore;
    }
    else if (RangedScore.bIsValid)
    {
        BestAttackType = EAIAttackType::Ranged;
        BestScore = RangedScore;
    }
    
    // 更新实例数据
    InstanceData.SelectedAttackType = BestAttackType;
    InstanceData.BestAttackScore = BestScore;
    InstanceData.bShouldAttack = (BestAttackType != EAIAttackType::None && BestScore.bIsValid);
    InstanceData.DecisionReason = GenerateDecisionReason(InstanceData);
    
    if (bEnableDebugOutput)
    {
        FString TypeName = UEnum::GetValueAsString(BestAttackType);
        LogDebug(FString::Printf(TEXT("SmartAttack: Selected %s, Score=%.3f, ShouldAttack=%s"), 
                                *TypeName, BestScore.FinalScore,
                                InstanceData.bShouldAttack ? TEXT("Yes") : TEXT("No")));
    }
    
    return true;
}

bool FStateTreeSmartAttackTask::ExecuteSelectedAttack(FStateTreeExecutionContext& Context) const
{
    const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
    
    if (!InstanceData.EnemyCharacter)
    {
        return false;
    }
    
    // 设置敌人的攻击类型
    InstanceData.EnemyCharacter->CurrentAttackType = InstanceData.SelectedAttackType;
    
    // 根据攻击类型执行相应的攻击逻辑
    switch (InstanceData.SelectedAttackType)
    {
    case EAIAttackType::Melee:
        // 执行近战攻击（调用基类方法）
        InstanceData.EnemyCharacter->DoAttackTrace(TEXT("hand_r"));
        break;
        
    case EAIAttackType::Ranged:
        // 执行远程攻击
        InstanceData.EnemyCharacter->DoAIRangedAttack();
        break;
        
    default:
        return false;
    }
    
    return true;
}

bool FStateTreeSmartAttackTask::ShouldReevaluate(const FInstanceDataType& InstanceData, float CurrentTime) const
{
    return (CurrentTime - InstanceData.LastDecisionTime) >= InstanceData.MinDecisionInterval;
}

FString FStateTreeSmartAttackTask::GenerateDecisionReason(const FInstanceDataType& InstanceData) const
{
    if (!InstanceData.bShouldAttack)
    {
        return TEXT("No valid attack option available");
    }
    
    FString TypeName = UEnum::GetValueAsString(InstanceData.SelectedAttackType);
    return FString::Printf(TEXT("Selected %s attack (Score: %.2f)"), 
                          *TypeName, InstanceData.BestAttackScore.FinalScore);
}

#if WITH_EDITOR
FText FStateTreeSmartAttackTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
    return NSLOCTEXT("StateTreeEditor", "SmartAttack", "Smart Attack Selection");
}
#endif

// === FStateTreeTacticalPositionTask 实现 ===

EStateTreeRunStatus FStateTreeTacticalPositionTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

    if (!InstanceData.EnemyCharacter)
    {
        LogDebug(TEXT("TacticalPositionTask: EnemyCharacter is null"));
        InstanceData.ErrorMessage = TEXT("No EnemyCharacter found");
        return EStateTreeRunStatus::Failed;
    }

    float CurrentTime = GetCurrentWorldTime(Context);
    
    // 检查是否需要重新评估
    if (!ShouldReevaluatePosition(InstanceData, CurrentTime))
    {
        return EStateTreeRunStatus::Succeeded;
    }

    // 评估并选择最佳位置
    if (!EvaluateAndSelectPosition(Context))
    {
        InstanceData.ErrorMessage = TEXT("Failed to find suitable position");
        return EStateTreeRunStatus::Failed;
    }

    // 开始移动到选择的位置
    if (!MoveToSelectedPosition(Context))
    {
        InstanceData.ErrorMessage = TEXT("Failed to start movement");
        return EStateTreeRunStatus::Failed;
    }

    return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FStateTreeTacticalPositionTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

    if (!InstanceData.bIsMoving)
    {
        return EStateTreeRunStatus::Succeeded;
    }

    // 检查移动是否完成
    if (CheckMovementCompletion(Context))
    {
        InstanceData.bIsMoving = false;
        InstanceData.bReachedTarget = true;
        InstanceData.bTaskCompleted = true;
        return EStateTreeRunStatus::Succeeded;
    }

    return EStateTreeRunStatus::Running;
}

void FStateTreeTacticalPositionTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
    
    // 停止移动
    if (InstanceData.bIsMoving && InstanceData.AIController)
    {
        InstanceData.AIController->StopMovement();
        InstanceData.bIsMoving = false;
    }
}

bool FStateTreeTacticalPositionTask::EvaluateAndSelectPosition(FStateTreeExecutionContext& Context) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
    
    // 简化的位置选择逻辑 - 在实际项目中会使用EQS
    FVector CurrentPosition = InstanceData.EnemyCharacter->GetActorLocation();
    FVector TargetPosition = InstanceData.TargetActor ? InstanceData.TargetActor->GetActorLocation() : CurrentPosition;
    
    // 选择一个基本的战术位置（可以通过Utility AI进一步优化）
    FVector Direction = (TargetPosition - CurrentPosition).GetSafeNormal();
    FVector SideDirection = FVector::CrossProduct(Direction, FVector::UpVector).GetSafeNormal();
    
    // 简单的侧翼位置
    FVector CandidatePosition = TargetPosition + SideDirection * 300.0f + Direction * -200.0f;
    
    // 创建评分上下文并评分
    FUtilityContext UtilityContext = CreateUtilityContext(Context);
    UtilityContext.DistanceToTarget = FVector::Dist(CandidatePosition, TargetPosition);
    
    FUtilityScore PositionScore = CalculateUtilityScoreWithCache(InstanceData.PositionScoringProfile, UtilityContext);
    
    // 更新实例数据
    InstanceData.SelectedPosition = CandidatePosition;
    InstanceData.PositionScore = PositionScore;
    
    if (bEnableDebugOutput)
    {
        LogDebug(FString::Printf(TEXT("TacticalPosition: Selected (%.1f,%.1f,%.1f), Score=%.3f"), 
                                CandidatePosition.X, CandidatePosition.Y, CandidatePosition.Z,
                                PositionScore.FinalScore));
    }
    
    return PositionScore.bIsValid;
}

bool FStateTreeTacticalPositionTask::MoveToSelectedPosition(FStateTreeExecutionContext& Context) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
    
    if (!InstanceData.AIController)
    {
        return false;
    }

    // 检查移动距离是否足够
    FVector CurrentPosition = InstanceData.EnemyCharacter->GetActorLocation();
    float MoveDistance = FVector::Dist(CurrentPosition, InstanceData.SelectedPosition);
    
    if (MoveDistance < InstanceData.MinMovementDistance)
    {
        InstanceData.bReachedTarget = true;
        return true;
    }

    // 发起移动请求
    InstanceData.MoveRequestID = InstanceData.AIController->MoveToLocation(InstanceData.SelectedPosition);
    InstanceData.CurrentMoveTarget = InstanceData.SelectedPosition;
    InstanceData.bIsMoving = true;
    InstanceData.bReachedTarget = false;

    return InstanceData.MoveRequestID.IsValid();
}

bool FStateTreeTacticalPositionTask::CheckMovementCompletion(FStateTreeExecutionContext& Context) const
{
    const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
    
    if (!InstanceData.AIController)
    {
        return true; // 无控制器，认为完成
    }

    // 检查移动状态
    EPathFollowingStatus::Type MoveStatus = InstanceData.AIController->GetMoveStatus();
    
    if (MoveStatus == EPathFollowingStatus::Idle)
    {
        return true;
    }
    
    if (MoveStatus == EPathFollowingStatus::Waiting || MoveStatus == EPathFollowingStatus::Paused)
    {
        return false; // 还在等待或暂停
    }

    // 检查距离容差
    FVector CurrentPosition = InstanceData.EnemyCharacter->GetActorLocation();
    float DistanceToTarget = FVector::Dist(CurrentPosition, InstanceData.CurrentMoveTarget);
    
    return DistanceToTarget <= InstanceData.MovementTolerance;
}

bool FStateTreeTacticalPositionTask::ShouldReevaluatePosition(const FInstanceDataType& InstanceData, float CurrentTime) const
{
    return (CurrentTime - InstanceData.LastEvaluationTime) >= InstanceData.ReevaluationInterval;
}

#if WITH_EDITOR
FText FStateTreeTacticalPositionTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
    return NSLOCTEXT("StateTreeEditor", "TacticalPosition", "Tactical Position Selection");
}
#endif

// === FStateTreeElementalDecisionTask 实现 ===

EStateTreeRunStatus FStateTreeElementalDecisionTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

    if (!InstanceData.EnemyCharacter)
    {
        LogDebug(TEXT("ElementalDecisionTask: EnemyCharacter is null"));
        InstanceData.ErrorMessage = TEXT("No EnemyCharacter found");
        return EStateTreeRunStatus::Failed;
    }

    // 评估元素选项
    if (!EvaluateElementalOptions(Context))
    {
        InstanceData.ErrorMessage = TEXT("Failed to evaluate elemental options");
        return EStateTreeRunStatus::Failed;
    }

    // 检查是否可以切换元素
    float CurrentTime = GetCurrentWorldTime(Context);
    if (InstanceData.bAllowElementSwitching && CanSwitchElement(InstanceData, CurrentTime))
    {
        if (InstanceData.bShouldSwitchElement)
        {
            if (!SwitchToElement(InstanceData.RecommendedElement, Context))
            {
                InstanceData.ErrorMessage = TEXT("Failed to switch element");
                return EStateTreeRunStatus::Failed;
            }
        }
    }

    InstanceData.bTaskCompleted = true;
    return EStateTreeRunStatus::Succeeded;
}

EStateTreeRunStatus FStateTreeElementalDecisionTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
    return EStateTreeRunStatus::Succeeded;
}

bool FStateTreeElementalDecisionTask::EvaluateElementalOptions(FStateTreeExecutionContext& Context) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
    
    // 清除之前的评分
    InstanceData.ElementScores.Empty();
    
    // 创建评分上下文
    FUtilityContext UtilityContext = CreateUtilityContext(Context);
    
    // 获取当前元素
    InstanceData.CurrentElement = EElementalType::None; // 需要从角色获取实际元素
    
    EElementalType BestElement = EElementalType::None;
    float BestScore = 0.0f;
    
    // 评估每种元素类型
    for (const auto& ElementProfilePair : InstanceData.ElementalProfiles)
    {
        EElementalType ElementType = ElementProfilePair.Key;
        const FUtilityProfile& Profile = ElementProfilePair.Value;
        
        FUtilityScore ElementScore = CalculateUtilityScoreWithCache(Profile, UtilityContext);
        InstanceData.ElementScores.Add(ElementType, ElementScore.FinalScore);
        
        if (ElementScore.FinalScore > BestScore)
        {
            BestScore = ElementScore.FinalScore;
            BestElement = ElementType;
        }
    }
    
    // 更新推荐元素
    InstanceData.RecommendedElement = BestElement;
    InstanceData.ElementAdvantageValue = BestScore;
    
    // 检查是否应该切换元素
    InstanceData.bShouldSwitchElement = (BestElement != InstanceData.CurrentElement) && 
                                       (BestScore > InstanceData.MinElementAdvantageThreshold);
    
    if (bEnableDebugOutput)
    {
        FString ElementName = UEnum::GetValueAsString(BestElement);
        LogDebug(FString::Printf(TEXT("ElementalDecision: Recommended %s, Score=%.3f, ShouldSwitch=%s"), 
                                *ElementName, BestScore,
                                InstanceData.bShouldSwitchElement ? TEXT("Yes") : TEXT("No")));
    }
    
    return true;
}

bool FStateTreeElementalDecisionTask::CanSwitchElement(const FInstanceDataType& InstanceData, float CurrentTime) const
{
    return (CurrentTime - InstanceData.LastSwitchTime) >= InstanceData.ElementSwitchCooldown;
}

bool FStateTreeElementalDecisionTask::SwitchToElement(EElementalType NewElement, FStateTreeExecutionContext& Context) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
    
    // 这里应该调用角色的元素切换方法
    // 目前作为占位符返回成功
    
    // 更新内部状态
    InstanceData.LastSwitchTime = GetCurrentWorldTime(Context);
    InstanceData.CurrentElement = NewElement;
    
    if (bEnableDebugOutput)
    {
        FString ElementName = UEnum::GetValueAsString(NewElement);
        LogDebug(FString::Printf(TEXT("ElementalDecision: Switched to %s"), *ElementName));
    }
    
    return true;
}

#if WITH_EDITOR
FText FStateTreeElementalDecisionTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
    return NSLOCTEXT("StateTreeEditor", "ElementalDecision", "Elemental Type Decision");
}
#endif

// === FStateTreeMasterDecisionTask 实现 ===

EStateTreeRunStatus FStateTreeMasterDecisionTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

    if (!InstanceData.EnemyCharacter)
    {
        LogDebug(TEXT("MasterDecisionTask: EnemyCharacter is null"));
        InstanceData.ErrorMessage = TEXT("No EnemyCharacter found");
        return EStateTreeRunStatus::Failed;
    }

    // 更新所有子决策系统
    if (!UpdateAllDecisions(Context))
    {
        InstanceData.ErrorMessage = TEXT("Failed to update decisions");
        return EStateTreeRunStatus::Failed;
    }

    // 计算综合决策优先级
    CalculateDecisionPriorities(InstanceData);

    // 执行最高优先级的行动
    if (!ExecutePrimaryAction(Context))
    {
        InstanceData.ErrorMessage = TEXT("Failed to execute primary action");
        return EStateTreeRunStatus::Failed;
    }

    InstanceData.bTaskCompleted = true;
    return EStateTreeRunStatus::Succeeded;
}

EStateTreeRunStatus FStateTreeMasterDecisionTask::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

    float CurrentTime = GetCurrentWorldTime(Context);
    if (ShouldUpdateDecisions(InstanceData, CurrentTime))
    {
        UpdateAllDecisions(Context);
        CalculateDecisionPriorities(InstanceData);
        InstanceData.LastDecisionUpdate = CurrentTime;
    }

    return EStateTreeRunStatus::Running;
}

void FStateTreeMasterDecisionTask::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    // 清理逻辑
}

bool FStateTreeMasterDecisionTask::UpdateAllDecisions(FStateTreeExecutionContext& Context) const
{
    // 这里应该更新所有子决策系统
    // 目前作为占位符返回成功
    return true;
}

void FStateTreeMasterDecisionTask::CalculateDecisionPriorities(FInstanceDataType& InstanceData) const
{
    InstanceData.DecisionPriorities.Empty();
    
    // 基于权重和评分计算优先级
    InstanceData.DecisionPriorities.Add(TEXT("Attack Decision"));
    InstanceData.DecisionPriorities.Add(TEXT("Position Decision"));
    InstanceData.DecisionPriorities.Add(TEXT("Element Decision"));
    
    InstanceData.CurrentPrimaryAction = InstanceData.DecisionPriorities.Num() > 0 ? 
                                       InstanceData.DecisionPriorities[0] : TEXT("No Action");
}

bool FStateTreeMasterDecisionTask::ExecutePrimaryAction(FStateTreeExecutionContext& Context) const
{
    const FInstanceDataType& InstanceData = Context.GetInstanceData(*this);
    
    if (bEnableDebugOutput)
    {
        LogDebug(FString::Printf(TEXT("MasterDecision: Executing %s"), *InstanceData.CurrentPrimaryAction));
    }
    
    return true;
}

bool FStateTreeMasterDecisionTask::ShouldUpdateDecisions(const FInstanceDataType& InstanceData, float CurrentTime) const
{
    return (CurrentTime - InstanceData.LastDecisionUpdate) >= InstanceData.DecisionUpdateInterval;
}

#if WITH_EDITOR
FText FStateTreeMasterDecisionTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
    return NSLOCTEXT("StateTreeEditor", "MasterDecision", "Master Decision Coordinator");
}
#endif