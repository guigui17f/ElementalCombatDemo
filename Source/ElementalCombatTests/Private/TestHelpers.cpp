// Copyright 2025 guigui17f. All Rights Reserved.

#include "TestHelpers.h"

#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

// FTestHelpers::FDamageTestData Implementation

FTestHelpers::FDamageTestData::FDamageTestData()
	: Damage(10.0f)
	, ImpactLocation(FVector::ZeroVector)
	, ImpulseDirection(FVector::ForwardVector)
	, ImpulseStrength(500.0f)
{
}

// FTestHelpers Implementation

FVector FTestHelpers::GetRandomLocation(float Range)
{
	return FVector(
		FMath::RandRange(-Range, Range),
		FMath::RandRange(-Range, Range),
		FMath::RandRange(0.0f, Range * 0.5f)  // 高度范围较小
	);
}

FRotator FTestHelpers::GetRandomRotation()
{
	return FRotator(
		FMath::RandRange(-45.0f, 45.0f),  // Pitch
		FMath::RandRange(0.0f, 360.0f),   // Yaw
		0.0f                               // Roll
	);
}

FTestHelpers::FDamageTestData FTestHelpers::CreateTestDamageData(float InDamage)
{
	FDamageTestData TestData;
	TestData.Damage = InDamage;
	TestData.ImpactLocation = GetRandomLocation(100.0f);
	TestData.ImpulseDirection = FVector(
		FMath::RandRange(-1.0f, 1.0f),
		FMath::RandRange(-1.0f, 1.0f),
		FMath::RandRange(0.2f, 1.0f)
	).GetSafeNormal();
	TestData.ImpulseStrength = FMath::RandRange(100.0f, 1000.0f);
	
	return TestData;
}

void FTestHelpers::WaitForFrames(UWorld* World, int32 NumFrames)
{
	if (!World)
	{
		return;
	}
	
	for (int32 i = 0; i < NumFrames; ++i)
	{
		World->Tick(LEVELTICK_All, 0.016f);  // 假设60 FPS
	}
}

void FTestHelpers::WaitForSeconds(UWorld* World, float Seconds)
{
	if (!World)
	{
		return;
	}
	
	const float DeltaTime = 0.016f;  // 60 FPS
	const int32 NumTicks = FMath::CeilToInt(Seconds / DeltaTime);
	
	for (int32 i = 0; i < NumTicks; ++i)
	{
		World->Tick(LEVELTICK_All, DeltaTime);
	}
}

bool FTestHelpers::AreFloatsNearlyEqual(const TArray<float>& Array1, const TArray<float>& Array2, float Tolerance)
{
	if (Array1.Num() != Array2.Num())
	{
		return false;
	}
	
	for (int32 i = 0; i < Array1.Num(); ++i)
	{
		if (!FMath::IsNearlyEqual(Array1[i], Array2[i], Tolerance))
		{
			return false;
		}
	}
	
	return true;
}

void FTestHelpers::CleanupTestActors(UWorld* World)
{
	if (!World)
	{
		return;
	}
	
	// 收集所有需要销毁的Actor
	TArray<AActor*> ActorsToDestroy;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (Actor && !Actor->IsA<AWorldSettings>())
		{
			ActorsToDestroy.Add(Actor);
		}
	}
	
	// 销毁所有Actor
	for (AActor* Actor : ActorsToDestroy)
	{
		Actor->Destroy();
	}
	
	// 清理待销毁的Actor - 使用全局垃圾回收
	CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
}

// FScopedTimer Implementation

FTestHelpers::FScopedTimer::FScopedTimer(const FString& InName)
	: Name(InName)
	, StartTime(FPlatformTime::Seconds())
{
	UE_LOG(LogTemp, Log, TEXT("[计时器] %s 开始"), *Name);
}

FTestHelpers::FScopedTimer::~FScopedTimer()
{
	double ElapsedTime = GetElapsedTime();
	UE_LOG(LogTemp, Log, TEXT("[计时器] %s 完成，耗时 %.3f 毫秒"), *Name, ElapsedTime * 1000.0);
}

double FTestHelpers::FScopedTimer::GetElapsedTime() const
{
	return FPlatformTime::Seconds() - StartTime;
}

// FTestDataGenerator Implementation

float FTestDataGenerator::GenerateRandomHP(float Min, float Max)
{
	return FMath::RandRange(Min, Max);
}

float FTestDataGenerator::GenerateRandomDamage(float Min, float Max)
{
	return FMath::RandRange(Min, Max);
}

int32 FTestDataGenerator::GenerateRandomElementType()
{
	// 0: None, 1: Metal, 2: Wood, 3: Water, 4: Fire, 5: Earth
	return FMath::RandRange(0, 5);
}

FString FTestDataGenerator::GenerateTestCharacterName()
{
	static int32 CharacterIndex = 0;
	return FString::Printf(TEXT("TestCharacter_%d"), CharacterIndex++);
}

FString FTestDataGenerator::GenerateTestEnemyName()
{
	static int32 EnemyIndex = 0;
	return FString::Printf(TEXT("TestEnemy_%d"), EnemyIndex++);
}