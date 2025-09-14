// Copyright 2025 guigui17f. All Rights Reserved.

#include "StateTreeSmartTasks.h"
#include "StateTreeExecutionContext.h"
#include "AI/ElementalCombatEnemy.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "ElementalCombatAIController.h"

// === FStateTreeSmartAttackTask 实现 ===

EStateTreeRunStatus FStateTreeSmartAttackTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

    if (!InstanceData.EnemyCharacter)
    {
        LogDebug(TEXT("智能攻击任务：敌人角色为空"));
        InstanceData.ErrorMessage = TEXT("未找到敌人角色");
        return EStateTreeRunStatus::Failed;
    }

    if (!InstanceData.TargetActor)
    {
        LogDebug(TEXT("智能攻击任务：目标角色为空"));
        InstanceData.ErrorMessage = TEXT("未找到有效目标");
        return EStateTreeRunStatus::Failed;
    }

    // 如果正在攻击，直接返回成功避免重复触发
    if (InstanceData.EnemyCharacter->IsAttacking())
    {
        LogDebug(TEXT("智能攻击任务：角色已在攻击中，任务成功"));
        InstanceData.bTaskCompleted = true;
        return EStateTreeRunStatus::Succeeded;
    }

    // 检查是否应该重新评估
    float CurrentTime = GetCurrentWorldTime(Context);
    bool bNeedsReevaluation = ShouldReevaluate(InstanceData, CurrentTime);

    // 只在需要重新评估时才重新计算攻击选项
    if (bNeedsReevaluation)
    {
        // 评估攻击选项
        if (!EvaluateAttackOptions(Context))
        {
            InstanceData.ErrorMessage = TEXT("评估攻击选项失败");
            return EStateTreeRunStatus::Failed;
        }

        // 更新决策时间
        InstanceData.LastDecisionTime = CurrentTime;
    }

    // 检查是否有有效的攻击决策（无论是新的还是上次的）
    if (!InstanceData.bShouldAttack)
    {
        // 如果没有有效的攻击决策，但这是首次评估，则失败
        if (InstanceData.LastDecisionTime < 0.0f)
        {
            InstanceData.ErrorMessage = TEXT("首次评估时无有效攻击决策");
            return EStateTreeRunStatus::Failed;
        }

        // 否则返回成功（保持不攻击的决策）
        InstanceData.bTaskCompleted = true;
        return EStateTreeRunStatus::Succeeded;
    }

    // 执行选择的攻击（基于当前有效的决策）
    if (!ExecuteSelectedAttack(Context))
    {
        InstanceData.ErrorMessage = TEXT("攻击执行失败");
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

    // 从AIController获取配置
    AElementalCombatAIController* AIController = Cast<AElementalCombatAIController>(InstanceData.EnemyCharacter->GetController());
    if (!AIController)
    {
        LogDebug(TEXT("智能攻击任务：AI控制器为空或不是元素战斗AI控制器"));
        InstanceData.ErrorMessage = TEXT("未找到元素战斗AI控制器");
        return false;
    }

    const FUtilityProfile& AIProfile = AIController->GetCurrentAIProfile();

    // 清除之前的评分
    InstanceData.AttackTypeScores.Empty();

    // 创建评分上下文
    FUtilityContext UtilityContext = CreateUtilityContext(Context);
    float DistanceToTarget = UtilityContext.DistanceToTarget;

    // 检查AI类型标签
    bool bIsRangedAI = AIProfile.AITypeTags.Contains(TEXT("Ranged"));
    // 无标签或有Melee标签都视为近战AI
    bool bIsMeleeAI = !bIsRangedAI; // 默认为近战AI

    // 根据AI类型决定攻击策略
    if (bIsRangedAI)
    {
        // 远程AI：总是倾向于远程攻击
        InstanceData.SelectedAttackType = EAIAttackType::Ranged;
        InstanceData.FinalScore = 1.0f;
        InstanceData.bShouldAttack = true;
        InstanceData.AttackTypeScores.Add(EAIAttackType::Ranged, 1.0f);
        InstanceData.DecisionReason = TEXT("远程AI - 始终倾向远程攻击");
    }
    else // 默认近战AI逻辑（包括无标签和Melee标签）
    {
        // 近战AI的决策逻辑
        float SwitchDistance = AIProfile.MeleeToRangedSwitchDistance;

        if (DistanceToTarget <= SwitchDistance)
        {
            // 近距离：不攻击，交由后续位移逻辑处理
            InstanceData.bShouldAttack = false;
            InstanceData.SelectedAttackType = EAIAttackType::None;
            InstanceData.FinalScore = 0.0f;
            InstanceData.DecisionReason = FString::Printf(
                TEXT("近战AI - 距离过近 (%.1f <= %.1f)，等待移动"),
                DistanceToTarget, SwitchDistance);
        }
        else
        {
            // 检查是否在远程攻击范围内
            float RangedRange = InstanceData.EnemyCharacter->GetRangedAttackRange();
            if (DistanceToTarget <= RangedRange)
            {
                // 远距离但在射程内：使用远程攻击
                InstanceData.SelectedAttackType = EAIAttackType::Ranged;
                InstanceData.FinalScore = 0.8f;
                InstanceData.bShouldAttack = true;
                InstanceData.AttackTypeScores.Add(EAIAttackType::Ranged, 0.8f);
                InstanceData.DecisionReason = FString::Printf(
                    TEXT("近战AI - 在距离 %.1f 使用远程攻击"),
                    DistanceToTarget);
            }
            else
            {
                // 超出范围：不攻击
                InstanceData.bShouldAttack = false;
                InstanceData.SelectedAttackType = EAIAttackType::None;
                InstanceData.FinalScore = 0.0f;
                InstanceData.DecisionReason = TEXT("近战AI - 目标超出范围");
            }
        }
    }

    if (bEnableDebugOutput)
    {
        LogDebug(FString::Printf(TEXT("智能攻击：%s"), *InstanceData.DecisionReason));
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

    // 检查是否正在攻击，避免重复攻击
    if (InstanceData.EnemyCharacter->IsAttacking())
    {
        LogDebug(TEXT("智能攻击任务：角色已在攻击中，跳过执行"));
        return true; // 返回true避免任务失败
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
        return TEXT("无有效攻击选项");
    }
    
    FString TypeName = UEnum::GetValueAsString(InstanceData.SelectedAttackType);
    return FString::Printf(TEXT("选择 %s 攻击（评分：%.2f）"), 
                          *TypeName, InstanceData.FinalScore);
}

#if WITH_EDITOR
FText FStateTreeSmartAttackTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
    return NSLOCTEXT("StateTreeEditor", "SmartAttack", "智能攻击选择");
}
#endif

// === FStateTreeTacticalPositionTask 实现 ===

EStateTreeRunStatus FStateTreeTacticalPositionTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

    if (!InstanceData.EnemyCharacter)
    {
        LogDebug(TEXT("战术位置任务：敌人角色为空"));
        InstanceData.ErrorMessage = TEXT("未找到敌人角色");
        return EStateTreeRunStatus::Failed;
    }

    float CurrentTime = GetCurrentWorldTime(Context);

    // 如果正在移动，继续运行任务
    if (InstanceData.bIsMoving)
    {
        return EStateTreeRunStatus::Running;
    }

    // 检查是否需要重新评估位置
    if (!ShouldReevaluatePosition(InstanceData, CurrentTime))
    {
        // 不需要重新评估且没有在移动时返回成功
        // 这表示AI已达到理想位置或上次的位置决策仍然有效
        return EStateTreeRunStatus::Succeeded;
    }

    // 评估并选择最佳位置
    if (!EvaluateAndSelectPosition(Context))
    {
        InstanceData.ErrorMessage = TEXT("未能找到合适位置");
        return EStateTreeRunStatus::Failed;
    }

    // 更新评估时间
    InstanceData.LastEvaluationTime = CurrentTime;

    // 开始移动到选择的位置
    if (!MoveToSelectedPosition(Context))
    {
        InstanceData.ErrorMessage = TEXT("无法开始移动");
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
    
    // 检查是否有配置的位置查询模板
    if (!InstanceData.PositionQuery)
    {
        if (bEnableDebugOutput)
        {
            LogDebug(TEXT("战术位置：未配置位置查询，使用备用逻辑"));
        }

        // 回退到简化逻辑
        FVector CurrentPosition = InstanceData.EnemyCharacter->GetActorLocation();
        FVector TargetPosition = InstanceData.TargetActor ? InstanceData.TargetActor->GetActorLocation() : CurrentPosition;

        FVector Direction = (TargetPosition - CurrentPosition).GetSafeNormal();
        FVector SideDirection = FVector::CrossProduct(Direction, FVector::UpVector).GetSafeNormal();
        FVector CandidatePosition = TargetPosition + SideDirection * 300.0f + Direction * -200.0f;

        FUtilityContext UtilityContext = CreateUtilityContext(Context);
        UtilityContext.DistanceToTarget = FVector::Dist(CandidatePosition, TargetPosition);

        // 获取AIController配置
        AElementalCombatAIController* AIController = Cast<AElementalCombatAIController>(InstanceData.AIController);
        if (!AIController)
        {
            return false;
        }
        const FUtilityProfile& AIProfile = AIController->GetCurrentAIProfile();

        float PositionScore = CalculateUtilityScoreWithCache(AIProfile, UtilityContext);

        InstanceData.SelectedPosition = CandidatePosition;
        InstanceData.FinalScore = PositionScore;

        return PositionScore > 0.01f;
    }

    // 使用EQS查询获取战术位置
    TArray<FVector> CandidatePositions;
    FVector BestPosition;

    if (ExecuteEQSQueryWithCache(InstanceData.PositionQuery, Context, CandidatePositions, BestPosition))
    {
        // 创建评分上下文
        FUtilityContext BaseUtilityContext = CreateUtilityContext(Context);

        // 评估所有候选位置并选择最佳位置
        float BestScore = 0.0f;
        FVector BestTacticalPosition = BestPosition;

        for (const FVector& Position : CandidatePositions)
        {
            // 为每个位置创建评分上下文
            FUtilityContext PositionContext = BaseUtilityContext;
            PositionContext.DistanceToTarget = FVector::Dist(Position,
                InstanceData.TargetActor ? InstanceData.TargetActor->GetActorLocation() : Position);

            // 获取AIController配置
            AElementalCombatAIController* AIController = Cast<AElementalCombatAIController>(InstanceData.AIController);
            if (!AIController)
            {
                continue;
            }
            const FUtilityProfile& AIProfile = AIController->GetCurrentAIProfile();

            // 计算该位置的Utility评分
            float PositionScore = CalculateUtilityScoreWithCache(AIProfile, PositionContext);

            if (PositionScore > BestScore)
            {
                BestScore = PositionScore;
                BestTacticalPosition = Position;
            }
        }

        // 更新实例数据
        InstanceData.SelectedPosition = BestTacticalPosition;
        InstanceData.FinalScore = BestScore;

        if (bEnableDebugOutput)
        {
            LogDebug(FString::Printf(TEXT("战术位置：EQS找到 %d 个位置，选中 (%.1f,%.1f,%.1f)，评分=%.3f"),
                                    CandidatePositions.Num(),
                                    BestTacticalPosition.X, BestTacticalPosition.Y, BestTacticalPosition.Z,
                                    BestScore));
        }

        return BestScore > 0.01f;
    }
    else
    {
        if (bEnableDebugOutput)
        {
            LogDebug(TEXT("战术位置：EQS查询失败"));
        }

        InstanceData.FinalScore = 0.0f;
        return false;
    }
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
    return NSLOCTEXT("StateTreeEditor", "TacticalPosition", "战术位置选择");
}
#endif

// === FStateTreeElementalDecisionTask 实现 ===

EStateTreeRunStatus FStateTreeElementalDecisionTask::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
    FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

    if (!InstanceData.EnemyCharacter)
    {
        LogDebug(TEXT("元素决策任务：敌人角色为空"));
        InstanceData.ErrorMessage = TEXT("未找到敌人角色");
        return EStateTreeRunStatus::Failed;
    }

    // 评估元素选项
    if (!EvaluateElementalOptions(Context))
    {
        InstanceData.ErrorMessage = TEXT("评估元素选项失败");
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
                InstanceData.ErrorMessage = TEXT("切换元素失败");
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
        
        float ElementScore = CalculateUtilityScoreWithCache(Profile, UtilityContext);
        InstanceData.ElementScores.Add(ElementType, ElementScore);
        
        if (ElementScore > BestScore)
        {
            BestScore = ElementScore;
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
        LogDebug(FString::Printf(TEXT("元素决策：推荐 %s，评分=%.3f，应否切换=%s"),
                                *ElementName, BestScore,
                                InstanceData.bShouldSwitchElement ? TEXT("是") : TEXT("否")));
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
        LogDebug(FString::Printf(TEXT("元素决策：切换到 %s"), *ElementName));
    }
    
    return true;
}

#if WITH_EDITOR
FText FStateTreeElementalDecisionTask::GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting) const
{
    return NSLOCTEXT("StateTreeEditor", "ElementalDecision", "元素类型决策");
}
#endif
