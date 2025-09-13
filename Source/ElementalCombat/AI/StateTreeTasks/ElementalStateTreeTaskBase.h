// Copyright 2025 guigui17f. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeConditionBase.h"
#include "EnvironmentQuery/EnvQueryInstanceBlueprintWrapper.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "AI/Utility/UtilityAITypes.h"
#include "ElementalStateTreeTaskBase.generated.h"

class AElementalCombatEnemy;
class AAIController;
class UEnvQueryInstanceBlueprintWrapper;

// EQS查询完成委托
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEQSRequestComplete, UEnvQueryInstanceBlueprintWrapper*, QueryInstance, EEnvQueryStatus::Type, QueryStatus);

/**
 * EQS查询结果缓存结构
 */
USTRUCT(BlueprintType)
struct ELEMENTALCOMBAT_API FEQSQueryCache
{
    GENERATED_BODY()

    /** 缓存的查询结果位置 */
    UPROPERTY(BlueprintReadOnly, Category = "EQS Cache")
    TArray<FVector> CachedLocations;

    /** 最佳位置 */
    UPROPERTY(BlueprintReadOnly, Category = "EQS Cache")
    FVector BestLocation = FVector::ZeroVector;

    /** 缓存时间戳 */
    UPROPERTY(BlueprintReadOnly, Category = "EQS Cache")
    float CacheTimestamp = -1.0f;

    /** 查询哈希值（用于检测查询参数是否变化） */
    UPROPERTY(BlueprintReadOnly, Category = "EQS Cache")
    int32 QueryHash = 0;

    /** 是否有效 */
    UPROPERTY(BlueprintReadOnly, Category = "EQS Cache")
    bool bIsValid = false;

    FEQSQueryCache()
    {
        Reset();
    }

    void Reset()
    {
        CachedLocations.Empty();
        BestLocation = FVector::ZeroVector;
        CacheTimestamp = -1.0f;
        QueryHash = 0;
        bIsValid = false;
    }

    bool IsExpired(float CurrentTime, float MaxAge) const
    {
        return !bIsValid || (CurrentTime - CacheTimestamp) > MaxAge;
    }
};

/**
 * Elemental StateTree任务基类
 * 提供Utility AI评分和EQS查询的通用功能
 */
USTRUCT()
struct ELEMENTALCOMBAT_API FElementalStateTreeTaskBase : public FStateTreeTaskCommonBase
{
    GENERATED_BODY()

protected:
    /** EQS查询缓存有效时间（秒） */
    UPROPERTY(EditAnywhere, Category = "Performance", meta = (ClampMin = "0.0"))
    float EQSCacheValidDuration = 1.0f;

    /** Utility评分缓存有效时间（秒） */
    UPROPERTY(EditAnywhere, Category = "Performance", meta = (ClampMin = "0.0"))
    float UtilityCacheValidDuration = 0.1f;

    /** 是否启用EQS缓存 */
    UPROPERTY(EditAnywhere, Category = "Performance")
    bool bUseEQSCache = true;

    /** 是否启用Utility缓存 */
    UPROPERTY(EditAnywhere, Category = "Performance")
    bool bUseUtilityCache = true;

    /** 是否输出调试信息 */
    UPROPERTY(EditAnywhere, Category = "Debug")
    bool bEnableDebugOutput = false;

    /** 调试信息显示时间 */
    UPROPERTY(EditAnywhere, Category = "Debug", meta = (ClampMin = "0.0"))
    float DebugDisplayDuration = 3.0f;

private:
    /** EQS查询结果缓存 */
    mutable TMap<int32, FEQSQueryCache> EQSCache;

    /** Utility评分缓存 */
    mutable TMap<int32, float> UtilityCache;

    /** Utility评分缓存时间戳 */
    mutable TMap<int32, float> UtilityCacheTimestamps;

protected:
    // === Utility AI辅助函数 ===

    /** 创建Utility评分上下文 */
    FUtilityContext CreateUtilityContext(const FStateTreeExecutionContext& Context) const;

    /** 使用缓存计算Utility评分 */
    float CalculateUtilityScoreWithCache(const FUtilityProfile& Profile, const FUtilityContext& UtilityContext) const;

    /** 计算Utility评分（不使用缓存） */
    float CalculateUtilityScoreDirect(const FUtilityProfile& Profile, const FUtilityContext& UtilityContext) const;

    /** 比较两个Utility评分，返回更好的那个 */
    float GetBetterUtilityScore(const float& ScoreA, const float& ScoreB) const;

    // === EQS查询辅助函数 ===

    /** 异步执行EQS查询 */
    void ExecuteEQSQuery(UEnvQuery* QueryTemplate, const FStateTreeExecutionContext& Context, 
                        FOnEQSRequestComplete OnComplete) const;

    /** 使用缓存执行EQS查询 */
    bool ExecuteEQSQueryWithCache(UEnvQuery* QueryTemplate, const FStateTreeExecutionContext& Context,
                                 TArray<FVector>& OutLocations, FVector& OutBestLocation) const;

    /** 计算EQS查询哈希（用于缓存键） */
    int32 CalculateEQSQueryHash(UEnvQuery* QueryTemplate, const FStateTreeExecutionContext& Context) const;

    /** 更新EQS缓存 */
    void UpdateEQSCache(int32 QueryHash, const TArray<FVector>& Locations, FVector BestLocation, float CurrentTime) const;

    /** 检查EQS缓存是否有效 */
    bool IsEQSCacheValid(int32 QueryHash, float CurrentTime) const;

    // === 数据获取辅助函数 ===

    /** 获取AI控制器 */
    AAIController* GetAIController(const FStateTreeExecutionContext& Context) const;

    /** 获取ElementalCombatEnemy */
    AElementalCombatEnemy* GetElementalCombatEnemy(const FStateTreeExecutionContext& Context) const;

    /** 获取目标Actor */
    AActor* GetTargetActor(const FStateTreeExecutionContext& Context) const;

    /** 获取当前世界时间 */
    float GetCurrentWorldTime(const FStateTreeExecutionContext& Context) const;

    /** 计算两个Actor之间的距离 */
    float CalculateDistance(AActor* ActorA, AActor* ActorB) const;

    /** 计算健康度百分比 */
    float CalculateHealthPercent(AActor* Actor) const;

    /** 计算元素优势值 */
    float CalculateElementAdvantage(const AElementalCombatEnemy* Self, const AActor* Target) const;

    /** 计算威胁等级 */
    float CalculateThreatLevel(const AElementalCombatEnemy* Self, const AActor* Target) const;

    // === 调试辅助函数 ===

    /** 输出调试日志 */
    void LogDebug(const FString& Message) const;

    /** 在屏幕上显示调试信息 */
    void DisplayDebugMessage(const FString& Message) const;

    /** 绘制调试球体 */
    void DrawDebugSphere(const UWorld* World, const FVector& Location, float Radius, 
                        FColor Color = FColor::Red, float Duration = 3.0f) const;

    /** 绘制调试线条 */
    void DrawDebugLine(const UWorld* World, const FVector& Start, const FVector& End,
                      FColor Color = FColor::Green, float Duration = 3.0f) const;

public:
    // === 缓存管理函数 ===

    /** 清除所有缓存 */
    void ClearAllCaches() const;

    /** 清除EQS缓存 */
    void ClearEQSCache() const;

    /** 清除Utility缓存 */
    void ClearUtilityCache() const;

    /** 清除过期缓存 */
    void ClearExpiredCaches(float CurrentTime) const;

    // === 性能监控接口 ===

    /** 获取EQS缓存统计信息 */
    FString GetEQSCacheStats() const;

    /** 获取Utility缓存统计信息 */
    FString GetUtilityCacheStats() const;

    /** 获取总体性能统计 */
    FString GetPerformanceStats() const;

private:
    /** 计算Utility上下文哈希 */
    int32 CalculateUtilityContextHash(const FUtilityContext& Context) const;

    /** EQS查询完成回调 */
    void OnEQSQueryCompleted(TSharedPtr<FEnvQueryResult> QueryResult) const;
};

/**
 * 通用实例数据基类 - 包含常用的StateTree实例数据
 */
USTRUCT()
struct ELEMENTALCOMBAT_API FElementalStateTreeInstanceDataBase
{
    GENERATED_BODY()

    /** 执行任务的AI角色 */
    UPROPERTY(EditAnywhere, Category = Context)
    TObjectPtr<AElementalCombatEnemy> EnemyCharacter;

    /** AI控制器 */
    UPROPERTY(EditAnywhere, Category = Context)
    TObjectPtr<AAIController> AIController;

    /** 目标Actor（可选） */
    UPROPERTY(EditAnywhere, Category = Input)
    TObjectPtr<AActor> TargetActor;

    /** 目标位置（可选） */
    UPROPERTY(EditAnywhere, Category = Input)
    FVector TargetLocation = FVector::ZeroVector;

    /** 到目标的距离 */
    UPROPERTY(VisibleAnywhere, Category = Output)
    float DistanceToTarget = 0.0f;

    /** 任务执行状态输出 */
    UPROPERTY(VisibleAnywhere, Category = Output)
    bool bTaskCompleted = false;

    /** 错误信息输出 */
    UPROPERTY(VisibleAnywhere, Category = Output)
    FString ErrorMessage;
};

// 添加POD宏以支持StateTree绑定
STATETREE_POD_INSTANCEDATA(FElementalStateTreeInstanceDataBase);