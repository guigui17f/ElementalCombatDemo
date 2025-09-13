// Copyright 2025 guigui17f. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UtilityAITypes.h"
#include "IUtilityScorer.generated.h"

/**
 * Utility评分器接口 - UE蓝图接口
 */
UINTERFACE(BlueprintType, Blueprintable)
class ELEMENTALCOMBAT_API UUtilityScorer : public UInterface
{
    GENERATED_BODY()
};

/**
 * Utility评分器接口实现
 * 定义AI决策中使用的评分能力
 */
class ELEMENTALCOMBAT_API IUtilityScorer
{
    GENERATED_BODY()

public:
    /**
     * 计算Utility评分
     * @param Context 评分上下文信息
     * @return 综合评分结果
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Utility AI")
    float CalculateUtilityScore(const FUtilityContext& Context);

    /**
     * 获取评分器的名称（用于调试）
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Utility AI")
    FString GetScorerName() const;

    /**
     * 检查评分器是否可用
     */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Utility AI")
    bool IsScoreValid(const FUtilityContext& Context) const;

    // C++原生接口（可选实现）
    virtual float CalculateUtilityScore_Implementation(const FUtilityContext& Context) { return 0.0f; }
    virtual FString GetScorerName_Implementation() const { return TEXT("BaseUtilityScorer"); }
    virtual bool IsScoreValid_Implementation(const FUtilityContext& Context) const { return true; }
};

/**
 * 基础Utility评分器组件
 * 提供默认的评分实现和辅助功能
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(AI), meta=(BlueprintSpawnableComponent))
class ELEMENTALCOMBAT_API UUtilityScorerComponent : public UActorComponent, public IUtilityScorer
{
    GENERATED_BODY()

public:
    UUtilityScorerComponent();

protected:
    /** 评分配置文件 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Utility Scorer")
    FUtilityProfile ScoringProfile;

    /** 评分缓存有效时间（秒） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Utility Scorer", meta = (ClampMin = "0.0"))
    float CacheValidDuration = 0.1f;

    /** 是否启用评分缓存 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Utility Scorer")
    bool bUseCaching = true;

    /** 评分器标识名称 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Utility Scorer")
    FString ScorerName = TEXT("DefaultScorer");

private:
    /** 缓存的评分结果 */
    mutable float CachedScore = 0.0f;    /** 缓存的评分有效性标记 */    mutable bool bCachedScoreValid = false;

    /** 缓存时间戳 */
    mutable float CacheTimestamp = -1.0f;

    /** 上一次评分的上下文哈希（用于缓存失效检查） */
    mutable uint32 LastContextHash = 0;

public:
    // IUtilityScorer接口实现
    /** 蓝图接口实现 */
    virtual float CalculateUtilityScore_Implementation(const FUtilityContext& Context) override;
    virtual FString GetScorerName_Implementation() const override;
    virtual bool IsScoreValid_Implementation(const FUtilityContext& Context) const override;

    /** 设置评分配置文件 */
    UFUNCTION(BlueprintCallable, Category = "Utility AI")
    void SetScoringProfile(const FUtilityProfile& NewProfile);

    /** 获取评分配置文件 */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Utility AI")
    const FUtilityProfile& GetScoringProfile() const { return ScoringProfile; }

    /** 清除评分缓存 */
    UFUNCTION(BlueprintCallable, Category = "Utility AI")
    void ClearCache();

    /** 获取缓存的评分结果（如果有效） */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Utility AI")
    float GetCachedScore() const { return CachedScore; }

    /** 检查缓存是否有效 */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Utility AI")
    bool IsCacheValid(const FUtilityContext& Context) const;

protected:
    /** 计算上下文哈希值（用于缓存失效检查） */
    uint32 CalculateContextHash(const FUtilityContext& Context) const;

    /** 执行实际的评分计算（不考虑缓存） */
    virtual float CalculateScoreInternal(const FUtilityContext& Context) const;

public:
    /** 调试：获取详细的评分信息 */
    UFUNCTION(BlueprintCallable, Category = "Utility AI|Debug")
    FString GetDebugScoreInfo(const FUtilityContext& Context) const;

    /** 调试：在屏幕上显示评分信息 */
    UFUNCTION(BlueprintCallable, Category = "Utility AI|Debug")
    void DisplayDebugScore(const FUtilityContext& Context, float DisplayDuration = 5.0f) const;
};

/**
 * 多评分器管理组件
 * 管理多个评分器并提供统一的评分接口
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup=(AI), meta=(BlueprintSpawnableComponent))
class ELEMENTALCOMBAT_API UMultiUtilityScorerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UMultiUtilityScorerComponent();

protected:
    /** 注册的评分器列表 */
    UPROPERTY(BlueprintReadOnly, Category = "Multi Utility Scorer")
    TMap<FString, TScriptInterface<IUtilityScorer>> RegisteredScorers;

    /** 默认评分器名称 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Multi Utility Scorer")
    FString DefaultScorerName = TEXT("Default");

public:
    /** 注册评分器 */
    UFUNCTION(BlueprintCallable, Category = "Utility AI")
    void RegisterScorer(const FString& Name, const TScriptInterface<IUtilityScorer>& Scorer);

    /** 注销评分器 */
    UFUNCTION(BlueprintCallable, Category = "Utility AI")
    void UnregisterScorer(const FString& Name);

    /** 获取指定评分器 */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Utility AI")
    TScriptInterface<IUtilityScorer> GetScorer(const FString& Name) const;

    /** 使用指定评分器计算分数 */
    UFUNCTION(BlueprintCallable, Category = "Utility AI")
    float CalculateScoreWithScorer(const FString& ScorerName, const FUtilityContext& Context) const;

    /** 使用默认评分器计算分数 */
    UFUNCTION(BlueprintCallable, Category = "Utility AI")
    float CalculateScore(const FUtilityContext& Context) const;

    /** 使用所有评分器计算分数并返回最高分 */
    UFUNCTION(BlueprintCallable, Category = "Utility AI")
    float CalculateBestScore(const FUtilityContext& Context, FString& OutBestScorerName) const;

    /** 获取所有注册的评分器名称 */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Utility AI")
    TArray<FString> GetRegisteredScorerNames() const;

    /** 检查是否有指定的评分器 */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Utility AI")
    bool HasScorer(const FString& Name) const;

    /** 设置默认评分器 */
    UFUNCTION(BlueprintCallable, Category = "Utility AI")
    void SetDefaultScorer(const FString& Name);

    /** 清除所有评分器的缓存 */
    UFUNCTION(BlueprintCallable, Category = "Utility AI")
    void ClearAllCaches();
};