// Copyright 2025 guigui17f. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Curves/CurveFloat.h"
#include "Engine/DataTable.h"
#include "UtilityAITypes.generated.h"

class AActor;
class AElementalCombatEnemy;

/**
 * Utility评分类型枚举
 */
UENUM(BlueprintType)
enum class EConsiderationType : uint8
{
    None = 0,
    Health,           // 健康度评分
    Distance,         // 距离评分
    ElementAdvantage, // 元素优势评分
    ThreatLevel,      // 威胁等级评分
    Cooldown,         // 冷却时间评分
    TeamStatus,       // 队伍状态评分
    Custom            // 自定义评分
};

/**
 * Utility评分上下文 - 提供评分计算所需的所有信息
 */
USTRUCT(BlueprintType)
struct ELEMENTALCOMBAT_API FUtilityContext
{
    GENERATED_BODY()

    /** 执行评分的AI角色 */
    UPROPERTY(BlueprintReadWrite, Category = "ElementalCombat|AI")
    TWeakObjectPtr<AElementalCombatEnemy> SelfActor;

    /** 目标角色（如果有的话） */
    UPROPERTY(BlueprintReadWrite, Category = "ElementalCombat|AI")
    TWeakObjectPtr<AActor> TargetActor;

    /** 当前时间戳 */
    UPROPERTY(BlueprintReadWrite, Category = "ElementalCombat|AI")
    float CurrentTime = 0.0f;

    /** 距离目标的距离 */
    UPROPERTY(BlueprintReadWrite, Category = "ElementalCombat|AI")
    float DistanceToTarget = 0.0f;

    /** 自身健康度百分比 [0.0 - 1.0] */
    UPROPERTY(BlueprintReadWrite, Category = "ElementalCombat|AI")
    float HealthPercent = 1.0f;

    /** 目标健康度百分比 [0.0 - 1.0] */
    UPROPERTY(BlueprintReadWrite, Category = "ElementalCombat|AI")
    float TargetHealthPercent = 1.0f;

    /** 威胁等级 [0.0 - 1.0] */
    UPROPERTY(BlueprintReadWrite, Category = "ElementalCombat|AI")
    float ThreatLevel = 0.0f;

    /** 元素优势值 [-1.0, 0.0, 1.0] */
    UPROPERTY(BlueprintReadWrite, Category = "ElementalCombat|AI")
    float ElementAdvantage = 0.0f;

    /** 自定义数据映射 */
    UPROPERTY(BlueprintReadWrite, Category = "ElementalCombat|AI")
    TMap<FString, float> CustomValues;

    FUtilityContext()
    {
        SelfActor = nullptr;
        TargetActor = nullptr;
    }

    /** 从上下文获取指定类型的评分输入值 */
    float GetInputValue(EConsiderationType Type) const
    {
        switch (Type)
        {
        case EConsiderationType::Health:
            return HealthPercent;
        case EConsiderationType::Distance:
            return FMath::Clamp(DistanceToTarget / 1000.0f, 0.0f, 1.0f); // 标准化到1000单位
        case EConsiderationType::ElementAdvantage:
            return FMath::Clamp((ElementAdvantage + 1.0f) / 2.0f, 0.0f, 1.0f); // [-1,1] 转 [0,1]
        case EConsiderationType::ThreatLevel:
            return ThreatLevel;
        case EConsiderationType::Cooldown:
            return GetCustomValue(TEXT("CooldownPercent"), 1.0f);
        case EConsiderationType::TeamStatus:
            return GetCustomValue(TEXT("TeamStatusPercent"), 0.5f);
        case EConsiderationType::Custom:
            return 0.0f; // 需要通过GetCustomValue单独获取
        default:
            return 0.0f;
        }
    }

    /** 设置自定义值 */
    void SetCustomValue(const FString& Key, float Value)
    {
        CustomValues.Add(Key, Value);
    }

    /** 获取自定义值 */
    float GetCustomValue(const FString& Key, float DefaultValue = 0.0f) const
    {
        const float* Value = CustomValues.Find(Key);
        return Value ? *Value : DefaultValue;
    }
};

/**
 * Utility单项评分配置
 * 定义如何计算单一因素的评分
 */
USTRUCT(BlueprintType)
struct ELEMENTALCOMBAT_API FUtilityConsideration
{
    GENERATED_BODY()

    /** 评分类型 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ElementalCombat|AI")
    EConsiderationType ConsiderationType = EConsiderationType::Health;

    /** 自定义评分标识（当类型为Custom时使用） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ElementalCombat|AI",
              meta = (EditCondition = "ConsiderationType == EConsiderationType::Custom"))
    FString CustomKey;

    /** 响应曲线 - 将输入值[0.0-1.0]映射到输出值[0.0-1.0] */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ElementalCombat|AI")
    FRuntimeFloatCurve ResponseCurve;

    /** 是否反转输入值（1.0 - input） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ElementalCombat|AI")
    bool bInvertInput = false;

    /** 输入值乘数 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ElementalCombat|AI", meta = (ClampMin = "0.0"))
    float InputMultiplier = 1.0f;

    /** 输出值偏移 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ElementalCombat|AI", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
    float OutputOffset = 0.0f;

    FUtilityConsideration()
    {
        // 默认线性曲线
        ResponseCurve.EditorCurveData.AddKey(0.0f, 0.0f);
        ResponseCurve.EditorCurveData.AddKey(1.0f, 1.0f);
    }

    /** 根据上下文计算此项评分 [0.0 - 1.0] */
    float CalculateScore(const FUtilityContext& Context) const;

private:
    /** 处理输入值（应用反转和乘数） */
    float ProcessInputValue(float RawInput) const;

    /** 处理输出值（应用偏移和限制） */
    float ProcessOutputValue(float RawOutput) const;
};

/**
 * Utility评分配置文件
 * 定义一组评分因素及其权重
 */
USTRUCT(BlueprintType)
struct ELEMENTALCOMBAT_API FUtilityProfile
{
    GENERATED_BODY()

    /** 配置文件名称 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ElementalCombat|AI")
    FString ProfileName = TEXT("Default");

    /** 评分因素列表 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ElementalCombat|AI")
    TArray<FUtilityConsideration> Considerations;

    /** 各项权重 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ElementalCombat|AI")
    TMap<EConsiderationType, float> Weights;

    /** 组合方式：true=乘法，false=加法平均 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ElementalCombat|AI")
    bool bUseMultiplicativeCombination = false;

    /** 最小分数阈值（低于此值的结果被视为无效） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ElementalCombat|AI", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float MinScoreThreshold = 0.01f;

    /** 是否启用调试输出 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ElementalCombat|AI")
    bool bEnableDebugOutput = false;

    /** AI类型标签 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ElementalCombat|AI")
    TArray<FString> AITypeTags;

    /** 近战AI切换到远程攻击的距离阈值（单位：虚幻单位） */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ElementalCombat|AI", meta = (ClampMin = "0.0"))
    float MeleeToRangedSwitchDistance = 300.0f;

    FUtilityProfile()
    {
        // 默认权重为1.0
        for (int32 i = 1; i < static_cast<int32>(EConsiderationType::Custom); ++i)
        {
            Weights.Add(static_cast<EConsiderationType>(i), 1.0f);
        }
    }

    /** 根据此配置文件计算综合评分 */
    float CalculateScore(const FUtilityContext& Context, TMap<EConsiderationType, float>* OutConsiderationScores = nullptr, bool* bOutIsValid = nullptr) const;

    /** 获取指定类型的权重 */
    float GetWeight(EConsiderationType Type) const
    {
        const float* Weight = Weights.Find(Type);
        return Weight ? *Weight : 1.0f;
    }

    /** 设置权重 */
    void SetWeight(EConsiderationType Type, float Weight)
    {
        Weights.Add(Type, FMath::Max(0.0f, Weight));
    }

private:
    /** 组合多个评分为最终结果 */
    float CombineScores(const TArray<float>& Scores, const TArray<float>& WeightArray) const;
};

/**
 * 数据表行结构 - 用于配置预设的Utility配置文件
 */
USTRUCT(BlueprintType)
struct ELEMENTALCOMBAT_API FUtilityProfileTableRow : public FTableRowBase
{
    GENERATED_BODY()

    /** Utility配置文件 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ElementalCombat|AI")
    FUtilityProfile Profile;

    /** 描述信息 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ElementalCombat|AI")
    FString Description;
};