// Copyright 2025 guigui17f. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ElementalStateTreeTaskBase.h"
#include "EnvironmentQuery/EnvQuery.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "AI/ElementalCombatEnemy.h"
#include "StateTreeEQSTasks.generated.h"

class UEnvQuery;
class UEnvQueryInstanceBlueprintWrapper;

/**
 * EQS查询任务实例数据
 */
USTRUCT()
struct FStateTreeEQSQueryInstanceData : public FElementalStateTreeInstanceDataBase
{
    GENERATED_BODY()

    /** EQS查询模板 */
    UPROPERTY(EditAnywhere, Category = "EQS Query")
    TObjectPtr<UEnvQuery> QueryTemplate;

    /** 查询运行模式 */
    UPROPERTY(EditAnywhere, Category = "EQS Query")
    TEnumAsByte<EEnvQueryRunMode::Type> RunMode = EEnvQueryRunMode::SingleResult;

    /** 最大查询结果数量 */
    UPROPERTY(EditAnywhere, Category = "EQS Query", meta = (ClampMin = "1", ClampMax = "100"))
    int32 MaxResults = 1;

    /** 是否异步执行查询 */
    UPROPERTY(EditAnywhere, Category = "EQS Query")
    bool bRunAsync = true;

    /** 查询超时时间（秒） */
    UPROPERTY(EditAnywhere, Category = "EQS Query", meta = (ClampMin = "0.1", ClampMax = "10.0"))
    float QueryTimeout = 2.0f;

    // === 输出结果 ===

    /** 查询到的位置列表（输出） */
    UPROPERTY(VisibleAnywhere, Category = "Output")
    TArray<FVector> ResultLocations;

    /** 最佳位置（输出） */
    UPROPERTY(VisibleAnywhere, Category = "Output")
    FVector BestLocation = FVector::ZeroVector;

    /** 查询是否成功（输出） */
    UPROPERTY(VisibleAnywhere, Category = "Output")
    bool bQuerySucceeded = false;

    /** 查询错误信息（输出） */
    UPROPERTY(VisibleAnywhere, Category = "Output")
    FString QueryError;

    // === 内部状态 ===

private:
    /** 查询请求ID（内部使用） */
    int32 QueryRequestID = INDEX_NONE;

    /** 查询开始时间（内部使用） */
    float QueryStartTime = -1.0f;

    /** 查询状态（内部使用） */
    bool bIsQueryRunning = false;

    friend struct FStateTreeEQSQueryTask;
};

STATETREE_POD_INSTANCEDATA(FStateTreeEQSQueryInstanceData);

/**
 * EQS查询任务
 * 执行环境查询并输出结果位置
 */
USTRUCT(meta=(DisplayName="EQS Query", Category="ElementalCombat|EQS"))
struct ELEMENTALCOMBAT_API FStateTreeEQSQueryTask : public FElementalStateTreeTaskBase
{
    GENERATED_BODY()

    using FInstanceDataType = FStateTreeEQSQueryInstanceData;
    virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

    FStateTreeEQSQueryTask() = default;

    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;
    virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

#if WITH_EDITOR
    virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif

protected:
    /** 启动EQS查询 */
    bool StartEQSQuery(FStateTreeExecutionContext& Context) const;

    /** 检查查询是否完成 */
    bool CheckQueryCompletion(FStateTreeExecutionContext& Context) const;

    /** 处理查询结果 */
    void ProcessQueryResults(const TArray<FVector>& Locations, FInstanceDataType& InstanceData) const;

    /** 查询完成回调 */
    void OnEQSQueryComplete(UEnvQueryInstanceBlueprintWrapper* QueryInstance, EEnvQueryStatus::Type QueryStatus);
};

/**
 * EQS战术位置查询任务实例数据
 */
USTRUCT()
struct FStateTreeEQSTacticalQueryInstanceData : public FElementalStateTreeInstanceDataBase
{
    GENERATED_BODY()

    /** 近战位置查询模板 */
    UPROPERTY(EditAnywhere, Category = "Tactical Queries")
    TObjectPtr<UEnvQuery> MeleePositionQuery;

    /** 远程位置查询模板 */
    UPROPERTY(EditAnywhere, Category = "Tactical Queries")
    TObjectPtr<UEnvQuery> RangedPositionQuery;

    /** 掩体位置查询模板 */
    UPROPERTY(EditAnywhere, Category = "Tactical Queries")
    TObjectPtr<UEnvQuery> CoverPositionQuery;

    /** 撤退路径查询模板 */
    UPROPERTY(EditAnywhere, Category = "Tactical Queries")
    TObjectPtr<UEnvQuery> RetreatPathQuery;

    /** 根据当前情况自动选择查询类型 */
    UPROPERTY(EditAnywhere, Category = "Tactical Queries")
    bool bAutoSelectQueryType = true;

    /** 手动指定查询类型 */
    UPROPERTY(EditAnywhere, Category = "Tactical Queries", 
              meta = (EditCondition = "!bAutoSelectQueryType"))
    EAIAttackType ForcedQueryType = EAIAttackType::Melee;

    // === 输出结果 ===

    /** 推荐的战术位置（输出） */
    UPROPERTY(VisibleAnywhere, Category = "Output")
    FVector RecommendedPosition = FVector::ZeroVector;

    /** 使用的查询类型（输出） */
    UPROPERTY(VisibleAnywhere, Category = "Output")
    EAIAttackType UsedQueryType = EAIAttackType::None;

    /** 可选位置列表（输出） */
    UPROPERTY(VisibleAnywhere, Category = "Output")
    TArray<FVector> AlternativePositions;

    /** 位置质量评分（输出） */
    UPROPERTY(VisibleAnywhere, Category = "Output")
    float PositionQuality = 0.0f;
};

STATETREE_POD_INSTANCEDATA(FStateTreeEQSTacticalQueryInstanceData);

/**
 * EQS战术位置查询任务
 * 根据当前战斗情况选择合适的EQS查询
 */
USTRUCT(meta=(DisplayName="EQS Tactical Query", Category="ElementalCombat|EQS"))
struct ELEMENTALCOMBAT_API FStateTreeEQSTacticalQueryTask : public FElementalStateTreeTaskBase
{
    GENERATED_BODY()

    using FInstanceDataType = FStateTreeEQSTacticalQueryInstanceData;
    virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

    FStateTreeEQSTacticalQueryTask() = default;

    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

#if WITH_EDITOR
    virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif

protected:
    /** 自动选择查询类型 */
    EAIAttackType SelectQueryType(const FUtilityContext& UtilityContext, const FInstanceDataType& InstanceData) const;

    /** 获取指定类型的查询模板 */
    UEnvQuery* GetQueryForType(EAIAttackType QueryType, const FInstanceDataType& InstanceData) const;

    /** 评估位置质量 */
    float EvaluatePositionQuality(const FVector& Position, const FUtilityContext& Context) const;
};

/**
 * EQS与Utility结合任务实例数据
 */
USTRUCT()
struct FStateTreeEQSUtilityInstanceData : public FElementalStateTreeInstanceDataBase
{
    GENERATED_BODY()

    /** EQS查询模板 */
    UPROPERTY(EditAnywhere, Category = "EQS Utility")
    TObjectPtr<UEnvQuery> QueryTemplate;

    /** 位置评分配置文件 */
    UPROPERTY(EditAnywhere, Category = "EQS Utility")
    FUtilityProfile PositionScoringProfile;

    /** 最小可接受评分 */
    UPROPERTY(EditAnywhere, Category = "EQS Utility", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MinAcceptableScore = 0.3f;

    /** 最大考虑位置数量 */
    UPROPERTY(EditAnywhere, Category = "EQS Utility", meta = (ClampMin = "1", ClampMax = "50"))
    int32 MaxPositionsToEvaluate = 10;

    // === 输出结果 ===

    /** 最佳评分位置（输出） */
    UPROPERTY(VisibleAnywhere, Category = "Output")
    FVector BestScoredPosition = FVector::ZeroVector;

    /** 最佳位置的评分（输出） */
    UPROPERTY(VisibleAnywhere, Category = "Output")
    FUtilityScore BestPositionScore;

    /** 评估的位置及其评分（输出，调试用） */
    UPROPERTY(VisibleAnywhere, Category = "Output")
    TMap<FVector, float> EvaluatedPositions;

    /** 是否找到合适的位置（输出） */
    UPROPERTY(VisibleAnywhere, Category = "Output")
    bool bFoundValidPosition = false;
};

STATETREE_POD_INSTANCEDATA(FStateTreeEQSUtilityInstanceData);

/**
 * EQS与Utility结合任务
 * 使用EQS获取候选位置，然后用Utility评分选择最佳位置
 */
USTRUCT(meta=(DisplayName="EQS Utility Scorer", Category="ElementalCombat|EQS"))
struct ELEMENTALCOMBAT_API FStateTreeEQSUtilityTask : public FElementalStateTreeTaskBase
{
    GENERATED_BODY()

    using FInstanceDataType = FStateTreeEQSUtilityInstanceData;
    virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

    FStateTreeEQSUtilityTask() = default;

    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
    virtual EStateTreeRunStatus Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const override;

#if WITH_EDITOR
    virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif

protected:
    /** 使用Utility评分评估位置 */
    FUtilityScore EvaluatePosition(const FVector& Position, const FUtilityContext& BaseContext, 
                                  const FUtilityProfile& ScoringProfile) const;

    /** 处理EQS查询结果并进行Utility评分 */
    bool ProcessEQSResults(const TArray<FVector>& Locations, FStateTreeExecutionContext& Context) const;
};

/**
 * EQS缓存管理任务实例数据
 */
USTRUCT()
struct FStateTreeEQSCacheInstanceData : public FElementalStateTreeInstanceDataBase
{
    GENERATED_BODY()

    /** 清除所有缓存 */
    UPROPERTY(EditAnywhere, Category = "Cache Management")
    bool bClearAllCaches = false;

    /** 清除过期缓存 */
    UPROPERTY(EditAnywhere, Category = "Cache Management")
    bool bClearExpiredCaches = true;

    /** 显示缓存统计信息 */
    UPROPERTY(EditAnywhere, Category = "Cache Management")
    bool bShowCacheStats = false;

    /** 缓存统计信息（输出） */
    UPROPERTY(VisibleAnywhere, Category = "Output")
    FString CacheStatsInfo;
};

STATETREE_POD_INSTANCEDATA(FStateTreeEQSCacheInstanceData);

/**
 * EQS缓存管理任务
 * 管理EQS查询结果的缓存
 */
USTRUCT(meta=(DisplayName="EQS Cache Manager", Category="ElementalCombat|EQS"))
struct ELEMENTALCOMBAT_API FStateTreeEQSCacheManagerTask : public FElementalStateTreeTaskBase
{
    GENERATED_BODY()

    using FInstanceDataType = FStateTreeEQSCacheInstanceData;
    virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

    FStateTreeEQSCacheManagerTask() = default;

    virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;

#if WITH_EDITOR
    virtual FText GetDescription(const FGuid& ID, FStateTreeDataView InstanceDataView, const IStateTreeBindingLookup& BindingLookup, EStateTreeNodeFormatting Formatting = EStateTreeNodeFormatting::Text) const override;
#endif
};