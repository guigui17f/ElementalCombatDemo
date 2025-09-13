// Copyright 2025 guigui17f. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ElementalStateTreeTaskBase.h"
#include "AI/Utility/UtilityAITypes.h"
#include "Engine/DataTable.h"
#include "StateTreeUtilityTasks.generated.h"

class AElementalCombatEnemy;

/**
 * 通用Utility评分任务实例数据
 */
USTRUCT()
struct FStateTreeUniversalUtilityInstanceData : public FElementalStateTreeInstanceDataBase
{
    GENERATED_BODY()

    // 移除DataTable配置，改为从AIController获取配置

    /** 最终评分结果（输出） */
    UPROPERTY(VisibleAnywhere, Category = "Output")
    FUtilityScore CalculatedScore;

    /** 是否重新计算评分（每次进入状态时） */
    UPROPERTY(EditAnywhere, Category = "Utility Scoring")
    bool bRecalculateOnEnter = true;

    /** 是否持续更新评分（Tick时） */
    UPROPERTY(EditAnywhere, Category = "Utility Scoring")
    bool bContinuousUpdate = false;

    /** 持续更新间隔（秒） */
    UPROPERTY(EditAnywhere, Category = "Utility Scoring", 
              meta = (EditCondition = "bContinuousUpdate", ClampMin = "0.1"))
    float UpdateInterval = 1.0f;

private:
    /** 上次更新时间（内部使用） */
    float LastUpdateTime = -1.0f;

    friend struct FStateTreeUniversalUtilityTask;
};

STATETREE_POD_INSTANCEDATA(FStateTreeUniversalUtilityInstanceData);

/**
 * 通用Utility评分任务
 * 根据配置的评分配置文件计算综合评分
 */
USTRUCT(meta=(DisplayName="Universal Utility Scorer", Category="ElementalCombat|Utility"))
struct ELEMENTALCOMBAT_API FStateTreeUniversalUtilityTask : public FElementalStateTreeTaskBase
{
    GENERATED_BODY()

    using FInstanceDataType = FStateTreeUniversalUtilityInstanceData;
    virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

    FStateTreeUniversalUtilityTask() = default;

    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
    virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

#if WITH_EDITOR
    virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif

protected:
    /** 更新评分计算 */
    bool UpdateScore(FStateTreeExecutionContext& Context) const;

    /** 检查是否需要更新 */
    bool ShouldUpdate(const FInstanceDataType& InstanceData, float CurrentTime) const;
};

/**
 * 单项Utility评分任务实例数据
 */
USTRUCT()
struct FStateTreeUtilityConsiderationInstanceData : public FElementalStateTreeInstanceDataBase
{
    GENERATED_BODY()

    /** 单个评分因素配置 */
    UPROPERTY(EditAnywhere, Category = "Consideration")
    FUtilityConsideration Consideration;

    /** 计算出的单项评分（输出） */
    UPROPERTY(VisibleAnywhere, Category = "Output")
    float ConsiderationScore = 0.0f;

    /** 评分输入值（调试用，输出） */
    UPROPERTY(VisibleAnywhere, Category = "Output")
    float InputValue = 0.0f;

    /** 是否达到有效阈值 */
    UPROPERTY(VisibleAnywhere, Category = "Output")
    bool bScoreValid = false;

    /** 评分阈值（低于此值认为无效） */
    UPROPERTY(EditAnywhere, Category = "Consideration", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float ValidScoreThreshold = 0.1f;
};

STATETREE_POD_INSTANCEDATA(FStateTreeUtilityConsiderationInstanceData);

/**
 * 单项Utility评分任务
 * 计算单一评分因素的值，用于调试或特定条件判断
 */
USTRUCT(meta=(DisplayName="Utility Consideration", Category="ElementalCombat|Utility"))
struct ELEMENTALCOMBAT_API FStateTreeUtilityConsiderationTask : public FElementalStateTreeTaskBase
{
    GENERATED_BODY()

    using FInstanceDataType = FStateTreeUtilityConsiderationInstanceData;
    virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

    FStateTreeUtilityConsiderationTask() = default;

    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

#if WITH_EDITOR
    virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif
};

/**
 * Utility评分比较任务实例数据
 */
USTRUCT()
struct FStateTreeUtilityComparisonInstanceData : public FElementalStateTreeInstanceDataBase
{
    GENERATED_BODY()

    /** 第一个评分配置 */
    UPROPERTY(EditAnywhere, Category = "Comparison")
    FUtilityProfile ProfileA;

    /** 第二个评分配置 */
    UPROPERTY(EditAnywhere, Category = "Comparison")
    FUtilityProfile ProfileB;

    /** 评分A的结果（输出） */
    UPROPERTY(VisibleAnywhere, Category = "Output")
    FUtilityScore ScoreA;

    /** 评分B的结果（输出） */
    UPROPERTY(VisibleAnywhere, Category = "Output")
    FUtilityScore ScoreB;

    /** 更好的评分结果（输出） */
    UPROPERTY(VisibleAnywhere, Category = "Output")
    FUtilityScore BetterScore;

    /** 更好的配置名称（输出） */
    UPROPERTY(VisibleAnywhere, Category = "Output")
    FString BetterProfileName;

    /** A是否更好（输出） */
    UPROPERTY(VisibleAnywhere, Category = "Output")
    bool bIsABetter = false;

    /** 评分差异（输出） */
    UPROPERTY(VisibleAnywhere, Category = "Output")
    float ScoreDifference = 0.0f;
};

STATETREE_POD_INSTANCEDATA(FStateTreeUtilityComparisonInstanceData);

/**
 * Utility评分比较任务
 * 比较两个不同的评分配置，选择更好的那个
 */
USTRUCT(meta=(DisplayName="Utility Score Comparison", Category="ElementalCombat|Utility"))
struct ELEMENTALCOMBAT_API FStateTreeUtilityComparisonTask : public FElementalStateTreeTaskBase
{
    GENERATED_BODY()

    using FInstanceDataType = FStateTreeUtilityComparisonInstanceData;
    virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

    FStateTreeUtilityComparisonTask() = default;

    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

#if WITH_EDITOR
    virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif
};

/**
 * 动态Utility权重调整任务实例数据
 */
USTRUCT()
struct FStateTreeDynamicUtilityInstanceData : public FElementalStateTreeInstanceDataBase
{
    GENERATED_BODY()

    /** 基础评分配置 */
    UPROPERTY(EditAnywhere, Category = "Dynamic Utility")
    FUtilityProfile BaseProfile;

    /** 权重调整映射 */
    UPROPERTY(EditAnywhere, Category = "Dynamic Utility")
    TMap<EConsiderationType, float> WeightAdjustments;

    /** 是否基于当前状态动态调整权重 */
    UPROPERTY(EditAnywhere, Category = "Dynamic Utility")
    bool bUseDynamicAdjustment = true;

    /** 调整后的评分结果（输出） */
    UPROPERTY(VisibleAnywhere, Category = "Output")
    FUtilityScore AdjustedScore;

    /** 当前应用的权重（输出，调试用） */
    UPROPERTY(VisibleAnywhere, Category = "Output")
    TMap<EConsiderationType, float> CurrentWeights;
};

STATETREE_POD_INSTANCEDATA(FStateTreeDynamicUtilityInstanceData);

/**
 * 动态Utility权重调整任务
 * 根据当前情况动态调整评分权重
 */
USTRUCT(meta=(DisplayName="Dynamic Utility Weights", Category="ElementalCombat|Utility"))
struct ELEMENTALCOMBAT_API FStateTreeDynamicUtilityTask : public FElementalStateTreeTaskBase
{
    GENERATED_BODY()

    using FInstanceDataType = FStateTreeDynamicUtilityInstanceData;
    virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

    FStateTreeDynamicUtilityTask() = default;

    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

#if WITH_EDITOR
    virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif

protected:
    /** 计算动态权重调整 */
    void CalculateDynamicWeights(const FUtilityContext& UtilityContext, FInstanceDataType& InstanceData) const;
};