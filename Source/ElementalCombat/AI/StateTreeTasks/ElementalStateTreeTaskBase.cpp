// Copyright 2025 guigui17f. All Rights Reserved.

#include "ElementalStateTreeTaskBase.h"
#include "StateTreeExecutionContext.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "DrawDebugHelpers.h"
#include "AI/ElementalCombatEnemy.h"
#include "EnvironmentQuery/EnvQuery.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "Kismet/GameplayStatics.h"

// === Utility AI辅助函数实现 ===

FUtilityContext FElementalStateTreeTaskBase::CreateUtilityContext(const FStateTreeExecutionContext& Context) const
{
    FUtilityContext UtilityContext;
    UtilityContext.CurrentTime = GetCurrentWorldTime(Context);

    // 获取基本对象
    AElementalCombatEnemy* SelfEnemy = GetElementalCombatEnemy(Context);
    AActor* Target = GetTargetActor(Context);

    if (SelfEnemy)
    {
        UtilityContext.SelfActor = SelfEnemy;
        UtilityContext.HealthPercent = CalculateHealthPercent(SelfEnemy);

        if (Target)
        {
            UtilityContext.TargetActor = Target;
            UtilityContext.TargetHealthPercent = CalculateHealthPercent(Target);
            UtilityContext.DistanceToTarget = CalculateDistance(SelfEnemy, Target);
            UtilityContext.ElementAdvantage = CalculateElementAdvantage(SelfEnemy, Target);
            UtilityContext.ThreatLevel = CalculateThreatLevel(SelfEnemy, Target);
        }
    }

    return UtilityContext;
}

FUtilityScore FElementalStateTreeTaskBase::CalculateUtilityScoreWithCache(const FUtilityProfile& Profile, const FUtilityContext& UtilityContext) const
{
    if (!bUseUtilityCache)
    {
        return CalculateUtilityScoreDirect(Profile, UtilityContext);
    }

    int32 ContextHash = CalculateUtilityContextHash(UtilityContext);

    // 检查缓存
    const FUtilityScore* CachedScore = UtilityCache.Find(ContextHash);
    const float* CachedTimestamp = UtilityCacheTimestamps.Find(ContextHash);

    if (CachedScore && CachedTimestamp)
    {
        if ((UtilityContext.CurrentTime - *CachedTimestamp) <= UtilityCacheValidDuration)
        {
            if (bEnableDebugOutput)
            {
                LogDebug(FString::Printf(TEXT("Using cached Utility score: %.3f"), CachedScore->FinalScore));
            }
            return *CachedScore;
        }
    }

    // 计算新评分并缓存
    FUtilityScore NewScore = CalculateUtilityScoreDirect(Profile, UtilityContext);
    UtilityCache.Add(ContextHash, NewScore);
    UtilityCacheTimestamps.Add(ContextHash, UtilityContext.CurrentTime);

    if (bEnableDebugOutput)
    {
        LogDebug(FString::Printf(TEXT("Calculated new Utility score: %.3f"), NewScore.FinalScore));
    }

    return NewScore;
}

FUtilityScore FElementalStateTreeTaskBase::CalculateUtilityScoreDirect(const FUtilityProfile& Profile, const FUtilityContext& UtilityContext) const
{
    return Profile.CalculateScore(UtilityContext);
}

FUtilityScore FElementalStateTreeTaskBase::GetBetterUtilityScore(const FUtilityScore& ScoreA, const FUtilityScore& ScoreB) const
{
    return ScoreA.IsBetterThan(ScoreB) ? ScoreA : ScoreB;
}

// === EQS查询辅助函数实现 ===

void FElementalStateTreeTaskBase::ExecuteEQSQuery(UEnvQuery* QueryTemplate, const FStateTreeExecutionContext& Context, 
                                                  FOnEQSRequestComplete OnComplete) const
{
    if (!QueryTemplate)
    {
        LogDebug(TEXT("ExecuteEQSQuery: QueryTemplate is null"));
        return;
    }

    AAIController* Controller = GetAIController(Context);
    if (!Controller)
    {
        LogDebug(TEXT("ExecuteEQSQuery: No AI Controller found"));
        return;
    }

    UWorld* World = Controller->GetWorld();
    if (!World)
    {
        LogDebug(TEXT("ExecuteEQSQuery: No World found"));
        return;
    }

    // TODO: 实现完整的EQS集成
    // 当前为简化版本，返回默认位置
    int32 QueryHash = CalculateEQSQueryHash(QueryTemplate, Context);
    FEQSQueryCache& Cache = EQSCache.FindOrAdd(QueryHash);
    Cache.bIsValid = true;
    Cache.CachedLocations.Empty();
    Cache.CachedLocations.Add(Controller->GetPawn()->GetActorLocation() + FVector(100, 0, 0)); // 简单的位置偏移
    Cache.CacheTimestamp = World->GetTimeSeconds();
    Cache.BestLocation = Cache.CachedLocations[0];
    
    if (bEnableDebugOutput)
    {
        LogDebug(FString::Printf(TEXT("EQS Query simplified - using default location: %s"), 
                                *Cache.BestLocation.ToString()));
    }

    if (bEnableDebugOutput)
    {
        LogDebug(FString::Printf(TEXT("Executing EQS Query: %s"), *QueryTemplate->GetName()));
    }
}

bool FElementalStateTreeTaskBase::ExecuteEQSQueryWithCache(UEnvQuery* QueryTemplate, const FStateTreeExecutionContext& Context,
                                                          TArray<FVector>& OutLocations, FVector& OutBestLocation) const
{
    if (!QueryTemplate)
    {
        return false;
    }

    int32 QueryHash = CalculateEQSQueryHash(QueryTemplate, Context);
    float CurrentTime = GetCurrentWorldTime(Context);

    // 检查缓存
    if (bUseEQSCache && IsEQSCacheValid(QueryHash, CurrentTime))
    {
        const FEQSQueryCache& Cache = EQSCache[QueryHash];
        OutLocations = Cache.CachedLocations;
        OutBestLocation = Cache.BestLocation;

        if (bEnableDebugOutput)
        {
            LogDebug(FString::Printf(TEXT("Using cached EQS result: %d locations"), OutLocations.Num()));
        }

        return Cache.bIsValid;
    }

    // 执行同步查询（注意：这可能会影响性能）
    AAIController* Controller = GetAIController(Context);
    if (!Controller)
    {
        return false;
    }

    // TODO: 实现完整的EQS查询
    // 当前为简化实现，返回一些测试位置
    OutLocations.Empty();
    FVector BaseLocation = Controller->GetPawn()->GetActorLocation();
    
    // 生成一些测试位置
    OutLocations.Add(BaseLocation + FVector(100, 0, 0));
    OutLocations.Add(BaseLocation + FVector(-100, 0, 0));
    OutLocations.Add(BaseLocation + FVector(0, 100, 0));
    OutLocations.Add(BaseLocation + FVector(0, -100, 0));
    
    if (OutLocations.Num() > 0)
    {
        OutBestLocation = OutLocations[0];
    }

    // 更新缓存
    if (bUseEQSCache)
    {
        UpdateEQSCache(QueryHash, OutLocations, OutBestLocation, CurrentTime);
    }

    if (bEnableDebugOutput)
    {
        LogDebug(FString::Printf(TEXT("EQS Query simplified: %d locations generated"), OutLocations.Num()));
    }

    return OutLocations.Num() > 0;
}

int32 FElementalStateTreeTaskBase::CalculateEQSQueryHash(UEnvQuery* QueryTemplate, const FStateTreeExecutionContext& Context) const
{
    int32 Hash = 0;

    if (QueryTemplate)
    {
        Hash = HashCombine(Hash, GetTypeHash(QueryTemplate));
    }

    // 包含当前位置信息
    AElementalCombatEnemy* Enemy = GetElementalCombatEnemy(Context);
    if (Enemy)
    {
        FVector Location = Enemy->GetActorLocation();
        Hash = HashCombine(Hash, GetTypeHash(Location.X));
        Hash = HashCombine(Hash, GetTypeHash(Location.Y));
    }

    // 包含目标信息
    AActor* Target = GetTargetActor(Context);
    if (Target)
    {
        Hash = HashCombine(Hash, GetTypeHash(Target));
    }

    return Hash;
}

void FElementalStateTreeTaskBase::UpdateEQSCache(int32 QueryHash, const TArray<FVector>& Locations, FVector BestLocation, float CurrentTime) const
{
    FEQSQueryCache& Cache = EQSCache.FindOrAdd(QueryHash);
    Cache.CachedLocations = Locations;
    Cache.BestLocation = BestLocation;
    Cache.CacheTimestamp = CurrentTime;
    Cache.QueryHash = QueryHash;
    Cache.bIsValid = Locations.Num() > 0;
}

bool FElementalStateTreeTaskBase::IsEQSCacheValid(int32 QueryHash, float CurrentTime) const
{
    const FEQSQueryCache* Cache = EQSCache.Find(QueryHash);
    return Cache && !Cache->IsExpired(CurrentTime, EQSCacheValidDuration);
}

// === 数据获取辅助函数实现 ===

AAIController* FElementalStateTreeTaskBase::GetAIController(const FStateTreeExecutionContext& Context) const
{
    if (AActor* Owner = Cast<AActor>(Context.GetOwner()))
    {
        if (APawn* Pawn = Cast<APawn>(Owner))
        {
            return Cast<AAIController>(Pawn->GetController());
        }
    }
    return nullptr;
}

AElementalCombatEnemy* FElementalStateTreeTaskBase::GetElementalCombatEnemy(const FStateTreeExecutionContext& Context) const
{
    return Cast<AElementalCombatEnemy>(Context.GetOwner());
}

AActor* FElementalStateTreeTaskBase::GetTargetActor(const FStateTreeExecutionContext& Context) const
{
    // 尝试从GetPlayerInfo任务获取目标
    AElementalCombatEnemy* Enemy = GetElementalCombatEnemy(Context);
    if (Enemy && Enemy->GetController())
    {
        if (AAIController* AIController = Cast<AAIController>(Enemy->GetController()))
        {
            return AIController->GetFocusActor();
        }
    }

    // 备用方案：获取玩家Pawn
    if (AActor* Owner = Cast<AActor>(Context.GetOwner()))
    {
        if (UWorld* World = Owner->GetWorld())
        {
            return UGameplayStatics::GetPlayerPawn(World, 0);
        }
    }

    return nullptr;
}

float FElementalStateTreeTaskBase::GetCurrentWorldTime(const FStateTreeExecutionContext& Context) const
{
    if (AActor* Owner = Cast<AActor>(Context.GetOwner()))
    {
        if (UWorld* World = Owner->GetWorld())
        {
            return World->GetTimeSeconds();
        }
    }
    return 0.0f;
}

float FElementalStateTreeTaskBase::CalculateDistance(AActor* ActorA, AActor* ActorB) const
{
    if (!ActorA || !ActorB)
    {
        return FLT_MAX;
    }

    return FVector::Distance(ActorA->GetActorLocation(), ActorB->GetActorLocation());
}

float FElementalStateTreeTaskBase::CalculateHealthPercent(AActor* Actor) const
{
    if (!Actor)
    {
        return 0.0f;
    }

    // 尝试获取健康度组件（需要根据实际项目的健康度系统调整）
    if (ACharacter* Character = Cast<ACharacter>(Actor))
    {
        // 这里应该根据实际的健康度系统来实现
        // 暂时返回1.0作为默认值
        return 1.0f;
    }

    return 1.0f;
}

float FElementalStateTreeTaskBase::CalculateElementAdvantage(const AElementalCombatEnemy* Self, const AActor* Target) const
{
    if (!Self || !Target)
    {
        return 0.0f;
    }

    // 这里需要根据实际的元素系统实现
    // 返回值：1.0表示优势，0.0表示平等，-1.0表示劣势
    
    // 暂时返回中性值
    return 0.0f;
}

float FElementalStateTreeTaskBase::CalculateThreatLevel(const AElementalCombatEnemy* Self, const AActor* Target) const
{
    if (!Self || !Target)
    {
        return 0.0f;
    }

    // 基于距离的简单威胁计算
    float Distance = CalculateDistance(const_cast<AElementalCombatEnemy*>(Self), const_cast<AActor*>(Target));
    
    // 距离越近威胁越高
    float ThreatFromDistance = FMath::Clamp(1.0f - (Distance / 1000.0f), 0.0f, 1.0f);
    
    // 可以根据目标类型、武器等其他因素调整威胁等级
    
    return ThreatFromDistance;
}

// === 调试辅助函数实现 ===

void FElementalStateTreeTaskBase::LogDebug(const FString& Message) const
{
    if (bEnableDebugOutput)
    {
        UE_LOG(LogTemp, Log, TEXT("ElementalStateTree: %s"), *Message);
    }
}

void FElementalStateTreeTaskBase::DisplayDebugMessage(const FString& Message) const
{
    if (bEnableDebugOutput && GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, DebugDisplayDuration, FColor::Yellow, 
                                        FString::Printf(TEXT("StateTree: %s"), *Message));
    }
}

void FElementalStateTreeTaskBase::DrawDebugSphere(const UWorld* World, const FVector& Location, float Radius, 
                                                 FColor Color, float Duration) const
{
    if (bEnableDebugOutput && World)
    {
        ::DrawDebugSphere(World, Location, Radius, 12, Color, false, Duration);
    }
}

void FElementalStateTreeTaskBase::DrawDebugLine(const UWorld* World, const FVector& Start, const FVector& End,
                                               FColor Color, float Duration) const
{
    if (bEnableDebugOutput && World)
    {
        ::DrawDebugLine(World, Start, End, Color, false, Duration, 0, 2.0f);
    }
}

// === 缓存管理函数实现 ===

void FElementalStateTreeTaskBase::ClearAllCaches() const
{
    ClearEQSCache();
    ClearUtilityCache();
}

void FElementalStateTreeTaskBase::ClearEQSCache() const
{
    EQSCache.Empty();
    if (bEnableDebugOutput)
    {
        LogDebug(TEXT("EQS Cache cleared"));
    }
}

void FElementalStateTreeTaskBase::ClearUtilityCache() const
{
    UtilityCache.Empty();
    UtilityCacheTimestamps.Empty();
    if (bEnableDebugOutput)
    {
        LogDebug(TEXT("Utility Cache cleared"));
    }
}

void FElementalStateTreeTaskBase::ClearExpiredCaches(float CurrentTime) const
{
    // 清除过期的EQS缓存
    TArray<int32> ExpiredEQSKeys;
    for (const auto& Pair : EQSCache)
    {
        if (Pair.Value.IsExpired(CurrentTime, EQSCacheValidDuration))
        {
            ExpiredEQSKeys.Add(Pair.Key);
        }
    }
    for (int32 Key : ExpiredEQSKeys)
    {
        EQSCache.Remove(Key);
    }

    // 清除过期的Utility缓存
    TArray<int32> ExpiredUtilityKeys;
    for (const auto& Pair : UtilityCacheTimestamps)
    {
        if ((CurrentTime - Pair.Value) > UtilityCacheValidDuration)
        {
            ExpiredUtilityKeys.Add(Pair.Key);
        }
    }
    for (int32 Key : ExpiredUtilityKeys)
    {
        UtilityCache.Remove(Key);
        UtilityCacheTimestamps.Remove(Key);
    }

    if (bEnableDebugOutput && (ExpiredEQSKeys.Num() > 0 || ExpiredUtilityKeys.Num() > 0))
    {
        LogDebug(FString::Printf(TEXT("Cleared %d expired EQS caches and %d expired Utility caches"), 
                                ExpiredEQSKeys.Num(), ExpiredUtilityKeys.Num()));
    }
}

int32 FElementalStateTreeTaskBase::CalculateUtilityContextHash(const FUtilityContext& Context) const
{
    int32 Hash = 0;
    
    Hash = HashCombine(Hash, GetTypeHash(Context.DistanceToTarget));
    Hash = HashCombine(Hash, GetTypeHash(Context.HealthPercent));
    Hash = HashCombine(Hash, GetTypeHash(Context.TargetHealthPercent));
    Hash = HashCombine(Hash, GetTypeHash(Context.ThreatLevel));
    Hash = HashCombine(Hash, GetTypeHash(Context.ElementAdvantage));

    if (Context.TargetActor.IsValid())
    {
        Hash = HashCombine(Hash, GetTypeHash(Context.TargetActor.Get()));
    }

    // 包含自定义值
    for (const auto& Pair : Context.CustomValues)
    {
        Hash = HashCombine(Hash, GetTypeHash(Pair.Key));
        Hash = HashCombine(Hash, GetTypeHash(Pair.Value));
    }

    return Hash;
}

void FElementalStateTreeTaskBase::OnEQSQueryCompleted(UEnvQueryInstanceBlueprintWrapper* QueryInstance, EEnvQueryStatus::Type QueryStatus)
{
    if (bEnableDebugOutput)
    {
        FString StatusString = (QueryStatus == EEnvQueryStatus::Success) ? TEXT("Success") : TEXT("Failed");
        LogDebug(FString::Printf(TEXT("EQS Query completed with status: %s"), *StatusString));
    }
}

// === 性能监控接口实现 ===

FString FElementalStateTreeTaskBase::GetEQSCacheStats() const
{
    int32 ValidCaches = 0;
    int32 TotalCaches = EQSCache.Num();
    float CurrentTime = 0.0f; // 需要从某处获取当前时间

    for (const auto& Pair : EQSCache)
    {
        if (!Pair.Value.IsExpired(CurrentTime, EQSCacheValidDuration))
        {
            ValidCaches++;
        }
    }

    return FString::Printf(TEXT("EQS Cache: %d/%d valid (%.1f%%)"), 
                          ValidCaches, TotalCaches, 
                          TotalCaches > 0 ? (ValidCaches * 100.0f / TotalCaches) : 0.0f);
}

FString FElementalStateTreeTaskBase::GetUtilityCacheStats() const
{
    int32 ValidCaches = 0;
    int32 TotalCaches = UtilityCache.Num();
    float CurrentTime = 0.0f; // 需要从某处获取当前时间

    for (const auto& Pair : UtilityCacheTimestamps)
    {
        if ((CurrentTime - Pair.Value) <= UtilityCacheValidDuration)
        {
            ValidCaches++;
        }
    }

    return FString::Printf(TEXT("Utility Cache: %d/%d valid (%.1f%%)"), 
                          ValidCaches, TotalCaches,
                          TotalCaches > 0 ? (ValidCaches * 100.0f / TotalCaches) : 0.0f);
}

FString FElementalStateTreeTaskBase::GetPerformanceStats() const
{
    FString Stats;
    Stats += GetEQSCacheStats() + TEXT("\n");
    Stats += GetUtilityCacheStats();
    return Stats;
}