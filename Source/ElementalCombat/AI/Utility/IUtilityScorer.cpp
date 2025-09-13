// Copyright 2025 guigui17f. All Rights Reserved.

#include "IUtilityScorer.h"
#include "UtilityCalculator.h"
#include "Engine/Engine.h"

// === UUtilityScorerComponent 实现 ===

UUtilityScorerComponent::UUtilityScorerComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    ScorerName = TEXT("DefaultScorer");
}

float UUtilityScorerComponent::CalculateUtilityScore_Implementation(const FUtilityContext& Context)
{
    // 使用缓存系统
    if (bUseCaching)
    {
        uint32 ContextHash = GetTypeHash(Context.HealthPercent) ^ 
                           GetTypeHash(Context.DistanceToTarget) ^ 
                           GetTypeHash(Context.ElementAdvantage);
                           
        if (LastContextHash == ContextHash && CachedScore > 0.01f)
        {
            return CachedScore;
        }
        
        LastContextHash = ContextHash;
    }

    // 计算实际评分
    CachedScore = UUtilityCalculator::CalculateUtilityScore(ScoringProfile, Context);
    
    return CachedScore;
}

FString UUtilityScorerComponent::GetScorerName_Implementation() const
{
    return ScorerName;
}

bool UUtilityScorerComponent::IsScoreValid_Implementation(const FUtilityContext& Context) const
{
    return IsValid(this) && ScoringProfile.ProfileName.Len() > 0;
}

void UUtilityScorerComponent::SetScoringProfile(const FUtilityProfile& NewProfile)
{
    ScoringProfile = NewProfile;
    
    // 清除缓存
    CachedScore = 0.0f;
    LastContextHash = 0;
}

void UUtilityScorerComponent::ClearCache()
{
    CachedScore = 0.0f;
    LastContextHash = 0;
    CacheTimestamp = -1.0f;
}

bool UUtilityScorerComponent::IsCacheValid(const FUtilityContext& Context) const
{
    if (!bUseCaching)
        return false;
        
    uint32 ContextHash = CalculateContextHash(Context);
    
    return (LastContextHash == ContextHash && 
            CachedScore > 0.01f && 
            CacheTimestamp > 0.0f);
}

uint32 UUtilityScorerComponent::CalculateContextHash(const FUtilityContext& Context) const
{
    return GetTypeHash(Context.HealthPercent) ^ 
           GetTypeHash(Context.DistanceToTarget) ^ 
           GetTypeHash(Context.ElementAdvantage) ^
           GetTypeHash(Context.ThreatLevel);
}

float UUtilityScorerComponent::CalculateScoreInternal(const FUtilityContext& Context) const
{
    return UUtilityCalculator::CalculateUtilityScore(ScoringProfile, Context);
}

FString UUtilityScorerComponent::GetDebugScoreInfo(const FUtilityContext& Context) const
{
    float Score = CalculateScoreInternal(Context);
    
    FString DebugInfo = FString::Printf(TEXT("Scorer: %s\nScore: %.3f\nValid: %s\n"), 
                                       *ScorerName, Score, 
                                       Score > 0.01f ? TEXT("Yes") : TEXT("No"));
    
    DebugInfo += FString::Printf(TEXT("Health: %.3f\nDistance: %.3f\nElement Advantage: %.3f\n"),
                                 Context.HealthPercent, Context.DistanceToTarget, Context.ElementAdvantage);
    
    return DebugInfo;
}

void UUtilityScorerComponent::DisplayDebugScore(const FUtilityContext& Context, float DisplayDuration) const
{
    if (GEngine)
    {
        FString DebugInfo = GetDebugScoreInfo(Context);
        GEngine->AddOnScreenDebugMessage(-1, DisplayDuration, FColor::Yellow, DebugInfo);
    }
}

// === UMultiUtilityScorerComponent 实现 ===

UMultiUtilityScorerComponent::UMultiUtilityScorerComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    DefaultScorerName = TEXT("Default");
}

void UMultiUtilityScorerComponent::RegisterScorer(const FString& Name, const TScriptInterface<IUtilityScorer>& Scorer)
{
    if (Scorer.GetObject() && Scorer.GetInterface())
    {
        RegisteredScorers.Add(Name, Scorer);
    }
}

void UMultiUtilityScorerComponent::UnregisterScorer(const FString& Name)
{
    RegisteredScorers.Remove(Name);
}

TScriptInterface<IUtilityScorer> UMultiUtilityScorerComponent::GetScorer(const FString& Name) const
{
    const TScriptInterface<IUtilityScorer>* FoundScorer = RegisteredScorers.Find(Name);
    return FoundScorer ? *FoundScorer : TScriptInterface<IUtilityScorer>();
}

float UMultiUtilityScorerComponent::CalculateScoreWithScorer(const FString& ScorerName, const FUtilityContext& Context) const
{
    TScriptInterface<IUtilityScorer> Scorer = GetScorer(ScorerName);
    if (Scorer.GetInterface())
    {
        return Scorer.GetInterface()->Execute_CalculateUtilityScore(Scorer.GetObject(), Context);
    }
    
    return 0.0f; // Invalid score
}

float UMultiUtilityScorerComponent::CalculateScore(const FUtilityContext& Context) const
{
    return CalculateScoreWithScorer(DefaultScorerName, Context);
}

float UMultiUtilityScorerComponent::CalculateBestScore(const FUtilityContext& Context, FString& OutBestScorerName) const
{
    float BestScore = 0.0f;
    OutBestScorerName = TEXT("");
    
    for (const auto& Pair : RegisteredScorers)
    {
        const FString& ScorerName = Pair.Key;
        const TScriptInterface<IUtilityScorer>& Scorer = Pair.Value;
        
        if (Scorer.GetInterface())
        {
            float CurrentScore = Scorer.GetInterface()->Execute_CalculateUtilityScore(Scorer.GetObject(), Context);
            
            if (CurrentScore > BestScore)
            {
                BestScore = CurrentScore;
                OutBestScorerName = ScorerName;
            }
        }
    }
    
    return BestScore;
}

TArray<FString> UMultiUtilityScorerComponent::GetRegisteredScorerNames() const
{
    TArray<FString> Names;
    RegisteredScorers.GetKeys(Names);
    return Names;
}

bool UMultiUtilityScorerComponent::HasScorer(const FString& Name) const
{
    return RegisteredScorers.Contains(Name);
}

void UMultiUtilityScorerComponent::SetDefaultScorer(const FString& Name)
{
    if (HasScorer(Name))
    {
        DefaultScorerName = Name;
    }
}

void UMultiUtilityScorerComponent::ClearAllCaches()
{
    for (const auto& Pair : RegisteredScorers)
    {
        if (UUtilityScorerComponent* ScorerComp = Cast<UUtilityScorerComponent>(Pair.Value.GetObject()))
        {
            ScorerComp->ClearCache();
        }
    }
}