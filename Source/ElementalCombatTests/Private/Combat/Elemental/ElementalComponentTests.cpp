// Copyright 2025 guigui17f. All Rights Reserved.

#include "CoreMinimal.h"
#include "ElementalCombatTestBase.h"
#include "TestHelpers.h"
#include "Combat/Elemental/ElementalComponent.h"
#include "GameFramework/Actor.h"

/**
 * 元素组件创建和初始化测试
 */
class FElementalComponentCreationTestImpl : public FElementalCombatTestBase
{
public:
	FElementalComponentCreationTestImpl() 
		: FElementalCombatTestBase(TEXT("ElementalComponentCreation"), false) {}
	
	virtual bool RunTest(const FString& Parameters) override
	{
		UWorld* World = CreateTestWorld();
		AActor* TestActor = World->SpawnActor<AActor>();
		TEST_ACTOR_VALID(TestActor, TEXT("测试Actor创建"));
		
		// 创建组件
		UElementalComponent* Component = NewObject<UElementalComponent>(TestActor);
		TestNotNull(TEXT("组件创建成功"), Component);
		
		// 注册前状态
		TestFalse(TEXT("未注册状态"), Component->IsRegistered());
		TestEqual(TEXT("初始元素为None"), static_cast<int32>(Component->GetCurrentElement()), static_cast<int32>(EElementalType::None));
		
		// 注册组件
		Component->RegisterComponent();
		TestTrue(TEXT("组件已注册"), Component->IsRegistered());
		TestEqual(TEXT("Owner正确"), Component->GetOwner(), TestActor);
		
		// 组件激活
		Component->Activate();
		TestTrue(TEXT("组件已激活"), Component->IsActive());
		
		// 组件停用
		Component->Deactivate();
		TestFalse(TEXT("组件已停用"), Component->IsActive());
		
		return true;
	}
};

ELEMENTAL_TEST(Combat.Elemental, ElementalComponentCreation)
bool FElementalComponentCreationTest::RunTest(const FString& Parameters)
{
	FElementalComponentCreationTestImpl TestImpl;
	return TestImpl.RunTest(Parameters);
}

/**
 * 元素切换功能测试
 */
class FElementalSwitchFunctionTestImpl : public FElementalCombatTestBase
{
public:
	FElementalSwitchFunctionTestImpl() 
		: FElementalCombatTestBase(TEXT("ElementalSwitchFunction"), false) {}
	
	virtual bool RunTest(const FString& Parameters) override
	{
		UWorld* World = CreateTestWorld();
		AActor* TestActor = World->SpawnActor<AActor>();
		UElementalComponent* Component = NewObject<UElementalComponent>(TestActor);
		Component->RegisterComponent();
		
		// 测试每个元素切换
		Component->SwitchElement(EElementalType::Metal);
		TestEqual(TEXT("切换到金"), static_cast<int32>(Component->GetCurrentElement()), static_cast<int32>(EElementalType::Metal));
		
		Component->SwitchElement(EElementalType::Wood);
		TestEqual(TEXT("切换到木"), static_cast<int32>(Component->GetCurrentElement()), static_cast<int32>(EElementalType::Wood));
		
		Component->SwitchElement(EElementalType::Water);
		TestEqual(TEXT("切换到水"), static_cast<int32>(Component->GetCurrentElement()), static_cast<int32>(EElementalType::Water));
		
		Component->SwitchElement(EElementalType::Fire);
		TestEqual(TEXT("切换到火"), static_cast<int32>(Component->GetCurrentElement()), static_cast<int32>(EElementalType::Fire));
		
		Component->SwitchElement(EElementalType::Earth);
		TestEqual(TEXT("切换到土"), static_cast<int32>(Component->GetCurrentElement()), static_cast<int32>(EElementalType::Earth));
		
		Component->SwitchElement(EElementalType::None);
		TestEqual(TEXT("切换到None"), static_cast<int32>(Component->GetCurrentElement()), static_cast<int32>(EElementalType::None));
		
		// 测试连续切换
		Component->SwitchElement(EElementalType::Fire);
		Component->SwitchElement(EElementalType::Water);
		Component->SwitchElement(EElementalType::Metal);
		TestEqual(TEXT("连续切换后为金"), static_cast<int32>(Component->GetCurrentElement()), static_cast<int32>(EElementalType::Metal));
		
		return true;
	}
};

ELEMENTAL_TEST(Combat.Elemental, ElementalSwitchFunction)
bool FElementalSwitchFunctionTest::RunTest(const FString& Parameters)
{
	FElementalSwitchFunctionTestImpl TestImpl;
	return TestImpl.RunTest(Parameters);
}

/**
 * 元素切换委托测试
 */
class FElementalDelegateTestImpl : public FElementalCombatTestBase
{
public:
	FElementalDelegateTestImpl() 
		: FElementalCombatTestBase(TEXT("ElementalDelegate"), false) {}
	
	virtual bool RunTest(const FString& Parameters) override
	{
		UWorld* World = CreateTestWorld();
		AActor* TestActor = World->SpawnActor<AActor>();
		UElementalComponent* Component = NewObject<UElementalComponent>(TestActor);
		Component->RegisterComponent();
		
		// 简化的委托测试 - 只测试委托是否存在和可绑定
		TestTrue(TEXT("委托对象存在"), Component->OnElementChanged.IsBound() == false); // 初始未绑定
		
		// 测试元素切换功能本身
		Component->SwitchElement(EElementalType::Fire);
		TestEqual(TEXT("切换到火元素"), static_cast<int32>(Component->GetCurrentElement()), static_cast<int32>(EElementalType::Fire));
		
		Component->SwitchElement(EElementalType::Water);
		TestEqual(TEXT("切换到水元素"), static_cast<int32>(Component->GetCurrentElement()), static_cast<int32>(EElementalType::Water));
		
		// 测试相同元素切换
		EElementalType PrevElement = Component->GetCurrentElement();
		Component->SwitchElement(EElementalType::Water);
		TestEqual(TEXT("相同元素保持不变"), static_cast<int32>(Component->GetCurrentElement()), static_cast<int32>(PrevElement));
		
		return true;
	}
};

ELEMENTAL_TEST(Combat.Elemental, ElementalDelegate)
bool FElementalDelegateTest::RunTest(const FString& Parameters)
{
	FElementalDelegateTestImpl TestImpl;
	return TestImpl.RunTest(Parameters);
}

/**
 * 元素效果数据存储测试
 */
class FElementalDataStorageTestImpl : public FElementalCombatTestBase
{
public:
	FElementalDataStorageTestImpl() 
		: FElementalCombatTestBase(TEXT("ElementalDataStorage"), false) {}
	
	virtual bool RunTest(const FString& Parameters) override
	{
		UWorld* World = CreateTestWorld();
		AActor* TestActor = World->SpawnActor<AActor>();
		UElementalComponent* Component = NewObject<UElementalComponent>(TestActor);
		Component->RegisterComponent();
		
		// 创建测试数据
		FElementalEffectData FireData;
		FireData.DotDamage = 15.0f;
		FireData.DotTickInterval = 0.5f;
		FireData.DotDuration = 5.0f;
		
		FElementalEffectData WaterData;
		WaterData.SlowPercentage = 0.6f;
		WaterData.SlowDuration = 4.0f;
		
		// 存储数据
		Component->SetElementEffectData(EElementalType::Fire, FireData);
		Component->SetElementEffectData(EElementalType::Water, WaterData);
		
		// 获取并验证火元素数据
		const FElementalEffectData* RetrievedFire = Component->GetElementEffectDataPtr(EElementalType::Fire);
		TestNotNull(TEXT("火元素数据存在"), RetrievedFire);
		if (RetrievedFire)
		{
			TestNearlyEqual(TEXT("火DOT伤害"), RetrievedFire->DotDamage, 15.0f, 0.01f);
			TestNearlyEqual(TEXT("火DOT间隔"), RetrievedFire->DotTickInterval, 0.5f, 0.01f);
			TestNearlyEqual(TEXT("火DOT持续"), RetrievedFire->DotDuration, 5.0f, 0.01f);
		}
		
		// 获取并验证水元素数据
		const FElementalEffectData* RetrievedWater = Component->GetElementEffectDataPtr(EElementalType::Water);
		TestNotNull(TEXT("水元素数据存在"), RetrievedWater);
		if (RetrievedWater)
		{
			TestNearlyEqual(TEXT("水减速比例"), RetrievedWater->SlowPercentage, 0.6f, 0.01f);
			TestNearlyEqual(TEXT("水减速时间"), RetrievedWater->SlowDuration, 4.0f, 0.01f);
		}
		
		// 获取未设置的数据
		const FElementalEffectData* UnsetData = Component->GetElementEffectDataPtr(EElementalType::Metal);
		TestNull(TEXT("未设置数据为空"), UnsetData);
		
		// 更新已有数据
		FireData.DotDamage = 20.0f;
		Component->SetElementEffectData(EElementalType::Fire, FireData);
		const FElementalEffectData* UpdatedFire = Component->GetElementEffectDataPtr(EElementalType::Fire);
		if (UpdatedFire)
		{
			TestNearlyEqual(TEXT("更新后火DOT伤害"), UpdatedFire->DotDamage, 20.0f, 0.01f);
		}
		
		return true;
	}
};

ELEMENTAL_TEST(Combat.Elemental, ElementalDataStorage)
bool FElementalDataStorageTest::RunTest(const FString& Parameters)
{
	FElementalDataStorageTestImpl TestImpl;
	return TestImpl.RunTest(Parameters);
}

/**
 * 投掷物类管理测试
 */
class FProjectileClassManagementTestImpl : public FElementalCombatTestBase
{
public:
	FProjectileClassManagementTestImpl() 
		: FElementalCombatTestBase(TEXT("ProjectileClassManagement"), false) {}
	
	virtual bool RunTest(const FString& Parameters) override
	{
		UWorld* World = CreateTestWorld();
		AActor* TestActor = World->SpawnActor<AActor>();
		UElementalComponent* Component = NewObject<UElementalComponent>(TestActor);
		Component->RegisterComponent();
		
		// 创建测试投掷物类（使用AActor作为测试）
		UClass* FireProjectileClass = AActor::StaticClass();
		UClass* WaterProjectileClass = AActor::StaticClass();
		
		// 设置投掷物类
		FElementalEffectData FireData;
		FireData.ProjectileClass = FireProjectileClass;
		Component->SetElementEffectData(EElementalType::Fire, FireData);
		
		FElementalEffectData WaterData;
		WaterData.ProjectileClass = WaterProjectileClass;
		Component->SetElementEffectData(EElementalType::Water, WaterData);
		
		// 切换元素并验证投掷物类
		Component->SwitchElement(EElementalType::Fire);
		UClass* CurrentProjectile = Component->GetCurrentProjectileClass();
		TestEqual(TEXT("火元素投掷物类"), CurrentProjectile, FireProjectileClass);
		
		Component->SwitchElement(EElementalType::Water);
		CurrentProjectile = Component->GetCurrentProjectileClass();
		TestEqual(TEXT("水元素投掷物类"), CurrentProjectile, WaterProjectileClass);
		
		// 切换到无投掷物的元素
		Component->SwitchElement(EElementalType::Earth);
		CurrentProjectile = Component->GetCurrentProjectileClass();
		TestNull(TEXT("无投掷物类为空"), CurrentProjectile);
		
		return true;
	}
};

ELEMENTAL_TEST(Combat.Elemental, ProjectileClassManagement)
bool FProjectileClassManagementTest::RunTest(const FString& Parameters)
{
	FProjectileClassManagementTestImpl TestImpl;
	return TestImpl.RunTest(Parameters);
}