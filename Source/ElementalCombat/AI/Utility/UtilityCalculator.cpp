// Copyright 2025 guigui17f. All Rights Reserved.

#include "UtilityCalculator.h"
#include "UtilityAITypes.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

// FUtilityContext 实现
float FUtilityContext::GetInputValue(EConsiderationType Type) const
{
    switch (Type)
    {
    case EConsiderationType::Health:
        return HealthPercent;
    case EConsiderationType::Distance:
        return FMath::Clamp(DistanceToTarget / 1000.0f, 0.0f, 1.0f); // 假设最大有效距离为1000单位
    case EConsiderationType::ElementAdvantage:
        return FMath::Clamp((ElementAdvantage + 1.0f) * 0.5f, 0.0f, 1.0f); // 转换[-1,1]到[0,1]
    case EConsiderationType::ThreatLevel:
        return ThreatLevel;
    case EConsiderationType::Cooldown:
        // 这里可以扩展为具体的技能冷却时间
        return GetCustomValue(TEXT("CooldownPercent"), 1.0f);
    case EConsiderationType::TeamStatus:
        return GetCustomValue(TEXT("TeamStatusPercent"), 0.5f);
    case EConsiderationType::Custom:
        // 需要配合CustomKey使用
        return 0.0f;
    default:
        return 0.0f;
    }
}

void FUtilityContext::SetCustomValue(const FString& Key, float Value)
{
    CustomValues.Add(Key, Value);
}

float FUtilityContext::GetCustomValue(const FString& Key, float DefaultValue) const
{
    const float* Found = CustomValues.Find(Key);
    return Found ? *Found : DefaultValue;
}

// FUtilityConsideration 实现
float FUtilityConsideration::CalculateScore(const FUtilityContext& Context) const
{
    // 获取原始输入值
    float RawInput = 0.0f;
    
    if (ConsiderationType == EConsiderationType::Custom)
    {
        RawInput = Context.GetCustomValue(CustomKey, 0.0f);
    }
    else
    {
        RawInput = Context.GetInputValue(ConsiderationType);
    }

    // 处理输入值
    float ProcessedInput = ProcessInputValue(RawInput);

    // 通过响应曲线计算输出值
    const FRichCurve* RichCurve = ResponseCurve.GetRichCurveConst();
    float RawOutput = RichCurve ? RichCurve->Eval(ProcessedInput) : ProcessedInput;

    // 处理输出值
    float FinalOutput = ProcessOutputValue(RawOutput);

    return FinalOutput;
}

float FUtilityConsideration::ProcessInputValue(float RawInput) const
{
    float ProcessedInput = RawInput;

    // 应用乘数
    ProcessedInput *= InputMultiplier;

    // 应用反转
    if (bInvertInput)
    {
        ProcessedInput = 1.0f - ProcessedInput;
    }

    // 限制到[0.0, 1.0]范围
    ProcessedInput = FMath::Clamp(ProcessedInput, 0.0f, 1.0f);

    return ProcessedInput;
}

float FUtilityConsideration::ProcessOutputValue(float RawOutput) const
{
    float ProcessedOutput = RawOutput;

    // 应用偏移
    ProcessedOutput += OutputOffset;

    // 限制到[0.0, 1.0]范围
    ProcessedOutput = FMath::Clamp(ProcessedOutput, 0.0f, 1.0f);

    return ProcessedOutput;
}

// FUtilityProfile 实现
FUtilityScore FUtilityProfile::CalculateScore(const FUtilityContext& Context) const
{
    FUtilityScore Result;
    Result.CalculationTime = Context.CurrentTime;

    if (Considerations.Num() == 0)
    {
        Result.bIsValid = false;
        return Result;
    }

    TArray<float> Scores;
    TArray<float> WeightArray;
    Scores.Reserve(Considerations.Num());
    WeightArray.Reserve(Considerations.Num());

    // 计算各项评分
    for (const FUtilityConsideration& Consideration : Considerations)
    {
        float Score = Consideration.CalculateScore(Context);
        float Weight = GetWeight(Consideration.ConsiderationType);

        Scores.Add(Score);
        WeightArray.Add(Weight);

        // 记录详细评分用于调试
        Result.ConsiderationScores.Add(Consideration.ConsiderationType, Score);

        // 调试输出
        if (bEnableDebugOutput)
        {
            FString TypeName = UEnum::GetValueAsString(Consideration.ConsiderationType);
            UE_LOG(LogTemp, Log, TEXT("UtilityProfile[%s]: %s = %.3f (Weight: %.3f)"), 
                   *ProfileName, *TypeName, Score, Weight);
        }
    }

    // 组合评分
    Result.FinalScore = CombineScores(Scores, WeightArray);

    // 检查是否达到最小阈值
    Result.bIsValid = Result.FinalScore >= MinScoreThreshold;

    if (bEnableDebugOutput)
    {
        UE_LOG(LogTemp, Log, TEXT("UtilityProfile[%s]: Final Score = %.3f (Valid: %s)"), 
               *ProfileName, Result.FinalScore, Result.bIsValid ? TEXT("Yes") : TEXT("No"));
    }

    return Result;
}

float FUtilityProfile::CombineScores(const TArray<float>& Scores, const TArray<float>& WeightArray) const
{
    if (Scores.Num() == 0)
    {
        return 0.0f;
    }

    if (bUseMultiplicativeCombination)
    {
        // 乘法组合：所有评分相乘，权重作为指数
        float Product = 1.0f;
        float TotalWeight = 0.0f;

        for (int32 i = 0; i < Scores.Num(); ++i)
        {
            if (WeightArray[i] > 0.0f)
            {
                // 使用权重作为指数：Score^Weight
                float WeightedScore = FMath::Pow(Scores[i], WeightArray[i]);
                Product *= WeightedScore;
                TotalWeight += WeightArray[i];
            }
        }

        // 如果没有有效权重，返回0
        if (TotalWeight <= 0.0f)
        {
            return 0.0f;
        }

        // 应用总权重的平方根来平衡结果
        return FMath::Pow(Product, 1.0f / TotalWeight);
    }
    else
    {
        // 加权平均组合
        float WeightedSum = 0.0f;
        float TotalWeight = 0.0f;

        for (int32 i = 0; i < Scores.Num(); ++i)
        {
            if (WeightArray[i] > 0.0f)
            {
                WeightedSum += Scores[i] * WeightArray[i];
                TotalWeight += WeightArray[i];
            }
        }

        // 如果没有有效权重，返回0
        if (TotalWeight <= 0.0f)
        {
            return 0.0f;
        }

        return WeightedSum / TotalWeight;
    }
}

// === UUtilityCalculator 静态函数实现 ===

FUtilityScore UUtilityCalculator::CalculateUtilityScore(const FUtilityProfile& Profile, const FUtilityContext& Context)
{
    return Profile.CalculateScore(Context);
}

float UUtilityCalculator::CalculateConsiderationScore(const FUtilityConsideration& Consideration, const FUtilityContext& Context)
{
    return Consideration.CalculateScore(Context);
}

float UUtilityCalculator::EvaluateResponseCurve(const FRuntimeFloatCurve& Curve, float InputValue)
{
    const FRichCurve* RichCurve = Curve.GetRichCurveConst();
    return RichCurve ? RichCurve->Eval(InputValue) : InputValue;
}

float UUtilityCalculator::CombineScores(const TArray<float>& Scores, const TArray<float>& Weights, bool bUseMultiplicative)
{
    if (Scores.Num() == 0 || Scores.Num() != Weights.Num())
    {
        return 0.0f;
    }

    if (bUseMultiplicative)
    {
        // 乘法组合
        float Product = 1.0f;
        float TotalWeight = 0.0f;

        for (int32 i = 0; i < Scores.Num(); ++i)
        {
            if (Weights[i] > 0.0f)
            {
                float WeightedScore = FMath::Pow(Scores[i], Weights[i]);
                Product *= WeightedScore;
                TotalWeight += Weights[i];
            }
        }

        return TotalWeight > 0.0f ? FMath::Pow(Product, 1.0f / TotalWeight) : 0.0f;
    }
    else
    {
        // 加权平均
        float WeightedSum = 0.0f;
        float TotalWeight = 0.0f;

        for (int32 i = 0; i < Scores.Num(); ++i)
        {
            if (Weights[i] > 0.0f)
            {
                WeightedSum += Scores[i] * Weights[i];
                TotalWeight += Weights[i];
            }
        }

        return TotalWeight > 0.0f ? WeightedSum / TotalWeight : 0.0f;
    }
}

bool UUtilityCalculator::ValidateUtilityProfile(const FUtilityProfile& Profile, FString& OutErrorMessage)
{
    if (Profile.ProfileName.IsEmpty())
    {
        OutErrorMessage = TEXT("Profile name is empty");
        return false;
    }

    if (Profile.Considerations.Num() == 0)
    {
        OutErrorMessage = TEXT("No considerations defined");
        return false;
    }

    for (const FUtilityConsideration& Consideration : Profile.Considerations)
    {
        if (Consideration.ConsiderationType == EConsiderationType::Custom && Consideration.CustomKey.IsEmpty())
        {
            OutErrorMessage = TEXT("Custom consideration has empty key");
            return false;
        }
    }

    OutErrorMessage = TEXT("Profile is valid");
    return true;
}