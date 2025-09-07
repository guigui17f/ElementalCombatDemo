// Copyright 2025 guigui17f. All Rights Reserved.

#include "ElementalCombatTestBase.h"
#include "Engine/World.h"
#include "Engine/Engine.h"

// Constructor
FElementalCombatTestBase::FElementalCombatTestBase(const FString& InName, const bool bInComplexTask)
	: FAutomationTestBase(InName, bInComplexTask)
	, TestWorld(nullptr)
{
}

// Destructor
FElementalCombatTestBase::~FElementalCombatTestBase()
{
	CleanupTestWorld();
}

// FAutomationTestBase overrides
bool FElementalCombatTestBase::RunTest(const FString& Parameters)
{
	// 默认实现，子类可以重写
	return true;
}

EAutomationTestFlags FElementalCombatTestBase::GetTestFlags() const
{
	return EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter;
}

FString FElementalCombatTestBase::GetBeautifiedTestName() const
{
	return GetTestName();
}

uint32 FElementalCombatTestBase::GetRequiredDeviceNum() const
{
	return 1;
}

void FElementalCombatTestBase::GetTests(TArray<FString>& OutBeautifiedNames, TArray<FString>& OutTestCommands) const
{
	OutBeautifiedNames.Add(GetBeautifiedTestName());
	OutTestCommands.Add(GetTestName());
}

UWorld* FElementalCombatTestBase::GetTestWorld() const
{
	return TestWorld;
}

void FElementalCombatTestBase::SetUp()
{
	// 默认空实现，子类可以重写
}

void FElementalCombatTestBase::TearDown()
{
	// 默认空实现，子类可以重写
}

UWorld* FElementalCombatTestBase::CreateTestWorld()
{
	if (TestWorld)
	{
		return TestWorld;
	}

	// 创建一个新的测试世界
	TestWorld = UWorld::CreateWorld(EWorldType::Game, false);
	
	if (TestWorld)
	{
		// 初始化世界
		FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
		WorldContext.SetCurrentWorld(TestWorld);
		
		TestWorld->InitializeActorsForPlay(FURL());
		TestWorld->BeginPlay();
		
		UE_LOG(LogTemp, Log, TEXT("测试世界创建成功"));
	}
	
	return TestWorld;
}

void FElementalCombatTestBase::CleanupTestWorld()
{
	if (TestWorld)
	{
		// 清理所有Actor
		TestWorld->DestroyWorld(false);
		
		// 从引擎中移除世界上下文
		for (int32 i = 0; i < GEngine->GetWorldContexts().Num(); ++i)
		{
			if (GEngine->GetWorldContexts()[i].World() == TestWorld)
			{
				GEngine->DestroyWorldContext(TestWorld);
				break;
			}
		}
		
		TestWorld = nullptr;
		UE_LOG(LogTemp, Log, TEXT("测试世界清理完成"));
	}
}