// Copyright 2025 guigui17f. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ElementalStateTreeTaskBase.h"
#include "AI/Utility/UtilityAITypes.h"
#include "EnvironmentQuery/EnvQuery.h"
#include "Combat/Elemental/ElementalTypes.h"
#include "AI/ElementalCombatEnemy.h"
#include "AITypes.h"
#include "Engine/DataTable.h"
#include "StateTreeSmartTasks.generated.h"

class AElementalCombatEnemy;

/**
 * 智能攻击选择任务实例数据
 */
USTRUCT()
struct FStateTreeSmartAttackInstanceData : public FElementalStateTreeInstanceDataBase
{
    GENERATED_BODY()

    // 移除DataTable配置，改为从AIController获取配置

    // 技能攻击功能已移除，因为EAIAttackType中没有Skill类型

    /** 最小决策间隔（秒，避免频繁切换） */
    UPROPERTY(EditAnywhere, Category = "Attack Scoring", meta = (ClampMin = "0.1"))
    float MinDecisionInterval = 1.0f;

    // === 输出结果 ===

    /** 选择的攻击类型（输出） */
    UPROPERTY(VisibleAnywhere, Category = "Output")
    EAIAttackType SelectedAttackType = EAIAttackType::None;

    /** 各攻击类型的评分（输出） */
    UPROPERTY(VisibleAnywhere, Category = "Output")
    TMap<EAIAttackType, float> AttackTypeScores;

    /** 最佳攻击评分（输出） */
    UPROPERTY(VisibleAnywhere, Category = "Output")
    FUtilityScore BestAttackScore;

    /** 攻击决策原因（输出，调试用） */
    UPROPERTY(VisibleAnywhere, Category = "Output")
    FString DecisionReason;

    /** 是否执行攻击（输出） */
    UPROPERTY(VisibleAnywhere, Category = "Output")
    bool bShouldAttack = false;

private:
    /** 上次决策时间（内部使用） */
    float LastDecisionTime = -1.0f;

    friend struct FStateTreeSmartAttackTask;
};

STATETREE_POD_INSTANCEDATA(FStateTreeSmartAttackInstanceData);

/**
 * 智能攻击选择任务
 * 结合Utility评分系统智能选择最佳攻击方式
 */
USTRUCT(meta=(DisplayName="Smart Attack Selector", Category="ElementalCombat|Smart"))
struct ELEMENTALCOMBAT_API FStateTreeSmartAttackTask : public FElementalStateTreeTaskBase
{
    GENERATED_BODY()

    using FInstanceDataType = FStateTreeSmartAttackInstanceData;
    virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

    FStateTreeSmartAttackTask() = default;

    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
    virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

#if WITH_EDITOR
    virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif

protected:
    /** 评估所有攻击类型并选择最佳的 */
    bool EvaluateAttackOptions(FStateTreeExecutionContext& Context) const;

    /** 执行选择的攻击 */
    bool ExecuteSelectedAttack(FStateTreeExecutionContext& Context) const;

    /** 检查是否应该重新评估 */
    bool ShouldReevaluate(const FInstanceDataType& InstanceData, float CurrentTime) const;

    /** 生成决策原因字符串 */
    FString GenerateDecisionReason(const FInstanceDataType& InstanceData) const;
};

/**
 * 战术位置选择任务实例数据
 */
USTRUCT()
struct FStateTreeTacticalPositionInstanceData : public FElementalStateTreeInstanceDataBase
{
    GENERATED_BODY()

    /** EQS位置查询模板 */
    UPROPERTY(EditAnywhere, Category = "Position Selection")
    TObjectPtr<UEnvQuery> PositionQuery;

    /** 位置评分配置 */
    UPROPERTY(EditAnywhere, Category = "Position Selection")
    FUtilityProfile PositionScoringProfile;

    /** 最小移动距离（避免微小移动） */
    UPROPERTY(EditAnywhere, Category = "Position Selection", meta = (ClampMin = "0.0"))
    float MinMovementDistance = 100.0f;

    /** 位置重新评估间隔（秒） */
    UPROPERTY(EditAnywhere, Category = "Position Selection", meta = (ClampMin = "0.5"))
    float ReevaluationInterval = 3.0f;

    /** 移动完成容差距离 */
    UPROPERTY(EditAnywhere, Category = "Position Selection", meta = (ClampMin = "10.0"))
    float MovementTolerance = 50.0f;

    // === 输出结果 ===

    /** 选择的目标位置（输出） */
    UPROPERTY(VisibleAnywhere, Category = "Output")
    FVector SelectedPosition = FVector::ZeroVector;

    /** 当前移动目标位置（输出） */
    UPROPERTY(VisibleAnywhere, Category = "Output")
    FVector CurrentMoveTarget = FVector::ZeroVector;

    /** 位置选择评分（输出） */
    UPROPERTY(VisibleAnywhere, Category = "Output")
    FUtilityScore PositionScore;

    /** 是否正在移动（输出） */
    UPROPERTY(VisibleAnywhere, Category = "Output")
    bool bIsMoving = false;

    /** 是否已到达目标位置（输出） */
    UPROPERTY(VisibleAnywhere, Category = "Output")
    bool bReachedTarget = false;

    /** 移动失败原因（输出） */
    UPROPERTY(VisibleAnywhere, Category = "Output")
    FString MovementFailureReason;

private:
    /** 上次位置评估时间（内部使用） */
    float LastEvaluationTime = -1.0f;

    /** 移动请求ID（内部使用） */
    FAIRequestID MoveRequestID;

    friend struct FStateTreeTacticalPositionTask;
};

STATETREE_POD_INSTANCEDATA(FStateTreeTacticalPositionInstanceData);

/**
 * 战术位置选择任务
 * 结合EQS和Utility评分选择最佳战术位置
 */
USTRUCT(meta=(DisplayName="Tactical Position Selector", Category="ElementalCombat|Smart"))
struct ELEMENTALCOMBAT_API FStateTreeTacticalPositionTask : public FElementalStateTreeTaskBase
{
    GENERATED_BODY()

    using FInstanceDataType = FStateTreeTacticalPositionInstanceData;
    virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

    FStateTreeTacticalPositionTask() = default;

    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
    virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

#if WITH_EDITOR
    virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif

protected:
    /** 评估并选择最佳位置 */
    bool EvaluateAndSelectPosition(FStateTreeExecutionContext& Context) const;

    /** 移动到选择的位置 */
    bool MoveToSelectedPosition(FStateTreeExecutionContext& Context) const;

    /** 检查移动是否完成 */
    bool CheckMovementCompletion(FStateTreeExecutionContext& Context) const;

    /** 检查是否应该重新评估位置 */
    bool ShouldReevaluatePosition(const FInstanceDataType& InstanceData, float CurrentTime) const;
};

/**
 * 元素决策任务实例数据
 */
USTRUCT()
struct FStateTreeElementalDecisionInstanceData : public FElementalStateTreeInstanceDataBase
{
    GENERATED_BODY()

    /** 各元素类型的评分配置 */
    UPROPERTY(EditAnywhere, Category = "Elemental Decision")
    TMap<EElementalType, FUtilityProfile> ElementalProfiles;

    /** 是否允许动态切换元素 */
    UPROPERTY(EditAnywhere, Category = "Elemental Decision")
    bool bAllowElementSwitching = true;

    /** 元素切换冷却时间（秒） */
    UPROPERTY(EditAnywhere, Category = "Elemental Decision", meta = (ClampMin = "0.0"))
    float ElementSwitchCooldown = 2.0f;

    /** 最小元素优势阈值（低于此值不切换） */
    UPROPERTY(EditAnywhere, Category = "Elemental Decision", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MinElementAdvantageThreshold = 0.2f;

    // === 输出结果 ===

    /** 推荐的元素类型（输出） */
    UPROPERTY(VisibleAnywhere, Category = "Output")
    EElementalType RecommendedElement = EElementalType::None;

    /** 当前元素（输出） */
    UPROPERTY(VisibleAnywhere, Category = "Output")
    EElementalType CurrentElement = EElementalType::None;

    /** 各元素的评分（输出） */
    UPROPERTY(VisibleAnywhere, Category = "Output")
    TMap<EElementalType, float> ElementScores;

    /** 是否应该切换元素（输出） */
    UPROPERTY(VisibleAnywhere, Category = "Output")
    bool bShouldSwitchElement = false;

    /** 元素优势值（输出） */
    UPROPERTY(VisibleAnywhere, Category = "Output")
    float ElementAdvantageValue = 0.0f;

private:
    /** 上次切换元素时间（内部使用） */
    float LastSwitchTime = -1.0f;

    friend struct FStateTreeElementalDecisionTask;
};

STATETREE_POD_INSTANCEDATA(FStateTreeElementalDecisionInstanceData);

/**
 * 元素决策任务
 * 基于当前战斗情况智能选择最佳元素类型
 */
USTRUCT(meta=(DisplayName="Elemental Decision Maker", Category="ElementalCombat|Smart"))
struct ELEMENTALCOMBAT_API FStateTreeElementalDecisionTask : public FElementalStateTreeTaskBase
{
    GENERATED_BODY()

    using FInstanceDataType = FStateTreeElementalDecisionInstanceData;
    virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

    FStateTreeElementalDecisionTask() = default;

    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

#if WITH_EDITOR
    virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif

protected:
    /** 评估所有元素类型 */
    bool EvaluateElementalOptions(FStateTreeExecutionContext& Context) const;

    /** 检查是否可以切换元素 */
    bool CanSwitchElement(const FInstanceDataType& InstanceData, float CurrentTime) const;

    /** 执行元素切换 */
    bool SwitchToElement(EElementalType NewElement, FStateTreeExecutionContext& Context) const;
};

/**
 * 综合智能决策任务实例数据
 */
USTRUCT()
struct FStateTreeMasterDecisionInstanceData : public FElementalStateTreeInstanceDataBase
{
    GENERATED_BODY()

    /** 攻击决策权重 */
    UPROPERTY(EditAnywhere, Category = "Master Decision", meta = (ClampMin = "0.0"))
    float AttackDecisionWeight = 1.0f;

    /** 位置决策权重 */
    UPROPERTY(EditAnywhere, Category = "Master Decision", meta = (ClampMin = "0.0"))
    float PositionDecisionWeight = 1.0f;

    /** 元素决策权重 */
    UPROPERTY(EditAnywhere, Category = "Master Decision", meta = (ClampMin = "0.0"))
    float ElementDecisionWeight = 0.8f;

    /** 决策更新间隔（秒） */
    UPROPERTY(EditAnywhere, Category = "Master Decision", meta = (ClampMin = "0.1"))
    float DecisionUpdateInterval = 0.5f;

    /** 是否启用主动决策模式 */
    UPROPERTY(EditAnywhere, Category = "Master Decision")
    bool bProactiveDecisions = true;

    // === 子决策组件 ===

    /** 攻击选择配置 */
    UPROPERTY(EditAnywhere, Category = "Sub Decisions")
    FStateTreeSmartAttackInstanceData AttackDecision;

    /** 位置选择配置 */
    UPROPERTY(EditAnywhere, Category = "Sub Decisions")
    FStateTreeTacticalPositionInstanceData PositionDecision;

    /** 元素选择配置 */
    UPROPERTY(EditAnywhere, Category = "Sub Decisions")
    FStateTreeElementalDecisionInstanceData ElementDecision;

    // === 输出结果 ===

    /** 综合决策评分（输出） */
    UPROPERTY(VisibleAnywhere, Category = "Output")
    FUtilityScore MasterDecisionScore;

    /** 当前执行的主要行动（输出） */
    UPROPERTY(VisibleAnywhere, Category = "Output")
    FString CurrentPrimaryAction;

    /** 决策优先级列表（输出） */
    UPROPERTY(VisibleAnywhere, Category = "Output")
    TArray<FString> DecisionPriorities;

private:
    /** 上次决策更新时间（内部使用） */
    float LastDecisionUpdate = -1.0f;

    friend struct FStateTreeMasterDecisionTask;
};

STATETREE_POD_INSTANCEDATA(FStateTreeMasterDecisionInstanceData);

/**
 * 综合智能决策任务
 * 协调攻击、位置、元素等多个决策系统
 */
USTRUCT(meta=(DisplayName="Master Decision Coordinator", Category="ElementalCombat|Smart"))
struct ELEMENTALCOMBAT_API FStateTreeMasterDecisionTask : public FElementalStateTreeTaskBase
{
    GENERATED_BODY()

    using FInstanceDataType = FStateTreeMasterDecisionInstanceData;
    virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

    FStateTreeMasterDecisionTask() = default;

    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
    virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

#if WITH_EDITOR
    virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif

protected:
    /** 更新所有子决策系统 */
    bool UpdateAllDecisions(FStateTreeExecutionContext& Context) const;

    /** 计算综合决策优先级 */
    void CalculateDecisionPriorities(FInstanceDataType& InstanceData) const;

    /** 执行最高优先级的行动 */
    bool ExecutePrimaryAction(FStateTreeExecutionContext& Context) const;

    /** 检查是否需要更新决策 */
    bool ShouldUpdateDecisions(const FInstanceDataType& InstanceData, float CurrentTime) const;
};