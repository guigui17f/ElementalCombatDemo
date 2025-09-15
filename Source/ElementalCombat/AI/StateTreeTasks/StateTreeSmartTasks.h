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
    UPROPERTY(EditAnywhere, Category = "ElementalCombat|AI", meta = (ClampMin = "0.1"))
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
    float FinalScore = 0.0f;

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
 * 元素决策任务实例数据
 */
USTRUCT()
struct FStateTreeElementalDecisionInstanceData : public FElementalStateTreeInstanceDataBase
{
    GENERATED_BODY()

    /** 各元素类型的评分配置 */
    UPROPERTY(EditAnywhere, Category = "ElementalCombat|AI")
    TMap<EElementalType, FUtilityProfile> ElementalProfiles;

    /** 是否允许动态切换元素 */
    UPROPERTY(EditAnywhere, Category = "ElementalCombat|AI")
    bool bAllowElementSwitching = true;

    /** 元素切换冷却时间（秒） */
    UPROPERTY(EditAnywhere, Category = "ElementalCombat|AI", meta = (ClampMin = "0.0"))
    float ElementSwitchCooldown = 2.0f;

    /** 最小元素优势阈值（低于此值不切换） */
    UPROPERTY(EditAnywhere, Category = "ElementalCombat|AI", meta = (ClampMin = "0.0", ClampMax = "1.0"))
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
