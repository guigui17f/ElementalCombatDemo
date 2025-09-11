// Copyright 2025 guigui17f. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UtilityAITypes.h"
#include "UtilityCalculator.generated.h"

/**
 * Utility AI评分计算器
 * 提供标准的评分计算功能
 */
UCLASS(BlueprintType, Category = "ElementalCombat|AI")
class ELEMENTALCOMBAT_API UUtilityCalculator : public UObject
{
    GENERATED_BODY()

public:
    /**
     * 计算Utility评分
     * @param Profile 评分配置文件
     * @param Context 评分上下文
     * @return 计算出的评分结果
     */
    UFUNCTION(BlueprintCallable, Category = "Utility AI", CallInEditor)
    static FUtilityScore CalculateUtilityScore(const FUtilityProfile& Profile, const FUtilityContext& Context);

    /**
     * 计算单项评分
     * @param Consideration 评分因素配置
     * @param Context 评分上下文
     * @return 单项评分值
     */
    UFUNCTION(BlueprintCallable, Category = "Utility AI", CallInEditor)
    static float CalculateConsiderationScore(const FUtilityConsideration& Consideration, const FUtilityContext& Context);

    /**
     * 评估响应曲线
     * @param Curve 响应曲线
     * @param InputValue 输入值
     * @return 输出值
     */
    UFUNCTION(BlueprintCallable, Category = "Utility AI", CallInEditor)
    static float EvaluateResponseCurve(const FRuntimeFloatCurve& Curve, float InputValue);

    /**
     * 组合多个评分为最终结果
     * @param Scores 各项评分
     * @param Weights 对应权重
     * @param bUseMultiplicative 是否使用乘法组合
     * @return 最终组合评分
     */
    UFUNCTION(BlueprintCallable, Category = "Utility AI", CallInEditor)
    static float CombineScores(const TArray<float>& Scores, const TArray<float>& Weights, bool bUseMultiplicative = false);

    /**
     * 验证评分配置文件
     * @param Profile 要验证的配置文件
     * @param OutErrorMessage 错误信息输出
     * @return 是否有效
     */
    UFUNCTION(BlueprintCallable, Category = "Utility AI", CallInEditor)
    static bool ValidateUtilityProfile(const FUtilityProfile& Profile, FString& OutErrorMessage);
};